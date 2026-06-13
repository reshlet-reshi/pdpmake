/*
 * Parse a makefile
 */
#include "make.h"
#include <glob.h>

int lineno;	// Physical line number in file
int dispno;	// Line number for display purposes

#include "input_expand.inc"
#include "input_condition.inc"
#include "input_classify.inc"

/*
 * Determine if a line is a target rule with an inline command.
 * Return a pointer to the semicolon separator if it is, else NULL.
 */
static char *
inline_command(char *line)
{
	char *p = find_char(line, ':');

	if (p)
		p = strchr(p, ';');
	return p;
}

/*
 * Parse input from the makefile and construct a tree structure of it.
 */
void
input(FILE *fd, int ilevel)
{
	char *p, *q, *s, *a, *str, *expanded, *copy;
	char *str1, *str2;
	struct name *np;
	struct depend *dp;
	struct cmd *cp;
	int startno, count;
	bool semicolon_cmd, seen_inference;
#if ENABLE_FEATURE_MAKE_EXTENSIONS
	uint8_t old_clevel = clevel;
	bool dbl;
	char *lib = NULL;
	glob_t gd;
	int nfile, i;
	char **files;
#else
	const bool dbl = FALSE;
#endif
#if ENABLE_FEATURE_MAKE_POSIX_2024
	bool minus;
#else
	const bool minus = FALSE;
#endif

	lineno = 0;
	str1 = readline(fd, FALSE);
	while (str1) {
		str2 = NULL;

		// Newlines and comments are handled differently in command lines
		// and other types of line.  Take a copy of the current line before
		// processing it as a non-command line in case it contains a
		// rule with a command line.  That is, a line of the form:
		//
		//   target: prereq; command
		//
		copy = xstrdup(str1);
		process_line(str1);
		str = str1;

		// Check for an include line
# if ENABLE_FEATURE_MAKE_EXTENSIONS
		if (!posix)
			while (isblank(*str))
				++str;
#endif
#if ENABLE_FEATURE_MAKE_POSIX_2024
		minus = !POSIX_2017 && *str == '-';
#endif
		p = str + minus;
		if (strncmp(p, "include", 7) == 0 && isblank(p[7])) {
			const char *old_makefile = makefile;
			int old_lineno = lineno;

			if (ilevel > 16)
				error("too many includes");

#if ENABLE_FEATURE_MAKE_POSIX_2024
			count = 0;
#endif
			q = expanded = expand_macros(p + 7, FALSE);
			while ((p = gettok(&q)) != NULL) {
				FILE *ifd;

#if ENABLE_FEATURE_MAKE_POSIX_2024
				++count;
				if (!POSIX_2017) {
					// Try to create include file or bring it up-to-date
					opts |= OPT_include;
					make(newname(p), 1);
					opts &= ~OPT_include;
				}
#endif
				if ((ifd = fopen(p, "r")) == NULL) {
					if (!minus)
						error("can't open include file '%s'", p);
				} else {
					makefile = p;
					input(ifd, ilevel + 1);
					fclose(ifd);
					makefile = old_makefile;
					lineno = old_lineno;
				}
#if ENABLE_FEATURE_MAKE_POSIX_2024
				if (POSIX_2017)
					break;
#endif
			}
#if ENABLE_FEATURE_MAKE_POSIX_2024
			if (POSIX_2017) {
				// In POSIX 2017 zero or more than one include file is
				// unspecified behaviour.
				if (p == NULL || gettok(&q)) {
					error("one include file per line");
				}
			} else if (count == 0) {
				// In POSIX 2024 no include file is unspecified behaviour.
# if ENABLE_FEATURE_MAKE_EXTENSIONS
				if (posix)
# endif
					error("no include file");
			}
#endif
			goto end_loop;
		}

		// Check for a macro definition
		str = str1;
#if ENABLE_FEATURE_MAKE_EXTENSIONS || ENABLE_FEATURE_MAKE_POSIX_2024
		// POSIX 2024 seems to allow a tab as the first character of
		// a macro definition, though most implementations don't.
		if (POSIX_2017 && *str == '\t')
			error("command not allowed here");
#endif
		if (find_char(str, '=') != NULL) {
			int level = (useenv || fd == NULL) ? 4 : 3;
			// Use a copy of the line:  we might need the original
			// if this turns out to be a target rule.
			char *copy2 = xstrdup(str);
#if ENABLE_FEATURE_MAKE_EXTENSIONS || ENABLE_FEATURE_MAKE_POSIX_2024
			char *newq = NULL;
			char eq = '\0';
#endif
			q = find_char(copy2, '=');		// q can't be NULL

#if ENABLE_FEATURE_MAKE_EXTENSIONS || ENABLE_FEATURE_MAKE_POSIX_2024
			if (q - 1 > copy2) {
				switch (q[-1]) {
				case ':':
# if ENABLE_FEATURE_MAKE_POSIX_2024
					// '::=' and ':::=' are from POSIX 2024.
					if (!POSIX_2017 && q - 2 > copy2 && q[-2] == ':') {
						if (q - 3 > copy2 && q[-3] == ':') {
							eq = 'B';	// BSD-style ':='
							q[-3] = '\0';
						} else {
							eq = ':';	// GNU-style ':='
							q[-2] = '\0';
						}
						break;
					}
# endif
# if ENABLE_FEATURE_MAKE_EXTENSIONS
					// ':=' is a non-POSIX extension.
					if (posix)
						break;
					IF_FEATURE_MAKE_POSIX_2024(goto set_eq;)
# else
					break;
# endif
# if ENABLE_FEATURE_MAKE_POSIX_2024
				case '+':
				case '?':
				case '!':
					// '+=', '?=' and '!=' are from POSIX 2024.
					if (POSIX_2017)
						break;
 IF_FEATURE_MAKE_EXTENSIONS(set_eq:)
# endif
					eq = q[-1];
					q[-1] = '\0';
					break;
				}
			}
#endif
			*q++ = '\0';	// Separate name and value
			while (isblank(*q))
				q++;
			if ((p = strrchr(q, '\n')) != NULL)
				*p = '\0';

			// Expand left-hand side of assignment
			p = expanded = expand_macros(copy2, FALSE);
			if ((a = gettok(&p)) == NULL)
				error("invalid macro assignment");

			// If the expanded LHS contains ':' and ';' it can't be a
			// macro assignment but it might be a target rule.
			if ((s = strchr(a, ':')) != NULL && strchr(s, ';') != NULL) {
				free(expanded);
				free(copy2);
				goto try_target;
			}

			if (gettok(&p))
				error("invalid macro assignment");

#if ENABLE_FEATURE_MAKE_EXTENSIONS || ENABLE_FEATURE_MAKE_POSIX_2024
			if (eq == ':') {
				// GNU-style ':='.  Expand right-hand side of assignment.
				// Macro is of type immediate-expansion.
				q = newq = expand_macros(q, FALSE);
				level |= M_IMMEDIATE;
			}
# if ENABLE_FEATURE_MAKE_POSIX_2024
			else if (eq == 'B') {
				// BSD-style ':='.  Expand right-hand side of assignment,
				// though not '$$'.  Macro is of type delayed-expansion.
				q = newq = expand_macros(q, TRUE);
			} else if (eq == '?' && getmp(a) != NULL) {
				// Skip assignment if macro is already set
				goto end_loop;
			} else if (eq == '+') {
				// Append to current value
				struct macro *mp = getmp(a);
				char *rhs;
				newq = mp && mp->m_val[0] ? xstrdup(mp->m_val) : NULL;
				if (mp && mp->m_immediate) {
					// Expand right-hand side of assignment (GNU make
					// compatibility)
					rhs = expand_macros(q, FALSE);
					level |= M_IMMEDIATE;
				} else {
					rhs = q;
				}
				newq = xappendword(newq, rhs);
				if (rhs != q)
					free(rhs);
				q = newq;
			} else if (eq == '!') {
				char *cmd = expand_macros(q, FALSE);
				q = newq = run_command(cmd);
				free(cmd);
			}
# endif
#endif
			setmacro(a, q, level);
#if ENABLE_FEATURE_MAKE_EXTENSIONS || ENABLE_FEATURE_MAKE_POSIX_2024
			free(newq);
#endif
			free(copy2);
			goto end_loop;
		}

		// If we get here it must be a target rule
 try_target:
		if (*str == '\t')	// Command without target
			error("command not allowed here");
		p = expanded = expand_macros(str, FALSE);

		// Look for colon separator
		q = find_colon(p);
		if (q == NULL)
			error("expected separator");

		*q++ = '\0';	// Separate targets and prerequisites

#if ENABLE_FEATURE_MAKE_EXTENSIONS
		// Double colon
		dbl = !posix && *q == ':';
		if (dbl)
			q++;
#endif

		// Look for semicolon separator
		cp = NULL;
		s = strchr(q, ';');
		if (s) {
			// Retrieve command from original or expanded copy of line
			char *copy3 = expand_macros(copy, FALSE);
			if ((p = inline_command(copy)) || (p = inline_command(copy3)))
				cp = newcmd(process_command(p + 1), cp);
			free(copy3);
			*s = '\0';
		}
		semicolon_cmd = cp != NULL && cp->c_cmd[0] != '\0';

		// Create list of prerequisites
		dp = NULL;
		while (((p = gettok(&q)) != NULL)) {
#if !ENABLE_FEATURE_MAKE_EXTENSIONS
# if ENABLE_FEATURE_MAKE_POSIX_2024
			if (!POSIX_2017 && strcmp(p, ".WAIT") == 0)
				continue;
# endif
			np = newname(p);
			dp = newdep(np, dp);
#else
			char *newp = NULL;

			if (!posix) {
				// Allow prerequisites of form library(member1 member2).
				// Leading and trailing spaces in the brackets are skipped.
				if (!lib) {
					s = strchr(p, '(');
					if (s && !ends_with_bracket(s) && strchr(q, ')')) {
						// Looks like an unterminated archive member
						// with a terminator later on the line.
						lib = p;
						if (s[1] != '\0') {
							p = newp = xconcat3(lib, ")", "");
							s[1] = '\0';
						} else {
							continue;
						}
					}
				} else if (ends_with_bracket(p)) {
					if (*p != ')')
						p = newp = xconcat3(lib, p, "");
					lib = NULL;
					if (newp == NULL)
						continue;
				} else {
					p = newp = xconcat3(lib, p, ")");
				}
			}

			// If not in POSIX mode expand wildcards in the name.
			nfile = 1;
			files = &p;
			if (!posix && wildcard(p, &gd)) {
				nfile = gd.gl_pathc;
				files = gd.gl_pathv;
			}
			for (i = 0; i < nfile; ++i) {
# if ENABLE_FEATURE_MAKE_POSIX_2024
				if (!POSIX_2017 && strcmp(files[i], ".WAIT") == 0)
					continue;
# endif
				np = newname(files[i]);
				dp = newdep(np, dp);
			}
			if (files != &p)
				globfree(&gd);
			free(newp);
#endif /* ENABLE_FEATURE_MAKE_EXTENSIONS */
		}
#if ENABLE_FEATURE_MAKE_EXTENSIONS
		lib = NULL;
#endif

		// Create list of commands
		startno = dispno;
		while ((str2 = readline(fd, TRUE)) && *str2 == '\t') {
			cp = newcmd(process_command(str2), cp);
			free(str2);
		}
		dispno = startno;

		// Create target names and attach rule to them
		q = expanded;
		count = 0;
		seen_inference = FALSE;
		while ((p = gettok(&q)) != NULL) {
#if ENABLE_FEATURE_MAKE_EXTENSIONS
			// If not in POSIX mode expand wildcards in the name.
			nfile = 1;
			files = &p;
			if (!posix && wildcard(p, &gd)) {
				nfile = gd.gl_pathc;
				files = gd.gl_pathv;
			}
			for (i = 0; i < nfile; ++i)
# define p files[i]
#endif
			{
				int ttype = target_type(p);

				np = newname(p);
				if (ttype != T_NORMAL) {
					// Enforce prerequisites/commands in POSIX mode
					if (IF_FEATURE_MAKE_EXTENSIONS(posix &&) 1) {
						if ((ttype & T_NOPREREQ) && dp)
							error_not_allowed("prerequisites", p);
						if ((ttype & T_INFERENCE)) {
							if (semicolon_cmd)
								error_in_inference_rule("'; command'");
							seen_inference = TRUE;
						}
						if ((ttype & T_COMMAND) && !cp &&
								!((ttype & T_INFERENCE) && !semicolon_cmd))
							error("commands required for %s", p);
						if (!(ttype & T_COMMAND) && cp)
							error_not_allowed("commands", p);
					}

					if ((ttype & T_INFERENCE)) {
						np->n_flag |= N_INFERENCE;
#if ENABLE_FEATURE_MAKE_EXTENSIONS
					} else if (strcmp(p, ".DEFAULT") == 0) {
						// .DEFAULT rule is a special case
						np->n_flag |= N_SPECIAL | N_INFERENCE;
#endif
					} else {
						np->n_flag |= N_SPECIAL;
					}
				} else if (!firstname) {
					firstname = np;
				}
				addrule(np, dp, cp, dbl);
				count++;
			}
#if ENABLE_FEATURE_MAKE_EXTENSIONS
# undef p
			if (files != &p)
				globfree(&gd);
#endif
		}
		if (IF_FEATURE_MAKE_EXTENSIONS(posix &&) seen_inference && count != 1)
			error_in_inference_rule("multiple targets");

		// Prerequisites and commands will be unused if there were
		// no targets.  Avoid leaking memory.
		if (count == 0) {
			freedeps(dp);
			freecmds(cp);
		}

 end_loop:
		free(str1);
		dispno = lineno;
		str1 = str2 ? str2 : readline(fd, FALSE);
		free(copy);
		free(expanded);
#if ENABLE_FEATURE_MAKE_EXTENSIONS
		if (!seen_first && fd) {
			if (findname(".POSIX")) {
				// The first non-comment line from a real makefile
				// defined the .POSIX special target.
				setenv("PDPMAKE_POSIXLY_CORRECT", "", 1);
				posix = TRUE;
			}
			seen_first = TRUE;
		}
#endif
	}
#if ENABLE_FEATURE_MAKE_EXTENSIONS
	// Conditionals aren't allowed to span files
	if (clevel != old_clevel)
		error("invalid conditional");
#endif
}

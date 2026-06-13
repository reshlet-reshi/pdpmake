#include "make_m2.h"

/*
 * Return a pointer to the next blank-delimited word or NULL if
 * there are none left.
 */
char *
gettok(char **ptr)
{
	char *p;

	while (isblank(**ptr))	// Skip blanks
		(*ptr)++;

	if (**ptr == '\0')	// Nothing after blanks
		return NULL;

	p = *ptr;		// Word starts here

	while (**ptr != '\0' && !isblank(**ptr))
		(*ptr)++;	// Find end of word

	// Terminate token and move on unless already at end of string
	if (**ptr != '\0') {
		**ptr = '\0';
		(*ptr)++;
	}

	return(p);
}

/*
 * Skip over (possibly adjacent or nested) macro expansions.
 */
char *
skip_macro(const char *s)
{
	while (*s && s[0] == '$') {
		if (s[1] == '(' || s[1] == '{') {
			char end;

			++s;
			if (*s == '(')
				end = ')';
			else
				end = '}';
			while (*s && *s != end)
				s = skip_macro(s + 1);
			if (*s == end)
				++s;
		} else if (s[1] != '\0') {
			s += 2;
		} else {
			break;
		}
	}
	return (char *)s;
}

/*
 * Process each whitespace-separated word in the input string:
 *
 * - replace paths with their directory or filename part
 * - replace prefixes and suffixes
 *
 * Returns an allocated string or NULL if the input is unmodified.
 */
static char *
modify_words(const char *val, int modifier, size_t lenf, size_t lenr,
				const char *find_pref, const char *repl_pref,
				const char *find_suff, const char *repl_suff)
{
	char *s, *copy, *word, *sep, *newword, *buf = NULL;
#if ENABLE_FEATURE_MAKE_POSIX_2024
	size_t find_pref_len = 0, find_suff_len = 0;
#endif

	if (!modifier && lenf == 0 && lenr == 0)
		return buf;

#if ENABLE_FEATURE_MAKE_POSIX_2024
	if (find_pref) {
		// get length of find prefix, e.g: src/
		find_pref_len = strlen(find_pref);
		// get length of find suffix, e.g: .c
		find_suff_len = lenf - find_pref_len - 1;
	}
#endif

	s = copy = xstrdup(val);
	while ((word = gettok(&s)) != NULL) {
		newword = NULL;
		if (modifier) {
			sep = strrchr(word, '/');
			if (modifier == 'D') {
				if (!sep) {
					word[0] = '.';	// no '/', return "."
					sep = word + 1;
				} else if (sep == word) {
					// '/' at start of word, return "/"
					sep = word + 1;
				}
				// else terminate at separator
				*sep = '\0';
			} else if (/* modifier == 'F' && */ sep) {
				word = sep + 1;
			}
		}
		if (find_pref != NULL || lenf != 0 || lenr != 0) {
			size_t lenw = strlen(word);
#if ENABLE_FEATURE_MAKE_POSIX_2024
			// This code implements pattern macro expansions:
			//    https://austingroupbugs.net/view.php?id=519
			//
			// find: <prefix>%<suffix>
			// example: src/%.c
			//
			// For a pattern of the form:
			//    $(string1:[op]%[os]=[np][%][ns])
			// lenf is the length of [op]%[os].  So lenf >= 1.
			if (find_pref != NULL && lenw + 1 >= lenf) {
				// If prefix and suffix of word match find_pref and
				// find_suff, then do substitution.
				if (strncmp(word, find_pref, find_pref_len) == 0 &&
						strcmp(word + lenw - find_suff_len, find_suff) == 0) {
					// replace: <prefix>[%<suffix>]
					// example: build/%.o or build/all.o (notice no %)
					// If repl_suff is NULL, replace whole word with repl_pref.
					if (!repl_suff) {
						word = newword = xstrdup(repl_pref);
					} else {
						word[lenw - find_suff_len] = '\0';
						word = newword = xconcat3(repl_pref,
									word + find_pref_len, repl_suff);
					}
				}
			} else
#endif
			if (lenw >= lenf && strcmp(word + lenw - lenf, find_suff) == 0) {
				word[lenw - lenf] = '\0';
				word = newword = xconcat3(word, repl_suff, "");
			}
		}
		buf = xappendword(buf, word);
		free(newword);
	}
	free(copy);
	return buf;
}

/*
 * Return a pointer to the next instance of a given character.  Macro
 * expansions are skipped so the ':' and '=' in $(VAR:.s1=.s2) aren't
 * detected as separators in macro definitions.  Some other situations
 * also require skipping the internals of a macro expansion.
 */
char *
find_char(const char *str, int c)
{
	const char *s;

	for (s = skip_macro(str); *s; s = skip_macro(s + 1)) {
		if (*s == c)
			return (char *)s;
	}
	return NULL;
}

/*
 * Check for a target rule by searching for a colon that isn't
 * part of a Windows path.  Return a pointer to the colon or NULL.
 */
char *
find_colon(char *p)
{
#if ENABLE_FEATURE_MAKE_EXTENSIONS && defined(__CYGWIN__)
	char *q;

	for (q = p; (q = strchr(q, ':')); ++q) {
		if (posix && !(pragma & P_WINDOWS))
			break;
		if (q == p || !isalpha(q[-1]) || q[1] != '/')
			break;
	}
	return q;
#else
	return strchr(p, ':');
#endif
}

/*
 * Recursively expand any macros in str to an allocated string.
 */
char *
expand_macros(const char *str, int except_dollar)
{
	char *exp, *newexp, *s, *t, *p, *q, *name;
	char *find, *replace, *modified;
	char *expval, *expfind, *find_suff, *repl_suff;
	char *find_pref = NULL, *repl_pref = NULL;
	size_t lenf, lenr;
	char modifier;
	struct macro *mp;

	exp = xstrdup(str);
	for (t = exp; *t; t++) {
		if (*t == '$') {
			if (t[1] == '\0') {
				break;
			}
#if ENABLE_FEATURE_MAKE_POSIX_2024
			if (t[1] == '$' && except_dollar) {
				t++;
				continue;
			}
#endif
			// Need to expand a macro.  Find its extent (s to t inclusive)
			// and take a copy of its content.
			s = t;
			t++;
			if (*t == '{' || *t == '(') {
				int end;

				if (*t == '{')
					end = '}';
				else
					end = ')';
				t = find_char(t, end);
				if (t == NULL)
					error("unterminated variable '%s'", s);
				name = xstrndup(s + 2, t - s - 2);
			} else {
				name = xmalloc(2);
				name[0] = *t;
				name[1] = '\0';
			}

			// Only do suffix replacement or pattern macro expansion
			// if both ':' and '=' are found, plus a '%' for the latter.
			// Suffix replacement is indicated by
			// find_pref == NULL && (lenf != 0 || lenr != 0);
			// pattern macro expansion by find_pref != NULL.
			expfind = NULL;
			find_suff = repl_suff = NULL;
			lenf = lenr = 0;
			if ((find = find_char(name, ':'))) {
				*find++ = '\0';
				expfind = expand_macros(find, FALSE);
				if ((replace = find_char(expfind, '='))) {
					*replace++ = '\0';
					lenf = strlen(expfind);
#if ENABLE_FEATURE_MAKE_POSIX_2024
					if (!POSIX_2017 && (find_suff = strchr(expfind, '%'))) {
						find_pref = expfind;
						repl_pref = replace;
						*find_suff++ = '\0';
						if ((repl_suff = strchr(replace, '%')))
							*repl_suff++ = '\0';
					} else
#endif
					{
						if (lenf == 0) {
							if (!ENABLE_FEATURE_MAKE_EXTENSIONS) {
								error("empty suffix%s", "");
							} else if (posix && !(pragma & P_EMPTY_SUFFIX)) {
								error("empty suffix%s",
									": allow with pragma empty_suffix");
							}
						}
						find_suff = expfind;
						repl_suff = replace;
						lenr = strlen(repl_suff);
					}
				}
			}

			p = q = name;
#if ENABLE_FEATURE_MAKE_POSIX_2024
			// If not in POSIX mode expand macros in the name.
			if (!POSIX_2017) {
				char *expname = expand_macros(name, FALSE);
				free(name);
				name = expname;
			} else
#endif
			// Skip over nested expansions in name
			do {
				*q++ = *p;
			} while ((p = skip_macro(p + 1)) && *p);

			// The internal macros support 'D' and 'F' modifiers
			modifier = '\0';
			switch (name[0]) {
			case '^':
			case '+':
				if (!ENABLE_FEATURE_MAKE_POSIX_2024 || POSIX_2017)
					break;
				// fall through
			case '@':
			case '%':
			case '?':
			case '<':
			case '*':
				if (name[2] == '\0') {
					if (name[1] == 'D' || name[1] == 'F') {
						modifier = name[1];
						name[1] = '\0';
					}
				}
				break;
			}

			modified = NULL;
			if ((mp = getmp(name)))  {
				// Recursive expansion
				if (mp->m_flag)
					error("recursive macro %s", name);
#if ENABLE_FEATURE_MAKE_POSIX_2024
				// Note if we've expanded $(MAKE)
				if (strcmp(name, "MAKE") == 0)
					opts |= OPT_make;
#endif
				mp->m_flag = TRUE;
#if ENABLE_FEATURE_MAKE_POSIX_2024 || ENABLE_FEATURE_MAKE_EXTENSIONS
				// Immediate-expansion macros aren't recursively expanded
				if (mp->m_immediate)
					expval = xstrdup(mp->m_val);
				else
#endif
					expval = expand_macros(mp->m_val, FALSE);
				mp->m_flag = FALSE;
				modified = modify_words(expval, modifier, lenf, lenr,
								find_pref, repl_pref, find_suff, repl_suff);
				if (modified)
					free(expval);
				else
					modified = expval;
			}
			free(name);
			free(expfind);

			if (modified && *modified) {
				// The text to be replaced by the macro expansion is
				// from s to t inclusive.
				*s = '\0';
				newexp = xconcat3(exp, modified, t + 1);
				t = newexp + (s - exp) + strlen(modified) - 1;
				free(exp);
				exp = newexp;
			} else {
				// Macro wasn't expanded or expanded to nothing.
				// Close the space occupied by the macro reference.
				q = t + 1;
				t = s - 1;
				while ((*s++ = *q++))
					continue;
			}
			free(modified);
		}
	}
	return exp;
}

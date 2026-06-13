#include "make.h"

/*
 * Instantiate all macros in an argv-style array of pointers.  Stop
 * processing at the first string that doesn't contain an equal sign.
 * As an extension, target arguments on the command line (level 1)
 * are skipped and will be processed later.
 */
char **
process_macros(char **argv, int level)
{
	char *equal;

	for (; *argv; argv++) {
#if ENABLE_FEATURE_MAKE_EXTENSIONS || ENABLE_FEATURE_MAKE_POSIX_2024
		char *colon = NULL;
		int immediate = 0;
#endif
#if ENABLE_FEATURE_MAKE_POSIX_2024
		int except_dollar = FALSE;
#endif

		if (!(equal = strchr(*argv, '='))) {
#if ENABLE_FEATURE_MAKE_EXTENSIONS
			// Skip targets on the command line
			if (!posix && level == 1)
				continue;
			else
#endif
				// Stop at first target
				break;
		}

#if ENABLE_FEATURE_MAKE_EXTENSIONS || ENABLE_FEATURE_MAKE_POSIX_2024
		if (equal - 1 > *argv && equal[-1] == ':') {
# if ENABLE_FEATURE_MAKE_POSIX_2024
			if (equal - 2 > *argv && equal[-2] == ':') {
				if (POSIX_2017)
					error("invalid macro assignment");
				if (equal - 3 > *argv  && equal[-3] == ':') {
					// BSD-style ':='.  Expand RHS, but not '$$',
					// resulting macro is delayed expansion.
					colon = equal - 3;
					except_dollar = TRUE;
				} else {
					// GNU-style ':='. Expand RHS, including '$$',
					// resulting macro is immediate expansion.
					colon = equal - 2;
					immediate = M_IMMEDIATE;
				}
				*colon = '\0';
			} else
# endif
			{
# if ENABLE_FEATURE_MAKE_EXTENSIONS
				if (posix)
					error("invalid macro assignment");
				colon = equal - 1;
				immediate = M_IMMEDIATE;
				*colon = '\0';
# else
				error("invalid macro assignment");
# endif
			}
		} else
#endif
			*equal = '\0';

		/* We want to process _most_ macro assignments.
		 * There are exceptions for particular values from the
		 * environment. */
		if (!((level & M_ENVIRON) &&
				(strcmp(*argv, "MAKEFLAGS") == 0
					|| strcmp(*argv, "SHELL") == 0
#if ENABLE_FEATURE_MAKE_POSIX_2024
					|| (strcmp(*argv, "CURDIR") == 0 && !useenv && !POSIX_2017)
#endif

				))) {
#if ENABLE_FEATURE_MAKE_EXTENSIONS || ENABLE_FEATURE_MAKE_POSIX_2024
			if (colon) {
				char *exp = expand_macros(equal + 1, except_dollar);
				setmacro(*argv, exp, level | immediate);
				free(exp);
			} else
#endif
				setmacro(*argv, equal + 1, level);
		}

#if ENABLE_FEATURE_MAKE_EXTENSIONS || ENABLE_FEATURE_MAKE_POSIX_2024
		if (colon)
			*colon = ':';
		else
#endif
			*equal = '=';
	}
	return argv;
}

/*
 * Update the MAKEFLAGS macro and environment variable to include any
 * command line options that don't have their default value (apart from
 * -f, -p and -S).  Also add any macros defined on the command line or
 * by the MAKEFLAGS environment variable (apart from MAKEFLAGS itself).
 * Add macros that were defined on the command line to the environment.
 */
void
update_makeflags(void)
{
	int i;
	char optbuf[] = "-?";
	char *makeflags = NULL;
	char *macro, *s;
	const char *t;
	struct macro *mp;

	t = OPTSTR1 + 1;
	for (i = 0; *t; t++) {
#if ENABLE_FEATURE_MAKE_POSIX_2024
		if (*t == ':')
			continue;
#endif
		if ((opts & OPT_MASK & (1 << i))) {
			optbuf[1] = *t;
			makeflags = xappendword(makeflags, optbuf);
#if ENABLE_FEATURE_MAKE_POSIX_2024
			if (*t == 'j') {
				makeflags = xappendword(makeflags, numjobs);
			}
#endif
		}
		i++;
	}

	for (i = 0; i < HTABSIZE; ++i) {
		for (mp = macrohead[i]; mp; mp = mp->m_next) {
			if ((mp->m_level == 1 || mp->m_level == 2) &&
					strcmp(mp->m_name, "MAKEFLAGS") != 0) {
				macro = xmalloc(strlen(mp->m_name) + 2 * strlen(mp->m_val) + 1);
				s = stpcpy(macro, mp->m_name);
				*s++ = '=';
				for (t = mp->m_val; *t; t++) {
					if (*t == '\\' || isblank(*t))
						*s++ = '\\';
					*s++ = *t;
				}
				*s = '\0';

				makeflags = xappendword(makeflags, macro);
				free(macro);

				// Add command line macro definitions to the environment
				if (mp->m_level == 1 && strcmp(mp->m_name, "SHELL") != 0)
					setenv(mp->m_name, mp->m_val, 1);
			}
		}
	}

	if (makeflags) {
		setmacro("MAKEFLAGS", makeflags, 0);
		setenv("MAKEFLAGS", makeflags, 1);
		free(makeflags);
	}
}

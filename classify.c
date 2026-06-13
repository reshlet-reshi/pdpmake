#include "make_m2.h"

#if ENABLE_FEATURE_MAKE_EXTENSIONS
#if ENABLE_FEATURE_MAKE_POSIX_2024
#define SPECIAL_TARGET_COUNT 10
#else
#define SPECIAL_TARGET_COUNT 7
#endif
#elif ENABLE_FEATURE_MAKE_POSIX_2024
#define SPECIAL_TARGET_COUNT 9
#else
#define SPECIAL_TARGET_COUNT 6
#endif

/*
 * Return a pointer to the suffix name if the argument is a known suffix
 * or NULL if it isn't.
 */
const char *
is_suffix(const char *s)
{
	struct name *np;
	struct rule *rp;
	struct depend *dp;

	np = newname(".SUFFIXES");
	for (rp = np->n_rule; rp; rp = rp->r_next) {
		for (dp = rp->r_dep; dp; dp = dp->d_next) {
			if (strcmp(s, dp->d_name->n_name) == 0) {
				return dp->d_name->n_name;
			}
		}
	}
	return NULL;
}

#if ENABLE_FEATURE_MAKE_EXTENSIONS
/*
 * Return TRUE if the argument is formed by concatenating two
 * known suffixes.
 */
static int
is_inference_target(const char *s)
{
	struct name *np;
	struct rule *rp1, *rp2;
	struct depend *dp1, *dp2;

	np = newname(".SUFFIXES");
	for (rp1 = np->n_rule; rp1; rp1 = rp1->r_next) {
		for (dp1 = rp1->r_dep; dp1; dp1 = dp1->d_next) {
			const char *suff1 = dp1->d_name->n_name;
			size_t len = strlen(suff1);

			if (strncmp(s, suff1, len) == 0) {
				for (rp2 = np->n_rule; rp2; rp2 = rp2->r_next) {
					for (dp2 = rp2->r_dep; dp2; dp2 = dp2->d_next) {
						const char *suff2 = dp2->d_name->n_name;
						if (strcmp(s + len, suff2) == 0) {
							return TRUE;
						}
					}
				}
			}
		}
	}
	return FALSE;
}
#endif

/*
 * Determine if the argument is a special target and return a set
 * of flags indicating its properties.
 */
int
target_type(char *s)
{
	int ret;
	static const char *s_name[] = {
		".DEFAULT",
		".POSIX",
		".IGNORE",
		".PRECIOUS",
		".SILENT",
		".SUFFIXES",
#if ENABLE_FEATURE_MAKE_POSIX_2024
		".PHONY",
		".NOTPARALLEL",
		".WAIT",
#endif
#if ENABLE_FEATURE_MAKE_EXTENSIONS
		".PRAGMA",
#endif
	};

	static const uint8_t s_type[] = {
		T_SPECIAL_NOPREREQ_COMMAND,
		T_SPECIAL_NOPREREQ,
		T_SPECIAL,
		T_SPECIAL,
		T_SPECIAL,
		T_SPECIAL,
#if ENABLE_FEATURE_MAKE_POSIX_2024
		T_SPECIAL,
		T_SPECIAL_NOPREREQ,
		T_SPECIAL_NOPREREQ,
#endif
#if ENABLE_FEATURE_MAKE_EXTENSIONS
		T_SPECIAL,
#endif
	};

	// Check for one of the known special targets
	for (ret = 0; ret < SPECIAL_TARGET_COUNT; ret++)
		if (strcmp(s_name[ret], s) == 0)
			return s_type[ret];

	// Check for an inference rule
	ret = T_NORMAL;
#if ENABLE_FEATURE_MAKE_EXTENSIONS
	if (!posix) {
		if (is_suffix(s) || is_inference_target(s)) {
			ret = T_INFERENCE | T_NOPREREQ | T_COMMAND;
		}
	} else
#endif
	{
		// In POSIX inference rule targets must contain one or two dots
		char *sfx = suffix(s);
		if (*s == '.' && is_suffix(sfx)) {
			if (s == sfx) {	// Single suffix rule
				ret = T_INFERENCE | T_NOPREREQ | T_COMMAND;
			} else {
				// Suffix is valid, check that prefix is too
				*sfx = '\0';
				if (is_suffix(s))
					ret = T_INFERENCE | T_NOPREREQ | T_COMMAND;
				*sfx = '.';
			}
		}
	}
	return ret;
}

#if ENABLE_FEATURE_MAKE_EXTENSIONS
int
ends_with_bracket(const char *s)
{
	const char *t = strrchr(s, ')');
	return t && t[1] == '\0';
}
#endif

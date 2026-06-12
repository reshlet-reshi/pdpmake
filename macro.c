/*
 * Macro control for make
 */
#include "make_m2.h"

struct macro *macrohead[HTABSIZE];

struct macro *
getmp(const char *name)
{
	struct macro *mp;
	struct macro **slot;

	slot = macrohead + getbucket(name);
	for (mp = *slot; mp; mp = mp->m_next)
		if (strcmp(name, mp->m_name) == 0)
			return mp;
	return NULL;
}

static int
is_valid_macro(const char *name)
{
	const char *s;
	for (s = name; *s; ++s) {
		// In POSIX mode only a limited set of characters are guaranteed
		// to be allowed in macro names.
		if (PDPMAKE_CHECK_MACRO_NAME) {
			// Find the appropriate character set
			if (PDPMAKE_MACRO_USES_FNAME) {
				if (!isfname(*s))
					return FALSE;
			} else {
				if (!ispname(*s))
					return FALSE;
			}
		}
		// As an extension allow anything that can get through the
		// input parser, apart from the following.
		if (*s == '=')
			return FALSE;
		if (PDPMAKE_REJECT_BLANK_CNTRL) {
			if (isblank(*s) || iscntrl(*s))
				return FALSE;
		}
	}
	return TRUE;
}

#if ENABLE_FEATURE_MAKE_EXTENSIONS
static int
potentially_valid_macro(const char *name)
{
	int ret = FALSE;

	if (!(pragma & P_MACRO_NAME)) {
		pragma |= P_MACRO_NAME;
		ret = is_valid_macro(name);
		pragma &= ~P_MACRO_NAME;
	}
	return ret;
}

static void
error_invalid_macro(const char *name)
{
	if (potentially_valid_macro(name)) {
		error("invalid macro name '%s'%s", name,
				": allow with pragma macro_name");
	} else {
		error("invalid macro name '%s'%s", name, "");
	}
}
#else
static void
error_invalid_macro(const char *name)
{
	error("invalid macro name '%s'", name);
}
#endif

void
setmacro(const char *name, const char *val, int level)
{
	struct macro *mp;
	bool valid = level & M_VALID;
	bool from_env = level & M_ENVIRON;
	bool immediate = level & M_IMMEDIATE;
	const char *newval = val;

	if (newval == NULL)
		newval = "";

	level &= ~(M_IMMEDIATE | M_VALID | M_ENVIRON);
	mp = getmp(name);
	if (mp) {
		// Don't replace existing macro from a lower level
		if (level > mp->m_level)
			return;

		// Replace existing macro
		free(mp->m_val);
	} else {
		// If not defined, allocate space for new
		unsigned int bucket;
		struct macro **slot;

		if (!valid && !is_valid_macro(name)) {
			// Silently drop invalid names from the environment
			if (from_env)
				return;
			PDPMAKE_ERROR_INVALID_MACRO(name);
		}

		bucket = getbucket(name);
		slot = macrohead + bucket;
		mp = xmalloc(sizeof(struct macro));
		mp->m_next = *slot;
		*slot = mp;
		mp->m_flag = FALSE;
		mp->m_name = xstrdup(name);
	}
	PDPMAKE_SET_IMMEDIATE(mp, immediate);
	mp->m_level = level;
	mp->m_val = xstrdup(newval);
}

#if ENABLE_FEATURE_CLEAN_UP
void
freemacros(void)
{
	int i;
	struct macro *mp, *nextmp;

	for (i = 0; i < HTABSIZE; i++) {
		for (mp = macrohead[i]; mp; mp = nextmp) {
			nextmp = mp->m_next;
			free(mp->m_name);
			free(mp->m_val);
			free(mp);
		}
	}
}
#endif

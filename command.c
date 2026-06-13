#include "make_m2.h"

/*
 * Process a command line
 */
char *
process_command(char *s)
{
	char *t, *u;
#if ENABLE_FEATURE_MAKE_POSIX_2024
	int len;
	char *outside;
#endif

#if ENABLE_FEATURE_MAKE_EXTENSIONS
	if (!(pragma & P_COMMAND_COMMENT) && posix) {
		// POSIX strips comments from command lines
		t = strchr(s, '#');
		if (t) {
			*t = '\0';
			warning("comment in command removed: keep with pragma command_comment");
		}
	}
#endif

#if ENABLE_FEATURE_MAKE_POSIX_2024
	len = strlen(s) + 1;
	outside = xmalloc(len);
	memset(outside, 0, len);
	for (t = skip_macro(s); *t; t = skip_macro(t + 1)) {
		outside[t - s] = 1;
	}
#endif

	// Process escaped newlines.  Stop at first non-escaped newline.
	t = u = s;
	while (*u && *u != '\n') {
		if (u[0] == '\\' && u[1] == '\n') {
#if ENABLE_FEATURE_MAKE_POSIX_2024
			if (POSIX_2017 || outside[u - s]) {
#endif
				// Outside macro: remove tab following escaped newline.
				*t++ = *u++;
				*t++ = *u++;
				u += (*u == '\t');
#if ENABLE_FEATURE_MAKE_POSIX_2024
			} else {
				// Inside macro: replace escaped newline and any leading
				// whitespace on the following line with a single space.
				u += 2;
				while (isspace(*u))
					++u;
				*t++ = ' ';
			}
#endif
		} else {
			*t++ = *u++;
		}
	}
	*t = '\0';
#if ENABLE_FEATURE_MAKE_POSIX_2024
	free(outside);
#endif
	return s;
}

char *
run_command(const char *cmd)
{
	FILE *fd;
	char *s, *val = NULL;
	char buf[256];
	size_t len = 0, nread;

	if ((fd = popen(cmd, "r")) == NULL)
		return val;

	while (TRUE) {
		nread = fread(buf, 1, sizeof(buf), fd);
		if (nread == 0)
			break;

		val = xrealloc(val, len + nread + 1);
		memcpy(val + len, buf, nread);
		len += nread;
		val[len] = '\0';
	}
	pclose(fd);

	if (val == NULL)
		return val;

	// Strip leading whitespace in POSIX 2024 mode
	if (!ENABLE_FEATURE_MAKE_EXTENSIONS || posix) {
		s = val;
		while (isspace(*s)) {
			++s;
			--len;
		}

		if (len == 0) {
			free(val);
			return NULL;
		}
		memmove(val, s, len + 1);
	}

	// Remove one newline from the end (BSD compatibility)
	if (val[len - 1] == '\n')
		val[len - 1] = '\0';
	// Other newlines are changed to spaces
	for (s = val; *s; ++s) {
		if (*s == '\n')
			*s = ' ';
	}
	return val;
}

#include "make_m2.h"

/*
 * If fd is NULL read the built-in rules.  Otherwise read from the
 * specified file descriptor.
 */
static char *
make_fgets(char *s, int size, FILE *fd)
{
	if (fd)
		return fgets(s, size, fd);
	return getrules(s, size);
}

/*
 * Read a newline-terminated line into an allocated string.
 * Backslash-escaped newlines don't terminate the line.
 * Ignore comment lines.  Return NULL on EOF.
 */
char *
readline(FILE *fd, int want_command)
{
	char *p, *str = NULL;
	int pos = 0;
	int len = 0;

	while (TRUE) {
		if (len - pos < READLINE_CHUNK) {
			// Need more room
			len += READLINE_CHUNK;
			str = xrealloc(str, len);
			continue;
		}

		if (make_fgets(str + pos, len - pos, fd) == NULL) {
			if (pos)
				return str;
			free(str);
			return NULL;	// EOF
		}

		if ((p = strchr(str + pos, '\n')) == NULL) {
			pos = len - 1;
			continue;
		}
		lineno++;

		// Remove CR before LF
		if (p != str && p[-1] == '\r') {
			p[-1] = '\n';
			*p-- = '\0';
		}

		// Keep going if newline has been escaped
		if (p != str && p[-1] == '\\') {
			pos = p - str + 1;
			continue;
		}
		dispno = lineno;

#if ENABLE_FEATURE_MAKE_EXTENSIONS
		// Check for lines that are conditionally skipped.
		if (posix || !skip_line(str))
#endif
		{
			if (want_command && *str == '\t')
				return str;

			// Check for comment lines
			p = str;
			while (isblank(*p))
				p++;

#if ENABLE_FEATURE_MAKE_EXTENSIONS
			if (*p != '\n') {
				if (posix) {
					if (*str != '#')
						return str;
				} else if (*p != '#') {
					return str;
				}
			}
#else
			if (*p != '\n' && *str != '#')
				return str;
#endif
		}

		pos = 0;
	}
}

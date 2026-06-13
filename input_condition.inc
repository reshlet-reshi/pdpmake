/*
 * Process a non-command line
 */
static void
process_line(char *s)
{
	char *t;

	// Strip comment
#if ENABLE_FEATURE_MAKE_EXTENSIONS
	// don't treat '#' in macro expansion as a comment
	// nor '#' outside macro expansion preceded by backslash
	if (!posix) {
		char *u = s;
		while ((t = find_char(u, '#')) && t > u && t[-1] == '\\') {
			for (u = t; *u; ++u) {
				u[-1] = u[0];
			}
			*u = '\0';
			u = t;
		}
	} else
#endif
		t = strchr(s, '#');
	if (t)
		*t = '\0';

	// Replace escaped newline and any leading white space on the
	// following line with a single space.  Stop processing at a
	// non-escaped newline.
	for (t = s; *s && *s != '\n'; ) {
		if (s[0] == '\\' && s[1] == '\n') {
			s += 2;
			while (isspace(*s))
				++s;
			*t++ = ' ';
		} else {
			*t++ = *s++;
		}
	}
	*t = '\0';
}

#if ENABLE_FEATURE_MAKE_EXTENSIONS
enum {
	INITIAL = 0,
	SKIP_LINE = 1 << 0,
	EXPECT_ELSE = 1 << 1,
	GOT_MATCH = 1 << 2
};

#define IF_MAX 10

static uint8_t clevel = 0;
static uint8_t cstate[IF_MAX + 1] = {INITIAL};

/*
 * Extract strings following ifeq/ifneq and compare them.
 * Return -1 on error.
 */
static int
compare_strings(char *arg1)
{
	char *arg2, *end, term, *t1, *t2;
	int ret;

	// Get first string terminator.
	if (arg1[0] == '(')
		term = ',';
	else if (arg1[0] == '"' || arg1[0] == '\'')
		term =  arg1[0];
	else
		return -1;

	arg2 = find_char(++arg1, term);
	if (arg2 == NULL)
		return -1;
	*arg2++ = '\0';

	// Get second string terminator.
	if (term == ',') {
		term = ')';
	} else {
		// Skip spaces between quoted strings.
		while (isspace(arg2[0]))
			arg2++;
		if (arg2[0] == '"' || arg2[0] == '\'')
			term = arg2[0];
		else
			return -1;
		++arg2;
	}

	end = find_char(arg2, term);
	if (end == NULL)
		return -1;
	*end++ = '\0';

	if (gettok(&end) != NULL) {
		warning("unexpected text");
	}

	t1 = expand_macros(arg1, FALSE);
	t2 = expand_macros(arg2, FALSE);

	ret = strcmp(t1, t2) == 0;
	free(t1);
	free(t2);
	return ret;
}

/*
 * Process conditional directives and return TRUE if the current line
 * should be skipped.
 */
int
skip_line(const char *str1)
{
	char *copy, *q, *token;
	bool new_level = TRUE;
	// Default is to return skip flag for current level
	int ret = cstate[clevel] & SKIP_LINE;

	q = copy = xstrdup(str1);
	process_line(copy);
	if ((token = gettok(&q)) != NULL) {
		if (strcmp(token, "endif") == 0) {
			if (gettok(&q) != NULL)
				error_unexpected("text");
			if (clevel == 0)
				error_unexpected(token);
			--clevel;
			ret = TRUE;
			goto end;
		} else if (strcmp(token, "else") == 0) {
			if (!(cstate[clevel] & EXPECT_ELSE))
				error_unexpected(token);

			// If an earlier condition matched we'll now skip lines.
			// If not we don't, though an 'else if' may override this.
			if ((cstate[clevel] & GOT_MATCH))
				cstate[clevel] |= SKIP_LINE;
			else
				cstate[clevel] &= ~SKIP_LINE;

			token = gettok(&q);
			if (token == NULL) {
				// Simple else with no conditional directive
				cstate[clevel] &= ~EXPECT_ELSE;
				ret = TRUE;
				goto end;
			} else {
				// A conditional directive is now required ('else if').
				new_level = FALSE;
			}
		}

		if (strcmp(token, "ifdef") == 0 || strcmp(token, "ifndef") == 0 ||
				strcmp(token, "ifeq") == 0 || strcmp(token, "ifneq") == 0) {
			int match;

			if (token[2] == 'd' || token[3] == 'd') {
				// ifdef/ifndef: find out if macro is defined.
				char *name = gettok(&q);
				if (name != NULL && gettok(&q) == NULL) {
					char *t = expand_macros(name, FALSE);
					struct macro *mp = getmp(t);
					match = mp != NULL && mp->m_val[0] != '\0';
					free(t);
				} else {
					match = -1;
				}
			} else {
				// ifeq/ifneq: compare strings.
				match = compare_strings(q);
			}

			if (match >= 0) {
				if (new_level) {
					// Start a new level.
					if (clevel == IF_MAX)
						error("nesting too deep");
					++clevel;
					cstate[clevel] = EXPECT_ELSE | SKIP_LINE;
					// If we were skipping lines at the previous level
					// we need to continue doing that unconditionally
					// at the new level.
					if ((cstate[clevel - 1] & SKIP_LINE))
						cstate[clevel] |= GOT_MATCH;
				}

				if (!(cstate[clevel] & GOT_MATCH)) {
					if (token[2] == 'n')
						match = !match;
					if (match) {
						cstate[clevel] &= ~SKIP_LINE;
						cstate[clevel] |= GOT_MATCH;
					}
				}
			} else {
				error("invalid condition");
			}
			ret = TRUE;
		} else if (!new_level) {
			error("missing conditional");
		}
	}
 end:
	free(copy);
	return ret;
}
#endif

#include "make_m2.h"

static void
usage(int exit_code)
{
	FILE *fp;

	if (ENABLE_FEATURE_MAKE_EXTENSIONS && exit_code == 0)
		fp = stdout;
	else
		fp = stderr;

	fprintf(fp, "Usage: %s", myname);
#if ENABLE_FEATURE_MAKE_EXTENSIONS
	fprintf(fp, " [--posix] [-C path]");
#endif
	fprintf(fp, " [-f makefile]");
#if ENABLE_FEATURE_MAKE_POSIX_2024
	fprintf(fp, " [-j num]");
#endif
#if ENABLE_FEATURE_MAKE_EXTENSIONS
	fprintf(fp, " [-x pragma]\n\t [-ehiknpqrsSt] ");
#else
	fprintf(fp, " [-eiknpqrsSt] ");
#endif
#if ENABLE_FEATURE_MAKE_POSIX_2024
#if ENABLE_FEATURE_MAKE_EXTENSIONS
	fprintf(fp, "[macro[:[:[:]]]=val ...]");
#else
	fprintf(fp, "[macro[::[:]]=val ...]");
#endif
#else
#if ENABLE_FEATURE_MAKE_EXTENSIONS
	fprintf(fp, "[macro[:]=val ...]");
#else
	fprintf(fp, "[macro=val ...]");
#endif
#endif
	fprintf(fp, " [target ...]\n");

	fprintf(fp, "\nThis build supports:");
#if ENABLE_FEATURE_MAKE_EXTENSIONS
	fprintf(fp, " non-POSIX extensions,");
#if ENABLE_FEATURE_MAKE_POSIX_2024
	fprintf(fp, " POSIX 2024,");
#endif
	fprintf(fp, " POSIX 2017\n");
#else
#if ENABLE_FEATURE_MAKE_POSIX_2024
	fprintf(fp, " POSIX 2024");
#else
	fprintf(fp, " POSIX 2017");
#endif
#endif
#if ENABLE_FEATURE_MAKE_EXTENSIONS && ENABLE_FEATURE_MAKE_POSIX_2024
	{
		const char *level;

		if (DEFAULT_POSIX_LEVEL == STD_POSIX_2017)
			level = "2017";
		else
			level = "2024";

		fprintf(fp,
				"In strict POSIX mode the %s standard is enforced by default.\n",
				level);
	}
#endif
#if !ENABLE_FEATURE_MAKE_EXTENSIONS
#if ENABLE_FEATURE_MAKE_POSIX_2024
	fprintf(fp, "\nFor details see:\n");
	fprintf(fp, "  https://pubs.opengroup.org/onlinepubs/9799919799.2024edition/utilities/make.html\n");
#else
	fprintf(fp, "\nFor details see:\n");
	fprintf(fp, "  https://pubs.opengroup.org/onlinepubs/9699919799.2018edition/utilities/make.html\n");
#endif
#endif
	exit(exit_code);
}

/*
 * Process options from an argv array.  If from_env is non-zero we're
 * handling options from MAKEFLAGS so skip '-C', '-f', '-p' and '-x'.
 */
uint32_t
process_options(int argc, char **argv, int from_env)
{
	int opt;
	uint32_t flags = 0;

	while ((opt = getopt(argc, argv, OPTSTR OPT_OFFSET)) != -1) {
		switch(opt) {
#if ENABLE_FEATURE_MAKE_EXTENSIONS
		case 'C':
			if (!posix && !from_env) {
				if (chdir(optarg) == -1) {
					error("can't chdir to %s: %s", optarg, strerror(PDPMAKE_ERRNO));
				}
				flags |= OPT_C;
				break;
			}
			error("-C not allowed");
			break;
#endif
		case 'f':	// Alternate file name
			if (!from_env) {
				makefiles = newfile(optarg, makefiles);
				flags |= OPT_f;
			}
			break;
		case 'e':	// Prefer env vars to macros in makefiles
			flags |= OPT_e;
			break;
#if ENABLE_FEATURE_MAKE_EXTENSIONS
		case 'h':	// Print usage message and exit
			if (posix)
				error("-h not allowed");
			usage(0);
			break;
#endif
		case 'i':	// Ignore fault mode
			flags |= OPT_i;
			break;
#if ENABLE_FEATURE_MAKE_POSIX_2024
		case 'j':
			if (!POSIX_2017) {
				const char *s;

				for (s = optarg; *s; ++s) {
					if (!isdigit(*s)) {
						usage(2);
					}
				}
				free(numjobs);
				numjobs = xstrdup(optarg);
				flags |= OPT_j;
				break;
			}
			error("-j not allowed");
			break;
#endif
		case 'k':	// Continue on error
			flags |= OPT_k;
			flags &= ~OPT_S;
			break;
		case 'n':	// Pretend mode
			flags |= OPT_n;
			break;
		case 'p':
			if (!from_env)
				flags |= OPT_p;
			break;
		case 'q':
			flags |= OPT_q;
			break;
		case 'r':
			flags |= OPT_r;
			break;
		case 't':
			flags |= OPT_t;
			break;
		case 's':	// Silent about commands
			flags |= OPT_s;
			break;
		case 'S':	// Stop on error
			flags |= OPT_S;
			flags &= ~OPT_k;
			break;
#if ENABLE_FEATURE_MAKE_EXTENSIONS
		case 'x':	// Pragma
			if (!from_env) {
				set_pragma(optarg);
				flags |= OPT_x;
			}
			break;
#endif
		default:
			if (from_env)
				error("invalid MAKEFLAGS");
			else
				usage(2);
		}
	}
	return flags;
}

/*
 * Split the contents of MAKEFLAGS into an argv array.  If the return
 * value (call it fargv) isn't NULL the caller should free fargv[1] and
 * fargv.
 */
char **
expand_makeflags(int *fargc)
{
	const char *m, *makeflags = getenv("MAKEFLAGS");
	char *p, *argstr;
	int argc;
	char **argv;

	if (makeflags == NULL)
		return NULL;

	while (isblank(*makeflags))
		makeflags++;

	if (*makeflags == '\0')
		return NULL;

	p = argstr = xmalloc(strlen(makeflags) + 2);

	// If MAKEFLAGS doesn't start with a hyphen, doesn't look like
	// a macro definition and only contains valid option characters,
	// add a hyphen.
	argc = 3;
	if (makeflags[0] != '-' && strchr(makeflags, '=') == NULL) {
		if (strspn(makeflags, OPTSTR1 + 1) != strlen(makeflags))
			error("invalid MAKEFLAGS");
		*p++ = '-';
	} else {
		// MAKEFLAGS may need to be split, estimate size of argv array.
		for (m = makeflags; *m; ++m) {
			if (isblank(*m))
				argc++;
		}
	}

	argv = xmalloc(argc * sizeof(char *));
	argc = 0;
	argv[argc++] = (char *)myname;
	argv[argc++] = argstr;

	// Copy MAKEFLAGS into argstr, splitting at non-escaped blanks.
	m = makeflags;
	do {
		if (*m == '\\' && m[1] != '\0')
			m++;	// Skip backslash, copy next character unconditionally.
		else if (isblank(*m)) {
			// Terminate current argument and start a new one.
			*p++ = '\0';
			argv[argc++] = p;
			do {
				m++;
			} while (isblank(*m));
			continue;
		}
		*p++ = *m++;
	} while (*m != '\0');
	*p = '\0';
	argv[argc] = NULL;

	*fargc = argc;
	return argv;
}

/*
 * main_entry.inc: program entry.
 */

int
main(int argc, char **argv)
{
#if ENABLE_FEATURE_MAKE_POSIX_2024
	const char *path, *newpath = NULL;
#else
	const char *path = "make";
#endif
	char **fargv, **fargv0, *shell;
	int fargc, estat;
	bool found_target;
	FILE *ifd;
	struct file *fp;

	if (argc == 0) {
		return EXIT_FAILURE;
	}

	myname = basename(*argv);
#if ENABLE_FEATURE_MAKE_EXTENSIONS
	if (argv[1] && strcmp(argv[1], "--posix") == 0) {
		argv[1] = argv[0];
		++argv;
		--argc;
		setenv("PDPMAKE_POSIXLY_CORRECT", "", 1);
		posix = TRUE;
	} else {
		posix = getenv("PDPMAKE_POSIXLY_CORRECT") != NULL;
	}
	pragmas_from_env();
#endif

#if ENABLE_FEATURE_MAKE_POSIX_2024
	if (!POSIX_2017) {
		path = argv[0];
		if (argv[0][0] != '/' && strchr(argv[0], '/')) {
			// Make relative path absolute
			path = newpath = realpath(argv[0], NULL);
			if (!path) {
				error("can't resolve path for %s: %s", argv[0], strerror(errno));
			}
		}
	} else {
		path = "make";
	}
#endif

	// Process options from MAKEFLAGS
	fargv = fargv0 = expand_makeflags(&fargc);
	if (fargv0) {
		opts = process_options(fargc, fargv, TRUE);
		fargv = fargv0 + optind;
		// Reset getopt(3) so we can call it again
		GETOPT_RESET();
	}

	// Process options from the command line
	opts |= process_options(argc, argv, FALSE);
	argv += optind;

	init_signal(SIGHUP);
	init_signal(SIGTERM);

	setmacro("$", "$", 0 | M_VALID);

#if ENABLE_FEATURE_MAKE_EXTENSIONS
	pragmas_to_env();

	// Process macro definitions from the command line
	if (!posix)
		process_macros(argv, 1);
	else
#endif
		// In POSIX mode macros must appear before targets.
		// argv should now point to targets only.
		argv = process_macros(argv, 1);

	// Process macro definitions from MAKEFLAGS
	if (fargv) {
		process_macros(fargv, 2);
		free(fargv0[1]);
		free(fargv0);
	}

	// Process macro definitions from the environment
	process_macros(environ, 3 | M_ENVIRON);

	// Update MAKEFLAGS and environment
	update_makeflags();

	// Read built-in rules
	input(NULL, 0);

	shell = get_shell();
	setmacro("SHELL", shell, 4);
	free((void *)shell);
	setmacro("MAKE", path, 4);
#if ENABLE_FEATURE_MAKE_POSIX_2024
	if (!POSIX_2017) {
		char *cwd = NULL;
		size_t len = 0;

		do {
			len += 256;
			cwd = xrealloc(cwd, len);
			if (getcwd(cwd, len)) {
				if (!useenv) {
					// Export cwd to environment, if necessary
					char *envcwd = getenv("CURDIR");
					if (envcwd && strcmp(cwd, envcwd) != 0)
						setenv("CURDIR", cwd, 1);
				}
				setmacro("CURDIR", cwd, 4);
				break;
			}
		} while (errno == ERANGE);
		free(cwd);
	}
	free((void *)newpath);
#endif

	fp = makefiles;
	if (!fp) {	// Look for a default Makefile
#if ENABLE_FEATURE_MAKE_EXTENSIONS
		if (!posix && (ifd = fopen("PDPmakefile", "r")) != NULL)
			makefile = "PDPmakefile";
		else
#endif
		if ((ifd = fopen("makefile", "r")) != NULL)
			makefile = "makefile";
		else if ((ifd = fopen("Makefile", "r")) != NULL)
			makefile = "Makefile";
		else
			error("no makefile found");
		goto read_makefile;
	}

	while (fp) {
		if (strcmp(fp->f_name, "-") == 0) {	// Can use stdin as makefile
			ifd = stdin;
			makefile = "stdin";
		} else {
			if ((ifd = fopen(fp->f_name, "r")) == NULL)
				error("can't open %s: %s", fp->f_name, strerror(errno));
			makefile = fp->f_name;
		}
		fp = fp->f_next;
 read_makefile:
		input(ifd, 0);
		fclose(ifd);
		makefile = NULL;
	}

	if (print)
		print_details();

	mark_special(".SILENT", OPT_s, N_SILENT);
	mark_special(".IGNORE", OPT_i, N_IGNORE);
	mark_special(".PRECIOUS", OPT_precious, N_PRECIOUS);
#if ENABLE_FEATURE_MAKE_POSIX_2024
	if (!POSIX_2017)
		mark_special(".PHONY", OPT_phony, N_PHONY);
#endif

#if ENABLE_FEATURE_MAKE_EXTENSIONS
	if (posix)
#endif
	{
		// In POSIX mode only targets should now be in argv.
		found_target = FALSE;
		for (char **a = argv; *a; a++) {
			if (!strchr(*a, '='))
				found_target = TRUE;
			else if (found_target)
				error("macro assignments must precede targets");
		}
	}

	estat = 0;
	found_target = FALSE;
	for (; *argv; argv++) {
#if ENABLE_FEATURE_MAKE_EXTENSIONS
		// Skip macro assignments.
		if (strchr(*argv, '='))
			continue;
#endif
		found_target = TRUE;
		estat |= make(newname(*argv), 0);
	}
	if (!found_target) {
		if (!firstname)
			error("no targets defined");
		estat = make(firstname, 0);
	}

#if ENABLE_FEATURE_CLEAN_UP
# if ENABLE_FEATURE_MAKE_POSIX_2024
	free((void *)numjobs);
# endif
	freenames();
	freemacros();
	freefiles(makefiles);
#endif

	return estat & MAKE_FAILURE;
}

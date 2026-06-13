static void
make_handler(int sig)
{
	signal(sig, SIG_DFL);
	remove_target();
	kill(getpid(), sig);
}

static void
init_signal(int sig)
{
	struct sigaction sa, new_action;

	sigemptyset(&new_action.sa_mask);
	new_action.sa_flags = 0;
	new_action.sa_handler = make_handler;

	sigaction(sig, NULL, &sa);
	if (sa.sa_handler != SIG_IGN)
		sigaction(sig, &new_action, NULL);
}

/*
 * If the global option flag associated with a special target hasn't
 * been set mark all prerequisites of the target with a flag.  If the
 * target had no prerequisites set the global option flag.
 */
static void
mark_special(const char *special, uint32_t oflag, uint16_t nflag)
{
	struct name *np;
	struct rule *rp;
	struct depend *dp;
	int marked = FALSE;

	if (!(opts & oflag) && (np = findname(special))) {
		for (rp = np->n_rule; rp; rp = rp->r_next) {
			for (dp = rp->r_dep; dp; dp = dp->d_next) {
				dp->d_name->n_flag |= nflag;
				marked = TRUE;
			}
		}

		if (!marked)
			opts |= oflag;
	}
}

static char *
get_shell(void)
{
	size_t len;
	char *path, *file;
	const char *end;
	struct stat st;

	len = confstr(_CS_PATH, NULL, (size_t)0);
	path = xmalloc(len);
	confstr(_CS_PATH, path, len);
	file = xmalloc(len + 3);

    for (const char *p = path; p; p = end) {
		end = strchr(p, ':');
		if (end == NULL)
			len = strlen(p);
		else
			len = end++ - p;

		memcpy(file, p, len);
		strcpy(file + len, "/sh");

		if (!access(file, X_OK) && !stat(file, &st) && S_ISREG(st.st_mode)) {
			free(path);
			return file;
		}
	}
	free(path);
	free(file);
	return xstrdup("/bin/sh");
}

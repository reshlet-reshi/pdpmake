/*
 * Control of the implicit suffix rules
 */
#include "make_m2.h"

/*
 * Return a pointer to the suffix of a name (which may be the
 * terminating NUL if there's no suffix).
 */
char *
suffix(const char *name)
{
	char *p = strrchr(name, '.');
	if (p)
		return p;
	return (char *)name + strlen(name);
}

/*
 * Find a name structure whose name is formed by concatenating two
 * strings.  If 'create' is TRUE the name is created if necessary.
 */
static struct name *
namecat(const char *s, const char *t, int create)
{
	char *p;
	struct name *np;

	p = xconcat3(s, t, "");
	if (create)
		np = newname(p);
	else
		np = findname(p);
	free(p);
	return np;
}

/*
 * Search for an inference rule to convert some suffix ('psuff')
 * to the target suffix 'tsuff'.  The basename of the prerequisite
 * is 'base'.
 */
struct name *
dyndep0(char *base, const char *tsuff, struct rule *infrule)
{
	char *psuff;
	struct name *xp;		// Suffixes
	struct name *sp;		// Suffix rule
	struct rule *rp;
	struct depend *dp;
#if ENABLE_FEATURE_MAKE_EXTENSIONS
	bool chain = FALSE;
#else
	const bool chain = FALSE;
#endif

	xp = newname(".SUFFIXES");
#if ENABLE_FEATURE_MAKE_EXTENSIONS
 retry:
#endif
	for (rp = xp->n_rule; rp; rp = rp->r_next) {
		for (dp = rp->r_dep; dp; dp = dp->d_next) {
			// Generate new suffix rule to try
			psuff = dp->d_name->n_name;
			sp = namecat(psuff, tsuff, FALSE);
			if (sp && sp->n_rule) {
				struct name *ip;
				int got_ip;

#if ENABLE_FEATURE_MAKE_EXTENSIONS
				// Has rule already been used in this chain?
				if ((sp->n_flag & N_MARK))
					continue;
#endif
				// Generate a name for an implicit prerequisite
				ip = namecat(base, psuff, TRUE);
				if ((ip->n_flag & N_DOING))
					continue;

				if (!ip->n_tim.tv_sec)
					modtime(ip);

				if (!chain) {
					got_ip = ip->n_tim.tv_sec || (ip->n_flag & N_TARGET);
				}
#if ENABLE_FEATURE_MAKE_EXTENSIONS
				else {
					sp->n_flag |= N_MARK;
					got_ip = dyndep(ip, NULL, NULL) != NULL;
					sp->n_flag &= ~N_MARK;
				}
#endif

				if (got_ip) {
					// Prerequisite exists or we know how to make it
					if (infrule) {
						infrule->r_dep = newdep(ip, NULL);
						infrule->r_cmd = sp->n_rule->r_cmd;
					}
					return ip;
				}
			}
		}
	}
#if ENABLE_FEATURE_MAKE_EXTENSIONS
	// If we didn't find an existing file or an explicit rule try
	// again, this time looking for a chained inference rule.
	if (!posix && !chain) {
		chain = TRUE;
		goto retry;
	}
#endif
	return NULL;
}

#if ENABLE_FEATURE_MAKE_EXTENSIONS
/*
 * If 'name' ends with 'suffix' return an allocated string containing
 * the name with the suffix removed, else return NULL.
 */
char *
has_suffix(const char *name, const char *suffix)
{
	ssize_t delta = strlen(name) - strlen(suffix);
	char *base = NULL;

	if (delta > 0 && strcmp(name + delta, suffix) == 0) {
		base = xstrdup(name);
		base[delta] = '\0';
	}

	return base;
}
#endif

/*
 * Dynamic dependency.  This routine applies the suffix rules
 * to try and find a source and a set of rules for a missing
 * target.  NULL is returned on failure.  On success the name of
 * the implicit prerequisite is returned and the rule used is
 * placed in the infrule structure provided by the caller.
 */
struct name *
dyndep(struct name *np, struct rule *infrule, const char **ptsuff)
{
	const char *tsuff;
	char *base, *base_suffix, *name, *member;
	struct name *pp = NULL;	// Implicit prerequisite

	member = NULL;
	name = splitlib(np->n_name, &member);

#if ENABLE_FEATURE_MAKE_EXTENSIONS
	// POSIX only allows inference rules with one or two periods.
	// As an extension this restriction is lifted, but not for
	// targets of the form lib.a(member.o).
	if (!posix && member == NULL) {
		struct name *xp = newname(".SUFFIXES");
		int found_suffix = FALSE;

		for (struct rule *rp = xp->n_rule; rp; rp = rp->r_next) {
			for (struct depend *dp = rp->r_dep; dp; dp = dp->d_next) {
				tsuff = dp->d_name->n_name;
				base = has_suffix(name, tsuff);
				if (base) {
					found_suffix = TRUE;
					pp = dyndep0(base, tsuff, infrule);
					free(base);
					if (pp) {
						goto done;
					}
				}
			}
		}

		if (!found_suffix) {
			// The name didn't have a known suffix. Try single-suffix rule.
			tsuff = "";
			pp = dyndep0(name, tsuff, infrule);
			if (pp) {
 done:
				if (ptsuff) {
					*ptsuff = tsuff;
				}
			}
		}
	} else
#endif
	{
		tsuff = xstrdup(suffix(name));
		if (member)
			base = member;
		else
			base = name;
		base_suffix = suffix(base);
		*base_suffix = '\0';

		pp = dyndep0(base, tsuff, infrule);
		free((void *)tsuff);
	}
	free(name);

	return pp;
}

static const char *rules[] = {
	".c.o:\n",
	"	$(CC) $(CFLAGS) -c $<\n",
	".y.o:\n",
	"	$(YACC) $(YFLAGS) $<\n",
	"	$(CC) $(CFLAGS) -c y.tab.c\n",
	"	rm -f y.tab.c\n",
	"	mv y.tab.o $@\n",
	".y.c:\n",
	"	$(YACC) $(YFLAGS) $<\n",
	"	mv y.tab.c $@\n",
	".l.o:\n",
	"	$(LEX) $(LFLAGS) $<\n",
	"	$(CC) $(CFLAGS) -c lex.yy.c\n",
	"	rm -f lex.yy.c\n",
	"	mv lex.yy.o $@\n",
	".l.c:\n",
	"	$(LEX) $(LFLAGS) $<\n",
	"	mv lex.yy.c $@\n",
	".c.a:\n",
	"	$(CC) -c $(CFLAGS) $<\n",
	"	$(AR) $(ARFLAGS) $@ $*.o\n",
	"	rm -f $*.o\n",
	".c:\n",
	"	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<\n",
	".sh:\n",
	"	cp $< $@\n",
	"	chmod a+x $@\n",
	NULL
};

static const char *rules_2017[] = {
	".SUFFIXES:.o .c .y .l .a .sh .f\n",
	".f.o:\n",
	"	$(FC) $(FFLAGS) -c $<\n",
	".f.a:\n",
	"	$(FC) -c $(FFLAGS) $<\n",
	"	$(AR) $(ARFLAGS) $@ $*.o\n",
	"	rm -f $*.o\n",
	".f:\n",
	"	$(FC) $(FFLAGS) $(LDFLAGS) -o $@ $<\n",
	NULL
};

static const char *rules_2024[] = {
	".SUFFIXES:.o .c .y .l .a .sh\n",
	NULL
};

static const char *macros[] = {
	"CFLAGS=-O1\n",
	"YACC=yacc\n",
	"YFLAGS=\n",
	"LEX=lex\n",
	"LFLAGS=\n",
	"AR=ar\n",
	"ARFLAGS=-rv\n",
	"LDFLAGS=\n",
	NULL
};

static const char *macros_2017[] = {
	"CC=c99\n",
	"FC=fort77\n",
	"FFLAGS=-O1\n",
	NULL
};

static const char *macros_2024[] = {
	"CC=c17\n",
	NULL
};

static const char *macros_ext[] = {
	"CC=cc\n",
	NULL
};

/*
 * Read the built-in rules using a fake fgets-like interface.
 */
#define TAIL_BLOCK_COUNT 3
char *
getrules(char *buf, int buf_sz)
{
	static const char **block = NULL;
	static int tail_block_idx = -1;
	static const char **tail_blocks[TAIL_BLOCK_COUNT] = { NULL, NULL, NULL };

	const char *line = NULL;
	char ch;
	char *cursor = buf;

	if (buf_sz < READLINE_CHUNK)
		error("internal error: built-in rule buffer too small");

	if (tail_block_idx == -1) {
		block = macros;

		tail_block_idx = 0;

#if ENABLE_FEATURE_MAKE_EXTENSIONS
		if (POSIX_2017)
			tail_blocks[0] = macros_2017;
		else if (posix)
			tail_blocks[0] = macros_2024;
		else
			tail_blocks[0] = macros_ext;
#elif ENABLE_FEATURE_MAKE_POSIX_2024
		tail_blocks[0] = macros_2024;
#else
		tail_blocks[0] = macros_2017;
#endif

		if (!norules) {
#if ENABLE_FEATURE_MAKE_EXTENSIONS
			if (POSIX_2017)
				tail_blocks[1] = rules_2017;
			else
				tail_blocks[1] = rules_2024;
#elif ENABLE_FEATURE_MAKE_POSIX_2024
			tail_blocks[1] = rules_2024;
#else
			tail_blocks[1] = rules_2017;
#endif

			tail_blocks[2] = rules;
		}
	}

	while (block == NULL || *block == NULL) {
		if (tail_block_idx >= TAIL_BLOCK_COUNT)
			return NULL;
		block = tail_blocks[tail_block_idx++];
	}

	line = *block++;

	while (--buf_sz) {
		ch = *line++;
		if (ch == '\0')
			error("internal error: unterminated built-in rule");
		*cursor++ = ch;
		if (ch == '\n') {
			*cursor = '\0';
			return buf;
		}
	}
	error("internal error: built-in rule line too long");
}

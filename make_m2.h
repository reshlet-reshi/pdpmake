/*
 * Minimal header for compiling check.c with M2-Mesoplanet.
 *
 * M2-Mesoplanet follows include directives before conditional
 * preprocessing, so the full host-oriented make.h is not yet usable.
 */
#ifndef PDPMAKE_MAKE_M2_H
#define PDPMAKE_MAKE_M2_H

typedef unsigned char bool;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

#ifndef ENABLE_FEATURE_MAKE_EXTENSIONS
#define ENABLE_FEATURE_MAKE_EXTENSIONS 1
#endif

#ifndef ENABLE_FEATURE_MAKE_POSIX_2024
#define ENABLE_FEATURE_MAKE_POSIX_2024 1
#endif

struct timespec {
	long tv_sec;
	long tv_nsec;
};

int printf(const char *format, ...);
int putchar(int c);

#if ENABLE_FEATURE_MAKE_EXTENSIONS
#define N_DOUBLE 0x10
#else
#define N_DOUBLE 0x00
#endif
#define HTABSIZE 199

struct name {
	struct name *n_next;
	char *n_name;
	struct rule *n_rule;
	struct timespec n_tim;
	uint16_t n_flag;
};

struct rule {
	struct rule *r_next;
	struct depend *r_dep;
	struct cmd *r_cmd;
};

struct depend {
	struct depend *d_next;
	struct name *d_name;
	int d_refcnt;
};

struct cmd {
	struct cmd *c_next;
	char *c_cmd;
	int c_refcnt;
	const char *c_makefile;
	int c_dispno;
};

struct macro {
	struct macro *m_next;
	char *m_name;
	char *m_val;
#if ENABLE_FEATURE_MAKE_EXTENSIONS || ENABLE_FEATURE_MAKE_POSIX_2024
	bool m_immediate;
#endif
	bool m_flag;
	uint8_t m_level;
};

extern struct name *namehead[HTABSIZE];
extern struct macro *macrohead[HTABSIZE];
extern struct name *firstname;

#endif

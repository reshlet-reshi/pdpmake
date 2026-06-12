/*
 * Minimal header for compiling selected files with M2-Mesoplanet.
 *
 * M2-Mesoplanet follows include directives before conditional
 * preprocessing, so the full host-oriented make.h is not yet usable.
 */
#ifndef PDPMAKE_MAKE_M2_H
#define PDPMAKE_MAKE_M2_H

typedef unsigned char bool;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

#define NULL 0
#define TRUE 1
#define FALSE 0

#define STD_POSIX_2017 0
#define STD_POSIX_2024 1

#ifndef ENABLE_FEATURE_MAKE_EXTENSIONS
#define ENABLE_FEATURE_MAKE_EXTENSIONS 1
#endif

#ifndef ENABLE_FEATURE_MAKE_POSIX_2024
#define ENABLE_FEATURE_MAKE_POSIX_2024 1
#endif

#ifndef ENABLE_FEATURE_CLEAN_UP
#define ENABLE_FEATURE_CLEAN_UP 0
#endif

#if ENABLE_FEATURE_MAKE_EXTENSIONS
#define POSIX_2017 (posix && posix_level == STD_POSIX_2017)
#elif ENABLE_FEATURE_MAKE_POSIX_2024
#define POSIX_2017 FALSE
#endif

struct timespec {
	long tv_sec;
	long tv_nsec;
};

int printf(const char *format, ...);
int putchar(int c);
void free(void *ptr);
int strcmp(const char *s1, const char *s2);
void *xmalloc(unsigned len);
char *xstrdup(const char *s);
unsigned int getbucket(const char *name);
void error(const char *msg, ...);

static int
pdpmake_islower(int c)
{
	return c >= 'a' && c <= 'z';
}

static int
pdpmake_isupper(int c)
{
	return c >= 'A' && c <= 'Z';
}

static int
pdpmake_isalpha(int c)
{
	return pdpmake_islower(c) || pdpmake_isupper(c);
}

static int
pdpmake_isdigit(int c)
{
	return c >= '0' && c <= '9';
}

static int
isblank(int c)
{
	return c == ' ' || c == '\t';
}

static int
iscntrl(int c)
{
	return c < ' ' || c == 127;
}

static int
ispname(int c)
{
	return pdpmake_isalpha(c) || pdpmake_isdigit(c) || c == '.' || c == '_';
}

static int
isfname(int c)
{
	return ispname(c) || c == '-';
}

#if ENABLE_FEATURE_MAKE_EXTENSIONS
#define N_DOUBLE 0x10
#else
#define N_DOUBLE 0x00
#endif
#define HTABSIZE 199

#define M_IMMEDIATE 0x08
#define M_VALID 0x10
#define M_ENVIRON 0x20

#define BIT_MACRO_NAME 0
#define P_MACRO_NAME 1

#if ENABLE_FEATURE_MAKE_EXTENSIONS
#define PDPMAKE_CHECK_MACRO_NAME posix
#else
#define PDPMAKE_CHECK_MACRO_NAME TRUE
#endif

#if ENABLE_FEATURE_MAKE_POSIX_2024
#define PDPMAKE_REJECT_BLANK_CNTRL TRUE
#else
#define PDPMAKE_REJECT_BLANK_CNTRL FALSE
#endif

#if ENABLE_FEATURE_MAKE_EXTENSIONS
#if ENABLE_FEATURE_MAKE_POSIX_2024
#define PDPMAKE_MACRO_USES_FNAME ((pragma & P_MACRO_NAME) || !POSIX_2017)
#else
#define PDPMAKE_MACRO_USES_FNAME (pragma & P_MACRO_NAME)
#endif
#else
#if ENABLE_FEATURE_MAKE_POSIX_2024
#define PDPMAKE_MACRO_USES_FNAME (!POSIX_2017)
#else
#define PDPMAKE_MACRO_USES_FNAME FALSE
#endif
#endif

#if ENABLE_FEATURE_MAKE_EXTENSIONS || ENABLE_FEATURE_MAKE_POSIX_2024
#define PDPMAKE_SET_IMMEDIATE(mp, value) (mp)->m_immediate = (value)
#else
#define PDPMAKE_SET_IMMEDIATE(mp, value)
#endif

#if ENABLE_FEATURE_MAKE_EXTENSIONS
#define PDPMAKE_ERROR_INVALID_MACRO(name) error_invalid_macro(name)
#else
#define PDPMAKE_ERROR_INVALID_MACRO(name) error("invalid macro name '%s'", name)
#endif

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
#if ENABLE_FEATURE_MAKE_EXTENSIONS || ENABLE_FEATURE_MAKE_POSIX_2024
	char m_padding[5];
#else
	char m_padding[6];
#endif
};

extern struct name *namehead[HTABSIZE];
extern struct macro *macrohead[HTABSIZE];
extern struct name *firstname;
extern bool posix;
extern unsigned char pragma;
extern unsigned char posix_level;

#endif

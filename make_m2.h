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
typedef unsigned int uint32_t;

#define NULL 0
#define TRUE 1
#define FALSE 0
#define INT_MAX 2147483647
#define ENOENT 2

#if !defined(__M2__)
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#else
typedef unsigned long size_t;
typedef long ssize_t;
typedef long time_t;

struct timespec {
	long tv_sec;
	long tv_nsec;
};

struct stat {
	struct timespec st_mtim;
};
#endif

#include <stdarg.h>

typedef struct __IO_FILE FILE;

#define ARMAG "!<arch>\n"
#define SARMAG 8
#define ARFMAG "`\n"
#define AR_NAME_LEN 16
#define AR_DATE_LEN 12
#define AR_UID_LEN 6
#define AR_GID_LEN 6
#define AR_MODE_LEN 8
#define AR_SIZE_LEN 10
#define AR_FMAG_LEN 2

#define SEEK_CUR 1

struct ar_hdr {
	char ar_name[AR_NAME_LEN];
	char ar_date[AR_DATE_LEN];
	char ar_uid[AR_UID_LEN];
	char ar_gid[AR_GID_LEN];
	char ar_mode[AR_MODE_LEN];
	char ar_size[AR_SIZE_LEN];
	char ar_fmag[AR_FMAG_LEN];
};

#define STD_POSIX_2017 0
#define STD_POSIX_2024 1
#define READLINE_CHUNK 256

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

int printf(const char *format, ...);
int putchar(int c);
void free(void *ptr);
int strcmp(const char *s1, const char *s2);
size_t strlen(const char *str);
char *strchr(const char *str, int ch);
char *strrchr(const char *str, int ch);
int memcmp(const void *s1, const void *s2, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
char *strerror(int errnum);
extern FILE *stdout;
extern FILE *stderr;
int fprintf(FILE *stream, const char *format, ...);
int vfprintf(FILE *stream, const char *format, va_list arg);
int fputc(int c, FILE *stream);
FILE *fopen(const char *path, const char *mode);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
int feof(FILE *stream);
int fseek(FILE *stream, long offset, int whence);
int fclose(FILE *stream);
void exit(int status);
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);
char *strndup(const char *s, size_t n);
#if defined(__M2__)
extern int errno;
int stat(const char *path, struct stat *buf);
#define PDPMAKE_ERRNO errno
#elif defined(__linux__)
int *__errno_location(void);
#define PDPMAKE_ERRNO (*__errno_location())
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__)
int *__error(void);
#define PDPMAKE_ERRNO (*__error())
#else
extern int errno;
#define PDPMAKE_ERRNO errno
#endif
void *xmalloc(size_t len);
void *xrealloc(void *ptr, size_t len);
char *xconcat3(const char *s1, const char *s2, const char *s3);
char *xstrdup(const char *s);
char *xstrndup(const char *s, size_t n);
char *xappendword(const char *str, const char *word);
unsigned int getbucket(const char *name);
void error(const char *msg, ...);
void diagnostic(const char *msg, ...);
void error_unexpected(const char *s);
void error_in_inference_rule(const char *s);
void error_not_allowed(const char *s, const char *t);
void warning(const char *msg, ...);

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

#define isdigit(c) pdpmake_isdigit(c)

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
#define N_DOING 0x01
#define N_TARGET 0x04
#if ENABLE_FEATURE_MAKE_EXTENSIONS || ENABLE_FEATURE_MAKE_POSIX_2024
#define N_MARK 0x100
#else
#define N_MARK 0x00
#endif
#define HTABSIZE 199

#if ENABLE_FEATURE_MAKE_EXTENSIONS
#define OPT_r (1 << 7)
#elif ENABLE_FEATURE_MAKE_POSIX_2024
#define OPT_r (1 << 6)
#else
#define OPT_r (1 << 5)
#endif
#define norules (opts & OPT_r)

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

struct file {
	struct file *f_next;
	char *f_name;
};

extern struct name *namehead[HTABSIZE];
extern struct macro *macrohead[HTABSIZE];
extern struct name *firstname;
extern uint32_t opts;
extern const char *myname;
extern const char *makefile;
extern int dispno;
extern struct cmd *curr_cmd;
extern bool posix;
extern unsigned char pragma;
extern unsigned char posix_level;

#if !ENABLE_FEATURE_MAKE_EXTENSIONS
#define dyndep(n, i, p) dyndep(n, i)
#endif

struct file *newfile(char *str, struct file *fphead);
void freefiles(struct file *fp);
char *splitlib(const char *name, char **member);
void modtime(struct name *np);
char *suffix(const char *name);
char *has_suffix(const char *name, const char *suffix);
struct name *dyndep(struct name *np, struct rule *infrule, const char **ptsuff);
struct name *findname(const char *name);
struct name *newname(const char *name);
struct depend *newdep(struct name *np, struct depend *dp);
char *getrules(char *buf, int buf_sz);

#endif

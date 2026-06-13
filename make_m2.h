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
#define EXIT_FAILURE 1
#define INT_MAX 2147483647
#define ENOENT 2
#define ERANGE 34
#define O_RDWR 2
#define O_CREAT 64
#define AT_FDCWD -100
#define UTIME_NOW 1073741823
#define CLOCK_REALTIME 0
#ifndef SIGHUP
#define SIGHUP 1
#endif
#ifndef SIGTERM
#define SIGTERM 15
#endif

#if !defined(__M2__)
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif
#include <stddef.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#else
typedef unsigned long size_t;
typedef long ssize_t;
typedef long time_t;
typedef unsigned long sigset_t;
typedef void (*pdpmake_sighandler_t)(int);

#define SIG_DFL ((pdpmake_sighandler_t)0)
#define SIG_IGN ((pdpmake_sighandler_t)1)

struct timespec {
	long tv_sec;
	long tv_nsec;
};

struct stat {
	long st_mode;
	struct timespec st_mtim;
};

struct sigaction {
	pdpmake_sighandler_t sa_handler;
	sigset_t sa_mask;
	int sa_flags;
	pdpmake_sighandler_t sa_restorer;
};

#endif

#include <stdarg.h>

typedef struct {
	size_t gl_pathc;
	char **gl_pathv;
	size_t gl_offs;
	int gl_flags;
	void *gl_closedir;
	void *gl_readdir;
	void *gl_opendir;
	void *gl_lstat;
	void *gl_stat;
} glob_t;

#define GLOB_NOSORT 4
#define GLOB_NOMATCH 3

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
#ifndef X_OK
#define X_OK 1
#endif

#ifndef _CS_PATH
#define _CS_PATH 0
#endif

#ifndef S_IFMT
#define S_IFMT 00170000
#endif

#ifndef S_IFREG
#define S_IFREG 0100000
#endif

#ifndef S_ISREG
#define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
#endif

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
#ifndef DEFAULT_POSIX_LEVEL
#define DEFAULT_POSIX_LEVEL STD_POSIX_2024
#endif
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
#else
#define POSIX_2017 FALSE
#endif

int printf(const char *format, ...);
int putchar(int c);
int puts(const char *s);
int fflush(FILE *stream);
void free(void *ptr);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
size_t strlen(const char *str);
char *strcpy(char *dest, const char *src);
char *stpcpy(char *dest, const char *src);
char *strchr(const char *str, int ch);
char *strrchr(const char *str, int ch);
size_t strspn(const char *s, const char *accept);
int memcmp(const void *s1, const void *s2, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
char *strerror(int errnum);
extern FILE *stdout;
extern FILE *stderr;
extern FILE *stdin;
int fprintf(FILE *stream, const char *format, ...);
int vfprintf(FILE *stream, const char *format, va_list arg);
int fputc(int c, FILE *stream);
FILE *fopen(const char *path, const char *mode);
char *fgets(char *s, int size, FILE *stream);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
int feof(FILE *stream);
int fseek(FILE *stream, long offset, int whence);
int fclose(FILE *stream);
FILE *popen(const char *command, const char *mode);
int pclose(FILE *stream);
int unlink(const char *path);
int access(const char *path, int mode);
int open(const char *path, int oflag, ...);
int close(int fd);
int kill(int pid, int sig);
int getpid(void);
int utimensat(int fd, const char *path, const struct timespec times[2], int flag);
int clock_gettime(int clk_id, struct timespec *tp);
size_t confstr(int name, char *buf, size_t len);
char *getenv(const char *name);
int chdir(const char *path);
char *getcwd(char *buf, size_t size);
char *realpath(const char *path, char *resolved_path);
char *basename(char *path);
int getopt(int argc, char **argv, const char *optstring);
extern char *optarg;
extern int optind;
extern char **environ;
#if defined(__M2__)
pdpmake_sighandler_t signal(int sig, pdpmake_sighandler_t func);
int sigemptyset(sigset_t *set);
int sigaction(int sig, const struct sigaction *act, struct sigaction *oldact);
#endif
int system(const char *command);
void exit(int status);
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);
char *strndup(const char *s, size_t n);
int glob(const char *pattern, int flags, void *errfunc, glob_t *pglob);
void globfree(glob_t *pglob);
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

#define isalpha(c) pdpmake_isalpha(c)

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
isspace(int c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\r'
			|| c == '\f' || c == '\v';
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

static int
pdpmake_wifexited(int status)
{
	return (status & 127) == 0;
}

static int
pdpmake_wexitstatus(int status)
{
	return (status >> 8) & 255;
}

static int
pdpmake_wifsignaled(int status)
{
	return (status & 127) != 0 && (status & 127) != 127;
}

static int
pdpmake_wtermsig(int status)
{
	return status & 127;
}

#if ENABLE_FEATURE_MAKE_EXTENSIONS
#define N_DOUBLE 0x10
#else
#define N_DOUBLE 0x00
#endif
#define N_DOING 0x01
#define N_DONE 0x02
#define N_TARGET 0x04
#define N_PRECIOUS 0x08
#define N_SILENT 0x20
#define N_IGNORE 0x40
#define N_SPECIAL 0x80
#if ENABLE_FEATURE_MAKE_EXTENSIONS || ENABLE_FEATURE_MAKE_POSIX_2024
#define N_MARK 0x100
#else
#define N_MARK 0x00
#endif
#if ENABLE_FEATURE_MAKE_POSIX_2024
#define N_PHONY 0x200
#else
#define N_PHONY 0x00
#endif
#define N_INFERENCE 0x400
#define HTABSIZE 199

#define T_NORMAL 0
#define T_SPECIAL 0x01
#define T_INFERENCE 0x02
#define T_NOPREREQ 0x04
#define T_COMMAND 0x08
#define T_SPECIAL_NOPREREQ 0x05
#define T_SPECIAL_NOPREREQ_COMMAND 0x0d

#if ENABLE_FEATURE_MAKE_EXTENSIONS
#define OPTSTR1 "+ehij:knqrsSt"
#define OPT_e (1 << 0)
#define OPT_h (1 << 1)
#define OPT_i (1 << 2)
#if ENABLE_FEATURE_MAKE_POSIX_2024
#define OPT_j (1 << 3)
#define OPT_k (1 << 4)
#define OPT_n (1 << 5)
#define OPT_q (1 << 6)
#define OPT_r (1 << 7)
#define OPT_s (1 << 8)
#define OPT_S (1 << 9)
#define OPT_t (1 << 10)
#define OPT_p (1 << 11)
#define OPT_f (1 << 12)
#define OPT_C (1 << 13)
#define OPT_x (1 << 14)
#define OPT_precious (1 << 15)
#define OPT_phony (1 << 16)
#define OPT_include (1 << 17)
#define OPT_make (1 << 18)
#else
#define OPT_j 0
#define OPT_k (1 << 3)
#define OPT_n (1 << 4)
#define OPT_q (1 << 5)
#define OPT_r (1 << 6)
#define OPT_s (1 << 7)
#define OPT_S (1 << 8)
#define OPT_t (1 << 9)
#define OPT_p (1 << 10)
#define OPT_f (1 << 11)
#define OPT_C (1 << 12)
#define OPT_x (1 << 13)
#define OPT_precious (1 << 14)
#define OPT_phony 0
#define OPT_include 0
#define OPT_make 0
#endif
#elif ENABLE_FEATURE_MAKE_POSIX_2024
#define OPTSTR1 "+eij:knqrsSt"
#define OPT_e (1 << 0)
#define OPT_h 0
#define OPT_i (1 << 1)
#define OPT_j (1 << 2)
#define OPT_k (1 << 3)
#define OPT_n (1 << 4)
#define OPT_q (1 << 5)
#define OPT_r (1 << 6)
#define OPT_s (1 << 7)
#define OPT_S (1 << 8)
#define OPT_t (1 << 9)
#define OPT_p (1 << 10)
#define OPT_f (1 << 11)
#define OPT_C 0
#define OPT_x 0
#define OPT_precious (1 << 12)
#define OPT_phony (1 << 13)
#define OPT_include (1 << 14)
#define OPT_make (1 << 15)
#else
#define OPTSTR1 "+eiknqrsSt"
#define OPT_e (1 << 0)
#define OPT_h 0
#define OPT_i (1 << 1)
#define OPT_j 0
#define OPT_k (1 << 2)
#define OPT_n (1 << 3)
#define OPT_q (1 << 4)
#define OPT_r (1 << 5)
#define OPT_s (1 << 6)
#define OPT_S (1 << 7)
#define OPT_t (1 << 8)
#define OPT_p (1 << 9)
#define OPT_f (1 << 10)
#define OPT_C 0
#define OPT_x 0
#define OPT_precious (1 << 11)
#define OPT_phony 0
#define OPT_include 0
#define OPT_make 0
#endif

#if ENABLE_FEATURE_MAKE_EXTENSIONS
#define OPTSTR2 "pf:C:x:"
#define OPTSTR "+ehij:knqrsStpf:C:x:"
#elif ENABLE_FEATURE_MAKE_POSIX_2024
#define OPTSTR2 "pf:"
#define OPTSTR "+eij:knqrsStpf:"
#else
#define OPTSTR2 "pf:"
#define OPTSTR "+eiknqrsStpf:"
#endif

#if defined(__GLIBC__) && ENABLE_FEATURE_MAKE_EXTENSIONS
#define OPT_OFFSET + !posix
#else
#define OPT_OFFSET
#endif

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__)
extern int optreset;
#endif

static void
getopt_reset(void)
{
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__)
	optind = 1;
	optreset = 1;
#elif defined(__sun__)
	optind = 1;
#else
	optind = 0;
#endif
}

#define OPT_MASK (~OPT_S)

#define useenv (opts & OPT_e)
#define ignore (opts & OPT_i)
#define errcont (opts & OPT_k)
#define dryrun (opts & OPT_n)
#define print (opts & OPT_p)
#define quest (opts & OPT_q)
#define norules (opts & OPT_r)
#define silent (opts & OPT_s)
#define dotouch (opts & OPT_t)
#define precious (opts & OPT_precious)
#define doinclude (opts & OPT_include)
#define domake (opts & OPT_make)

#define MAKE_FAILURE 0x01
#define MAKE_DIDSOMETHING 0x02

#define M_IMMEDIATE 0x08
#define M_VALID 0x10
#define M_ENVIRON 0x20

#define BIT_MACRO_NAME 0
#define BIT_TARGET_NAME 1
#define BIT_COMMAND_COMMENT 2
#define BIT_EMPTY_SUFFIX 3
#if defined(__CYGWIN__)
#define BIT_WINDOWS 4
#define BIT_POSIX_2017 5
#define BIT_POSIX_2024 6
#define BIT_POSIX_202X 7
#define P_WINDOWS (1 << BIT_WINDOWS)
#else
#define BIT_POSIX_2017 4
#define BIT_POSIX_2024 5
#define BIT_POSIX_202X 6
#endif

#define P_MACRO_NAME (1 << BIT_MACRO_NAME)
#define P_TARGET_NAME (1 << BIT_TARGET_NAME)
#define P_COMMAND_COMMENT (1 << BIT_COMMAND_COMMENT)
#define P_EMPTY_SUFFIX (1 << BIT_EMPTY_SUFFIX)

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
extern struct name *target;
extern struct file *makefiles;
extern uint32_t opts;
extern const char *myname;
extern const char *makefile;
extern int lineno;
extern int dispno;
extern struct cmd *curr_cmd;
extern char *numjobs;
extern bool posix;
extern unsigned char pragma;
extern unsigned char posix_level;
extern uint8_t clevel;

#if !ENABLE_FEATURE_MAKE_EXTENSIONS
#define dyndep(n, i, p) dyndep(n, i)
#endif

#if !ENABLE_FEATURE_MAKE_POSIX_2024
#define expand_macros(s, e) expand_macros(s)
#endif

struct file *newfile(char *str, struct file *fphead);
void freefiles(struct file *fp);
char *splitlib(const char *name, char **member);
void modtime(struct name *np);
char *suffix(const char *name);
char *has_suffix(const char *name, const char *suffix);
int target_type(char *s);
struct name *dyndep(struct name *np, struct rule *infrule, const char **ptsuff);
struct name *findname(const char *name);
struct name *newname(const char *name);
struct depend *newdep(struct name *np, struct depend *dp);
char *getrules(char *buf, int buf_sz);
struct cmd *newcmd(char *str, struct cmd *cphead);
void freecmds(struct cmd *cp);
void freedeps(struct depend *dp);
void freerules(struct rule *rp);
struct cmd *getcmd(struct name *np);
void addrule(struct name *np, struct depend *dp, struct cmd *cp, int flag);
void set_pragma(const char *name);
void pragmas_to_env(void);
int is_valid_target(const char *name);
int setenv(const char *name, const char *value, int overwrite);
void remove_target(void);
int make(struct name *np, int level);
char *expand_macros(const char *str, int except_dollar);
char *gettok(char **ptr);
char *skip_macro(const char *s);
char *find_char(const char *str, int c);
char *find_colon(char *p);
struct macro *getmp(const char *name);
void setmacro(const char *name, const char *val, int level);
const char *is_suffix(const char *s);
int ends_with_bracket(const char *s);
void process_line(char *s);
int skip_line(const char *str1);
void pragmas_from_env(void);
uint32_t process_options(int argc, char **argv, int from_env);
char **expand_makeflags(int *fargc);
char **process_macros(char **argv, int level);
void update_makeflags(void);
void init_signal(int sig);
char *get_shell(void);
void input(FILE *ifd, int builtin);
char *process_command(char *s);
char *run_command(const char *cmd);
void print_details(void);
void mark_special(const char *special, uint32_t oflag, uint16_t nflag);
void freenames(void);
void freemacros(void);

#endif

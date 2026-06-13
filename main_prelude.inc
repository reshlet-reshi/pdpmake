/*
 * main_prelude.inc: prologue and global state.
 */

/*
 * make [--posix] [-C path] [-f makefile] [-j num] [-x pragma]
 *      [-ehiknpqrsSt] [macro[:[:[:]]]=val ...] [target ...]
 *
 *  --posix  Enforce POSIX mode (non-POSIX)
 *  -C  Change directory to path (non-POSIX)
 *  -f  Makefile name
 *  -j  Number of jobs to run in parallel (not implemented)
 *  -x  Pragma to make POSIX mode less strict (non-POSIX)
 *  -e  Environment variables override macros in makefiles
 *  -h  Display help information (non-POSIX)
 *  -i  Ignore exit status
 *  -k  Continue on error
 *  -n  Pretend to make
 *  -p  Print all macros & targets
 *  -q  Question up-to-dateness of target.  Return exit status 1 if not
 *  -r  Don't use inbuilt rules
 *  -s  Make silently
 *  -S  Stop on error
 *  -t  Touch files instead of making them
 */
#include "make.h"

uint32_t opts;
const char *myname;
const char *makefile;
struct file *makefiles;
struct cmd *curr_cmd;
#if ENABLE_FEATURE_MAKE_POSIX_2024
char *numjobs = NULL;
#endif
#if ENABLE_FEATURE_MAKE_EXTENSIONS
bool posix;
bool seen_first;
unsigned char pragma = 0;
unsigned char posix_level = DEFAULT_POSIX_LEVEL;
#endif


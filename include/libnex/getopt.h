#ifndef GETOPT_LIBNEX_H
#define GETOPT_LIBNEX_H

#ifdef IN_LIBNEX
#include <libnex_config.h>
#else
#include <libnex/libnex_config.h>
#endif
#include <libnex/decls.h>

/* include files needed by this include file */

/* macros defined by this include file */
#define no_argument       0
#define required_argument 1
#define OPTIONAL_ARG      2

/* types defined by this include file */

/* GETOPT_LONG_OPTION_T: The type of long option */
typedef struct GETOPT_LONG_OPTION_T
{
    const char* name; /* the name of the long option */
    int has_arg;      /* one of the above macros */
    int* flag;        /* determines if getopt_long() returns a
                       * value for a long option; if it is
                       * non-NULL, 0 is returned as a function
                       * value and the value of val is stored in
                       * the area pointed to by flag.  Otherwise,
                       * val is returned. */
    int val;          /* determines the value to return if flag is
                       * NULL. */
} GETOPT_LONG_OPTION_T;

typedef GETOPT_LONG_OPTION_T option;

__DECL_START

/* externally-defined variables */
extern char* optarg;
extern int optind;
extern int opterr;
extern int optopt;

/* function prototypes */
#ifndef _AIX
PUBLIC int _libnex_getopt (int argc, char** argv, char* optstring);
#endif
PUBLIC int _libnex_getopt_long (int argc,
                                char** argv,
                                const char* shortopts,
                                const GETOPT_LONG_OPTION_T* longopts,
                                int* longind);

PUBLIC int _libnex_getopt_long_only (int argc,
                                     char** argv,
                                     const char* shortopts,
                                     const GETOPT_LONG_OPTION_T* longopts,
                                     int* longind);

__DECL_END

// A hack so we can use these functions even if the host provides getopt(3)
#undef getopt
#define getopt _libnex_getopt
#undef getopt_long
#define getopt_long _libnex_getopt_long
#undef getopt_long_only
#define getopt_long_only

#endif /* GETOPT_H */

/* END OF FILE getopt.h */

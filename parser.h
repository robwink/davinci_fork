#ifndef PARSER_H
#define PARSER_H

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <stddef.h>
#include <float.h>


#include <unistd.h>

#ifndef _WIN32
#include <sys/socket.h>
#include <sys/un.h>
#endif

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>


#include "system.h"

// NOTE(rswinkle) this is only used 3 times, twice in ff_ix.c
// and once in tools.c.  Is it really necessary?  Also it precludes
// checking for malloc failure...
#define memdup(p, l) memcpy(malloc(l), (p), l)

#include "parser_types.h"

// could ifdef this based on architecture
#define extract_int extract_i32
//#define extract_int extract_i64


#define HBUFSIZE 8192
#define PATH_SEP ' '

typedef double (*dfunc)(double);
typedef double (*ddfunc)(double, double);


#define YYSTYPE Vptr
#define YYDEBUG 1

#include "globals.h"

#if defined(_WIN32) && !defined(__MINGW32__)
#define readline w_readline
#define add_history w_add_history
#define history_get_history_state w_history_get_history_state
#endif

#include "ufunc.h"
#include "scope.h"
#include "func.h"

#endif /* PARSER_H */

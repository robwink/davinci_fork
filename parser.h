#ifndef PARSER_H
#define PARSER_H

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#ifndef _WIN32
#include <sys/socket.h>
#include <sys/un.h>
#endif
#include <sys/time.h>
#include <sys/types.h>

#ifdef HAVE_VALUES_H
#include <values.h>
#endif

#include <float.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>

#include "system.h"


// NOTE(rswinkle) this is only used 3 times, twice in ff_ix.c
// and once in tools.c.  Is it really necessary?  Also it precludes
// checking for malloc failure...
#define memdup(p, l) memcpy(malloc(l), (p), l)

#include "parser_types.h"


#define HBUFSIZE 8192
#define PATH_SEP ' '
#define RECORD_SUFFIX '#'


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


#endif /* PARSER_H */

/********************************** system.h **********************************/

#include "config.h"

/**
 ** This file contains all system specific dependencies, and defines
 ** some values that aren't on all systems
 **/
#ifndef _WIN32
#include <netinet/in.h>
#endif

#ifndef MINSHORT
#define MINSHORT    ((short)(1 << ((8*sizeof(short) - 1))))
#endif

#ifndef MININT
#define MININT    ((int)(1 << ((8*sizeof(int) - 1))))
#endif

#ifndef max
#define max(a,b) ((a) < (b) ? (b) : (a))
#define min(a,b) ((a) > (b) ? (b) : (a))
#endif

#ifndef u_char
#define u_char unsigned char
#endif

#ifdef NEED_UDEFS
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char uchar;
#endif /* NEED_UDEFS */


#ifndef HAVE_STRDUP
char *strdup(const char *);
#endif

void *my_realloc(void *, int);

/********************************** system.h **********************************/

/**
 ** This file contains all system specific dependencies, and defines
 ** some values that aren't on all systems
 **/

#include <netinet/in.h>

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


char *strdup(const char *);
void *my_realloc(void *, int);

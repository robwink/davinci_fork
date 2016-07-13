/********************************** system.h **********************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

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

//for some reason configure on mingw64 doesn't detect
//strndup even though the compiler is complaining about having it
#ifndef HAVE_STRNDUP
char *strndup(const char *s1, size_t len);
#endif

#ifndef HAVE_BASENAME
char *basename(const char *);
#endif

#ifndef HAVE_MMAP
#define PROT_READ   0x1
#define PROT_WRITE  0x2
#define MAP_SHARED  0x1
#define MAP_PRIVATE 0x2 /* unsupported */

#define MAP_FAILED (void*)-1

// for size_t
#include <stdlib.h>

void *mmap(void *x0, size_t len, int x1, int x2, int fd, size_t off);
void munmap(void *buf,int len);
#endif

void *my_realloc(void *, int);
void rmrf(const char *path);

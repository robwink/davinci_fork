/*
** File: configwin.h
**
** This file should be used instead of config.h when compiling
** on the Windows platform.
*/

#ifndef _CONFIGWIN_H_
#define _CONFIGWIN_H_

/*
** Makes uint, ufloat, ushort type definitions available. Use
** only when these are not already built in.
*/
#define NEED_UDEFS


/*
** Announce that this application is a Windows Console App.
*/

#define _CONSOLE 


/*
** Undefine HAVE_STRTOUL if strtoul is available int the headers.
*/

#define HAVE_STRTOUL 


/*
** Define to build module support into davinci. When undefined,
** ff_modules.c may still be compiled. See Makefile.win.
*/

/* #define BUILD_MODULE_SUPPORT  */
#undef BUILD_MODULE_SUPPORT


/*
** Various different macros meaning that the code is being compiled 
** for Microsoft Windows.
*/

#ifndef _WIN32
#define _WIN32
#endif

#ifndef MSDOS
#define MSDOS
#endif

#ifndef _MSDOS
#define _MSDOS
#endif

#ifndef _MSDOS_
#define _MSDOS_
#endif

#ifndef __MSDOS__
#define __MSDOS__
#endif /* __MSDOS__ */


/*
** Defined for regex.[ch]. Basically means that we have "string.h"
** available in our include path.
*/

#define HAVE_STRING_H 1

/* The following macro is defined for big-endian machines */
#undef WORDS_BIGENDIAN

/* The following macro is defined if we have libMagick */
#undef HAVE_LIBMAGICK

/* The following macro is defined if we have libreadline */
#define HAVE_LIBREADLINE 1

/* The following macro is defined if we have libhdf5 */
#define HAVE_LIBHDF5 1

/* The following macro is defined if we have zlib */
#define HAVE_LIBZLIB 1

/*Define if you have the PlPlot library */
/* #define HAVE_PLPLOT 1 */
#undef HAVE_PLPLOT

#endif /* _CONFIGWIN_H_ */

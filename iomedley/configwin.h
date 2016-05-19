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


#ifndef _WIN32
#define _WIN32
#endif /* _WIN32 */

#ifndef MSDOS
#define MSDOS
#endif /* MSDOS */

#ifndef _MSDOS
#define _MSDOS
#endif /* _MSDOS */

#ifndef _MSDOS_
#define _MSDOS_
#endif /* _MSDOS_ */

#ifndef __MSDOS__
#define __MSDOS__
#endif /* __MSDOS__ */


/*
** Defined for regex.[ch]. Basically means that we have "string.h"
** available in our include path.
*/

#define HAVE_STRING_H

/* The following macro is defined for big-endian machines */
#undef WORDS_BIGENDIAN

/* #define HAVE_LIBHDF5 1 */

#endif /* _CONFIGWIN_H_ */

/* src/H5config.h.  Generated automatically by configure.  */
/* src/H5config.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define if your struct tm has tm_zone.  */
/* #undef HAVE_TM_ZONE */

/* Define if you don't have tm_zone but do have the external array
   tzname.  */
#define HAVE_TZNAME 1

/* Define as __inline if that's what the C compiler calls it.  */
/* #undef inline */

/* Define to `long' if <sys/types.h> doesn't define.  */
/* #undef off_t */

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define if your <sys/time.h> declares struct tm.  */
/* #undef TM_IN_SYS_TIME */

/* Define if your processor stores words with the most significant
   byte first (like Motorola and SPARC, unlike Intel and VAX).  */
#define WORDS_BIGENDIAN 1

/* Define if the __attribute__(()) extension is present */
#define HAVE_ATTRIBUTE 1

/* Define if the compiler understands the __FUNCTION__ keyword. */
#define HAVE_FUNCTION 1

/* Define if we have parallel support */
/* #undef HAVE_PARALLEL */

/* Define if it's safe to use `long long' for hsize_t and hssize_t */
/*#define HAVE_LARGE_HSIZET 1 */
#undef HAVE_LARGE_HSIZET

/* Width for printf() for type `long long' or `__int64', us. `ll' */
/*#define PRINTF_LL_WIDTH "ll" */
#undef PRINTF_LL_WIDTH

/* Define if `tm_gmtoff' is a member of `struct tm' */
/* #undef HAVE_TM_GMTOFF */

/* Define if `__tm_gmtoff' is a member of `struct tm' */
/* #undef HAVE___TM_GMTOFF */

/* Define if `timezone' is a global variable */
#define HAVE_TIMEZONE 1

/* Define if `struct timezone' is defined */
#define HAVE_STRUCT_TIMEZONE 1

/* Define if `struct stat' has the `st_blocks' field */
#define HAVE_STAT_ST_BLOCKS 1

/* Define if `struct text_info' is defined */
/* #undef HAVE_STRUCT_TEXT_INFO */

/* Define if `struct videoconfig' is defined */
/* #undef HAVE_STRUCT_VIDEOCONFIG */

/* Define if the ioctl TIOCGETD is defined */
/* #undef HAVE_TIOCGETD */

/* Define if the ioctl TIOCGWINSZ is defined */
/* #undef HAVE_TIOCGWINSZ */

/* The number of bytes in a __int64.  */
#define SIZEOF___INT64 0

/* The number of bytes in a char.  */
#define SIZEOF_CHAR 1

/* The number of bytes in a double.  */
#define SIZEOF_DOUBLE 8

/* The number of bytes in a float.  */
#define SIZEOF_FLOAT 4

/* The number of bytes in a int.  */
#define SIZEOF_INT 4

/* The number of bytes in a int16_t.  */
#define SIZEOF_INT16_T 2

/* The number of bytes in a int32_t.  */
#define SIZEOF_INT32_T 4

/* The number of bytes in a int64_t.  */
#define SIZEOF_INT64_T 8

/* The number of bytes in a int8_t.  */
#define SIZEOF_INT8_T 1

/* The number of bytes in a int_fast16_t.  */
#define SIZEOF_INT_FAST16_T 0

/* The number of bytes in a int_fast32_t.  */
#define SIZEOF_INT_FAST32_T 0

/* The number of bytes in a int_fast64_t.  */
#define SIZEOF_INT_FAST64_T 0

/* The number of bytes in a int_fast8_t.  */
#define SIZEOF_INT_FAST8_T 0

/* The number of bytes in a int_least16_t.  */
#define SIZEOF_INT_LEAST16_T 2

/* The number of bytes in a int_least32_t.  */
#define SIZEOF_INT_LEAST32_T 4

/* The number of bytes in a int_least64_t.  */
#define SIZEOF_INT_LEAST64_T 8

/* The number of bytes in a int_least8_t.  */
#define SIZEOF_INT_LEAST8_T 1

/* The number of bytes in a long.  */
#define SIZEOF_LONG 4

/* The number of bytes in a long double.  */
#define SIZEOF_LONG_DOUBLE 16

/* The number of bytes in a long long.  */
#define SIZEOF_LONG_LONG 8

/* The number of bytes in a off_t.  */
#define SIZEOF_OFF_T 4

/* The number of bytes in a short.  */
#define SIZEOF_SHORT 2

/* The number of bytes in a size_t.  */
#define SIZEOF_SIZE_T 4

/* The number of bytes in a uint16_t.  */
#define SIZEOF_UINT16_T 2

/* The number of bytes in a uint32_t.  */
#define SIZEOF_UINT32_T 4

/* The number of bytes in a uint64_t.  */
#define SIZEOF_UINT64_T 8

/* The number of bytes in a uint8_t.  */
#define SIZEOF_UINT8_T 1

/* The number of bytes in a uint_fast16_t.  */
#define SIZEOF_UINT_FAST16_T 0

/* The number of bytes in a uint_fast32_t.  */
#define SIZEOF_UINT_FAST32_T 0

/* The number of bytes in a uint_fast64_t.  */
#define SIZEOF_UINT_FAST64_T 0

/* The number of bytes in a uint_fast8_t.  */
#define SIZEOF_UINT_FAST8_T 0

/* The number of bytes in a uint_least16_t.  */
#define SIZEOF_UINT_LEAST16_T 2

/* The number of bytes in a uint_least32_t.  */
#define SIZEOF_UINT_LEAST32_T 4

/* The number of bytes in a uint_least64_t.  */
#define SIZEOF_UINT_LEAST64_T 8

/* The number of bytes in a uint_least8_t.  */
#define SIZEOF_UINT_LEAST8_T 1

/* Define if you have the BSDgettimeofday function.  */
/* #undef HAVE_BSDGETTIMEOFDAY */

/* Define if you have the GetConsoleScreenBufferInfo function.  */
/* #undef HAVE_GETCONSOLESCREENBUFFERINFO */

/* Define if you have the _getvideoconfig function.  */
/* #undef HAVE__GETVIDEOCONFIG */

/* Define if you have the _scrsize function.  */
/* #undef HAVE__SCRSIZE */

/* Define if you have the compress2 function.  */
/* #undef HAVE_COMPRESS2 */

/* Define if you have the difftime function.  */
/* #undef HAVE_DIFFTIME */

/* Define if you have the fork function.  */
/*#define HAVE_FORK 1 */
#undef HAVE_FORK

/* Define if you have the fseek64 function.  */
/* #undef HAVE_FSEEK64 */

/* Define if you have the gethostname function.  */
/* #define HAVE_GETHOSTNAME 1 */
#undef HAVE_GETHOSTNAME

/* Define if you have the getpwuid function.  */
/*#define HAVE_GETPWUID 1 */
#undef HAVE_GETPWUID

/* Define if you have the getrusage function.  */
/*#define HAVE_GETRUSAGE 1 */
#undef HAVE_GETRUSAGE

/* Define if you have the gettextinfo function.  */
/* #undef HAVE_GETTEXTINFO */

/* Define if you have the gettimeofday function.  */
/* #undef HAVE_GETTIMEOFDAY */

/* Define if you have the ioctl function.  */
#define HAVE_IOCTL 1

/* Define if you have the longjmp function.  */
#define HAVE_LONGJMP 1

/* Define if you have the lseek64 function.  */
/*#define HAVE_LSEEK64 1 */
#undef HAVE_LSEEK64

/* Define if you have the setsysinfo function.  */
/* #undef HAVE_SETSYSINFO */

/* Define if you have the sigaction function.  */
/*#define HAVE_SIGACTION 1*/
#undef HAVE_SIGACTION

/* Define if you have the signal function.  */
/*#define HAVE_SIGNAL 1*/
#undef HAVE_SIGNAL

/* Define if you have the snprintf function.  */
/*#define HAVE_SNPRINTF 1*/
#undef HAVE_SNPRINTF

/* Define if you have the system function.  */
#define HAVE_SYSTEM 1

/* Define if you have the vsnprintf function.  */
/*#define HAVE_VSNPRINTF 1 */
#undef HAVE_VSNPRINTF

/* Define if you have the waitpid function.  */
/*#define HAVE_WAITPID 1*/
#undef HAVE_WAITPID

/* Define if you have the <io.h> header file.  */
/* #undef HAVE_IO_H */

/* Define if you have the <mfhdf.h> header file.  */
/* #undef HAVE_MFHDF_H */

/* Define if you have the <setjmp.h> header file.  */
#define HAVE_SETJMP_H 1

/* Define if you have the <stddef.h> header file.  */
#define HAVE_STDDEF_H 1

/* Define if you have the <stdint.h> header file.  */
/* #undef HAVE_STDINT_H */

/* Define if you have the <sys/ioctl.h> header file.  */
/*#define HAVE_SYS_IOCTL_H 1*/
#undef HAVE_SYS_IOCTL_H

/* Define if you have the <sys/proc.h> header file.  */
#undef HAVE_SYS_PROC_H 

/* Define if you have the <sys/resource.h> header file.  */
/*#define HAVE_SYS_RESOURCE_H 1*/
#undef HAVE_SYS_RESOURCE_H

/* Define if you have the <sys/stat.h> header file.  */
#define HAVE_SYS_STAT_H 1

/* Define if you have the <sys/sysinfo.h> header file.  */
/* #undef HAVE_SYS_SYSINFO_H */

/* Define if you have the <sys/time.h> header file.  */
/*#define HAVE_SYS_TIME_H 1*/
#undef HAVE_SYS_TIME_H

/* Define if you have the <sys/timeb.h> header file.  */
#define HAVE_SYS_TIMEB_H 1

/* Define if you have the <unistd.h> header file.  */
/*#define HAVE_UNISTD_H 1*/
#undef HAVE_UNISTD_H

/* Define if you have the <winsock.h> header file.  */
/* #undef HAVE_WINSOCK_H */
#define HAVE_WINSOCK_H

/* Define if you have the <zlib.h> header file.  */
/*#define HAVE_ZLIB_H 1*/
#undef HAVE_ZLIB_H

/* Define if you have the coug library (-lcoug).  */
#undef HAVE_LIBCOUG 

/* Define if you have the df library (-ldf).  */
/* #undef HAVE_LIBDF */

/* Define if you have the jpeg library (-ljpeg).  */
/*#define HAVE_LIBJPEG 1*/

/* Define if you have the m library (-lm).  */
/*#define HAVE_LIBM 1*/

/* Define if you have the mfhdf library (-lmfhdf).  */
/* #undef HAVE_LIBMFHDF */

/* Define if you have the mpich library (-lmpich).  */
/* #undef HAVE_LIBMPICH */

/* Define if you have the nsl library (-lnsl).  */
/*#define HAVE_LIBNSL 1*/

/* Define if you have the z library (-lz).  */
/*#define HAVE_LIBZ 1*/

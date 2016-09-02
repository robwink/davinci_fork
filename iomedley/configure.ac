dnl Process this file with autoconf to produce a configure script.
AC_INIT(iomedley.h)
dnl AM_CONFIG_HEADER(iom_config.h)
AC_CONFIG_HEADER(iom_config.h)

dnl AC_CONFIG_AUX_DIR(aux)
AC_CANONICAL_HOST
AC_CANONICAL_SYSTEM

dnl AM_INIT_AUTOMAKE(iomedley, 1.0)
dnl PACKAGE(iomedley)
dnl VERSION(1.0)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_CPP
AC_PROG_MAKE_SET
AC_PROG_RANLIB

dnl Initialize libtool and tell it to generate static libraries
dnl AM_DISABLE_SHARED
dnl AM_PROG_LIBTOOL

AC_AIX

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_OFF_T
AC_STRUCT_TM

dnl Check for endian
AC_C_BIGENDIAN

dnl Enable Position Independent Code (often required by 64-bit systems)
export CFLAGS="${CFLAGS} -fPIC"

dnl Set some defaults
dnl test "$prefix" = NONE && prefix=/usr/local
dnl test "$exec_prefix" = NONE && exec_prefix=${prefix}

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h)
AC_CHECK_HEADERS(math.h string.h strings.h)

dnl Check for X
dnl AC_PATH_XTRA
dnl CPPFLAGS="$CPPFLAGS $X_CFLAGS"
dnl LIBS="$LIBS $X_PRE_LIBS $X_LIBS $X_EXTRA_LIBS"
dnl AC_CHECK_LIB(X11, XOpenDisplay)
dnl AC_CHECK_LIB(Xt, XtToolkitInitialize)
dnl AC_CHECK_LIB(Xext, XShmPutImage)

AC_CHECK_LIB(z, compress, [found_libz=yes], [found_libz=no])

dnl Warn the user if zlib not found
if test "$found_libz" = "yes"; then
	AC_DEFINE(HAVE_LIBZ, 1, [ define to 1 if you have libz ] )
	AC_DEFINE(HAVE_LIBPNG, 1, [ define to 1 if you have libpng ] )
	libpng="libpng16.a"
	libpngdir="libpng-1.6.23"
	save_CFLAGS="$CFLAGS" save_LIBS="$LIBS" save_LDFLAGS="$LDFLAGS"
	CFLAGS="$CFLAGS -I$libpngdir" LIBS="$LIBS -lpng16" LDFLAGS="$LDFLAGS -L$libpngdir/.libs"
else
	echo "**********************************************************************"
	echo "*                                                                    *"
	echo "*     Warning: zlib not found - PNG support will not be included     *"
	echo "*                                                                    *"
	echo "**********************************************************************"
fi

dnl Checks for library functions.
AC_FUNC_MMAP
AC_FUNC_MEMCMP
AC_CHECK_FUNCS(strdup strerror strspn strstr)
AC_CHECK_FUNCS(strcasecmp strncasecmp)
AC_FUNC_ALLOCA

dnl Get a list of libiomedley.la source objects
dnl Xsed="sed -e s/^X// "
dnl LTLIBOBJS=`echo X"$LIBOBJS" | $Xsed -e "s,\.[^.]* ,.lo ,g;s,\.[^.]*\$,.lo,"`

dnl Substitute the following additional variables
AC_SUBST(libpng)
AC_SUBST(libpngdir)
AC_SUBST(CFLAGS)
AC_SUBST(LDFLAGS)
dnl AC_SUBST(LTLIBOBJS)
dnl AC_SUBST(LTALLOCA)

AC_CONFIG_SUBDIRS(libjpeg libgif tiff-4.0.6)
if test "$found_libz" = "yes"; then
	AC_CONFIG_SUBDIRS(libpng-1.6.23)
fi

AC_OUTPUT(Makefile)

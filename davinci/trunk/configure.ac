dnl Process this file with autoconf to produce a configure script.
AC_INIT(davinci, 1.17)
.mAC_CONFIG_AUX_DIR(config)

dnl Do some AIX-specific pre-config.
AC_AIX

dnl Check for system type.
AC_CANONICAL_TARGET

dnl Setup automake.
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE
AM_CONFIG_HEADER(config.h)

dnl Check for some required programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
dnl AC_PROG_YACC
dnl AM_PROG_LEX
AC_PROG_INSTALL
dnl AC_PROG_RANLIB
AC_PROG_MAKE_SET

dnl FIX: need AC_LIBTOOL_DLOPEN?
AC_LIBTOOL_DLOPEN
AC_LIBLTDL_CONVENIENCE
AC_PROG_LIBTOOL

dnl Check endianness.
AC_C_BIGENDIAN

dnl Handle user-controllable options.

dnl Enable/disable loadable modules (default = yes).
AC_ARG_ENABLE(modules,
	AC_HELP_STRING([--disable-modules], [remove loadable module support.]))
AM_CONDITIONAL(MODULE_SUPPORT, test "x$enable_modules" != "xno")

dnl iomedley (default = yes).
AC_ARG_ENABLE([iomedley],
	AC_HELP_STRING([--disable-iomedley], [remove iomedley support.]))

dnl VICAR widget (default = yes).
AC_ARG_ENABLE([vicar],
	AC_HELP_STRING([--disable-vicar], [remove VICAR widget support.]))

dnl Add X paths.
AC_PATH_XTRA
CFLAGS="$CFLAGS $X_CFLAGS"
LIBS="$LIBS $X_PRE_LIBS $X_LIBS $X_EXTRA_LIBS"

dnl Check for X libraries.
AC_CHECK_LIB(X11, XOpenDisplay)
AC_CHECK_LIB(Xt, XtToolkitInitialize)
AC_CHECK_LIB(Xm, XmCreateForm)
AC_CHECK_LIB(Xmu, XmuClientWindow)

dnl Check for miscellaneous libraries.
AC_CHECK_LIB(m, sqrt,, [echo "ERROR: unable to locate libm"; exit 1])
AC_CHECK_LIB(z, deflate)
AC_CHECK_LIB(hdf5, H5open)
AC_CHECK_LIB(usds, Themis_Entry)
AC_CHECK_LIB(msss_vis, read_DCT)
AC_CHECK_LIB(termcap, tgetent)
AC_CHECK_LIB(ltdl, lt_dlopen)
AC_CHECK_LIB(png, png_check_version)
AC_CHECK_LIB(readline, rl_callback_handler_install,, [libreadline_found=no])
dnl AC_CHECK_LIB(readline, rl_callback_handler_install)

dnl We're including this, so it'll always be there.
dnl This define is to satisfy some ifdefs in io_loadmod.c.
AC_DEFINE(HAVE_LIBLTDL, 1, [ltdl library present])

dnl Check for ImageMagic.
AC_CHECK_LIB(Magick, GetImageInfo, [libmagick_found=yes])
AM_CONDITIONAL(HAVE_LIBMAGICK, test "x$libmagick_found" = "xyes")

dnl This is a C++ library; set the language so the test succeeds.
dnl FIX: not sure why we need both lib and header check..

AC_LANG_PUSH(C++)
AC_CHECK_LIB(qmv, qmv_version)
AC_CHECK_HEADERS(qmv/hvector.h)
AC_LANG_POP

dnl Check for headers.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h limits.h malloc.h sys/file.h sys/time.h unistd.h)
AC_CHECK_HEADERS(values.h)
AC_CHECK_HEADERS(limits.h)
AC_CHECK_HEADERS(libgen.h)
AC_CHECK_HEADERS(Xm/Xrt3d.h)

dnl Check for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME
AC_STRUCT_TM

dnl Check for library functions.
AC_FUNC_MEMCMP
AC_TYPE_SIGNAL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(mkdir putenv select strdup strndup strspn strstr strtod strtol strtoul)
AC_CHECK_FUNCS(index rindex getdtablesize getwd usleep strtoul basename)
AC_CHECK_FUNCS(dirname bcopy bcmp bzero)

dnl Check for plplot library.
dnl FIX: why two distinct defines here?
dnl FIX: does the plplot lib have to be pre-pended to $LIBS?
dnl FIX: disabling all of this for now.

dnl PL_LIB=""
dnl AC_CHECK_LIB(plplotFX, c_plinit, LIBS="$LIBS -lplplotFX")
dnl if test "$PL_LIB" != ""
dnl then
dnl 	AC_DEFINE(INCLUDE_API, 1, [plplot library is present])
dnl 	AC_DEFINE(HAVE_PLPLOT, 1, [plplot library is present])
dnl fi

dnl Check for and configure local readline if not found yet.

if test "x$libreadline_found" = "xno"; then

	AC_MSG_CHECKING(if local libreadline present)

	if test -x readline/configure; then
		AC_MSG_RESULT(yes)
		CFLAGS="$CFLAGS -Ireadline"
		AC_DEFINE(HAVE_LIBREADLINE, 1)
		AC_CONFIG_SUBDIRS(readline)
		MY_SUBDIRS="$MY_SUBDIRS readline"
		local_readline=yes
		libreadline_found=yes
	else
		AC_MSG_RESULT(no)
	fi

fi
AM_CONDITIONAL(LOCAL_READLINE, test "x$local_readline" = "xyes")

dnl Configure iomedley.

if test "x$enable_iomedley" != "xno"; then

	AC_MSG_CHECKING(if local libiomedley present)
	if test -x iomedley/configure; then
		AC_MSG_RESULT(yes)
		CFLAGS="$CFLAGS -Iiomedley"
		AC_DEFINE(HAVE_LIBIOMEDLEY, 1, [iomedley library is present])
		AC_CONFIG_SUBDIRS(iomedley)
		MY_SUBDIRS="$MY_SUBDIRS iomedley"
		local_iomedley=yes
	else
		AC_MSG_RESULT(no)
	fi

fi
AM_CONDITIONAL(LOCAL_IOMEDLEY, test x$local_iomedley = xyes)

dnl Configure VICAR widget.

if test "x$enable_vicar" != "xno"; then

	AC_MSG_CHECKING(if local libvicar present)
	if test -d vicar; then
		AC_MSG_RESULT(yes)
		dnl This should only be added to modules/Makefile.am's CFLAGS, but I'm sick of autoconf.
		dnl Not sure this is working either; doesn't look like $top_builddir gets set..
		CFLAGS="$CFLAGS -I$top_builddir/vicar"
		AC_DEFINE(HAVE_LIBVICAR, 1, [VICAR widget library is present])
		MY_SUBDIRS="$MY_SUBDIRS vicar"
		local_vicar=yes
	else
		AC_MSG_RESULT(no)
	fi

fi
AM_CONDITIONAL(LOCAL_VICAR, test x$local_vicar = xyes)

dnl Add modules subdir.

if test "x$enable_modules" != "xno"; then
	AC_DEFINE(BUILD_MODULE_SUPPORT, 1, [loadable module support])
	MY_SUBDIRS="$MY_SUBDIRS modules"
fi

dnl FIX: this stuff can go away if libtool is used.

AC_SUBST(MY_SUBDIRS)

dnl For libtool support.
AC_CONFIG_SUBDIRS(libltdl)

AC_OUTPUT([Makefile modules/Makefile vicar/Makefile])

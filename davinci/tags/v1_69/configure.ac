dnl Process this file with autoconf to produce a configure script.
AC_INIT(davinci, 1.69)

## libdavinci version
LIB_CURRENT=0
LIB_REVISION=0
LIB_AGE=0
AC_SUBST(LIB_CURRENT)
AC_SUBST(LIB_REVISION)
AC_SUBST(LIB_AGE)


AC_CONFIG_AUX_DIR(config)

dnl Do some AIX-specific pre-config.
AC_AIX

dnl Check for system type.
AC_CANONICAL_TARGET

dnl Setup automake.
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE
AM_CONFIG_HEADER(config.h)


#Default prefix
#AC_PREFIX_DEFAULT(/usr/local)

dnl Check for some required programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
dnl AC_PROG_YACC
dnl AM_PROG_LEX
AC_PROG_INSTALL
dnl AC_PROG_RANLIB
AC_PROG_MAKE_SET


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

dnl libisis for ISIS cube writer (default = yes)
AC_ARG_ENABLE([libisis],
    AC_HELP_STRING([--disable-libisis],
                   [disable use of libisis and hence ISIS cube writer.]))


dnl Aqua build for OSX/dawrin
case $host in
	*-*-darwin*)
		AC_ARG_ENABLE([aqua],
			AC_HELP_STRING([--enable-aqua], [add AQUA widget support.]),
			enable_aqua=yes, enable_aqua=no)
	;;
esac

case $host in
	*-*-darwin*)
		if test "x$enable_aqua" != "xno" ; then 
			echo "Configuring with aqua"
			AC_DEFINE(HAVE_AQUA, 1, [ Use aqua instead of X11 ])
		else
			echo "Configuring without aqua"
		fi
	;;
esac




dnl Add X paths.
AC_PATH_XTRA
CFLAGS="$CFLAGS $X_CFLAGS"
LIBS="$X_PRE_LIBS $X_LIBS $X_EXTRA_LIBS $LIBS"

dnl Check for Xt and X libraries
dnl    We don't really care if any of these are found, we just want to know
dnl    if they're NOT found
AC_CHECK_LIB(X11, XOpenDisplay)
AC_CHECK_LIB(Xt, XtToolkitInitialize)
AC_CHECK_HEADERS(X11/Intrinsic.h)


# Ask user for path to motif stuff:
AC_ARG_WITH(motif,
    [  --with-motif=<path>     prefix of motif installation. e.g. /usr/local or /usr],
    [LDFLAGS="$LDFLAGS -L$withval/lib" ; CFLAGS="$CFLAGS -I$withval/include"]
)

if test "x$ac_cv_lib_X11_XOpenDisplay" = "xyes" -a \
        "x$ac_cv_lib_Xt_XtToolkitInitialize" = "xyes" -a \
        "x$ac_cv_header_X11_Intrinsic_h" = "xyes"; then
	have_xt="yes"
	AC_DEFINE(HAVE_XT, 1, [ headers and libraries for X11 and Xt ])
	AC_CHECK_HEADERS(Xm/Xm.h)
	AC_CHECK_LIB(Xm, XmCreateForm)
	have_motif="no"
	if test "x$ac_cv_header_Xm_Xm_h" = "xyes" -a \
			"x$ac_cv_lib_Xm_XmCreateForm" = "xyes"; then
		AC_DEFINE(HAVE_MOTIF, 1, [if all motif parts are present])
		have_motif="yes";
	fi
fi
AM_CONDITIONAL(HAVE_MOTIF, [test "x$have_motif" = "xyes"])

dnl Check for miscellaneous libraries.
AC_CHECK_LIB(m, sqrt,, [echo "ERROR: unable to locate libm"; exit 1])
AC_CHECK_LIB(usds, Themis_Entry)
AC_CHECK_LIB(msss_vis, read_DCT)
AC_CHECK_LIB(termcap, tgetent)
AC_CHECK_LIB(ltdl, lt_dlopen)
have_libltdlc=yes

# Used for MINGW
AC_CHECK_LIB(wsock32, main, [have_libwsock32=yes], [have_libwsock32=no])
AM_CONDITIONAL(HAVE_LIBWSOCK32, [test "x$have_libwsock32" = "xyes"])

# Used for MINGW
AC_CHECK_LIB(regex, main, [have_libregex=yes],[have_libregex=no])
AM_CONDITIONAL(HAVE_LIBREGEX, [test "x$have_libregex" = "xyes"])

AC_CHECK_LIB(z, deflate)
AC_CHECK_LIB(png, png_read_info)

#check for cfitsio
AC_ARG_WITH(cfitsio,
    [  --with-cfitsio=<path>      prefix of cfitsio installation. e.g. /usr/local or /usr],
    [LDFLAGS="$LDFLAGS -L$withval/lib" ; CFLAGS="$CFLAGS -I$withval/include"]
)
AC_CHECK_LIB(cfitsio, ffopen,[have_cfitsio=yes],[have_cfitsio=no])

# Ask user for path to hdf5 stuff:.
have_libhdf=yes
AC_ARG_WITH(hdf5,
    [  --with-hdf5=<path>      prefix of hdf5 installation. e.g. /usr/local or /usr],
    [LDFLAGS="$LDFLAGS -L$withval/lib -lz" ; CFLAGS="$CFLAGS -I$withval/include"]
)
dnl with the path the specified, see if we can find libhdf.
## For MINGW since wsock32 is needed for hdf5
if test "x$have_libwsock32" = "xyes"; then
   AC_CHECK_LIB(hdf5, H5open,,,[-lwsock32])
else
   AC_CHECK_LIB(hdf5, H5open)
fi
AC_CHECK_LIB(hdf5, H5open,[have_libhdf=yes],[have_libhdf=no])

#if test "x$ac_cv_lib_cfitsio_ffopen" = "xyes"; then
#	have_cfitsio="yes"
#fi


dnl Check for ImageMagic.
AC_CHECK_LIB(Magick, GetImageInfo, [have_libmagick=yes],[have_libmagick=no])
if test "x$have_libmagic" = "xyes"; then
	AC_DEFINE(HAVE_LIBMAGICK, 1, [ if you have libmagick] )
fi
AM_CONDITIONAL(HAVE_LIBMAGICK, [test "x$have_libmagick" = "xyes"])

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
AC_CHECK_FUNCS(mkdir putenv select strdup strndup strspn strstr strtod strtol strtoul drand48)
AC_CHECK_FUNCS(index rindex getdtablesize getwd usleep strtoul basename)
AC_CHECK_FUNCS(dirname bcopy bcmp bzero)

dnl Get path to libisis+includes from system env if ISISR is defined
if test "x$ISISR" != "x"; then
	isis_runtime="$ISISR"
fi

dnl If the user specified a path to the enable-libisis, use that
if test "x$enable_libisis" != "xno" -a "x$enable_libisis" != "xyes" -a "x$enable_libisis" != "x"; then
	isis_runtime=$enable_libisis
fi

dnl If libisis is supposed to be used, then check for its existance
if test "x$enable_libisis" != "xno"; then

	dnl Add library path to libisis
	CPPFLAGS="$CPPFLAGS -I$isis_runtime/inc"
	LDFLAGS="$LDFLAGS -L$isis_runtime/lib"

	dnl Check for existance of libisis
	have_libisis=yes
	AC_CHECK_LIB(isis, q_open,
				 [LIBS="$LIBS -lisis -lisisbind -lisisgen -lg2c"],
				 [have_libisis=no],
				 [-lisisbind -lisisgen -lg2c])
	AC_CHECK_HEADERS(q.h, [/bin/true], [have_libisis=no])

	if test "x$have_libisis" = "xyes"; then
		AC_DEFINE(HAVE_LIBISIS, 1, [ headers and libraries for libisis ])
	fi
fi

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

AC_CHECK_LIB(readline, rl_callback_handler_install)
AC_CHECK_LIB(readline, rl_callback_handler_install, [have_readline=yes],[have_readline=no])
dnl AC_CHECK_LIB(readline, rl_callback_handler_install)
AC_CHECK_LIB(readline, history_get_history_state, [need_libhistory=no],[need_libhistory=yes])

if test "x$have_readline" = "xno" -o "x$need_libhistory" = "xyes"; then
	AC_MSG_CHECKING(if local libreadline present)
	if test -x readline/configure; then
		AC_MSG_RESULT(yes)
		CFLAGS="$CFLAGS -Ireadline"
		AC_DEFINE(HAVE_LIBREADLINE, 1, [have readline library])
		AC_CONFIG_SUBDIRS(readline)
		MY_SUBDIRS="$MY_SUBDIRS readline"
		local_readline=yes
		have_readline=yes
		dnl this looks a little fishy, but seems to work.
		dnl this is causing modules to compile against ./readline, which
		dnl doesn't exist.  At a minimum, this needs something like a ${local}
		dnl prefix, but modules don't need this at all, so can we remove 
		dnl it instead?
		if test "x$need_libhistory" = "xyes"; then
			LIBS="$LIBS -L\$(top_srcdir)/readline -lhistory"
		fi
	else
		AC_MSG_RESULT(no)
	fi
fi

AM_CONDITIONAL(LOCAL_READLINE, test "x$local_readline" = "xyes")

dnl Configure iomedley.

if test "x$enable_iomedley" != "xno"; then
	CFLAGS="$CFLAGS -I\$(top_srcdir)/iomedley"
	AC_DEFINE(HAVE_LIBIOMEDLEY, 1, [iomedley library is present])
	AC_CONFIG_SUBDIRS(iomedley)
	MY_SUBDIRS="$MY_SUBDIRS iomedley"
	local_iomedley=yes
fi
AM_CONDITIONAL(LOCAL_IOMEDLEY, test x$local_iomedley = xyes)


dnl Add module support.
if test "x$enable_modules" != "xno"; then
	AC_CONFIG_SUBDIRS(libltdl)
	AC_LIBTOOL_DLOPEN
    AC_LIBLTDL_CONVENIENCE
    AC_PROG_LIBTOOL
	CFLAGS="$CFLAGS $LTDLINCL"
	AC_DEFINE(BUILD_MODULE_SUPPORT, 1, [loadable module support])
	build_module_support=yes
	MY_SUBDIRS="$MY_SUBDIRS modules libltdl modules/thm"

	dnl We're including this, so it'll always be there.
	dnl This define is to satisfy some ifdefs in io_loadmod.c.
	AC_DEFINE(HAVE_LIBLTDL, 1, [ltdl library present])

	dnl Configure VICAR widget.
	if test "x$enable_vicar" != "xno"; then
		if test "x$have_motif" = "xyes"; then
			AC_CHECK_LIB(Xmu, XmuClientWindow)
			dnl This should only be added to modules/Makefile.am's CFLAGS, but I'm sick of autoconf.
			dnl Not sure this is working either; doesn't look like $ac_top_builddir gets set..
			CFLAGS="$CFLAGS -I\$(top_srcdir)/vicar"
			AC_DEFINE(HAVE_LIBVICAR, 1, [VICAR widget library is present])
			MY_SUBDIRS="$MY_SUBDIRS vicar modules/gui"
			local_vicar=yes
		else
			AC_MSG_RESULT(no)
		fi
	fi
fi

AM_CONDITIONAL(LOCAL_VICAR, [test "x$have_motif" = "xyes" -a "x$local_vicar" = "xyes"])


dnl Need to handle gnuplot and display defaults

dnl AC_ARG_VAR(DV_VIEWER, [default image display program])
dnl AC_ARG_VAR(GPLOT_CMD, [default gnuplot program])
dnl AC_ARG_VAR(DVHELP,    [default path for helpfile])
dnl AC_ARG_VAR(EDITOR,    [default text editor])

AC_ARG_WITH(viewer, 
    [  --with-viewer=<path>    Path to image display program (default: xv)],
    [DV_VIEWER=$withval],
    [AC_PATH_PROG(DV_VIEWER, [xv])]
)
AC_ARG_WITH(gplot, 
    [  --with-gnuplot=<path>   Path to gnuplot program (default: gnuplot) ],
    [GPLOT_CMD=$withval],
	[AC_PATH_PROG(GPLOT_CMD, [gnuplot])]
)
AC_ARG_WITH(editor, 
    [  --with-editor=<path>    Path to text file editor (default: vim)],
    [EDITOR=$withval],
	[AC_PATH_PROGS(EDITOR,[vim vi emacs edit])]
)
AC_ARG_WITH(help, 
    [  --with-help=<path>      Path to davinci help file (default: /usr/local/share/davinci/docs/dv.gih)],
    [DVHELP=$withval],
    [DVHELP="/usr/local/share/davinci/docs/dv.gih"]
)

AC_ARG_WITH(modpath, 
    [  --with-modpath=<path>   Path to davinci loadable modules (if enabled) (default: /usr/local/lib/davinci)],
    [DV_MOD_PATH=$withval],
    [DV_MOD_PATH="/usr/local/lib/davinci"]
)

AC_ARG_WITH(library, 
	[  --without-library       Don't install Davinci library scripts],
	[DV_LIBRARY="no"],
	[AC_CHECK_FILES(library/utils.dvrc, [DV_LIBRARY="yes"],[DV_LIBRARY="no"])]
)
AM_CONDITIONAL(DV_LIBRARY, [test "x$DV_LIBRARY" != "xno"])

AC_ARG_WITH(examples, 
	[  --without-examples      Don't install Davinci examples],
	[DV_EXAMPLES="no"],
	[AC_CHECK_FILES(examples/day_maps_final.vicar,[DV_EXAMPLES="yes"],[DV_EXAMPLES="no"])]
)
AM_CONDITIONAL(DV_EXAMPLES, [test "x$DV_EXAMPLES" != "xno"])







if test "$DV_VIEWER" != "" ; then
AC_DEFINE_UNQUOTED(DV_VIEWER, "$DV_VIEWER", Path to image display program for display() function)
fi

if test "$GPLOT_CMD" != "" ; then
AC_DEFINE_UNQUOTED(GPLOT_CMD, "$GPLOT_CMD",  Path to gnuplot program for plot() function)
fi

if test "$EDITOR" != "" ; then
AC_DEFINE_UNQUOTED(EDITOR, "$EDITOR", Path to text file editor for edit() function)
fi

AC_DEFINE_UNQUOTED(DVHELP, "$DVHELP", Path to davinci help file)
AC_DEFINE_UNQUOTED(DV_MOD_PATH, "$DV_MOD_PATH", Path to davinci loadable modules (if enabled))
AC_DEFINE_UNQUOTED(DV_EXAMPLES, "$DV_EXAMPLES", Path to davinci examples)
AC_DEFINE_UNQUOTED(DV_LIBRARY, "$DV_LIBRARY", Path to davinci library scripts)



dnl FIX: this stuff can go away if libtool is used.

AC_SUBST(MY_SUBDIRS)

AC_OUTPUT([Makefile modules/Makefile modules/kjn/Makefile modules/cse/Makefile modules/thm/Makefile modules/gui/Makefile vicar/Makefile])

PRINT_YES_NO() {
  answer="Yes"
  if test $# -lt 2; then
    answer="No"
  else
      if test "$2" != "yes"; then
        answer="No"
      fi
  fi
  echo "$1:  ${answer}"
  echo "printf(\"$1:  ${answer}\n\");" >> build_summary
}

PRINT() {
	echo "$1"
	echo "printf(\"$1\n\");" >> build_summary
}

rm -f build_summary

PRINT ""
PRINT        "Configuration Summary"
PRINT        "----------------------"
PRINT_YES_NO "        Readline" "$have_readline"
PRINT_YES_NO "          X11/Xt" "$have_xt"
PRINT_YES_NO "           Motif" "$have_motif"
PRINT_YES_NO "            HDF5" "$have_libhdf"
PRINT_YES_NO "GUI/Vicar Module" "$local_vicar"
PRINT_YES_NO "         libisis" "$have_libisis"
PRINT_YES_NO "            FITS" "$have_cfitsio"

PRINT ""

PRINT_YES_NO "  Module Support" "$build_module_support"
if test "x$build_module_support" = "xyes"; then
PRINT        "Modules Lib Path:  $DV_MOD_PATH"
fi

PRINT        " Display program:  $DV_VIEWER"
PRINT        "Plotting program:  $GPLOT_CMD"
PRINT        "     Text editor:  $EDITOR"
PRINT        "   Helpfile path:  $DVHELP"
PRINT_YES_NO " Davinci library"  "$DV_LIBRARY"
PRINT_YES_NO "Davinci examples"  "$DV_EXAMPLES"
PRINT ""

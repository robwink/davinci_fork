/* acconfig.h
   This file is in the public domain.

   Descriptive text for the C preprocessor macros that
   the distributed Autoconf macros can define.
   No software package will use all of them; autoheader copies the ones
   your configure.in uses into your configuration header file templates.

   The entries are in sort -df order: alphabetical, case insensitive,
   ignoring punctuation (such as underscores).  Although this order
   can split up related entries, it makes it easier to check whether
   a given entry is in the file.

   Leave the following blank line there!!  Autoheader needs it.  */


/*Define if you have module library*/
#undef BUILD_MODULE_SUPPORT

/*Define if you have projection library libproj*/
#undef HAVE_LIBPROJ

/*Define if you have Xm library*/
#undef HAVE_LIBXM

/*Define if you have the PlPlot library */
#undef HAVE_PLPLOT

/*Define if you have plplotFX library*/
#undef INCLUDE_API

/*Define if you have use hpux shl library*/
#undef USE_HPUX_SHL

/*Define if you have xrt3d library*/
#undef XRT_ENABLED

/*Define if you have readline library */
#undef HAVE_LIBREADLINE



/* Leave that blank line there!!  Autoheader needs it.
   If you're adding to this file, keep in mind:
   The entries are in sort -df order: alphabetical, case insensitive,
   ignoring punctuation (such as underscores).  */

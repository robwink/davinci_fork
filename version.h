char *version = "@(#) daVinci Version #0.55a";

/*
Version 0.55a:
	* modified ufunc loader
	+ added delete()


Version 0.55:
	* fixed eval again.  I think you can return values now
	+ global()
	+ dump()
	+ structures: { }, [], add_struct(), get_struct()
	+ HDF read/write
	* Added new object: text_array; multiple lines of text and support functions:
	- array subsetting
	- line by line grep search
	- line by line substring substitution
 	- basename and dirname substring substition
	- read/write functionality

Version 0.54a:
	* Interim delivery for themis

Version 0.54
    * Added support code and resourse files for a Win32/Console version of davinci
  	-Support for reading Big-Endian files correctly
	-Use of a win32 compliant readline lib (readline.dll)
	-Use of a win32 compliant hdf5 lib (hdf5.lib)
	-Supports most file format input/output (as big endian) at this time

Version 0.53
    * Added the Pl-Plot library with an Xt interface (api.h apifunc.c apidef.h)
    * Added ff_plplot file which congolerates several 
			plotting commands into a simple davinci command
    * Added an interactive cursor command (for use with a plplot window)
	which returns normalize coords
	+ added line continuation  "\"
	+ added C-style (multiline) comments 
	* fixed line-number reporting in ufuncs.  It's a hack.
	+ added eval(), but it can't return a value
	* Found some missing NULL problems in various options declarations.

Version 0.52a
    * Added stereo projection function
    * Added ImageMagick library for 40+ graphics file types (R/W) including Mpeg
    * Updated configure.in and Makefile.in

Version 0.52:
    * modified grammar to streamline to ansi standard.  Side effects:
    +    you can subset return values and other subsets w/o prior assignment.
	+    you can take a where of a where

Version 0.51:
    * added -w option to bypass connecting to X-server
    * Modified [where] to allow arrays on rhs
    * Modified [where] to allow disparate id[where] and expr objects

Version 0.50: Mon Jun 28 13:29:59 MST 1999, 
    * Modified to use push parser in bison.  This will cause compile
      problems, but nothing that can't be handled.
      This eliminated input.c
    * Added xrt stuff

Version 0.47: Wed Mar 24 19:02:39 MST 1999
    + added minvert() LU decomposition matrix inversion
    * updated io_vicar to write new format, but is sun specific.
    * speedup to array.c
    + added limited support for 16-bit pgm files

Version 0.46: Wed Jan 20 17:41:32 MST 1999
    * interim version for VIMS.
    + beginnings of XRT
    + fft(), ifft()
    + added subsets to ff_read() 
    + fixed io_isis to properly handle blackplane corners

Version 0.45: Tue May  5 13:23:32 MST 1998
    * Preliminary version of apifunc

Version 0.44c:
    * rgb2hsv(), hsv2rgb()
    * started using autoconf
    * sort(), min(), max()
    * Added [where expr]

Version 0.44b: 
    * isis():   Get isis info, or load suffix planes
    * edit():   a numeric argument invokes a history editor, starting
                with the specified command.
    * history()
    * fsize()

Version 0.44a: Wed Nov 26 17:46:01 MST 1997
    * transfer version

Version 0.44: Tue Nov 18 23:35:08 MST 1997
    + newfunc.c
    + Added shortest unique match for argument keywords in newfunc
    + ff_ix.c: histogram()          (should move basis and mxm into here)
    + changed ff_rnoise into ff_random, and promoted it to newfunc.
    + Added ctrl-c handler, but it doesn't handle exiting ufuncs well.
    * changed bbr() and btemp() to try to avoid division by zero
    + avg(), sum()

Version 0.43c:
    * transfer version
    ! ID_ARGV wasn't evaluating its ranges.
    ! Broke ID_ARGV, above.  Fixed it.  p.c#123
    + mxm(), basis()

Version 0.43b:
    + added ff_bop(), moved bbr() and btemp() under it
    - problems with !, took it out of lexer.l
    * Lots of new help 
    + Added ifill()

Version 0.43a:
    + system()
    + sprintf()
    + printf()
    * upgraded to new readline library (2.1)
    + !cmd shell execution.
    * help routines overhaul (stolen from Gnuplot)
    + Inline concatenation:  1//2//3

Version 0.43: Sat Jul 19 17:21:38 MST 1997
    * Array subsetting steps (ie: [lo:hi:step])
    * Array replacements, finally..  And they work with subsetting steps too! 

Version 0.42m: Sat Jul 19 17:21:38 MST 1997
    * backup version, just in case

Version 0.42k: Thu Jun 19 01:25:26 MST 1997
    + pause()

Version 0.42j: Thu Jun 19 00:26:41 MST 1997
    * (unfinished) Modified just about every file to pass C++ strict linting.
    * Fixed io_isis.c, VAX_REAL wasn't getting handled properly.
    * Minor mod to io_isis.c, to match NJPL cookie, for micas PDS images
    * transfer version

Version 0.42i: Tue May  6 18:47:56 MST 1997
    * ff_vignette function
    * io_imath.c
    * Fixed pp_math to actually handle % operator, using fmod().

Version 0.42h: Fri Feb 14 14:16:53 MST 1997
    * Added bbr() black-body radiance function.
    * Added pow() function.
    * transfer version

Version 0.42g: Thu Dec 26 12:43:49 MST 1996
    * Modified ISIS reader to handle PDS IMAGES and VAX_INTEGER values
    * Added GEOS reader.
    * transfer version

Version 0.42f: Wed Oct  9 15:49:12 MST 1996
    * Added io_ers, Write routines for ER Mapper output.
    * patched io_isis to handle: CORE_ITEM_BYTE = VAX_REAL, including
      conversion routines.
    * patch to dispatch_ufunc: add unput_argv when a function has too many or
      not enough arguments.  Passed arguments were getting lost.

Version 0.42e: Thu Sep  5 12:06:19 MST 1996
    * patched io_isis to handle: CORE_ITEM_BYTE = SUN_REAL
    * patched several files using realloc() on NULL ptrs.
    * problem with return() from ufunc, when returning named variables.
      deleteing the name of the returned variable seemed to fix it.

Version 0.42d: Fri Mar 15 13:35:38 MST 1996
    * bug in ff_org().

Version 0.42c: Fri Feb 16 15:31:18 MST 1996
    * cleanup wasn't getting called for the top level, so memory stacked up.

Version 0.42b: Tue Feb  6 12:21:20 MST 1996
    * bug in unquote
    * added ability to use short and byte in range values

Version 0.42:  Mon Jan 22 12:16:20 MST 1996
    * Many misc changes during VIMS thermal vac.

Version 0.41b: Fri Jan  5 17:14:41 MST 1996
    * added OBSERVATIONAL_NOTES to the ISIS output, and made title="" go there.
      (Frank requested this)

Version 0.41a:  Fri Jan  5 17:14:41 MST 1996
    * interim version

Version 0.41: Sat Dec 30 02:11:05 MST 1995
    **************************************
    *          major rewrite             *
    **************************************
    * first implmentation of user defined functions completed
        + implemented scopes, dd, symtab and tmptab
        + added edit() command
    * made INC and DEC their own nodes (quick hack to fix free^2 bug)
    * moved to stack machine for evaluation
    * improved garbage collection (necessary for ufunc)

    ! Need to move to total bytecode machine.  Too many nodes as is.
    ! caused many headaches when doing GC.

Version 0.40a: Fri Dec 15 18:32:31 MST 1995
    * interim version
        + working on ufunc

Version 0.39d: Mon Nov 20 21:45:16 MST 1995
    * interim version
        + started on ufunc

Version 0.39c: Thu Oct  5 13:15:19 MDT 1995
    * Added fit().
    * minor changes to create()

Version 0.39b: Mon Aug 21 12:25:10 MST 1995
    * sym->title was getting multiply freed in many places.

Version 0.39a: Mon Aug 21 12:25:10 MST 1995
    * Broke $N variables when fixing filename expansion 

Version 0.39: Fri Aug 18 17:54:09 MST 1995
    * more minor compile patches.
    * included xfred library in distribution

Version 0.38: Sat Aug 12 13:22:56 MST 1995
    * added readline support, includes filename expansion
    * added ability to include $VAR and ~username in filenames
    * Minor compile fixes

Version 0.37: Thu Jul 20 17:01:15 MDT 1995
    * Modifications made while at JPL for VIMS cal
    * added delim= to ascii()
    * fixed bug in plot() involving newlines

Version 0.36:   Sun May 21 14:00:05 MDT 1995
    * moment() - compute statistics
    * interp() - point interpolation
    * PDS/ISIS write

Version 0.35a:  Mon Apr 24 05:00:58 MDT 1995
    * Added for loops
    * Write will not overwrite a file without force=1 keyword
    * added ASCII export (cheap hack)
    * removed recusion from compound statements

Version 0.34a: Tue Apr 11 17:02:24 MDT 1995
    * fixed getline bug in ff_ascii.c
    * added ability to set individual values in array
    * modified memory usage for speed (less allocations)
    * added skips to read routines

Version 0.33a: Fri Apr  7 02:50:09 MDT 1995
    * massive rewrite of the lexer and parser files.
    * added relational operators, and conditional loops (if/else, while)
        + broke daemon mode
        + need to fix history stuff

Version 0.32b: Wed Mar 29 17:01:46 PST 1995
    * Fixes while at vims.  io_isis.c, ff_ascii.c

Version 0.32a: Mon Mar 27 15:59:45 MST 1995
    * gplot() hack
    * display() hack

Version 0.31: Sun Mar 26 23:35:34 MST 1995
    * translate()
    * ascii()

Version 0.30: Thu Mar 16 15:26:32 MST 1995
    * Cleanup for VIMS distribution

Version 0.29: Wed Mar 15 17:04:11 MST 1995
    * online help

Version 0.28: Tue Mar  7 12:43:04 MST 1995
    * cat(), clone() and rnoise() functions
    * PDS/ISIS file I/O

Version 0.27: Thu Feb 16 05:24:27 MST 1995
    * -e option: specify expression on command line
    * -d option: shell daemon mode
    * -v option: set verbosity
    * added ';' to parser.y

Version 0.26: Wed Feb  8 14:24:03 MST 1995
    * pgm and ppm write
    * rgb()
    * 24bit ppm read -> 3 plane 8-bit cube
    * pbm, pgm read
    * fixed bug(s) in extract_array

Version 0.25: Mon Feb  6 17:20:23 MST 1995
    * added compressed file support to load() and filetype()
    * implemented reserved variable VERBOSE and SCALE
    * added echo()
    * added basename()
    * enhanced format() to do conversions through ff_conv
    * wrote primer

Version 0.24: Fri Feb  3 18:51:10 MST 1995
    * Added cluster and ccount
    * added list()
    * added (and repaired) gnoise()
    * added string addition

Version 0.23: Tue Jan 10 15:29:14 MST 1995
    * Added grd I/O
    * Added projection() 
    * Added Imagick Libriary routines making 30+ graphics formats read/writeable
*/

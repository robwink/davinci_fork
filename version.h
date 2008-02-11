char *version = "@(#) daVinci Version #1.71";

#include "build.h"

/*
Version 
    * Fixed CFITSIO configure/make problem
    * Fixed a memory allocation bug in thm.column_fill 
    + Created ff_transform.c file and added ff_image_rescale function 

Version 1.69:
    * Splited the main.c with the rest of davinci (libavinci shared lib)
    * Changed the default paths for modules, help, etc from /usr/local to just /usr
    * Fixed --with-help in configure.ac to sue a path
    * Help file is also distributed when installed
    * libdavinci and it's header files are distributed under 
      /usr/lib and /usr/include/davinci respectively
    
Version 1.68
    * Added MINGW32 support

Version 1.67:
    * Added flag for using aqua under darwin (gplot needs this)
    * Checked in changes for radial symmetry

Version 1.66: 
    * Fixed crashing bug in xplot for "struct.val" being undefined
    * Fixed crashing bug when assinging to TEXTs beyond end
    + added ignore to xplot

Version 1.65: Mon Mar 26 13:43:12 MST 2007
    Maintenance version for hdf libraries
    + Added ff_cinterp
    + Added type and ignore to interp.

Version 1.64:
    * Maintenance version for cfitsio library problem.
    + make display() output pngs not ppms

Version 1.63: Thu Feb 15 16:51:24 MST 2007
    * Modifications to move external program configurations into
      configure and report them during the configure process
    * Maybe fixed --with-hdf in configure to sue a path.
      Only concern is that -lz is hardcoded.

Version 1.62: Wed Jan 17 18:31:15 MST 2007 NsG
    Fixed a bug related to iomedley data being double free'd

Version 1.61: Thu Nov 30 17:38:16 MST 2006, NsG
    added a "get all" default behavior to get_struct_keys().
    fixed segfault in load_vanilla related to double free on error

Version 1.60: Tue Nov  7 13:11:37 MST 2006
    Fixes to do stream processing after X is initialized

Version 1.59: Wed Oct 18 20:38:31 MST 2006
    Several realloc bugs reported by gcc 4.0.2, 
        including load_module() and help()

Version 1.58:  Mon Sep 18 12:34:11 MST 2006
    Fixed isis() to not dump on numeric plane number
    Added round() to dfunc
    Added load_tdb()

Version 1.57: Wed May 31 14:49:02 MST 2006
    * changed all occurances of getline to dv_getline for cygwin conflict.
    * put #ifdef around strndup declaration, again for cygwin

Version 1.56: Tue May 30 16:35:20 MST 2006
    * Merged in a bunch of changes from trip to hawaii
        * fixed printf stdarg compile problem
        * fixed line buffering during debug output
        * added source file name to error handlers
    * Moved the gui module into its own directory
    * Fixed some configuration issues (needs more testing).
    * Re-added michaels' deghost changes from 1.53.

Version 1.55: Wed May 10 10:49:35 MST 2006
        * Inserted error handing for functions:
          bil()     bip()       bsq()        org()      order()
          load_ir() load_fits() read_lines() basename() dirname()
          length()  resize()    interp()     gplot()    bbr()
          btemp()   atan2()     jfjll()      ifill()    slant()
          ascii()   and         read_ascii()
          to not crash when no arguments are provided.

Version 1.52: Wed Nov 16 14:58:20 MST 2005
    * Fixed bug#136: Real valued array history keywords
      were not being properly written out. The function
      call expected floats while doubles were being passed.

Version 1.51: Tue Aug 23 19:10:49 MST 2005
    * Upgraded LoadPDS() to handle compressed files
    * Fixed problems detailed in tests/regression_0508023
    * Added "position parameter to replace_struct

Version 1.50: Wed Aug 10 15:58:22 MST 2005
    * Modified the deghost routine to use themis3.frmgeom instead of
      themis2.geometry_detail, for retrieving framelet corners.

Version 1.49: Thu Jul 28 14:24:05 MST 2005
        * Finally formally committing the deghost updates developed back in
      sept of 2004.
    * Basically, these use actual MOLA data and geometry_detail
      interpolation to derive a more accurate ghost estimate, instead of
      using the single reference values originally derived for a particular
      altitude.

Version 1.48: Mon May 27 15:53:58 MST 2005
    * Fixed variety of gui module bugs
    * Fixed example scripts in modules/guitest
    * Fixed gui module's constraint resource handling
    * Added documentation for ff_extract() and gui module
    * Added gui module test examples in modules/guitest
    * Added combobox widget support to gui module
    * Added rectf overlay support to gui module's Vicar interface
    * Added lookup table set/get functions to gui module's Vicar interface
    * Added LineBox widget to gui module

Version 1.47: Fri May 20 11:41:13 MST 2005
    * Bugs in ff_text were all related to an off by one counting 
      in single_replace.  Fixed, and the other bogosities in there
      also removed.

Version 1.46: Fri May 20 11:41:13 MST 2005
    * Fixed crash in ff_text:single_replace (don't ask me why)

Version 1.45: Thu May 19 13:30:22 MST 2005
    * Updates to window() for histogram functions min/max.

version 1.44a: Wed May 18 11:05:56 MST 2005
    + Fixed a bug in load_paci for linux machines
   + Minor change to ff_text.c ID_VAL --> ID_UNK; more flexibility
   + Added a simple FITS writing routine

Version 1.44: Fri Apr 29 21:38:52 MST 2005
    * 'fixed' scope temporarys to insert if the last element is null,
      instead of making the temporary stack longer. This should help to
      prevent really long things from running out of memory by overloading
      the stack of temporaries.
    * Removed an apparently extraneous mem_claim from duplicate_struct,
      however with the above fix, this is less brilliant.

Version 1.43a: Thu Mar 24 16:47:07 MST 2005
    * Missing prototype in isis_obj.c

Version 1.43: Thu Mar 24 15:41:25 MST 2005
    * Fixed lingering module dependency (module_help) for when
       modules aren't included.

Version 1.42a: Wed Mar 23 16:45:01 MST 2005
        * Changed system() semantics slightly.

Version 1.42: Mon Feb  7 19:49:59 MST 2005
    * Commited ff_window, ff_gconvolve, ff_warp and ff_median.

    + Added load_fits() and write_fits(), read/write
        routines for FITS type data.  Currrently, tables
        are not supported; their labels are read, but the
        data contained in the table is NOT. write_fits
        currently writes out single VAR objects and slaps
        a default label on them. 

Version 1.41: Fri Oct  1 11:06:45 MST 2004
    + Fixed typo in the definition of get_struct_key()
      where the "index" parameter was incorrectly labelled
      as "int".
    + Updated dv.gih with help text for load_bin5(), 
      write_isis_cub() and get_struct_key().

Version 1.40: Thu Sep 30 09:43:50 MST 2004
    + Added load_bin5() to load bin5 files generated by krc.

Version 1.39: Fri Sep 24 18:01:46 MST 2004
    + Fixed stomping over out of bounds memory in the last
      line of history data while removing indent marks from
      it.
    + Initialized a return variable in HDF reader.

Version 1.38: Thu Aug 12 10:17:38 MST 2004
    + Added davinci function get_struct_key(), which returns
      the name associated with a particular structure element.
      If the element is unnamed, an empty string is returned.
      If the element is invalid, null is returned.

Version 1.37: Thu Aug 12 09:39:52 MST 2004
    * Fixed write_isis_planes() to correctly output all three
      suffix planes.
    * Source before this fix is tagged before_isis_backplane_fix.
    * Source before write_isis_cub() function was added is
    + New function write_isis_cub() added. As the name suggests,
      it an ISIS cube file writer. It is dependent upon ISIS
      libraries. "configure" script has been modified to check
      for the appropriate ISIS libraries.
    + Added a new option to load_pds() called "suffix_data". When
      set, the suffix data from PDS qubes is read in and is made
      available as *qube.suffix_data.

Version 1.36: Tue Aug 10 02:40:22 MST 2004
    * some leftover HAVE_LIBX11 macros caused the plotting stuff to not be on.
    + added C-style unary increment and decrement.  This is odd for strings.
    + ff_boxfilter()  (in 3-D even!)
    * fixed broken echo, left over from fixing args back from around v1.29

Version 1.35: Fri Aug  6 16:04:28 MST 2004
    * The penultimate check-in for the new autoconf stuff

Version 1.34: 
    * Who knows?  Saadat didn't make a log entry.  Something to do with
      new libisis modules.

Version 1.33: Tue Aug  3 13:21:04 MST 2004
    * Checked in a bunch of pending minor changes.  Maybe autoconf isn't
      quite right though.

Version 1.32: 
    * Fixed fit to use ignore for types other than linear.
    * Fixed fit to use default x axis if one isn't supplied.  It's one based.
    * Finished fixing the HAVE_DRAND48 business

Version 1.31a: Thu Jul  1 18:09:24 MST 2004
    * Part of a hack to fix HAVE_DRAND48

Version 1.31: Wed Jun 30 03:35:03 MST 2004
    * Minor changes to hdf reader to be compatible with new version
          of the library

Version 1.30: Tue Jun 29 23:03:41 MST 2004
    * Fixed fit(type=linear) to avoid a complete duplication of the data

Version 1.29: Fri Jun 25 14:11:55 MST 2004
    * fixed another bug in global()
    * bug in ff_min with ignore values
    * several hacks to make module.help() work

Version 1.28: Tue Jun 22 10:07:08 MST 2004
    * Renamed OBJDESC field "class" to "obj_class" to keep
      in sync with iomedley.

Version 1.27: Wed Jun 16 19:29:59 MST 2004
    * Bug in global() related to new argument handling

Version 1.26: Fri Jun  4 13:48:45 MST 2004
    * Bug in ff_format related to new argument handling

Version 1.25: Thu Apr 15 19:47:19  2004
    * Hopefully the final cleanup for the new argument handling

Version 1.24: Tue Feb 10 14:58:27 MST 2004
    * Fixed ff_convolve (wasn't using ignore properly)[Ben]

Version 1.23: Mon Jan 26 14:08:02 MST 2004
    * Fixed some uninitialized VAR* variables in ff_deghost that were
      causing core dumps for Kim. [Michael]

Version 1.22: Tue Jan 20 12:37:20 MST 2004
    * there's versioning problems in /usr/local/bin.
      Incremeting version to make sure I know what's installed.


Version 1.21: Thu Jan  8 14:27:41 MST 2004
    + Fixed GUI modules X resource setter to handle XmStringList sets with
      redundant elements.
    + Added gui.listcallbacks() function to GUI module to list supported
      callbacks for each widget.

Version 1.20: Wed Dec 24 02:22:58 MST 2003
    * Fixed embedded (){} in quotes for load_pds
    * Fixed "first element looks like an int but rest are floats" in
      load_pds, when loading a sequence or set
    * Moved over ff_source from -hstr to use dv_locate_file and new args

Version 1.19: Tue Dec 23 15:12:00 MST 2003
    * Modified event_loop to do Xt setup before processing streams,
      so that you can, for instance, load_module("gui") in your .dvrc.
      This also means that you don't have to be interactive to use Xt.

Version 1.18: 
    * Who knows?  Jim won't put in log messages.

Version 1.17: Thu Dec  4 20:35:59 MST 2003
    + Converted to use automake, libtool, and modern autoconf.
    + Added GUI module and VICAR widget library.
    + Added HTML documentation for GUI module.
    + Cleaned up miscellaneous compilation warnings.

Version 1.14: Fri Aug 29 16:09:36 MST 2003
    + Have added a new image processing function: ff_fncc
        it is called as fncc(template,source).  It currently
        returns two objects in a structure.  corner_point is
        the only useful object at the moment

Version 1.13: Tue Jul 15 15:43:38 MST 2003
    + misc checking, including bug in load() for mini-tes .QUB files
    (No, I have no idea why this fixed it)

Version 1.12: Tue Apr  1 15:15:04 MST 2003
    + Fixed a file handle leakage while reading ISIS suffix planes.

Version 1.12: Tue Apr  1 15:15:04 MST 2003
    + Fixed a file handle leakage while reading ISIS suffix planes.

Version 1.11: Mon Mar 24 15:51:18 MST 2003
    + Various fixes plus modifications to dump to handle other variables (texts)
    + added slant and unslant, plus rice code

Version 1.10:
    + Fix to io_lablib3 (PDS label reader) which overwrote memory 
      elsewhere while reading constructs like:
                    INSTRUMENT_FOV = 20<MRAD>

Version 1.09: Wed Jan 22 15:55:44 MST 2003
    + Fixes (again) to gplot to allow GPLOT_CMD environment variable.

Version 1.08: 
    + Updates to ff_raw, and some other minor changes

Version 1.07: Tue Oct  8 16:54:47 MST 2002
    + Merged in OS/X changes

Version 1.06: Mon Sep 30 21:16:01 MST 2002
    + promoted unsigned shorts to ints in load_pds()

Version 1.05: Fri Sep 20 22:56:35 MST 2002
    + load_pds() table reader didn't handle endian_ness

Version 1.04: Mon Sep 23 10:35:34  2002
    + Added a new version of HDF5 library and enabled it.
    + Moved HDF5 in its own subdir in the win32 directory 

Version 1.03: Fri Sep 20 18:30:36  2002
    + Cleaned the code for windows compile

Version 1.02: Fri Sep 20 18:30:36  2002
    + environment variables were getting freed (that's bad)

Version 1.01: Wed Sep 18 15:30:40 MST 2002
    + Bug in io_isis.c, detached labels weren't working correctly.

Version 0.99d: Wed Jun 26 17:33:23 MST 2002
    + merged everything together

Version 0.99a: Wed Jun 26 17:33:23 MST 2002
    + geom_deghost

Version 0.99: Wed Jun 26 17:33:23 MST 2002
    + Added a real envi io module
    
Version 0.98b:
    + Added insert_struct, and add_struct is now just an alias

Version 0.98a: Tue Jun 25 00:16:58 MST 2002
    + Added left and right shifting
    * think I fixed the (arg=NULL) problem with function calls

Version 0.98: Wed Jun 12 03:28:21 MST 2002
    + Modifications to write_isis routines to handle BIP cubes and planes

Version 0.97: Wed Jun 12 03:28:21 MST 2002
    + Checking of several bug fixes and misc modifications

Version 0.96: Wed Jun 12 03:28:21 MST 2002
    + Added moments(), covar(ignore) and avg(ignore)

Version 0.95: Fri May 31 16:41:03 MST 2002
    + Made HDF use automatic compression

Version 0.94: Tue May 28 13:45:12 MST 2002
    + added changes to support read/write IO modules, using libtool

Version 0.93: Mon May  6 20:13:47 MST 2002
    + Modified dvio_specpr to read whole file if record not specified

Version 0.92: Tue Apr 18 11:09:06 MST 2002
    + separated out endian logic from HDF code, causing compile
      to fail when HDF not configured.

Version 0.91: Mon Apr 15 19:07:21 MST 2002
    + Added hstats() to compute histogram statistics

Version 0.90: Fri Mar 29 16:39:56 MST 2002
    *Added SPECTRAL_QUBE as a legit PDS object 
        to the pds READER and WRITER


Version 0.89: Tue Mar 26 18:07:55 MST 2002
    * Added atod()
    * Added remove_struct()
    * From version 0.87c, added structure addition
    * Also fixed SCALE 

Version 0.88: Thu Mar 21 12:27:50 MST 2002
    * Added a working rf_HISTORY function to dvio_pds
    * Added a working HISOTRY writer to WritePDS

Version 0.87b: Wed Mar 13 18:17:30  2002
    * Added some #defines for windows compile
    * Replaced gplot with gnuplot for ff_plot
    + structure addition

Version 0.87a: Fri Mar  8 13:24:12 MST 2002
    * Bug in [where] for single valued rhs.

Version 0.87: Mon Feb 25 16:26:57 MST 2002
    * Made ff_avg use doubles instead of floats
    * Randy forgot to update the version when he fixed hdf5 reads

Version 0.86: Thu Feb 21 13:32:54 MST 2002
    * Modified ff_interp so that it caches the slopes and the
      y-intercepts of all the line segments in domain.

Version 0.85: Wed Feb 20 18:42:58 MST 2002
    * Modified ff_interp to use binary-searching for searching 
      domain values (instead of linear search).

Version 0.84: Tue Feb 19 23:15:41 MST 2002
    * Make load() take a text to load multiple files, returns struct
    * Fixed io_isis to load by filename rather than file pointer
    * Made display use pnm io routines

Version 0.83: Wed Jan  9 15:41:00 MST 2002
    * Fixed a bug in xplot regarding NULL (or no) varaibles being passed
    * Changes PDS reader functionality to be able to have multiple objects in a file
    * Fixed bug in the pds table reader (when it was scaling values)

Version 0.82: Mon Nov 26 12:37:07 MST 2001
    * Errors in alist for ff_ix.c:hsv2rgb
    * users were using 'scale' and setting reserverd word SCALE

Version 0.81:
    * dvio_imath was reading data size incorrectly from
      the header, resulting in memory overwrites.

Version 0.80:
    * Made the INTFMT and REALFMT keyword values (in a VICAR
      file) default to the platform-defaults instead of the 
      standard-defaults (when they are not specified in the
      VICAR file).

Version 0.79b: Thu Jul 19 21:25:13 MST 2001
    * lost fexist from ff.h
    + added ff_shade

Version 0.79a: Fri Jun  8 20:29:21 MST 2001
    * Someone overwrote with an old version of ff.h.  Fixed.

Version 0.79: Tue May 29 18:43:38 MST 2001
    * Brought all changes up to date between 0.7 tree and 0.6 tree.
    * Fixed version numbering.

Version 0.7.2: Thu Apr 12 13:45:00 MST 2001
    * Added verbosity control to iomedley.

Version 0.7.1: Tue Apr  3 13:46:19 MST 2001
    * Version just after merging the iomedley branch in.
    * Made modules code conditional as well.

Version 0.7: Mon Apr  2 17:57:21 MST 2001
    * Version just before merging the iomedley branch in.
    * iomedley branch to be merged v-0_64_1-iomedley
    * last tag on v-0_64_1-iomedley is v-0_64_1-iom-final

Version 0.68: 
    * Extended cat to take a variable number of args
    * fixed bug in ff_history/print_history

Version 0.67.1: Thu Mar 29 14:35:21 MST 2001
    * buf in ff_rgb.  V_DSIZE() was getting improperly set

Version 0.67: Thu Mar 22 13:51:39 MST 2001
    * This is a tag version before I start mucking with scope
    * Fixed a bug with dereference of null structs.
    * added length()
    * fixed a bug for zero sized structures

Version 0.66: Wed Mar 21 16:23:28 MST 2001
    * fixed bug in syscall(), it could return 0 length arrays
    * added fexists() and putenv()

Version 0.65: Thu Mar  8 19:42:56 MST 2001
    * Added ability to specify empty options for enums 
    * bugs in WriteAscii().
    * added identity()
    * made if () handle arrays.  This is tricky.  
      It is possible for this to be false: (x < 1 || x > 1)
      The entire array of conditionals must be true to get true.

Version 0.64.2: Thu Feb  1 17:50:00 MST 2001
    * problem in reading detached labels

Version 0.64.1: Fri Jan 19 11:54:50 MST 2001
    * Fixed core dump on user-function return value
    * Removed "readline/readline.h" from Makefile.in

Version 0.64: Thu Jan 11 17:27:46 MST 2001
    * Fixed bugs is isis()
    * Added modules support
    * Fixed a lot of stuff for multi-platform compiles

Version 0.63: Tue Jan  9 17:47:46 MST 2001
    * Found memory loss bug in make_args/parse_args.
    * Above bug found AFTER converting tmp table to Darray

Version 0.62c: Wed Dec  6 16:50:32 MST 2000
    * yet another bug in detached ISIS labels

Version 0.62b: Mon Oct 23 17:21:59 MST 2000
    * ff_histogram only extracted ints

Version 0.62a: Thu Oct 12 17:00:43 MST 2000
    * broke ufunc

Version 0.62: Thu Oct 12 17:00:43 MST 2000
    * enhanced version info and -V flag
    * fixed bug with struct/scope memory
    * fixed stack errors in unescape and unquote
    * made X11 input depend on Xt not Xm
    * Added ID_LINE for a robust debug capability
    * mk_node no longer uses scope memory.  This should make things faster

Version 0.61: Thu Sep 14 17:55:05 MST 2000
    * Changed structures to use Narray associative arrays

Version 0.60: Wed Aug  9 17:42:00 MST 2000
    + Added "niter" parameter to ff_pcs() and ff_eigen().
      Use this parameter to increase the maximum number of 
      iterations performed by the Eigen Values convergence 
      algorithm before giving up.

Version 0.59a: Wed Aug  2 16:17:03 MST 2000
    * Fixed line nmumbers, again.  Pushes pp_lineno onto the stack for source()

Version 0.59: Wed Jul 26 15:15:16 MST 2000
    * Modifications to io_specpr
    * Added ONE_AXIS and ANY_AXIS to newfunc
    * This is really more of 0.57a

Version 0.58 Thu Jun  1 09:16:56 MST 2000
    *Have made several modifications to io_themis to increase
        it's robusticity for reading corrupted datasets.
    *Have also added more informational output to io_themis

Version 0.57a: Mon Apr 24 16:00:26 MST 2000
    * updated historam()
    * made a fb for ff_min()
    * enhanced ff_avg() to use 1 pass algorithm.

Version 0.57: Thu Apr 20 15:00:49 MST 2000
    * Modified structure subsetting to return the actual var, not a copy.
      This might have ramifications, but was necessary to make a.b.c work.
      It also facilitates inc and dec working.
    + Added both=1 flag to avg();
    * mucked with ff_text to make additions work, and comparisons faster

Version 0.56f: Tue Apr 11 19:37:10 MST 2000
    * Fixed a bunch of compile problems without plplot and hdf.

Version 0.56e: Wed Apr  5 15:19:57 MST 2000
    = load_vanilla() updated -- made faster
    
Version 0.56d: Mon Apr  3 18:51:48 MST 2000
    * Bug: file pointer in read_lines() wasn't getting closed.
    * Bug: signal handler for SIG_PIPE was disabled

Version 0.56c: Sat Apr  1 19:28:44 MST 2000
    * bug in array.c.  Caused a scrolling effect in array replacement.

Version 0.56b Wed Mar 29 15:19:05 MST 2000
    + load_vanilla() - Read vanilla generated text output files

Version 0.56a Thu Mar 23 14:45:30 MST 2000
   + pcs() - Principal Component Stretch
   + covar() - Covariance Matrix
   + corr() - Coefficients of Correlation Matrix
   + scp() - Sum of squares and cross products matrix
   + eigen() - Eigen Values & Vectors of Real Symmetric Matrices
   + Test cases and data for testing the above functions
    + HDF read/write for textarray objects

Version 0.56: Wed Mar 22 23:46:06 MST 2000
    + undid ufunc modifications
    + fixed !shell
    + finally started tagging revisions

Version 0.55a:
    * modified ufunc loader
    + added delete()

Version 0.55:
    * fixed eval again.  I think you can return values now
    + global()
    + dump()
    + structures: { }, [], add_struct(), get_struct()
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

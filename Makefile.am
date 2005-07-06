## Makefile.am
##
## Jim Stewart
## 20 Nov 2003
##
## Used by automake and autoconf to build a Makefile.

## FIX: put config.h.in, configure.ac, Makefile.am somewhere so make dist works?

## DIST_SUBDIRS = config docs lib libltdl moddvmagick modules tests vicar win32
EXTRA_DIST = README.modules README_FF docs gplot.c lexer.l lib moddvmagick parser.y push.skel tests win32
## EXTRA_DIST = README.modules README_FF docs push.skel? shared_globals.c? tests win32

if MODULE_SUPPORT
  MY_MODULES_LIB = libmodsupp.a
endif

if LOCAL_READLINE
  MY_READLINE_LDADD = readline/libreadline.a
endif

MY_IOMEDLEY_LDADD =-Liomedley -liomedley

SUBDIRS = @MY_SUBDIRS@ libltdl

bin_PROGRAMS = davinci

BUILT_SOURCES = config.h build.h

noinst_LIBRARIES = $(MY_MODULES_LIB)

libmodsupp_a_SOURCES = ff_modules.c module_io.c io_loadmod.c

davinci_LDADD = $(MY_READLINE_LDADD) $(MY_IOMEDLEY_LDADD) $(MY_MODULES_LIB) 
davinci_LDFLAGS = -export-dynamic

davinci_SOURCES = apifunc.c array.c avl.c binDCT.c \
	darray.c \
	dct.c deghost.C dvio.c dvio_ascii.c \
	dvio_aviris.c dvio_envi.c dvio_ers.c dvio_goes.c dvio_grd.c \
	dvio_hdf.c dvio_imath.c dvio_iomedley.c dvio_isis.c dvio_magic.c \
	dvio_pds.c dvio_pnm.c dvio_raw.c dvio_specpr.c \
	dvio_themis.c dvio_vicar.c endian_norm.c \
	error.c ff.c ff_ascii.c ff_avg.c \
	ff_bbr.c ff_cluster.c ff_convolve.c ff_deghost.c ff_display.c \
	ff_fft.c ff_filetype.c ff_fncc.c ff_gnoise.c ff_gplot.c \
	ff_header.c ff_hstretch.c ff_ifill.c ff_interp.c ff_ix.c \
	ff_load.c ff_loadvan.c ff_meta.c \
	ff_moment.c ff_pause.c ff_pbm.c ff_pca.c ff_plplot.c \
	ff_projection.c ff_random.c ff_raw.c ff_rgb.c ff_rice.c \
	ff_shade.c ff_slant.c ff_sort.c ff_source.c ff_struct.c \
	ff_text.c ff_version.c ff_vignette.c ff_write.c ff_xfrm.c \
	fft.c fft2f.c fft_mayer.c fit.c ff_coreg.c ff_boxfilter.c \
	ff_bin5.c ff_extract.c \
	header.c \
	help.c init.c \
	isis_obj.c main.c matrix.c \
	misc.c motif_tools.c \
	newfunc.c p.c pp.c \
	pp_math.c printf.c reserved.c \
	rfunc.c rice.c rpos.c scope.c \
	spread.c string.c symbol.c \
	system.c tools.c \
	ufunc.c util.c \
	x.c xrt_print_3d.c \
	XformTable.h darray.h fit.h trigtbl.h api.h dos.h func.h \
	products.h ufunc.h api_extern_defs.h dvio.h header.h \
	proto.h version.h apidef.h dvio_specpr.h help.h rfunc.h \
	xrt_print_3d.h avl.h endian_norm.h io_loadmod.h scope.h \
	ff.h isis_obj.h system.h ff_modules.h module_io.h \
	toolbox.h configwin.h fft.h motif_tools.h tools.h \
	lexer.c parser.c parser.h y_tab.h \
	ff_gconvolve.c ff_warp.c ff_median.c ff_radial.c ff_window.c \
	dvio_fits.c

build.h: Makefile
	echo 'char *build = "'`date`'";' > $@
	echo 'char *builder = "'`id | sed -e 's/[^(]*(\([^)]*\)).*/\1/'`'@'`hostname`'";' >> $@

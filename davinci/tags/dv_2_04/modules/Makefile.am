if HAVE_LIBMAGICK
  MY_MAGICK_LIBRARY = dvmagick.la
  dvmagick_la_SOURCES = dvmagick.c
  dvmagick_la_LDFLAGS = -module
endif

mod1_la_SOURCES = mod1.c
mod1_la_LDFLAGS = -module

mod2_la_SOURCES = mod2.c
mod2_la_LDFLAGS = -module

pnm_mod_la_SOURCES = pnm_mod.c
pnm_mod_la_LDFLAGS = -module

pkglib_LTLIBRARIES = mod1.la mod2.la pnm_mod.la $(MY_MAGICK_LIBRARY)
SUBDIRS = thm cse kjn

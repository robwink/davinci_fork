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

MY_GUI_LIBRARY =

if LOCAL_VICAR
	  MY_VICAR_LIBADD = $(top_srcdir)/vicar/libvicar.la
endif
if HAVE_MOTIF
	gui_la_SOURCES = gui.c \
		widget_arrowbutton.c widget_cascadebutton.c widget_command.c \
		widget_drawnbutton.c widget_errordialog.c widget_fileselectionbox.c \
		widget_form.c widget_frame.c widget_label.c \
		widget_list.c widget_menubar.c widget_panedwindow.c \
		widget_pushbutton.c widget_radiobox.c widget_rowcolumn.c \
		widget_scale.c widget_scrollbar.c widget_scrolledlist.c \
		widget_scrolledwindow.c widget_selectionbox.c widget_separator.c \
		widget_text.c widget_textfield.c widget_togglebutton.c \
		widget_toplevelshell.c widget_transientshell.c \
		widget_vicar.c
		gui.h widget_arrowbutton.h widget_cascadebutton.h widget_command.h \
		widget_drawnbutton.h widget_errordialog.h widget_fileselectionbox.h \
		widget_form.h widget_frame.h widget_label.h widget_list.h \
		widget_menubar.h widget_panedwindow.h widget_pushbutton.h \
		widget_radiobox.h widget_rowcolumn.h widget_scale.h \
		widget_scrollbar.h widget_scrolledlist.h widget_scrolledwindow.h \
		widget_selectionbox.h widget_separator.h widget_textfield.h \
		widget_text.h widget_togglebutton.h widget_toplevelshell.h \
		widget_transientshell.h widget_vicar.h widget_xbaematrix.h

	gui_la_LDFLAGS = -module
	gui_la_LIBADD = $(MY_VICAR_LIBADD)

	MY_GUI_LIBRARY = gui.la
endif

pkglib_LTLIBRARIES = mod1.la mod2.la pnm_mod.la $(MY_GUI_LIBRARY) $(MY_MAGICK_LIBRARY)


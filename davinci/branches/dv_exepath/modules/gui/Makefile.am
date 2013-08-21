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
	widget_vicar.c widget_combobox.c LineBox.c widget_linebox.c \
	gui.h widget_arrowbutton.h widget_cascadebutton.h widget_command.h \
	widget_drawnbutton.h widget_errordialog.h widget_fileselectionbox.h \
	widget_form.h widget_frame.h widget_label.h widget_list.h \
	widget_menubar.h widget_panedwindow.h widget_pushbutton.h \
	widget_radiobox.h widget_rowcolumn.h widget_scale.h \
	widget_scrollbar.h widget_scrolledlist.h widget_scrolledwindow.h \
	widget_selectionbox.h widget_separator.h widget_textfield.h \
	widget_text.h widget_togglebutton.h widget_toplevelshell.h \
	widget_transientshell.h widget_vicar.h widget_xbaematrix.h \
	widget_combobox.h widget_linebox.h LineBox.h LineBoxP.h
gui_la_LDFLAGS = -module
gui_la_LIBADD = $(top_srcdir)/vicar/libvicar.la
$(top_srcdir)/vicar/libvicar.la :
	make -C $(top_srcdir)/vicar

pkglib_LTLIBRARIES = gui.la

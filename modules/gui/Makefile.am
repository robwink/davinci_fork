gui_la_SOURCES = \
    gui.c \
    LineBox.c \
    widget_arrowbutton.c \
    widget_cascadebutton.c \
    widget_combobox.c \
    widget_command.c \
    widget_drawingarea.c \
    widget_drawnbutton.c \
    widget_errordialog.c \
    widget_fileselectionbox.c \
    widget_form.c \
    widget_frame.c \
    widget_label.c \
    widget_linebox.c \
    widget_list.c \
    widget_mainwindow.c \
    widget_menubar.c \
    widget_menupopup.c \
    widget_menupulldown.c \
    widget_notebook.c \
    widget_panedwindow.c \
    widget_pushbutton.c \
    widget_radiobox.c \
    widget_rowcolumn.c \
    widget_scale.c \
    widget_scrollbar.c \
    widget_scrolledlist.c \
    widget_scrolledwindow.c \
    widget_selectionbox.c \
    widget_separator.c \
    widget_text.c \
    widget_textfield.c \
    widget_togglebutton.c \
    widget_toplevelshell.c \
    widget_transientshell.c \
    widget_vicar.c \
    widget_xyplot.c \
    gui.h \
    LineBox.h \
    LineBoxP.h \
    widget_arrowbutton.h \
    widget_cascadebutton.h \
    widget_combobox.h \
    widget_command.h \
    widget_drawingarea.h \
    widget_drawnbutton.h \
    widget_errordialog.h \
    widget_fileselectionbox.h \
    widget_form.h \
    widget_frame.h \
    widget_label.h \
    widget_linebox.h \
    widget_list.h \
    widget_mainwindow.h \
    widget_menubar.h \
    widget_menupopup.h \
    widget_menupulldown.h \
    widget_notebook.h \
    widget_panedwindow.h \
    widget_pushbutton.h \
    widget_radiobox.h \
    widget_rowcolumn.h \
    widget_scale.h \
    widget_scrollbar.h \
    widget_scrolledlist.h \
    widget_scrolledwindow.h \
    widget_selectionbox.h \
    widget_separator.h \
    widget_textfield.h \
    widget_text.h \
    widget_togglebutton.h \
    widget_toplevelshell.h \
    widget_transientshell.h \
    widget_vicar.h \
    widget_xyplot.h
gui_la_LDFLAGS = -module
gui_la_LIBADD = $(top_srcdir)/vicar/libvicar.la
$(top_srcdir)/vicar/libvicar.la :
	make -C $(top_srcdir)/vicar
$(top_srcdir)/plot_widgets/plot_widgets/libPlotW.a :
	make -C $(top_srcdir)/plot_widgets
$(top_srcdir)/plot_widgets/util/libNUtil.a :
	make -C $(top_srcdir)/plot_widgets/util
pkglib_LTLIBRARIES = gui.la

#ifndef _widget_xyplot_h
#define _widget_xyplot_h

#include "gui.h"
#include "../plot_widgets/plot_widgets/XY.h"

int gui_isXYPlot (const char *);
WidgetClass gui_getXYPlotClass ();
void gui_setXYPlotPseudoResources (Widget, Var *, Narray *);
CallbackList gui_getXYPlotCallbacks ();
Widget gui_initXYPlot(const char *, WidgetClass, Widget, Var *, void **,
	Narray *, Widget *);
void gui_getXYPlotPseudoResources(const Widget, Var *);

#endif

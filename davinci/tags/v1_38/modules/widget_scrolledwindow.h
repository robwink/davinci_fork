/* widget_scrolledwindow.h
 *
 * Davinci GUI binding for various Motif widgets.
 *
 * Copyright 2003
 * Mars Space Flight Facility
 * Department of Geological Sciences
 * Arizona State University
 *
 * Jim Stewart <Jim.Stewart@asu.edu>
 *
 */

#ifndef DV_WIDGET_SCROLLEDWINDOW_H
#define DV_WIDGET_SCROLLEDWINDOW_H

#include <Xm/ScrolledW.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int		gui_isScrolledWindow(const char *);
WidgetClass	gui_getScrolledWindowClass(void);
CallbackList	gui_getScrolledWindowCallbacks(void);
Narray *	gui_getScrolledWindowPublicResources(void);

#endif /* DV_WIDGET_SCROLLEDWINDOW_H */

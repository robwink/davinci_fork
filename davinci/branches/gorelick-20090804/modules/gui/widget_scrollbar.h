/* widget_scrollbar.h
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

#ifndef DV_WIDGET_SCROLLBAR_H
#define DV_WIDGET_SCROLLBAR_H

#include <Xm/ScrollBar.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int		gui_isScrollBar(const char *);
WidgetClass	gui_getScrollBarClass(void);
CallbackList	gui_getScrollBarCallbacks(void);

#endif /* DV_WIDGET_SCROLLBAR_H */

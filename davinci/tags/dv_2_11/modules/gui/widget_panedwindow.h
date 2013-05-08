/* Widget_panedwindow.h
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

#ifndef DV_WIDGET_PANEDWINDOW_H
#define DV_WIDGET_PANEDWINDOW_H

#include <Xm/PanedW.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int		gui_isPanedWindow(const char *);
WidgetClass	gui_getPanedWindowClass(void);
Narray *	gui_getPanedWindowPublicResources(void);

#endif /* DV_WIDGET_PANEDWINDOW_H */

/* widget_notebook.h
 *
 * Davinci GUI binding for various Motif widgets.
 *
 * Copyright 2005
 * Mars Space Flight Facility
 * Department of Geological Sciences
 * Arizona State University
 *
 * Eric Engle <eric.engle@asu.edu>
 *
 */

#ifndef DV_WIDGET_MAINWINDOW_H
#define DV_WIDGET_MAINWINDOW_H

#include <Xm/MainW.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int				gui_isMainWindow(const char *);
WidgetClass		gui_getMainWindowClass(void);
CallbackList	gui_getMainWindowCallbacks(void);

#endif /* DV_WIDGET_MAINWINDOW_H */

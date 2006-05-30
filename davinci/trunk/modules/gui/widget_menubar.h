/* widget_menubar.h
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

#ifndef DV_WIDGET_MENUBAR_H
#define DV_WIDGET_MENUBAR_H

#include "gui.h"
#include <Xm/RowColumn.h>

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int			gui_isMenuBar(const char *);
Widget		gui_initMenuBar(const char *, WidgetClass, Widget, Var *, void **, Narray *, Widget *);
WidgetClass	gui_getMenuBarClass(void);

#endif /* DV_WIDGET_MENUBAR_H */

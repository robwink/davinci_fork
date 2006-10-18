/* widget_menupulldown.h
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

#ifndef DV_WIDGET_MENUPULLDOWN_H
#define DV_WIDGET_MENUPULLDOWN_H

#include <Xm/RowColumn.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int				gui_isMenuPulldown(const char *);
WidgetClass		gui_getMenuPulldownClass(void);
Widget			gui_initMenuPulldown (const char *, WidgetClass, Widget, Var *,
	void **, Narray *, Widget *);

#endif /* DV_WIDGET_MENUPULLDOWN_H */

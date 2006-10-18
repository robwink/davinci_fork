/* widget_menupopup.h
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

#ifndef DV_WIDGET_MENUPOPUP_H
#define DV_WIDGET_MENUPOPUP_H

#include <Xm/RowColumn.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int				gui_isMenuPopup(const char *);
WidgetClass		gui_getMenuPopupClass(void);
Widget			gui_initMenuPopup (const char *, WidgetClass, Widget, Var *,
	void **, Narray *, Widget *);

#endif /* DV_WIDGET_MENUPOPUP_H */

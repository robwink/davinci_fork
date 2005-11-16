/* widget_radiobox.h
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

#ifndef DV_WIDGET_RADIOBOX_H
#define DV_WIDGET_RADIOBOX_H

#include <Xm/RowColumn.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int		gui_isRadioBox(const char *);
Widget		gui_initRadioBox(const char *, WidgetClass, Widget,
				 Var *, void **, Narray *, Widget *);
WidgetClass	gui_getRadioBoxClass(void);
CallbackList	gui_getRadioBoxCallbacks(void);
Narray *	gui_getRadioBoxPublicResources(void);

#endif /* DV_WIDGET_RADIOBOX_H */

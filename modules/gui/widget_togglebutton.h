/* widget_togglebutton.h
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

#ifndef DV_WIDGET_TOGGLEBUTTON_H
#define DV_WIDGET_TOGGLEBUTTON_H

#include <Xm/ToggleB.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int		gui_isToggleButton(const char *);
WidgetClass	gui_getToggleButtonClass(void);
CallbackList	gui_getToggleButtonCallbacks(void);
Narray *	gui_getToggleButtonPublicResources(void);

#endif /* DV_WIDGET_TOGGLEBUTTON_H */

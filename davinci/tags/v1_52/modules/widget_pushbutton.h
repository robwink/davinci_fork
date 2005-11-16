/* widget_pushbutton.h
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

#ifndef DV_WIDGET_PUSHBUTTON_H
#define DV_WIDGET_PUSHBUTTON_H

#include <Xm/PushB.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int		gui_isPushButton(const char *);
WidgetClass	gui_getPushButtonClass(void);
CallbackList	gui_getPushButtonCallbacks(void);
Narray *	gui_getPushButtonPublicResources(void);

#endif /* DV_WIDGET_PUSHBUTTON_H */

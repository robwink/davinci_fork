/* widget_cascadebutton.h
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

#ifndef DV_WIDGET_CASCADEBUTTON_H
#define DV_WIDGET_CASCADEBUTTON_H

#include <Xm/CascadeB.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int		gui_isCascadeButton(const char *);
WidgetClass	gui_getCascadeButtonClass(void);
CallbackList	gui_getCascadeButtonCallbacks(void);
Narray *	gui_getCascadeButtonPublicResources(void);

#endif /* DV_WIDGET_CASCADEBUTTON_H */

/* widget_arrowbutton.h
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

#ifndef DV_WIDGET_ARROWBUTTON_H
#define DV_WIDGET_ARROWBUTTON_H

#include <Xm/ArrowB.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int		gui_isArrowButton(const char *);
WidgetClass	gui_getArrowButtonClass(void);
CallbackList	gui_getArrowButtonCallbacks(void);
Narray *	gui_getArrowButtonPublicResources(void);

#endif /* DV_WIDGET_ARROWBUTTON_H */

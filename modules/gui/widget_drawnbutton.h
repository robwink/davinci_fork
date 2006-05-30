/* widget_drawnbutton.h
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

#ifndef DV_WIDGET_DRAWNBUTTON_H
#define DV_WIDGET_DRAWNBUTTON_H

#include <Xm/DrawnB.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int		gui_isDrawnButton(const char *);
WidgetClass	gui_getDrawnButtonClass(void);
CallbackList	gui_getDrawnButtonCallbacks(void);

#endif /* DV_WIDGET_DRAWNBUTTON_H */

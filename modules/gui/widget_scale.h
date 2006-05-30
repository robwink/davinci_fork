/* widget_scale.h
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

#ifndef DV_WIDGET_SCALE_H
#define DV_WIDGET_SCALE_H

#include <Xm/Scale.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int		gui_isScale(const char *);
WidgetClass	gui_getScaleClass(void);
CallbackList	gui_getScaleCallbacks(void);
Narray *	gui_getScalePublicResources(void);

#endif /* DV_WIDGET_SCALE_H */

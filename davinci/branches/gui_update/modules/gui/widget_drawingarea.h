/* widget_drawingarea.h
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

#ifndef DV_WIDGET_DRAWINGAREA_H
#define DV_WIDGET_DRAWINGAREA_H

#include <Xm/DrawingA.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int				gui_isDrawingArea(const char *);
WidgetClass		gui_getDrawingAreaClass(void);
CallbackList	gui_getDrawingAreaCallbacks(void);

#endif /* DV_WIDGET_DRAWINGAREA_H */

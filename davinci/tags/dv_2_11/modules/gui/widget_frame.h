/* widget_frame.h
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

#ifndef DV_WIDGET_FRAME_H
#define DV_WIDGET_FRAME_H

#include <Xm/Frame.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int		gui_isFrame(const char *);
WidgetClass	gui_getFrameClass(void);
Narray *	gui_getFramePublicResources(void);

#endif /* DV_WIDGET_FRAME_H */

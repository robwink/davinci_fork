/* widget_text.h
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

#ifndef DV_WIDGET_TEXT_H
#define DV_WIDGET_TEXT_H

#include <Xm/Text.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int		gui_isText(const char *);
WidgetClass	gui_getTextClass(void);
CallbackList	gui_getTextCallbacks(void);

#endif /* DV_WIDGET_TEXT_H */

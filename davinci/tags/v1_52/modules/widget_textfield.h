/* widget_textfield.h
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

#ifndef DV_WIDGET_TEXTFIELD_H
#define DV_WIDGET_TEXTFIELD_H

#include <Xm/TextF.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int		gui_isTextField(const char *);
WidgetClass	gui_getTextFieldClass(void);
CallbackList	gui_getTextFieldCallbacks(void);
Narray *	gui_getTextFieldPublicResources(void);

#endif /* DV_WIDGET_TEXTFIELD_H */

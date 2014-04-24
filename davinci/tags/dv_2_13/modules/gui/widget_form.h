/* widget_form.h
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

#ifndef DV_WIDGET_FORM_H
#define DV_WIDGET_FORM_H

#include <Xm/Form.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int		gui_isForm(const char *);
WidgetClass	gui_getFormClass(void);

#endif /* DV_WIDGET_FORM_H */

/* widget_text.h
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

#ifndef DV_WIDGET_COMBOBOX_H
#define DV_WIDGET_COMBOBOX_H

#include <Xm/Xm.h>			/* necessary addition for solaris.  */
#include <Xm/ComboBox.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int				gui_isComboBox(const char *);
WidgetClass		gui_getComboBoxClass(void);
CallbackList	gui_getComboBoxCallbacks(void);

#endif /* DV_WIDGET_COMBOBOX_H */

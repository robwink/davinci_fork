/* widget_separator.h
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

#ifndef DV_WIDGET_SEPARATOR_H
#define DV_WIDGET_SEPARATOR_H

#include <Xm/Separator.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int		gui_isSeparator(const char *);
WidgetClass	gui_getSeparatorClass(void);
Narray *	gui_getSeparatorPublicResources(void);

#endif /* DV_WIDGET_SEPARATOR_H */

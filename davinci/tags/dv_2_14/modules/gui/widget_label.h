/* widget_label.h
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

#ifndef DV_WIDGET_LABEL_H
#define DV_WIDGET_LABEL_H

#include <Xm/Label.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int		gui_isLabel(const char *);
WidgetClass	gui_getLabelClass(void);
Narray *	gui_getLabelPublicResources(void);

#endif /* DV_WIDGET_LABEL_H */

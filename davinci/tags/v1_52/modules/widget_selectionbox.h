/* widget_selectionbox.h
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

#ifndef DV_WIDGET_SELECTIONBOX_H
#define DV_WIDGET_SELECTIONBOX_H

#include <Xm/SelectioB.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int		gui_isSelectionBox(const char *);
WidgetClass	gui_getSelectionBoxClass(void);
CallbackList	gui_getSelectionBoxCallbacks(void);
void		gui_getSelectionBoxPseudoResources(Widget, Var *);
void		gui_setSelectionBoxPseudoResources(Widget, Var *, Narray *);
Narray *	gui_getSelectionBoxPublicResources(void);

#endif /* DV_WIDGET_SELECTIONBOX_H */

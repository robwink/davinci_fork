/* widget_list.h
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

#ifndef DV_WIDGET_LIST_H
#define DV_WIDGET_LIST_H

#include <Xm/List.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int		gui_isList(const char *);
WidgetClass	gui_getListClass(void);
CallbackList	gui_getListCallbacks(void);
void		gui_getListPseudoResources(Widget, Var *);
void		gui_setListPseudoResources(Widget, Var *, Narray *);
Narray *	gui_getListPublicResources(void);

#endif /* DV_WIDGET_LIST_H */

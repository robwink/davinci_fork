/* widget_scrolledlist.h
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

#ifndef DV_WIDGET_SCROLLEDLIST_H
#define DV_WIDGET_SCROLLEDLIST_H

#include <Xm/List.h>
#include <Xm/ScrolledW.h>
#include <Xm/ScrollBar.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int		gui_isScrolledList(const char *);
Widget		gui_initScrolledList(const char *, WidgetClass, Widget,
				     Var *, void **, Narray *, Widget *);
WidgetClass	gui_getScrolledListClass(void);
CallbackList	gui_getScrolledListCallbacks(void);
void		gui_getScrolledListPseudoResources(Widget, Var *);
void		gui_setScrolledListPseudoResources(Widget, Var *, Narray *);
Narray *	gui_getScrolledListPublicResources(void);

#endif /* DV_WIDGET_SCROLLEDLIST_H */

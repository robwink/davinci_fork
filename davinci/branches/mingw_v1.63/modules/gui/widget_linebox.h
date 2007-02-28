/* widget_linebox.h
 *
 * Copyright 2003
 * Mars Space Flight Facility
 * Department of Geological Sciences
 * Arizona State University
 *
 * Eric Engle <eric.engle@asu.edu>
 *
 */

#ifndef DV_WIDGET_LINEBOX_H
#define DV_WIDGET_LINEBOX_H

#include "gui.h"

/* Prototypes used in gui.c */
int				gui_isLineBox(const char *);
WidgetClass		gui_getLineBoxClass();
CallbackList	gui_getLineBoxCallbacks();
Widget			gui_initLineBoxWidget (
						const char *dvName, WidgetClass class, Widget parent,
						Var *dvResources, void **optData,
						Narray *publicResources, Widget *outerWidget);
void			gui_getLineBoxPseudoResources(Widget, Var *);
void			gui_setLineBoxPseudoResources(Widget, Var *, Narray *);
Narray *		gui_getLineBoxPublicResources();

#endif /* DV_WIDGET_LINEBOX_H */

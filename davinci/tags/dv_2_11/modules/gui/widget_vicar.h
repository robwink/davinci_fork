/* vicar_widget.h
 *
 * Copyright 2003
 * Mars Space Flight Facility
 * Department of Geological Sciences
 * Arizona State University
 *
 * Jim Stewart <Jim.Stewart@asu.edu>
 *
 * Portions of this code are based on test_iw.c, part of the VICAR
 * software package.
 *
 */

#ifdef HAVE_LIBVICAR

#ifndef DV_WIDGET_VICAR_H
#define DV_WIDGET_VICAR_H

#include "../vicar/XvicImage.h" /* FIX: this is bad */
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int		gui_isVicar(const char *);
Widget		gui_initVicar(const char *, WidgetClass, Widget,
			      Var *, void **, Narray *, Widget *);
void		gui_destroyVicar(Widget, void *);
WidgetClass	gui_getVicarClass(void);
CallbackList	gui_getVicarCallbacks(void);
void		gui_getVicarPseudoResources(Widget, Var *);
void		gui_setVicarPseudoResources(Widget, Var *, Narray *);
Narray *	gui_getVicarPublicResources(void);

/* Data types. */

#endif /* DV_WIDGET_VICAR_H */
#endif /* HAVE_LIBVICAR */

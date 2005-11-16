/* widget_transientshell.h
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

#ifndef DV_WIDGET_TRANSIENTSHELL_H
#define DV_WIDGET_TRANSIENTSHELL_H

#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int		gui_isTransientShell(const char *);
WidgetClass	gui_getTransientShellClass(void);
Widget		gui_initTransientShell(const char *, WidgetClass, Widget,
				      Var *, void **, Narray *, Widget *);
Narray *	gui_getTransientShellPublicResources(void);

#endif /* DV_WIDGET_TRANSIENTSHELL_H */

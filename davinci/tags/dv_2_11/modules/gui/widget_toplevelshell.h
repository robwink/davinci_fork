/* widget_toplevelshell.h
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

#ifndef DV_WIDGET_TOPLEVELSHELL_H
#define DV_WIDGET_TOPLEVELSHELL_H

#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int		gui_isTopLevelShell(const char *);
WidgetClass	gui_getTopLevelShellClass(void);
Widget		gui_initTopLevelShell(const char *, WidgetClass, Widget,
				      Var *, void **, Narray *, Widget *);
Narray *	gui_getTopLevelShellPublicResources(void);

#endif /* DV_WIDGET_TOPLEVELSHELL_H */

/* widget_command.h
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

#ifndef DV_WIDGET_COMMAND_H
#define DV_WIDGET_COMMAND_H

#include <Xm/Command.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int		gui_isCommand(const char *);
WidgetClass	gui_getCommandClass(void);
CallbackList	gui_getCommandCallbacks(void);
void		gui_getCommandPseudoResources(Widget, Var *);
void		gui_setCommandPseudoResources(Widget, Var *, Narray *);
Narray *	gui_getCommandPublicResources(void);

#endif /* DV_WIDGET_COMMAND_H */

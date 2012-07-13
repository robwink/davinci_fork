/* widget_fileselectionbox.h
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

#ifndef DV_WIDGET_FILESELECTIONBOX_H
#define DV_WIDGET_FILESELECTIONBOX_H

#include <Xm/FileSB.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int		gui_isFileSelectionBox(const char *);
WidgetClass	gui_getFileSelectionBoxClass(void);
CallbackList	gui_getFileSelectionBoxCallbacks(void);
void		gui_getFileSelectionBoxPseudoResources(Widget, Var *);
void		gui_setFileSelectionBoxPseudoResources(Widget, Var *, Narray *);
Narray *	gui_getFileSelectionBoxPublicResources(void);

#endif /* DV_WIDGET_FILESELECTIONBOX_H */

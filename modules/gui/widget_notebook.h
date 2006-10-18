/* widget_notebook.h
 *
 * Davinci GUI binding for various Motif widgets.
 *
 * Copyright 2005
 * Mars Space Flight Facility
 * Department of Geological Sciences
 * Arizona State University
 *
 * Eric Engle <eric.engle@asu.edu>
 *
 */

#ifndef DV_WIDGET_NOTEBOOK_H
#define DV_WIDGET_NOTEBOOK_H

#include <Xm/Notebook.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int				gui_isNotebook(const char *);
WidgetClass		gui_getNotebookClass(void);
CallbackList	gui_getNotebookCallbacks(void);
void			gui_setNotebookPseudoResources(Widget, Var *, Narray *);
void			gui_getNotebookPseudoResources(const Widget, Var *);

#endif /* DV_WIDGET_NOTEBOOK_H */

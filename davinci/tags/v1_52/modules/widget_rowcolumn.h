/* widget_rowcolumn.h
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

#ifndef DV_WIDGET_ROWCOLUMN_H
#define DV_WIDGET_ROWCOLUMN_H

#include <Xm/RowColumn.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int		gui_isRowColumn(const char *);
WidgetClass	gui_getRowColumnClass(void);
CallbackList	gui_getRowColumnCallbacks(void);
Narray *	gui_getRowColumnPublicResources(void);

#endif /* DV_WIDGET_ROWCOLUMN_H */

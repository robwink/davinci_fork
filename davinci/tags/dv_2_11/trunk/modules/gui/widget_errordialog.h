/* widget_errordialog.h
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

#ifndef DV_WIDGET_ERRORDIALOG_H
#define DV_WIDGET_ERRORDIALOG_H

#include <Xm/MessageB.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int				gui_isErrorDialog(const char *);
WidgetClass		gui_getErrorDialogClass(void);
Widget			gui_initErrorDialog(const char *, WidgetClass, Widget,
					Var *, void **, Narray *);
Narray *		gui_getErrorDialogPublicResources(void);
CallbackList	gui_getErrorDialogCallbacks(void);

#endif /* DV_WIDGET_ERRORDIALOG_H */

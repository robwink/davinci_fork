/* widget_xbaematrix.h
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

#ifndef DV_WIDGET_XBAEMATRIX_H
#define DV_WIDGET_XBAEMATRIX_H

#include <Xbae/Matrix.h>
#include "gui.h"

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

int		gui_isXbaeMatrix(const char *);
WidgetClass	gui_getXbaeMatrixClass(void);
CallbackList	gui_getXbaeMatrixCallbacks(void);
Narray *	gui_getXbaeMatrixPublicResources(void);

#endif /* DV_WIDGET_XBAEMATRIX_H */

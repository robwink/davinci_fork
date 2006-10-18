/* widget_drawingarea.c
 *
 * Davinci GUI bindings for various basic Xt/Motif widgets.
 *
 * Copyright 2005
 * Mars Space Flight Facility
 * Department of Geological Sciences
 * Arizona State University
 *
 * Eric Engle <eric.engle@asu.edu>
 *
 */

/*****************************************************************************
 *
 * INCLUDES
 *
 *****************************************************************************/

#include "widget_drawingarea.h"

/*****************************************************************************
 *
 * CALLBACKS
 *
 *****************************************************************************/

static CallbackEntry drawingareaCallbacks[] = {
	{ "exposeCallback", XmNexposeCallback, gui_defaultCallback },
	{ "inputCallback", XmNinputCallback, gui_defaultCallback },
	{ "resizeCallback", XmNresizeCallback, gui_defaultCallback },
	{ NULL, NULL, NULL }
};

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

/*****************************************************************************
 *
 * FUNCTION DEFINITIONS
 *
 *****************************************************************************/

int
gui_isDrawingArea(const char *name)
{
	const char *aliases[] = { "drawingarea", "xmDrawingAreaWidgetClass", NULL };
	return gui_isDefault(aliases, name);
}

WidgetClass
gui_getDrawingAreaClass(void)
{
	return xmDrawingAreaWidgetClass;
}

CallbackList
gui_getDrawingAreaCallbacks(void)
{
	return drawingareaCallbacks;
}



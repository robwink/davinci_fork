/* widget_mainwindow.c
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

#include "widget_mainwindow.h"

/*****************************************************************************
 *
 * CALLBACKS
 *
 *****************************************************************************/

/*
static CallbackEntry mainwindowCallbacks[] = {
	{ "pageChangedCallback", XmNpageChangedCallback, gui_defaultCallback },
	{ NULL, NULL, NULL }
};
*/

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
gui_isMainWindow(const char *name)
{
	const char *aliases[] = { "mainwindow", "xmMainWindowWidgetClass", NULL };
	return gui_isDefault(aliases, name);
}

WidgetClass
gui_getMainWindowClass(void)
{
	return xmMainWindowWidgetClass;
}

/*
CallbackList
gui_getMainWindowCallbacks(void)
{
	return mainwindowCallbacks;
}
*/


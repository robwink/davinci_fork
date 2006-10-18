/* widget_menupulldown.c
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

#include "widget_menupulldown.h"

/*****************************************************************************
 *
 * CALLBACKS
 *
 *****************************************************************************/

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
gui_isMenuPulldown(const char *name)
{
	const char *aliases[] = { "menupulldown", "xmMenuPulldownWidgetClass", NULL };
	return gui_isDefault(aliases, name);
}

WidgetClass
gui_getMenuPulldownClass(void)
{
	// return xmMenuPulldownWidgetClass;
	return xmRowColumnWidgetClass;
}

Widget
gui_initMenuPulldown (const char *dvName, WidgetClass class, Widget parent,
	Var *dvResources, void **instanceData,
	Narray *publicResources, Widget *outerWidget)
{
	/* the Pulldown crashes the app when created without a name */
	if (dvName == NULL) {
		dvName = "_DVforceAvoidMenuPulldownCrashOnNoName";
	}
	return XmCreatePulldownMenu(parent, dvName, NULL, 0);
}


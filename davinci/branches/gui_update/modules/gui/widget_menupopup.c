/* widget_menupopup.c
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

#include "widget_menupopup.h"

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
gui_isMenuPopup(const char *name)
{
	const char *aliases[] = { "menupopup", "xmMenuPopupWidgetClass", NULL };
	return gui_isDefault(aliases, name);
}

WidgetClass
gui_getMenuPopupClass(void)
{
	// return xmMenuPopupWidgetClass;
	return xmRowColumnWidgetClass;
}

Widget
gui_initMenuPopup (const char *dvName, WidgetClass class, Widget parent,
	Var *dvResources, void **instanceData,
	Narray *publicResources, Widget *outerWidget)
{
	return XmCreatePopupMenu(parent, dvName, NULL, 0);
}


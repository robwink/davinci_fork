/* widget_menubar.c
 *
 * Davinci GUI bindings for various basic Xt/Motif widgets.
 *
 * Copyright 2003
 * Mars Space Flight Facility
 * Department of Geological Sciences
 * Arizona State University
 *
 * Jim Stewart <Jim.Stewart@asu.edu>
 *
 */

/*****************************************************************************
 *
 * INCLUDES
 *
 *****************************************************************************/

#include "widget_menubar.h"

/*****************************************************************************
 *
 * CALLBACKS
 *
 *****************************************************************************/

/* No callbacks. */

/*****************************************************************************
 *
 * FUNCTION DEFINITIONS
 *
 *****************************************************************************/

int
gui_isMenuBar(const char *name)
{
  const char *aliases[] = { "menubar", "xmMenuBarWidgetClass", NULL };
  return gui_isDefault(aliases, name);
}

Widget
gui_initMenuBar(const char *dvName, WidgetClass class, Widget parent,
		void **optData)
{
  /* FIX:  return XmCreateMenuBar(parent, dvName, NULL, 0); */
  return NULL;
}

WidgetClass
gui_getMenuBarClass(void)
{
  /* FIX: return xmMenuBarWidgetClass; */
  return NULL;
}


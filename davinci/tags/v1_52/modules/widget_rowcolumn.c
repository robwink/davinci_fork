/* widget_rowcolumn.c
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

#include "widget_rowcolumn.h"

/*****************************************************************************
 *
 * RESOURCE LISTS
 *
 *****************************************************************************/

static const char *rowColumnPublicResources[] = {
  "adjustLast", "numColumns", "orientation", "packing"
};

/*****************************************************************************
 *
 * CALLBACKS
 *
 *****************************************************************************/

/* CONFIRMED, COMPLETE */

static CallbackEntry rowColumnCallbacks[] = {
  {
    "entry",
    XmNentryCallback,
    gui_defaultCallback
  },
  {
    "map",
    XmNmapCallback,
    gui_defaultCallback
  },
  {
    "tearOffMenuActivate",
    XmNtearOffMenuActivateCallback,
    gui_defaultCallback
  },
  {
    "tearOffMenuDeactivate",
    XmNtearOffMenuDeactivateCallback,
    gui_defaultCallback
  },
  {
    "unmap",
    XmNunmapCallback,
    gui_defaultCallback
  },
  { NULL, NULL, NULL }
};

/*****************************************************************************
 *
 * FUNCTION DEFINITIONS
 *
 *****************************************************************************/

int
gui_isRowColumn(const char *name)
{
  const char *aliases[] = { "rowcolumn", "xmRowColumnWidgetClass", NULL };
  return gui_isDefault(aliases, name);
}

WidgetClass
gui_getRowColumnClass(void)
{
  return xmRowColumnWidgetClass;
}

CallbackList
gui_getRowColumnCallbacks(void)
{
  return rowColumnCallbacks;
}

Narray *
gui_getRowColumnPublicResources()
{

  Narray	*resList;
  int		i, num;

#if DEBUG
  fprintf(stderr, "DEBUG: gui_getRowColumnPublicResources()\n");
#endif

  num = sizeof(rowColumnPublicResources) / sizeof(rowColumnPublicResources[0]);
  resList = Narray_create(num);
  for (i = 0; i < num; i++) {
    Narray_add(resList, (char *) rowColumnPublicResources[i], NULL);
  }

  return resList;

}

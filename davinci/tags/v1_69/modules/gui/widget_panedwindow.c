/* widget_panedwindow.c
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

#include "widget_panedwindow.h"

/*****************************************************************************
 *
 * CALLBACKS
 *
 *****************************************************************************/

/* No callbacks. */

/*****************************************************************************
 *
 * RESOURCES
 *
 *****************************************************************************/

static const char *panedWindowPublicResources[] = {
  "separatorOn",
};

/*****************************************************************************
 *
 * FUNCTION DEFINITIONS
 *
 *****************************************************************************/

int
gui_isPanedWindow(const char *name)
{
  const char *aliases[] = { "panedWindow", "xmPanedWindowWidgetClass", NULL };
  return gui_isDefault(aliases, name);
}

WidgetClass
gui_getPanedWindowClass(void)
{
  return xmPanedWindowWidgetClass;
}

Narray *
gui_getPanedWindowPublicResources()
{

  Narray	*resList;
  int		i, num;

  num = sizeof(panedWindowPublicResources) / sizeof(panedWindowPublicResources[0]);
  resList = Narray_create(num);
  for (i = 0; i < num; i++) {
    Narray_add(resList, (char *) panedWindowPublicResources[i], NULL);
  }

  return resList;

}

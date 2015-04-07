/* widget_scrolledwindow.c
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

#include "widget_scrolledwindow.h"

/*****************************************************************************
 *
 * CALLBACKS
 *
 *****************************************************************************/

static CallbackEntry scrolledWindowCallbacks[] = {
  { "traverseObscured",	XmNtraverseObscuredCallback,	gui_defaultCallback },
  { NULL,		NULL,				NULL		    }
};

/*****************************************************************************
 *
 * RESOURCES
 *
 *****************************************************************************/

static const char *scrolledWindowPublicResources[] = {
  "scrollBarDisplayPolicy", "scrollBarPlacement",
};

/*****************************************************************************
 *
 * FUNCTION DEFINITIONS
 *
 *****************************************************************************/

int
gui_isScrolledWindow(const char *name)
{
  const char *aliases[] = { "scrolledwindow", "xmScrolledWindowWidgetClass", NULL };
  return gui_isDefault(aliases, name);
}

WidgetClass
gui_getScrolledWindowClass(void)
{
  return xmScrolledWindowWidgetClass;
}

CallbackList
gui_getScrolledWindowCallbacks(void)
{
  return scrolledWindowCallbacks;
}

Narray *
gui_getScrolledWindowPublicResources()
{

  Narray	*resList;
  int		i, num;

  num = sizeof(scrolledWindowPublicResources) / sizeof(scrolledWindowPublicResources[0]);
  resList = Narray_create(num);
  for (i = 0; i < num; i++) {
    Narray_add(resList, (char *) scrolledWindowPublicResources[i], NULL);
  }

  return resList;

}

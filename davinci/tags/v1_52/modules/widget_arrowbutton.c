/* widget_arrowbutton.c
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

#include "widget_arrowbutton.h"

/*****************************************************************************
 *
 * CALLBACKS
 *
 *****************************************************************************/

static CallbackEntry arrowButtonCallbacks[] = {
  {
    "arm",
    XmNarmCallback,
    gui_defaultCallback
  },
  {
    "activate",
    XmNactivateCallback,
    gui_defaultCallback
  },
  {
    "disarm",
    XmNdisarmCallback,
    gui_defaultCallback
  },
  { NULL, NULL, NULL }
};

/*****************************************************************************
 *
 * RESOURCES
 *
 *****************************************************************************/

static const char *arrowButtonPublicResources[] = {
  "arrowDirection",
};

/*****************************************************************************
 *
 * FUNCTION DEFINITIONS
 *
 *****************************************************************************/

int
gui_isArrowButton(const char *name)
{
  const char *aliases[] = { "arrowbutton", "xmArrowButtonWidgetClass", NULL };
  return gui_isDefault(aliases, name);
}

WidgetClass
gui_getArrowButtonClass(void)
{
  return xmArrowButtonWidgetClass;
}

CallbackList
gui_getArrowButtonCallbacks(void)
{
  return arrowButtonCallbacks;
}

Narray *
gui_getArrowButtonPublicResources()
{

  Narray	*resArrowButton;
  int		i, num;

#if DEBUG
  fprintf(stderr, "DEBUG: gui_getArrowButtonPublicResources()\n");
#endif

  num = sizeof(arrowButtonPublicResources) / sizeof(arrowButtonPublicResources[0]);
  resArrowButton = Narray_create(num);
  for (i = 0; i < num; i++) {
    Narray_add(resArrowButton, (char *) arrowButtonPublicResources[i], NULL);
  }

  return resArrowButton;

}

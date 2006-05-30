/* widget_togglebutton.c
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

#include "widget_togglebutton.h"

/*****************************************************************************
 *
 * CALLBACKS
 *
 *****************************************************************************/

static CallbackEntry toggleButtonCallbacks[] = {
  {
    "arm",
    XmNarmCallback,
    gui_defaultCallback
  },
  {
    "disarm",
    XmNdisarmCallback,
    gui_defaultCallback
  },
  {
    "valueChanged",
    XmNvalueChangedCallback,
    gui_defaultCallback
  },
  { NULL, NULL, NULL }
};

/*****************************************************************************
 *
 * RESOURCES
 *
 *****************************************************************************/

static const char *toggleButtonPublicResources[] = {
  "set", "labelString",
};

/*****************************************************************************
 *
 * FUNCTION DEFINITIONS
 *
 *****************************************************************************/

int
gui_isToggleButton(const char *name)
{
  const char *aliases[] = { "togglebutton", "xmToggleButtonWidgetClass", NULL };
  return gui_isDefault(aliases, name);
}

WidgetClass
gui_getToggleButtonClass(void)
{
  return xmToggleButtonWidgetClass;
}

CallbackList
gui_getToggleButtonCallbacks(void)
{
  return toggleButtonCallbacks;
}

Narray *
gui_getToggleButtonPublicResources()
{

  Narray	*resList;
  int		i, num;

  num = sizeof(toggleButtonPublicResources) / sizeof(toggleButtonPublicResources[0]);
  resList = Narray_create(num);
  for (i = 0; i < num; i++) {
    Narray_add(resList, (char *) toggleButtonPublicResources[i], NULL);
  }

  return resList;

}

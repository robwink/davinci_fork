/* widget_pushbutton.c
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

#include "widget_pushbutton.h"

/*****************************************************************************
 *
 * CALLBACKS
 *
 *****************************************************************************/

static CallbackEntry pushButtonCallbacks[] = {
  {
    "activate",
    XmNactivateCallback,
    gui_defaultCallback
  },
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
  { NULL, NULL, NULL }
};

/*****************************************************************************
 *
 * RESOURCES
 *
 *****************************************************************************/

static const char *pushButtonPublicResources[] = {
  "labelString",
};

/*****************************************************************************
 *
 * FUNCTION DEFINITIONS
 *
 *****************************************************************************/

int
gui_isPushButton(const char *name)
{
  const char *aliases[] = { "pushbutton", "xmPushButtonWidgetClass", NULL };
  return gui_isDefault(aliases, name);
}

WidgetClass
gui_getPushButtonClass(void)
{
  return xmPushButtonWidgetClass;
}

CallbackList
gui_getPushButtonCallbacks(void)
{
  return pushButtonCallbacks;
}

Narray *
gui_getPushButtonPublicResources()
{

  Narray	*resList;
  int		i, num;

  num = sizeof(pushButtonPublicResources) / sizeof(pushButtonPublicResources[0]);
  resList = Narray_create(num);
  for (i = 0; i < num; i++) {
    Narray_add(resList, (char *) pushButtonPublicResources[i], NULL);
  }

  return resList;

}

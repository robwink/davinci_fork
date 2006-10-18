/* Widget_cascadebutton.c
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

#include "widget_cascadebutton.h"

/*****************************************************************************
 *
 * CALLBACKS
 *
 *****************************************************************************/

static CallbackEntry cascadeButtonCallbacks[] = {
  {
    "activate",
    XmNactivateCallback,
    gui_defaultCallback
  },
  {
    "cascading",
    XmNcascadingCallback,
    gui_defaultCallback
  },
  { NULL, NULL, NULL }
};

/*****************************************************************************
 *
 * RESOURCES
 *
 *****************************************************************************/

static const char *cascadeButtonPublicResources[] = {
  "subMenuId",
};

/*****************************************************************************
 *
 * FUNCTION DEFINITIONS
 *
 *****************************************************************************/

int
gui_isCascadeButton(const char *name)
{
  const char *aliases[] = { "cascadebutton", "xmCascadeButtonWidgetClass", NULL };
  return gui_isDefault(aliases, name);
}

WidgetClass
gui_getCascadeButtonClass(void)
{
  return xmCascadeButtonWidgetClass;
}

CallbackList
gui_getCascadeButtonCallbacks(void)
{
  return cascadeButtonCallbacks;
}

Narray *
gui_getCascadeButtonPublicResources()
{

  Narray	*resCascadeButton;
  int		i, num;

#if DEBUG
  fprintf(stderr, "DEBUG: gui_getCascadeButtonPublicResources()\n");
#endif

  num = sizeof(cascadeButtonPublicResources) / sizeof(cascadeButtonPublicResources[0]);
  resCascadeButton = Narray_create(num);
  for (i = 0; i < num; i++) {
    Narray_add(resCascadeButton, (char *) cascadeButtonPublicResources[i], NULL);
  }

  return resCascadeButton;

}

/* widget_separator.c
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

#include "widget_separator.h"

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

static const char *separatorPublicResources[] = {
  "orientation", "separatorType",
};

/*****************************************************************************
 *
 * FUNCTION DEFINITIONS
 *
 *****************************************************************************/

int
gui_isSeparator(const char *name)
{
  const char *aliases[] = { "separator", "xmSeparatorWidgetClass", NULL };
  return gui_isDefault(aliases, name);
}

WidgetClass
gui_getSeparatorClass(void)
{
  return xmSeparatorWidgetClass;
}

Narray *
gui_getSeparatorPublicResources()
{

  Narray	*resList;
  int		i, num;

  num = sizeof(separatorPublicResources) / sizeof(separatorPublicResources[0]);
  resList = Narray_create(num);
  for (i = 0; i < num; i++) {
    Narray_add(resList, (char *) separatorPublicResources[i], NULL);
  }

  return resList;

}

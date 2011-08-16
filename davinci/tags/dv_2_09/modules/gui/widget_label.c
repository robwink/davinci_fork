/* widget_label.c
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

#include "widget_label.h"

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

static const char *labelPublicResources[] = {
  "labelString", "alignment",
};

/*****************************************************************************
 *
 * FUNCTION DEFINITIONS
 *
 *****************************************************************************/

int
gui_isLabel(const char *name)
{
  const char *aliases[] = { "label", "xmLabelWidgetClass", NULL };
  return gui_isDefault(aliases, name);
}

WidgetClass
gui_getLabelClass(void)
{
  return xmLabelWidgetClass;
}

Narray *
gui_getLabelPublicResources()
{

  Narray	*resList;
  int		i, num;

  num = sizeof(labelPublicResources) / sizeof(labelPublicResources[0]);
  resList = Narray_create(num);
  for (i = 0; i < num; i++) {
    Narray_add(resList, (char *) labelPublicResources[i], NULL);
  }

  return resList;

}

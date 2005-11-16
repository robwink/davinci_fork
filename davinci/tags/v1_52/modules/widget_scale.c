/* widget_scale.c
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

#include "widget_scale.h"

/*****************************************************************************
 *
 * CALLBACKS
 *
 *****************************************************************************/

static CallbackEntry scaleCallbacks[] = {
  { "drag",		XmNdragCallback,		gui_defaultCallback },
  { "valueChanged",	XmNvalueChangedCallback,	gui_defaultCallback },
  { NULL,		NULL,				NULL 		    }
};

/*****************************************************************************
 *
 * RESOURCES
 *
 *****************************************************************************/

static const char *scalePublicResources[] = {
  "decimalPoints", "maximum", "minimum", "orientation", "processingDirection",
  "showValue", "titleString", "value",
};

/*****************************************************************************
 *
 * FUNCTION DEFINITIONS
 *
 *****************************************************************************/

int
gui_isScale(const char *name)
{
  const char *aliases[] = { "scale", "xmScaleWidgetClass", NULL };
  return gui_isDefault(aliases, name);
}

WidgetClass
gui_getScaleClass(void)
{
  return xmScaleWidgetClass;
}

CallbackList
gui_getScaleCallbacks(void)
{
  return scaleCallbacks;
}

Narray *
gui_getScalePublicResources()
{

  Narray	*resList;
  int		i, num;

  num = sizeof(scalePublicResources) / sizeof(scalePublicResources[0]);
  resList = Narray_create(num);
  for (i = 0; i < num; i++) {
    Narray_add(resList, (char *) scalePublicResources[i], NULL);
  }

  return resList;

}

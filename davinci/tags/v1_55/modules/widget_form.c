/* widget_form.c
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

#include "widget_form.h"

/*****************************************************************************
 *
 * FUNCTION DEFINITIONS
 *
 *****************************************************************************/

int
gui_isForm(const char *name)
{
  const char *aliases[] = { "form", "xmFormWidgetClass", NULL };
  return gui_isDefault(aliases, name);
}

WidgetClass
gui_getFormClass(void)
{
  return xmFormWidgetClass;
}

/* widget_drawnbutton.c
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

#include "widget_drawnbutton.h"

/*****************************************************************************
 *
 * CALLBACKS
 *
 *****************************************************************************/

/* CONFIRMED, COMPLETE */

static CallbackEntry drawnButtonCallbacks[] = {
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
  {
    "expose",
    XmNexposeCallback,
    gui_defaultCallback
  },
  {
    "resize",
    XmNresizeCallback,
    gui_defaultCallback
  },
  { NULL, NULL, NULL }
};

/*****************************************************************************
 *
 * FUNCTION DEFINITIONS
 *
 *****************************************************************************/

int
gui_isDrawnButton(const char *name)
{
  const char *aliases[] = { "drawnbutton", "xmDrawnButtonWidgetClass", NULL };
  return gui_isDefault(aliases, name);
}

WidgetClass
gui_getDrawnButtonClass(void)
{
  return xmDrawnButtonWidgetClass;
}

CallbackList
gui_getDrawnButtonCallbacks(void)
{
  return drawnButtonCallbacks;
}

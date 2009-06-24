/* widget_scrollbar.c
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

#include "widget_scrollbar.h"

/*****************************************************************************
 *
 * CALLBACKS
 *
 *****************************************************************************/

static CallbackEntry scrollBarCallbacks[] = {
  {
    "decrement",
    XmNdecrementCallback,
    gui_defaultCallback
  },
  {
    "increment",
    XmNincrementCallback,
    gui_defaultCallback
  },
  {
    "pageDecrement",
    XmNpageDecrementCallback,
    gui_defaultCallback
  },
  {
    "pageIncrement",
    XmNpageIncrementCallback,
    gui_defaultCallback
  },
  {
    "drag",
    XmNdragCallback,
    gui_defaultCallback
  },
  {
    "toBottom",
    XmNtoBottomCallback,
    gui_defaultCallback
  },
  {
    "toTop",
    XmNtoTopCallback,
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
 * FUNCTION DEFINITIONS
 *
 *****************************************************************************/

int
gui_isScrollBar(const char *name)
{
  const char *aliases[] = { "scrollbar", "xmScrollBarWidgetClass", NULL };
  return gui_isDefault(aliases, name);
}

WidgetClass
gui_getScrollBarClass(void)
{
  return xmScrollBarWidgetClass;
}

CallbackList
gui_getScrollBarCallbacks(void)
{
  return scrollBarCallbacks;
}

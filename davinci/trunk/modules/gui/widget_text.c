/* widget_text.c
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

#include "widget_text.h"

/*****************************************************************************
 *
 * CALLBACKS
 *
 *****************************************************************************/

static CallbackEntry textCallbacks[] = {
  {
    "activate",
    XmNactivateCallback,
    gui_defaultCallback
  },
  {
    "focus",
    XmNfocusCallback,
    gui_defaultCallback
  },
  {
    "losingFocus",
    XmNlosingFocusCallback,
    gui_defaultCallback
  },
  {
    "modifyVerify",
    XmNmodifyVerifyCallback,
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
gui_isText(const char *name)
{
  const char *aliases[] = { "text", "xmTextWidgetClass", NULL };
  return gui_isDefault(aliases, name);
}

WidgetClass
gui_getTextClass(void)
{
  return xmTextWidgetClass;
}

CallbackList
gui_getTextCallbacks(void)
{
  return textCallbacks;
}

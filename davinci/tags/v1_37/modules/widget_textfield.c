/* widget_textfield.c
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

#include "widget_textfield.h"

/*****************************************************************************
 *
 * CALLBACKS
 *
 *****************************************************************************/

static CallbackEntry textFieldCallbacks[] = {
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
 * RESOURCES
 *
 *****************************************************************************/

static const char *textFieldPublicResources[] = {
  "value", "cursorPosition", "columns", "editable",
};

/*****************************************************************************
 *
 * FUNCTION DEFINITIONS
 *
 *****************************************************************************/

int
gui_isTextField(const char *name)
{
  const char *aliases[] = { "textfield", "xmTextFieldWidgetClass", NULL };
  return gui_isDefault(aliases, name);
}

WidgetClass
gui_getTextFieldClass(void)
{
  return xmTextFieldWidgetClass;
}

CallbackList
gui_getTextFieldCallbacks(void)
{
  return textFieldCallbacks;
}

Narray *
gui_getTextFieldPublicResources()
{

  Narray	*resList;
  int		i, num;

  num = sizeof(textFieldPublicResources) / sizeof(textFieldPublicResources[0]);
  resList = Narray_create(num);
  for (i = 0; i < num; i++) {
    Narray_add(resList, (char *) textFieldPublicResources[i], NULL);
  }

  return resList;

}

/* widget_radiobox.c
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

#include "widget_radiobox.h"

/*****************************************************************************
 *
 * RESOURCE LISTS
 *
 *****************************************************************************/

static const char *radioBoxPublicResources[] = {
  "menuHistory", "adjustLast", "numColumns", "orientation",
};

/*****************************************************************************
 *
 * CALLBACKS
 *
 *****************************************************************************/

/* CONFIRMED, COMPLETE */

static CallbackEntry radioBoxCallbacks[] = {
  {
    "entry",
    XmNentryCallback,
    gui_defaultCallback
  },
  {
    "map",
    XmNmapCallback,
    gui_defaultCallback
  },
  {
    "unmap",
    XmNunmapCallback,
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
gui_isRadioBox(const char *name)
{
  const char *aliases[] = { "radioBox", NULL };
  return gui_isDefault(aliases, name);
}

WidgetClass
gui_getRadioBoxClass(void)
{
  return xmRowColumnWidgetClass;
}

CallbackList
gui_getRadioBoxCallbacks(void)
{
  return radioBoxCallbacks;
}

Widget
gui_initRadioBox(const char *dvName, WidgetClass class, Widget parent,
		 Var *dvResources, void **instanceData,
		 Narray *publicResources, Widget *outerWidget)
{

  Widget		widget;
  String		widgetName;
  Arg			xtArgs[DV_MAX_XT_ARGS];
  Cardinal		xtArgCount;
  FreeStackListEntry	freeStack;

#if DEBUG
  fprintf(stderr, "DEBUG: gui_initRadioBox(dvName = \"%s\", class = %ld, "
	  "parent = %ld, dvResources = %ld, instanceData = %ld)\n",
	  dvName, class, parent, dvResources, instanceData);
#endif  

  /* Parse resources, if the user supplied any. */

#if DEBUG
  fprintf(stderr, "DEBUG: parsing resources\n");
#endif  

  xtArgCount = 0;
  freeStack.head = freeStack.tail = NULL; /* FIX: free these */
  if (dvResources != NULL) {
    xtArgCount = gui_setResourceValues(NULL, widgetClass, dvResources,
			  xtArgs, &freeStack,
			  publicResources);
  }

  /* Set the screen, required for popups. */

#if DEBUG
  fprintf(stderr, "DEBUG: calling XmCreateRadioBox(parent = %ld, "
	  "name = %ld, args = %ld, argcount = %d)\n",
	  parent, dvName, xtArgs, xtArgCount);
#endif

  /* This stupid function can't cope with NULL names! */
  if (dvName == NULL) {
    widgetName = "";
  }
  else {
    widgetName = (String) dvName;
  }

  if (xtArgCount == 0) {
    widget = XmCreateRadioBox(parent, widgetName, NULL, xtArgCount);
  }
  else {
    widget = XmCreateRadioBox(parent, widgetName, xtArgs, xtArgCount);
  }

  /* Free anything in the free stack. */
  gui_freeStackFree(&freeStack);

  /* Manage the new widget since XmCreateRadioBox() doesn't do it. */
  XtManageChild(widget);

  return widget;

}

Narray *
gui_getRadioBoxPublicResources()
{

  Narray	*resList;
  int		i, num;

#if DEBUG
  fprintf(stderr, "DEBUG: gui_getRadioBoxPublicResources()\n");
#endif

  num = sizeof(radioBoxPublicResources) / sizeof(radioBoxPublicResources[0]);
  resList = Narray_create(num);
  for (i = 0; i < num; i++) {
    Narray_add(resList, (char *) radioBoxPublicResources[i], NULL);
  }

  return resList;

}

/* widget_toplevelshell.c
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

#include "widget_toplevelshell.h"

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

static const char *topLevelShellPublicResources[] = {
  "iconName", "allowShellResize",
};

/*****************************************************************************
 *
 * FUNCTION DEFINITIONS
 *
 *****************************************************************************/

int
gui_isTopLevelShell(const char *name)
{
  const char *aliases[] = { "top", "topLevelShell", "topLevelShellWidgetClass", NULL };
  return gui_isDefault(aliases, name);
}

WidgetClass
gui_getTopLevelShellClass(void)
{
  return topLevelShellWidgetClass;
}

Widget
gui_initTopLevelShell(const char *dvName, WidgetClass class, Widget parent,
		      Var *dvResources, void **instanceData,
		      Narray *publicResources, Widget *outerWidget)
{

  Widget		topShell;
  Arg			xtArgs[DV_MAX_XT_ARGS];
  Cardinal		xtArgCount;
  FreeStackListEntry	freeStack;

#if DEBUG
  fprintf(stderr, "DEBUG: gui_initTopLevelShell(dvName = \"%s\", class = %ld, "
	  "parent = %ld, instanceData = %ld)\n", dvName, class, parent,
	  instanceData);
#endif  

  /* Parse resources, if the user supplied any. */

  xtArgCount = 0;
  freeStack.head = freeStack.tail = NULL; /* FIX: free these */
  if (dvResources != NULL) {
    gui_setResourceValues(NULL, widgetClass, dvResources,
			  xtArgs, &xtArgCount, &freeStack,
			  publicResources);
  }

  /* Set the screen, required for popups. */

  XtSetArg(xtArgs[xtArgCount], XmNscreen,
	   DefaultScreenOfDisplay(XtDisplay(parent)));
  xtArgCount++;

#if DEBUG
  fprintf(stderr, "DEBUG: xtArgCount = %d\n", xtArgCount);
#endif  

  topShell = XtCreatePopupShell(dvName, topLevelShellWidgetClass, parent,
				xtArgs, xtArgCount);

  /* Free anything in the free stack. */
  gui_freeStackFree(&freeStack);

  return topShell;

}

Narray *
gui_getTopLevelShellPublicResources()
{

  Narray	*resTopLevelShell;
  int		i, num;

#if DEBUG
  fprintf(stderr, "DEBUG: gui_getTopLevelShellPublicResources()\n");
#endif

  num = sizeof(topLevelShellPublicResources) / sizeof(topLevelShellPublicResources[0]);
  resTopLevelShell = Narray_create(num);
  for (i = 0; i < num; i++) {
    Narray_add(resTopLevelShell, (char *) topLevelShellPublicResources[i], NULL);
  }

  return resTopLevelShell;

}

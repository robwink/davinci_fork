/* widget_transientshell.c
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

#include "widget_transientshell.h"

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

static const char *transientShellPublicResources[] = {
  "transientFor",
};

/*****************************************************************************
 *
 * FUNCTION DEFINITIONS
 *
 *****************************************************************************/

int
gui_isTransientShell(const char *name)
{
  const char *aliases[] = { "transientShell", "transientShellWidgetClass", NULL };
  return gui_isDefault(aliases, name);
}

WidgetClass
gui_getTransientShellClass(void)
{
  return transientShellWidgetClass;
}

Widget
gui_initTransientShell(const char *dvName, WidgetClass class, Widget parent,
		      Var *dvResources, void **instanceData,
		      Narray *publicResources, Widget *outerWidget)
{

  Widget		shell;
  Arg			xtArgs[DV_MAX_XT_ARGS];
  Cardinal		xtArgCount;
  FreeStackListEntry	freeStack;

#if DEBUG
  fprintf(stderr, "DEBUG: gui_initTransientShell(dvName = \"%s\", class = %ld, "
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

  shell = XtCreatePopupShell(dvName, transientShellWidgetClass, parent,
				xtArgs, xtArgCount);

  /* Free anything in the free stack. */
  gui_freeStackFree(&freeStack);

  return shell;

}

Narray *
gui_getTransientShellPublicResources()
{

  Narray	*resTransientShell;
  int		i, num;

#if DEBUG
  fprintf(stderr, "DEBUG: gui_getTransientShellPublicResources()\n");
#endif

  num = sizeof(transientShellPublicResources) / sizeof(transientShellPublicResources[0]);
  resTransientShell = Narray_create(num);
  for (i = 0; i < num; i++) {
    Narray_add(resTransientShell, (char *) transientShellPublicResources[i], NULL);
  }

  return resTransientShell;

}

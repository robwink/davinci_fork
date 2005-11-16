/* widget_errordialog.c
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

#include "widget_errordialog.h"

/*****************************************************************************
 *
 * CALLBACKS
 *
 *****************************************************************************/

static CallbackEntry pushButtonCallbacks[] = {
	{ "ok",			XmNokCallback,		gui_defaultCallback },
	{ "cancel",		XmNcancelCallback,	gui_defaultCallback },
	{ "help",		XmNhelpCallback,	gui_defaultCallback },
	{ NULL,	NULL, NULL },
};

/*****************************************************************************
 *
 * RESOURCES
 *
 *****************************************************************************/

/* No resources. */

/*****************************************************************************
 *
 * FUNCTION DEFINITIONS
 *
 *****************************************************************************/

int
gui_isErrorDialog(const char *name)
{
	const char *aliases[] = { "errorDialog", NULL };
	return gui_isDefault(aliases, name);
}

WidgetClass
gui_getErrorDialogClass(void)
{
	return xmMessageBoxWidgetClass;
}

Widget
gui_initErrorDialog(const char *dvName, WidgetClass class, Widget parent,
	Var *dvResources, void **instanceData,
	Narray *publicResources)
{
	Widget				dialog;
	Arg					xtArgs[DV_MAX_XT_ARGS];
	Cardinal			xtArgCount;
	FreeStackListEntry	freeStack;

#if DEBUG
	fprintf(stderr, "DEBUG: gui_initErrorDialog(dvName = \"%s\", class = %ld, "
		"parent = %ld, instanceData = %ld)\n", dvName, class, parent,
		instanceData);
#endif  

	/* Parse resources, if the user supplied any. */

	xtArgCount = 0;
	freeStack.head = freeStack.tail = NULL; /* FIX: free these */
	if (dvResources != NULL) {
		xtArgCount = gui_setResourceValues(NULL, widgetClass, dvResources,
			xtArgs, &freeStack, publicResources);
	}

	/* Set the screen, required for popups. */

	XtSetArg(xtArgs[xtArgCount], XmNscreen,
		DefaultScreenOfDisplay(XtDisplay(parent)));
	xtArgCount++;

	dialog = XmCreateErrorDialog(parent, (char *) dvName, xtArgs, xtArgCount);
	XtManageChild(dialog);

	/* Free anything in the free stack. */
	gui_freeStackFree(&freeStack);

	return dialog;
}

CallbackList
gui_getErrorDialogCallbacks(void)
{
	return pushButtonCallbacks;
}

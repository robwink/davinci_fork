/* widget_notebook.c
 *
 * Davinci GUI bindings for various basic Xt/Motif widgets.
 *
 * Copyright 2005
 * Mars Space Flight Facility
 * Department of Geological Sciences
 * Arizona State University
 *
 * Eric Engle <eric.engle@asu.edu>
 *
 */

/*****************************************************************************
 *
 * INCLUDES
 *
 *****************************************************************************/

#include "widget_notebook.h"

/*****************************************************************************
 *
 * CALLBACKS
 *
 *****************************************************************************/

static CallbackEntry notebookCallbacks[] = {
	{ "pageChangedCallback", XmNpageChangedCallback, gui_defaultCallback },
	{ NULL, NULL, NULL }
};

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************/

void setScroller (Widget widget, Var *value);

/*****************************************************************************
 *
 * FUNCTION DEFINITIONS
 *
 *****************************************************************************/

int
gui_isNotebook(const char *name)
{
	const char *aliases[] = { "notebook", "xmNotebookWidgetClass", NULL };
	return gui_isDefault(aliases, name);
}

WidgetClass
gui_getNotebookClass(void)
{
	return xmNotebookWidgetClass;
}

CallbackList
gui_getNotebookCallbacks(void)
{
	return notebookCallbacks;
}

/* adds current values of pseudo-resources to dvStruct */
void
gui_getNotebookPseudoResources(const Widget widget, Var *dvStruct)
{
	Widget scroller;
	char *showScroller = "false";

	dbgprintf ("gui_getNotebookPseudoResources(%ld, %ld)\n", widget, dvStruct);

	scroller = XtNameToWidget (widget, "PageScroller");
	if (scroller) {
		if (XtIsManaged(scroller)) {
			showScroller = "true";
		}
	}

	add_struct(dvStruct, "showScroller", newString(strdup(showScroller)));
}

/* gui_setListPseudoResources()
 *
 * Handles getting/setting of item lists.  These need special handling
 * because both items and itemCount must be simultaneously set.
 *
 * NOTE: this deletes anything that it sets from the struct.
 * FIX: will need to add anything set to the gettable-resources list.
 *
 */

void
gui_setNotebookPseudoResources(Widget widget, Var *dvStruct, Narray *publicResources)
{
	int		i, cont;
	char	*name;
	Var		*value;

	dbgprintf ("gui_setNotebookPseudoResources(widget = %ld, "
			"dvStruct = %ld, publicResources = %ld)\n",
			widget, dvStruct, publicResources);

	/* Iterate over the struct, extracting any pseudo-resource items that
	 * we set.  Delete the items from the struct, and start iterating at the
	 * beginning again. */

	cont = 1;
	while (cont && get_struct_count(dvStruct)) {
		cont = 0;
		for (i = 0; i < get_struct_count(dvStruct); i++) {
			get_struct_element(dvStruct, i, &name, &value);
			if (!strcmp(name, "showScroller")) {
				setScroller (widget, value);
				Narray_add(publicResources, name, NULL);
				free_var(Narray_delete(V_STRUCT(dvStruct), name));
				cont = 1;
			}
			/* ...new comparisons go here. */
		}
	}
}

void
setScroller (Widget widget, Var *value)
{
	Widget scroller;

	dbgprintf ("widget_notebook.c:setScroller\n");
	if (V_TYPE(value) == ID_STRING) {
		scroller = XtNameToWidget (widget, "PageScroller");
		if (scroller) {
			if (!strcasecmp(V_STRING(value), "true")) {
				XtManageChild (scroller);
				/* manage the scroller */
			} else if (!strcasecmp(V_STRING(value), "false")) {
				/* unmanage the scroller */
				XtUnmanageChild (scroller);
			} else {
				/* do nothing */
				dbgprintf ("Unparseable string\n");
			}
		} else {
			/* do nothing */
			dbgprintf ("No scroller allocated\n");
		}
	} else {
		/* do nothing */
		dbgprintf ("Not a string\n");
	}
}



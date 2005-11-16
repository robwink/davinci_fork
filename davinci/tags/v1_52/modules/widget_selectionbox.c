/* widget_selectionbox.c
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

#include "widget_selectionbox.h"

/*****************************************************************************
 *
 * PROTOTYPES
 *
 *****************************************************************************/

static void setItems(const Widget, const String, const String, const Var *);

/*****************************************************************************
 *
 * CALLBACKS
 *
 *****************************************************************************/

static CallbackEntry selectionBoxCallbacks[] = {
  { "apply",	XmNapplyCallback,	gui_defaultCallback },
  { "cancel",	XmNcancelCallback,	gui_defaultCallback },
  { "help",	XmNhelpCallback,	gui_defaultCallback },
  { "noMatch",	XmNnoMatchCallback,	gui_defaultCallback },
  { "ok",	XmNokCallback,		gui_defaultCallback },
  { NULL,	NULL,			NULL                },
};

/*****************************************************************************
 *
 * RESOURCES
 *
 *****************************************************************************/

static const char *selectionBoxPublicResources[] = {
  "listItems", "textColumns", "textString",
};

/*****************************************************************************
 *
 * FUNCTION DEFINITIONS
 *
 *****************************************************************************/

int
gui_isSelectionBox(const char *name)
{
  const char *aliases[] = { "selectionbox", "xmSelectionBoxWidgetClass", NULL };
  return gui_isDefault(aliases, name);
}

WidgetClass
gui_getSelectionBoxClass(void)
{
  return xmSelectionBoxWidgetClass;
}

CallbackList
gui_getSelectionBoxCallbacks(void)
{
  return selectionBoxCallbacks;
}

Narray *
gui_getSelectionBoxPublicResources()
{

  Narray	*resList;
  int		i, num;

  num = sizeof(selectionBoxPublicResources) / sizeof(selectionBoxPublicResources[0]);
  resList = Narray_create(num);
  for (i = 0; i < num; i++) {
    Narray_add(resList, (char *) selectionBoxPublicResources[i], NULL);
  }

  return resList;

}

/* FIX: move this to gui.c */

static void
setItems(const Widget widget, const String resourceName,
	 const String countResourceName, const Var *value)
{

  FreeStackListEntry	localFreeStack;
  Darray		*stringList;
  int			stringCount;
  XtArgVal		itemTable;

  localFreeStack.head = localFreeStack.tail = NULL;

  stringList = gui_extractDarray(value);

  if (stringList == NULL) {
    parse_error("Warning: keeping old item list setting.");
  }
  else {
    stringCount = Darray_count(stringList);
    if (stringCount == -1) {
      /* Should never happen. */
      parse_error("Internal error: Darray_count == -1 in setItems().");
    }
    else {
      if (stringCount > 0) {
	itemTable = gui_setXmStringTable(widget, resourceName, NULL, value,
					 &localFreeStack);
      }
      else {
	itemTable = (XtArgVal) NULL;
      }
      /* Set the list and the count. */
      XtVaSetValues(widget, resourceName, itemTable,
		    countResourceName, (XtArgVal) stringCount, NULL);
    }
  }

  gui_freeStackFree(&localFreeStack);

  return;

}

void
gui_getSelectionBoxPseudoResources(const Widget widget, Var *dvStruct)
{

  int	listItemCount;
  Var	*listItems;

#if DEBUG
  fprintf(stderr, "DEBUG: gui_getSelectionBoxPseudoResources(%ld, %ld)\n",
	  widget, dvStruct);
#endif

  XtVaGetValues(widget, "listItemCount", &listItemCount, NULL);
  if (listItemCount == 0) {
    listItems = newText(0, NULL);
  }
  else {
    listItems = gui_getXmStringTableCount(widget, "listItems", 0, listItemCount);
  }
  add_struct(dvStruct, "listItems", listItems);

  return;

}

void
gui_setSelectionBoxPseudoResources(Widget widget, Var *dvStruct,
			   Narray *publicResources)
{

  int		i, cont;
  char		*name;
  Var		*value;

#if DEBUG
  fprintf(stderr, "DEBUG: gui_setSelectionBoxPseudoResources(widget = %ld, "
	  "dvStruct = %ld, publicResources = %ld)\n",
	  widget, dvStruct, publicResources);
#endif

  /* Iterate over the struct, extracting any pseudo-resource items that
   * we set.  Delete the items from the struct, and start iterating at the
   * beginning again.  This is not really efficient, but I'm not sure how
   * else to do it without mucking about with Davinci's structures more than
   * I'd like to, or keeping track of what has/hasn't been set.
   */

  cont = 1;
  while (cont && get_struct_count(dvStruct)) {
    cont = 0;
    for (i = 0; i < get_struct_count(dvStruct); i++) {
      get_struct_element(dvStruct, i, &name, &value);
      if (!strcmp(name, "listItemCount")) {
	/* Extrapolated from listItems. */
	parse_error("WARNING: ignoring listItemCount");
	free_var(Narray_delete(V_STRUCT(dvStruct), "listItemCount"));
	cont = 1;
	break;
      }
      if (!strcmp(name, "listItems")) {
	setItems(widget, name, "listItemCount", value);
	Narray_add(publicResources, name, NULL);
	free_var(Narray_delete(V_STRUCT(dvStruct), "listItems"));
	cont = 1;
	break;
      }
     /* ...new comparisons go here. */
    }
  }

}

/* widget_list.c
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

#include "widget_list.h"

/*****************************************************************************
 *
 * PROTOTYPES
 *
 *****************************************************************************/

static void setItems(const Widget, const String, const String, const Var *);

/*****************************************************************************
 *
 * RESOURCES
 *
 *****************************************************************************/

static const char *listPublicResources[] = {
  "itemList", "selectedItemList", "visibleItemCount",
};

/*****************************************************************************
 *
 * CALLBACKS
 *
 *****************************************************************************/

static CallbackEntry listCallbacks[] = {
  { "browseSelection",	XmNbrowseSelectionCallback,	gui_defaultCallback },
  { "defaultAction",	XmNdefaultActionCallback,	gui_defaultCallback },
  { "extendedSelection",XmNextendedSelectionCallback,	gui_defaultCallback },
  { "multipleSelection",XmNmultipleSelectionCallback,	gui_defaultCallback },
  { "singleSelection",	XmNsingleSelectionCallback,	gui_defaultCallback },
  { NULL,		NULL,				NULL                }
};

/*****************************************************************************
 *
 * FUNCTION DEFINITIONS
 *
 *****************************************************************************/

int
gui_isList(const char *name)
{
  const char *aliases[] = { "list", "xmListWidgetClass", NULL };
  return gui_isDefault(aliases, name);
}

WidgetClass
gui_getListClass(void)
{
  return xmListWidgetClass;
}

CallbackList
gui_getListCallbacks(void)
{
  return listCallbacks;
}

static void
setItems(const Widget widget, const String resourceName,
	 const String countResourceName, const Var *value)
{

  FreeStackListEntry	localFreeStack;
  Narray		*stringList;
  int			stringCount;
  XtArgVal		itemTable;

  localFreeStack.head = localFreeStack.tail = NULL;

  stringList = gui_extractStringList(value);

  if (stringList == NULL) {
    parse_error("Warning: keeping old item list setting.");
  }
  else {
    stringCount = Narray_count(stringList);
    if (stringCount == -1) {
      /* Should never happen. */
      parse_error("Internal error: Narray_count == -1 in setItems().");
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

/* void
 * gui_getListPseudoResources()
 *
 * Obtains the item counts for 'items' and 'selectedItems' and retrieves the
 * string lists.  This is necessary because these resource values are
 * undefined when the count is 0.  Lame!
 *
 * FIX: deal with public resources list.
 *
 */

void
gui_getListPseudoResources(const Widget widget, Var *dvStruct)
{

  int	itemCount, selectedItemCount;
  Var	*items, *selectedItems;

#if DEBUG
  fprintf(stderr, "DEBUG: gui_getListPseudoResources(%ld, %ld)\n",
	  widget, dvStruct);
#endif

  XtVaGetValues(widget, "itemCount", &itemCount, NULL);
  if (itemCount == 0) {
    items = newText(0, NULL);
  }
  else {
    items = gui_getXmStringTableCount(widget, "items", 0, itemCount);
  }
  add_struct(dvStruct, "itemList", items);

  XtVaGetValues(widget, "selectedItemCount", &selectedItemCount, NULL);
  if (selectedItemCount == 0) {
    selectedItems = newText(0, NULL);
  }
  else {
    selectedItems = gui_getXmStringTableCount(widget, "selectedItems", 0,
					      selectedItemCount);
  }
  add_struct(dvStruct, "selectedItemList", selectedItems);

  return;

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
gui_setListPseudoResources(Widget widget, Var *dvStruct,
			   Narray *publicResources)
{

  int		i, cont;
  char		*name;
  Var		*value;

#if DEBUG
  fprintf(stderr, "DEBUG: gui_setListPseudoResources(widget = %ld, "
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
      if (!strcmp(name, "itemList")) {
	setItems(widget, "items", "itemCount", value);
	Narray_add(publicResources, name, NULL);
	free_var(Narray_delete(V_STRUCT(dvStruct), "itemList"));
	cont = 1;
	break;
      }
      if (!strcmp(name, "selectedItemList")) {
	setItems(widget, "selectedItems", "selectedItemCount", value);
	Narray_add(publicResources, name, NULL);
	free_var(Narray_delete(V_STRUCT(dvStruct), "selectedItemList"));
	cont = 1;
	break;
      }
     /* ...new comparisons go here. */
    }
  }

  return;

}

/* gui_getListPublicResources()
 *
 * FIX: this could be turned into a generic function in gui.c, with a
 *      helper function in widget_*.c that just returns the static
 *      string list.
 *
 * Builds a Narray based from the public resources list.
 *
 * Returns pointer to Narray.
 *
 */

Narray *
gui_getListPublicResources()
{

  Narray	*resList;
  int		i, num;

#if DEBUG
  fprintf(stderr, "DEBUG: gui_getListPublicResources()\n");
#endif

  num = sizeof(listPublicResources) / sizeof(listPublicResources[0]);
  resList = Narray_create(num);
  for (i = 0; i < num; i++) {
    Narray_add(resList, (char *) listPublicResources[i], NULL);
  }

  return resList;

}

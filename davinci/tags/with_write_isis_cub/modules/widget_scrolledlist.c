/* widget_scrolledlist.c
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

#include "widget_scrolledlist.h"

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

static const char *scrolledListPublicResources[] = {
  "itemList", "selectedItemList", "visibleItemCount", "selectedPosition",
};

/*****************************************************************************
 *
 * CALLBACKS
 *
 *****************************************************************************/

static CallbackEntry scrolledListCallbacks[] = {
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
gui_isScrolledList(const char *name)
{
  const char *aliases[] = { "scrolledList", NULL };
  return gui_isDefault(aliases, name);
}

WidgetClass
gui_getScrolledListClass(void)
{
  return xmListWidgetClass;
}

CallbackList
gui_getScrolledListCallbacks(void)
{
  return scrolledListCallbacks;
}

Widget
gui_initScrolledList(const char *dvName, WidgetClass class, Widget parent,
		      Var *dvResources, void **instanceData,
		      Narray *publicResources, Widget *outerWidget)
{

  Widget		widget;
  String		widgetName;
  Arg			xtArgs[DV_MAX_XT_ARGS];
  Cardinal		xtArgCount;
  FreeStackListEntry	freeStack;
#if DEBUG
  int			vis;
#endif

#if DEBUG
  fprintf(stderr, "DEBUG: gui_initScrolledList(dvName = \"%s\", class = %ld, "
	  "parent = %ld, dvResources = %ld, instanceData = %ld)\n",
	  dvName, class, parent, dvResources, instanceData);
#endif  

  /* Initialize the parent widget classes. */
  XtInitializeWidgetClass(xmScrolledWindowWidgetClass);
  XtInitializeWidgetClass(xmScrollBarWidgetClass);

  /* Parse resources, if the user supplied any. */

#if DEBUG
  fprintf(stderr, "DEBUG: parsing resources\n");
#endif  

  xtArgCount = 0;
  freeStack.head = freeStack.tail = NULL; /* FIX: free these */
  if (dvResources != NULL) {
    gui_setResourceValues(NULL, widgetClass, dvResources,
			  xtArgs, &xtArgCount, &freeStack,
			  publicResources);
  }

  /* Set the screen, required for popups. */

#if DEBUG
  fprintf(stderr, "DEBUG: calling XmCreateScrolledList(parent = %ld, "
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
    widget = XmCreateScrolledList(parent, widgetName, NULL, xtArgCount);
  }
  else {
    widget = XmCreateScrolledList(parent, widgetName, xtArgs, xtArgCount);
  }

  /* Free anything in the free stack. */
  gui_freeStackFree(&freeStack);

  /* Manage the new widget since XmCreateScrolledList() doesn't do it. */
  XtManageChild(widget);

#if DEBUG
  XtVaGetValues(widget, XmNvisibleItemCount, &vis, NULL);
  fprintf(stderr, "DEBUG: visibleItemCount = %d\n", vis);
#endif

  return widget;

}

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

/* void
 * gui_getScrolledListPseudoResources()
 *
 * Obtains the item counts for 'items' and 'selectedItems' and retrieves the
 * string lists.  This is necessary because these resource values are
 * undefined when the count is 0.  Lame!
 *
 * FIX: deal with public resources list.
 *
 */

void
gui_getScrolledListPseudoResources(const Widget widget, Var *dvStruct)
{

  int	itemCount, selectedItemCount;
  Var	*items, *selectedItems;
  int   *selectedList, N_selectedList;

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
  add_struct(dvStruct, "items", items);

  XtVaGetValues(widget, "selectedItemCount", &selectedItemCount, NULL);
  if (selectedItemCount == 0) {
    selectedItems = newText(0, NULL);
  }
  else {
    selectedItems = gui_getXmStringTableCount(widget, "selectedItems", 0,
					      selectedItemCount);
  }
  add_struct(dvStruct, "selectedItems", selectedItems);

  if (XmListGetSelectedPos(widget, &selectedList, &N_selectedList) == TRUE) {
          add_struct(dvStruct, "selectedPosition",
                newVal(BSQ, 1, N_selectedList, 1, INT, selectedList));
  } else {
	  add_struct(dvStruct, "selectedPosition", newInt(-1));
  }


  return;

}

/* gui_setScrolledListPseudoResources()
 *
 * Handles getting/setting of item lists.  These need special handling
 * because both items and itemCount must be simultaneously set.
 *
 * NOTE: this deletes anything that it sets from the struct.
 * FIX: will need to add anything set to the gettable-resources list.
 *
 */

void
gui_setScrolledListPseudoResources(Widget widget, Var *dvStruct,
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

/* gui_getScrolledListPublicResources()
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
gui_getScrolledListPublicResources()
{

  Narray	*resList;
  int		i, num;

#if DEBUG
  fprintf(stderr, "DEBUG: gui_getScrolledListPublicResources()\n");
#endif

  num = sizeof(scrolledListPublicResources) / sizeof(scrolledListPublicResources[0]);
  resList = Narray_create(num);
  for (i = 0; i < num; i++) {
    Narray_add(resList, (char *) scrolledListPublicResources[i], NULL);
  }

  return resList;

}

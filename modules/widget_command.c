/* widget_command.c
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

#include "widget_command.h"

/*****************************************************************************
 *
 * PROTOTYPES
 *
 *****************************************************************************/

static void setItems(const Widget, const String, const Var *);

/*****************************************************************************
 *
 * RESOURCES
 *
 *****************************************************************************/

static const char *commandPublicResources[] = {
  "promptString", "command",
};

/*****************************************************************************
 *
 * CALLBACKS
 *
 *****************************************************************************/

static CallbackEntry commandCallbacks[] = {
  {
    "commandEntered",
    XmNcommandEnteredCallback,
    gui_defaultCallback
  },
  {
    "commandChanged",
    XmNcommandChangedCallback,
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
gui_isCommand(const char *name)
{
  const char *aliases[] = { "command", "xmCommandWidgetClass", NULL };
  return gui_isDefault(aliases, name);
}

WidgetClass
gui_getCommandClass(void)
{
  return xmCommandWidgetClass;
}

CallbackList
gui_getCommandCallbacks(void)
{
  return commandCallbacks;
}

/* void
 * setItems(widget, resourceName, value)
 *
 * Parses value for a command of strings, converts to XmStringTable, sets resource
 * resourceName on widget to the XmStringTable.
 *
 */

static void
setItems(const Widget widget, const String resourceName, const Var *value)
{

  FreeStackListEntry	localFreeStack;
  Narray		*stringList;
  int			stringCount;
  XtArgVal		itemTable;
  String		countResourceName;

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
      countResourceName = "historyItemCount";
      XtVaSetValues(widget, resourceName, itemTable,
		    countResourceName, (XtArgVal) stringCount, NULL);
    }
  }

  gui_freeStackFree(&localFreeStack);

  return;

}

/* void
 * gui_getCommandPseudoResources()
 *
 * Obtains the item counts for 'items' and 'selectedItems' and retrieves the
 * string commands.  This is necessary because these resource values are
 * undefined when the count is 0.  Lame!
 *
 * FIX: deal with public resources command.
 *
 */

void
gui_getCommandPseudoResources(const Widget widget, Var *dvStruct)
{

  Var	*items;
  int	itemCount;

#if DEBUG
  fprintf(stderr, "DEBUG: gui_getCommandPseudoResources(%ld, %ld)\n",
	  widget, dvStruct);
#endif

  XtVaGetValues(widget, "historyItemCount", &itemCount, NULL);
  if (itemCount == 0) {
    items = newText(0, NULL);
  }
  else {
    items = gui_getXmStringTableCount(widget, "historyItems", 0, itemCount);
  }
  add_struct(dvStruct, "historyItems", items);

  return;

}

/* gui_setCommandPseudoResources()
 *
 * Handles getting/setting of item commands.  These need special handling
 * because both items and itemCount must be simultaneously set.
 *
 * NOTE: this deletes anything that it sets from the struct.
 * FIX: will need to add anything set to the gettable-resources command.
 *
 */

void
gui_setCommandPseudoResources(Widget widget, Var *dvStruct,
			   Narray *publicResources)
{

  int		i, cont;
  char		*name;
  Var		*value;

#if DEBUG
  fprintf(stderr, "DEBUG: gui_setCommandPseudoResources(widget = %ld, "
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
      if (!strcmp(name, "itemCount")) {
	/* Extrapolated from items. */
	parse_error("WARNING: ignoring itemCount");
	free_var(Narray_delete(V_STRUCT(dvStruct), "itemCount"));
	cont = 1;
	break;
      }
      if (!strcmp(name, "items")) {
	setItems(widget, name, value);
	Narray_add(publicResources, name, NULL);
	free_var(Narray_delete(V_STRUCT(dvStruct), "items"));
	cont = 1;
	break;
      }
     /* ...new comparisons go here. */
    }
  }

}

/* gui_getCommandPublicResources()
 *
 * FIX: this could be turned into a generic function in gui.c, with a
 *      helper function in widget_*.c that just returns the static
 *      string command.
 *
 * Builds a Narray based from the public resources command.
 *
 * Returns pointer to Narray.
 *
 */

Narray *
gui_getCommandPublicResources()
{

  Narray	*resCommand;
  int		i, num;

#if DEBUG
  fprintf(stderr, "DEBUG: gui_getCommandPublicResources()\n");
#endif

  num = sizeof(commandPublicResources) / sizeof(commandPublicResources[0]);
  resCommand = Narray_create(num);
  for (i = 0; i < num; i++) {
    Narray_add(resCommand, (char *) commandPublicResources[i], NULL);
  }

  return resCommand;

}

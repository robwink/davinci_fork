/* widget_fileselectionbox.c
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

#include "widget_fileselectionbox.h"

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

static CallbackEntry pushButtonCallbacks[] = {
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

static const char *fileSelectionBoxPublicResources[] = {
  "directory", "dirSpec", "fileTypeMask", "pattern",
};

/*****************************************************************************
 *
 * FUNCTION DEFINITIONS
 *
 *****************************************************************************/

int
gui_isFileSelectionBox(const char *name)
{
  const char *aliases[] = { "fileSelectionBox", "xmFileSelectionBoxWidgetClass", NULL };
  return gui_isDefault(aliases, name);
}

WidgetClass
gui_getFileSelectionBoxClass(void)
{
  return xmFileSelectionBoxWidgetClass;
}

CallbackList
gui_getFileSelectionBoxCallbacks(void)
{
  return pushButtonCallbacks;
}

Narray *
gui_getFileSelectionBoxPublicResources()
{

  Narray	*resList;
  int		i, num;

  num = sizeof(fileSelectionBoxPublicResources) / sizeof(fileSelectionBoxPublicResources[0]);
  resList = Narray_create(num);
  for (i = 0; i < num; i++) {
    Narray_add(resList, (char *) fileSelectionBoxPublicResources[i], NULL);
  }

  return resList;

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

void
gui_getFileSelectionBoxPseudoResources(const Widget widget, Var *dvStruct)
{

  int	dirListItemCount, fileListItemCount;
  Var	*dirListItems, *fileListItems;

#if DEBUG
  fprintf(stderr, "DEBUG: gui_getFileSelectionBoxPseudoResources(%ld, %ld)\n",
	  widget, dvStruct);
#endif

  XtVaGetValues(widget, "dirListItemCount", &dirListItemCount, NULL);
  if (dirListItemCount == 0) {
    dirListItems = newText(0, NULL);
  }
  else {
    dirListItems = gui_getXmStringTableCount(widget, "dirListItems", 0, dirListItemCount);
  }
  add_struct(dvStruct, "dirListItems", dirListItems);

  XtVaGetValues(widget, "fileListItemCount", &fileListItemCount, NULL);
  if (fileListItemCount == 0) {
    fileListItems = newText(0, NULL);
  }
  else {
    fileListItems = gui_getXmStringTableCount(widget, "fileListItems", 0,
					      fileListItemCount);
  }
  add_struct(dvStruct, "fileListItems", fileListItems);

}

void
gui_setFileSelectionBoxPseudoResources(Widget widget, Var *dvStruct,
			   Narray *publicResources)
{

  int		i, cont;
  char		*name;
  Var		*value;

#if DEBUG
  fprintf(stderr, "DEBUG: gui_setFileSelectionBoxPseudoResources(widget = %ld, "
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
      if (!strcmp(name, "dirListItemCount")) {
	/* Extrapolated from dirListItems. */
	parse_error("WARNING: ignoring dirListItemCount");
	free_var(Narray_delete(V_STRUCT(dvStruct), "dirListItemCount"));
	cont = 1;
	break;
      }
      if (!strcmp(name, "fileListItemCount")) {
	/* Extrapolated from fileListItems. */
	parse_error("WARNING: ignoring fileListItemCount");
	free_var(Narray_delete(V_STRUCT(dvStruct), "fileListItemCount"));
	cont = 1;
	break;
      }
      if (!strcmp(name, "dirListItems")) {
	setItems(widget, name, "dirListItemCount", value);
	free_var(Narray_delete(V_STRUCT(dvStruct), "dirListItems"));
	Narray_add(publicResources, name, NULL);
	cont = 1;
	break;
      }
      if (!strcmp(name, "fileListItems")) {
	setItems(widget, name, "fileListItemCount", value);
	free_var(Narray_delete(V_STRUCT(dvStruct), "fileListItems"));
	Narray_add(publicResources, name, NULL);
	cont = 1;
	break;
      }
     /* ...new comparisons go here. */
    }
  }

}

/* widget_xbaematrix.c
 *
 * Davinci GUI bindings for the Bellcore Application Environment XbaeMatrix
 * widget.
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

#include "widget_xbaematrix.h"

/*****************************************************************************
 *
 * RESOURCE LISTS
 *
 *****************************************************************************/

static const char *xbaeMatrixPublicResources[] = {
  "rows"
};

/*****************************************************************************
 *
 * CALLBACKS
 *
 *****************************************************************************/

/* FIX: pending */

/*****************************************************************************
 *
 * FUNCTION DEFINITIONS
 *
 *****************************************************************************/

int
gui_isXbaeMatrix(const char *name)
{
  const char *aliases[] = { "XbaeMatrix", "xbaeMatrixWidgetClass", NULL };
  return gui_isDefault(aliases, name);
}

WidgetClass
gui_getXbaeMatrixClass(void)
{
  return xbaeMatrixWidgetClass;
}

Narray *
gui_getXbaeMatrixPublicResources()
{

  Narray	*resList;
  int		i, num;

#if DEBUG
  fprintf(stderr, "DEBUG: gui_getXbaeMatrixPublicResources()\n");
#endif

  num = sizeof(xbaeMatrixPublicResources) / sizeof(xbaeMatrixPublicResources[0]);
  resList = Narray_create(num);
  for (i = 0; i < num; i++) {
    Narray_add(resList, (char *) xbaeMatrixPublicResources[i], NULL);
  }

  return resList;

}

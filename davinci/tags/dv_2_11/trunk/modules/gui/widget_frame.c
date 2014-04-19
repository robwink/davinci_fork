/* widget_frame.c
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

#include "widget_frame.h"

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

#if 0
static const char *framePublicResources[] = {
};
#endif

/*****************************************************************************
 *
 * FUNCTION DEFINITIONS
 *
 *****************************************************************************/

int
gui_isFrame(const char *name)
{
  const char *aliases[] = { "frame", "xmFrameWidgetClass", NULL };
  return gui_isDefault(aliases, name);
}

WidgetClass
gui_getFrameClass(void)
{
  return xmFrameWidgetClass;
}

Narray *
gui_getFramePublicResources()
{

  return Narray_create(0);

#if 0
  Narray	*resList;
  int		i, num;

  num = sizeof(framePublicResources) / sizeof(framePublicResources[0]);
  resList = Narray_create(num);
  for (i = 0; i < num; i++) {
    Narray_add(resList, (char *) framePublicResources[i], NULL);
  }

  return resList;
#endif

}

/* widget_combobox.c
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

#include "widget_combobox.h"

/*****************************************************************************
 *
 * CALLBACKS
 *
 *****************************************************************************/

static CallbackEntry comboboxCallbacks[] = {
	{ "selectionCallback", XmNselectionCallback, gui_defaultCallback },
	{ NULL, NULL, NULL }
};

/*****************************************************************************
 *
 * FUNCTION DEFINITIONS
 *
 *****************************************************************************/

int
gui_isComboBox(const char *name)
{
	const char *aliases[] = { "combobox", "xmComboBoxWidgetClass", NULL };
	return gui_isDefault(aliases, name);
}

WidgetClass
gui_getComboBoxClass(void)
{
	return xmComboBoxWidgetClass;
}

CallbackList
gui_getComboBoxCallbacks(void)
{
	return comboboxCallbacks;
}

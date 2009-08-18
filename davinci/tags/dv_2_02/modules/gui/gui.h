/* gui.h
 *
 * Copyright 2002,2003
 * Mars Space Flight Facility
 * Department of Geological Sciences
 * Arizona State University
 *
 * Jim Stewart <Jim.Stewart@asu.edu>
 *
 * Portions of this code are based on test_iw.c, part of the VICAR
 * software package.
 *
 */

/* Davinci/X widget ID mapping and widget-specific property storage. */

#ifndef DV_GUI_H
#define DV_GUI_H

/* #define DEBUG 1 */

#define DV_INVALID_WIDGET_ID -1
#define DV_MAX_XT_ARGS 1024

/* Widget, String */
#include <X11/Intrinsic.h>

#include "parser.h"

/* Data types */

/* FIX: pick a better name (like WidgetListEntryPtr maybe) */
/*      WidgetList conflicts with Xt stuff. */

typedef struct _WidgetListEntry *MyWidgetList;

typedef struct _WidgetListEntry {
  int		id;			/* Davinci id. */
  Widget	widget;			/* Xt widget instance. */
#if 0
  Boolean	valid;			/* Widget still valid? */
#endif
  Widget	outerWidget;		/* Outermost widget when nested. */
  WidgetClass	widgetClass;		/* Xt WidgetClass. */
  int		widgetClassId;		/* Internal widget class id. */
  Narray	*publicResources;	/* List of resources for gui.get(). */
  /* FIX: rename data to instanceData */
  void		*data;			/* Instance-specific data. */
  Var		*dvData;		/* Instance-specific user data. */
  MyWidgetList	prev;			/* Previous widget in list. */
  MyWidgetList	next;			/* Next widget in list. */
} WidgetListEntry;

/* Callback stuff */

typedef struct _CallbackEntry {
  char			*dvCallbackName;
  String		xtCallbackName;
  XtCallbackProc	callback;
} CallbackEntry;

typedef struct _CallbackEntry *CallbackList;

/* Public functions */

MyWidgetList	gui_getWidgetListEntryFromWidget(Widget);
int				gui_isDefault(const char **, const char *);
void			gui_defaultCallback(Widget, XtPointer, XtPointer);
void			gui_pseudoCallback(Widget, String);
Widget			gui_initDefault(const char *, WidgetClass, Widget, Var *,
					void **, Narray *, Widget *);

/* resources.h
 *
 * Support for Davinci GUI module, Xt resource handling.
 *
 * Copyright 2003
 * Mars Space Flight Facility
 * Department of Geological Sciences
 * Arizona State University
 *
 * Jim Stewart <Jim.Stewart@asu.edu>.
 *
 */

/* EnumResource and EnumList are used to restrict resource values,
 * used in conjunction with gui_setEnum, gui_getEnum.
 */

#ifndef DV_GUI_RESOURCES_H
#define DV_GUI_RESOURCES_H

#include <Xm/Xm.h>
#include "parser.h"

#if 0
/* FIX: remove */
typedef struct _EnumResource {
  String	name;
  unsigned char	value;
} EnumResource;

typedef EnumResource *	EnumList;
#endif

/* Enum name map is used to map enumerated types to their names.  It's done
 * based on the resource class (FIX: maybe resource type, update comment).
 */

typedef struct _EnumNameEntry {
  const char	*enumName;
  unsigned int	enumValue;
} EnumNameEntry;

typedef EnumNameEntry *EnumNameList;

typedef struct _EnumTypeNameMapEntry {
  String	enumType;
  EnumNameList	enumNameList;
} EnumTypeNameMapEntry;

/* Free stack is used to maintain a list of addresses to free after setting
 * resources.  It's required because of the hacked-up way resources values
 * are parsed.
 */

typedef struct _FreeStackEntry *FreeStackEntryPtr;

typedef struct _FreeStackEntry {
  XtPointer		address;
  FreeStackEntryPtr	next;
} FreeStackEntry;

typedef struct _FreeStackListEntry {
  FreeStackEntryPtr	head;
  FreeStackEntryPtr	tail;
} FreeStackListEntry;

typedef FreeStackListEntry *FreeStackList;

/* Resource type map is needed because Xt/Motif use 10 gajillion different
 * string constants for resource types and it seems to be impossible to do an
 * meaningful pointer comparison on them, so I've resorted to literal string
 * comparisons to determine resource types.  Bleah!
 */

typedef Var *		(*_get_resource_func)(const Widget, const String,
					      const String);
typedef XtArgVal	(*_set_resource_func)(const Widget, const String,
					      const String, const Var *,
					      FreeStackList);
typedef struct _ResourceTypeMapEntry {
  const char		**typeNames;
  _get_resource_func	getValue;
  _set_resource_func	setValue;
} ResourceTypeMapEntry;

/* Misc. functions for getting/setting widget resources, based on data type. */

typedef struct _WidgetResourceEntry {
  char			*dvName;
  char			*xtName;
  _get_resource_func	get;
  _set_resource_func	set;
  /* FIX: remove allowedValues */
  EnumNameList		allowedValues;
} WidgetResourceEntry;

typedef struct _WidgetResourceEntry *ResourceList;

Var *	 gui_getIgnore(const Widget, const String, const String);
Var *	 gui_getEnum(const Widget, const String, const String);
XtArgVal gui_setEnum(const Widget, const String, const String,
			const Var *, FreeStackList);
Var *	 gui_getWidget(const Widget, const String, const String);
XtArgVal gui_setWidget(const Widget, const String, const String,
			const Var *, FreeStackList);
Var *	 gui_getBoolean(const Widget, const String, const String);
XtArgVal gui_setBoolean(const Widget, const String, const String,
			const Var *, FreeStackList);
Var *	 gui_getBool(const Widget, const String, const String);
XtArgVal gui_setBool(const Widget, const String, const String,
			const Var *, FreeStackList);
Var *	 gui_getByte(const Widget, const String, const String);
XtArgVal gui_setByte(const Widget, const String, const String,
			const Var *, FreeStackList);
Var *	 gui_getShort(const Widget, const String, const String);
XtArgVal gui_setShort(const Widget, const String, const String,
			const Var *, FreeStackList);
Var *	 gui_getFloat(const Widget, const String, const String);
XtArgVal gui_setFloat(const Widget, const String, const String,
			const Var *, FreeStackList);
Var *	 gui_getDouble(const Widget, const String, const String);
XtArgVal gui_setDouble(const Widget, const String, const String,
			const Var *, FreeStackList);
Var *	 gui_getInt(const Widget, const String, const String);
XtArgVal gui_setInt(const Widget, const String, const String,
			const Var *, FreeStackList);
Var *	 gui_getCardinal(const Widget, const String, const String);
XtArgVal gui_setCardinal(const Widget, const String, const String,
			const Var *, FreeStackList);
Var *	 gui_getDimension(const Widget, const String, const String);
XtArgVal gui_setDimension(const Widget, const String, const String,
			const Var *, FreeStackList);
Var *	 gui_getString(const Widget, const String, const String);
XtArgVal gui_setString(const Widget, const String, const String,
			const Var *, FreeStackList);
Var *	 gui_getXmString(const Widget, const String, const String);
XtArgVal gui_setXmString(const Widget, const String, const String,
			const Var *, FreeStackList);
Var *	 gui_getXmStringTable(const Widget, const String, const String);
Var *	 gui_getXmStringTableFree(const Widget, const String, const String);
Var *	 gui_getXmStringTableCount(const Widget, const String, const int, const int);
XtArgVal gui_setXmStringTable(const Widget, const String, const String,
			const Var *, FreeStackList);
XtArgVal gui_setXmStringTableFromDarray(const Widget, const String,
			const String, const Darray *, FreeStackList);
Var *	 gui_getTextPosition(const Widget, const String, const String);
XtArgVal gui_setTextPosition(const Widget, const String, const String,
			const Var *, FreeStackList);
XtArgVal gui_setListItems(const Widget, const String, const String,
			const Var *, FreeStackList);
XtArgVal gui_setReadOnly(const Widget, const String, const String,
			const Var *, FreeStackList);

#endif /* DV_GUI_RESOURCES_H */

/* FIX: this function is heinous, maybe return a struct. */
Cardinal gui_setResourceValues(Widget, WidgetClass, Var *, Arg *,
			FreeStackList, Narray *);
Darray *gui_extractDarray(const Var *);
Narray *gui_extractNarray(const Var *);
void	gui_freeStackPush(FreeStackList, void *);
void	gui_freeStackFree(FreeStackList);
void	gui_addEvent(Widget, String);
char *	dupString(const char *);
void	dbgprintf (char *fmt, ...);
void    gui_setFloatNow (Widget w, char *name, float val);

#endif /* DV_GUI_H */

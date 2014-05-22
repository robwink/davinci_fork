/*
 *
 * Davinci loadable module for using Xt widgets.
 *
 * See the "DATA TYPES AND GLOBAL VARIABLES" section below to add widgets.
 *
 * Copyright 2002,2003
 * Mars Space Flight Facility
 * Department of Geological Sciences
 * Arizona State University
 *
 * Modified/maintained by Jim Stewart <Jim.Stewart@asu.edu>.
 * Original code written November 12-22, 2002 by Randy Kaelber.
 *
 */

/* FIX TODO:
 *
 * Make the public resources stuff generic with a default function
 *   in this file.  Same for the "is" functions; make a getAliases,
 *   or just allow a single name for each.
 * Make sure all non-static functions have a gui_ prefix.
 * Make sure all resource name lists from get_struct_names() are freed.
 * Try to create single-exit-point functions.
 * Use const function arguments whereever appropriate.
 * See if anything uses resourceType in the resource setters and ditch it if
 *   not.
 * Return XmStringTable as TEXT rather than STRUCT.
 * Widget destroy:
 *   . in gui.destroy():
 *       call gui_destroyDefault()
 *   . in gui_destroyDefault():
 *       call gui_addEvent(widget, "destroy")
 *       call gui_invalidateWidget()
 *       call XtDestroyWidget()
 *   . in gui_defaultDestroyCallback()
 *       call gui_addEvent(widget, "destroy")
 *       call gui_invalidateWidget()
 *   . in gui_invalidateWidget():
 *   . need some sort of app work process:
 *       when safe, free up all widget memory and destroy WidgetListEntry
 * support reading of read-only resources (need to handle each type separately)
 *   . example: TopLevelShell.numChildren
 *
 */

#define MAXBYTE 255
#define	DV_GUI_PRIVATE_RESOURCE_NAME "davinciPrivate"
/* First widget ID to use. */
#define DV_GUI_APPSHELL_WIDGETID 0

/*****************************************************************************
 *
 * INCLUDES
 *
 *****************************************************************************/

/* Davinci Includes */

/* System Includes */
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <X11/IntrinsicP.h>
#include <Xm/DialogS.h>

#include "parser.h"
/* FIX: #include "help.h" */
#include "gui.h"
/* FIX: #include "resources.h" */

#include "widget_arrowbutton.h"
#include "widget_cascadebutton.h"
#include "widget_combobox.h"
#include "widget_command.h"
#include "widget_drawnbutton.h"
#include "widget_errordialog.h"
#include "widget_fileselectionbox.h"
#include "widget_form.h"
#include "widget_frame.h"
#include "widget_label.h"
#include "widget_list.h"
#include "widget_menubar.h"
#include "widget_pushbutton.h"
#include "widget_panedwindow.h"
#include "widget_radiobox.h"
#include "widget_rowcolumn.h"
#include "widget_scale.h"
#include "widget_scrollbar.h"
#include "widget_scrolledlist.h"
#include "widget_scrolledwindow.h"
#include "widget_selectionbox.h"
#include "widget_separator.h"
#include "widget_text.h"
#include "widget_textfield.h"
#include "widget_togglebutton.h"
#include "widget_toplevelshell.h"
#include "widget_transientshell.h"
#if PLOT_WIDGETS
#include "widget_xyplot.h"
#endif
#include "widget_linebox.h"
#if 0
#include "widget_xbaematrix.h"
#endif

#ifdef HAVE_LIBVICAR
#include "widget_vicar.h"
#endif

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES (more after variables block below)
 *
 *****************************************************************************/

/* Pseudo-resource functions for tagging widget instances with Davinci vars.
 * This allows the user to implement callbacks or anything else they want to
 * do on a per-widget-instance basis.
 */

static Var *	gui_getPrivate(Widget);
static void	gui_setPrivate(Widget, Var *);
static Var *gui_getResources (int dvWidgetId, Var *dvResourceList, int dvAllResources);

/* These don't have prototypes in any Davinci headers.. */
extern Var *eval_buffer(char *);
extern void parse_buffer(char *);

/* For some reason this isn't prototyped in string.h, according to gcc.. */
/* FIX: remove this and rely on string.h once debugging is over. */
extern int strcasecmp(const char *, const char *);

/* Davinci parser hooks.  These are the only external entry points. */

static Var *dv_CreateWidget(vfuncptr, Var *);
static Var *dv_DestroyWidget(vfuncptr, Var *);
static Var *dv_XGet(vfuncptr, Var *);
static Var *dv_XSet(vfuncptr, Var *);
static Var *dv_XEventWait(vfuncptr, Var *);
static Var *dv_Realize(vfuncptr, Var *);
static Var *dv_AddCallback(vfuncptr, Var *);
static Var *dv_ListCallbacks(vfuncptr, Var *);
static Var *dv_RemoveCallback(vfuncptr, Var *);
#if 0
static Var *dv_WidgetShow(vfuncptr, Var *);
static Var *dv_WidgetHide(vfuncptr, Var *);
#endif

/* Default widget initializer/destroyers, misc. */

#if 0
static Boolean	isValidWidget(MyWidgetList);
#endif
Widget			gui_initDefault(const char *, WidgetClass, Widget, Var *,
					void **, Narray *, Widget *);
static void		gui_destroyDefault(Widget, void *);
void			gui_defaultDestroyCallback(Widget, XtPointer, XtPointer);
void			gui_destroyWidget(Widget);
static void		printEnumOptions (Var *widgetEnumData);
static void		gui_getResourceValues(Widget, Var *, Narray *, int,
					XtResourceList *, Cardinal);
static int		gui_doSetResourceValues(Widget, char *, Var *, Arg *,
					Cardinal *, FreeStackList, XtResourceList *, Cardinal);

/* Top-level widget (app shell) placeholder helper function. */

static int		gui_isTop(const char *);

/* Functions for manipulating the widget instance list. */

static Widget	getWidgetFromId(int);
static int		getIdFromWidget(Widget);
static int		getWidgetClassIdByName(const char *);
static WidgetClass	getWidgetClassFromId(int);
#if 0
static MyWidgetList	getWidgetListEntryFromId(int);
#endif

/*****************************************************************************
 *
 * DATA TYPES AND GLOBAL VARIABLES
 *
 *****************************************************************************/

/* FIX: why are these instantiated externally if all GUI functionality is here? */
extern Widget		applicationShell;
extern XtAppContext	applicationContext;

/* FIX: this should probably be EventQueueEntryPtr, but that's long. */
/*      maybe use EventQueueEntry.. */
typedef struct _EventQueueEntry *EventQueuePtr;

typedef struct _EventQueueEntry {
	Widget	widget;
	/* FIX: maybe change event below to eventName.. */
	String	event;
	EventQueuePtr	next;
} EventQueueEntry;

/* These have to be volatile due to asynchronous Xt callbacks. */
static volatile EventQueuePtr EventQueueHead = NULL;
static volatile EventQueuePtr EventQueueTail = NULL;

/* FIX: these names are irrational */
typedef struct _EventWatchEntry	*EventWatchList;

typedef struct _EventWatchEntry {
	int			dvWidgetId;
	EventWatchList	next;
} EventWatchEntry;

Var *widgetEnumData;

/* FIX: This was previously set in DavinciCallback() but I dunno why.. */
/* static volatile int eventloopexit = 1; */

/*****************************************************************************
 *
 * Resource type map
 *
 * This maps Xt/Motif resource types to the get/set functions that convert
 * Xt/Motif resource values to/from Davinci Var values.  The resource type
 * string is compared to a list of string literals to determine a match.  This
 * is a less than ideal way of determining the resource type, however it seems
 * to be impossible to do a direct pointer comparison with the XtRfoo/XmRfoo
 * resource name strings provided in the headers.
 *
 ****************************************************************************/

/* FIX: try to move most of this stuff to the widget-specific files. */
/* FIX: instead of having an entry with capitalization up front, can we do case-
   insensitive matching when these are used? */

/* ignoreTypes is a hack to hide types we intentionally do not handle, without
 * spamming the user with confusing messages about unhandled types.
 */

static const char *ignoreTypes[] = {
	"AcceleratorTable",		/* XmTextField */
	"BottomShadowPixmap",	/* XmTextField */
	"Callback",				/* XmTextField */
	"Colormap",				/* XmTextField */
	"FontList",				/* XmToggleButton */
	"Function",
	"HighlightPixmap",		/* XmTextField */
	"KeySym",				/* XmToggleButton */
	"ManBottomShadowPixmap",
	"ManHighlightPixmap",
	"ManTopShadowPixmap",
	"NavigationType",		/* XmTextField */
	"Pixel",				/* XmTextField */
	"Pixmap",				/* XmTextField */
	"Pointer",				/* XmTextField */
	"PrimForegroundPixmap",	/* XmToggleButton */
	"Screen",				/* XmToggleButton */
	"TearOffModel",
	"TopShadowPixmap",		/* XmTextField */
	"TranslationTable",		/* XmTextField */
	"ValueWcs",				/* XmTextField */
	"WhichButton",
	"WidgetClass",
	"WidgetList",
	"XmBackgroundPixmap",	/* XmTextField */
	"NoScalingDynamicPixmap",
	"Direction",
	"RenderTable",
	"DynamicPixmap",
	"ButtonFontList",		/* XmForm */
	"ButtonRenderTable",	/* XmForm */
	"DialogStyle",			/* XmForm */
	"LabelFontList",		/* XmForm */
	"LabelRenderTable",		/* XmForm */
	"TextFontList",			/* XmForm */
	"TextRenderTable",		/* XmForm */
	"UnsignedChar",			/* XmForm */
	"AutomaticSelection",	/* XmList */
	"MatchBehavior",		/* XmList */
	"PrimaryOwnership",		/* XmList */
	"SelectColor",			/* XmList */
	"SelectionMode",		/* XmList */
	NULL
};

static const char *booleanTypes[] = { "Boolean", NULL };
static const char *boolTypes[] = { "Bool", NULL };
static const char *widgetTypes[] = { "Widget", "MenuWidget", NULL };
static const char *dimensionTypes[] = { "HorizontalDimension",
	"VerticalDimension",
	"BooleanDimension",
	"Dimension",
	NULL };
static const char *stringTypes[] = { "String", NULL };
static const char *xmStringTypes[] = { "XmString", NULL };
static const char *xmStringTableTypes[] = { "XmStringTable", NULL };
static const char *cardinalTypes[] = { "Cardinal", NULL };
static const char *intTypes[] = { "Int",
	"TopItemPosition",
	"VerticalInt",
	"HorizontalInt",
	NULL };
static const char *shortTypes[] = { "Short",
	"Position",
	"HorizontalPosition",
	"VerticalPosition",
	NULL };
static const char *floatTypes[] = { "Float", NULL };
static const char *doubleTypes[] = { "Double", NULL };
static const char *textPositionTypes[] = { "TextPosition", NULL };

/* Enum types are all data types whose values are represented by
 * enumeration constants such as XmPACK_COLUMN for XmRowColumn.rowColumnType.
 * Comments next to each type show a class that uses the given type (most of
 * the time not the only class, however).
 */

static const char *enumTypes[] = {
	"UnitType",
	"VerticalAlignment",
	"RowColumnType",
	"Alignment",
	"Orientation",
	"Packing",
	"StringDirection",
	"LabelType",
	"ArrowDirection",
	"MultiClick",
	"ShadowType",
	"SelectionPolicy",
	"ListSizePolicy",
	"ProcessingDirection",
	"SeparatorType",			/* XmSeparator */
	"IndicatorType",			/* XmToggleButton */
	XmCSet,						/* XmToggleButton */
	XmCAttachment,				/* XmForm */
	XmCEditMode,				/* XmText */
	XmRFileTypeMask,
	XmRScrollBarDisplayPolicy,	/* XmScrolledWindow */
	XmRScrollBarPlacement,		/* XmScrolledWindow */
	XmRScrollingPolicy,			/* XmScrolledWindow */
	XmRResizePolicy,			/* XmBulletinBoard, XmForm, etc. */
	XmCComboBoxType,			/* XmComboBox */
#ifdef HAVE_LIBVICAR
	XvicCDataSavePolicy,			/* VICAR */
	XvicCDataType,
	XvicCDitherMode,
	XvicCImageMode,
	XvicCLutType,
	XvicCLut16Type,
	XvicCStretchPolicy,
	XvicCColormapPolicy,
	XvicCVisualType,
	XvicCWorkProcPolicy,
#endif
	NULL
};

/* FIX:
 *   Need to provide a method for widgets to register their own types, in
 *   addition to the defaults.  The Vicar widget in particular has its own
 *   needs that are unlikely to be relevent for any other widget.
 */

static ResourceTypeMapEntry ResourceTypeMap[] = {
	{ ignoreTypes,			gui_getIgnore,			gui_setReadOnly },
	{ booleanTypes,			gui_getBoolean,			gui_setBoolean },
	{ boolTypes,            gui_getBool,            gui_setBool },
	{ widgetTypes,			gui_getWidget,			gui_setWidget },
	{ enumTypes,			gui_getEnum,			gui_setEnum },
	{ stringTypes,			gui_getString,			gui_setString },
	{ textPositionTypes,	gui_getTextPosition,	gui_setTextPosition },
	{ xmStringTypes,		gui_getXmString,		gui_setXmString },
	{ xmStringTableTypes,	gui_getXmStringTable,	gui_setXmStringTable },
	{ cardinalTypes,		gui_getCardinal,		gui_setCardinal },
	{ intTypes,				gui_getInt,				gui_setInt },
	{ shortTypes,			gui_getShort,			gui_setShort },
	{ floatTypes,           gui_getFloat,           gui_setFloat },
	{ doubleTypes,          gui_getDouble,          gui_setDouble },
	{ dimensionTypes,		gui_getDimension,		gui_setDimension },
	{ NULL,					NULL,					NULL }
};

static EnumNameEntry alignmentEnums[] = {
	{ "ALIGNMENT_BEGINNING",		XmALIGNMENT_BEGINNING },
	{ "ALIGNMENT_CENTER",			XmALIGNMENT_CENTER },
	{ "ALIGNMENT_END",			XmALIGNMENT_END },
	{ NULL,				0 }
};

static EnumNameEntry arrowDirectionEnums[] = {
	{ "ARROW_UP",				XmARROW_UP },
	{ "ARROW_DOWN",			XmARROW_DOWN },
	{ "ARROW_LEFT",			XmARROW_LEFT },
	{ "ARROW_RIGHT",			XmARROW_RIGHT },
	{ NULL,				0 }
};

static EnumNameEntry fileTypeMaskEnums[] = {
	{ "DIRECTORY",			XmFILE_DIRECTORY },
	{ "REGULAR",				XmFILE_REGULAR },
	{ "ANY_TYPE",				XmFILE_ANY_TYPE },
	{ NULL,				0 }
};

static EnumNameEntry multiClickEnums[] = {
	{ "MULTICLICK_DISCARD",		XmMULTICLICK_DISCARD },
	{ "MULTICLICK_KEEP",			XmMULTICLICK_KEEP },
	{ NULL,				0 }
};

static EnumNameEntry orientationEnums[] = {
	{ "VERTICAL",				XmVERTICAL },
	{ "HORIZONTAL",			XmHORIZONTAL },
	{ NULL,				0 }
};

static EnumNameEntry resizePolicyEnums[] = {
	{ "RESIZE_NONE",			XmRESIZE_NONE },
	{ "RESIZE_GROW",			XmRESIZE_GROW },
	{ "RESIZE_ANY",			XmRESIZE_ANY  },
	{ NULL,				0 }
};

static EnumNameEntry shadowTypeEnums[] = {
	{ "SHADOW_IN",			XmSHADOW_IN },
	{ "SHADOW_OUT",			XmSHADOW_OUT },
	{ "SHADOW_ETCHED_IN",			XmSHADOW_ETCHED_IN },
	{ "SHADOW_ETCHED_OUT",		XmSHADOW_ETCHED_OUT },
	{ NULL,				0 }
};

static EnumNameEntry unitTypeEnums[] = {
	{ "PIXELS",				XmPIXELS },
	{ "100TH_MILLIMETERS",		Xm100TH_MILLIMETERS },
	{ "1000TH_INCHES",			Xm1000TH_INCHES },
	{ "100TH_POINTS",			Xm100TH_POINTS },
	{ "100TH_FONT_UNITS",			Xm100TH_FONT_UNITS },
	{ NULL,				0 }
};

static EnumNameEntry verticalAlignmentEnums[] = {
	{ "ALIGNMENT_BASELINE_BOTTOM",	XmALIGNMENT_BASELINE_BOTTOM },
	{ "ALIGNMENT_BASELINE_TOP",		XmALIGNMENT_BASELINE_TOP },
	{ "ALIGNMENT_CENTER",			XmALIGNMENT_CENTER },
	{ "ALIGNMENT_CONTENTS_BOTTOM",	XmALIGNMENT_CONTENTS_BOTTOM },
	{ "ALIGNMENT_CONTENTS_TOP",		XmALIGNMENT_CONTENTS_TOP },
	{ NULL,				0 }
};

static EnumNameEntry labelTypeEnums[] = {
#if 0
	/* FIX: enable when pixmaps handled */
	{ "PIXMAP",				XmPIXMAP },
#endif
	{ "STRING",				XmSTRING },
	{ NULL,				0 }
};

static EnumNameEntry listSizePolicyEnums[] = {
	{ "VARIABLE",				XmVARIABLE },
	{ "CONSTANT",				XmCONSTANT },
	{ "RESIZE_IF_POSSIBLE",		XmRESIZE_IF_POSSIBLE },
	{ NULL,				0 }
};

static EnumNameEntry scrollBarDisplayPolicyEnums[] = {
	{ "STATIC",				XmSTATIC },
	{ "AS_NEEDED",			XmAS_NEEDED },
	{ NULL,				0 }
};

static EnumNameEntry scrollBarPlacementEnums[] = {
	{ "TOP_LEFT",				XmTOP_LEFT },
	{ "BOTTOM_LEFT",			XmBOTTOM_LEFT },
	{ "TOP_RIGHT",			XmTOP_RIGHT },
	{ "BOTTOM_RIGHT",			XmBOTTOM_RIGHT },
	{ NULL,				0 }
};

static EnumNameEntry scrollingPolicyEnums[] = {
	{ "AUTOMATIC",			XmAUTOMATIC },
	{ "APPLICATION_DEFINED",		XmAPPLICATION_DEFINED },
	{ NULL,				0 }
};

static EnumNameEntry selectionPolicyEnums[] = {
	{ "SINGLE_SELECT",			XmSINGLE_SELECT },
	{ "MULTIPLE_SELECT",			XmMULTIPLE_SELECT },
	{ "BROWSE_SELECT",			XmBROWSE_SELECT },
	{ "EXTENDED_SELECT",			XmEXTENDED_SELECT },
	{ NULL,				0 }
};

static EnumNameEntry packingEnums[] = {
	{ "PACK_TIGHT",			XmPACK_TIGHT },
	{ "PACK_COLUMN",			XmPACK_COLUMN }, 
	{ "PACK_NONE",			XmPACK_NONE },
	{ NULL,				0 }
};

static EnumNameEntry rowColumnTypeEnums[] = {
	{ "WORK_AREA",			XmWORK_AREA },
	{ "MENU_BAR",				XmMENU_BAR },
	{ "MENU_POPUP",			XmMENU_POPUP },
	{ "MENU_PULLDOWN",			XmMENU_PULLDOWN },
	{ "MENU_OPTION",			XmMENU_OPTION },
	{ NULL,				0 }
};

static EnumNameEntry tearOffModelEnums[] = {
	{ "TEAR_OFF_DISABLED",		XmTEAR_OFF_DISABLED },
	{ "TEAR_OFF_ENABLED",			XmTEAR_OFF_ENABLED },
	{ NULL,				0 }
};

static EnumNameEntry stringDirectionEnums[] = {
	{ "STRING_DIRECTION_L_TO_R",		XmSTRING_DIRECTION_L_TO_R },
	{ "STRING_DIRECTION_R_TO_L",		XmSTRING_DIRECTION_R_TO_L },
	{ "STRING_DIRECTION_DEFAULT",		XmSTRING_DIRECTION_DEFAULT },
	{ NULL,				0 }
};

static EnumNameEntry processingDirectionEnums[] = {
	{ "MAX_ON_TOP",			XmMAX_ON_TOP },
	{ "MAX_ON_BOTTOM",			XmMAX_ON_BOTTOM },
	{ "MAX_ON_LEFT",			XmMAX_ON_LEFT },
	{ "MAX_ON_RIGHT",			XmMAX_ON_RIGHT },
	{ NULL,				0 }
};

static EnumNameEntry separatorTypeEnums[] = {
	{ "NO_LINE",				XmNO_LINE },
	{ "SINGLE_LINE",			XmSINGLE_LINE },
	{ "DOUBLE_LINE",			XmDOUBLE_LINE },
	{ "SINGLE_DASHED_LINE",		XmSINGLE_DASHED_LINE },
	{ "DOUBLE_DASHED_LINE",		XmDOUBLE_DASHED_LINE },
	{ "SHADOW_ETCHED_IN",			XmSHADOW_ETCHED_IN },
	{ "SHADOW_ETCHED_OUT",		XmSHADOW_ETCHED_OUT },
	{ NULL,				0 }
};

static EnumNameEntry indicatorTypeEnums[] = {
	{ "N_OF_MANY",			XmN_OF_MANY },
	{ "ONE_OF_MANY",			XmONE_OF_MANY },
	{ NULL,				0 }
};

static EnumNameEntry attachmentEnums[] = {
	{ "ATTACH_NONE",			XmATTACH_NONE },
	{ "ATTACH_FORM",			XmATTACH_FORM },
	{ "ATTACH_OPPOSITE_FORM",		XmATTACH_OPPOSITE_FORM },
	{ "ATTACH_WIDGET",			XmATTACH_WIDGET },
	{ "ATTACH_OPPOSITE_WIDGET",		XmATTACH_OPPOSITE_WIDGET },
	{ "ATTACH_POSITION",			XmATTACH_POSITION },
	{ "ATTACH_SELF",			XmATTACH_SELF },
	{ NULL,				0 }
};

static EnumNameEntry toggleEnums[] = {
	{ "TOGGLE_SET",				XmSET },
	{ "TOGGLE_UNSET",			XmUNSET },
	{ "TOGGLE_INDETERMINATE",	XmINDETERMINATE },
	{ NULL,						0 }
};

static EnumNameEntry editModeEnums[] = {
	{ "EDIT_SINGLE",			XmSINGLE_LINE_EDIT },
	{ "EDIT_MULTI",				XmMULTI_LINE_EDIT },
	{ NULL,						0 }
};

static EnumNameEntry comboBoxTypeEnums[] = {
	{ "COMBO_LIST",				XmCOMBO_BOX },
	{ "COMBO_DROPEDIT",			XmDROP_DOWN_COMBO_BOX },
	{ "COMBO_DROP",				XmDROP_DOWN_LIST },
	{ NULL,						0 }
};

#ifdef HAVE_LIBVICAR

static EnumNameEntry vicarDataSavePolicyEnums[] = {
	{ "NONE",			XvicNONE },
	{ "RAW",			XvicRAW },
	{ "XIMAGE",			XvicXIMAGE },
	{ "PIXMAP",			XvicPIXMAP },
	{ NULL,				0 }
};

static EnumNameEntry vicarDataTypeEnums[] = {
	{ "BYTE",				XvicBYTE },
	{ "HALF",				XvicHALF },
	{ "UHALF",				XvicUHALF },
	{ "FULL",				XvicFULL },
	{ "UFULL",				XvicUFULL },
	{ "REAL",				XvicREAL },
	{ "DOUBLE",				XvicDOUBLE },
	{ NULL,				0 }
};

static EnumNameEntry vicarDitherModeEnums[] = {
	{ "NONE",				XvicNONE },
	{ "ORDERED",				XvicORDERED },
	{ "KAGELS",				XvicKAGELS },
	{ NULL,				0 }
};

static EnumNameEntry vicarImageModeEnums[] = {
	{ "COLOR",				XvicCOLOR },
	{ "BW",				XvicBW },
	{ NULL,				0 }
};

static EnumNameEntry vicarLutTypeEnums[] = {
	{ "STRETCH",				XvicSTRETCH },
	{ "RAW",				XvicRAW },
	{ "PSEUDO",				XvicPSEUDO },
	{ "PSEUDO_ONLY",			XvicPSEUDO_ONLY },
	{ NULL,				0 }
};

static EnumNameEntry vicarLut16TypeEnums[] = {
	{ "STRETCH",				XvicSTRETCH },
	{ "RAW",				XvicRAW },
	{ "PSEUDO",				XvicPSEUDO },
	{ "PSEUDO_ONLY",			XvicPSEUDO_ONLY },
	{ NULL,				0 }
};

static EnumNameEntry vicarStretchPolicyEnums[] = {
	{ "USE_HW",				XvicUSE_HW },
	{ "USE_SW",				XvicUSE_SW },
	{ NULL,				0 }
};

static EnumNameEntry vicarColormapPolicyEnums[] = {
	{ "FULL",				XvicFULL },
	{ "HALF",				XvicHALF },
	{ "DITHER",				XvicDITHER },
	{ "ALLOC",				XvicALLOC },
	{ "FULL_COLOR",			XvicFULL_COLOR },
	{ NULL,				0 }
};

static EnumNameEntry vicarVisualTypeEnums[] = {
	{ "USE_DEFAULT",			XvicUSE_DEFAULT },
	{ "USE_8BIT",				XvicUSE_8BIT },
	{ "USE_24BIT",			XvicUSE_24BIT },
	{ NULL,				0 }
};

static EnumNameEntry vicarWorkProcPolicyEnums[] = {
	{ "NONE",				XvicNONE },
	{ "READ",				XvicREAD },
	{ "ALL",				XvicALL },
	{ NULL,				0 }
};

#endif

static EnumTypeNameMapEntry EnumTypeNameMap[] = {
	{ XmRUnitType,				unitTypeEnums },
	{ XmRVerticalAlignment,		verticalAlignmentEnums },
	{ XmRRowColumnType,			rowColumnTypeEnums },
	{ XmRAlignment,				alignmentEnums },
	{ XtROrientation,			orientationEnums },
	{ XmRPacking,				packingEnums },
	{ XmRLabelType,				labelTypeEnums },
	{ XmRStringDirection,		stringDirectionEnums },
	{ XmRArrowDirection,		arrowDirectionEnums },
	{ XmRMultiClick,			multiClickEnums },
	{ XmRShadowType,			shadowTypeEnums },
	{ XmRSelectionPolicy,		selectionPolicyEnums },
	{ XmRListSizePolicy,		listSizePolicyEnums },
	{ XmRProcessingDirection,	processingDirectionEnums },
	{ XmRSeparatorType,			separatorTypeEnums },
	{ XmRIndicatorType,			indicatorTypeEnums },
	/* FIX: confirm XmRTearOffModel */
	{ XmRTearOffModel,			tearOffModelEnums },
	{ XmRAttachment,			attachmentEnums },
	{ XmRFileTypeMask,			fileTypeMaskEnums },
	{ XmRScrollBarDisplayPolicy,scrollBarDisplayPolicyEnums },
	{ XmRScrollBarPlacement,	scrollBarPlacementEnums },
	{ XmRScrollingPolicy,		scrollingPolicyEnums },
	{ XmRResizePolicy,			resizePolicyEnums },
	{ XmCSet,					toggleEnums },
	{ XmCEditMode,				editModeEnums },
	{ XmCComboBoxType,			comboBoxTypeEnums },
#ifdef HAVE_LIBVICAR
	{ XvicCDataSavePolicy,		vicarDataSavePolicyEnums },
	{ XvicCDataType,			vicarDataTypeEnums },
	{ XvicCDitherMode,			vicarDitherModeEnums },
	{ XvicCImageMode,			vicarImageModeEnums },
	{ XvicCLutType,				vicarLutTypeEnums },
	{ XvicCLut16Type,			vicarLut16TypeEnums },
	{ XvicCStretchPolicy,		vicarStretchPolicyEnums },
	{ XvicCColormapPolicy,		vicarColormapPolicyEnums },
	{ XvicCVisualType,			vicarVisualTypeEnums },
	{ XvicCWorkProcPolicy,		vicarWorkProcPolicyEnums },
#endif
	{ NULL,				NULL }
};

/*****************************************************************************
 *
 * Widget list hooks.  Any new widgets must be entered here.
 *
 * All new widgets must provide four functions:
 *
 * The "is" function should return 1 if a string matches a widget alias, 0
 * otherwise.
 *
 * The initialization function should create and return an Xt Widget, or NULL
 * if the widget cannot be created.  Basic widgets can use gui_initDefault()
 * instead of supplying a custom init function (see comments below at the
 * gui_initDefault() definition).  Any widgets requiring special actions at
 * creation time can use a custom init function.
 *
 * The WidgetClass function should return the Xt WidgetClass for the widget.
 *
 * The resources function should return a WidgetResources struct (see below).
 *
 *****************************************************************************/

MyWidgetList	WidgetListHead = NULL, WidgetListTail = NULL;
int		WidgetListCursor = DV_GUI_APPSHELL_WIDGETID;

typedef int		(*_is_func)(const char *);
typedef Widget		(*_init_func)(const char *, WidgetClass, Widget, Var *,
		void **, Narray *, Widget *);
typedef void		(*_destroy_func)(Widget, void *);
typedef	WidgetClass	(*_class_func)(void);
typedef CallbackList	(*_callbacks_func)(void);
typedef void		(*_get_pseudo_func)(Widget, Var *);
typedef void		(*_set_pseudo_func)(Widget, Var *, Narray *);
typedef Narray *	(*_get_pub_func)(void);

/* CallbackMap maps Davinci user callbacks to Xt callbacks. */
/* FIX: make/find a generic linked list thingy. */

#define DV_GUI_DESTROY_CALLBACK_NAME "destroy"

typedef struct _CallbackMapEntry *CallbackMapList;
typedef struct _CallbackMapEntry {
	int			widgetId;
	String		xtCallbackName;
	XtCallbackProc	callbackProc;
	const char		*dvCallbackName;
	const char		*evalString;
	CallbackMapList	next;
	CallbackMapList	prev;
} CallbackMapEntry;

CallbackMapList	CallbackMapHead = NULL, CallbackMapTail = NULL;

/* Callback handling functions. */
void gui_davinciCallback(CallbackMapEntry *);

typedef struct {
	_is_func		is;
	_init_func		init;
	_destroy_func		destroy;
	_class_func		getClass;
	_callbacks_func	getCallbacks;
	_get_pseudo_func	getPseudoResources;
	_set_pseudo_func	setPseudoResources;
	_get_pub_func		getPublicResources;
} WidgetMapEntry;

/* This should match the index of the top-level placeholder in WidgetMap. */
#define DV_GUI_APPSHELL_CLASSID 0

static WidgetMapEntry WidgetMap[] = {
	/* Placeholder for top-level widget.  It should never be instantiated.
	 * There must be an "is" function because we need something non-null, but
	 * the function always returns false.
	 */
	{
		gui_isTop,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	/* xmArrowButtonWidgetClass */
	{
		gui_isArrowButton,
		gui_initDefault,
		gui_destroyDefault,
		gui_getArrowButtonClass,
		gui_getArrowButtonCallbacks,
		NULL,
		NULL,
		gui_getArrowButtonPublicResources
	},
	/* xmFrameWidgetClass */
	{
		gui_isFrame,
		gui_initDefault,
		gui_destroyDefault,
		gui_getFrameClass,
		NULL,
		NULL,
		NULL,
		gui_getFramePublicResources
	},
	/* xmLabelWidgetClass */
	{
		gui_isLabel,
		gui_initDefault,
		gui_destroyDefault,
		gui_getLabelClass,
		NULL,
		NULL,
		NULL,
		gui_getLabelPublicResources
	},
	/* xmListWidgetClass */
	{
		gui_isList,
		gui_initDefault,
		gui_destroyDefault,
		gui_getListClass,
		gui_getListCallbacks,
		gui_getListPseudoResources,
		gui_setListPseudoResources,
		gui_getListPublicResources
	},
	/* xmScrollBarWidgetClass */
	{
		gui_isScrollBar,
		gui_initDefault,
		gui_destroyDefault,
		gui_getScrollBarClass,
		gui_getScrollBarCallbacks,
		NULL,
		NULL,
		NULL
	},
	/* xmTextWidgetClass */
	{
		gui_isText,
		gui_initDefault,
		gui_destroyDefault,
		gui_getTextClass,
		gui_getTextCallbacks,
		NULL,
		NULL,
		NULL
	},
	/* xmTextFieldWidgetClass */
	{
		gui_isTextField,
		gui_initDefault,
		gui_destroyDefault,
		gui_getTextFieldClass,
		gui_getTextFieldCallbacks,
		NULL,
		NULL,
		gui_getTextFieldPublicResources
	},
	/* xmCascadeButtonWidgetClass */
	{
		gui_isCascadeButton,
		gui_initDefault,
		gui_destroyDefault,
		gui_getCascadeButtonClass,
		gui_getCascadeButtonCallbacks,
		NULL,
		NULL,
		gui_getCascadeButtonPublicResources
	},
	/* xmComboBoxClass */
	{
		gui_isComboBox,
		gui_initDefault,
		gui_destroyDefault,
		gui_getComboBoxClass,
		gui_getComboBoxCallbacks,
		NULL,
		NULL,
		NULL
	},
	/* xmCommandWidgetClass */
	{
		gui_isCommand,
		gui_initDefault,
		gui_destroyDefault,
		gui_getCommandClass,
		gui_getCommandCallbacks,
		gui_getCommandPseudoResources,
		gui_setCommandPseudoResources,
		gui_getCommandPublicResources
	},
	/* xmDrawnButtonWidgetClass */
	{
		gui_isDrawnButton,
		gui_initDefault,
		gui_destroyDefault,
		gui_getDrawnButtonClass,
		gui_getDrawnButtonCallbacks,
		NULL,
		NULL,
		NULL
	},
	/* xmErrorDialogWidgetClass */
	{
		gui_isErrorDialog,
		gui_initDefault,
		gui_destroyDefault,
		gui_getErrorDialogClass,
		gui_getErrorDialogCallbacks,
		NULL,
		NULL,
		NULL
	},
	/* xmFileSelectionBoxWidgetClass */
	{
		gui_isFileSelectionBox,
		gui_initDefault,
		gui_destroyDefault,
		gui_getFileSelectionBoxClass,
		gui_getFileSelectionBoxCallbacks,
		gui_getFileSelectionBoxPseudoResources,
		gui_setFileSelectionBoxPseudoResources,
		gui_getFileSelectionBoxPublicResources
	},
	/* xmFormWidgetClass */
	{
		gui_isForm,
		gui_initDefault,
		gui_destroyDefault,
		gui_getFormClass,
		NULL,
		NULL,
		NULL,
		NULL
	},
	/* xmPanedWindowWidgetClass */
	{
		gui_isPanedWindow,
		gui_initDefault,
		gui_destroyDefault,
		gui_getPanedWindowClass,
		NULL,
		NULL,
		NULL,
		gui_getPanedWindowPublicResources
	},
	/* xmPushButtonWidgetClass */
	{
		gui_isPushButton,
		gui_initDefault,
		gui_destroyDefault,
		gui_getPushButtonClass,
		gui_getPushButtonCallbacks,
		NULL,
		NULL,
		gui_getPushButtonPublicResources
	},
	/* xmMenuBarWidgetClass */
	{
		gui_isMenuBar,
		gui_initMenuBar,
		gui_destroyDefault,
		gui_getMenuBarClass,
		NULL,
		NULL,
		NULL,
		NULL
	},
	/* RadioBox */
	{
		gui_isRadioBox,
		gui_initRadioBox,
		gui_destroyDefault,
		gui_getRadioBoxClass,
		gui_getRadioBoxCallbacks,
		NULL,
		NULL,
		gui_getRadioBoxPublicResources
	},
	/* xmRowColumnWidgetClass */
	{
		gui_isRowColumn,
		gui_initDefault,
		gui_destroyDefault,
		gui_getRowColumnClass,
		gui_getRowColumnCallbacks,
		NULL,
		NULL,
		gui_getRowColumnPublicResources
	},
	/* xmScaleWidgetClass */
	{
		gui_isScale,
		gui_initDefault,
		gui_destroyDefault,
		gui_getScaleClass,
		gui_getScaleCallbacks,
		NULL,
		NULL,
		gui_getScalePublicResources
	},
	/* ScrolledList */
	{
		gui_isScrolledList,
		gui_initScrolledList,
		gui_destroyDefault,
		gui_getScrolledListClass,
		gui_getScrolledListCallbacks,
		gui_getScrolledListPseudoResources,
		gui_setScrolledListPseudoResources,
		gui_getScrolledListPublicResources
	},
	/* xmScrolledWindowWidgetClass */
	{
		gui_isScrolledWindow,
		gui_initDefault,
		gui_destroyDefault,
		gui_getScrolledWindowClass,
		gui_getScrolledWindowCallbacks,
		NULL,
		NULL,
		gui_getScrolledWindowPublicResources
	},
	/* xmSelectionBoxWidgetClass */
	{
		gui_isSelectionBox,
		gui_initDefault,
		gui_destroyDefault,
		gui_getSelectionBoxClass,
		gui_getSelectionBoxCallbacks,
		gui_getSelectionBoxPseudoResources,
		gui_setSelectionBoxPseudoResources,
		gui_getSelectionBoxPublicResources
	},
	/* xmSeparatorWidgetClass */
	{
		gui_isSeparator,
		gui_initDefault,
		gui_destroyDefault,
		gui_getSeparatorClass,
		NULL,
		NULL,
		NULL,
		gui_getSeparatorPublicResources
	},
	/* xmToggleButtonWidgetClass */
	{
		gui_isToggleButton,
		gui_initDefault,
		gui_destroyDefault,
		gui_getToggleButtonClass,
		gui_getToggleButtonCallbacks,
		NULL,
		NULL,
		gui_getToggleButtonPublicResources
	},
	/* topLevelShellWidgetClass */
	{
		gui_isTopLevelShell,
		gui_initTopLevelShell,
		gui_destroyDefault,
		gui_getTopLevelShellClass,
		NULL,
		NULL,
		NULL,
		gui_getTopLevelShellPublicResources
	},
	/* transientShellWidgetClass */
	{
		gui_isTransientShell,
		gui_initTransientShell,
		gui_destroyDefault,
		gui_getTransientShellClass,
		NULL,
		NULL,
		NULL,
		gui_getTransientShellPublicResources
	},
#ifdef PLOT_WIDGETS
	/* xyWidgetClass */
	{
		gui_isXYPlot,
		gui_initDefault,
		gui_destroyDefault,
		gui_getXYPlotClass,
		NULL,
		NULL,
		gui_setXYPlotPseudoResources,
		NULL
	},
#endif
	/* lineBoxWidgetClass */
	{
		gui_isLineBox,
		gui_initLineBoxWidget,
		gui_destroyDefault,
		gui_getLineBoxClass,
		gui_getLineBoxCallbacks,
		gui_getLineBoxPseudoResources,
		gui_setLineBoxPseudoResources,
		gui_getLineBoxPublicResources
	},
#if 0
	/* xbaeMatrixWidgetClass */
	{
		gui_isXbaeMatrix,
		gui_initDefault,
		gui_destroyDefault,
		gui_getXbaeMatrixClass,
		NULL,
		NULL,
		NULL,
		gui_getXbaeMatrixPublicResources
	},
#endif
#ifdef HAVE_LIBVICAR
	/* xvicImageWidgetClass */
	{
		gui_isVicar,
		gui_initVicar,
		gui_destroyVicar,
		gui_getVicarClass,
		gui_getVicarCallbacks,
		gui_getVicarPseudoResources,
		gui_setVicarPseudoResources,
		gui_getVicarPublicResources
	},
#endif
	{ NULL, NULL, NULL, NULL },
};

/*****************************************************************************
 *
 * Davinci parser hooks.  Maps module functions in Davinci to real functions
 * defined in this file.  See the module initialization function defined below.
 *
 *****************************************************************************/

#define DV_MODULE_FUNC_COUNT 8 /* NOTE! If export_funcs changes, so must this */

static dvModuleFuncDesc export_funcs [] = {
	{ "create", 		dv_CreateWidget },
	{ "destroy",		dv_DestroyWidget },
	/* { "wait",    		dv_XEventWait }, FIX: fix this or remove? */
	{ "get",     		dv_XGet },
	{ "set",     		dv_XSet },
	{ "realize", 		dv_Realize },
	{ "addcallback", 	dv_AddCallback },
	{ "listcallbacks", 	dv_ListCallbacks },
	{ "removecallback",	dv_RemoveCallback },
#if 0
	{ "show",    		dv_WidgetShow },
	{ "hide",    		dv_WidgetHide },
#endif
	{ NULL,      		NULL }
};

/* Davinci does not need to load any dependencies.. */

static dvDepAttr deps[] = {
	{ NULL, NULL }
};

/* Structure describing the entry points and function names for the parser. */
/* FIX: i don't like this struct name.  in fact, i don't even know if it's a
 * struct, nor do i know what this is doing.  the magic numbers have got to go.
 * this should be handled inside dv_module_init.
 */

static dvModuleInitStuff dvinit = {
	export_funcs, DV_MODULE_FUNC_COUNT, deps, 0
};

/* Parser hook prototypes that rely on the definitions above. */

int		dv_module_init(const char *, dvModuleInitStuff *);
void	dv_module_fini(const char *);

/*****************************************************************************
 *
 * FUNCTION PROTOTYPES (more above)
 *
 *****************************************************************************/

/* Callback/event functions. */

/* FIX: moved to gui.h; uncomment this if it is not needed externally. */
/* void gui_defaultCallback(Widget, XtPointer, XtPointer); */

#if 0
static void		registerCallbacks(Widget, CallbackList); /* FIX: remove this proc and supporting procs */
#endif
static EventQueuePtr	inspectEvents(EventWatchList);
static Var *		interpretEvent(EventQueuePtr);
static void		clearEventWatchList(EventWatchList);

/*****************************************************************************
 *
 * FUNCTION DEFINITIONS
 *
 *****************************************************************************/

/* debug helper */
void dbgprintf (char *fmt, ...)
{
#ifdef DEBUG
	va_list list;
	va_start (list, fmt);
	vsprintf (stderr, "DEBUG: ");
	vsprintf (stderr, fmt, list);
#endif
}

/*****************************************************************************
 *
 * Module initialization/destruction hooks.
 *
 * dv_module_init is called by DaVinci when you load the module.
 * The second argument to this subroutine passes back the pointer
 * to initialization stucture defined above.  This module is analogous
 * to _init() when you do C-style shared libraries, so it is the perfect
 * place to initialize anything the module will require for the duration.
 *
 *****************************************************************************/

int
dv_module_init(const char *modname, dvModuleInitStuff *ini)
{
	MyWidgetList		widgetListEntry;

	/* Add the application context. */
	widgetListEntry = (MyWidgetList) malloc(sizeof(WidgetListEntry));
	if (widgetListEntry ==  NULL) {
		return 0;
	}

	WidgetListHead = WidgetListTail = widgetListEntry;
	widgetListEntry->id = WidgetListCursor++;
	widgetListEntry->widget = applicationShell;
	widgetListEntry->widgetClassId = DV_GUI_APPSHELL_CLASSID;
	widgetListEntry->widgetClass = NULL;
	widgetListEntry->next = NULL;

	*ini = dvinit;
	return 1;
}

void
dv_module_fini(const char *modname)
{
	parse_error("%s Unloaded.", modname);
}

/* int
 * gui_isTop(name)
 *
 * name		Davinci widget type alias.
 *
 * Placeholder function for the top-level widget (application).
 * This always returns 0 because it shouldn't be instantiated.
 * top is created externally.
 *
 */

static int
gui_isTop(const char *name)
{
	return 0;
}

/*****************************************************************************
 *
 * Event-handling/Xt callback functions
 *
 *****************************************************************************/

/* EventQueuePtr
 * inspectEvents(eventList)
 *
 * eventList	List of events we're waiting for.
 *
 * Examines the event queue to see if any of the events received are ones
 * we've been waiting for.
 *
 * Returns: FIX: I think it returns a list of events that have occurred.
 *               Maybe it only returns one event though.
 *
 */

/* FIX: EventQueuePtr and EventWatchList are suk names. */
static EventQueuePtr
inspectEvents(EventWatchList eventList)
{

	EventQueuePtr		currentEvent;
	EventWatchList	eventListEntry;

	if (EventQueueHead == NULL) {
		/* Queue is empty. */
		return NULL;
	}

	/* Grab the first event off the queue. */
	currentEvent = EventQueueHead;

	if (EventQueueTail == EventQueueHead) {
		/* This is the only event, reset the tail. */
		EventQueueTail = NULL;
	}

	/* Pop the event off the queue (move the head up). */
	EventQueueHead = EventQueueHead->next;

	if (eventList == NULL) {
		/* We want to see all events, return the event. */
		return currentEvent;
	}

	/* Check this event against the watchlist. */

	eventListEntry = eventList;
	while (eventListEntry != NULL) {
		if (currentEvent->widget == getWidgetFromId(eventListEntry->dvWidgetId)) {
			return currentEvent;
		}
		eventListEntry = eventListEntry->next;
	}

	/* Not waiting for this event; discard it. */
	free(currentEvent);

	return NULL;
}

/* Var *
 * interpretEvent(event)
 *
 * event	The event that occurred.
 *
 * Turns an Xt event into a Davinci struct describing the event.
 *
 * Returns: Davinci struct with Davinci widget Id and event name.
 *
 */

static Var *
interpretEvent(EventQueuePtr event) {

	Var	*returnStruct, *returnWidgetId, *returnEventName;
	int	dvWidgetId;

	/* Build a new Davinci struct to return. */
	returnStruct = new_struct(0);

	dvWidgetId = getIdFromWidget(event->widget);
	if (dvWidgetId == DV_INVALID_WIDGET_ID) {
		/* This isn't supposed to be possible. */
		parse_error("Event generated by non-existant widget!");
		return NULL;
	}

	returnWidgetId = newInt(dvWidgetId);
	add_struct(returnStruct, "widget", returnWidgetId);

	returnEventName = newString(dupString(event->event));
	add_struct(returnStruct, "event", returnEventName);

	return returnStruct;
}

/* void
 * clearEventWatchList(watchList)
 *
 * watchList	List of all widgets we are waiting for events on.
 *
 * Clears the entire widget watchlist out.  Called before returning
 * from gui.wait().  User must supply the watchlist again on subsequent calls.
 *
 */

static void
clearEventWatchList(EventWatchList watchList)
{
	EventWatchList	currentEvent, previousEvent;

	/* Iterate over watchList and free each event. */

	currentEvent = watchList;

	while (currentEvent != NULL) {
		previousEvent = currentEvent;
		currentEvent = currentEvent->next;
		free(previousEvent);
	}
}

/* void
 * gui_defaultCallback(widget, clientData, callData)
 *
 * widget	Widget instance.
 * clientData	Xt callback client data (callback name).
 * callData	Xt callback call data (unused).
 *
 * Default callback function for widget callbacks.  Passed to XtAddCallback()
 * during widget initialization.  This function just sticks a new event on the
 * queue, which will be handled later.
 *
 */

void
gui_defaultCallback(Widget widget, XtPointer clientData, XtPointer callData)
{

	CallbackMapEntry	*callbackMapEntry, *callbackMapPtr;

	dbgprintf ("gui_defaultCallback(widget = %ld, clientData = %ld, callData = %ld)\n",
			widget, clientData, callData);

#if 0

	/* FIX: remove */
	gui_addEvent(widget, (String) clientData);

#else

	/* Use Davinci to eval() the user-supplied string.  This code is based on ff_eval() in ff.c. */

	callbackMapEntry = (CallbackMapEntry *) clientData;

	callbackMapPtr = CallbackMapHead;
	while (callbackMapPtr != NULL) {
		if (callbackMapPtr == callbackMapEntry) {
			gui_davinciCallback(callbackMapEntry);
			break;
		}
		callbackMapPtr = callbackMapPtr->next;
	}

	if (callbackMapPtr == NULL) {
		parse_error("INTERNAL ERROR: gui_defaultCallback() called with unknown CallbackMapEntry.");
	}

#endif

	return;

}

void
gui_pseudoCallback(Widget widget, String callbackName)
{

	int			widgetId;
	CallbackMapEntry	*callbackMapEntry;

	dbgprintf ("entering gui_pseudoCallback(%ld, '%s')\n", widget, callbackName);

	widgetId = getIdFromWidget(widget);
	if (widgetId == DV_INVALID_WIDGET_ID) {
		parse_error("INTERNAL ERROR: gui_pseudoCallback() called with invalid widget.");
		return;
	}

	callbackMapEntry = CallbackMapHead;
	while (callbackMapEntry != NULL) {
		if (callbackMapEntry->widgetId == widgetId) {
			if (!strcasecmp(callbackMapEntry->dvCallbackName, callbackName)) {
				dbgprintf ("calling davinci pseudo-callback '%s'\n", callbackName);
				gui_davinciCallback(callbackMapEntry);
				break;
			}
		}
		callbackMapEntry = callbackMapEntry->next;
	}

	dbgprintf ("leaving gui_pseudoCallback()\n");
}

void
gui_davinciCallback(CallbackMapEntry *callbackMapEntry)
{
	dbgprintf ("entering gui_davinciCallback(%ld)\n", callbackMapEntry);

	if (callbackMapEntry != NULL && callbackMapEntry->evalString != NULL) {
#if 0
		eval_buffer(dupString(callbackMapEntry->evalString)); /* FIX: confirm that string is freed */
		(void) pop(scope_tos()); /* FIX: confirm this, maybe free it.. */
#else
		parse_buffer(dupString(callbackMapEntry->evalString)); /* FIX: confirm that string is freed */
#endif
	}
	else {
		parse_error("INTERNAL ERROR: NULL in gui_davinciCallback().");
	}
}


void
gui_addEvent(Widget widget, String eventName)
{

	EventQueuePtr	newEvent;

	dbgprintf ("gui_addEvent(widget = %ld, eventName = '%s')\n", widget, eventName);

	newEvent = (EventQueuePtr) malloc(sizeof(EventQueueEntry));
	if (newEvent == NULL) {
		parse_error("Error: unable to allocate memory for Xt event.");
		return;
	}

	newEvent->widget = widget;
	newEvent->event = eventName;
	newEvent->next = NULL;

	if (EventQueueTail != NULL) {
		EventQueueTail->next = newEvent;
		EventQueueTail = newEvent;
	} else {
		/* First event. */
		EventQueueHead = EventQueueTail = newEvent;
	}
}

#if 0

/* FIX: implement */

	static Var *
dv_XEventFlush(vfuncptr f, Var *args)
{

	Alist		alist[2];
	Var		*dvWidgetId;
	int		widgetId;
	MyWidgetList	widgetListEntry;

	alist[0] = make_alist("parent", INT, NULL, &dvWidgetId);
	alist[1].name = NULL;

	if (parse_args(f, args, alist) == 0) {
		parse_error("Usage: gui.eventflush([widget]).");
		return newInt(0);
	}

	widgetId = extract_int(dvWidgetId, 0);
	if (widgetId == DV_INVALID_WIDGET_ID) {
		parse_error("Error: unknown widget id.");
		return newInt(0);
	}

}

#endif

void
gui_defaultDestroyCallback(Widget widget,
		XtPointer clientData, XtPointer callData)
{

	MyWidgetList		widgetListEntry;
	CallbackMapEntry	*callbackMapEntry;

	dbgprintf ("gui_defaultDestroyCallback(widget = %ld, "
			"clientData = %ld, callData = %ld)\n", widget, clientData, callData);

#if 0
	gui_addEvent(widget, "destroy");
	gui_invalidateWidget(widget);
#else

	/* Check for a user 'destroy' callback and call if present. */

	widgetListEntry = gui_getWidgetListEntryFromWidget(widget);
#if 0
	if (widgetListEntry == NULL) {
		/* FIX: not sure if this is really an error condition; it might happen during cascading destroys. */
		parse_error("INTERNAL ERROR: NULL widgetListEntry in gui_defaultDestroyCallback; cannot call user callback.");
	}
	else {
#else
		if (widgetListEntry != NULL) {
#endif
			callbackMapEntry = CallbackMapHead;
			while (callbackMapEntry != NULL) {
				if (callbackMapEntry->widgetId == widgetListEntry->id) {
					if (!strcasecmp(callbackMapEntry->dvCallbackName, DV_GUI_DESTROY_CALLBACK_NAME)) {
						dbgprintf ("calling davinci destroy callback\n");
						gui_davinciCallback(callbackMapEntry);
						break;
					}
				}
				callbackMapEntry = callbackMapEntry->next;
			}
		}

		dbgprintf ("destroying widget\n");
		/* FIX: make sure the callbacks are removed from the callback map */
		gui_destroyWidget(widget);

#endif

		return;

	}

#if 0

/* void
* registerCallbacks(widget, callBacks)
*
* widget	Widget instance.
* callBacks	CallbackList of callbacks to register.
*
* Registers all the callbacks in the CallbackList for the supplied Widget
* instance, with the Davinci callback name as the client data.
*
*/

void
registerCallbacks(Widget widget, CallbackList callBacks)
{

	int	i;

	dbgprintf ("registerCallbacks(widget = %ld, "
			"callBacks = %ld)\n", widget, callBacks);

	if (callBacks != NULL) {
		i = 0;
		while (callBacks[i].dvCallbackName != NULL) {
			/* FIX: pushbutton activate? */
			dbgprintf ("registering '%s'\n", callBacks[i].xtCallbackName);
			XtAddCallback(widget, callBacks[i].xtCallbackName, callBacks[i].callback,
					callBacks[i].dvCallbackName);
			i++;
		}
	}

	/* Add the destroy callback to all widgets. */
	XtAddCallback(widget, XtNdestroyCallback, gui_defaultDestroyCallback, NULL);

	return;

}

#endif

/*****************************************************************************
*
* Misc. functions to map Davinci GUI IDs to Widget/WidgetClass instances.
*
*****************************************************************************/

/* Widget
* getWidgetFromId(id)
*
* id		Davinci widget id.
*
* Returns the Widget instance associated with the Davinci widget id.
*
*/

static Widget
getWidgetFromId(int id) {

	MyWidgetList	cur = WidgetListHead;

	while (cur != NULL) {
		if (cur->id == id) {
			return cur->widget;
		}
		cur = cur->next;
	}

	return (Widget)NULL;

}

#if 0

static MyWidgetList
getWidgetListEntryFromId(int id) {

	MyWidgetList	cur = WidgetListHead;

	while (cur != NULL) {
		if (cur->id == id) {
			return cur;
		}
		cur = cur->next;
	}

	return NULL;

}

#endif

/* int
* getIdFromWidget(widget)
*
* widget	Xt Widget instance.
*
* Returns the Davinci widget Id associated with the Xt Widget (opposite
* of getWidgetFromId()).
*
* Returns: the Davinci widget Id, or DV_INVALID_WIDGET_ID if not found.
*
*/

static int
getIdFromWidget(Widget widget) {

	MyWidgetList	cur = WidgetListHead;

	while (cur != NULL) {
		if (cur->widget == widget) {
			return cur->id;
		}
		cur = cur->next;
	}

	return DV_INVALID_WIDGET_ID;

}

/* Widget
* getWidgetClassFromId(id)
*
* id		Davinci widget id.
*
* Returns the WidgetClass of the widget instance associated with the Davinci
* widget id.
*
*/

static WidgetClass
getWidgetClassFromId(int id) {

	MyWidgetList	cur = WidgetListHead;

	while (cur != NULL) {
		if (cur->id == id) {
			return cur->widgetClass;
		}
		cur = cur->next;
	}

	return (WidgetClass)NULL;

}

/* MyWidgetList
* gui_getWidgetListEntryFromWidget(widget)
*
* widget	A Widget instance.
*
* Returns a pointer to the entire WidgetListEntry associated with the widget
* instance.
*
*/

MyWidgetList
gui_getWidgetListEntryFromWidget(Widget widget) {

	MyWidgetList	cur = WidgetListHead;

	while (cur != NULL) {
		if (cur->widget == widget) {
			return cur;
		}
		cur = cur->next;
	}

	return NULL;

}

/* int
* getWidgetClassIdByName(className)
*
* className	A string containing a widget class name/alias.
*
* Returns the widget class id (an internal identifier, really just the array
* index in WidgetMap) for the given className.  The className is looked up in
* the alias list for each known widget class until a match is found.
*
*/

static int
getWidgetClassIdByName(const char *className) {

	int	i;
	int	classId = -1;

	dbgprintf ("getWidgetClassIdByName(className = \"%s\")\n", className);

	i = 0;
	while (WidgetMap[i].is != NULL) {
		if ((*WidgetMap[i].is)(className)) {
			break;
		}
		i++;
	}

	if (WidgetMap[i].is != NULL) {
		classId = i;
	}

	dbgprintf ("classId = %d\n", classId);

	return classId;

}

/*****************************************************************************
*
* Davinci parser entry points.
*
*****************************************************************************/

/* FIX: document */

static Var *
dv_AddCallback(vfuncptr f, Var *args)
{

	Alist			alist[4];
	int			widgetId;
	Widget		widget;
	const char *		userCallbackName;
	const char *		evalString;
	char *		tmpString;
	CallbackList		callbacks;
	MyWidgetList		widgetListEntry;
	int			widgetClassId;
	Boolean		callbackFound;
	CallbackMapEntry	*callbackMapEntry;
	int			i;

	dbgprintf ("entering dv_AddCallback()\n");

	widgetId = DV_INVALID_WIDGET_ID;

	alist[0] = make_alist("widgetid", INT, NULL, &widgetId);
	alist[1] = make_alist("callback", ID_STRING, NULL, &userCallbackName);
	alist[2] = make_alist("eval", ID_STRING, NULL, &evalString);
	alist[3].name = NULL;

	if (parse_args(f, args, alist) == 0) {
		parse_error("Usage: a = gui.addcallback(widgetid, callback, eval).");
		return newInt(-1);
	}

	if (widgetId == DV_INVALID_WIDGET_ID) {
		parse_error("widgetid is required.");
		return newInt(-1);
	}

	widget = getWidgetFromId(widgetId);
	if (widget == NULL) {
		parse_error("Invalid widget id.");
		return NULL;
	}

	if (!strlen(userCallbackName)) {
		parse_error("callback is required.");
		return NULL;
	}

	if (!strlen(evalString)) {
		parse_error("eval is required.");
		return NULL;
	}

	/* Make sure the callback doesn't already exist. */

	callbackMapEntry = CallbackMapHead;
	while (callbackMapEntry != NULL) {
		if (callbackMapEntry->widgetId == widgetId && !strcasecmp(callbackMapEntry->dvCallbackName, userCallbackName)) {
			parse_error("Callback already registered for widget.");
			return NULL;
		}
		callbackMapEntry = callbackMapEntry->next;
	}

	widgetListEntry = gui_getWidgetListEntryFromWidget(widget);
	widgetClassId = widgetListEntry->widgetClassId;

	/* Add a newline to the end of evalString, for the Davinci eval() parser. */
	tmpString = calloc(strlen(evalString) + 3, 1);
	if (tmpString == NULL) {
		parse_error("ERROR: out of memory trying to add CallBackMap entry.");
		return NULL;
	}
	strcpy(tmpString, evalString);
	strcat(tmpString, "\n");
	evalString = tmpString;

	callbackFound = False;

	/* Special case for destroy callbacks. */
	/* FIX: implement this with pseudo-callbacks in CallbackList? */

	if (!strcasecmp(userCallbackName, DV_GUI_DESTROY_CALLBACK_NAME)) {

		dbgprintf ("adding destroy callback\n");

		callbackFound = True;

		/* Add callback to CallbackMap. */

		callbackMapEntry = (CallbackMapEntry *) malloc(sizeof(CallbackMapEntry));
		if (callbackMapEntry == NULL) {
			parse_error("ERROR: out of memory trying to add CallBackMap entry.");
			free((void *) evalString);
			return NULL;
		}
		callbackMapEntry->widgetId = widgetId;
		callbackMapEntry->xtCallbackName = NULL;
		callbackMapEntry->dvCallbackName = DV_GUI_DESTROY_CALLBACK_NAME;
		callbackMapEntry->callbackProc = NULL;
		callbackMapEntry->evalString = evalString;
		callbackMapEntry->widgetId = widgetId;
		callbackMapEntry->prev = CallbackMapTail;
		callbackMapEntry->next = NULL;
		if (CallbackMapHead == NULL) {
			CallbackMapHead = CallbackMapTail = callbackMapEntry;
		}
		else {
			CallbackMapTail->next = callbackMapEntry;
			CallbackMapTail = callbackMapEntry;
		}

		dbgprintf ("callback added\n");
	}
	else {

		/* Look up the callback name in the widget's callback list. */

		callbacks = NULL;
		if (WidgetMap[widgetClassId].getCallbacks != NULL) {
			callbacks = (*WidgetMap[widgetClassId].getCallbacks)();
		}

		if (callbacks != NULL) {
			for (i=0; callbacks[i].dvCallbackName != NULL; i++) {
				if (!strcasecmp(callbacks[i].dvCallbackName, userCallbackName)) {
					/* Add callback to CallbackMap. */
					callbackMapEntry = (CallbackMapEntry *) malloc(sizeof(CallbackMapEntry));
					if (callbackMapEntry == NULL) {
						parse_error("ERROR: out of memory trying to add CallBackMap entry.");
						free((void *) evalString);
						return NULL;
					}
					callbackMapEntry->widgetId = widgetId;
					callbackMapEntry->xtCallbackName = callbacks[i].xtCallbackName;
					callbackMapEntry->dvCallbackName = callbacks[i].dvCallbackName;
					callbackMapEntry->callbackProc = callbacks[i].callback;
					callbackMapEntry->evalString = evalString;
					callbackMapEntry->widgetId = widgetId;
					callbackMapEntry->prev = CallbackMapTail;
					callbackMapEntry->next = NULL;
					if (CallbackMapHead == NULL) {
						CallbackMapHead = CallbackMapTail = callbackMapEntry;
					}
					else {
						CallbackMapTail->next = callbackMapEntry;
						CallbackMapTail = callbackMapEntry;
					}
					if (callbacks[i].xtCallbackName != NULL) {
						XtAddCallback(widget, callbacks[i].xtCallbackName, callbacks[i].callback,
								(XtPointer) callbackMapEntry);
					}
					/* else it's a directly-called pseudo-callback. */
					callbackFound = True;
					break;
				}
			}

			/* report callbacks available */
			if (callbackFound == False) {
				parse_error ("Callback '%s' not found, choose from:", userCallbackName);
				for (i=0; callbacks[i].dvCallbackName != NULL; i++) {
					parse_error ("\t%s", callbacks[i].dvCallbackName);
				}
			}
		} else {
			parse_error ("No callbacks defined for that widget");
		}
	}

	if (callbackFound == True) {
		return newInt(1);
	}
	else {
		return newInt(0);
	}
}

static Var *
dv_ListCallbacks(vfuncptr f, Var *args)
{
	Alist			alist[2];
	int			widgetId;
	Widget		widget;
	MyWidgetList		widgetListEntry;
	int			widgetClassId;
	CallbackList		callbacks;
	Var			*dvCallbackList;
	CallbackMapEntry	*callbackMapEntry;
	Var			*dvEvalString;
	Boolean		foundEval;
	int			i, len;
	char			*tmpString;

	dbgprintf ("entering dv_AddCallback()\n");

	widgetId = DV_INVALID_WIDGET_ID;

	alist[0] = make_alist("widgetid", INT, NULL, &widgetId);
	alist[1].name = NULL;

	if (parse_args(f, args, alist) == 0) {
		parse_error("Usage: a = gui.listcallbacks(widgetid).");
		return newInt(-1);
	}

	if (widgetId == DV_INVALID_WIDGET_ID) {
		parse_error("widgetid is required.");
		return newInt(-1);
	}

	widget = getWidgetFromId(widgetId);
	if (widget == NULL) {
		parse_error("Invalid widget id.");
		return NULL;
	}

	widgetListEntry = gui_getWidgetListEntryFromWidget(widget);
	widgetClassId = widgetListEntry->widgetClassId;

	/* Struct to return to user. */
	dvCallbackList = new_struct(0);

	/* Check for the destroy callback first. */

	foundEval = False;
	callbackMapEntry = CallbackMapHead;

	while (callbackMapEntry != NULL) {
		if (callbackMapEntry->widgetId == widgetId &&
				!strcmp(callbackMapEntry->dvCallbackName, DV_GUI_DESTROY_CALLBACK_NAME)) {
			if (callbackMapEntry->evalString != NULL) {
				dvEvalString = newString(dupString(callbackMapEntry->evalString));
			}
			else {
				/* This shouldn't happen, but just in case.. */
				dbgprintf ("null eval string in callback map!\n");
				dvEvalString = newString(dupString("<null>"));
			}
			add_struct(dvCallbackList, DV_GUI_DESTROY_CALLBACK_NAME, dvEvalString);
			foundEval = True;
			break;
		}
		callbackMapEntry = callbackMapEntry->next;
	}

	if (foundEval == False) {
		add_struct(dvCallbackList, DV_GUI_DESTROY_CALLBACK_NAME, newString(dupString("<default>")));
	}

	/* For each callback defined for widgets of this class, get the current eval string (if any) and add it
	 * to the return struct.  If no eval string has been set for a given callback, return "<default>".  If a
	 * callback map entry has a null eval string, "<null>" is returned, but that should never happen.
	 */

	if (WidgetMap[widgetClassId].getCallbacks != NULL) {
		callbacks = (*WidgetMap[widgetClassId].getCallbacks)();

		i = 0;
		while (callbacks[i].dvCallbackName != NULL) {
			foundEval = False;
			callbackMapEntry = CallbackMapHead;
			while (callbackMapEntry != NULL) {
				if (callbackMapEntry->widgetId == widgetId &&
						!strcmp(callbackMapEntry->dvCallbackName, callbacks[i].dvCallbackName)) {
					if (callbackMapEntry->evalString != NULL) {
						/* Removing the trailing newline that dv_AddCallback() added. */
						len = strlen(callbackMapEntry->evalString);
						tmpString = (char *) calloc(len, sizeof(char)); /* Don't free; normally we'd dupString() anyway. */
						strncpy(tmpString, callbackMapEntry->evalString, len - 1);
						dvEvalString = newString(tmpString);
					}
					else {
						/* This shouldn't happen, but just in case.. */
						dbgprintf ("null eval string in callback map!\n");
						dvEvalString = newString(dupString("<null>"));
					}
					add_struct(dvCallbackList, callbackMapEntry->dvCallbackName, dvEvalString);
					foundEval = True;
					break;
				}
				callbackMapEntry = callbackMapEntry->next;
			}

			if (foundEval == False) {
				add_struct(dvCallbackList, callbacks[i].dvCallbackName, newString(dupString("<default>")));
			}

			i++;
		}
	}

	return dvCallbackList;
}

static Var *
dv_RemoveCallback(vfuncptr f, Var *args)
{

	Alist			alist[3];
	int			widgetId;
	Widget		widget;
	const char *		userCallbackName;
	MyWidgetList		widgetListEntry;
	int			widgetClassId;
	Boolean		callbackFound;
	CallbackMapEntry	*callbackMapEntry;

	widgetId = DV_INVALID_WIDGET_ID;

	alist[0] = make_alist("widgetid", INT, NULL, &widgetId);
	alist[1] = make_alist("callback", ID_STRING, NULL, &userCallbackName);
	alist[2].name = NULL;

	if (parse_args(f, args, alist) == 0) {
		parse_error("Usage: a = gui.addcallback(widgetid, callback, eval).");
		return newInt(-1);
	}

	if (widgetId == DV_INVALID_WIDGET_ID) {
		parse_error("widgetid is required.");
		return newInt(-1);
	}

	widget = getWidgetFromId(widgetId);
	if (widget == NULL) {
		parse_error("Invalid widget id.");
		return NULL;
	}

	if (!strlen(userCallbackName)) {
		parse_error("callback is required.");
		return NULL;
	}

	/* Make sure the callback exists. */

	callbackMapEntry = CallbackMapHead;
	while (callbackMapEntry != NULL) {
		if (callbackMapEntry->widgetId == widgetId && !strcasecmp(callbackMapEntry->dvCallbackName, userCallbackName)) {
			callbackFound = True;
			break;
		}
		callbackMapEntry = callbackMapEntry->next;
	}

	if (callbackMapEntry == NULL) {
		parse_error("Callback not registered for widget.");
		return NULL;
	}

	widgetListEntry = gui_getWidgetListEntryFromWidget(widget);
	widgetClassId = widgetListEntry->widgetClassId;

	/* Remove from CallbackMap. */

	if (callbackMapEntry->next != NULL) {
		callbackMapEntry->next->prev = callbackMapEntry->prev;
	}
	else {
		CallbackMapTail = callbackMapEntry->prev;
	}

	if (callbackMapEntry->prev != NULL) {
		callbackMapEntry->prev->next = callbackMapEntry->next;
	}
	else {
		CallbackMapHead = callbackMapEntry->next;
	}

	/* Remove Xt callback. */

	if (callbackMapEntry->xtCallbackName != NULL) {
		XtRemoveCallback(widgetListEntry->widget, callbackMapEntry->xtCallbackName, callbackMapEntry->callbackProc,
				(XtPointer) callbackMapEntry);
	}
	/* else it's a pseudo-callback. */

	/* Free up the entry. */

	if (callbackMapEntry->evalString != NULL) {
		free((void *) callbackMapEntry->evalString);
	}
	else {
		parse_error("INTERNAL ERROR: NULL evalString in CallbackMap");
	}

	free(callbackMapEntry);

	return newInt(1);

}

/* Var *
* dv_CreateWidget(f, args)
*
* FIX: f, args
* FIX: start widgets at 1, return 0 on error
*
* Implements Davinci gui.create(type, label)
*	type	Widget type (Xt widget class name or short alias).
*	label	User-defined label/name for the widget.
*
*/

static Var *
dv_CreateWidget(vfuncptr f, Var *args)
{

	Alist			alist[5];
	char 			*dvWidgetClass, *dvName;
	int 			parentId;
	Var				*dvResources = NULL;
	Var				*tmpDvResources = NULL;
	MyWidgetList	widgetListEntry;
	Widget			parentWidget, newWidget;
	WidgetClass		widgetClass;
	int				widgetClassId;
#if 0
	CallbackList		callbacks;
#endif
	FreeStackListEntry	freeStack;
	Arg				xtArgs[DV_MAX_XT_ARGS];

	dvWidgetClass = NULL;
	parentId = DV_GUI_APPSHELL_WIDGETID;
	tmpDvResources = NULL;
	dvName = NULL;
	alist[0] = make_alist("widgetclass", ID_STRING, NULL, &dvWidgetClass);
	alist[1] = make_alist("parent", INT, NULL, &parentId);
	alist[2] = make_alist("resources", ID_STRUCT, NULL, &tmpDvResources);
	alist[3] = make_alist("name", ID_STRING, NULL, &dvName);
	alist[4].name = NULL;

	if (parse_args(f, args, alist) == 0) {
		parse_error("Usage: a = gui.create(widgetclass,[,parent,resources,name]).");
		return newInt(-1);
	}

	if (dvWidgetClass == NULL) {
		parse_error("widgetclass is required.");
		return newInt(-1);
	}

	if ((widgetClassId = getWidgetClassIdByName(dvWidgetClass)) < 0) {
		parse_error("Undefined widgetclass '%s'.", dvWidgetClass);
		return newInt(-1);
	}

	/* Get the Xt Widget object associated with the parent Id (if any). */
	parentWidget = getWidgetFromId(parentId);

	if (parentWidget == NULL) {
		parse_error("Invalid parent widget id.");
		return newInt(-1);
	}

	/* Create a new object to enter into the widget map. */

	widgetListEntry = (MyWidgetList) malloc(sizeof(WidgetListEntry));

	if (widgetListEntry ==  NULL) {
		return newInt(-1);
	}

	widgetListEntry->id = WidgetListCursor++;
	widgetListEntry->widgetClassId = widgetClassId;
	widgetListEntry->data = NULL;
	widgetListEntry->dvData = NULL;

	/* Set the public resources list. */
	if (WidgetMap[widgetClassId].getPublicResources != NULL) {
		/* Default list provided by widget. */
		widgetListEntry->publicResources = (*WidgetMap[widgetClassId].getPublicResources)();
	}
	else {
		/* Start with a blank list. */
		widgetListEntry->publicResources = Narray_create(0);
	}

	widgetClass = (*WidgetMap[widgetClassId].getClass)();
	widgetListEntry->widgetClass = widgetClass;

	/* Initialize widget class so that resources can be checked properly. */
	XtInitializeWidgetClass(widgetClass);

	/* Make sure we control the dvResources struct. */

	if (tmpDvResources != NULL) {
		if (mem_claim(tmpDvResources) != NULL) {
			/* We got it. */
			dvResources = tmpDvResources;
		}
		else {
			/* We didn't get it, clone it. */
			dvResources = V_DUP(tmpDvResources);
		}
	}

	/* Create the real X Widget object, and do any special initialization. */
	/* Optional instance data can be stored in widgetListEntry->data.      */

	widgetListEntry->outerWidget = NULL;

	newWidget = (*WidgetMap[widgetClassId].init)(dvName, widgetClass, parentWidget,
		dvResources,
		&(widgetListEntry->data),
		widgetListEntry->publicResources,
		&(widgetListEntry->outerWidget));

	if (newWidget == NULL) {
		parse_error("Unable to create new widget.");
		free_var(dvResources);
		return newInt(-1);
	}

	/* FIX: this is a hack for nested widgets.  Should be moved to init. */
	if (widgetListEntry->outerWidget == NULL) {
		dbgprintf ("no outer widget\n");
		/* If the widget didn't set its outerWidget, set it for it. */
		widgetListEntry->outerWidget = newWidget;
	}
	else {
		dbgprintf ("outer widget = %ld\n", widgetListEntry->outerWidget);
		dbgprintf ("outer outer widget = %ld\n", XtParent(widgetListEntry->outerWidget));
		dbgprintf ("outer outer outer widget = %ld\n", XtParent(XtParent(widgetListEntry->outerWidget)));
	}

	widgetListEntry->widget = newWidget;
	widgetListEntry->prev = WidgetListTail;
	widgetListEntry->next = NULL;

	if (WidgetListTail != NULL) {
		WidgetListTail->next = widgetListEntry;
		WidgetListTail = widgetListEntry;
	}
	else {    /* First widget. */
		WidgetListHead = WidgetListTail = widgetListEntry;
	}

	/* Set resources again now that there's an instance of this widget, so that
	 * the resources which weren't set during the first call get handled.  This
	 * is still fairly efficient, since the already-set resources were deleted
	 * from the struct the first time around.
	 */

	if (dvResources != NULL) {

		/* FIX: need to somehow handle pseudo-resources that are also real
		 * resources, since they will have been deleted from dvResources by
		 * now..
		 */

		freeStack.head = freeStack.tail = NULL;

		/* this will actually set the values */
		gui_setResourceValues(newWidget, widgetClass, dvResources,
			xtArgs, &freeStack,
			widgetListEntry->publicResources);

		/* Free anything in the free stack. */
		gui_freeStackFree(&freeStack);

		/* Set pseudo-resources, if any. */
		if (WidgetMap[widgetClassId].setPseudoResources != NULL) {
			(*WidgetMap[widgetClassId].setPseudoResources)
				(newWidget, dvResources, widgetListEntry->publicResources);
		}

		/* Free the resources we claimed/cloned above. */
		free_var(dvResources);

	}

	/* Add the destroy callback to all widgets. */
	XtAddCallback(newWidget, XtNdestroyCallback, gui_defaultDestroyCallback, NULL);

#if 0
	/* FIX: remove */
	/* Register callbacks (if any). */
	callbacks = NULL;
	if (WidgetMap[widgetClassId].getCallbacks != NULL) {
		callbacks = (*WidgetMap[widgetClassId].getCallbacks)();
	}    
	registerCallbacks(newWidget, callbacks);
#endif

	if (XtIsRealized(parentWidget) == True) {
		XtRealizeWidget(newWidget);
	}

	return newInt(widgetListEntry->id);

}

/* Var *
* dv_DestroyWidget()
*
*/

Var *
dv_DestroyWidget(vfuncptr f, Var *args)
{

	Alist		alist[2];
	int		widgetId;
	Widget	widget;

	dbgprintf ("dv_DestroyWidget()\n");

	alist[0] = make_alist("widgetid", INT, NULL, &widgetId);
	alist[1].name = NULL;

	if (parse_args(f, args, alist) == 0) {
		parse_error("Usage: a = gui.destroy(widget).");
		return newInt(0);
	}

	if (widgetId == DV_GUI_APPSHELL_WIDGETID) {
		/* Don't want the user nuking the app shell. */
		parse_error("Invalid widget id.");
		return newInt(0);
	}

	widget = getWidgetFromId(widgetId);

	if (widget == NULL) {
		parse_error("Invalid widget id.");
		return newInt(0);
	}

	gui_destroyWidget(widget);

	return newInt(1);

}

void
gui_destroyWidget(Widget widget)
{

	MyWidgetList	widgetListEntry;
	int		widgetClassId;
	void		*instanceData;
	Narray	*publicResources;
	int		publicResourcesCount;
	char		*publicResourcesKey;
	void		*publicResourcesValue;
	Var		*dvData;

	dbgprintf ("gui_destroyWidget(widget = %ld)\n", widget);

	widgetListEntry = gui_getWidgetListEntryFromWidget(widget);

	if (widgetListEntry == NULL) {
		/* This was probably triggered by the destroy callback after an
		 * explicit gui.destroy(); just ignore it.
		 */
		return;
	}

	dbgprintf ("widgetListEntry = %ld\n", widgetListEntry);

	/* Remove this node from the linked list. */

	if (widgetListEntry->prev == NULL) {
		dbgprintf ("previous node is NULL\n");
		WidgetListHead = widgetListEntry->next;
	}
	else {
		dbgprintf ("previous node is %ld\n", widgetListEntry->prev);
		widgetListEntry->prev->next = widgetListEntry->next;
	}
	if (widgetListEntry->next == NULL) {
		dbgprintf ("next node is NULL\n");
		WidgetListTail = widgetListEntry->prev;
	}
	else {
		dbgprintf ("next node is %ld\n", widgetListEntry->next);
		widgetListEntry->next->prev = widgetListEntry->prev;
	}

	/* Destroy the widget and free any instance data. */

	dbgprintf ("calling destroy function\n");
	widget = widgetListEntry->widget;
	widgetClassId = widgetListEntry->widgetClassId;
	instanceData = widgetListEntry->data;
	(*WidgetMap[widgetClassId].destroy)(widget, instanceData);

	/* Free the public resources list. */

	dbgprintf ("freeing public resources\n");
	publicResources = widgetListEntry->publicResources;
	publicResourcesCount = Narray_count(publicResources);
	while (publicResourcesCount) {
		if (Narray_get(publicResources, --publicResourcesCount,
					&publicResourcesKey, &publicResourcesValue) == 1) {
			/* Nothing to free; all values are NULL. */
			(void) Narray_delete(publicResources, publicResourcesKey);
		}
	}

	/* Free the Davinci private pseudo-resource. */

	dbgprintf ("freeing davinci private pseudo-resource\n");
	dvData = widgetListEntry->dvData;
	if (dvData != NULL) {
		free_var(dvData);
	}

	/* Lastly, free the widget list node. */
	dbgprintf ("freeing widgetlist node\n");
	free(widgetListEntry);
}

#if 0

/* FIX: is this needed?  if so, implement.. */

static Var *
dv_WidgetShow(vfuncptr f, Var * args) {
	return NULL;
}

/* FIX: is this needed?  if so, implement.. */

static Var *
dv_WidgetHide(vfuncptr f, Var * args) {
	return NULL;
}

#endif

/* Var *
* dv_Realize(f, args)
*
* f		FIX: what is this
* args		FIX: what is this
*
* Implements Davinci gui.realize().
* Causes all widgets to be realized (displayed).
*
*/

static Var *
dv_Realize(vfuncptr f, Var * args)
{

	Alist			alist[2];
	int			dvWidgetId;
	Widget		widget, parentWidget;

	/* Parse function args. */

	dvWidgetId = DV_INVALID_WIDGET_ID;
	alist[0] = make_alist("widgetid", INT, NULL, &dvWidgetId);
	alist[1].name = NULL;

	if (parse_args(f, args, alist) == 0) {
		parse_error("Usage: a = gui.realize(widgetid)");
		return newInt(0);
	}

	if (dvWidgetId == DV_INVALID_WIDGET_ID) {
		parse_error("Usage: a = gui.realize(widgetid)");
		return newInt(0);
	}

	if (dvWidgetId == DV_GUI_APPSHELL_WIDGETID) {
		parse_error("Invalid widget id.");
		return newInt(0);
	}

	widget = getWidgetFromId(dvWidgetId);

	if (widget == NULL) {
		parse_error("Invalid widget id.");
		return newInt(0);
	}

	/* Make sure the widget is inside an already-realized widget, or is a
	 * shell widget.  This seems to be necessary to prevent realizing non-
	 * shell widgets without a shell parent, which causes errors.  I'm not
	 * sure if this is the right solution; there might be a different problem.
	 */

	parentWidget = XtParent(widget);

	/* FIX: this should be implemented more generically.. */
	/* FIX: xmMessageBoxWidgetClass probably isn't the only thing that should be checked.. */

	if (XtIsShell(widget) || XtClass(widget) == xmMessageBoxWidgetClass) {
		XtPopup(widget, XtGrabNone);
	}
	else {
		if (!XtIsRealized(parentWidget)) {
			parse_error("Error: unable to realize non-shell widget with unrealized parent.");
			return newInt(0);
		}
		else {
			XtRealizeWidget(widget);
		}
	}

	return newInt(1);

}

/* Var *
* dv_XEventWait(f, args)
*
* f		FIX: what is this
* args		FIX: what is this
*
* Checks the event queue to see if any of the events the user wants to know
* about have occurred yet, and returns a Davinci Var describing the events.
*
*/

static Var *
dv_XEventWait(vfuncptr f, Var *args)
{
	XEvent			event;
	Var				*returnVal;
	EventQueuePtr	dvEvent = NULL;
	EventWatchList	watchList;

	Alist			alist[2];
	Var				*waitList = NULL;
	int				i;
	EventWatchList	watchListEntry, watchListPrev;

	dbgprintf ("dv_XEventWait()\n");

	/* Parse the args from Davinci. */
	alist[0] = make_alist("widgetlist", ID_UNK, NULL, &waitList);
	alist[1].name = NULL;
	if (parse_args(f, args, alist) == 0) {
		/* No widgetlist; wait for events on any widget. */
		/* FIX: should a NULL list be the same as no list at all? */
		dbgprintf ("watchList = NULL\n");
		watchList = NULL;
	}
	else if (waitList == NULL) {
		watchList = NULL;
	}
	else {
		if (waitList == NULL) {
			parse_error("DEBUG: waitList = NULL\n"); /* FIX */
			return NULL;
		}
		/* Sanity checks. */
		if (V_FORMAT(waitList) != INT) {
			dbgprintf ("V_FORMAT(waitList) = %d\n", V_FORMAT(waitList));
			parse_error("widgetlist must be a vector of integers.");
			return NULL;
		}
		if ((GetY(waitList) > 1) || (GetZ(waitList) > 1)) {
			parse_error("widgetlist must have one line and one band only.");
			return NULL;
		}
		/* Parse the widgetlist. */
		for (i = 0; i < GetX(waitList); i++) {
			/* Create a new EventWatchEntry for each widget. */
			watchListEntry = (EventWatchList) malloc(sizeof(EventWatchEntry));
			if (watchListEntry == NULL) {
				parse_error("unable to allocate memory.");
				return NULL;
			}
			if (i == 0) {
				/* First entry in list, save pointer as watch list head. */
				watchList = watchListEntry;
			}
			if (watchListPrev != NULL) {
				/* Linked list.. */
				watchListPrev->next = watchListEntry;
			}
			/* Calculate address of entry in waitList and store value. */
			/* FIX: there's gotta be a cleaner-looking way to do this.. */
			watchListEntry->dvWidgetId = *(((int *) V_DATA(waitList)) + i);
			dbgprintf ("dvWidgetid = %d\n", watchListEntry->dvWidgetId);
			watchListEntry->next = NULL;
			watchListPrev = watchListEntry;
		}
	}

	/* Wait until an event occurs.  If the watchList is NULL, wait for any
	 * event at all.
	 */

	while (dvEvent == NULL) {
		if (EventQueueHead == NULL) {
			/* Nothing going on, so we simulate the mainloop behavior. */
			XtAppNextEvent(applicationContext, &event);
			XtDispatchEvent(&event);
		}
		/* FIX: desc */
		dvEvent = inspectEvents(watchList);
	}

	returnVal = interpretEvent(dvEvent);

	/* Free the event now that it has been processed. */
	free(dvEvent);

	/* Clear out the entire watchlist. */
	/* FIX: do we really want to clear the list here? */
	clearEventWatchList(watchList);

	return returnVal;

}

/*****************************************************************************/

/* int
* gui_isDefault(aliases, name)
*
* aliases	Array of strings.
* name		String to check against array.
*
* This is a helper function used by the various isFoo() widget functions,
* to compare Davinci names to a list of widget aliases.  Nothing in this file
* calls it, but I wasn't sure where else to put it.
* FIX: maybe in gui.h?
*
* Returns: 1 if name is in aliases, 0 otherwise.
*
*/

/* FIX: rename this to checkAliases() or something since it's used in the
* resource map lookup as well.
*/

int
gui_isDefault(const char **aliases, const char *name)
{

	int	i;

	i = 0;
	while (aliases[i] != NULL) {
		if (!strcasecmp(aliases[i], name)) {
			return 1;
		}
		i++;
	}

	return 0;

}

/* Widget
* gui_initDefault(dvName, class, parent)
*
* Default Widget initializer, for basic Xt widgets that don't require any
* special initialization beyond calling XtVaCreateManagedWidget.
*
* dvName	User-supplied label.
* class	Xt WidgetClass for the new widget.
* parent      	Parent Widget object.
*
* Returns the newly-created Widget object, or NULL if the create failed.
*
*/

Widget
gui_initDefault(const char *dvName, WidgetClass widgetClass, Widget parent,
	Var *dvResources, void **optData, Narray *publicResources,
	Widget *outerWidget)
{
	Widget				newWidget;
	Arg					xtArgs[DV_MAX_XT_ARGS];
	Cardinal			xtArgCount;
	FreeStackListEntry	freeStack;

	dbgprintf ("gui_initDefault(dvName = \"%s\", class = %ld, "
		"parent = %ld, optData = %ld, outerWidget = %ld)\n",
		dvName, widgetClass, parent, optData, outerWidget);

	/* Parse resources, if the user supplied any. */
	freeStack.head = freeStack.tail = NULL;
	if (dvResources != NULL) {
		xtArgCount = gui_setResourceValues(NULL, widgetClass, dvResources,
			xtArgs, &freeStack,
			publicResources);
	} else {
		xtArgCount = 0;
	}

	newWidget = XtCreateManagedWidget(dvName, widgetClass, parent, xtArgs, xtArgCount);
	/* FIX: maybe move XtRealizeWidget() to dv_CreateWidget() for all widgets */

	/* Free anything in the free stack. */
	gui_freeStackFree(&freeStack);

	return newWidget;
}

/* void
* gui_destroyDefault(widget, instanceData)
*
* Destroys widget, freeing up any memory used by it.  instanceData should
* be NULL for anything using this default function.  If a non-default init
* function created the widget and stored anything in instanceData, then a
* non-default destroy function should be called to clean it up.
*
*/

void
gui_destroyDefault(Widget widget, void *instanceData)
{

	XtDestroyWidget(widget);
	return;

}

/*****************************************************************************
*
* FIX: FUNCTION DEFINITIONS (move around or re-split into resources.c)
*
*****************************************************************************/

/* helper for gui_setResourceValues() */
static int
compString (const void *a, const void *b)
{
	return strcmp (*(char **)a, *(char **)b);
}

/* gui_setResourceValues()
*
* If widget == NULL, the function will fill the xtArgs array (should be pre-
* allocated) with resource names/values, and sets numXtArgs to the actual
* number of values.  These values are essentially returned for use by the
* caller.  This method can only set resources specific to the specified class.
*
* If widget != NULL, the function will create xtArgs arrays as it parses the
* user's set structure.  When parsing has finished, this function actually
* sets the resources.  The FreeStack still needs to be freed.
*/

Cardinal
gui_setResourceValues(Widget widget, WidgetClass widgetClass,
	Var *usrStruct, Arg *xtArgs,
	FreeStackList freeStack, Narray *publicResources)
{
	XtResourceList	xtResources;
	Cardinal		numXtResources;
	Cardinal		numXtArgs;
	MyWidgetList	widgetListEntry;
	Widget			currentWidget, lastWidget;
	char			**usrResourceNames;
	int				numUsrResources;
	unsigned int	usrIdx;
	Var				*setValue;
	int				setSuccess;
	char			*usrResourceName;

	dbgprintf ("gui_setResourceValues(widget = %ld, "
		"widgetClass = %ld, usrStruct = %ld, xtArgs = %ld, numXtArgs = %ld, "
		"freeStack = %ld, publicResources = %ld)\n", widget, widgetClass,
		usrStruct, xtArgs, numXtArgs, freeStack, publicResources);

	if (usrStruct == NULL) {
		/* Nothing to do. */
		return 0;
	}

	/* Iterate over user resources, parsing the Davinci value into an Xt value.
	 * Store the value itself (cast into an XtArgVal), or a pointer to the
	 * value, in an Xt Arg array.  Store the address(es) of any memory that
	 * needs to be freed post-set in the FreeStackList.
	 */

	/* Get the names and count of all the user resources. */
	numUsrResources = get_struct_names(usrStruct, &usrResourceNames, NULL);
	numUsrResources--; /* Ignore trailing NULL. */
	dbgprintf ("numUsrResources = %d\n", numUsrResources);

	/* get the widget list entry if a widget was provided */
	if (widget != NULL) {
		widgetListEntry = gui_getWidgetListEntryFromWidget(widget);
	}

	numXtArgs = 0;
	lastWidget = NULL;

	/* for each user-specified resource */
	for (usrIdx = 0; usrIdx < numUsrResources; usrIdx++) {
		/* get name and value to set */
		find_struct(usrStruct, usrResourceNames[usrIdx], &setValue);
		setSuccess = FALSE;
		usrResourceName = usrResourceNames[usrIdx];

		/* if this is the private pseudo-resource */
		if (!strcmp(DV_GUI_PRIVATE_RESOURCE_NAME, usrResourceNames[usrIdx])) {
			gui_setPrivate(widget, setValue);
			/* Store this resource name for gui.get() later. */
			Narray_add(publicResources, usrResourceNames[usrIdx], NULL);
			/* Delete from user struct. */
			free_var(Narray_delete(V_STRUCT(usrStruct), DV_GUI_PRIVATE_RESOURCE_NAME));
			continue;
		}

		/* if we have a widget, we might have constraints */
		if (widget != NULL) {
			currentWidget = widgetListEntry->outerWidget;

			/* get constraints list from outer widget's class */
			XtGetConstraintResourceList (XtClass(XtParent(currentWidget)),
				&xtResources, &numXtResources);

			/* try setting the resource */
			setSuccess = gui_doSetResourceValues(currentWidget,
				usrResourceName, setValue,
				xtArgs, &numXtArgs,
				freeStack,
				&xtResources, numXtResources);

			XtFree((char *) xtResources);
		}

		/* if this is NOT a constraint resource */
		if (setSuccess == FALSE) {
			currentWidget = widget;

			/* Get the names and count of all the real widget resources. */
			XtGetResourceList(widgetClass, &xtResources, &numXtResources);

			/* populate the resource list */
			setSuccess = gui_doSetResourceValues(currentWidget,
				usrResourceName, setValue,
				xtArgs, &numXtArgs,
				freeStack,
				&xtResources, numXtResources);

			XtFree((char *) xtResources);
		}

		/*
		 * We buffer the resources to set in the xtArgs array until a different
		 * widget needs to be set.  This decreases the number of XtSetArgs calls
		 * that must be made (although we'd do better to queue up all resources
		 * by widget and then flush each at the end, with only two widgets it's
		 * not that big of a deal.)
		 * NOTE!!!
		 * if widget==null this function will never set the resources
		 *   the caller must use the xtArgs / numXtArgs variables to set them.
		 * If widget!=null this function will set all the resources
		 *   the caller does NOT need to set them in that case
		 */

		if (setSuccess) {
			if (currentWidget != lastWidget) {
				if (lastWidget != NULL) {
					XtSetValues (lastWidget, xtArgs, numXtArgs);
					numXtArgs = 0;
				}
				lastWidget = currentWidget;
			}

			/* Store this resource name for gui.get() later. */
			Narray_add(publicResources, usrResourceNames[usrIdx], NULL);
			/* Delete from user struct. */
			free_var(Narray_delete(V_STRUCT(usrStruct), usrResourceNames[usrIdx]));
		}
	}

	/* If widget!=null then we're supposed to set resources in this function, so
	   in that case, if we have any leftovers set them now */
	if (numXtArgs > 0 && currentWidget != NULL) {
		XtSetValues (currentWidget, xtArgs, numXtArgs);
		numXtArgs = 0;
	}

	/* Free the resource name list. */
	free(usrResourceNames);

	return (numXtArgs);
}

/*
 * Searches for usrResourceName in xtResources array, and if found converts
 * setValue with the appropriate type map and sets it into xtArgs array, which
 * needs to be initialized in the calling function (along with numXtArgs)
 */

static int
gui_doSetResourceValues(
	Widget widget,
	char *usrResourceName, Var *setValue,
	Arg *xtArgs, Cardinal *numXtArgs,
	FreeStackList freeStack,
	XtResourceList *xtResources, Cardinal numXtResources)
{
	Cardinal		xtIdx;
	unsigned int	typeIdx;
	String			xtResourceName;
	String			xtResourceType;
	XtArgVal		xtArgValue;

	dbgprintf ("gui_doSetResourceValues()\n");
	dbgprintf ("numXtResources = %d\n", numXtResources);

	for (xtIdx = 0; xtIdx < numXtResources; xtIdx++) {
		xtResourceName = (*xtResources)[xtIdx].resource_name;
		xtResourceType = (*xtResources)[xtIdx].resource_type;
		if (!strcmp(usrResourceName, xtResourceName)) {
			/* Found a match.  Lookup the type and find the setValue function. */
			typeIdx = 0;
			while (ResourceTypeMap[typeIdx].typeNames != NULL) {
				if (gui_isDefault(ResourceTypeMap[typeIdx].typeNames, xtResourceType)) {
					/* Store the value. */
					xtArgValue = (*ResourceTypeMap[typeIdx].setValue)
						(widget, xtResourceName, xtResourceType, setValue, freeStack);
					XtSetArg(xtArgs[(*numXtArgs)], xtResourceName, xtArgValue);
					(*numXtArgs)++;
					/* return set success */
					return TRUE;
				}
				typeIdx++;
			}

			/* Report unknown resources, but carry on with the rest. */
			if (ResourceTypeMap[typeIdx].typeNames == NULL) {
				/* FIX: print the resource name in the error */
				parse_error("attempt to set unknown widget resource type");
				dbgprintf ("name = '%s', type = '%s'\n", xtResourceName, xtResourceType);
			}

			/* match but no type map, so return set failure */
			return FALSE;
		}
	}

	/* no match, so return set failure */
	return FALSE;
}

/* Var *
 * dv_XGet(f, args)
 *
 * f		FIX: what is this
 * args		FIX: what is this
 *
 * Implementation of Davinci gui.get(widgetid).
 *
 * widgetid	Davinci widget id from widget map.
 *
 * Returns a Davinci struct of key/value pairs representing all the modifiable
 * widget resources.
 *		key = resource name (string).
 *		value = Davinci Var of the appropriate type for the resource.
 *
 */

static Var *
dv_XGet(vfuncptr f, Var * args)
{
	Alist			alist[5];
	int				dvWidgetId, dvAllResources, dvShowEnums;
	Var				*dvResourceList;
	Var				*dvRetList;

	/* Parse function args. */
	dvWidgetId = DV_INVALID_WIDGET_ID;
	dvShowEnums = 0;
	dvAllResources = 0;
	dvResourceList = NULL;
	alist[0] = make_alist("widgetid", INT, NULL, &dvWidgetId);
	alist[1] = make_alist("resourcelist", ID_UNK, NULL, &dvResourceList);
	alist[2] = make_alist("all", INT, NULL, &dvAllResources);
	alist[3] = make_alist("showenums", INT, NULL, &dvShowEnums);
	alist[4].name = NULL;

	if (parse_args(f, args, alist) == 0) {
		parse_error("Usage: a = gui.get(widgetid [,all=0|1] [,resourcelist=LIST])");
		return NULL;
	}

	if (dvWidgetId == DV_INVALID_WIDGET_ID) {
		parse_error("widgetid is required.");
		return NULL;
	}

	widgetEnumData = (dvShowEnums ? create_struct (NULL) : NULL);

	dvRetList = gui_getResources (dvWidgetId, dvResourceList, dvAllResources);

	if (widgetEnumData) {
		printEnumOptions (widgetEnumData);
		widgetEnumData = NULL;
	}

	return dvRetList;
}

static Var *gui_getResources (int dvWidgetId, Var *dvResourceList, int dvAllResources)
{
	Widget			widget, constraintWidget;
	int				widgetClassId;
	Var				*dvStruct = NULL, *privateResource;
	XtResourceList	xtResources;
	Cardinal		numResources;
	MyWidgetList	widgetListEntry;
	Narray			*visibleResources;
	Boolean			needToFreeVisible = False;

	widget = getWidgetFromId(dvWidgetId);

	if (widget == NULL) {
		parse_error("Invalid widget id specified");
		return NULL;
	}

	/* Build visible resource list, or obtain default list. */
	widgetListEntry = gui_getWidgetListEntryFromWidget(widget);
	dvStruct = new_struct(0);
	visibleResources = NULL;
	needToFreeVisible = False;
	if (dvResourceList == NULL) {
		dbgprintf ("using default resource list\n");
		visibleResources = widgetListEntry->publicResources;
	}
	else {
		visibleResources = gui_extractNarray(dvResourceList);
		if (visibleResources == NULL) {
			/* FIX: should this fail silently? */
			parse_error("Warning: resource list is empty.");
		}
		else {
			needToFreeVisible = True;
		}
	}

	if (visibleResources == NULL) {
		return NULL;
	}

	/* Retrieve any pseudo-resources the widget has implemented.  This needs to
	 * happen first so that the widget has the chance to override the default
	 * retrieval mechanism (see widget_list.c for a good example).
	 */
	widgetClassId = widgetListEntry->widgetClassId;
	if (WidgetMap[widgetClassId].getPseudoResources != NULL) {
		(*WidgetMap[widgetClassId].getPseudoResources)(widget, dvStruct);
	}

	/* Get constraint resources for the specified outer widget.  This should be
	 * the widget itself, UNLESS the widget creates itself inside other widgets.
	 * In that case the outermost widget will carry the constraints, and the
	 * corresponding widget_<widget type>.c module must specify that widget as
	 * the outerWidget.  The create function uses the widget itself as the
	 * outerWidget when none is otherwise specified.
	 */

	constraintWidget = widgetListEntry->outerWidget;
	if (XtParent(constraintWidget) != NULL) {
		dbgprintf ("checking for constraints from parent = %ld)\n",
			XtParent(constraintWidget));
		XtGetConstraintResourceList(XtClass(XtParent(constraintWidget)),
			&xtResources, &numResources);
		if (xtResources != NULL) {
			gui_getResourceValues(constraintWidget, dvStruct,
				visibleResources, dvAllResources,
				&xtResources, numResources);
			XtFree((char *) xtResources);
		}
	}

	/* Get normal resources. */
	XtGetResourceList(XtClass(widget), &xtResources, &numResources);
	gui_getResourceValues(widget, dvStruct,
		visibleResources, dvAllResources,
		&xtResources, numResources);
	XtFree((char *) xtResources);

	/* Add the private pseudo-resource, if set. */
	privateResource = gui_getPrivate(widget);
	if (privateResource != NULL) {
		add_struct(dvStruct, DV_GUI_PRIVATE_RESOURCE_NAME, privateResource);
	}

	/* Free visibleResources, if it was dynamically built. */
	if (needToFreeVisible == True) {
		Narray_free(visibleResources, NULL);
	}

	return dvStruct;
}

void
gui_getResourceValues(
	Widget widget,
	Var *dvStruct,
	Narray *visibleResources,
	int dvAllResources,
	XtResourceList *xtResources,
	Cardinal numResources)
{
	String	resourceName, resourceType;
	int		i, j;
	char    scratch[512];
	Var		*getValue;

	resourceName = scratch;

	dbgprintf ("gui_getResourceValues(widget = %ld, dvStruct = %ld, "
		"xtResources = %ld, numResources = %d)\n",
		widget, dvStruct, xtResources, numResources);

	/* Get the values for each resource type we support. */
	for (i = 0; i < numResources; i++) {
		getValue = NULL;

		resourceName = (*xtResources)[i].resource_name;
		resourceType = (*xtResources)[i].resource_type;

		/* See if this resource has already been set (by the pseudo-resource
		 * handler, most likely).  If not, see if we're in get-all-resources mode
		 * or it's in the list of visible resources. */

		if (find_struct(dvStruct, resourceName, NULL) != -1
		|| (!dvAllResources && Narray_find(visibleResources, resourceName, NULL) == -1))
		{
			/* Skip it. */
			continue;
		}

		/* Look up the resource type in the ResourceTypeMap and call the proper
		 * getValue function.
		 */

		j = 0;
		while (ResourceTypeMap[j].typeNames != NULL) {
			if (gui_isDefault(ResourceTypeMap[j].typeNames, resourceType)) {
				getValue = (*ResourceTypeMap[j].getValue)(widget,
					resourceName, resourceType);
				break;
			}
			j++;
		}

		/* if a type map was not available then use a 'not supported' message */
		if (ResourceTypeMap[j].typeNames == NULL) {
			char val[1024] = "";
			sprintf (val, "<unsupported resource type %s>", resourceType);
			getValue = newString(strdup(val));
		}

		/* if all is well, add the name/value pair to the structure */
		if (getValue != NULL) {
			add_struct(dvStruct, resourceName, getValue);

			if (widgetEnumData) {
				Var *data, *names;
				/* iif this resource is enumerated will there be an entry */
				if (-1 != find_struct (widgetEnumData, resourceType, &data)) {
					find_struct (data, "names", &names);
					add_struct (names, resourceName, newInt(0));
				}
			}
		}
	}

	/* Add xhandle if all resources requested, or xhandle was requested */
	if (dvAllResources || Narray_find(visibleResources, "xhandle", NULL) != -1)
		add_struct (dvStruct, "xhandle", newInt (widget->core.window));
}

/*
 * printEnumOptions
 *
 * widgetEnumData: struct Var containing the enumerations to display.
 * No return value, no side effects other than user output
 */

static void
printEnumOptions (Var *widgetEnumData)
{
	Var *enumVal, *names, *values;
	char *string;
	int i, j;
	for (i=0; i<get_struct_count(widgetEnumData); i++) {
		get_struct_element (widgetEnumData, i, &string, &enumVal);
		parse_error ("Resource Type '%s'", string);
		find_struct (enumVal, "values", &values);
		parse_error ("\tValues:");
		for (j=0; j<get_struct_count(values); j++) {
			get_struct_element (values, j, &string, NULL);
			parse_error ("\t\t%s", string);
		}
		find_struct (enumVal, "names", &names);
		parse_error ("\tUsed by:");
		for (j=0; j<get_struct_count(names); j++) {
			get_struct_element (names, j, &string, NULL);
			parse_error ("\t\t%s", string);
		}
	}
}

/* Var *
* dv_XSet(f, args)
*
* f		FIX: what is this
* args		FIX: what is this
*
* Implementation of Davinci gui.set(widgetid, astruct).
*
* widgetid	Davinci widget id from widget map.
* astruct	Davinci struct containing key-value pairs to set.
*      key = widget resource name
*      value = new value
*
* Looks up all the entries in the struct and compares them to the list of
* resources for the widget, setting each one
* Returns a struct of the values as set (normally the same struct passed in).
*
* FIX: allow setting of a specific single resource:
*	gui.set(widgetid, resourceName, resourceValue)
*	still needs to lookup the resource and make sure it's valid
*	this barely buys any performance increase so it's only worth it
*	as an API analogue to gui.get(widgetid, resourceName)
*
*/

static Var *
dv_XSet(vfuncptr f, Var * args)
{
	Alist				alist[3];
	int					dvWidgetId;
	Widget				widget;
	WidgetClass			widgetClass;
	Var					*usrStruct;
	Arg					xtArgs[DV_MAX_XT_ARGS];
	FreeStackListEntry	freeStack;
	int					widgetClassId;
	MyWidgetList		widgetListEntry;

	/* Parse function args. */
	dvWidgetId = DV_INVALID_WIDGET_ID;
	alist[0] = make_alist("widgetid", INT, NULL, &dvWidgetId);
	alist[1] = make_alist("resources", ID_STRUCT, NULL, &usrStruct);
	alist[2].name = NULL;

	if (parse_args(f, args, alist) == 0) {
		parse_error("Usage: a = gui.set(widgetid, resources)");
		return NULL;
	}

	if (dvWidgetId == DV_INVALID_WIDGET_ID) {
		parse_error("Usage: a = gui.set(widgetid, resources)");
		return NULL;
	}

	if (usrStruct == NULL) {
		parse_error("Usage: a = gui.set(widgetid, resources)");
		return NULL;
	}

	widgetClass = getWidgetClassFromId(dvWidgetId);

	if (widgetClass == NULL) {
		parse_error("Invalid widget id.");
		return NULL;
	}

	widget = getWidgetFromId(dvWidgetId);

	if (widget == NULL) {
		parse_error("Invalid widget id.");
		return NULL;
	}

	/* Let the widget have a crack at it first, to handle any pseudo-
	 * resources.  The widget will delete anything from the struct
	 * that it's taken care of setting.
	 */

	usrStruct = V_DUP (usrStruct);

	widgetListEntry = gui_getWidgetListEntryFromWidget(widget);
	widgetClassId = widgetListEntry->widgetClassId;

	if (WidgetMap[widgetClassId].setPseudoResources != NULL) {
		(*WidgetMap[widgetClassId].setPseudoResources)
			(widget, usrStruct, widgetListEntry->publicResources);
	}

	freeStack.head = freeStack.tail = NULL;

	/* this will actually set the values */
	gui_setResourceValues(widget, widgetClass, usrStruct,
		xtArgs, &freeStack,
		widgetListEntry->publicResources);

	/* Free anything in the free stack. */
	gui_freeStackFree(&freeStack);

	/* Used to return the values that were actually set, now just returning 1. */
	return newInt(1);
}

/*****************************************************************************
*
* These functions are all used to get or set Xt widget resources.
*
* Var *
* gui_getFoo(widget, resource)
*
* widget	Xt Widget instance.
* resource	Xt widget resource name.
*
* Retrieves the current resource value from the widget and packs it into
* a Davinci Var of the appropriate type (depends on the resource data type).
*
* void
* gui_setFoo(widget, resource, value)
*
* widget	Xt Widget instance.
* resource	Xt widget resource name.
* value	Davinci Var containing the new value.
*
* Sets the widget resource to the value in the Davinci variable.  The
* Davinci variable must be of a compatible data type.
*
*****************************************************************************/

/* Basic X String. */

Var *
gui_getString(Widget widget, String resourceName, String resourceType)
{

	Var		*o;
	String	strValue;	/* FIX: need to allocate? */

	dbgprintf ("gui_getString(widget = %ld, "
			"resourceName = '%s', resourceType = '%s')\n",
			widget, resourceName, resourceType);

	XtVaGetValues(widget, resourceName, &strValue, NULL);

	if (strValue == NULL) {
		o = NULL;
	} else {
		o = newString(dupString(strValue));
		/* Don't free strValue; Motif doesn't make a copy. */
	}

	return o;

}

/* XtArgVal
* gui_setString()
*
* NOTE: returns the old value on error.  This is inefficient
*       but the only other option (with this model) is to plow over the
*       existing value with a NULL.  Either way, it's the user's fault
*       for supplying the wrong type, and it generates an error, so it
*       doesn't really have to be that efficient.  It's still got to
*	 waste time storing the unneeded old value for successful sets,
*	 though.  Bleah.  I hate this resource model.
*
*/

XtArgVal
gui_setString(
	const Widget widget,
	const String resourceName,
	const String resourceType,
	const Var *value,
	FreeStackList freeStack)
{

	String	oldValue;
	String	newValue;

	dbgprintf ("gui_setString(widget = %ld, resourceName = '%s', "
			"resourceType = '%s', value = %ld, freeStack = %ld)\n",
			widget, resourceName, resourceType, value, freeStack);

	if (widget != NULL) {
		/* Don't free strValue; Motif doesn't make a copy. */
		XtVaGetValues(widget, resourceName, &oldValue, NULL);
	}
	else {
		oldValue = NULL;
	}

	if (V_TYPE(value) != ID_STRING) {
		parse_error("attempt to set string resource to non-string");
		newValue = oldValue;
	}
	else {
		newValue = dupString(V_STRING(value));
		gui_freeStackPush(freeStack, newValue);
	}

	dbgprintf ("newValue = '%s'\n", newValue);
	dbgprintf ("XtArgVal) newValue = '%s'\n", (XtArgVal) newValue);

	return (XtArgVal) newValue;
}

/* Handle the pseudo-resource named in DV_GUI_PRIVATE_RESOURCE_NAME. */
Var *
gui_getPrivate(Widget widget)
{
	MyWidgetList	listEntry;
	Var		*returnVal;

	dbgprintf ("gui_getPrivate(widget = %ld)\n", widget);

	listEntry = gui_getWidgetListEntryFromWidget(widget);
	returnVal = listEntry->dvData;

	if (returnVal != NULL) {
		/* Create a new Var to return */
		returnVal = V_DUP(returnVal);
	}

	return returnVal;
}

void
gui_setPrivate(Widget widget, Var *value)
{

	MyWidgetList	listEntry;
	Var		*dvData, *newData;

	dbgprintf ("gui_setPrivate(widget = %ld, value = %ld)\n", widget, value);

	/* Get the current Var */
	listEntry = gui_getWidgetListEntryFromWidget(widget);
	dvData = listEntry->dvData;

	/* Free the current Var. */
	if (dvData != NULL) {
		free_var(dvData);
	}

	/* Dup the new var and store it. */
	newData = V_DUP(value);
	mem_claim(newData);
	listEntry->dvData = newData;

	return;

}

/* Motif compound string (XmString). */
Var *
gui_getXmString(Widget widget, String resourceName, String resourceType)
{
	Var		*o;
	XmString	cstrValue;
	String	strValue;

	dbgprintf ("gui_getXmString(widget = %ld, "
		"resourceName = '%s', resourceType = '%s')\n",
		widget, resourceName, resourceType);

	XtVaGetValues(widget, resourceName, &cstrValue, NULL);

	if (XmStringGetLtoR(cstrValue, XmFONTLIST_DEFAULT_TAG, &strValue)) {
		o = newString(dupString(strValue));
		XtFree(strValue);
	} else {
		/* This occurs iif a string resource exists but is set to NULL
		   we actually WANT to get back "", so replaced: o = NULL; with: */
		o = newString(dupString(""));
	}

	return o;
}

/* XtArgVal
* gui_setXmString()
*
* NOTE: this returns 0 on error, which will be translated to NULL when
*       the value is set.  Any attempt to set string resources to non-
*       strings will thus set them to NULL, and print an error message.
*
*/

XtArgVal
gui_setXmString(const Widget widget,
	const String resourceName,
	const String resourceType,
	const Var *value,
	FreeStackList freeStack)
{
	XmString	oldValue;
	XmString	strValue;

	dbgprintf ("gui_setXmString(widget = %ld, "
			"resourceName = '%s', resourceType = '%s', value = %ld)\n",
			widget, resourceName, resourceType, value);

	if (widget != NULL) {
		XtVaGetValues(widget, resourceName, &oldValue, NULL);
		gui_freeStackPush(freeStack, oldValue);
	}
	else {
		oldValue = NULL;
	}

	if (V_TYPE(value) != ID_STRING) {
		parse_error("attempt to set string resource to non-string");
		return (XtArgVal) oldValue;
	}

	strValue = XmStringCreateLocalized(V_STRING(value));
	/* 
	 ** offhand, I think this is wrong.  The string will get
	 ** returned and should NOT be freed.
	 */
	/*
	 * gui_freeStackPush(freeStack, strValue);
	 */

	return (XtArgVal) strValue;
}

/* Table (array) of Motif compound strings (XmString). */

/* Var *
* gui_getXmStringTableFree()
*
* I don't think this works, because of Motif stupidity about needing to
* know the string count to read an XmStringTable.
*
*/

Var *
gui_getXmStringTableFree(const Widget widget,
	const String resource,
	const String resourceType)
{
	return gui_getXmStringTableCount(widget, resource, 1, -1);
}

/* Var *
* gui_getXmStringTable()
*
* I don't think this works, because of Motif stupidity about needing to
* know the string count to read an XmStringTable.
*
*/

Var *
gui_getXmStringTable(const Widget widget,
	const String resource,
	const String resourceType)
{
	return gui_getXmStringTableCount(widget, resource, 0, -1);
}

Var *
gui_getXmStringTableCount(const Widget widget,
	const String resource,
	const int free, const int count)
{
	Var				*text; 
	int				stringCount, stringIdx;
	char			**newStrings;
	XmStringTable	stringTable;
	String			strValue;
	char			*tmpString;

	dbgprintf ("gui_getXmStringTableCount(widget = %ld, "
		"resource = \"%s\", free = %d, count = %d)\n",
		widget, resource, free, count);

	/* Unnecessary hack, I think
	   if (count) {
	   stringTable = XtMalloc((count+1) * sizeof(XmString));
	   } 
	 */
	XtVaGetValues(widget, resource, &stringTable, NULL);

	dbgprintf ("stringTable = %ld\n", stringTable);

	if (stringTable == NULL) {
		/* No items. */
		/* NOTE: this doesn't seem to ever happen.  Instead, you need to know
		 * exactly how many strings you're expecting, ahead of time.
		 */
		text = newText(0, NULL);
	}
	else {
		/* Try to count strings, unless count supplied. */
		if (count == -1) {
			stringCount = 0;
			while (stringTable[stringCount] != NULL) {
				stringCount++;
			}
		}
		else {
			stringCount = count;
		}
		/* Allocate text buffer. */
		newStrings = (char **) malloc(sizeof(char *) * stringCount);
		if (newStrings == NULL) {
			parse_error("Error: unable to allocate memory for text.");
			text = newText(0, NULL);
		}
		else {
			/* Build the list of strings. */
			for (stringIdx = 0; stringIdx < stringCount; stringIdx++) {
				dbgprintf ("string #%d\n", stringIdx);
				/* Parse the XmString, clone it, and add it to the list. */
				if (XmStringGetLtoR(stringTable[stringIdx], XmFONTLIST_DEFAULT_TAG,
							&strValue)) {
					dbgprintf ("strValue = \"%s\"\n", strValue);
					tmpString = dupString(strValue);
					newStrings[stringIdx] = tmpString;
				}
				else {
					parse_error("Warning: unparsable string in XmStringTable "
							"being ignored.");
				}
			}
			text = newText(stringCount, newStrings);
		}
	}

	/* Free the unneeded string table. */
	if (free) {
		dbgprintf ("freeing stringTable\n");
		XtFree((XtPointer) stringTable);
	}

	return text;
}

/* XtArgVal
* gui_setXmStringTableFromDarray()
*
* FIX: describe.
*
*/

XtArgVal
gui_setXmStringTableFromDarray(
	const Widget widget,
	const String resourceName,
	const String resourceType,
	const Darray *value,
	FreeStackList freeStack)
{
	XmStringTable	oldValue;
	XmStringTable	stringTable;
	int		numStrings, stringIdx;
	char		*stringValue;
	XmString	xmStringValue;

	dbgprintf ("gui_setXmStringTableFromDarray(widget = %ld, "
			"resourceName = '%s', resourceType = '%s', value = %ld, "
			"freeStack = %ld)\n",
			widget, resourceName, resourceType, value, freeStack);

	if (widget != NULL) {
		XtVaGetValues(widget, resourceName, &oldValue, NULL);
#if 0
		/* FIX: this almost definitely needs to be freed. */
		if (oldValue != NULL) {
			gui_freeStackPush(freeStack, oldValue);
		}
#endif
	}
	else {
		oldValue = NULL;
	}

	numStrings = Darray_count(value);
	dbgprintf ("numStrings = %d\n", numStrings);

	if (numStrings == -1) {
		stringTable = oldValue;
	}
	else {
		/* Create a table of XmStrings. */
		stringTable = (XmStringTable) XtMalloc(sizeof(XmString) * numStrings);
		if (stringTable == NULL) {
			parse_error("Error: unable to allocate memory for string list.");
		}
		else {
			// gui_freeStackPush(freeStack, stringTable);
			for (stringIdx = 0; stringIdx < numStrings; stringIdx++) {
				if (Darray_get(value, stringIdx, (void **) &stringValue) == -1) {
					parse_error("Internal error: unable to read Darray.");
					stringTable = oldValue;
					break;
				}
				else {
					xmStringValue = XmStringCreateLocalized(stringValue);
					/* FIX: can't find documentation on failure return, so this is
					 * an assumption; confirm
					 */
					if (xmStringValue == NULL) {
						parse_error("Internal error: unable to create XmString.");
						stringTable = oldValue;
						break;
					}
					// gui_freeStackPush(freeStack, xmStringValue);
					stringTable[stringIdx] = xmStringValue;
				}
			}
		}
	}

	return (XtArgVal) stringTable;
}

/* XtArgVal
* gui_setXmStringTable()
*
* Builds a Narray of strings from a Var and calls
* gui_setXmStringTableFromNarray().
*
*/

XtArgVal
gui_setXmStringTable(const Widget widget,
	const String resourceName,
	const String resourceType,
	const Var *value,
	FreeStackList freeStack)
{
	Darray	*stringList;
	XtArgVal	stringTable;

	stringList = gui_extractDarray(value);
	stringTable = gui_setXmStringTableFromDarray(widget, resourceName,
		resourceType, stringList,
		freeStack);

	return stringTable;
}

/* Handle enumerated types. */
Var *
gui_getEnum(const Widget widget,
	const String resourceName,
	const String resourceType)
{
	EnumNameList	allowed;
	int		typeIdx, nameIdx;
	unsigned char	enumVal;

	dbgprintf ("gui_getEnum(widget = %ld, "
		"resourceName = '%s', resourceType = '%s')\n",
		widget, resourceName, resourceType);

	XtVaGetValues(widget, resourceName, &enumVal, NULL);

	/* Lookup the value in the enumerated list of allowed values. */

	allowed = NULL;

	typeIdx = 0;
	while (EnumTypeNameMap[typeIdx].enumType != NULL) {
		if (!strcmp(EnumTypeNameMap[typeIdx].enumType, resourceType)) {
			allowed = EnumTypeNameMap[typeIdx].enumNameList;
			break;
		}
		typeIdx++;
	}

	if (allowed == NULL) {
		/* Unknown enum type.  Return the value anyway, with a warning. */
		parse_error("WARNING: unknown widget resource enum type encountered");
		return newInt(enumVal);
	}

	/* if widgetEnumData being collected but this type is not yet created */
	if (widgetEnumData && -1 == find_struct (widgetEnumData, resourceType, NULL)) {
		Var *values, *data;
		data = create_struct (NULL);
		/* build values struct and add it */
		values = create_struct (NULL);
		for (nameIdx = 0; allowed[nameIdx].enumName != NULL; nameIdx ++) {
			add_struct (values, allowed[nameIdx].enumName, newInt(0));
		}
		add_struct (data, "values", values);
		/* add empty names struct, filled in later on */
		add_struct (data, "names", create_struct(NULL));
		/* attach new data to widgetEnumData */
		add_struct (widgetEnumData, resourceType, data);
	}

	for (nameIdx = 0; allowed[nameIdx].enumName != NULL; nameIdx ++) {
		if (allowed[nameIdx].enumValue == enumVal) {
			/* Found a known value. */
			return newString(dupString(allowed[nameIdx].enumName));
		}
	}

	/* Illegal/unknown value. */
	parse_error("WARNING: unknown widget resource enum value encountered");
	dbgprintf ("value = %d\n", (int) enumVal);

	return newInt(-1);
}

/* XtArgVal
* gui_setEnum()
*
* NOTE: returns the existing value on error.
*
*/

XtArgVal
gui_setEnum(const Widget widget,
	const String resourceName,
	const String resourceType,
	const Var *value,
	FreeStackList freeStack)
{
	EnumNameList	allowed;
	unsigned int	typeIdx, nameIdx;
	unsigned char	oldValue;

	dbgprintf ("gui_setEnum(widget = %ld, "
			"resourceName = \"%s\", resourceType = '%s', value = %ld)\n",
			widget, resourceName, resourceType, value);

	if (widget != NULL) {
		XtVaGetValues(widget, resourceName, &oldValue, NULL);
	}
	else {
		oldValue = 0; /* Can't think of anything better. */
	}

	if (V_TYPE(value) != ID_STRING) {
		parse_error("attempt to widget enum resource from non-string");
		return (XtArgVal) oldValue;
	}

	/* Lookup the value in the enumerated list of allowed values. */

	allowed = NULL;

	typeIdx = 0;
	while (EnumTypeNameMap[typeIdx].enumType != NULL) {
		if (!strcmp(EnumTypeNameMap[typeIdx].enumType, resourceType)) {
			allowed = EnumTypeNameMap[typeIdx].enumNameList;
			break;
		}
		typeIdx++;
	}

	if (allowed == NULL) {
		/* Unknown enum type. */
		parse_error("attempt to set unhandled widget enum resource type");
		return (XtArgVal) oldValue;
	}

	nameIdx = 0;
	while (allowed[nameIdx].enumName != NULL) {
		if (!strcmp(allowed[nameIdx].enumName, V_STRING(value))) {
			/* Found a known value, return the resource value. */
			return (XtArgVal) allowed[nameIdx].enumValue;
		}
		nameIdx++;
	}

	/* Illegal/unknown value. */
	parse_error("attempt to set widget enum resource to unknown value");
	dbgprintf ("value = '%s'\n", V_STRING(value));
	return (XtArgVal) oldValue;
}

/* Various numeric types. */

Var *
gui_getBoolean(const Widget widget,
	const String resource,
	const String resourceType)
{
	Boolean	booleanValue;
	char	*returnValue;

	dbgprintf ("gui_getBoolean(widget = %ld, resource = \"%s\")\n", widget, resource);
	XtVaGetValues(widget, resource, &booleanValue, NULL);
	returnValue = booleanValue ? "true" : "false";
	return newString(dupString(returnValue));
}

XtArgVal
gui_setBoolean(const Widget widget,
	const String resourceName,
	const String resourceType,
	const Var *value,
	FreeStackList freeStack)
{
	Boolean	 booleanVal = False;

	dbgprintf ("gui_setBoolean(widget = %ld, "
			"resource = \"%s\", value = %ld)\n", widget, resourceName, value);

	if (widget != NULL)
		XtVaGetValues(widget, resourceName, &booleanVal, NULL);

	if (V_TYPE(value) == ID_STRING) {
		if (!strcasecmp(V_STRING(value), "true"))
			booleanVal = True;
		else if (!strcasecmp(V_STRING(value), "false"))
			booleanVal = False;
		else
			parse_error("attempt to set Boolean Xt resource to invalid value");
	} else {
		parse_error("attempt to set Boolean Xt resource from non-string");
	}

	return (XtArgVal) booleanVal;
}

Var *
gui_getBool(const Widget widget,
	const String resource,
	const String resourceType)
{
	Bool	boolValue;
	char	*returnValue;

	dbgprintf ("gui_getBool(widget = %ld, resource = \"%s\")\n", widget, resource);

	XtVaGetValues (widget, resource, &boolValue, NULL);
	returnValue = boolValue ? "true" : "false";
	return newString(dupString(returnValue));
}

XtArgVal
gui_setBool(const Widget widget,
	const String resourceName,
	const String resourceType,
	const Var *value,
	FreeStackList freeStack)
{
	Bool     boolVal = False;

	dbgprintf ("gui_setBool(widget = %ld, "
			"resource = \"%s\", value = %ld)\n", widget, resourceName, value);

	if (widget != NULL)
		XtVaGetValues(widget, resourceName, &boolVal, NULL);

	if (V_TYPE(value) == ID_STRING) {
		if (!strcasecmp(V_STRING(value), "true"))
			boolVal = True;
		else if (!strcasecmp(V_STRING(value), "false"))
			boolVal = False;
		else
			parse_error("attempt to set Bool Xt resource to invalid value");
	} else {
		parse_error("attempt to set Bool Xt resource from non-string");
	}

	return (XtArgVal) boolVal;
}

Var *
gui_getByte(Widget widget, String resourceName, String resourceType)
{
	Var		*o;
	char		byteValue;

	XtVaGetValues(widget, resourceName, &byteValue, NULL);

	o = newInt((int) byteValue);
	return o;
}

XtArgVal
gui_setByte(const Widget widget,
	const String resourceName,
	const String resourceType,
	const Var *value,
	FreeStackList freeStack)
{
	unsigned int	intVal;
	u_char	byteVal;
	u_char	oldValue;

	dbgprintf ("gui_setByte(widget = %ld, "
			"resourceName = \"%s\", resourceType = '%s', value = %ld)\n",
			widget, resourceName, resourceType, value);

	XtVaGetValues(widget, resourceName, &oldValue, NULL);

	intVal = extract_int(value, 0);

	/* Range checking. */
	if (intVal > MAXBYTE) {
		parse_error("attempt to set byte resource to value "
				"outside allowed range.");
		return (XtArgVal) oldValue;
	}

	byteVal = (u_char) intVal;
	return (XtArgVal) byteVal;
}

Var *
gui_getShort(const Widget widget,
	const String resourceName,
	const String resourceType)
{
	Var		*o;
	short		shortValue;

	XtVaGetValues(widget, resourceName, &shortValue, NULL);

	o = newInt((int) shortValue);
	return o;
}

XtArgVal
gui_setShort(const Widget widget,
	const String resourceName,
	const String resourceType,
	const Var *value,
	FreeStackList freeStack)
{
	int	intVal;
	short	shortVal;
	short	oldValue;

	dbgprintf ("gui_setByte(widget = %ld, "
			"resource = '%s', resourceType = '%s', value = %ld)\n",
			widget, resourceName, resourceType, value);

	XtVaGetValues(widget, resourceName, &oldValue, NULL);

	intVal = extract_int(value, 0);

	/* Range checking. */
	if (intVal < MINSHORT || intVal > MAXSHORT) {
		parse_error("attempt to set short resource to value "
				"outside allowed range.");
		return (XtArgVal) oldValue;
	}

	shortVal = (short) intVal;
	return (XtArgVal) shortVal;
}

Var *
gui_getFloat (const Widget widget,
	const String resourceName,
	const String resourceType)
{
	float floatValue;
	XtVaGetValues (widget, resourceName, &floatValue, NULL);
	return newFloat (floatValue);
}

/* Note!  This is an ugly, but simple hack to get around XLib not knowing
   float is cast to double in vararg lists. */
void
gui_setFloatNow (Widget w, char *name, float val)
{
	XtVaSetValues (w, name, *(int*)&val, NULL);
}

XtArgVal
gui_setFloat (const Widget widget,
	const String resourceName,
	const String resourceType,
	const Var *value,
	FreeStackList freeStack)
{
	float floatValue;
	floatValue = extract_float(value, 0);
	/* Note!  This is an ugly, but simple hack to get around XLib not knowing
	   float is cast to double in vararg lists */
	return (XtArgVal) *(int*)&floatValue;
}

Var *
gui_getDouble (const Widget widget,
	const String resourceName,
	const String resourceType)
{
	double doubleValue;
	XtVaGetValues (widget, resourceName, &doubleValue, NULL);
	return newDouble (doubleValue);
}

XtArgVal
gui_setDouble (const Widget widget,
	const String resourceName,
	const String resourceType,
	const Var *value,
	FreeStackList freeStack)
{
	double doubleValue;
	doubleValue = extract_double(value, 0);
	return (XtArgVal) doubleValue;
}

Var *
gui_getInt(const Widget widget,
	const String resourceName,
	const String resourceType)
{
	Var		*o;
	int		intValue;

	dbgprintf ("gui_getInt(widget = %ld, "
			"resourceName = '%s', resourceType = '%s')\n",
			resourceName, resourceType);

	XtVaGetValues(widget, resourceName, &intValue, NULL);

	o = newInt(intValue);
	return o;
}

XtArgVal
gui_setInt(const Widget widget,
	const String resourceName,
	const String resourceType,
	const Var *value,
	FreeStackList freeStack)
{
	int	intValue;

	dbgprintf ("gui_setInt(widget = %ld, "
			"resourceName = '%s', resourceType = '%s', value = %ld)\n",
			widget, resourceName, resourceType, value);

	intValue = extract_int(value, 0);
	dbgprintf ("intValue = %d\n", intValue);

	return (XtArgVal) intValue;
}

/* FIX: Cardinal is unsigned int, Davinci has no unsigned and no long.
*      Returning an int for now, but this sucks.  I don't want to use
*      a float.  Talk to Noel about adding long and/or unsigned.
*/

Var *
gui_getCardinal(const Widget widget,
	const String resourceName,
	const String resourceType)
{
	Var		*o;
	Cardinal	xtValue;

	dbgprintf ("gui_getCardinal(widget = %ld, "
			"resourceName = '%s', resourceType = '%s')\n",
			resourceName, resourceType);

	XtVaGetValues(widget, resourceName, &xtValue, NULL);

	o = newInt((signed int) xtValue);
	return o;
}

XtArgVal
gui_setCardinal(const Widget widget,
	const String resourceName,
	const String resourceType,
	const Var *value,
	FreeStackList freeStack)
{
	Cardinal	setValue;

	dbgprintf ("gui_setCardinal(widget = %ld, "
			"resourceName = '%s', resourceType = '%s', value = %ld)\n",
			widget, resourceName, resourceType, value);

	setValue = (Cardinal) extract_int(value, 0);

	return (XtArgVal) setValue;
}

	/* X Dimension objects. */

Var *
gui_getDimension(const Widget widget,
	const String resourceName,
	const String resourceType)
{
	Var		*o;
	Dimension	dimValue;

	dbgprintf ("gui_getDimension(widget = %ld, "
			"resourceName = '%s', resourceType = '%s')\n",
			resourceName, resourceType);

	XtVaGetValues(widget, resourceName, &dimValue, NULL);

	o = newInt((int) dimValue);
	return o;
}

XtArgVal
gui_setDimension(const Widget widget,
	const String resourceName,
	const String resourceType,
	const Var *value,
	FreeStackList freeStack)
{
	Dimension	setValue;

	dbgprintf ("gui_setDimension(widget = %ld, "
			"resourceName = '%s', resourceType = '%s', value = %ld)\n",
			widget, resourceName, resourceType, value);

	setValue = (Dimension) extract_int(value, 0);
	return (XtArgVal) setValue;
}

/* Handler for ignored resources.  This is a hack to allow us to intentionally
* ignore some resource types.
*/

Var *
gui_getIgnore(const Widget widget,
	const String resourceName,
	const String resourceType)
{
	dbgprintf ("gui_getIgnore(widget = %ld, "
			"resourceName = '%s', resourceType = '%s')\n",
			widget, resourceName, resourceType);

	return NULL;
}

/* Handler for read-only resources.  Produces an error if called to set. */
/* FIX: is this needed anymore? */

XtArgVal
gui_setReadOnly(const Widget widget,
		const String resourceName,
		const String resourceType,
		const Var *value,
		FreeStackList freeStack)
{
	parse_error("attempt to set a read-only resource");
	return (XtArgVal) NULL;
}

/* Var *
* gui_getTextPosition(widget, resource)
*
* Returns: XmTextPosition cast to int.  It's really a long (at least in the
* Lesstif implementation), but Davinci has no concept of longs, and I don't
* think anyone's going to be shoving more than 2GB of text into a widget.
*
*/

/* FIX: make sure these work. */

Var *
gui_getTextPosition(const Widget widget,
	const String resourceName,
	const String resourceType)
{
	Var			*o;
	XmTextPosition	tpValue;

	dbgprintf ("gui_getTextPosition(widget = %ld, "
			"resourceName = '%s', resourceType = '%s')\n",
			widget, resourceName, resourceType);

	XtVaGetValues(widget, resourceName, &tpValue, NULL);

	o = newInt((int) tpValue);
	return o;
}

XtArgVal
gui_setTextPosition(const Widget widget,
	const String resourceName,
	const String resourceType,
	const Var *value,
	FreeStackList freeStack)
{
	int intValue;

	dbgprintf ("gui_setTextPosition(widget = %ld, "
		"resourceName = '%s', resourceType = '%s', value = %ld)\n",
		widget, resourceName, resourceType, value);

	intValue = extract_int(value, 0);
	return (XtArgVal) intValue;
}

/*****************************************************************************/

void
gui_freeStackPush(FreeStackList freeStack, void *address)
{
	FreeStackEntryPtr	newAddr;

	dbgprintf ("gui_freeStackPush(freeStack = %ld, address = %ld)\n",
		freeStack, address);

	newAddr = (FreeStackEntryPtr) malloc(sizeof(FreeStackEntry));

	newAddr->address = address;
	newAddr->next = NULL;

	if (freeStack->head == NULL) {
		freeStack->head = newAddr;
	} else {
		freeStack->tail->next = newAddr;
	}

	freeStack->tail = newAddr;
}

void
gui_freeStackFree(FreeStackList freeStack)
{
	FreeStackEntryPtr	cursor;

	dbgprintf ("gui_freeStackFree(freeStack = %ld)\n", freeStack);

	cursor = freeStack->head;

	while (cursor != NULL) {
		dbgprintf ("free(%ld)\n", cursor->address);
		free(cursor->address);
		cursor = cursor->next;
	}

	freeStack->head = freeStack->tail = NULL;
}

/* Var *
* gui_getWidget()
*
* Handles Widget resource types by mapping to/from Davinci widgetIds.
*
*/

Var *
gui_getWidget(const Widget widget,
	const String resourceName,
	const String resourceType)
{
	Var		*o;
	Widget	widgetValue;
	int		widgetId;

	dbgprintf ("gui_getWidget(widget = %ld, "
			"resourceName = '%s', resourceType = '%s')\n",
			resourceName, resourceType);

	XtVaGetValues(widget, resourceName, &widgetValue, NULL);

	widgetId = getIdFromWidget(widgetValue);

	if (widgetId == DV_INVALID_WIDGET_ID) {
		o = newInt(-1);
	}
	else {
		o = newInt(widgetId);
	}

	return o;
}

/* XtArgVal
* gui_setWidget()
*
* Handles Widget resource types by mapping to/from Davinci widgetIds.
*
*/

XtArgVal
gui_setWidget(const Widget widget,
	const String resourceName,
	const String resourceType,
	const Var *value,
	FreeStackList freeStack)
{
	int		widgetId;
	Widget	widgetValue;

	dbgprintf ("gui_setWidget(widget = %ld, "
			"resourceName = '%s', resourceType = '%s', value = %ld)\n",
			widget, resourceName, resourceType, value);

	widgetId = extract_int(value, 0);

	if (widgetId == DV_INVALID_WIDGET_ID) {
		parse_error("Invalid widgetid in Widget resource.");
	}

	/* If the widgetId is invalid, widgetValue will be NULL, which is what
	 * it would be set to anyway in the event of an invalid widgetId..
	 */

	widgetValue = getWidgetFromId(widgetId);
	return (XtArgVal) widgetValue;
}

char *
dupString(const char *source)
{
	char *dest;

	/* FIX: what about when source is null? */

	dest = malloc((strlen(source) + 1) * sizeof(char *));

	if (dest != NULL) {
		strcpy(dest, source);
	}

	return dest;
}

/* Darray *
* gui_extractDarray(Var *dvResourceList)
*
* Builds a list of strings in a Darray (NULL values) from a Davinci Var.
* Supported Var types are ID_STRING, ID_TEXT, and ID_STRUCT (values only).
*
* Returns pointer to a new Darray.  Caller is responsible for freeing memory.
* Returns NULL and prints failure message on any error.
*
*/

Darray *
gui_extractDarray(const Var *value)
{
	Darray	*stringList;
	TextArray	textArray;
	int		nameIdx, structCount;
	Var		*elementValue;

	dbgprintf ("gui_extractDarray(value = %ld)\n", value);

	stringList = NULL;

	if (value != NULL)
	{
		switch (V_TYPE(value))
		{
			case ID_STRING:
				stringList = Darray_create(1);
				if (Darray_add(stringList, V_STRING(value)) == -1) {
					parse_error("Internal error: unable to add to Darray.");
					Darray_free(stringList, NULL);
					stringList = NULL;
				}
				break;

			case ID_TEXT:
				textArray = V_TEXT(value);
				stringList = Darray_create(textArray.Row); /* 0 is ok. */
				for (nameIdx = 0; nameIdx < textArray.Row; nameIdx++) {
					if (Darray_add(stringList, textArray.text[nameIdx]) == -1) {
						parse_error("Internal error: unable to add to Darray.");
						Darray_free(stringList, NULL);
						stringList = NULL;
					}
				}
				break;

			case ID_STRUCT:
				structCount = get_struct_count(value);
				stringList = Darray_create(structCount); /* 0 is ok. */
				for (nameIdx = 0; nameIdx < structCount; nameIdx++) {
					get_struct_element(value, nameIdx, NULL, &elementValue);
					if (V_TYPE(elementValue) != ID_STRING) {
						parse_error("Error: expecting STRING values in struct.");
						Darray_free(stringList, NULL);
						stringList = NULL;
						break;
					}
					if (Darray_add(stringList, V_STRING(elementValue)) == -1) {
						parse_error("Internal error: unable to add to Darray.");
						Darray_free(stringList, NULL);
						stringList = NULL;
						break;
					}
				}
				break;

			default:
				parse_error("Error: string list must be STRING, TEXT, or STRUCT.");
				return NULL;
		}
	}

	return stringList;
}

/* Narray *
* gui_extractNarray(Var *dvResourceList)
*
* Builds a list of strings in an Narray (NULL values) from a Davinci Var.
* Supported Var types are ID_STRING, ID_TEXT, and ID_STRUCT (values only).
*
* Returns pointer to a new Narray.  Caller is responsible for freeing memory.
* Returns NULL and prints failure message on any error.
*
*/

Narray *
gui_extractNarray(const Var *value)
{
	Narray	*stringList;
	TextArray	textArray;
	int		nameIdx, structCount;
	Var		*elementValue;

	dbgprintf ("gui_extractNarray(value = %ld)\n", value);

	stringList = NULL;

	if (value != NULL) { 

		switch (V_TYPE(value)) {

			case ID_STRING:
				stringList = Narray_create(1);
				if (Narray_add(stringList, V_STRING(value), NULL) == -1) {
					parse_error("Internal error: unable to create Narray.");
					Narray_free(stringList, NULL);
					stringList = NULL;
				}
				break;

			case ID_TEXT:
				textArray = V_TEXT(value);
				stringList = Narray_create(textArray.Row); /* 0 is ok. */
				for (nameIdx = 0; nameIdx < textArray.Row; nameIdx++) {
					if (Narray_add(stringList, textArray.text[nameIdx], NULL) == -1) {
						parse_error("Internal error: unable to create Narray.");
						Narray_free(stringList, NULL);
						stringList = NULL;
					}
				}
				break;

			case ID_STRUCT:
				structCount = get_struct_count(value);
				stringList = Narray_create(structCount); /* 0 is ok. */
				for (nameIdx = 0; nameIdx < structCount; nameIdx++) {
					get_struct_element(value, nameIdx, NULL, &elementValue);
					if (V_TYPE(elementValue) != ID_STRING) {
						parse_error("Error: expecting STRING values in struct.");
						Narray_free(stringList, NULL);
						stringList = NULL;
						break;
					}
					if (Narray_add(stringList, V_STRING(elementValue), NULL) == -1) {
						parse_error("Internal error: unable to create Narray.");
						Narray_free(stringList, NULL);
						stringList = NULL;
						break;
					}
				}
				break;

			default:
				parse_error("Error: string list must be STRING, TEXT, or STRUCT.");
				return NULL;

		}
	}

	return stringList;
}

#if 0
static Boolean
isValidWidget(MyWidgetList widgetListEntry)
{

	if (widgetListEntry == NULL || widgetListEntry->valid != True) {
		return False;
	}

	return True;

}
#endif

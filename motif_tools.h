/******************************************************************************
*
* Copyright (c) 1999, KL GROUP INC.  All Rights Reserved.
* http://www.klgroup.com
*
* This file is provided for demonstration and educational uses only.
* Permission to use, copy, modify and distribute this file for
* any purpose and without fee is hereby granted, provided that the
* above copyright notice and this permission notice appear in all
* copies, and that the name of KL Group not be used in advertising
* or publicity pertaining to this material without the specific,
* prior written permission of an authorized representative of
* KL Group.
*
* KL GROUP MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY
* OF THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
* TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
* PURPOSE, OR NON-INFRINGEMENT. KL GROUP SHALL NOT BE LIABLE FOR ANY
* DAMAGES SUFFERED BY USERS AS A RESULT OF USING, MODIFYING OR
* DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
*
******************************************************************************/

#ifndef _motif_tools_h
#define _motif_tools_h

#ifdef MOTIF_TOOLS
#ifdef NDEBUG
static char sccsid_h[] = "@(#)motif_tools.h	4.42 98/12/11	KL Group Inc.";
#endif
#endif

#ifndef Xmalloc
#	define Xmalloc(size) malloc(((size) > 0 ? (size) : 1))
#	define Xrealloc(ptr, size) realloc((ptr), ((size) > 0 ? (size) : 1))
#	define Xcalloc(nelem, elsize) calloc(((nelem) > 0 ? (nelem) : 1), (elsize))
#	define Xfree(ptr) free(ptr)
#endif

#ifndef XTFREE
#	define XTFREE(ptr) XtFree((char *)(ptr))
#	define XTMALLOC(type, size) ((size) > 0 ? \
					(type *)XtMalloc((size) * sizeof(type)) : NULL);
#	define XTREALLOC(ptr, type, size) (ptr \
						   ? (type *)XtRealloc((char *)ptr,(size)*sizeof(type))\
						   : (type *)XtMalloc((size)*sizeof(type)))
#	define XTCALLOC(type, size)    (type *)XtCalloc((size), sizeof(type))
#endif

/*
 * A little magic to make SunOS compile free of warnings with "gcc -Wall".
 */
  
#if 0 && defined(sun) && !defined(SOLARIS) && defined(__GNUC__)
int fputs(char *, FILE *fp);
int printf(char *format, ...);
int fprintf(FILE *stream, char *format, ...);
int sscanf(char *str, char *format, ...);
void bcopy(char *b1, char *b2, int length);
int tolower(int);
int toupper(int);
long strtol(const char *str, char **ptr, int base);
double strtod(const char *str, char **ptr);
int fflush(FILE *stream);
void perror(char *s);
void fclose(FILE *stream);
void pclose(FILE *stream);
int fread(char *ptr, int size, int nitems, FILE *stream);
int fwrite(char *ptr, int size, int nitems, FILE *stream);
int rename(char *from, char *to);
int putenv(char *string);
char *memset(char *s, int c, int n);
int system(char *string);
#endif

#ifndef Max
#	define Max(x, y)	(((x) > (y)) ? (x) : (y))
#	define Min(x, y)	(((x) < (y)) ? (x) : (y))
#endif

#define OK		"OK"
#define CANCEL	"Cancel"
#define HELP	"Help"
#define LABEL	"label"
#define FIELD	"field"
#define BUTTON	"button"

typedef struct _MenuItem {
	String		name;
	String		label;
	WidgetClass	*w_class;
	void		(*callback)(_WidgetRec *, void *, void *);
	XtPointer	data;
	int			toggle_button_type;		/* XmRADIOBUTTON or XmCHECKBUTTON */
	struct _MenuItem *subitems;			/* pullright menu items, if not NULL */
} MenuItem;

typedef struct _MenuOptionItem {
	String		name;
	String		label;
	MenuItem	*menu;
} CascadeButtonItem;

typedef struct {
	String		name;
	String		label;
	void		(*callback)(_WidgetRec *, void *, void *);
	XtPointer	data;
	Boolean		set;
} ButtonItem;

typedef struct {
	String		name;
	String		label;
	void		(*callback)(_WidgetRec *, void *, void *);
	XtPointer	data;
	int			length;
	String		value;
	Boolean		sensitive;
} FieldItem;

typedef struct {
	int *min, *max;
	Widget	container, text, up_arrow, down_arrow;
} CounterItem;

typedef struct {
	String		name;
	String		label;
	void		(*drag_callback)(_WidgetRec *, void *, void *);
	void		(*value_callback)(_WidgetRec *, void *, void *);
	XtPointer	data;
	int			min;
	int			max;
	int			value;
} SliderItem;

typedef struct {
	String		ok_label;
	String		cancel_label;		/* NULL = "Cancel" */
	String		help_label;			/* NULL = "Help" */
	String		dir_mask;
	void		(*ok_callback)(_WidgetRec *, void *, void *);
	XtPointer	ok_data;
	void		(*cancel_callback)(_WidgetRec *, void *, void *);
	XtPointer	cancel_data;
	void		(*help_callback)(_WidgetRec *, void *, void *);
	XtPointer	help_data;
} FileSelectionData;

typedef struct {
	String label;
	String *info;
} MtInfoStruct;

typedef enum {
	TOOL_INT = 1,
	TOOL_FLOAT,
	TOOL_STRING,
	TOOL_BOOLEAN
} Datatype;

typedef Widget (*MtCreatePopupChildProc)(
#ifndef _NO_PROTO
		Widget parent,
		String label,
		XtPointer data
#endif
);

#if defined(__QNX__)
#define Datatype int
#endif

typedef Widget (*WorkAreaFunc)(...);

#if defined(__cplusplus)
extern "C" {
#endif

#ifdef _NO_PROTO
extern void 		DialogCB();
extern Widget 		GetChildByUserData();
extern void			MtAlignLabels();
extern String 		MtBooleanToString();
extern Widget 		MtBuildMenu();
extern Widget 		MtBuildRadioBox();
extern void 		MtCloseDialogCB();
extern void 		MtCloseFileSelectorCB();
extern Widget 		MtCreateActionArea();
extern Widget 		MtCreateCascadeButtonBox();
extern CounterItem* MtCreateCounter();
extern Widget 		MtCreateFieldBox();
extern Widget 		MtCreateFieldRadioBox();
extern Widget 		MtCreateLabeledBox();
extern Widget 		MtCreateLabeledContainer();
extern String 		MtCreateName();
extern Widget 		MtCreateOptionMenuBox();
extern void 		MtCreatePopupShell();
extern Widget 		MtCreatePromptDialog();
extern Widget 		MtCreateShell();
extern Widget 		MtCreateSliderField();
extern Widget 		MtCreateTextField();
extern XmString 	MtCreateXmString();
extern XmString 	MtCreateXmString();
extern XmString 	MtCreateXmStringFromStringArray();
extern XmFontList 	MtCvtFontStringToFontList();
extern String 		MtCvtPixelToString();
extern Pixel 		MtCvtStringToPixel();
extern String 		MtCvtXmStringToString();
extern void 		MtDisplayFileSelector();
extern void 		MtDisplayMessageDialog();
extern String  		MtDisplayModal();
extern void 		MtDisplayPromptDialog();
extern void 		MtExpandFilename();
extern int 			MtFindChildren();
extern String 		MtFloatToString();
extern Widget 		MtGetChild();
extern Widget 		MtGetShell();
extern Widget 		MtGetSibling();
extern Boolean 		MtGetToken();
extern Widget 		MtGetTopLevel();
extern Widget 		MtGetTopShell();
extern Boolean 		MtGetValue();
extern Boolean 		MtGetWidgetValue();
extern String 		MtIntToString();
extern Boolean 		MtMonoDisplay();
extern int 			MtRandomInt();
extern void 		MtSeedRandom();
extern void 		MtSetBackingStore();
extern void 		MtSetLabel();
extern void 		MtSetSensitive();
extern Boolean 		MtSetValue();
extern Boolean 		MtSetWidgetValue();
extern void 		MtSetXmStringConverter();
extern Boolean 		MtStringCompare();

#else

extern Widget 		GetChildByUserData(Widget parent, XtPointer userData);
extern void 		MtAlignLabels(Widget dummy, ...);
extern String 		MtBooleanToString(Boolean value);
extern Widget 		MtBuildMenu(Widget parent, int menu_type, String name,
								String title, MenuItem *items); 
extern Widget 		MtBuildRadioBox(Widget parent, String name, String title,
									ButtonItem *buttons, int orientation, int
									button_type); 
extern void 		MtCloseDialogCB(Widget w);
extern void 		MtCloseFileSelectorCB(Widget w);
extern Widget 		MtCreateActionArea(Widget parent, ButtonItem *buttons);
extern Widget 		MtCreateCascadeButtonBox(Widget parent, String name, 
									String title, CascadeButtonItem *buttons, 
									int orientation, int num_columns);
extern CounterItem* MtCreateCounter(Widget parent, FieldItem *fielditem, 
									int min, int max);
extern Widget 		MtCreateFieldBox(Widget parent, String name, String title,
									 FieldItem *fields, int orientation);
extern Widget 		MtCreateFieldRadioBox(Widget parent, String name, 
										  String title, FieldItem *fields, 
										  int orientation);
extern Widget 		MtCreateLabeledBox(Widget parent, String name, 
									   String label, Widget *box);
extern Widget 		MtCreateLabeledContainer(Widget parent, String name, 
											 String label, 
											 WidgetClass containerWidgetClass,
											 Widget *frame);
extern String 		MtCreateName(String name, String type);
extern Widget 		MtCreateOptionMenuBox(Widget, String, String, 
										  CascadeButtonItem *, int, int);
extern void 		MtCreatePopupShell(Widget parent, 
									   MtCreatePopupChildProc create_proc, 
									   String title, XtPointer data);
extern Widget 		MtCreatePromptDialog(Widget parent);
extern Widget 		MtCreateShell(Widget parent, String name, String title, 
								  WorkAreaFunc work_area_func, 
								  MenuItem *menu_items, 
								  ButtonItem *action_area_items, 
								  Boolean display_msg_area);
extern Widget 		MtCreateSliderField(Widget parent, SliderItem *slideritem);
extern Widget 		MtCreateTextField(Widget parent, FieldItem *fielditem);
extern XmString 	MtCreateXmString(String text);
extern XmString 	MtCreateXmString(String text);
extern XmString 	MtCreateXmStringFromStringArray(String *text);
extern XmFontList 	MtCvtFontStringToFontList(Widget w, String str, XmFontType type);
extern String 		MtCvtPixelToString(Widget w, Pixel pixel);
extern Pixel  		MtCvtStringToPixel(Widget w, String color);
extern String 		MtCvtXmStringToString(XmString string);
extern void 		MtDisplayFileSelector(Widget parent, String title, 
										  FileSelectionData *data);
extern void 		MtDisplayMessageDialog(Widget parent, String title, 
										   String message, int type, 
										   ButtonItem *buttons);
extern String 		MtDisplayModal(Widget shell, ButtonItem *action_buttons);
extern void 		MtDisplayPromptDialog(Widget dialog, String title, 
										  String message, ButtonItem *buttons);
extern void 		MtExpandFilename(String filename, String new_filename);
extern int 			MtFindChildren(Widget parent, Widget **children, 
								   Boolean normal, Boolean popup); 
extern String 		MtFloatToString(float value);
extern Widget 		MtGetChild(Widget parent, String name);
extern Widget 		MtGetShell(Widget w);
extern Widget 		MtGetSibling(Widget w, String name);
extern Boolean 		MtGetToken(String *string, char delim, String *token);
extern Widget 		MtGetTopLevel(Widget w);
extern Widget 		MtGetTopShell(Widget w);
extern Boolean 		MtGetValue(Widget widget, String name, int type, 
							   XtArgVal *value);
extern Boolean 		MtGetWidgetValue(Widget widget, int datatype, 
									 XtArgVal *value);
extern String 		MtIntToString(int value);
extern Boolean 		MtMonoDisplay(Display	*display);
extern int 			MtRandomInt(int a, int b);
extern void 		MtSeedRandom(void);
extern void 		MtSetBackingStore(Widget w);
extern void 		MtSetLabel(Widget, String label);
extern void 		MtSetSensitive(Widget w, String name, Boolean value);
extern Boolean 		MtSetValue(Widget w, String name, int type, 
							   XtArgVal value);
extern Boolean 		MtSetWidgetValue(Widget widget, int datatype, 
									 XtArgVal value);
extern void 		MtSetXmStringConverter(void);
extern Boolean 		MtStringCompare(String in, String test, int num_char);

#endif

#if defined(__cplusplus)
}
#endif

#endif /* _motif_tools_h */

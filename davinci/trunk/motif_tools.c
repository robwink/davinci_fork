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
#ifdef XRT_ENABLED
#define MOTIF_TOOLS

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#ifdef __VMS
# include "vms_param.h"
#elif defined(__QNX__)
# include <rpc/types.h>
# include <limits.h>
# define MAXPATHLEN PATH_MAX
#elif defined(sco)
# include <sys/param.h>
# define MAXPATHLEN PATHSIZE
#else
# include <sys/param.h>
#endif /* __VMS */

#ifndef MAXPATHLEN
#define MAXPATHLEN 200
#endif

#if __STDC__
#undef _NO_PROTO
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <Xm/AtomMgr.h>
#include <Xm/ArrowB.h>
#include <Xm/BulletinB.h>
#include <Xm/CascadeB.h>
#include <Xm/DialogS.h>
#include <Xm/FileSB.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/MainW.h>
#include <Xm/MessageB.h>
#include <Xm/PanedW.h>
#include <Xm/Protocols.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/SelectioB.h>
#include <Xm/SeparatoG.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#include <Xm/XmP.h>
#include "motif_tools.h"

#define TIGHTNESS	20

#ifdef NDEBUG
static char	sccsid[] = "@(#)motif_tools.c	4.104 98/12/21	KL Group Inc.";
#endif

XmString
#ifndef _NO_PROTO
MtCreateXmStringFromStringArray(String *text)
#else
    MtCreateXmStringFromStringArray(text)
    String *text;
#endif
{
    int			line;
    XmString	str = NULL, tmp1, tmp2, separator;

#if (XmVersion < 2000) || defined(__linux)
    separator = XmStringSeparatorCreate();
    for (line = 0; text[line]; line++) {
        if (!str) {
            str = XmStringCreate("", XmFONTLIST_DEFAULT_TAG);
        }
        else {
            tmp1 = XmStringConcat(str, separator);
            XmStringFree(str);
            str = tmp1;
        }

        tmp1 = MtCreateXmString(text[line]); 
        tmp2 = XmStringConcat(str, tmp1);
        XmStringFree(tmp1);		
        XmStringFree(str);
        str = tmp2;
    }
    XmStringFree(separator);
#else
    str = XmStringCreate("", XmFONTLIST_DEFAULT_TAG);
#endif

    return(str);
}


Pixel 
#ifndef _NO_PROTO
MtCvtStringToPixel(Widget w, String color)
#else
    MtCvtStringToPixel(w, color)
    Widget w;
    String color;
#endif
{
    XrmValue	from, to;
    Pixel		pixel = 0;

    from.addr = (caddr_t) color;
    from.size = strlen(color);
    to.addr = (caddr_t) &pixel;
    to.size = sizeof(pixel);
    (void) XtConvertAndStore(w, XmRString, &from, XmRPixel, &to);
    return (pixel);
}

String
#ifndef _NO_PROTO
MtCvtPixelToString(Widget w, Pixel pixel)
#else
    MtCvtPixelToString(w, pixel)
    Widget	w;
    Pixel	pixel;
#endif
{
    char				string[50];
    XColor				color;
    XWindowAttributes	wattrs;

    string[0] = '\0';

    /*
      if (!XtIsRealized(w)) {
      return(NULL);
      }
      */
	
    if (pixel == 0) {
        return(NULL);
    }

    if (!XGetWindowAttributes(XtDisplay(w), XtWindow(w), &wattrs)) {
        return (False);
    }

    color.pixel = pixel;
    color.flags = DoRed | DoGreen | DoBlue;
    XQueryColor(XtDisplay(w), wattrs.colormap, &color);
    sprintf(string, "#%04X%04X%04X", color.red, color.green, color.blue);
	
    return(XtNewString(string));
}

XmFontList
#ifndef _NO_PROTO
MtCvtFontStringToFontList(Widget parent, char *font_str, XmFontType type)
#else
    MtCvtFontStringToFontList(parent, font_str, type)
    Widget		 parent;
    char		*font_str;
    XmFontType	 type;
#endif
{
    XmFontList		fl;
    XmFontListEntry entry;
    Widget			vendor_shell;

    entry =  XmFontListEntryLoad(XtDisplay(parent), font_str, type,
                                 XmFONTLIST_DEFAULT_TAG);
    if (NULL == entry) {
        char buf[128];
		
        sprintf(buf,
                "Can't create font list entry using '%s'.\nUsing default font list.\n",
                font_str); 
        XtWarning(buf);
        vendor_shell = parent;
        while (NULL != vendor_shell && !XmIsVendorShell(vendor_shell)) {
            vendor_shell = XtParent(vendor_shell);
        }

        if (NULL == vendor_shell) {
            sprintf(buf,
                    "Cannot find vendor shell to get the default fontlist. Exiting...\n");
            XtError(buf);
            exit(1);
        }
        XtVaGetValues(vendor_shell, XmNdefaultFontList, &fl, NULL);
    }
    else {
        fl = XmFontListAppendEntry(NULL, entry);
        XmFontListEntryFree(&entry);
    }

    return(fl);
}

void
#ifndef _NO_PROTO
MtCreatePopupShell(Widget parent, MtCreatePopupChildProc create_proc,
                   String title, XtPointer data)
#else
    MtCreatePopupShell(parent, create_proc, title, data)
    Widget					parent;
    MtCreatePopupChildProc	create_proc;
    String					title;
    XtPointer				data;
#endif
{
    Atom	WM_DELETE_WINDOW;
    Widget	dialog, page;

    /* Create the dialog */
    dialog = XtVaCreatePopupShell(title,
                                  xmDialogShellWidgetClass, MtGetShell(parent),
                                  XmNtransient,             False,
                                  XmNallowShellResize,      True,
                                  XmNdeleteResponse, 		XmDO_NOTHING,
                                  NULL);
    page = create_proc(dialog, title, data);
    WM_DELETE_WINDOW = XmInternAtom(XtDisplay(dialog),
                                    "WM_DELETE_WINDOW", False);
    XmAddWMProtocolCallback(dialog, WM_DELETE_WINDOW,
                            (XtCallbackProc) MtCloseDialogCB, (XtPointer) 0);

    XtManageChild(page);
    XtPopup(dialog, XtGrabNone);
}


typedef struct {
    int		min, max;
    Widget	container, text, up_arrow, down_arrow;
    void	(*callback)();
} Counter;

/*
 *  Align the NULL terminated list of labels
 */
void
#if __STDC__
MtAlignLabels(Widget dummy, ...) /* Only use this if ANSI */
#else
    MtAlignLabels(va_alist)
    va_dcl
#endif
{
    Dimension		width, max;
    va_list			pvar;
    Widget			cur_label;

    max = 0;
#if __STDC__
    va_start(pvar, dummy);
    XtVaGetValues(dummy, XmNwidth, &width, NULL);
    max = width;
#else
    va_start(pvar);
#endif
    while ((cur_label = (Widget) va_arg(pvar, Widget)))	{
        XtVaGetValues(cur_label, XmNwidth, &width, NULL);
        if (width > max) {
            max = width;
        }
    }
    va_end(pvar);
#if __STDC__
    va_start(pvar, dummy);
    XtVaSetValues(dummy, 
                  XmNwidth,				max, 
                  XmNalignment,			XmALIGNMENT_END,
                  XmNrecomputeSize,		FALSE,
                  NULL);
#else
    va_start(pvar);
#endif
    while ((cur_label = (Widget) va_arg(pvar, Widget)))	{
        XtVaSetValues(cur_label, 
                      XmNwidth,				max, 
                      XmNalignment,			XmALIGNMENT_END,
                      XmNrecomputeSize,		FALSE,
                      NULL);
    }
    va_end(pvar);
}

/*
 * Expand environment variables in filename 
 */
void
#ifndef _NO_PROTO
MtExpandFilename(String filename, String new_filename)
#else
    MtExpandFilename(filename, new_filename)
    String filename, new_filename;
#endif
{
    int      bufflen;
    char     buff[MAXPATHLEN], *ptr;

#ifndef __VMS
    /*
     *  Expand the directory name contained in filename into new_filename;
     *  this routine will perform variable expansion if the first
     *  character of filename is a '$'; it uses getenv() to extract the
     *  name of the variable from the environment.  However, all path
     *  components after the first are treated as actual path components
     *  (i.e. no environment variable substitution is done).
     */
    strcpy(new_filename, filename);
    if (filename[0] == '$')  {
        /*
         * Get the environment variable name (after '$', and before '/', if
         * any)
         */
        ptr = strchr(filename, '/');
        if (ptr) {
            bufflen = ptr-filename-1;
            strncpy(buff, filename+1, bufflen);
            buff[bufflen] = (char) NULL;
        }
        else    {
            bufflen = strlen(filename) - 1;
            strcpy(buff, filename+1);
        }
        /*
         * Get the pointer to the string associated with the environment
         * variable.
         */
        if (!(ptr = getenv(buff)))   {
            return;
        }
        strcpy(new_filename, ptr);
        strcat(new_filename, filename+bufflen+1);
    }
#else  /* __VMS */ 
    /* 
     * This routine detects a file name specified in UNIX format and
     * translates it to VMS format. A UNIX file name format is detected by 
     * looing for a '/' in variable filename. If the file name is specifiend 
     * in UNIX format and the first character of the filename is a '$', then 
     * the the word between '$' and the first '/' is assumed to be a 
     * UNIX-type envirionment variable. This variable is translated to the 
     * VMS virtual device name. Any directories and the name of the file 
     * that follow the envirionment variable are translated into VMS format, 
     * and the VMS-style file name is put in new_filename.
     */
    char *ptrlast, *ptrnext;

    new_filename[0] = (char) NULL;
    if ( (ptr = strchr(filename, '/')) ) {
        ptrlast = strrchr(filename, '/');
        if (filename[0] == '$') {
            /* 
             * Unix style file name with environment variable was specified, get
             * the variable name
             */
            bufflen = ptr-filename-1;
            strncpy(new_filename, filename+1, bufflen);
            new_filename[bufflen] = (char) NULL;
            strcat(new_filename,":[");

            if (ptr == ptrlast) {
                /* there is only one '/' */
                strcat(new_filename, "]");
                strcat(new_filename, ptrlast+1);
                return;
            } 
            else { 
                /* there is more than one '/' */
                ptrnext = strchr(ptr+1, '/');
                bufflen = ptrnext-ptr-1;
                strncpy(buff, ptr+1, bufflen);
                buff[bufflen] = (char) NULL;
                strcat(new_filename, buff);
                strcat(new_filename,".");
                ptr = ptrnext;
            }
        }
        else {
            strcpy(new_filename,"[");
            bufflen = ptr-filename;
            strncpy(buff, filename, bufflen);
            buff[bufflen] = (char) NULL;
            strcat(new_filename, buff);
            if (ptr == ptrlast) {
                strcat(new_filename, "]");
                strcat(new_filename, ptrlast+1);
                return;
            }
            else {
                strcat(new_filename, ".");
            }
        }

        while (ptr != ptrlast) {
            ptrnext = strchr(ptr+1, '/');
            bufflen = ptrnext-ptr-1;
            strncpy(buff, ptr+1, bufflen);
            buff[bufflen] = '.';
            buff[bufflen+1] = (char) NULL;
            strcat(new_filename, buff);
            ptr = ptrnext;
        }
        new_filename[strlen(new_filename)-1] = ']';
        strcat (new_filename, ptrlast+1);
    }
    else {
        strcpy(new_filename, filename);
    }
#endif /* __VMS */
}

/*
 * Case-insensitive string compare, ignoring trailing blanks in 1st string
 * If num_char > 0, comparison ends after this many characters
 */
Boolean
#ifndef _NO_PROTO
MtStringCompare(String in, String test, int num_char)
#else
    MtStringCompare(in, test, num_char)
    String	in, test;
    int		num_char;
#endif
{
    int		i, inLen, testLen;
    String	in2 = in;

    if (!in || strlen(in) == 0 || !test) {
        return (False);
    }

    /* Strip leading whitespace */
    for (; *in2 && isspace(*in2); in2++) {
        ;
    }

    if (!in2) {
        return (False);
    }

    inLen = strlen(in2);
    testLen = strlen(test);
    for (i = 0; i < Min(inLen, testLen); i++, in2++) {
        if (tolower(*in2) != tolower(test[i])) {
            return (False);
        }
        else if (num_char > 0 && i == num_char-1) {
            return (True);
        }
    }

    if (inLen < testLen) {
        return (False);
    }

    for (; i < inLen; i++, in2++) {
        if (!isspace(in2[i])) {
            return (False);
        }
        else if (num_char > 0 && i == num_char-1) {
            return (True);
        }
    }
    return (True);
}

/*
 * Get next token from delimited string.
 * The delimeter can be "escaped" by a \
 * To include "\,", precede the \ by a \, 
 * 		e.g.: a,b\,c,\\,d ==> a  b,c  \,d
 *
 * "token" must be freed after use
 */
Boolean 
#ifndef _NO_PROTO
MtGetToken(String *string, char delim, String *token)
#else
    MtGetToken(string, delim, token)
    char **string, delim, **token;
#endif 
{
    String tmp;

    *token = NULL;
    if (!*string || **string == '\0' || strlen(*string) == 0) {
        return (False);
    }

    *token = XtMalloc(strlen(*string) + 1);
    tmp = *token;

    while((**string) != '\0') {
        if ((**string) == '\\' && *((*string)+1) == delim) { /* Quoted delim */
            (*string) += 2;
            *tmp = delim;
            tmp++;
        }
        else {
            if ((**string) == delim) { /* End of a string */
                *tmp = '\0';
                (*string)++;
                return (True);
            }
            else {
                *tmp = **string;
                tmp++; 
                (*string)++;
            }
        }
    }

    *tmp = '\0';
    return (True);
}

String
#ifndef _NO_PROTO
MtIntToString(int value)
#else
    MtIntToString(value)
    int value;
#endif
{
    static char buf[20];
	
    sprintf((char *)buf, "%d", value);
    return (buf);
}

String
#ifndef _NO_PROTO
MtFloatToString(float value)
#else
    MtFloatToString(value)
    float value;
#endif
{
    static char buf[20];
	
    sprintf(buf, "%g", value);
    return (buf);
}

String
#ifndef _NO_PROTO
MtBooleanToString(Boolean value)
#else
    MtBooleanToString(value)
    Boolean value;
#endif
{
    if (value) {
        return ("True");
    }
    else {
        return ("False");
    }
}
				
String
#ifndef _NO_PROTO
MtCreateName(String name, String type)
#else
    MtCreateName(name, type)
    String name, type;
#endif
{
    static char buf[100];
	
    sprintf(buf, "%s_%s", name, type);
    return (buf);
}

/*
 * Set button label if not set in resource file
 */
void
#ifndef _NO_PROTO
MtSetLabel(Widget w, String label)
#else
    MtSetLabel(w, label)
    Widget	w;
    String	label;
#endif
{
    String		string;
    XmString	label_string = NULL;

    XtVaGetValues(w, XmNlabelString, &label_string, NULL);
    if (!label_string) {
        return;
    }
    string = MtCvtXmStringToString(label_string);
    if (strcmp(string, XtName(w)) == 0) {
        XmString new_label = MtCreateXmString(label);
        XtVaSetValues(w, XmNlabelString, new_label, NULL);
        XmStringFree(new_label);
    }
    XtFree((XtPointer)string);
    XmStringFree(label_string);
}

/*	Function Name: MtFindChildren
 *	Description: Retuns all children (popup, normal and otherwise)
 *                   of this widget
 *	Arguments: parent - the parent widget.
 *                 children - the list of children.
 *                 normal - return normal children.
 *                 popup - return popup children.
 *	Returns: the number of children.
 */
int
#ifndef _NO_PROTO
MtFindChildren(Widget parent, Widget **children,
               Boolean normal, Boolean popup)
#else
    MtFindChildren(parent, children, normal, popup)
    Widget	parent, **children;
    Boolean	normal, popup;
#endif
{
    int					i, num_children, current;
    CompositeWidget		cw;
    
    num_children = current = 0;
    cw = (CompositeWidget) parent;
    if (XtIsWidget(parent) && popup) {
        num_children += parent->core.num_popups;
    }
	
    if (XtIsComposite(parent) && normal) {
        num_children += cw->composite.num_children;
    }

    if (num_children == 0) {	
        *children = NULL; 
        return(0);
    }

    *children = (Widget *) XtMalloc((Cardinal) sizeof(Widget) * num_children);

    if (XtIsComposite(parent) && normal) {
        for (i = 0; i < cw->composite.num_children; i++,current++) {
            (*children)[current] = cw->composite.children[i];
        }
    }

    if (XtIsWidget(parent) && popup) {
        for (i = 0; i < parent->core.num_popups; i++, current++) {
            (*children)[current] = parent->core.popup_list[i];
        }
    }

    return (num_children);
}

/*
 * Get the top level
 */
Widget
#ifndef _NO_PROTO
MtGetTopLevel(Widget w)
#else
    MtGetTopLevel(w)
    Widget w;
#endif
{
    Widget parent = w;

    while (XtParent(parent)) {
        parent = XtParent(parent);
    }
    return (parent);
}


/*
 * Get the WM shell
 */
Widget
#ifndef _NO_PROTO
MtGetTopShell(Widget w)
#else
    MtGetTopShell(w)
    Widget w;
#endif
{
    while (w && !XtIsWMShell(w)) {
        w = XtParent(w);
    }
    return(w);
}

/*
 * Get shell of a widget
 */
Widget
#ifndef _NO_PROTO
MtGetShell(Widget w)
#else
    MtGetShell(w)
    Widget w;
#endif
{
    Widget parent = w;
    while (!XtIsVendorShell(parent) &&
           !XmIsDialogShell(parent) &&
           (w = XtParent(parent)))
    {
        parent = w;
    }
    return (parent);
}

/*
 * Get widget's sibling by name
 */
Widget
#ifndef _NO_PROTO
MtGetSibling(Widget w, String name)
#else
    MtGetSibling(w, name)
    Widget w;
    String name;
#endif
{
    return(MtGetChild(MtGetShell(w), name));
}

/*
 * Get widget's child or grandchild by name
 */
Widget
#ifndef _NO_PROTO
MtGetChild(Widget parent, String name)
#else
    MtGetChild(parent, name)
    Widget parent;
    String name;
#endif
{
    int		i, num_children;
    Widget	child, *children, widget;
	
    num_children = MtFindChildren(parent, &children, True, True);
    for (i = 0; children && i < num_children; i++) {
        if (!strcmp(XtName(children[i]), name)) {
            child = children[i];
            XTFREE(children);
            return (child);
        }
        else if ((widget = MtGetChild(children[i], name))) {
            XTFREE(children);
            return (widget);
        }
    }
    if (children) {
        XTFREE(children);
    }
    return (NULL);
}

/*
 * Get widget's child or grandchild by userdata
 */
Widget
#ifndef _NO_PROTO
GetChildByUserData(Widget parent, XtPointer userData)
#else
    GetChildByUserData(parent, userData)
    Widget		parent;
    XtPointer	userData;
#endif
{
    Cardinal	 i, num_children;
    XtPointer	 data;
    Widget		*children = NULL, widget;

    XtVaGetValues(parent,
                  XmNchildren,		&children,
                  XmNnumChildren,	&num_children,
                  NULL);
    for (i = 0; children && i < num_children; i++) {
        XtVaGetValues(children[i], XmNuserData, &data, NULL);
        if (data == userData) {
            return (children[i]);
        }
        else if ((widget = GetChildByUserData(children[i], userData))) {
            return (widget);
        }
    }			
    return (NULL);
}

/* Invoke OK button's activate callback */
static void
#ifndef _NO_PROTO
OKcallback(Widget w, XtPointer client_data, XmAnyCallbackStruct *cbs)
#else
    OKcallback(w, client_data, cbs)
    Widget				 w;
    XtPointer			 client_data;
    XmAnyCallbackStruct	*cbs;
#endif
{
    Widget	ok_button;
	
    ok_button = MtGetSibling(w, MtCreateName(OK,"button"));
    if (ok_button) {
        XtCallActionProc(ok_button, "ArmAndActivate", cbs->event, NULL, 0);
    }
}

static void
#ifndef _NO_PROTO
fieldRadioBoxCB(Widget w, XtPointer client_data)
#else
    fieldRadioBoxCB(w, client_data)
    Widget		w;
    XtPointer	client_data;
#endif
{
    int		i;
    char	name[100];
    Boolean	set;
    Widget	widget, parent = (Widget) client_data;

    XtVaGetValues(w, XmNset, &set, NULL);
    if (!set) {
        return;
    }

    /* Desensitize all other widgets in parent */
    for (i=1; (widget = GetChildByUserData(parent, (XtPointer) i)); i++) {
        strcpy(name, XtName(widget));
        name[strrchr(name, '_') - name] = '\0';
        MtSetSensitive(w, name, False);
    }
    strcpy(name, XtName(w));
    name[strrchr(name, '_') - name] = '\0';
    MtSetSensitive(w, name, True);
    if ((widget = MtGetSibling(w, MtCreateName(name, "field")))) {
        XmProcessTraversal(widget, XmTRAVERSE_CURRENT);
    }
}

void
#ifndef _NO_PROTO
MtCloseDialogCB(Widget w)
#else
    MtCloseDialogCB(w)
    Widget w;
#endif
{
    XtPopdown(MtGetShell(w));
}

static void
#ifndef _NO_PROTO
DestroyDialogCB(Widget w)
#else
    DestroyDialogCB(w)
    Widget w;
#endif
{
    XtDestroyWidget(MtGetShell(w));
}

void
#ifndef _NO_PROTO
MtCloseFileSelectorCB(Widget w)
#else
    MtCloseFileSelectorCB(w)
    Widget w;
#endif
{
    XtUnmanageChild(w);
}

Widget
#ifndef _NO_PROTO
MtCreateActionArea(Widget parent, ButtonItem *buttons)
#else
    MtCreateActionArea(parent, buttons)
    Widget		 parent;
    ButtonItem	*buttons;
#endif
{
    int		num_buttons = 0, i;
    Widget	action_area, button;

    while (buttons[num_buttons].name) {
        num_buttons++;
    }

    action_area = XtVaCreateWidget("action_area",
                                   xmFormWidgetClass, parent,
                                   XmNskipAdjust,	  True,
                                   XmNfractionBase,   TIGHTNESS*num_buttons - 1,
                                   NULL);

    for (i=0; i < num_buttons; i++) {
        button = XtVaCreateManagedWidget(
            MtCreateName(buttons[i].name, "button"),
            xmPushButtonWidgetClass,	action_area,
            XmNleftAttachment,			i ? XmATTACH_POSITION : XmATTACH_FORM,
            XmNleftPosition,			TIGHTNESS*i,
            XmNtopAttachment,			XmATTACH_FORM,
            XmNbottomAttachment,		XmATTACH_FORM,          
            XmNrightAttachment,
            i != num_buttons-1 ? XmATTACH_POSITION : XmATTACH_FORM,
            XmNrightPosition,			TIGHTNESS*i + TIGHTNESS - 1,
            XmNshowAsDefault,			i == 0 || buttons[i].set,
            XmNdefaultButtonShadowThickness,	1,
            NULL);

        /* Set label if not provided in resource file */
        MtSetLabel(button, buttons[i].label);

        if (buttons[i].callback) {
            XtAddCallback(button, XmNactivateCallback,
                          buttons[i].callback, buttons[i].data);
        }
        /* If "Cancel" button, set popdown callback if not supplied */
        else if (strcmp(buttons[i].name, CANCEL) == 0) {
            XtAddCallback(button, XmNactivateCallback,
                          (XtCallbackProc) MtCloseDialogCB, NULL);
        }
        if (XtClass(parent) == xmPanedWindowWidgetClass && i == 0)  {
            Dimension height, h;
            XtVaGetValues(action_area, XmNmarginHeight, &h, NULL);
            XtVaGetValues(button, XmNheight, &height, NULL);
            height += 2 * h;
            XtVaSetValues(action_area,
                          XmNdefaultButton,		button,
                          XmNpaneMaximum,		height,
                          XmNpaneMinimum,		height,
                          NULL);
        }
    }
    XtManageChild(action_area);
    return (action_area);
}

Boolean
#ifndef _NO_PROTO
MtGetValue(Widget widget, String name, int type, XtArgVal *value)
#else
    MtGetValue(widget, name, type, value)
    Widget		 widget;
    String		 name;
    int			 type;
    XtArgVal	*value;
#endif
{
    int		num_children, i;
    Widget	*children, parent;

    if (name) {
        parent = MtGetSibling(widget, name);
    }
    else {
        parent = widget;
    }

    if (!parent) {
        return (False);
    }

    if (MtGetWidgetValue(parent, type, value)) {
        return (True);
    }

    num_children = MtFindChildren(parent, &children, TRUE, TRUE);
    for (i=0; children && i < num_children; i++) {
        if (MtGetWidgetValue(children[i], type, value)) {
            XTFREE(children);
            return (True);
        }
        else if (MtGetValue(children[i], NULL, type, value)) {
            XTFREE(children);
            return (True);
        }
    }			
    if (children) {
        XTFREE(children);
    }
    return (False);
}

void
#ifndef _NO_PROTO
MtSetSensitive(Widget w, String name, Boolean value)
#else
    MtSetSensitive(w, name, value)
    Widget	w;
    String	name;
    Boolean	value;
#endif
{
    Widget widget;
	
    if ((widget = MtGetSibling(w, MtCreateName(name, "label")))) {
        XtSetSensitive(widget, value);
    }

    if ((widget = MtGetSibling(w, MtCreateName(name, "field")))) {
        XtSetSensitive(widget, value);
    }

    if ((widget = MtGetSibling(w, MtCreateName(name, "button")))) {
        XmToggleButtonSetState(widget, value, False);
    }
}

Boolean
#ifndef _NO_PROTO
MtSetValue(Widget widget, String name, int type, XtArgVal value)
#else
    MtSetValue(widget, name, type, value)
    Widget		widget;
    String		name;
    int			type;
    XtArgVal	value;
#endif
{
    return (MtSetWidgetValue(MtGetSibling(widget, name), type, value));
}

Boolean
#ifndef _NO_PROTO
MtGetWidgetValue(Widget widget, int datatype, XtArgVal *value)
#else
    MtGetWidgetValue(widget, datatype, value)
    Widget 		 widget;
    int			 datatype;
    XtArgVal	*value;
#endif
{
    short			dec;
    int 			i, iValue;
    float			div, fValue;
    String			string;
    XmString		xm_string;
    WidgetClass		widget_class;
    XtPointer		user_data;
    Boolean			status, sensitive;

    status = True;
    /* Get value according to the widget class */
    if (!widget || !XtIsWidget(widget)) {
        return (False);
    }
    XtVaGetValues(widget, XmNsensitive, &sensitive, NULL);
    if (!sensitive) {
        return (False);
    }

    widget_class = XtClass(widget);
    if (widget_class == xmTextWidgetClass || widget_class == xmTextFieldWidgetClass) {
        string = XmTextGetString(widget);
        switch (datatype) {
        case TOOL_INT:
            status = (sscanf(string, "%d", &iValue) == 1);
            XtFree((XtPointer)string);
            if (status) {
                *value = (XtArgVal) iValue;
            }
            break;
        case TOOL_FLOAT:
            status = (sscanf(string, "%f", &fValue) == 1);
            XtFree((XtPointer)string);
            if (status)	{
                assert(sizeof(XtArgVal) >= sizeof(float));
                memcpy((char *) value, (char *)&fValue, sizeof(fValue));
                /* THIS IS WRONG
                 *value = (XtArgVal) fValue;
                 */
            }
            break;
        default:
            *value = (XtArgVal) string;
            status = True;
            break;
        }
        return (status);
    }
    else if (widget_class == xmLabelWidgetClass) {
        XtVaGetValues(widget,
                      XmNlabelString, &xm_string,
                      NULL);
        string = MtCvtXmStringToString(xm_string);
        XmStringFree(xm_string);
        switch (datatype) {
        case TOOL_INT:
            status = (sscanf(string, "%d", &iValue) == 1);
            XtFree((XtPointer)string);
            if (status) {
                *value = (XtArgVal) iValue;
            }
            break;
        case TOOL_FLOAT:
            status = (sscanf(string, "%f", &fValue) == 1);
            XtFree((XtPointer)string);
            if (status)	{
                assert(sizeof(XtArgVal) >= sizeof(float));
                memcpy((char *) value, (char *)&fValue, sizeof(fValue));
                /* THIS IS WRONG
                 *value = (XtArgVal) fValue;
                 */
            }
            break;
        default:
            *value = (XtArgVal) string;
            status = True;
            break;
        }
        return (status);
    }
    else if (widget_class == xmRowColumnWidgetClass) {
        Widget	button;
		
        XtVaGetValues(widget, XmNmenuHistory, &button, NULL);
        if (!button) {
            return (False);
        }
        XtVaGetValues(button, XmNuserData, &user_data, NULL);
        if (user_data || datatype != TOOL_STRING) {
            *value = (XtArgVal) user_data;
        }
        else {
            *value = (XtArgVal) XtNewString(XtName(button));
        }
        return(True);
    }
    else if (widget_class == xmScaleWidgetClass) {
        XmScaleGetValue(widget, &iValue);
        switch(datatype)	{
        case TOOL_INT:
            *value = (XtArgVal) iValue;
            break;
        case TOOL_FLOAT:
            assert(sizeof(XtArgVal) >= sizeof(float));
            fValue = iValue;
            XtVaGetValues(widget, XmNdecimalPoints, &dec, NULL);
            div = 1;
            for (i = 0; i < dec; i++) {
                div *= 10;
            }
            fValue /= div;
            memcpy((char *) value, (char *)&fValue, sizeof(fValue));
            break;
        }
        return(True);
    }
    else if (widget_class == xmListWidgetClass) {
        int			 count;
        XmString	*items;
		
        XtVaGetValues (widget,
                       XmNselectedItemCount, &count,
                       XmNselectedItems, &items,
                       NULL);
        if (count == 0) {
            return (False);
        }
        if (!XmStringGetLtoR(items[0], XmSTRING_DEFAULT_CHARSET, &string)) {
            return (False);
        }
        *value = (XtArgVal) string;
    }

    else if (widget_class == xmToggleButtonWidgetClass) {
        if (!XmToggleButtonGetState(widget)) {
            return (False);
        }
        if (datatype == TOOL_BOOLEAN) {
            *value = (XtArgVal) True;
        }
        else {
            XtVaGetValues(widget, XmNuserData, &user_data, NULL);
            *value = (XtArgVal) user_data;
        }
        return(True);
    }

    /* Invalid class */
    else {
        return (False);
    }

    status = True;

    switch (datatype) {
    case TOOL_INT:
        status = (sscanf((char *) value, "%d", &iValue) == 1);
        XTFREE(value);
        if (status) {
            *value = (XtArgVal) iValue;
        }
        break;
    case TOOL_FLOAT:
        status = (sscanf((char *) value, "%f", &fValue) == 1);
        XTFREE(value);
        if (status)	{
            assert(sizeof(XtArgVal) >= sizeof(float));
            memcpy((char *) value, (char *)&fValue, sizeof(fValue));
            /* THIS IS WRONG
             *value = (XtArgVal) fValue;
             */
        }
        break;
    default:
        break;
    }
    return (status);
}

/*
 * Set widget value
 */
Boolean
#ifndef _NO_PROTO
MtSetWidgetValue(Widget widget, int datatype, XtArgVal value)
#else
    MtSetWidgetValue(widget, datatype, value)
    Widget widget;
    int datatype;
    XtArgVal value;
#endif
{
    XmString xm_string;
    WidgetClass widget_class;
    int i;
    short dec;
    float div;
    int iValue;

    /* Set value according to the widget type */
    if (!widget ||!XtIsWidget(widget))
        return (False);

#define CONVERT \
    if (datatype == TOOL_INT) \
                                  value = (XtArgVal) MtIntToString((int) value); \
                                                                                     else if (datatype == TOOL_FLOAT) \
                                                                                                                          value = (XtArgVal) MtFloatToString(*((float *) value)); \
                                                                                                                                                                                      else if (datatype == TOOL_BOOLEAN) \
                                                                                                                                                                                                                             value = (XtArgVal) MtBooleanToString((Boolean) value); \

                                                                                                                                                                                                                                                                                        widget_class = XtClass(widget);

                                                                                                                                                                                                                             if (widget_class == xmListWidgetClass) {
                                                                                                                                                                                                                                 CONVERT;
                                                                                                                                                                                                                                 if (strlen((char *)value) == 0) 
                                                                                                                                                                                                                                     XmListDeleteAllItems(widget);
                                                                                                                                                                                                                                 else {
                                                                                                                                                                                                                                     xm_string = XmStringCreateSimple((char *) value);
                                                                                                                                                                                                                                     XmListAddItem(widget, xm_string, 0);
                                                                                                                                                                                                                                     XmStringFree(xm_string);
                                                                                                                                                                                                                                 }
                                                                                                                                                                                                                             }

                                                                                                                                                                                                                             else if (widget_class == xmTextWidgetClass || widget_class == xmTextFieldWidgetClass) {
                                                                                                                                                                                                                                 CONVERT;
                                                                                                                                                                                                                                 XmTextSetString (widget, (char *) value);
                                                                                                                                                                                                                                 XmTextShowPosition (widget, 0);
                                                                                                                                                                                                                             }

                                                                                                                                                                                                                             else if (widget_class == xmRowColumnWidgetClass) {
                                                                                                                                                                                                                                 Boolean isRadio;
                                                                                                                                                                                                                                 Widget button, cascade;
                                                                                                                                                                                                                                 if (datatype == TOOL_INT || datatype == TOOL_BOOLEAN) {
                                                                                                                                                                                                                                     button = GetChildByUserData(widget, (XtPointer) value);
                                                                                                                                                                                                                                 }
                                                                                                                                                                                                                                 else {
                                                                                                                                                                                                                                     CONVERT;
                                                                                                                                                                                                                                     button = MtGetChild(widget, (char *)value);
                                                                                                                                                                                                                                 }
                                                                                                                                                                                                                                 if (!button) 
                                                                                                                                                                                                                                     return (False);
                                                                                                                                                                                                                                 if (XtClass(button) == xmToggleButtonWidgetClass) {
                                                                                                                                                                                                                                     XtVaGetValues(XtParent(widget),
                                                                                                                                                                                                                                                   XmNradioBehavior, &isRadio,
                                                                                                                                                                                                                                                   NULL);
                                                                                                                                                                                                                                     if (isRadio) {
                                                                                                                                                                                                                                         XtVaSetValues(button, 
                                                                                                                                                                                                                                                       XmNset,			True,
                                                                                                                                                                                                                                                       XmNmenuHistory,	button,
                                                                                                                                                                                                                                                       NULL);
                                                                                                                                                                                                                                     }
                                                                                                                                                                                                                                     else {
                                                                                                                                                                                                                                         XmToggleButtonSetState(button, True, True);
                                                                                                                                                                                                                                     }
                                                                                                                                                                                                                                 }
                                                                                                                                                                                                                                 else {
                                                                                                                                                                                                                                     /* Set the menu history resource of the rc to the widget */
                                                                                                                                                                                                                                     XtVaGetValues(widget, XmNuserData, &cascade, NULL);
                                                                                                                                                                                                                                     if (cascade)
                                                                                                                                                                                                                                         XtVaSetValues(cascade, XmNmenuHistory, button, NULL);
                                                                                                                                                                                                                                 }
                                                                                                                                                                                                                             }
	
                                                                                                                                                                                                                             else if (widget_class == xmScaleWidgetClass) {
                                                                                                                                                                                                                                 if (datatype == TOOL_STRING)	{
                                                                                                                                                                                                                                     if (sscanf((char *)value, "%d", &iValue) != 1)	{
                                                                                                                                                                                                                                         return (False);
                                                                                                                                                                                                                                     }
                                                                                                                                                                                                                                 }
                                                                                                                                                                                                                                 else if (datatype == TOOL_FLOAT)	{
                                                                                                                                                                                                                                     iValue = (int) *((float *) value);
                                                                                                                                                                                                                                     XtVaGetValues(widget, XmNdecimalPoints, &dec, NULL);
                                                                                                                                                                                                                                     div = 1;
                                                                                                                                                                                                                                     for (i=0; i<dec; i++)	{
                                                                                                                                                                                                                                         div *= 10;
                                                                                                                                                                                                                                     }
                                                                                                                                                                                                                                     iValue *= div;
                                                                                                                                                                                                                                 }
                                                                                                                                                                                                                                 XmScaleSetValue(widget, iValue);
                                                                                                                                                                                                                             }
	
                                                                                                                                                                                                                             else if (widget_class == xmToggleButtonWidgetClass) {
                                                                                                                                                                                                                                 XmToggleButtonSetState(widget, (Boolean) value, True);
                                                                                                                                                                                                                                 if ((Boolean) value
                                                                                                                                                                                                                                     && XtClass(XtParent(widget)) == xmRowColumnWidgetClass)
                                                                                                                                                                                                                                     XtVaSetValues(XtParent(widget), XmNmenuHistory, widget, NULL);
                                                                                                                                                                                                                             }

                                                                                                                                                                                                                             else if (widget_class == xmLabelWidgetClass) {
                                                                                                                                                                                                                                 CONVERT;
                                                                                                                                                                                                                                 xm_string = XmStringCreateSimple((char *) value);
                                                                                                                                                                                                                                 XtVaSetValues(widget, XmNlabelString, xm_string, NULL);
                                                                                                                                                                                                                                 XmStringFree(xm_string);
                                                                                                                                                                                                                             }

                                                                                                                                                                                                                             /* Invalid class */
                                                                                                                                                                                                                             else {
                                                                                                                                                                                                                                 return (False);
                                                                                                                                                                                                                             }

                                                                                                                                                                                                                             return (True);
#undef CONVERT
}

static int
#ifndef _NO_PROTO
counter_verify_text_value(Counter *counter, int value)
#else
    counter_verify_text_value(counter, value)
    Counter	*counter;
    int		value;
#endif
{
    char	new_val[32];
	
    if (value >= counter->max) {
        value = counter->max;
    }

    if (value <= counter->min) {
        value = counter->min;
    }

    sprintf(new_val, "%d", value);
    XtVaSetValues(counter->text, XmNvalue, new_val, NULL);
    return(value);
}

static void
#ifndef _NO_PROTO
counter_activate_cb(Widget w, Counter *counter, XtPointer call_data)
#else
    counter_activate_cb(w, counter, call_data)
    Widget		w;
    Counter		*counter;
    XtPointer	call_data;
#endif
{
    String	s;
    int		value;

    XtVaGetValues(counter->text, XmNvalue, &s, NULL);
    value = atoi(s);
    value = counter_verify_text_value(counter, value);
    if (counter->callback) {
        counter->callback(value);
    }

    XtFree((XtPointer)s);
}

static void
#ifndef _NO_PROTO
counter_up_cb(Widget w, Counter *counter, XtPointer call_data)
#else
    counter_up_cb(w, counter, call_data)
    Widget		w;
    Counter		*counter;
    XtPointer	call_data;
#endif
{
    int		value;
    char	*cur_val;

    XtVaGetValues(counter->text, XmNvalue, &cur_val, NULL);
    value = atoi(cur_val) + 1;
    value = counter_verify_text_value(counter, value);
    if (counter->callback) {
        counter->callback(value);
    }
    XtFree((XtPointer)cur_val);
}

static void
#ifndef _NO_PROTO
counter_down_cb(Widget w, Counter *counter, XtPointer call_data)
#else
    counter_down_cb(w, counter, call_data)
    Widget		w;
    Counter		*counter;
    XtPointer	call_data;
#endif
{
    int			value;
    char		*cur_val;

    XtVaGetValues(counter->text, XmNvalue, &cur_val, NULL);
    value = atoi(cur_val) - 1;
    value = counter_verify_text_value(counter, value);
    if (counter->callback) {
        counter->callback(value);
    }

    XtFree((XtPointer)cur_val);
}

/*
 * Build popup, option and pulldown menus, depending on menu_type:
 * XmMENU_PULLDOWN: returns CascasdeButton
 * XmMENU_OPTION: returns managing RowColumn, unmanaged
 * XmMENU_POPUP: returns menu
 */
Widget
#ifndef _NO_PROTO
MtBuildMenu(Widget parent, int menu_type, String name, String title,
            MenuItem *items)
#else
    MtBuildMenu(parent, menu_type, name, title, items)
    Widget 		parent;
    int			menu_type;
    char 		*name, *title;
    MenuItem 	*items;
#endif
{
    Widget		menu, cascade = NULL, widget;
    int			i;
    Boolean		had_toggle = False;
    XmString	string;
    Arg 		args[2];

    if (menu_type == XmMENU_PULLDOWN) {
        menu = XmCreatePulldownMenu(parent, MtCreateName(name, "menu"),NULL, 0);
#if XmVersion >= 1002
        XtVaSetValues(menu, XmNtearOffModel, XmTEAR_OFF_ENABLED, NULL);
#endif
    }
    else if (menu_type == XmMENU_OPTION) {
        menu = XmCreatePulldownMenu(parent,MtCreateName(name, "menu"),NULL,0);
    }
    else if (menu_type == XmMENU_POPUP) {
        menu = XmCreatePopupMenu(parent, MtCreateName(name, "menu"), NULL, 0);
    }

    string = MtCreateXmString(title);
    XtSetArg(args[0], XmNsubMenuId, menu);
    XtSetArg(args[1], XmNlabelString, string);
    if (menu_type == XmMENU_PULLDOWN) {
        cascade = XtCreateManagedWidget(MtCreateName(name, "cascade"),
                                        xmCascadeButtonWidgetClass,	parent, args, 2);
    }
    else if (menu_type == XmMENU_OPTION) {
        cascade = XmCreateOptionMenu(parent, MtCreateName(name, "cascade"),
                                     args, 2);
    }
    XmStringFree(string);

    /* Set menu's userdata to cascade, for use in MtSetWidgetValue() */
    XtVaSetValues(menu, XmNuserData, cascade, NULL);

    for (i=0; items[i].name; i++) {
        if (items[i].subitems) {
            if (menu_type != XmMENU_OPTION) {
                widget = MtBuildMenu(menu, XmMENU_PULLDOWN, items[i].name,
                                     items[i].label, items[i].subitems);
            }
            else {
                XtWarning("Option menu items can't have submenus");
                continue;
            }
        }
        else {
            WidgetClass widget_class = items[i].w_class ?
                *items[i].w_class : xmPushButtonWidgetClass;
            widget = XtVaCreateManagedWidget(
                MtCreateName(items[i].name, "button"),
                widget_class, 				menu,
                XmNuserData,		items[i].data,
                NULL);
            if (widget_class == xmToggleButtonWidgetClass)	{
                XtVaSetValues(widget, 
                              XmNindicatorType, 		
                              items[i].toggle_button_type == XmRADIOBUTTON
                              ? XmONE_OF_MANY : XmN_OF_MANY, 
                              XmNvisibleWhenOff, 		True, 
                              NULL);
                if (items[i].toggle_button_type == XmRADIOBUTTON)	{
                    XmToggleButtonSetState(widget, i==0, False);
                    had_toggle = True;
                }
            }
            if (items[i].label)	{
                MtSetLabel(widget, items[i].label);
            }
        }
        if (items[i].callback) {
            XtAddCallback(widget,
                          items[i].w_class == &xmToggleButtonWidgetClass
                          ? XmNvalueChangedCallback : XmNactivateCallback,
                          items[i].callback, items[i].data);
        }
    }

    if (had_toggle)	{
        XtVaSetValues(menu,
                      XmNradioBehavior,		had_toggle,
                      XmNradioAlwaysOne,    True,
                      NULL);
    }

    if (menu_type == XmMENU_POPUP) {
        return (menu);
    }
    else {
        XtManageChild(cascade);
        return (cascade);
    }
}

/*
 * Build radio box
 */
Widget
#ifndef _NO_PROTO
MtBuildRadioBox(Widget parent, String name, String title, ButtonItem
                *buttons, int orientation, int button_type)
#else
    MtBuildRadioBox(parent, name, title, buttons, orientation, button_type)
    Widget 		parent;
    char 		*name, *title;
    ButtonItem 	*buttons;
    int			orientation;	/* XmHORIZONTAL or XmVERTICAL */
    int			button_type;	/* XmCHECKBUTTON or XmRADIOBUTTON */
#endif
{
    Widget		menu, button, button0 = NULL, box, frame;
    int			i;
    Boolean		set = False;

    if (title)
        frame = MtCreateLabeledBox(parent, name, title, &box);
    else
        frame = box = parent;

    menu = XmCreateRadioBox(frame, name, NULL, 0);
    XtVaSetValues(menu,
                  XmNradioBehavior, button_type == XmRADIOBUTTON,
                  XmNorientation,	orientation,
                  NULL);

    for (i=0; buttons[i].name; i++)	{
        button = XtVaCreateManagedWidget(
            MtCreateName(buttons[i].name, "button"),
            xmToggleButtonWidgetClass,	 	menu,
            XmNuserData,					buttons[i].data,
            XmNset,							buttons[i].set,	
            NULL);
        MtSetLabel(button, buttons[i].label);
        if (i == 0)
            button0 = button;
        if (buttons[i].set) {
            XtVaSetValues(menu, XmNmenuHistory, button, NULL);
            if (!set)
                set = True;
        }
        if (buttons[i].callback) {
            XtAddCallback(button, XmNvalueChangedCallback,
                          buttons[i].callback, buttons[i].data);
        }
    }

    /* If no button was set, set 1st */
    if (!set && button0) {
        XtVaSetValues(button0, XmNset, True, NULL);
        XtVaSetValues(menu, XmNmenuHistory, button0, NULL);
    }

    XtManageChild(menu);
    return (box);
}


/*
 * Create a labeled box to contain a group of controls
 */
Widget
#ifndef _NO_PROTO
MtCreateLabeledBox(Widget parent, String name, String label, Widget *box)
#else
    MtCreateLabeledBox(parent, name, label, box)
    Widget	parent, *box;
    char	*name, *label;
#endif
{
    Widget label_w, container;

    if (!label) {
        *box = container = parent;
        goto exit;
    }
    *box = XtVaCreateManagedWidget(MtCreateName(name, "box"),
                                   xmFrameWidgetClass,		parent,
                                   NULL);
    label_w = XtVaCreateManagedWidget(MtCreateName(name, "label"),
                                      xmLabelWidgetClass,		*box,
                                      XmNchildType,              XmFRAME_TITLE_CHILD,
                                      NULL);
    MtSetLabel(label_w, label);
    container = XtVaCreateManagedWidget("containedRC",
                                        xmRowColumnWidgetClass, 	*box,
                                        XmNorientation,				XmVERTICAL,
                                        XmNnumColumns,				1,
                                        NULL);
exit:
    return (container);
}

/*
 * Create a labeled container to contain a group of controls
 */
Widget
#ifndef _NO_PROTO
MtCreateLabeledContainer(Widget parent, String name, String label,
                         WidgetClass containerWidgetClass, Widget *frame)
#else
    MtCreateLabeledContainer(parent, name, label, containerWidgetClass, frame)
    Widget	parent, *frame;
    char	*name, *label;
    WidgetClass containerWidgetClass;
#endif
{
    Widget frame_w, label_w, container;

    if (!label) {
        frame_w = container = parent;
        goto exit;
    }

    frame_w = XtVaCreateManagedWidget(MtCreateName(name, "frame"),
                                      xmFrameWidgetClass,		parent,
                                      NULL);
    label_w = XtVaCreateManagedWidget(MtCreateName(name, "label"),
                                      xmLabelWidgetClass,		 frame_w,
                                      XmNchildType,              XmFRAME_TITLE_CHILD,
                                      NULL);
    MtSetLabel(label_w, label);
    container = XtVaCreateManagedWidget(MtCreateName(name, "container"),
                                        containerWidgetClass, frame_w,
                                        NULL);
exit:
    if (frame)
        *frame = frame_w;
    return (container);
}

/*
 * Create dialog shell with optional menubar and action area
 */
Widget
MtCreateShell(Widget parent, String name, String title,
              WorkAreaFunc work_area_func, MenuItem *menu_items,
              ButtonItem *action_area_items, Boolean display_msg_area) 
{
    int					i;
    Widget				base, mainWindow, menubar, form, separator;
    Widget				message, action_area, workArea, frame, button;
    WidgetClass			widget_class;
    Atom				WM_DELETE_WINDOW;
    static XmString		empty_label = NULL;

    if (name) {
        base = XtVaCreatePopupShell(MtCreateName(name, "shell"),
                                    vendorShellWidgetClass, MtGetShell(parent),
                                    XmNtitle,				title,
                                    XmNtraversalOn,			True,
                                    XmNdeleteResponse, 		XmDO_NOTHING,
                                    NULL);
    }
    else {
        base = parent;
    }

    WM_DELETE_WINDOW = XmInternAtom(XtDisplay(base), "WM_DELETE_WINDOW",False);
    XmAddWMProtocolCallback(base, WM_DELETE_WINDOW,
                            (XtCallbackProc) MtCloseDialogCB, (XtPointer) 0);

    /* Use a Motif XmMainWindow widget to handle window layout */
    mainWindow = XtCreateManagedWidget("mainWindow", 
                                       xmMainWindowWidgetClass,	base, 
                                       NULL, 0 );
    
    /* Create menu bar if specified */
    if (menu_items) {
        menubar = XmCreateMenuBar(mainWindow, "menubar", NULL, 0);
        for (i = 0; menu_items[i].name; i++) {
            if (menu_items[i].subitems != NULL) {
                button = MtBuildMenu(menubar, XmMENU_PULLDOWN,
                                     menu_items[i].name, menu_items[i].label,
                                     menu_items[i].subitems);
            }
            else {
                widget_class = (menu_items[i].w_class
                         ? *menu_items[i].w_class
                         : xmCascadeButtonWidgetClass);
                button =
                    XtVaCreateManagedWidget(MtCreateName(menu_items[i].name,
                                                         "button"),
                                            widget_class, 			menubar,
                                            XmNuserData,	menu_items[i].data,
                                            NULL);
                MtSetLabel(button, menu_items[i].label);
                if (menu_items[i].callback) {
                    XtAddCallback(button,
                                  ((menu_items[i].w_class ==
                                    &xmPushButtonWidgetClass)
                                   ? XmNvalueChangedCallback
                                   : XmNactivateCallback),
                                  menu_items[i].callback, menu_items[i].data);
                }
            }
            /* If "HELP" menu, set menubar's resource accordingly */
            if (strcmp(menu_items[i].name, HELP) == 0){
                XtVaSetValues(menubar, XmNmenuHelpWidget, button, NULL);
            }
        }
        XtManageChild(menubar);
    }

    form = XtVaCreateWidget("form", xmFormWidgetClass, mainWindow, NULL);

    /* Create button box first, so that work area widgets can access it */
    if (action_area_items){
        action_area = MtCreateActionArea(form, action_area_items);
    }

    /* Call supplied routine to create the work area */
    workArea = work_area_func(form);
    assert(workArea);

    separator = XtVaCreateManagedWidget("separator",
                                        xmSeparatorGadgetClass, form, 
                                        NULL );

    /* Create message area and attach to bottom of window */
    frame = XtCreateWidget("frame",
                           xmFrameWidgetClass, form, NULL, 0) ;

    if (!empty_label){
        empty_label = XmStringCreateSimple(" ");
    }
    message = XtVaCreateWidget("message_label",
                               xmLabelWidgetClass, 	frame, 
                               XmNalignment,		XmALIGNMENT_BEGINNING,
                               XmNlabelString,		empty_label,
                               NULL);
    if (display_msg_area) {
        XtManageChild(message);
    }
    XtManageChild(frame);

    XtVaSetValues(workArea,
                  XmNtopAttachment, 		XmATTACH_FORM,
                  XmNleftAttachment, 		XmATTACH_FORM,
                  XmNrightAttachment, 		XmATTACH_FORM,
                  XmNbottomAttachment, 		XmATTACH_WIDGET,
                  XmNbottomWidget, 			separator,
                  NULL);

    XtVaSetValues(separator,
                  XmNleftAttachment, 		XmATTACH_FORM,
                  XmNrightAttachment, 		XmATTACH_FORM,
                  NULL);

    if (action_area_items) {
        XtVaSetValues(separator,
                      XmNbottomAttachment, 		XmATTACH_WIDGET,
                      XmNbottomWidget, 			action_area,
                      NULL);

        XtVaSetValues(action_area,
                      XmNleftAttachment, 		XmATTACH_FORM,
                      XmNrightAttachment, 		XmATTACH_FORM,
                      XmNbottomAttachment, 		XmATTACH_WIDGET,
                      XmNbottomWidget, 			frame,
                      NULL);
    }
    else {
        XtVaSetValues(separator,
                      XmNbottomAttachment, 	XmATTACH_FORM,
                      NULL);
    }
    XtVaSetValues(frame,
                  XmNshadowType,		XmSHADOW_IN,
                  XmNleftAttachment, 	XmATTACH_FORM,
                  XmNrightAttachment, 	XmATTACH_FORM,
                  XmNbottomAttachment, 	XmATTACH_FORM,
                  NULL);
    
    /* Manage the work area if necessary */
    if (!XtIsManaged(workArea)){
        XtManageChild(workArea);
    }

    XtManageChild(form);
    return (base);
}

/*
 *  Create a text field with name, label, callback, data, length, value
 *  and sensitivity defined by the fielditem structure.
*/
Widget
#ifndef _NO_PROTO
MtCreateTextField(Widget parent, FieldItem *fielditem)
#else
    MtCreateTextField(parent, fielditem)
    Widget		parent;
    FieldItem	*fielditem;
#endif
{
    Widget field, label_w, container_rc;

    container_rc = XtVaCreateManagedWidget(MtCreateName(fielditem->name, 
                                                        "container"),
                                           xmRowColumnWidgetClass,		parent,
                                           XmNorientation,				XmHORIZONTAL,
                                           XmNnumColumns,				2,
                                           XmNisAligned,				TRUE,
                                           XmNentryAlignment,			XmALIGNMENT_END,
                                           NULL);
    label_w = XtVaCreateManagedWidget(MtCreateName(fielditem->name, "label"),
                                      xmLabelWidgetClass, container_rc,
                                      XmNalignment,		XmALIGNMENT_END,
                                      NULL);

    MtSetLabel(label_w, fielditem->label);
    field = XtVaCreateManagedWidget(MtCreateName(fielditem->name, "field"),
                                    xmTextWidgetClass, 	container_rc,
                                    XmNcolumns,					fielditem->length,
                                    XmNsensitive,				fielditem->sensitive,
                                    NULL);
    if (fielditem->value)
        XtVaSetValues(field, XmNvalue, fielditem->value, NULL);
    if (fielditem->callback) 
        XtAddCallback(field, XmNactivateCallback,
                      fielditem->callback, fielditem->data);
    else 
        XtAddCallback(field, XmNactivateCallback,
                      (XtCallbackProc) OKcallback, NULL); 

    return(container_rc);
}

Widget
#ifndef _NO_PROTO
MtCreateSliderField(Widget parent, SliderItem *slideritem)
#else
    MtCreateSliderField(parent, slideritem)
    Widget		parent;
    SliderItem	*slideritem;
#endif
{
    /*
     *  Create a text field with name, label, callback, data, length, value
     *  and sensitivity defined by the fielditem structure.
	*/
    Widget	slider, label_w, container_rc;

    container_rc = XtVaCreateManagedWidget(MtCreateName(slideritem->name, 
                                                        "container"),
                                           xmRowColumnWidgetClass,		parent,
                                           XmNorientation,				XmHORIZONTAL,
                                           XmNnumColumns,				2,
                                           XmNisAligned,				TRUE,
                                           XmNentryAlignment,			XmALIGNMENT_END,
                                           NULL);
    label_w = XtVaCreateManagedWidget(MtCreateName(slideritem->name, "label"),
                                      xmLabelWidgetClass, container_rc,
                                      XmNalignment,		XmALIGNMENT_END,
                                      NULL);

    MtSetLabel(label_w, slideritem->label);
    slider = XtVaCreateManagedWidget(MtCreateName(slideritem->name, "slider"),
                                     xmScaleWidgetClass, 		container_rc,
                                     XmNorientation,				XmHORIZONTAL,
                                     XmNminimum,					slideritem->min,
                                     XmNmaximum,					slideritem->max,
                                     XmNvalue,					slideritem->value,
                                     NULL);
    if (slideritem->drag_callback) {
        XtAddCallback(slider, XmNdragCallback,
                      slideritem->drag_callback, slideritem->data);
    }
    if (slideritem->value_callback) {
        XtAddCallback(slider, XmNvalueChangedCallback,
                      slideritem->value_callback, slideritem->data);
    }

    return(container_rc);
}


/*
 *	This is the new and improved version of MtCreateCounter.
 *	Now you have access to the data that MtCreateCounter and
 *	its sibling functions use. The old MtCreateCounter will
 *	remain now, for both posterity, and backwards compatibility
*/
CounterItem *
#ifndef _NO_PROTO
MtCreateCounter(Widget parent, FieldItem *fielditem, int min, int max)
#else
    MtCreateCounter(parent, fielditem, min, max)
    Widget		parent;
    FieldItem	*fielditem;
    int			min, max;
#endif
{
    Widget		container, rc, text, up_arrow, down_arrow;
    Counter		*counter;
    CounterItem *new_counteritem;
    FieldItem	new_fi;
    counter = (Counter *) XtMalloc(sizeof(Counter));

    /* Don't use fielditem callback for the text field as it is
     * also used for the counter_up_cb, counter_down_cb, and
     * counter_activate_cb which requires the callback to have one
     * integer argument.
     */

    new_fi = *fielditem;
    new_fi.callback = NULL;
    container = MtCreateTextField(parent, &new_fi);
	
    text = MtGetChild(parent, MtCreateName(fielditem->name, "field"));
    XtAddCallback(text, XmNlosePrimaryCallback,
                  (XtCallbackProc) counter_activate_cb, counter); 
    XtAddCallback(text, XmNactivateCallback,
                  (XtCallbackProc) counter_activate_cb, counter);
    XtAddCallback(text, XmNlosingFocusCallback,
                  (XtCallbackProc) counter_activate_cb, counter);

    rc = XtVaCreateManagedWidget(MtCreateName(fielditem->name, "arrowrc"),
                                 xmRowColumnWidgetClass, container,
                                 XmNmarginHeight,	0,
                                 XmNspacing,			0,
                                 NULL);

    up_arrow = XtVaCreateManagedWidget(MtCreateName(fielditem->name, "up"),
                                       xmArrowButtonWidgetClass, rc,
                                       XmNarrowDirection,		XmARROW_UP,
                                       XmNshadowThickness,		0,
                                       XmNhighlightThickness,	0,
                                       NULL);
    XtAddCallback(up_arrow, XmNactivateCallback,
                  (XtCallbackProc) counter_up_cb, counter);

    down_arrow = XtVaCreateManagedWidget(MtCreateName(fielditem->name, "down"),
                                         xmArrowButtonWidgetClass, rc,
                                         XmNarrowDirection,		XmARROW_DOWN,
                                         XmNshadowThickness,		0,
                                         XmNhighlightThickness,	0,
                                         NULL);
    XtAddCallback(down_arrow, XmNactivateCallback,
                  (XtCallbackProc) counter_down_cb, counter);

    new_counteritem = (CounterItem *) XtMalloc(sizeof(CounterItem));

    new_counteritem->container = counter->container = container;
    new_counteritem->text = counter->text = text;
    new_counteritem->up_arrow = counter->up_arrow = up_arrow;
    new_counteritem->down_arrow = counter->down_arrow = down_arrow;
    counter->min = min;
    new_counteritem->min = &counter->min;
    counter->max = max;
    new_counteritem->max = &counter->max;
    counter->callback = fielditem->callback;
    return(new_counteritem);
}

Widget 
#ifndef _NO_PROTO
MtCreateFieldBox(Widget parent, String name, String title, FieldItem *fields,
                 int orientation)
#else
    MtCreateFieldBox(parent, name, title, fields, orientation)
    Widget parent;
    String name, title;
    FieldItem *fields;
    int orientation;
#endif
{
    Widget label, rc, container_rc, frame, box;
    Dimension max, width, i;

    if (title) 
        frame = MtCreateLabeledBox(parent, name, title, &box);
    else
        frame = parent;

    rc = XtVaCreateManagedWidget(MtCreateName(name, "box"),
                                 xmRowColumnWidgetClass, 	frame,
                                 XmNorientation,			orientation,
                                 XmNnumColumns,				1,
                                 NULL);
    if (!title)
        box = rc;
    if (XtClass(parent) == xmFormWidgetClass)
        XtVaSetValues(rc,
                      XmNleftAttachment,			XmATTACH_FORM,
                      XmNrightAttachment,		XmATTACH_FORM,
                      NULL);

    /* Add label and text field to rc */
    for (i=0; fields[i].name; i++) 
        container_rc = MtCreateTextField(rc, &fields[i]);

    /* Align labels by setting width to widest label */
    max = 0;
    for (i=0; fields[i].name; i++) {
        label = MtGetChild(rc, MtCreateName(fields[i].name, "label"));
        XtVaGetValues(label, XmNwidth, &width, NULL);
        max = width > max ? width : max;
    }
    for (i=0; fields[i].name; i++) {
        label = MtGetChild(rc, MtCreateName(fields[i].name, "label"));
        XtVaSetValues(label, 
                      XmNwidth,				max, 
                      XmNrecomputeSize,		False,
                      NULL);
    }
    return (box);
}

Widget 
#ifndef _NO_PROTO
MtCreateFieldRadioBox(Widget parent, String name, String title,
                      FieldItem *fields, int orientation)
#else
    MtCreateFieldRadioBox(parent, name, title, fields, orientation)
    Widget parent;
    String name, title;
    FieldItem *fields;
    int orientation;
#endif
{
    Widget field, label, button, rc, container_rc, frame, box;
    Dimension max, width, i;

    if (title)
        frame = MtCreateLabeledBox(parent, name, title, &box);
    else 
        frame = box = parent;

    rc = XtVaCreateManagedWidget(MtCreateName(name, "box"),
                                 xmRowColumnWidgetClass, 	frame,
                                 XmNorientation,			orientation,
                                 XmNnumColumns,				1,
                                 NULL);
    if (XtClass(parent) == xmFormWidgetClass)
        XtVaSetValues(rc,
                      XmNleftAttachment,			XmATTACH_FORM,
                      XmNrightAttachment,		XmATTACH_FORM,
                      NULL);

    /* Add label and text field to rc */
    for (i=0; fields[i].name; i++) {
        container_rc = XtVaCreateManagedWidget(fields[i].name,
                                               xmRowColumnWidgetClass,		rc,
                                               XmNorientation,				XmHORIZONTAL,
                                               XmNnumColumns,				2,
                                               XmNisAligned,				TRUE,
                                               XmNentryAlignment,			XmALIGNMENT_END,
                                               NULL);
        button = XtVaCreateManagedWidget(
            MtCreateName(fields[i].name, "button"),
            xmToggleButtonWidgetClass, container_rc,
            XmNalignment,		XmALIGNMENT_BEGINNING,
            XmNset,				fields[i].sensitive,
            XmNindicatorType,	XmONE_OF_MANY,
            XmNuserData,		i+1,
            NULL);
        MtSetLabel(button, fields[i].label);
        XtAddCallback(button, XmNvalueChangedCallback,
                      (XtCallbackProc) fieldRadioBoxCB, (XtPointer) rc);
        field = XtVaCreateManagedWidget(MtCreateName(fields[i].name, "field"),
					xmTextWidgetClass, 	container_rc,
					XmNcolumns,					Max(fields[i].length, 1),
					XmNsensitive,				fields[i].sensitive,
					NULL);
        if (fields[i].length == 0)
            XtVaSetValues(field, XmNmappedWhenManaged, False, NULL);
        if (fields[i].value)
            XtVaSetValues(field, XmNvalue, fields[i].value, NULL);
        if (fields[i].callback) 
            XtAddCallback(field, XmNactivateCallback,
                          (XtCallbackProc)  fields[i].callback,
                          fields[i].data);
        else 
            XtAddCallback(field, XmNactivateCallback,
                          (XtCallbackProc) OKcallback, NULL);
    }

    /* Align labels by setting width to widest label */
    max = 0;
    for (i=0; fields[i].name; i++) {
        label = MtGetChild(rc, MtCreateName(fields[i].name, "button"));
        XtVaGetValues(label, XmNwidth, &width, NULL);
        max = width > max ? width : max;
    }
    for (i=0; fields[i].name; i++) {
        label = MtGetChild(rc, MtCreateName(fields[i].name, "button"));
        XtVaSetValues(label, 
                      XmNwidth,				max, 
                      XmNrecomputeSize,		False,
                      NULL);
    }

    return (box);
}

/*
 * Create box of cascade box. If orientation is XmVERTICAL, buttons will be 
 * arranged in num_columns per row, if XmHORIZONTAL then num_columns per 
 * columns. If buttons[i].menu is null, a dummy widget will be inserted
 * so that you can have the following effect:
 *
 * label [BUTTON]     <dummy widgets>
 * label [BUTTON]      label [BUTTON]
 * label [BUTTON]      label [BUTTON]
 *
 * Where dummy widget is located, there will be no cascade button (the space
 * appears empty).
 */
Widget 
#ifndef _NO_PROTO
MtCreateCascadeButtonBox(Widget parent, String name, String title,
                         CascadeButtonItem *buttons, int orientation, int num_columns)
#else
    MtCreateCascadeButtonBox(parent, name, title, buttons, orientation, 
                             num_columns)
    Widget				 parent; 
    String				 name;
    String				 title;
    CascadeButtonItem	*buttons;
    int					 orientation;
    int					 num_columns;

#endif
{
    int			i;
    Dimension	max, width;
    XmString	xmstring;
    Widget		rc, column, frame, box, label;

    if (title){
        frame = MtCreateLabeledBox(parent, name, title, &box);
    }
    else {
        frame = box = parent;
    }

    rc = XtVaCreateManagedWidget(MtCreateName(name, "box"),
                                 xmRowColumnWidgetClass, 	frame,
                                 XmNorientation,			orientation,
                                 XmNnumColumns,				num_columns,
                                 XmNisAligned,				TRUE,
                                 XmNentryAlignment,			XmALIGNMENT_END,
                                 NULL);

    if (XtClass(parent) == xmFormWidgetClass) {
        XtVaSetValues(rc,
                      XmNleftAttachment,		XmATTACH_FORM,
                      XmNrightAttachment,		XmATTACH_FORM,
                      NULL);
    }

    /* Add Option Menu to rc */
    for (i = 0; buttons[i].name; i++) {
        if ( ( i % num_columns ) == 0 ) {
            column = XtVaCreateManagedWidget("column",
                                             xmRowColumnWidgetClass, 	rc,
                                             XmNorientation,				
                                             (orientation == XmVERTICAL
                                              ? XmHORIZONTAL
                                              : XmVERTICAL),
                                             XmNnumColumns,	 	2 * num_columns,
                                             XmNisAligned,		TRUE,
                                             XmNentryAlignment, XmALIGNMENT_END,
                                             NULL);
        }
		
        if (buttons[i].menu) {
            xmstring = XmStringCreateSimple(buttons[i].label);
            XtVaCreateManagedWidget(MtCreateName(buttons[i].name, "label"),
                                    xmLabelWidgetClass,	column,
                                    XmNlabelString, 	xmstring,
                                    NULL);
            XmStringFree(xmstring);
            MtBuildMenu(column, XmMENU_OPTION, buttons[i].name, NULL, 
                        buttons[i].menu);
        }
        else {
            xmstring = XmStringCreateSimple((buttons[i].label
                                             ? buttons[i].label
                                             : ""));
            XtVaCreateManagedWidget(MtCreateName(buttons[i].name, "label"),
                                    xmLabelWidgetClass,	column,
                                    XmNlabelString,		xmstring,
                                    NULL);
            XmStringFree(xmstring);
            xmstring = XmStringCreateSimple("");
            XtVaCreateManagedWidget(MtCreateName(buttons[i].name, "cascade"),
                                    xmLabelWidgetClass,	column,
                                    XmNlabelString,		xmstring,
                                    NULL);
            XmStringFree(xmstring);
        }
    }

    /* Align labels by setting width to widest label */
    max = 0;
    for (i = 0; buttons[i].name; i++) {
        width = 0;
        label = MtGetChild(rc, MtCreateName(buttons[i].name, "label"));
        if (label){
            XtVaGetValues(label, XmNwidth, &width, NULL);
        }
        max = width > max ? width : max;
    }
    for (i = 0; buttons[i].name; i++) {
        label = MtGetChild(rc, MtCreateName(buttons[i].name, "label"));
        XtVaSetValues(label, 
                      XmNwidth,				max, 
                      XmNrecomputeSize,		False,
                      NULL);
    }

    return (box);
}

String  
#ifndef _NO_PROTO
MtDisplayModal(Widget shell, ButtonItem *action_buttons)
#else
    MtDisplayModal(shell, action_buttons)
    Widget shell;
    ButtonItem *action_buttons;
#endif
{
    Widget button;
    int i, nbuttons;
    Window *windows = NULL;
    Boolean eventTrapping = True;
    Display *display = XtDisplay(shell);
    XEvent event;

    /* Create list of all action area button windows */
    for (i=0, nbuttons=0; action_buttons[i].name; i++) {
        if ((button = MtGetChild(shell,
                                 MtCreateName(action_buttons[i].name, "button")))) {
            windows = (Window *)XtRealloc((char *)windows,
                                          (nbuttons+1)*sizeof(Window));
            windows[nbuttons] = XtWindow(button);
            nbuttons++;
        }
    }
/*
  XtVaSetValues(shell, XmNdialogStyle,XmDIALOG_FULL_APPLICATION_MODAL, NULL);
  */
    XtPopup(shell, XtGrabExclusive);

    /* Redirect user input to the modal shell */
    XtAddGrab(shell, True, False);

    /* Enter into X event dispatch loop */
    while (eventTrapping) {
        XNextEvent(display, &event);
        if (event.type == ButtonRelease) {
            for (i=0; i < nbuttons; i++) {
                if (windows[i] == event.xbutton.window) {
                    XSync(display, False);
                    eventTrapping = False;
                    break;
                }
            }
        }

        /* Dispatch the event */
        XtDispatchEvent(&event);
    }
    if (windows)
        XTFREE(windows);

    return (action_buttons[i].name);
}

void
#ifndef _NO_PROTO
MtDisplayFileSelector(Widget parent, String title, FileSelectionData *data)
#else
    MtDisplayFileSelector(parent, title, data)
    Widget				parent;
    char				*title;
    FileSelectionData 	*data;
#endif
{
    XmString mask, ok_label, help_label, cancel_label;
    static Widget dialog = NULL, help_button, cancel_button;

    if (!dialog) {
        dialog = XmCreateFileSelectionDialog(parent, title, NULL, 0);
        help_button = XmFileSelectionBoxGetChild(dialog, XmDIALOG_HELP_BUTTON);
        cancel_button = XmFileSelectionBoxGetChild(dialog,
                                                   XmDIALOG_CANCEL_BUTTON);
    }
    XtRemoveAllCallbacks(dialog, XmNokCallback);
    XtRemoveAllCallbacks(dialog, XmNcancelCallback);
    XtRemoveAllCallbacks(dialog, XmNhelpCallback);

    XtVaSetValues(XtParent(dialog), XmNtitle, title, NULL);
    mask = XmStringCreateSimple(data->dir_mask);
    if (data->help_label)
        help_label = MtCreateXmString(data->help_label);
    else
        help_label = XmStringCreateSimple("Help");
    if (data->cancel_label)
        cancel_label = MtCreateXmString(data->cancel_label);
    else
        cancel_label = XmStringCreateSimple("Cancel");
    ok_label = MtCreateXmString(data->ok_label);
    XtVaSetValues(dialog, 
                  XmNpattern,			mask, 
                  XmNhelpLabelString,	help_label,
                  XmNcancelLabelString,	cancel_label,
                  XmNokLabelString, 	ok_label,
                  NULL);
    XmStringFree(mask);
    XmStringFree(help_label);
    XmStringFree(cancel_label);
    XmStringFree(ok_label);

    if (data->help_callback) {
        XtManageChild(help_button);
        XtAddCallback(dialog, XmNhelpCallback,
                      data->help_callback, data->help_data);
    }
    else {
        XtUnmanageChild(help_button);
    }
    if (data->cancel_callback) {
        XtAddCallback(dialog, XmNcancelCallback,
                      data->cancel_callback, data->cancel_data);
    }
    else {
        XtAddCallback(dialog, XmNcancelCallback,
                      (XtCallbackProc) MtCloseFileSelectorCB, NULL);
    }

    XtAddCallback(dialog, XmNokCallback, data->ok_callback, data->ok_data);
    XtManageChild(dialog);
}

/*
 * type: XmDIALOG_PROMPT, _ERROR, _INFORMATION, _MESSAGE, _QUESTION,
   		_WARNING, _WORKING
*/
void
#ifndef _NO_PROTO
MtDisplayMessageDialog(Widget parent, String title, String message, int
                       type, ButtonItem *buttons)
#else
    MtDisplayMessageDialog(parent, title, message, type, buttons)
    Widget parent;
    String title, message;
    int type;
    ButtonItem *buttons;
#endif 
{
    XmString xm_title, xm_message, label;
    Widget dialog, button;
    Arg args[1];
    Boolean cancelCB = False;
    int i;

    if (!parent || !XtIsRealized(parent)) {
        puts(message);
        return;
    }
    xm_title = XmStringCreateLtoR(title, XmSTRING_DEFAULT_CHARSET);
    xm_message = XmStringCreateLtoR(message, XmSTRING_DEFAULT_CHARSET);
    XtSetArg(args[0], XmNdeleteResponse, XmDESTROY);
    dialog = XmCreateMessageDialog(MtGetShell(parent), "message", args, 1);
    XtVaSetValues(dialog,
                  XmNdialogType,	type,
                  XmNdialogTitle,	xm_title,
                  XmNmessageString, xm_message,
                  NULL);
    XmStringFree(xm_message);
    XmStringFree(xm_title);

    /* Display HELP button only if callback supplied */
    XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_OK_BUTTON));
    XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));

    for (i=0; buttons && buttons[i].name; i++)	{
        if (!strcmp(buttons[i].name, OK)) {
            button = XmMessageBoxGetChild(dialog, XmDIALOG_OK_BUTTON);
            XtManageChild(XmMessageBoxGetChild(dialog, XmDIALOG_OK_BUTTON));
        }
        else if (!strcmp(buttons[i].name, CANCEL)) {
            button = XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON);
            cancelCB = True;
        }
        else if (!strcmp(buttons[i].name, HELP)) {
            button = XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON);
        }
        else {
            continue;
        }
        XtVaSetValues(button, XmNset, buttons[i].set, NULL);
        MtSetLabel(button, buttons[i].label);
        if (buttons[i].callback) {
            XtAddCallback(button, XmNactivateCallback,
                          buttons[i].callback, buttons[i].data);
            XtManageChild(button);
        }
        else if (strcmp(buttons[i].name, CANCEL))
            XtUnmanageChild(button);
    }

    if (!cancelCB)
        XtAddCallback(XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON),
                      XmNactivateCallback,
                      (XtCallbackProc) DestroyDialogCB, NULL);
    if (!buttons) {
        label = XmStringCreateSimple("OK");
        button = XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON);
        XtVaSetValues(button, XmNlabelString, label, NULL);
        XmStringFree(label);
    }
    XtManageChild(dialog);
    XtPopup(XtParent(dialog), XtGrabNone);
}

Widget
#ifndef _NO_PROTO
MtCreatePromptDialog(Widget parent)
#else
    MtCreatePromptDialog(parent)
    Widget parent;
#endif
{
    Arg args[2];
    XtSetArg(args[0], XmNautoUnmanage, False);
    XtSetArg(args[1], XmNdeleteResponse, XmDO_NOTHING);
    return (XmCreatePromptDialog(MtGetShell(parent), "message", args, 2));
}

void
#ifndef _NO_PROTO
MtDisplayPromptDialog(Widget dialog, String title, String message,
                      ButtonItem *buttons)
#else
    MtDisplayPromptDialog(dialog, title, message, buttons)
    Widget dialog;
    String title, message;
    ButtonItem *buttons;
#endif
{
    XmString xm_title = XmStringCreateSimple(title);
    XmString xm_message = XmStringCreateSimple(message), label;
    Widget button;
    Boolean cancelCB = False;
    Atom WM_DELETE_WINDOW = XmInternAtom(XtDisplay(dialog), "WM_DELETE_WINDOW",
                                         False);
    int i;

    XtVaSetValues(dialog,
                  XmNdialogTitle,			xm_title,
                  XmNselectionLabelString, 	xm_message,
                  NULL);
    XmStringFree(xm_message);
    XmStringFree(xm_title);

    /* Display HELP button only if callback supplied */
    XtUnmanageChild(XmSelectionBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));
    XtRemoveAllCallbacks(dialog, XmNokCallback);
    XtRemoveAllCallbacks(dialog, XmNcancelCallback);
    XtRemoveAllCallbacks(dialog, XmNhelpCallback);
    for (i=0; buttons[i].name; i++)	{
        if (!strcmp(buttons[i].name, OK))
            button = XmSelectionBoxGetChild(dialog, XmDIALOG_OK_BUTTON);
        else if (!strcmp(buttons[i].name, CANCEL))
            button = XmSelectionBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON);
        else if (!strcmp(buttons[i].name, HELP))
            button = XmSelectionBoxGetChild(dialog, XmDIALOG_HELP_BUTTON);
        else
            continue;
        label = XmStringCreateSimple(buttons[i].label);
        XtVaSetValues(button,
                      XmNlabelString,	label,
                      XmNset,			buttons[i].set,	
                      NULL);
        XmStringFree(label);
        if (buttons[i].callback) {
            if (!strcmp(buttons[i].name, OK))
                XtAddCallback(dialog, XmNokCallback,
                              buttons[i].callback, buttons[i].data);
            else if (!strcmp(buttons[i].name, CANCEL)) {
                cancelCB = True;
                XtAddCallback(dialog, XmNcancelCallback,
                              buttons[i].callback, buttons[i].data);
                XmAddWMProtocolCallback(XtParent(dialog), WM_DELETE_WINDOW,
                                        (XtCallbackProc) buttons[i].callback, buttons[i].data);
            }
            else if (!strcmp(buttons[i].name, HELP))
                XtAddCallback(dialog, XmNhelpCallback,
                              buttons[i].callback, buttons[i].data);
            XtManageChild(button);
        }
        else if (strcmp(buttons[i].name, CANCEL))
            XtUnmanageChild(button);
    }
    if (!cancelCB) {
        XtAddCallback(dialog, XmNcancelCallback,
                      (XtCallbackProc) MtCloseDialogCB, NULL);
        XmAddWMProtocolCallback(XtParent(dialog), WM_DELETE_WINDOW,
                                (XtCallbackProc) MtCloseDialogCB, NULL);
    }
    XtManageChild(dialog);
    XtPopup(XtParent(dialog), XtGrabNone);
}

Boolean
#ifndef _NO_PROTO
MtMonoDisplay(Display *display)
#else
    MtMonoDisplay(display)
    Display		*display;
#endif
{
    /*
     *  Determine whether the server is a mono or color.
     *  This just allows us to set up appropriate resources
     *  for the two cases.
	*/

    return(DefaultDepth(display, DefaultScreen(display)) < 2);
}

/*
 * Convert XmString to String
 */
String
#ifndef _NO_PROTO
MtCvtXmStringToString(XmString string)
#else
    MtCvtXmStringToString(string)
    XmString	string;
#endif
{
    XmStringContext context;
    XmStringCharSet charset;
    XmStringDirection direction;
    Boolean status, separator;    
    String buf, substring, p;
    int length;

    if (!string)
        return (XtNewString(""));

    if ((length = XmStringLength(string)) == 0)
        return (XtNewString(""));
		
    if (!XmStringInitContext(&context, string))
        return (XtNewString(""));

    status = XmStringGetNextSegment(context, &substring,
                                    &charset, &direction, &separator);
    if (!status) 
        return (XtNewString(""));

    /*
     * Get a section of the Motif string, copy it into the previously
     * allocated buffer and then look for additional sections.
     * p is a pointer to the current end point in the string.
     */
    buf = XTMALLOC(char, length);
    p = buf;
    while (status) {
        p += strlen(strcpy(p, substring));
        if (separator) {
            *p++ = '\n';
            *p = '\0';
    	} 
    	XTFREE(charset);
    	XTFREE(substring);
        status = XmStringGetNextSegment(context, &substring,
                                        &charset, &direction, &separator);
    }
    XmStringFreeContext(context);
    
    return (buf);
}

/*
 *  Make a Compound String looking for embedded font changes: 
 *		@B@ = Bold
 *		@I@ = Italic
 *		@R@ = Roman
 *		@S@ = Symbol
 *		@P@ = previous font
 *		@font@ = other font
 */
static XmString
#ifdef _NO_PROTO
create_simple_xmstring(str)
    String str;
#else
    create_simple_xmstring(String str)
#endif
{
    int incr;
    XmString s1, s2, s3;
    String part0, part1, part2, ptr, font, next_font;

    part0 = part1 = XtNewString(str);

    /*
     * Temporarily replace all literal "@"
     */
    while ((ptr = strstr(part1, "\\@")) != NULL) {
        strcpy(&part1[ptr - part1], &part1[ptr - part1 + 1]);
        part1[ptr-part1] = '\1';
    }

    next_font = font = "roman";
    s1 = XmStringCreate("", font);

    do {
        if ((part2 = strstr(part1, "@"))) {
            incr = 3;
            if (part2[1] == '\0') {
                part2 = NULL;
            }
            else if (part2[1] == 'B' && part2[2] == '@') {
                next_font = "bold";
            }
            else if (part2[1] == 'R' && part2[2] == '@') {
                next_font = "roman";
            }
            else if (part2[1] == 'I' && part2[2] == '@') {
                next_font = "italic";
            }
            else if (part2[1] == 'S' && part2[2] == '@') {
                next_font = "symbol";
            }
            else if (part2[1] == 'P' && part2[2] == '@') {
                /* do nothing */
            }
            else if ((ptr = strstr(&part2[1], "@"))) {
                next_font = &part2[1];
                *ptr = '\0';
                incr = strlen(next_font) + 2;
            }
            else {
                /* False alarm */
                part2 = NULL;
            }
        }
        if (part2) {
            *part2 = '\0';
        }
        while ((ptr = strchr(part1, '\1')) != NULL) {
            part1[ptr-part1] = '@';
        }
        s2 = XmStringCreate(part1, font);
        s3 = XmStringConcat(s1, s2);
        XmStringFree(s1);
        XmStringFree(s2);
        s1 = s3;
        if (part2) {
            String tmp;

            part1 = part2 + incr;
            /* swap font, next font */
            tmp = font;
            font = next_font;
            next_font = tmp;
        }
    } while (part2);

    XtFree((XtPointer)part0);
    return (s1);
}

XmString
#ifdef _NO_PROTO
MtCreateXmString(string)
    String string;
#else
    MtCreateXmString(String string)
#endif
{
    int value, length;
    char buf[5], *p, *start, *line, *str;
    XmString str1, str2, str3, sep;

    /* NULL or empty */
    if (!string || string[0] == '\0') {
        return (XmStringCreateSimple(""));
    }

    str = XtNewString(string);

    /* Convert all "\n" to newlines */
    if ((p = strstr(str, "\\n"))) {
        do {
            p[0] = '\n';
            length = strlen(p);
            if (length < 3) {
                p[1] = '\0';
                break;
            }
            memmove(&p[1], &p[2], length-2);
            p[length-1] = '\0';
            start = p + 1;
        } while ((p = strstr(start, "\\n")));
    }

    /* Convert all "\nnn" to octal chars */
    if ((p = strstr(str, "\\"))) {
        do {
            length = strlen(p);
            if (length >= 4
                && isdigit(p[1]) && isdigit(p[2]) && isdigit(p[3])) {
                strncpy(buf, &p[1], 3);
                buf[3] = '\0';
                sscanf(buf, "%o", &value);
                p[0] = value;
                memmove(&p[1], &p[4], length-4);
                p[length-3] = '\0';
            }
            start = p + 1;
        } while ((p = strstr(start, "\\")));
    }

    line = strtok(str, "\n");
    str1 = NULL;
    while (line) {
        str2 = create_simple_xmstring(line);
        if (str1) {
            sep = XmStringSeparatorCreate();
            str3 = XmStringConcat(str1, sep);
            XmStringFree(sep);
            XmStringFree(str1);
            str1 = XmStringConcat(str3, str2);
            XmStringFree(str2);
            XmStringFree(str3);
        }
        else {
            str1 = str2;
        }

        line = strtok(NULL, "\n");
    }

    XtFree((XtPointer)str);
    return(str1);
}

/*
 * Translate \n to newline
 */	
static Boolean
#ifndef _NO_PROTO
CvtStringToXmString(Display *dpy,
                    XrmValuePtr args, Cardinal *num_args,
                    XrmValuePtr from, XrmValuePtr to, XtPointer *data)
#else
    CvtStringToXmString(dpy, args, num_args, from, to, data)
    Display *dpy;
    XrmValuePtr args;
    Cardinal *num_args;
    XrmValuePtr from, to;
    XtPointer *data;
#endif
{
    Boolean converted = True;
    XmString string = MtCreateXmString(from->addr);

    if (to->addr) {
        if (to->size >= sizeof(XmString)) {
            *(XmString *) to->addr = string;
        }
        else {
            converted = False;
            XmStringFree(string);
        }
    }
    else {
        to->addr = (caddr_t) &string;
    }
    to->size = sizeof(XmString);
    return (converted);
}

void
#ifndef _NO_PROTO
MtSetXmStringConverter(void)
#else
    MtSetXmStringConverter()
#endif
{
    XtSetTypeConverter(XmRString, XmRXmString,
                       CvtStringToXmString, NULL, 0, XtCacheNone, NULL);
}

/*
 * Sets backing store for widget and all its children
 */
void
#ifndef _NO_PROTO
MtSetBackingStore(Widget w)
#else
    MtSetBackingStore(w)
    Widget w;
#endif
{
    XSetWindowAttributes attr;
    int i, num_child;
    Widget *child;

    if (!XtIsRealized(w) || !XDoesBackingStore(XtScreen(w)))
        return;
    num_child = MtFindChildren(w, &child, True, False);
    attr.backing_store = WhenMapped;
    XChangeWindowAttributes(XtDisplay(w), XtWindow(w), CWBackingStore, &attr);
    for (i = 0; i < num_child; i++)
        XChangeWindowAttributes(XtDisplay(w), XtWindow(child[i]),
                                CWBackingStore, &attr);
    if (child)
        XTFREE(child);
}

/*
 * Initialize the random generator
 */
void
MtSeedRandom()
{
#if defined(__VMS) || defined(__QNX__) || defined(__NUTC__) 
    srand(((int) time(NULL)));
#else
    srand48(time(NULL));
#endif
}

#ifdef __cplusplus
    extern	"C" double drand48();
#else
    extern	double drand48();
#endif
/*
 *  Return a random integer in the range [a, b).
 *  NOTE: drand48() returns a random value in
 *  the range [0.0, 1.0).
 */
int
#ifndef _NO_PROTO
MtRandomInt(int a, int b)
#else
    MtRandomInt(a, b)
    int a, b;
#endif
{
    double 	ratio;

#ifdef __VMS
/*
 * Attempt to convert rand() which returns integers
 * in range from 0 to 2E31-1 into drand498 which
 * return numbers in the range 0.0 to 1.0
 */
    ratio = rand()/(double)((1U << 31) - 1);  /* rand()/(2E31-1) */
#else
#if defined(__QNX__) || defined(__NUTC__)
    ratio = rand()/(double)(RAND_MAX);		  /* rand()/(2E31-1) */
#else
    ratio = drand48();
#endif /* __QNX__ */
#endif /* __VMS */

    return (a + (int) (ratio*(b-a)));
}

#endif

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

#define PRINT_3D

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 *
 *  Example of printing an XRT/3d Widget.
 *  This code can merged with any program using an XRT/3d
 *  widget. Essentially what you get here is a robust print dialog
 *  box, allowing you to interactively specify the parameters for
 *  XRT/3d print functions
 *
 *-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#ifdef __VMS
#include <unixlib.h>
#include <unixio.h>
#include <processes.h>
#include "vms_port.h"
#include "vms_param.h"
#else
#include <unistd.h>
#if defined(__QNX__)
#include <rpc/types.h>
#include <limits.h>
#define MAXPATHLEN PATH_MAX
#else
#include <sys/param.h>
#endif /* __QNX__ */
#endif /* __VMS */

#ifndef _NO_PROTO
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <X11/Intrinsic.h>
#include <X11/Composite.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/CascadeB.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/MainW.h>
#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include <Xm/Xrt3d.h>
#include "motif_tools.h"
#include "xrt_print_3d.h"

#ifdef NDEBUG
static char sccsid[] = "@(#)print_3d.c	1.29	98/12/15	KL Group Inc.";
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif  /* MAXPATHLEN */

#define  MAXFONTLENGTH  81
#define  SCALE_DEFAULT  -1.0
#define  PRINT_ERROR	"Print Error"

/**********************************************************************/
/* print_3d typedefs and global definitions                           */
/**********************************************************************/


typedef enum {
    PRINT_POSTSCRIPT = 1,
    PRINT_XWD,
    PRINT_CGM
} PrintType;

typedef enum {
    PRINT_PRINTER = 1,
    PRINT_FILE
} PrintTo;

#if defined(__QNX__)
#define PrintType			int
#define PrintTo				int
#endif


typedef struct {
    double			metric;
    Xrt3dStrokeFont	header;
    Xrt3dStrokeFont	footer;
    Xrt3dStrokeFont	legend;
} Xrt3dDrawCGMProperties;

typedef struct {
    int			inches;
    float		pwidth;
    float		pheight;
    float		pmargin;
    int			landscape;
    float		xoffset;
    float		yoffset;
    float		imagewidth;
    float		imageheight;
    char			header_font[MAXFONTLENGTH];
    char			footer_font[MAXFONTLENGTH];
    char			legend_font[MAXFONTLENGTH];
    int			fill_background;
    int			color;
    int			showpage;
} Xrt3dDrawpsProperties;

typedef struct {
    Xrt3dStrokeFont		charset;
    char 				*name;
} SFontList;


/* This is the graph that we are printing out */
static ButtonItem error_button[] = {
    {CANCEL, "Cancel", NULL, NULL, True}, 
    {NULL, NULL, NULL, NULL, False}
};
static Widget			graph;
static Widget			print_dialog = NULL;

static Widget			format_radio;
static Widget			dest_radio;
static Widget			dir_txt, file_txt, printer_txt;


static int				print_format = PRINT_POSTSCRIPT;
static int				print_dest = PRINT_PRINTER;

static char				directory[MAXPATHLEN] = "";
static char				file[64] = "";
#ifdef __VMS
static char				printer[64] = "SYS$PRINT";
#else /* not VMS */
static char				printer[64] = "ps";
#endif /* VMS */


/**********************************************************************/
/* Internal function prototypes                                       */
/**********************************************************************/
#ifdef _NO_PROTO

static Widget	GetActualRadioBox();
static Widget	CreatePrintDialog();
static Widget	CreatePSPropsDialog();
static Widget	CreateCGMProps();

static void     print_callback();
static int      GetPrintRadioValue();
static void     SetPrintRadioValue();
static void		ShowCGMProps();
static void		ShowPSProps();
static void		format_callback();
static void		dest_callback();
static void		dismiss_PSprops_callback();
static void		dismiss_print_callback();
static void		SetRadioSensitive();
static int		PrintGraph();
static void		set_dest();
static void		print_reset();
static void		unmanage_all();
static void		props_callback();
static void		PSupdate();
static void		PSextract();
static void		PSok_callback();
static void		PSreset_callback();
static char		*UniqueName();
static Widget	MakeSFListPopup();
static void		CGMupdate();
static void		CGMextract();
static void		dismiss_CGMprops_callback();
static void		CGMok_callback();
static void		CGMreset_callback();
#else

static Widget	GetActualRadioBox(Widget radio);
static Widget	CreatePrintDialog(Widget parent, XtPointer client_data, 
                                  XtPointer call_data);
static Widget	CreatePSPropsDialog(Widget parent, XtPointer client_data, 
                                    XtPointer call_data);

static void     print_callback(Widget w, XtPointer client_data,
                               XtPointer call_data);
static int      GetPrintRadioValue(Widget radio);
static void     SetPrintRadioValue(Widget radio, XtPointer data);
static void		ShowCGMProps(Widget parent, Xrt3dDrawCGMProperties *cgm_props);
static void		ShowPSProps(Widget parent, Xrt3dDrawpsProperties *ps_props);
static void		format_callback(Widget w,XtPointer client_data,
                                        XtPointer call_data);
static void		dest_callback(Widget w, XtPointer client_data,
                                      XtPointer call_data);
static void		dismiss_PSprops_callback(Widget w, XtPointer client_data, 
                                                 XtPointer call_data);
static void		dismiss_print_callback(Widget w, XtPointer client_data, 
                                               XtPointer call_data);

static void		SetRadioSensitive(Widget radio, Boolean sval);
static int		PrintGraph(Widget button);
static void		set_dest(PrintTo dest);
static void		print_reset();
static void		unmanage_all();
static void		props_callback(Widget w, XtPointer client_data,
                                       XtPointer call_data);
static void		PSupdate();
static void		PSextract();
static void		PSok_callback(Widget w, XtPointer client_data,
                                      XtPointer call_data);
static void		PSreset_callback(Widget w, XtPointer client_data, 
                                         XtPointer call_data);
static char		*UniqueName();
static Widget	MakeSFListPopup(Widget parent, char *title);
static void		CGMupdate();
static void		CGMextract();
static void		dismiss_CGMprops_callback(Widget w, XtPointer client_data, 
                                                  XtPointer call_data);
static void		CGMok_callback(Widget w, XtPointer client_data,
                                       XtPointer call_data);
static void		CGMreset_callback(Widget w, XtPointer client_data, 
                                          XtPointer call_data);
#endif


/**********************************************************************/
/* psprops typedefs and global definitions                           */
/**********************************************************************/

#define TIGHTNESS	20

static Xrt3dDrawpsProperties *PSprops = NULL;

static Widget		pspropsheet;
static Widget		measure_radio_w, orient_radio_w;
static Widget		param_width_txt_w, param_height_txt_w, 
    param_margin_txt_w;
static Widget		pos_left_txt_w, pos_bottom_txt_w, pos_width_txt_w, 
    pos_height_txt_w;
static Widget		font_header_txt_w, font_footer_txt_w, 
    font_legend_txt_w;
static Widget		fill_bg_w, color_radio_w;
static Xrt3dDrawpsProperties ps_props = {
    True, 8.5, 11.0, 0.25, False, 0.0, 0.0, 0.0, 0.0, "", "", "", 1,
    XRT3D_PS_COLOR_AS_IS, 1 
};



/**********************************************************************/
/* cgmprops typedefs and global definitions                           */
/**********************************************************************/

#define TIGHTNESS	20

static Xrt3dDrawCGMProperties *CGMprops = NULL;

static Widget		cgmpropsheet;
static Widget		metric_txt_w;
static Widget		header_pop, footer_pop, legend_pop;
static Xrt3dDrawCGMProperties cgm_props = {
    SCALE_DEFAULT,  XRT3D_SF_ROMAN_DUPLEX, XRT3D_SF_ROMAN_DUPLEX, 
    XRT3D_SF_ROMAN_DUPLEX
};

static SFontList sfont_list[] = {
    { XRT3D_SF_ROMAN_DUPLEX			, "Roman Duplex"},
    { XRT3D_SF_CYRILLIC_COMPLEX		, "Cyrillic Complex"},
    { XRT3D_SF_GOTHIC_ENGLISH			, "Gothic English"},
    { XRT3D_SF_GOTHIC_GERMAN			, "Gothic German"},
    { XRT3D_SF_GOTHIC_ITALIAN			, "Gothic Italian"},
    { XRT3D_SF_GREEK_COMPLEX			, "Greek Complex"},
    { XRT3D_SF_GREEK_COMPLEX_SMALL	, "Greek Complex Small"},
    { XRT3D_SF_GREEK_SIMPLEX			, "Greek Simplex"},
    { XRT3D_SF_ITALIC_COMPLEX			, "Italic Complex"},
    { XRT3D_SF_ITALIC_COMPLEX_SMALL	, "Italic Complex Small"},
    { XRT3D_SF_ITALIC_TRIPLEX			, "Italic Triplex" },
    { XRT3D_SF_ROMAN_COMPLEX			, "Roman Complex"},
    { XRT3D_SF_ROMAN_COMPLEX_SMALL	, "Roman Complex Small"},
    { XRT3D_SF_ROMAN_SIMPLEX			, "Roman Simplex"},
    { XRT3D_SF_ROMAN_TRIPLEX			, "Roman Triplex"},
    { XRT3D_SF_SCRIPT_COMPLEX			, "Script Complex"},
    { XRT3D_SF_SCRIPT_SIMPLEX			, "Script Simplex"},
    { XRT3D_SF_ROMAN_DUPLEX			, (char *) NULL }
};


static Widget
#ifndef _NO_PROTO
CreatePrintDialog(Widget parent, XtPointer client_data, XtPointer call_data)
#else
    CreatePrintDialog(parent, client_data, call_data)
    Widget			parent;
    Widget			client_data;
    Widget			call_data;
#endif
{
    /* This function creates the inside of the main print dialog */
    Widget	container_w;
    Widget	pdf_w;
    static	ButtonItem format_items[] = {
        {"postscript", "PostScript", format_callback, 
         (XtPointer) PRINT_POSTSCRIPT, True},
        {"xwd", "XWD", format_callback, 
         (XtPointer) PRINT_XWD, False},
        {"cgm", "CGM", format_callback,
         (XtPointer) PRINT_CGM, False},
        {NULL, NULL, NULL, NULL, False}
    };
    static	ButtonItem dest_items[] = {
        {"printer", "Printer", dest_callback, 
         (XtPointer) PRINT_PRINTER, True},
        {"file", "File", dest_callback,
         (XtPointer) PRINT_FILE, False},
        {NULL, NULL, NULL, NULL, False}
    };
    static	FieldItem pdf_items[] = {
        {"printer", "Printer:", NULL, NULL, 32, printer, True},
        {"directory", "Directory:", NULL, NULL, 32, directory, True},
        {"file", "File:", NULL, NULL, 32, file, True},
        {NULL}
    };
				
    getcwd(directory, sizeof(directory) - 1);

    container_w = XtVaCreateWidget("container_w",
                                   xmFormWidgetClass, parent,
                                   NULL);

    format_radio = MtBuildRadioBox(container_w, "format_radio", "Format:",
                                   format_items, XmVERTICAL, XmRADIOBUTTON); 
    dest_radio = MtBuildRadioBox(container_w,"destination_radio", "Destination:",
                                 dest_items, XmVERTICAL, XmRADIOBUTTON);
    pdf_w = MtCreateFieldBox(container_w, "pdf_w", NULL, pdf_items, XmVERTICAL);

    printer_txt	= MtGetChild(pdf_w, "printer_field");
    dir_txt		= MtGetChild(pdf_w, "directory_field");
    file_txt	= MtGetChild(pdf_w, "file_field");

    XtVaSetValues(format_radio, 
                  XmNtopAttachment,	XmATTACH_FORM,
                  XmNleftAttachment,	XmATTACH_FORM, 
                  NULL);
    XtVaSetValues(dest_radio,
                  XmNtopAttachment,	XmATTACH_FORM,
                  XmNleftAttachment,	XmATTACH_WIDGET,
                  XmNleftWidget,		format_radio,
                  NULL);
    XtVaSetValues(pdf_w,
                  XmNtopAttachment,		XmATTACH_WIDGET,
                  XmNtopWidget,			format_radio,
                  XmNleftAttachment,		XmATTACH_FORM,
                  XmNrightAttachment,		XmATTACH_FORM,
                  NULL);

    print_reset();
    return (container_w);
}
	



/* Since the MtBuildRadioBox function returns a radio box nested
 * deep inside a couple other widgets, this functions essentially
 * plucks it out and returns its Widget pointer
 */
static Widget
#ifndef _NO_PROTO
GetActualRadioBox(Widget radio)
#else
    GetActualRadioBox(radio)
    Widget		radio;
#endif
{
    WidgetList	children;
    Widget		contained;
    int			num_children;


    contained = MtGetChild(radio, "containedRC");
    XtVaGetValues(contained,
                  XmNchildren,		&children,
                  XmNnumChildren,	&num_children,
                  NULL);
    if (num_children > 1) {
        /* then the MtBuildRadioBox function has been changed */
        return (NULL);
    }
    else {
        return (children[0]);
    }

}

static void
#ifndef _NO_PROTO
SetPrintRadioValue(Widget radio, XtPointer data)
#else
    SetPrintRadioValue(radio, data)
    Widget		radio;
    XtPointer		data;
#endif
{
    /*
     *  Set the value of the given RadioBox.
	 */

    int			i, n;
    XtPointer		tval;
    Widget		tmp_radio;
    WidgetList	children;

    tmp_radio = GetActualRadioBox(radio);

    XtVaGetValues(tmp_radio,
                  XmNchildren,		&children,
                  XmNnumChildren,	&n,
                  NULL);
    for (i=0; i<n; i++)	{
        XtVaGetValues(children[i],
                      XmNuserData,		&tval,
                      NULL);
        XtVaSetValues(children[i],
                      XmNset,			(tval == data),
                      NULL);
    }
}


static int
#ifndef _NO_PROTO
GetPrintRadioValue(Widget radio)
#else
    GetPrintRadioValue(radio)
    Widget		radio;
#endif
{
    /*
     *  Get the value of the given RadioBox.
	 */

    int			i, n;
    Boolean		set;
    Widget		tmp_radio;
    XtPointer		tval;
    WidgetList	children;

    tmp_radio = GetActualRadioBox(radio);
    XtVaGetValues(tmp_radio,
                  XmNchildren,		&children,
                  XmNnumChildren,	&n,
                  NULL);
    for (i=0; i<n; i++)	{
        XtVaGetValues(children[i],
                      XmNset,			&set,
                      XmNuserData,		&tval,
                      NULL);
        if (set) {
            return((int) tval);
        }
    }
    return((int) NULL);	
}

static void
#ifndef _NO_PROTO
SetRadioSensitive(Widget radio, Boolean sval)
#else
    SetRadioSensitive(radio, sval)
    Widget		radio;
    Boolean		sval;
#endif
{
    /*
     *  Set all buttons in the given Radio Box active or inactive
     */

    int			i, n;
    Widget		tmp_radio;
    WidgetList	children;

    tmp_radio = GetActualRadioBox(radio);

    XtVaGetValues(tmp_radio,
                  XmNchildren,		&children,
                  XmNnumChildren,	&n,
                  NULL);
    for (i=0; i<n; i++)	{
        XtSetSensitive(children[i], sval);
    }
}



/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 *
 *  Internal routines.
 *
 *-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/


static int
#ifndef _NO_PROTO
PrintGraph(Widget button)
#else
    PrintGraph(button)
    Widget	button;
#endif
{
    /*
     *  Print the XRT/3d widget in the requested format to the requested
     *  destination.
	 */

    int			status = 0;
    char		msg[128];
#ifdef __VMS
    char 			tmp_psfname[L_tmpnam+3]; /* +3 for ".PS" type */
    char            *err_msg;
#endif
    char			buffer[MAXPATHLEN];
    FILE		*fp;
    char		*value;
								

    print_format = GetPrintRadioValue(format_radio);
    print_dest = GetPrintRadioValue(dest_radio);

    XtVaGetValues(dir_txt, XmNvalue, &value, NULL);
    strcpy(directory, value);
    XtVaGetValues(file_txt, XmNvalue, &value, NULL);
    strcpy(file, value);
    XtVaGetValues(printer_txt, XmNvalue, &value, NULL);
    strcpy(printer, value);

    if((print_format == PRINT_POSTSCRIPT) && (print_dest == PRINT_PRINTER)) {
#ifdef __VMS
        tmpnam(tmp_psfname);
        strcat(tmp_psfname, ".PS");
        if ((fp = fopen(tmp_psfname, "w")) == NULL)	{
            sprintf(msg,
                    "Can't open temporary Postscript file %s for writing.\n",
                    tmp_psfname);
            MtDisplayMessageDialog(button, PRINT_ERROR, msg,
                                   XmDIALOG_ERROR, error_button);
            goto done;
        }
#else
#if defined(ultrix) || defined(__QNX__)
        if (*printer) {
            sprintf(buffer, "lp -d%s", printer);
        }
        else {
            strcpy(buffer, "lp");
        }
#else
        if (*printer) {
            sprintf(buffer, "lp -s -d%s", printer);
        }
        else {
            strcpy(buffer, "lp -s");
        }
#endif /* ultrix || __QNX__ */
        signal(SIGPIPE, SIG_IGN);
        fp = (FILE *) popen((char *)buffer, "w");
        if (!fp) {
            sprintf(msg, "Can't execute %s\n", buffer);
            MtDisplayMessageDialog(button, PRINT_ERROR, msg,
                                   XmDIALOG_ERROR, error_button);
            goto done;
        }
#endif /* __VMS */
        status = Xrt3dDrawPS(graph, fp, msg,
                             ps_props.inches,
                             ps_props.pwidth,
                             ps_props.pheight,
                             ps_props.pmargin,
                             ps_props.landscape,
                             ps_props.xoffset,
                             ps_props.yoffset,
                             ps_props.imagewidth,
                             ps_props.imageheight,
                             (ps_props.header_font[0] == (char) NULL) 
                             ? (char *) NULL : ps_props.header_font,
                             (ps_props.footer_font[0] == (char) NULL)
                             ? (char *) NULL : ps_props.footer_font,
                             (ps_props.legend_font[0] == (char) NULL)
                             ? (char *) NULL : ps_props.legend_font,
                             ps_props.fill_background,
                             ps_props.color,
                             ps_props.showpage
            );
#ifdef __VMS
        fclose(fp);
        chmod(tmp_psfname,0666);
        if (!print_file(printer, tmp_psfname, TRUE, FALSE, NULL, &err_msg)) {
            sprintf(msg, "Print Error: %s\nPrint Failed.\n", err_msg);
            MtDisplayMessageDialog(button, PRINT_ERROR, msg,
                                   XmDIALOG_ERROR, error_button);
            goto done;
        }
#else
        pclose(fp);
#endif
    }
    else {
        if (!*file)	{
            MtDisplayMessageDialog(button, PRINT_ERROR,
                                   "Please enter a file name.",
                                   XmDIALOG_ERROR, error_button);
            goto done;
        }
#ifdef __VMS
        if (!*directory)	{
            buffer[0] = '\0';
        }
        else	{
            strcpy(buffer, directory);
        }
#else /* is VMS */
        if (!*directory)	{
            strcpy(buffer, "./");
        }
        else	{
            strcpy(buffer, directory);
            strcat(buffer, "/");
        }
#endif /* VMS */
        strcat(buffer, file);

        if ((fp = fopen(buffer, "w")) == NULL)	{
            sprintf(msg, "Can't open %s for writing.\n", buffer);
            MtDisplayMessageDialog(button, PRINT_ERROR, msg, 
                                   XmDIALOG_ERROR, error_button);
            goto done;
        }

        switch(print_format)	{
        case PRINT_POSTSCRIPT:
            status = Xrt3dDrawPS(graph, fp, msg,
                                 ps_props.inches,
                                 ps_props.pwidth,
                                 ps_props.pheight,
                                 ps_props.pmargin,
                                 ps_props.landscape,
                                 ps_props.xoffset,
                                 ps_props.yoffset,
                                 ps_props.imagewidth,
                                 ps_props.imageheight,
                                 (ps_props.header_font[0] == (char) NULL) 
                                 ? (char *) NULL : ps_props.header_font,
                                 (ps_props.footer_font[0] == (char) NULL)
                                 ? (char *) NULL : ps_props.footer_font,
                                 (ps_props.legend_font[0] == (char) NULL)
                                 ? (char *) NULL : ps_props.legend_font,
                                 ps_props.fill_background,
                                 ps_props.color,
                                 ps_props.showpage
                );
            break;
        case PRINT_XWD:
            status = Xrt3dOutputXwd(graph, fp, msg);
            break;
        case PRINT_CGM:
            status = Xrt3dDrawCGM(graph, fp, msg,
                                  cgm_props.metric,
                                  cgm_props.header,
                                  cgm_props.footer,
                                  cgm_props.legend
                );
            break;
        }
        fclose(fp);
    }

    if (status == 0) {
        MtDisplayMessageDialog(button, PRINT_ERROR, msg, 
                               XmDIALOG_ERROR, error_button);
    }

done:

    return(status);
}


static void
#ifndef _NO_PROTO
set_dest(PrintTo dest)
#else
    set_dest(dest)
    PrintTo	dest;
#endif
{
    /*
     *  Set controls active or inactive depending on the print destination.
	 */
    if (dest == PRINT_PRINTER)	{
        XtSetSensitive(XtParent(printer_txt), TRUE);
        XtSetSensitive(XtParent(dir_txt), FALSE);
        XtSetSensitive(XtParent(file_txt), FALSE);
    }
    else	{
        XtSetSensitive(XtParent(printer_txt), FALSE);
        XtSetSensitive(XtParent(dir_txt), TRUE);
        XtSetSensitive(XtParent(file_txt), TRUE);
    }
    SetPrintRadioValue(dest_radio, (XtPointer) dest);
}

static void
print_reset()
{
    /*
     *  Initialize print dialog values
     */
	
    SetPrintRadioValue(format_radio, (XtPointer) print_format);
    SetPrintRadioValue(dest_radio, (XtPointer) print_dest);

    XtVaSetValues(dir_txt, 
                  XmNvalue, getcwd(directory, sizeof(directory) - 1), NULL);
    XtVaSetValues(file_txt, XmNvalue, file, NULL);
    XtVaSetValues(printer_txt, XmNvalue, printer, NULL);
    if (print_format != PRINT_POSTSCRIPT)		{
        SetRadioSensitive(dest_radio, FALSE);
    }
    set_dest((PrintTo)print_dest);
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 *
 *  Callbacks.
 *
 *-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/


static void
unmanage_all()
{
    if (pspropsheet)	{
        XtUnmanageChild(pspropsheet);
    }
    if (cgmpropsheet)	{
        XtUnmanageChild(cgmpropsheet);
    }
    if (print_dialog) {
        XtUnmanageChild(print_dialog);
    }
}

static void
#ifndef _NO_PROTO
print_callback(Widget w, XtPointer client_data, XtPointer call_data)
#else
    print_callback(w, client_data, call_data)
    Widget				w;
    XtPointer				client_data;
    XtPointer				call_data;
#endif
{
    if (PrintGraph(w)) {
        unmanage_all();
    }
}

static void
#ifndef _NO_PROTO
props_callback(Widget w, XtPointer client_data, XtPointer call_data)
#else
    props_callback(w, client_data, call_data)
    Widget				w;
    XtPointer				client_data;
    XtPointer				call_data;
#endif
{
    /*
     *  Call appropriate property sheet for print properties.
	 */

    switch(GetPrintRadioValue(format_radio))	{
    case PRINT_CGM:
        if (pspropsheet) {
            XtUnmanageChild(pspropsheet);
        }
        ShowCGMProps(w, &cgm_props);
        break;
    case PRINT_POSTSCRIPT:
    default:
        if (cgmpropsheet) {
            XtUnmanageChild(cgmpropsheet);
        }
        ShowPSProps(w, &ps_props);
        break;
    }
}

static void
#ifndef _NO_PROTO
dismiss_print_callback(Widget w, XtPointer client_data, XtPointer call_data)
#else
    dismiss_print_callback(w, client_data, call_data)
    Widget				w;
    XtPointer				client_data;
    XtPointer				call_data;
#endif
{
    unmanage_all();
}

static void
#ifndef _NO_PROTO
format_callback(Widget w, XtPointer client_data, XtPointer call_data)
#else
    format_callback(w, client_data, call_data)
    Widget				w;
    XtPointer				client_data;
    XtPointer				call_data;
#endif
{
    Boolean			set;
    PrintType		type;
    Widget			props_button = MtGetChild(print_dialog, "props_button");

    XtVaGetValues(w, XmNset, &set, NULL);
    if (set) {
        type = (PrintType) client_data;
        XtSetSensitive(props_button, (type != PRINT_XWD));
        switch(type)	{
        case PRINT_POSTSCRIPT:
            SetRadioSensitive(dest_radio, TRUE);
            break;
        case PRINT_XWD:
            SetPrintRadioValue(dest_radio, (XtPointer) PRINT_FILE);
            SetRadioSensitive(dest_radio, FALSE);
            set_dest(PRINT_FILE);
            break;
        case PRINT_CGM:
            SetPrintRadioValue(dest_radio, (XtPointer) PRINT_FILE);
            SetRadioSensitive(dest_radio, FALSE);
            set_dest(PRINT_FILE);
            break;
        }
    }
}

static void
#ifndef _NO_PROTO
dest_callback(Widget w, XtPointer client_data, XtPointer call_data)
#else
    dest_callback(w, client_data, call_data)
    Widget				w;
    XtPointer				client_data;
    XtPointer				call_data;
#endif
{
    Boolean			set;

    XtVaGetValues(w, XmNset, &set, NULL);
    if (set) {
        set_dest((PrintTo) client_data);
    }
}

void
#ifndef _NO_PROTO
MtShowPrintCallback(Widget w, XtPointer client_data, XtPointer call_data)
#else
    MtShowPrintCallback(w, client_data, call_data)
    Widget				w;
    XtPointer				client_data;
    XtPointer				call_data;
#endif
{
    XtManageChild(print_dialog);
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 *
 *  Print Panel Construction.
 *
 *-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/


/*********************************************************************/
/**                            PSPROPS code                         **/
/*********************************************************************/

#define PSPROPS



/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-==
 *
 *  Internal Functions.
 *
 *-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-==
*/
static void
PSupdate()
{
    String s;
    /*
     *  Set the print panel default PS print properties from this panel.
     */

    if (PSprops)	{
        PSprops->inches = GetPrintRadioValue(measure_radio_w);
        PSprops->landscape = GetPrintRadioValue(orient_radio_w);
        PSprops->color = (int) GetPrintRadioValue(color_radio_w);

        PSprops->fill_background = (int) XmToggleButtonGetState(fill_bg_w);

        XtVaGetValues(param_width_txt_w, XmNvalue, &s, NULL);
        PSprops->pwidth = (float) atof(s);
        XtVaGetValues(param_height_txt_w, XmNvalue, &s, NULL);
        PSprops->pheight = (float) atof(s);
        XtVaGetValues(param_margin_txt_w, XmNvalue, &s, NULL);
        PSprops->pmargin = (float) atof(s);
        XtVaGetValues(pos_left_txt_w, XmNvalue, &s, NULL);
        PSprops->xoffset = (float) atof(s);
        XtVaGetValues(pos_bottom_txt_w, XmNvalue, &s, NULL);
        PSprops->yoffset = (float) atof(s);
        XtVaGetValues(pos_width_txt_w, XmNvalue, &s, NULL);
        PSprops->imagewidth = (float) atof(s);
        XtVaGetValues(pos_height_txt_w, XmNvalue, &s, NULL);
        PSprops->imageheight = (float) atof(s);

        XtVaGetValues(font_header_txt_w, XmNvalue, &s, NULL);
        strcpy(PSprops->header_font, s);
        XtVaGetValues(font_footer_txt_w, XmNvalue, &s, NULL);
        strcpy(PSprops->footer_font, s);
        XtVaGetValues(font_legend_txt_w, XmNvalue, &s, NULL);
        strcpy(PSprops->legend_font, s);
    }
}

static void
PSextract()
{
    /*
     *  Get and display the default print properties from the print panel.
	 */
    char		buf[80];

    if (PSprops) {
        SetPrintRadioValue(measure_radio_w, (XtPointer) PSprops->inches);
        SetPrintRadioValue(orient_radio_w, (XtPointer) PSprops->landscape);
        SetPrintRadioValue(color_radio_w, (XtPointer) PSprops->color);

        XmToggleButtonSetState(fill_bg_w,PSprops->fill_background, FALSE);

        sprintf(buf, "%.2f",PSprops->pwidth);
        XtVaSetValues(param_width_txt_w, XmNvalue, buf, NULL);
        sprintf(buf, "%.2f",PSprops->pheight);
        XtVaSetValues(param_height_txt_w, XmNvalue, buf, NULL);
        sprintf(buf, "%.2f",PSprops->pmargin);
        XtVaSetValues(param_margin_txt_w, XmNvalue, buf, NULL);
        sprintf(buf, "%.2f",PSprops->xoffset);
        XtVaSetValues(pos_left_txt_w, XmNvalue, buf, NULL);
        sprintf(buf, "%.2f",PSprops->yoffset);
        XtVaSetValues(pos_bottom_txt_w, XmNvalue, buf, NULL);
        sprintf(buf, "%.2f",PSprops->imagewidth);
        XtVaSetValues(pos_width_txt_w, XmNvalue, buf, NULL);
        sprintf(buf, "%.2f",PSprops->imageheight);
        XtVaSetValues(pos_height_txt_w, XmNvalue, buf, NULL);

        XtVaSetValues(font_header_txt_w, XmNvalue,PSprops->header_font, NULL);
        XtVaSetValues(font_footer_txt_w, XmNvalue,PSprops->footer_font, NULL);
        XtVaSetValues(font_legend_txt_w, XmNvalue,PSprops->legend_font, NULL);
    }
}


/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 *
 *  Callbacks.
 *
 *-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

static void
#ifndef _NO_PROTO
dismiss_PSprops_callback(Widget w, XtPointer client_data, XtPointer call_data)
#else
    dismiss_PSprops_callback(w, client_data, call_data)
    Widget				w;
    XtPointer				client_data;
    XtPointer				call_data;
#endif
{
    XtUnmanageChild(pspropsheet);
}

static void
#ifndef _NO_PROTO
PSok_callback(Widget w, XtPointer client_data, XtPointer call_data)
#else
    PSok_callback(w, client_data, call_data)
    Widget				w;
    XtPointer				client_data;
    XtPointer				call_data;
#endif
{
    PSupdate();
    print_callback(w, client_data, call_data);
}


static void
#ifndef _NO_PROTO
PSreset_callback(Widget w, XtPointer client_data, XtPointer call_data)
#else
    PSreset_callback(w, client_data, call_data)
    Widget				w;
    XtPointer				client_data;
    XtPointer				call_data;
#endif
{
    PSextract();
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 *
 *  PostScript Property Panel Construction.
 *
 *-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/
static Widget
#ifndef _NO_PROTO
CreatePSPropsDialog (Widget parent, XtPointer client_data, XtPointer call_data)
#else
    CreatePSPropsDialog (parent, client_data, call_data)
    Widget		parent;
    XtPointer	client_data;
    XtPointer	call_data;
#endif
{
    Widget	form, controls, frame1, frame1_1, frame1_2, frame1_3, frame2,
        fill_radio_w, paper_param_w, printed_pos_w, fonts_w; 

    static	ButtonItem measure_items[] = {
        {"inches", "INCHES", NULL, (XtPointer) True, True},
        {"cm", "CM", NULL, (XtPointer) False, False},
        {NULL, NULL, NULL, NULL, False}
    };
    static	ButtonItem orient_items[] = {
        {"portrait", "Portrait", NULL, (XtPointer) False, True},
        {"landscape", "Landscape", NULL, (XtPointer) True, False},
        {NULL, NULL, NULL, NULL, False}
    };
    static ButtonItem color_items[] = {
        {"Color","Color", NULL, (XtPointer) XRT3D_PS_COLOR_AS_IS, True},
        /* smart mono */
        {"Auto","Auto detect",  NULL, (XtPointer) XRT3D_PS_COLOR_AUTO, False}, 
        {"Mono","Monochrome",  NULL, (XtPointer) XRT3D_PS_COLOR_MONO,  False},
        {NULL, NULL, NULL, NULL, False}
    };
    static	FieldItem paper_items[] = {
        {"width", "Width:", NULL, NULL, 8, "8.50", True},
        {"height", "Height:", NULL, NULL, 8, "11.0", True},
        {"margin", "Margin:", NULL, NULL, 8, "0.25", True},
        {NULL, NULL, NULL, NULL, 0, NULL, False}
    };
    static	FieldItem position_items[] = {
        {"left", "From Left:", NULL, NULL, 8, "0.00", True},
        {"bottom", "From Bottom:", NULL, NULL, 8, "0.00", True},
        {"width", "Width:", NULL, NULL, 8, "0.00", True},
        {"height", "Height:", NULL, NULL, 8, "0.00", True},
        {NULL, NULL, NULL, NULL, 0, NULL, False}
    };
    static	FieldItem font_items[] = {
        {"header", "Header:", NULL, NULL, 20, NULL, True},
        {"footer", "Footer:", NULL, NULL, 20, NULL, True},
        {"legend", "Legend:", NULL, NULL, 20, NULL, True},
        {NULL, NULL, NULL, NULL, 0, NULL, False}
    };	
    static ButtonItem fill_items[] = {
        {"fill_bg",		"Fill Background",	NULL, (XtPointer) NULL, True},
        {NULL, NULL, NULL, NULL, False}
    }; 

    form = XtVaCreateWidget("container_w",
                            xmFormWidgetClass, parent,
                            NULL);

    controls = XtVaCreateManagedWidget("controlsRC",
                                       xmRowColumnWidgetClass, 	form,
                                       XmNorientation,			XmVERTICAL,
                                       XmNtopAttachment,		XmATTACH_FORM,
                                       XmNleftAttachment,		XmATTACH_FORM,
                                       XmNrightAttachment,		XmATTACH_FORM,
                                       XmNbottomAttachment,		XmATTACH_FORM,
                                       NULL);

    frame1 = XtVaCreateManagedWidget("formatRC1",
                                     xmRowColumnWidgetClass, 	controls,
                                     XmNorientation,			XmHORIZONTAL,
                                     NULL);

    frame1_1 = XtVaCreateManagedWidget("formatRC1_1",
                                       xmRowColumnWidgetClass, 	frame1,
                                       XmNorientation,			XmVERTICAL,
                                       NULL);

    measure_radio_w	= MtBuildRadioBox(frame1_1, "measure_radio", 
                                          "Measure In:", measure_items,
                                          XmVERTICAL, XmRADIOBUTTON); 

    orient_radio_w	=  MtBuildRadioBox(frame1_1, "orient_radio",
                                           "Orientation:", orient_items,
                                           XmVERTICAL, XmRADIOBUTTON);  

    frame1_2 = XtVaCreateManagedWidget("formatRC1_2",
                                       xmRowColumnWidgetClass, 	frame1,
                                       XmNorientation,				XmVERTICAL,
                                       NULL);

    color_radio_w = MtBuildRadioBox(frame1_2,"color","Color:", color_items,
                                    XmVERTICAL, XmRADIOBUTTON);

    fill_radio_w = MtBuildRadioBox(frame1_2, "fill", NULL,
                                   fill_items, XmHORIZONTAL, XmCHECKBUTTON);

    fill_bg_w = MtGetChild(fill_radio_w, "fill_bg_button");

    frame1_3 = XtVaCreateManagedWidget("formatRC1_3",
                                       xmRowColumnWidgetClass, 	frame1,
                                       XmNorientation,			XmVERTICAL,
                                       NULL);

    paper_param_w = MtCreateFieldBox(frame1_3,"paper", "Paper Parameters:",
                                     paper_items, XmVERTICAL);
    param_width_txt_w  = MtGetChild (paper_param_w, "width_field");
    param_height_txt_w = MtGetChild (paper_param_w, "height_field");
    param_margin_txt_w = MtGetChild (paper_param_w, "margin_field");

    frame2 = XtVaCreateManagedWidget("formatRC2",
                                     xmRowColumnWidgetClass, 	controls,
                                     XmNorientation,			XmHORIZONTAL,
                                     NULL);

    printed_pos_w 	  = MtCreateFieldBox(frame2,"positon", "Printed Position:",
                                             position_items, XmVERTICAL);
    pos_left_txt_w    = MtGetChild (printed_pos_w, "left_field");
    pos_bottom_txt_w  = MtGetChild (printed_pos_w, "bottom_field");
    pos_width_txt_w   = MtGetChild (printed_pos_w, "width_field");
    pos_height_txt_w  = MtGetChild (printed_pos_w, "height_field");


    fonts_w			  = MtCreateFieldBox(frame2,"fonts", "Fonts:",
                                                     font_items, XmVERTICAL);
    font_header_txt_w = MtGetChild (fonts_w, "header_field");
    font_footer_txt_w = MtGetChild (fonts_w, "footer_field");
    font_legend_txt_w = MtGetChild (fonts_w, "legend_field");

    return (form);
}
	

static void
#ifndef _NO_PROTO
ShowPSProps(Widget parent, Xrt3dDrawpsProperties *ps_props)
#else
    ShowPSProps(parent, ps_props)
    Widget						parent;
    Xrt3dDrawpsProperties	*ps_props;
#endif
{
    static ButtonItem ps_buttons[] = {
        {"Print", "Print", PSok_callback, False},
        {"Reset", "Reset", PSreset_callback, False},
        {CANCEL, "Cancel", dismiss_PSprops_callback, False},
        {NULL, NULL, NULL, False}
    };
    /*
     *  Create the PostScript property dialog if it doesn't exist.
     *  Get the current print properties and display them.
     */
    if (!pspropsheet) {
        pspropsheet = MtCreateShell(MtGetTopLevel(parent), "pspropsheet",
                                    "PostScript Print Properties",
                                    CreatePSPropsDialog, NULL,
                                    ps_buttons, False);
    }

    PSprops = ps_props;
    PSextract();
    XtManageChild(pspropsheet);
}

/*********************************************************************/
/**                            CGMPROPS code                        **/
/*********************************************************************/

#define CGMPROPS


	
/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-==
 *
 *  Internal Functions.
 *
 *-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-==
*/

static String
UniqueName()
{
    static int		ctr = 0;
    static char		name[64];

    sprintf(name, "Print3D_%d", ctr);
    ctr++;
    return(name);
}

static void
#ifndef _NO_PROTO
SFOptionsCB(Widget w, XtPointer client, XtPointer call)
#else
    SFOptionsCB(w, client, call)
    Widget		w;
    XtPointer	client, call;
#endif
{
    Widget	parent = XtParent(w);
    int		index = *((int *) &client);

    if (parent == header_pop) 
        CGMprops->header = sfont_list[index].charset;
    else if (parent == footer_pop) 
        CGMprops->footer = sfont_list[index].charset;
    else 
        CGMprops->legend = sfont_list[index].charset;
}

static Widget 
#ifndef _NO_PROTO
MakeSFListPopup(Widget parent, char *title)
#else
    MakeSFListPopup(parent, title)
    Widget 	parent;
    char	*title;
#endif
{
    int			i;
    Widget		item, menu, popup;
    XmString	string;
    Arg			args[2];

    menu = XmCreatePulldownMenu(parent, UniqueName(), NULL, 0);
    string = XmStringCreateSimple(title);

    XtSetArg(args[0], XmNsubMenuId, menu);
    XtSetArg(args[1], XmNlabelString, string);
    popup = XmCreateOptionMenu(parent, title, args, 2);
    XmStringFree(string);

    for (i=0; sfont_list[i].name ; i++) {
        item = XtVaCreateManagedWidget(sfont_list[i].name, 
                                       xmPushButtonGadgetClass, 	menu,
                                       NULL);
        XtAddCallback(item, XmNactivateCallback, (XtCallbackProc) SFOptionsCB, 
                      (XtPointer) i);
    }

    XtManageChild(popup);
    return(menu);
}

static void
CGMupdate()
{
    String	s;
    XtVaGetValues (metric_txt_w, XmNvalue, &s, NULL);
    CGMprops->metric = (float) atof(s);
}

static void
CGMextract()
{
    /*
     *  Get and display the default print properties from the print panel.
	 */
    char		buf[80];

    if (CGMprops)	{
        sprintf(buf, "%.2f",CGMprops->metric);
        XtVaSetValues(metric_txt_w, XmNvalue, buf, NULL);
    }
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 *
 *  Callbacks.
 *
 *-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

static void
#ifndef _NO_PROTO
dismiss_CGMprops_callback(Widget w, XtPointer client_data, XtPointer call_data)
#else
    dismiss_CGMprops_callback(w, client_data, call_data)
    Widget				w;
    XtPointer				client_data;
    XtPointer				call_data;
#endif
{
    XtUnmanageChild(cgmpropsheet);
}

static void
#ifndef _NO_PROTO
CGMok_callback(Widget w, XtPointer client_data, XtPointer call_data)
#else
    CGMok_callback(w, client_data, call_data)
    Widget				w;
    XtPointer				client_data;
    XtPointer				call_data;
#endif
{
    CGMupdate();
    print_callback(w, client_data, call_data);
}


static void
#ifndef _NO_PROTO
CGMreset_callback(Widget w, XtPointer client_data, XtPointer call_data)
#else
    CGMreset_callback(w, client_data, call_data)
    Widget				w;
    XtPointer				client_data;
    XtPointer				call_data;
#endif
{
    CGMextract();
}

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 *
 *  CGM Property Panel Construction.
 *
 *-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/


static Widget 
#ifndef _NO_PROTO
CreateCGMProps(Widget parent, XtPointer client_data, XtPointer call_data)
#else
    CreateCGMProps(parent, client_data, call_data)
    Widget				parent;
    XtPointer			client_data;
    XtPointer			call_data;
#endif
{
    Widget controls, scale_w;
    Widget header_label, footer_label, legend_label;
    static FieldItem scale_item = {"scale_w", "Scale (mm/pixel):",
                                   NULL, NULL,  8, "-1.00", True };

    controls = XtVaCreateManagedWidget("controlsRC",
                                       xmRowColumnWidgetClass,     parent,
                                       XmNnumColumns,              1,
                                       NULL);

    scale_w = MtCreateTextField (controls, &scale_item);
    metric_txt_w = MtGetChild (scale_w, "scale_w_field");
    /* the build menu convenience function won't do us much good
     * here, so instead we have written a new convenience function
     * just for print_3d
     */
    header_pop = MakeSFListPopup(controls, "Header");
    footer_pop = MakeSFListPopup(controls, "Footer");
    legend_pop = MakeSFListPopup(controls, "Legend");

    header_label = XmOptionLabelGadget(MtGetSibling(header_pop, "Header"));
    footer_label = XmOptionLabelGadget(MtGetSibling(header_pop, "Footer"));
    legend_label = XmOptionLabelGadget(MtGetSibling(header_pop, "Legend"));

    MtAlignLabels (header_label, footer_label, legend_label, NULL);

    return(controls);

}

static void
#ifndef _NO_PROTO
ShowCGMProps(Widget parent, Xrt3dDrawCGMProperties *cgm_props)
#else
    ShowCGMProps(parent, cgm_props)
    Widget					parent;
    Xrt3dDrawCGMProperties	*cgm_props;
#endif
{
    /*
     *  Create the CGM property dialog if it doesn't exist.
     *  Get the current print properties and display them.
	 */
    static ButtonItem cgm_buttons[] = {
        {"Print", "Print", CGMok_callback, False},
        {"Reset", "Reset", CGMreset_callback, False},
        {CANCEL, "Cancel", dismiss_CGMprops_callback, False},
        {NULL, NULL, NULL, False}
    };

    if (!cgmpropsheet)	{
        cgmpropsheet = MtCreateShell(MtGetTopLevel(parent), "cgmpropsheet",
                                     "CGM Print Properties",
                                     CreateCGMProps, NULL,
                                     cgm_buttons, False);
    }

    CGMprops = cgm_props;
    CGMextract();
    XtManageChild(cgmpropsheet);
}


/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 *
 *  Routine exported by this file.
 *
 *-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 */

/* Creates new dialog if one doesn't already exist. Otherwise
 * it just pops up the existing dialog.
 */
void
#ifndef _NO_PROTO
MtShow3dPrintDialog(Widget graph_to_print, char *default_printer, 
                    char *default_file)
#else
    MtShow3dPrintDialog(graph_to_print, default_printer, default_file)
    Widget			 graph_to_print;
    char			*default_printer;
    char			*default_file;
#endif
{
    static ButtonItem 	print_buttons[] = {
        {"print", "Print", print_callback, NULL, True},
        {"props", "Properties...", props_callback, NULL, False},
        {CANCEL, "Cancel", dismiss_print_callback, NULL, False},
        {NULL, NULL, NULL, False}
    };

    strcpy(printer, default_printer);
    strcpy(file, default_file);

    if (!print_dialog) {
        print_dialog = MtCreateShell(MtGetShell(graph_to_print), 
                                     "print_dialog", "Print XRT/3d",
                                     CreatePrintDialog, NULL, 
                                     print_buttons, False);
    }	
    graph = graph_to_print;
    XtManageChild(print_dialog);
}

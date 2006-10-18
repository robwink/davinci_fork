/*******************************************************************************
*									       *
* Curves.c -- Curves Plot Widget                                               *
*									       *
* Copyright (c) 1991 Universities Research Association, Inc.		       *
* All rights reserved.							       *
* 									       *
* This material resulted from work developed under a Government Contract and   *
* is subject to the following license:  The Government retains a paid-up,      *
* nonexclusive, irrevocable worldwide license to reproduce, prepare derivative *
* works, perform publicly and display publicly by or for the Government,       *
* including the right to distribute to other Government contractors.  Neither  *
* the United States nor the United States Department of Energy, nor any of     *
* their employees, makes any warrenty, express or implied, or assumes any      *
* legal liability or responsibility for the accuracy, completeness, or         *
* usefulness of any information, apparatus, product, or process disclosed, or  *
* represents that its use would not infringe privately owned rights.           *
*                                        				       *
* Fermilab Nirvana GUI Library						       *
* Aug. 20, 1992								       *
*									       *
* Written by Baolin Ren		  				               *
*									       *
*******************************************************************************/
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#if XmVersion == 1002
#include <Xm/PrimitiveP.h>
#endif
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/DrawingA.h>
#include <Xm/PushB.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include <Xm/BulletinB.h>
#include "../util/psUtils.h"
#include "../util/DialogF.h"
#include "../util/misc.h"
#include "drawAxes.h"
#include "dragAxes.h"
#include "CurvesP.h"

#define REDRAW_NONE 0
#define REDRAW_H_AXIS 1
#define REDRAW_V_AXIS 2
#define REDRAW_CONTENTS 4
#define REDRAW_LABELS 8
#define REDRAW_LEGEND 16
#define REDRAW_ALL 31

#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 500
#define LEGEND_HEIGHT 15
#define GAP 10
#define SAMPLE_WIDTH 180
#define FORMULAR_WIDTH 250
#define COLOR_FACTOR 65535.0
#define COMMAND_WIDTH 100
#define symbol_width 8
#define symbol_height 8
#define view_symbol_width 16
#define view_symbol_height 16
#define lines_width 80
#define lines_height 16
#define ps_scale 4
/* PostScript Coordinate conversion if necessary is done here */
#define x_X2PS(x)     (x)
#define y_X2PS(y)     (PSHeight?PSHeight-y:(y))

#define ERROR_BAR_WIDTH 8	/* width of error bars in pixels */
#define ZOOM_FACTOR .25		/* (linear) fraction of currently displayed
				   data to place outside of current limits 
				   when user invokes zoom command */
#define LEFT_MARGIN 0		/* empty space to left of widget.  Should be
				   small since v axis usually reserves more
				   space than it needs (for long numbers) */
#define TOP_MARGIN 7		/* empty space at top of widget */
#define RIGHT_MARGIN 0		/* empty space to right of widget.  Should be
				   small because h axis reserves more room
				   than it needs for last label to stick out */
#define BOTTOM_MARGIN 3		/* empty space at bottom of widget */
#define X_LABEL_MARGIN 7	/* space between x axis label and numbers */
#define Y_LABEL_MARGIN 5	/* space between y axis label and axis line */


typedef struct {
                 char *name;
                 char *line_dashes_list;
                 unsigned char bitmap[160];
                } LinePattern;

typedef struct {
                 char *name;
                 unsigned char bitmap[8];
                } Pattern;
     
typedef struct {                    /* mark pixmap struct for setting widget */
                 char *name;
                 unsigned char bitmap[32];
               } ViewPattern;

static LinePattern lines[] = {
        "line1", "",
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0},
        "line2", "\2\2",
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0},
        "line3", "\12\2\2\2",
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0xff, 0xcc, 0xff, 0xcc, 0xff, 0xcc, 0xff, 0xcc, 0xff, 0xff,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0},
        "line4", "\10\10", 
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0xff,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0},
        "line5", "\20\20",
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0},
        "line6", "\20\10", 
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0},
         "blank", "\10\10",
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0}
      };

static Pattern symbol[] = {
               "square",
               {0x00, 0x7e, 0x42, 0x42, 0x42, 0x42, 0x7e, 0x00},
               "star",
               {0x84, 0x48, 0x30, 0xfe, 0x30, 0x48, 0x84, 0x00},
               "triangle",
               {0x10, 0x10, 0x38, 0x38, 0x7c, 0x7c, 0xfe, 0x00},
               "circle",
               {0x18, 0x66, 0x42, 0x81, 0x81, 0x42, 0x66, 0x18},
               "solid_circle",
               {0x18, 0x7e, 0x7e, 0xff, 0xff, 0x7e, 0x7e, 0x18},
               "cross",
               {0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81},
               "blank_symbol",
               {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
               };

static ViewPattern view_symbol[] = {
               "view_square",
               {0x00, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x3f, 0xfc,
                0x38, 0x1c, 0x30, 0x0c, 0x30, 0x0c, 0x30, 0x0c,
                0x30, 0x0c, 0x30, 0x0c, 0x30, 0x0c, 0x38, 0x1c,
                0x3f, 0xfc, 0x1f, 0xf8, 0x00, 0x00, 0x00, 0x00},
               "view_star",
               {0x00, 0x00, 0x00, 0x00, 0x1c, 0x38, 0x1c, 0x38,
                0x0e, 0x70, 0x07, 0xe0, 0x03, 0xc0, 0x3f, 0xfc,
                0x3f, 0xfc, 0x03, 0xc0, 0x07, 0xe0, 0x0e, 0x70,
                0x1c, 0x38, 0x1c, 0x38, 0x00, 0x00, 0x00, 0x00},
               "view_triangle",
               {0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x03, 0xc0,
                0x03, 0xc0, 0x07, 0xe0, 0x07, 0xe0, 0x0f, 0xf0,
                0x0f, 0xf0, 0x1f, 0xf8, 0x1f, 0xf8, 0x1f, 0xf8,
                0x3f, 0xfc, 0x3f, 0xfc, 0x00, 0x00, 0x00, 0x00},
               "view_circle",
               {0x00, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x0f, 0xf0,
                0x1c, 0x38, 0x18, 0x18, 0x30, 0x0c, 0x30, 0x0c,
                0x30, 0x0c, 0x30, 0x0c, 0x18, 0x18, 0x1c, 0x38,
                0x0f, 0xf0, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x00},
               "view_solid_circle",
               {0x00, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x0f, 0xf0,
                0x1f, 0xf8, 0x1f, 0xf8, 0x3f, 0xfc, 0x3f, 0xfc,
                0x3f, 0xfc, 0x3f, 0xfc, 0x1f, 0xf8, 0x1f, 0xf8,
                0x0f, 0xf0, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x00},
               "view_cross",
               {0x00, 0x00, 0x00, 0x00, 0x30, 0x0c, 0x38, 0x1c,
                0x1c, 0x38, 0x0e, 0x70, 0x07, 0xe0, 0x03, 0xc0,
                0x03, 0xc0, 0x07, 0xe0, 0x0e, 0x70, 0x1c, 0x38,
                0x38, 0x1c, 0x30, 0x0c, 0x00, 0x00, 0x00, 0x00},
               "view_blank_symbol",
               {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
               };
  
static FILE *PSFile;
static int PSWidth  = 0;
static int PSHeight = 0; 
static char *CURVES_COMMAND[] = {"OK", "Apply", "Dismiss", "Cancel"};
static char *scaleName[] ={"Ln( |X| )", "Log( |X| )", "exp(X)", "10^X", 
                    "1/X", "X^2", "|X|", "Sq.Root", "Undo", "Add Const",
                    "Mult Const", "Reset"};
                                                                              
static void motionAP(CurvesWidget w, XEvent *event, char *args, int n_args);
static void btnUpAP(CurvesWidget w, XEvent *event, char *args, int n_args);
static void btn2AP(CurvesWidget w, XEvent *event, char *args, int n_args);
static void btn3AP(CurvesWidget w, XEvent *event, char *args, int n_args);
static void initialize(CurvesWidget request, CurvesWidget new);
static void redisplay(CurvesWidget w, XEvent *event, Region region);
static void redisplayContents(CurvesWidget w, int outDevice, int redrawArea);
static void drawCurvesPlot(CurvesWidget w, Drawable drawBuf, int outDevice);
static void destroy(CurvesWidget w);
static void resize(CurvesWidget w);
static Boolean setValues(CurvesWidget current, CurvesWidget request,
                         CurvesWidget new);
static void updateBufferAllocation(CurvesWidget w);
static XFontStruct *getFontStruct(XmFontList font);
static void verifyDataForLogScaling(CurvesWidget w);
static double dMin(double d1, double d2);
static double dMax(double d1, double d2);
static void drawLegend(CurvesWidget w, Drawable drawBuf, int outDevice);
static void closeShell(CurvesWidget w);
static void compressMotionEvents(Display *display, XEvent *event);
static void freeScalePtr(ScalePtr *ptr);
static void rescale_scale(CurvesWidget sw, Boolean sortX);
static void scaling_treat_apply(CurvesWidget data, int rescale);
static void draw_scale_formular(CurvesWidget data);
static int n_drawable_curves(CurvesWidget data);
static void test_scale(CurvesWidget data, int scale_fn, int current_y,
	double value);

static char defaultTranslations[] = 
    "<Btn1Motion>: Motion()\n\
     <Btn1Down>: Motion()\n\
     <Btn1Up>: BtnUp()\n\
     <Btn2Down>: Btn2Press()\n\
     <Btn3Down>: Btn3Press()\n";

static XtActionsRec actionsList[] = {
    {"Motion", (XtActionProc)motionAP},
    {"BtnUp", (XtActionProc)btnUpAP},
    {"Btn2Press", (XtActionProc)btn2AP},
    {"Btn3Press", (XtActionProc)btn3AP}
};

static XtResource resources[] = {
    {XmNdoubleBuffer, XmCDoubleBuffer, XmRBoolean, sizeof(Boolean),
      XtOffset(CurvesWidget, curves.doubleBuffer), XmRString, "False"},
    {XmNxLogScaling, XmCXLogScaling, XmRBoolean, sizeof(Boolean),
      XtOffset(CurvesWidget, curves.xLogScaling), XmRString, "False"},
    {XmNyLogScaling, XmCYLogScaling, XmRBoolean, sizeof(Boolean),
      XtOffset(CurvesWidget, curves.yLogScaling), XmRString, "False"},
    {XmNfontList, XmCFontList, XmRFontList, sizeof(XmFontList),
      XtOffset(CurvesWidget, curves.font), XmRImmediate, NULL},
    {XmNxAxisLabel, XmCXAxisLabel, XmRXmString, sizeof (XmString), 
      XtOffset(CurvesWidget, curves.xAxisLabel), XmRString, NULL},
    {XmNyAxisLabel, XmCYAxisLabel, XmRXmString, sizeof (XmString), 
      XtOffset(CurvesWidget, curves.yAxisLabel), XmRString, NULL},
    {XmNresizeCallback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (CurvesWidget, curves.resize), XtRCallback, NULL},
    {XmNbtn2Callback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (CurvesWidget, curves.btn2), XtRCallback, NULL},
    {XmNbtn3Callback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (CurvesWidget, curves.btn3), XtRCallback, NULL},
    {XmNredisplayCallback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (CurvesWidget, curves.redisplay), XtRCallback, NULL},
};

CurvesClassRec  curvesClassRec = {
     /* CoreClassPart */
  {
    (WidgetClass) &xmPrimitiveClassRec,  /* superclass            */
    "Curves",                            /* class_name            */
    sizeof(CurvesRec),                   /* widget_size           */
    NULL,                                /* class_initialize      */
    NULL,                                /* class_part_initialize */
    FALSE,                               /* class_inited          */
    (XtInitProc)initialize,		 /* initialize            */
    NULL,                                /* initialize_hook       */
    XtInheritRealize,                    /* realize               */
    actionsList,                         /* actions               */
    XtNumber(actionsList),               /* num_actions           */
    resources,                           /* resources             */
    XtNumber(resources),                 /* num_resources         */
    NULLQUARK,                           /* xrm_class             */
    TRUE,                                /* compress_motion       */
    TRUE,                                /* compress_exposure     */
    TRUE,                                /* compress_enterleave   */
    TRUE,                                /* visible_interest      */
    (XtWidgetProc)destroy,               /* destroy               */
    (XtWidgetProc)resize,                /* resize                */
    (XtExposeProc)redisplay,             /* expose                */
    (XtSetValuesFunc)setValues,          /* set_values            */
    NULL,                                /* set_values_hook       */
    XtInheritSetValuesAlmost,            /* set_values_almost     */
    NULL,                                /* get_values_hook       */
    NULL,                                /* accept_focus          */
    XtVersion,                           /* version               */
    NULL,                                /* callback private      */
    defaultTranslations,                 /* tm_table              */
    NULL,                                /* query_geometry        */
    NULL,                                /* display_accelerator   */
    NULL,                                /* extension             */
  },
  /* Motif primitive class fields */
  {
     (XtWidgetProc)_XtInherit,   	/* Primitive border_highlight   */
     (XtWidgetProc)_XtInherit,   	/* Primitive border_unhighlight */
     XtInheritTranslations,		/* translations                 */
     (XtActionProc)motionAP,		/* arm_and_activate             */
     NULL,				/* get resources      		*/
     0,					/* num get_resources  		*/
     NULL,         			/* extension                    */
  },
  /* Curves class part */
  {
    0,                              	/* ignored	                */
  }
};

WidgetClass curvesWidgetClass = (WidgetClass) &curvesClassRec;

static void register_pattern(name, bits, width, height)
  char *name;
  unsigned char *bits;
  int width, height;

 {
  XImage *image;

    image = (XImage *)XtCalloc(1, sizeof(XImage)) ;
    image->width = width ;
    image->height = height ;
    image->data = (char *)bits ;
    image->depth = 1 ;
    image->xoffset = 0 ;
    image->format = XYBitmap ;
    image->byte_order = LSBFirst ;
    image->bitmap_unit = 8 ;
    image->bitmap_bit_order = MSBFirst ;
    image->bitmap_pad = 8 ;
    image->bytes_per_line = (width + 7) >> 3 ;
  XmInstallImage(image, name);
 }

/*
** Widget initialize method
*/
static void initialize(CurvesWidget request, CurvesWidget new)
{
    XGCValues values;
    int i;
    Display *dpy = XtDisplay(new);
   
    /* Make sure the window size is not zero. */ 
    if (request->core.width == 0)
    	new->core.width = WINDOW_WIDTH;
    if (request->core.height == 0)
   	new->core.height = WINDOW_HEIGHT;

    /* Make a local copy of the fontlist,
       or get the default if not specified */
    if (new->curves.font == NULL)
#ifdef MOTIF10
	new->curves.font = XmFontListCreate(
	    XLoadQueryFont(dpy, "fixed"),
	    XmSTRING_DEFAULT_CHARSET);
#else
    	new->curves.font =
    	    XmFontListCopy(_XmGetDefaultFontList(
    	    			(Widget) new, XmLABEL_FONTLIST));
#endif
    else
        new->curves.font = XmFontListCopy(new->curves.font);

    /* Make local copies of the XmStrings */
    if (new->curves.xAxisLabel != NULL)
    	new->curves.xAxisLabel = XmStringCopy(new->curves.xAxisLabel);
    if (new->curves.yAxisLabel != NULL)
    	new->curves.yAxisLabel = XmStringCopy(new->curves.yAxisLabel);

    /* Create graphics contexts for drawing in the widget */
    values.font = getFontStruct(new->curves.font)->fid;
    values.foreground = new->primitive.foreground;
    values.background = new->core.background_pixel;
    new->curves.gc = XCreateGC(dpy, XDefaultRootWindow(dpy), GCForeground |
                               GCBackground|GCFont, &values);
    new->curves.legendGC = XtGetGC((Widget) new,
                                   GCForeground | GCBackground | GCFont, 
                                   &values);
    new->curves.contentsGC = XCreateGC(dpy, XDefaultRootWindow(dpy),
                             GCForeground | GCBackground, &values);
    for (i = 0; i < NUMLINE; i++) {
      values.line_style = (i == 0) ? LineSolid : LineOnOffDash;
      new->curves.linesGC[i] = XCreateGC(dpy, XDefaultRootWindow(dpy),
                                         GCForeground | GCBackground |
                                         GCFont | GCLineStyle, &values);
      if (i != 0)
        XSetDashes(dpy, new->curves.linesGC[i], 0,
                 lines[i].line_dashes_list, strlen(lines[i].line_dashes_list));
     
      new->curves.legendLineGC[i] = XCreateGC(dpy, XDefaultRootWindow(dpy),
                                         GCForeground | GCBackground |
                                         GCFont | GCLineStyle, &values);
      if (i != 0)
        XSetDashes(dpy, new->curves.legendLineGC[i], 0,
                 lines[i].line_dashes_list, strlen(lines[i].line_dashes_list));
    }
    
    /* Initialize various fields */
    ResetAxisDragging(&new->curves.dragState);
    new->curves.showLegend = True;
    new->curves.isTimeSeries = True;
    new->curves.isShellSet = False;
    new->curves.isPixmapRegisted = False;
    new->curves.isScaleSet = False;
    new->curves.isEmptyData = False;
    new->curves.drawBuffer = 0;
    new->curves.index = NULL;
    new->curves.x_data = NULL;
    new->curves.y_list = NULL;
    new->curves.scaleFunction = NULL;
    new->curves.updateMarkLine = NULL;
    new->curves.scaleFormular = NULL;
    new->curves.current_x_axis = 0;
    new->curves.num_variable = 0;
        
    /* register symbol patterns */
    for (i = 0; i < NUMMARK; i++) {
      register_pattern(symbol[i].name, symbol[i].bitmap, symbol_width, 
                       symbol_height);
      new->curves.markTile[i] = XmGetPixmap(XtScreen(new), symbol[i].name,
                                            new->primitive.foreground,
                                            new->core.background_pixel);
    }
                       
    /* Default plotting boundaries */
    CurvesSetContents((Widget)new, NULL, 0, CURVES_RESCALE, True);

    /* Set size dependent items */
    new->curves.drawBuffer = 0;
    resize(new);
}

/*
** Widget destroy method
*/
static void destroy(CurvesWidget w)
{
    int i;
    Display *dpy = XtDisplay(w);

    for (i = 0; i < NUMLINE; i++) {
      XFreeGC(dpy, w->curves.linesGC[i]);
      XFreeGC(dpy, w->curves.legendLineGC[i]);
    }
    XFreeGC (dpy, w->curves.gc);
    XFreeGC (dpy, w->curves.contentsGC);
    XtReleaseGC ((Widget) w, w->curves.legendGC);
    if (w->curves.font != NULL)
    	XmFontListFree(w->curves.font);

    if (w->curves.data != NULL) {
      for (i = 0; i < w->curves.num_variable; i++) {
         if ((w->curves.data+i)->points != NULL)
           XtFree((char *)(w->curves.data+i)->points);
         if ((w->curves.data+i)->variable_name != NULL)
           XtFree((char *)(w->curves.data+i)->variable_name);
         if ((w->curves.store_data+i)->points != NULL)
           XtFree((char *)(w->curves.store_data+i)->points);
         if ((w->curves.store_data+i)->variable_name != NULL)
           XtFree((char *)(w->curves.store_data+i)->variable_name);
      }
      XtFree((char *)w->curves.data);
      XtFree((char *)w->curves.store_data);
    }
    if (w->curves.xAxisLabel != NULL)
      XmStringFree(w->curves.xAxisLabel);
    if (w->curves.yAxisLabel != NULL)
      XmStringFree(w->curves.yAxisLabel);
    if (w->curves.x_data != NULL)
      XtFree((char *)w->curves.x_data);
    if (w->curves.index != NULL)
      XtFree((char *)w->curves.index);
    if (w->curves.y_list != NULL)
      XtFree((char *)w->curves.y_list);
    if (w->curves.updateMarkLine != NULL)
      XtFree((char *)w->curves.updateMarkLine); 
    for (i = 0; i < w->curves.num_variable; i++) {
      freeScalePtr(w->curves.scaleFunction+i);
    }
    XtFree((char *)w->curves.scaleFunction);
    XtRemoveAllCallbacks ((Widget) w, XmNresizeCallback);
    XtRemoveAllCallbacks ((Widget) w, XmNbtn2Callback);
    XtRemoveAllCallbacks ((Widget) w, XmNbtn3Callback);
    XtRemoveAllCallbacks ((Widget) w, XmNredisplayCallback);
}

/*
** Widget resize method
*/
static void resize(CurvesWidget w)
{
    XFontStruct *fs = getFontStruct(w->curves.font);
    int borderWidth =
    	w->primitive.shadow_thickness + w->primitive.highlight_thickness;
    XRectangle clipRect;
    int i, n, current_y, legendWidth, legendHeight, string_len; 
    int text_x, line_x_end;
    CurveStruct *curve;

    /* resize the drawing buffer, an offscreen pixmap for smoother animation */
    updateBufferAllocation(w); 

    /* calculate the area of the widget where contents can be drawn */
    w->curves.xMin = borderWidth;
    w->curves.yMin = borderWidth;
    w->curves.xMax = w->core.width - borderWidth;
    w->curves.yMax = w->core.height - borderWidth;
    
    legendHeight = 0;
    if ((w->curves.showLegend) && (w->curves.y_list != NULL)) { 
      if ((n = strlen(w->curves.y_list)) != 0) {
        legendWidth = w->curves.xMin + GAP;
        legendHeight = LEGEND_HEIGHT;
        for (i = 0; i < n; i++) {
          current_y = (int)(w->curves.y_list[i] - 'A');
          curve = w->curves.data+current_y;
          string_len = strlen(curve->variable_name);
          string_len = XTextWidth(fs, curve->variable_name, string_len);
          text_x = legendWidth;
          line_x_end = text_x + string_len + GAP*3 + 70;
          if (line_x_end > (w->curves.xMax - GAP)) {
            text_x = w->curves.xMin + GAP;
            line_x_end = text_x + string_len + GAP*3 + 70;
            legendWidth = text_x;
            legendHeight += LEGEND_HEIGHT;
          }
          legendWidth = line_x_end + LEGEND_HEIGHT;
        }
      }
      legendHeight += GAP;
    }
                         
    /* calculate positions for the axes and contents depending on whether
       axis labels are specified, and the measurements of the current font */
    if (w->curves.yAxisLabel != NULL)
    	w->curves.yEnd = w->curves.yMin + fs->ascent + fs->descent + 
    	                 TOP_MARGIN;
    else
    	w->curves.yEnd = VAxisEndClearance(fs) + fs->ascent/2 + TOP_MARGIN;
    w->curves.axisTop = w->curves.yEnd - VAxisEndClearance(fs);
    if (w->curves.xAxisLabel != NULL) {
    	w->curves.axisBottom = w->curves.yMax - BOTTOM_MARGIN - fs->ascent -
    				fs->descent - X_LABEL_MARGIN - legendHeight;
        w->curves.legendHeight = w->curves.axisBottom + fs->ascent +
                                 X_LABEL_MARGIN + fs->descent + GAP;
    }
    else {
    	w->curves.axisBottom = w->curves.yMax - fs->ascent/2 - 
                               BOTTOM_MARGIN - legendHeight;
        w->curves.legendHeight = w->curves.axisBottom + fs->ascent + GAP;
    }
    w->curves.yOrigin = w->curves.axisBottom - HAxisHeight(fs);
    w->curves.axisLeft = w->curves.xMin + LEFT_MARGIN;
    w->curves.xOrigin = w->curves.axisLeft + VAxisWidth(fs);
    w->curves.axisRight = w->curves.xMax - RIGHT_MARGIN;
    w->curves.xEnd = w->curves.axisRight - HAxisEndClearance(fs);

    /* set drawing gc to clip drawing before motif shadow and highlight */
    clipRect.x = borderWidth;
    clipRect.y = borderWidth;
    clipRect.width = w->core.width - 2 * borderWidth;
    clipRect.height = w->core.height - 2 * borderWidth;
    XSetClipRectangles(XtDisplay(w),w->curves.gc,0,0,&clipRect, 1,Unsorted);
    
    /* set plot contents gc to clip drawing at the edges */
    clipRect.x = w->curves.xOrigin;
    clipRect.y = w->curves.yEnd;
    clipRect.width = w->curves.xEnd - w->curves.xOrigin;
    clipRect.height = w->curves.yOrigin - w->curves.yEnd;
    XSetClipRectangles(XtDisplay(w), w->curves.contentsGC, 0, 0, &clipRect,
    		       1, Unsorted);
    for (i = 0; i < NUMLINE; i++) {
      XSetClipRectangles(XtDisplay(w), w->curves.linesGC[i], 0, 0, 
                         &clipRect, 1, Unsorted);
    }
    
    /* call the resize callback */
    if (XtIsRealized((Widget)w))
    	XtCallCallbacks((Widget) w, XmNresizeCallback, NULL);
}

/*
** Widget redisplay method
*/
static void redisplay(CurvesWidget w, XEvent *event, Region region)
{
    /* Draw the Motif required shadows and highlights */
    if (w->primitive.shadow_thickness > 0) {
	_XmDrawShadow (XtDisplay(w), XtWindow(w), 
		       w->primitive.bottom_shadow_GC,
		       w->primitive.top_shadow_GC,
                       w->primitive.shadow_thickness,
                       w->primitive.highlight_thickness,
                       w->primitive.highlight_thickness,
                       w->core.width - 2 * w->primitive.highlight_thickness,
                       w->core.height-2 * w->primitive.highlight_thickness);
    }
    if (w->primitive.highlighted)
	_XmHighlightBorder((Widget)w);
    else if (_XmDifferentBackground((Widget)w, XtParent(w)))
	_XmUnhighlightBorder((Widget)w);
    
    /* draw the contents of the curves widget */
    redisplayContents(w, X_SCREEN, REDRAW_ALL);
}

/*
** Widget setValues method
*/
static Boolean setValues(current, request, new)
     CurvesWidget current, request, new;
{
    Boolean redraw = False;
    Display *dpy = XtDisplay(new);

    /* If the foreground or background color has changed, change the GCs */
    if (new->core.background_pixel !=current->core.background_pixel) {
    	XSetForeground(dpy, new->curves.gc, new->primitive.foreground);
    	redraw = TRUE;
    }
    if (new->primitive.foreground != current->primitive.foreground) {
    	XSetBackground(dpy, new->curves.gc, new->core.background_pixel);
    	redraw = TRUE;
    }

    /* if double buffering changes, allocate or deallocate offscreen pixmap */
    if (new->curves.doubleBuffer != current->curves.doubleBuffer) {
    	updateBufferAllocation(new);
    	redraw = TRUE;
    }

    /* if log scaling changes, verify data, reset dragging, and redraw */
    if   (new->curves.xLogScaling != current->curves.xLogScaling ||
    	  new->curves.yLogScaling != current->curves.yLogScaling) {
    	verifyDataForLogScaling(new);
    	ResetAxisDragging(&new->curves.dragState);
    	redraw = TRUE;
    }

    /* if labels are changed, free the old ones and copy the new ones */
    if (new->curves.xAxisLabel != current->curves.xAxisLabel) {
    	if (current->curves.xAxisLabel != NULL)
    	    XmStringFree(current->curves.xAxisLabel);
    	new->curves.xAxisLabel = XmStringCopy(new->curves.xAxisLabel);
    }
    if (new->curves.yAxisLabel != current->curves.yAxisLabel) {
    	if (current->curves.yAxisLabel != NULL)
    	    XmStringFree(current->curves.yAxisLabel);
    	new->curves.yAxisLabel = XmStringCopy(new->curves.yAxisLabel);
    }

    /* if highlight thickness or shadow thickness changed, resize and redraw */
    if  ((new->primitive.highlight_thickness != 
          current->primitive.highlight_thickness) ||
         (new -> primitive.shadow_thickness !=
          current->primitive.shadow_thickness)) {
    	redraw = TRUE;
        resize (new);
    }
    return redraw; 
} 

/*
** Button press and button motion action proc.
*/
static void motionAP(CurvesWidget w, XEvent *event, char *args, int n_args)
{
    int chgdArea, redrawArea = REDRAW_NONE;

    if (event->type == ButtonPress)
    	XmProcessTraversal((Widget)w, XmTRAVERSE_CURRENT);

    if (event->type == MotionNotify)
    	compressMotionEvents(XtDisplay(w), event);
    
    chgdArea = DragAxes(event, w->curves.xOrigin, w->curves.xEnd, w->curves.yOrigin,
    	w->curves.yEnd, w->curves.axisLeft, w->curves.axisTop, w->curves.axisBottom,
    	w->curves.axisRight, w->curves.minXData, w->curves.maxXData, w->curves.minYData,
    	w->curves.maxYData, w->curves.xLogScaling, w->curves.yLogScaling,
    	&w->curves.minXLim, &w->curves.maxXLim, &w->curves.minYLim, &w->curves.maxYLim,
    	&w->curves.dragState, &w->curves.xDragStart, &w->curves.yDragStart);
    if (chgdArea & DA_REDRAW_H_AXIS) redrawArea |= REDRAW_H_AXIS;
    if (chgdArea & DA_REDRAW_V_AXIS) redrawArea |= REDRAW_V_AXIS;
    if (chgdArea & DA_REDRAW_CONTENTS) redrawArea |= REDRAW_CONTENTS;

    redisplayContents(w, X_SCREEN, redrawArea);
}


/*
** Button up action proc.
*/
static void btnUpAP(CurvesWidget w, XEvent *event, char *args, int n_args)
{
    ResetAxisDragging(&w->curves.dragState);
}

static void btn2AP(CurvesWidget w, XEvent *event, char *args, int n_args)
{
    CurvesCallbackStruct cbStruct;

    XmProcessTraversal((Widget)w, XmTRAVERSE_CURRENT);

    /* Just call the callback */
    cbStruct.reason = XmCR_INPUT;
    cbStruct.event = event;
    XtCallCallbacks((Widget) w, XmNbtn2Callback, &cbStruct);
}

static void btn3AP(CurvesWidget w, XEvent *event, char *args, int n_args)
{    
    CurvesCallbackStruct cbStruct;

    XmProcessTraversal((Widget)w, XmTRAVERSE_CURRENT);

    /* Just call the callback */
    cbStruct.reason = XmCR_INPUT;
    cbStruct.event = event;
    XtCallCallbacks((Widget) w, XmNbtn3Callback, &cbStruct);
}


/*
** merge: merge routine for merge sort
*/

static void merge(CurvesWidget w, float *x, int firstA, int firstB,
                                            int secondA, int secondB) {
 int indexA, indexB, indexC, *index;
 int a_index, b_index;
 
 index = (int *)XtMalloc(sizeof(int)*(secondB-firstA+1));
 indexA = firstA; indexB = secondA; indexC = 0;
 while (indexA <= firstB && indexB <=secondB) {
   a_index = w->curves.index[indexA];
   b_index = w->curves.index[indexB];
   if (x[a_index] < x[b_index]) {
     index[indexC] = a_index;
     indexA++;
   }
   else {
     index[indexC] = b_index;
     indexB++;
   }
   indexC++;
 }
 
 if (indexA > firstB)
   for (;indexB <= secondB; indexC++, indexB++) 
     index[indexC] = w->curves.index[indexB];
 else
   for (;indexA <= firstB; indexA++, indexC++)
     index[indexC] = w->curves.index[indexA];
 for (indexC = 0, indexA = firstA; indexA <=  secondB; indexA++, indexC++)
   w->curves.index[indexA] = index[indexC];
 XtFree((char *)index);
}  


/*
** sort_x: it is a routine for merge sort algorithm
**      x : an array to be sroted;
**  first : first index of x;
**   last : last index of x;
*/

static void sort_x(CurvesWidget w, float *x, int first, int last) {
  if (first < last) {
    sort_x(w, x, first, (first+last)/2);
    sort_x(w, x, (first+last)/2+1, last);
    merge(w, x, first, (first+last)/2, (first+last)/2+1, last);
  }
}

 
/*
** max_min
*/

double max_min(float *list, int n, double *min) {
  int i;
  double max;

  max = list[0];
  *min = list[0];

  for (i = 1; i < n; i++) {
    if (max < list[i]) max = list[i];
    if (*min > list[i]) *min = list[i];
  }

  return max;
} /* max_min */



/*
** DataRange 
*/

static void DataRange(w, maxX, minX, maxY, minY, sortX)
          CurvesWidget w;
          double *maxX, *minX, *maxY, *minY;
          Boolean sortX;
{
  float *maxList, *minList;
  double trash;
  int i, n, nn, current_y, current_x;

  n = strlen(w->curves.y_list);
  if (n == 0) {
    w->curves.isEmptyData = True;
    *maxX = 1; *minX = 0; *maxY = 1; *minY = 0;
    return;
  }
  maxList = (float *)XtMalloc(sizeof(float)*n);
  minList = (float *)XtMalloc(sizeof(float)*n);
  if (sortX) {                    /* when scaling, do not need to sort X data */
    if (w->curves.index != NULL) 
      XtFree((char *)w->curves.index);
    w->curves.index = (int *)XtMalloc(sizeof(int)*(w->curves.max_point));
    for (i = 0; i < w->curves.max_point; i++)
      w->curves.index[i] = i;
    if (w->curves.x_data != NULL)
      XtFree((char *)w->curves.x_data);
    w->curves.x_data = (float *)XtMalloc(sizeof(float)*(w->curves.max_point));
  }
  current_x = w->curves.current_x_axis;
  if (w->curves.isTimeSeries) {
    *minX = 1.;
    *maxX = w->curves.max_point;
    for (i = 0; i < w->curves.max_point; i++) {
      w->curves.x_data[i] = (float)(i+1);
    } /* for */
    w->curves.x_num_point = w->curves.max_point;
  }
  else { /* not time series */
    if ((w->curves.data+current_x)->num_point == 0) {
      w->curves.isEmptyData = True;
      *maxX = 1; *minX = 0; *maxY = 1; *minY = 0;
      return;
    }
    /* sort X data */
    if (sortX) {   
      *maxX = max_min((w->curves.data + current_x)->points,
                    (w->curves.data + current_x)->num_point, minX);
      for (i = 0; i < (w->curves.data+current_x)->num_point; i++)
        w->curves.x_data[i] = (w->curves.data+current_x)->points[i];
      w->curves.x_num_point = (w->curves.data+current_x)->num_point;
      sort_x(w, w->curves.x_data, 0, (w->curves.x_num_point-1));
    }
  } /* time series */

  n = strlen(w->curves.y_list);
  for (nn = 0, i = 0; i < n; i++) {
    current_y = w->curves.y_list[i] - 'A';
    if ((w->curves.data+current_y)->num_point > 0) {
      maxList[nn] = (float)max_min((w->curves.data + current_y)->points,
                         (w->curves.data + current_y)->num_point,
                         &trash);
      minList[nn++] = (float)trash;
    }  
  } /* for */
  if (nn == 0) {
    w->curves.isEmptyData = True;
    return;
  }
  *maxY = max_min(maxList, nn, &trash);
  trash = max_min(minList, nn, minY);

  XtFree((char *)maxList);
  XtFree((char *)minList);
} /* DataRange */

/*
** CurvesSetContents
*/
void CurvesSetContents(Widget w, CurveStruct *curves, int nCurves,
                 int rescale, Boolean isTimeSeries)
{
    CurvesWidget sw = (CurvesWidget)w;
    int redrawArea = REDRAW_NONE;
    int i, j, k, len, max_point = 0;
    double minX, minY, maxX, maxY;
    char string[2];
    CurveStruct *curve, *store_curve;
    
    /* Free the previous data */
    if (sw->curves.num_variable != 0) {
      for (i = 0; i < sw->curves.num_variable; i++) 
        if (sw->curves.data != NULL && (sw->curves.data+i)->points !=NULL)
          XtFree((char *)(sw->curves.data+i)->points);
      if (sw->curves.data != NULL)
    	XtFree((char *)sw->curves.data);
      for (i = 0; i < sw->curves.num_variable; i++) 
        if (sw->curves.store_data != NULL && 
                   (sw->curves.store_data+i)->points !=NULL)
          XtFree((char *)(sw->curves.store_data+i)->points);
      if (sw->curves.store_data != NULL)
    	XtFree((char *)sw->curves.store_data);
      if (sw->curves.y_list != NULL)
    	XtFree((char *)sw->curves.y_list);
      sw->curves.num_variable = 0;
    }
    
    /* Copy in the new data, if any, and calculate the min and max values */
    sw->curves.isTimeSeries = isTimeSeries;
    if ((isTimeSeries && nCurves == 1) || ((!isTimeSeries) && nCurves == 2))
      sw->curves.showLegend = False;
    sw->curves.num_variable = nCurves;
    sw->curves.current_x_axis = 0;
    if (nCurves == 0) {
    	minX = minY = 0.;
    	maxX = maxY = 1.;
    } else {
    	/* allocate memory and copy the data */
    	sw->curves.isEmptyData = False;
        sw->curves.y_list = (char *)XtMalloc(sizeof(char)*nCurves+1);
        strcpy(sw->curves.y_list, "");
        string[1] = '\0';
        k = isTimeSeries ? 0 : 1;
        for (i = k; i < nCurves; i++) {
          string[0] = i + 'A';
          strcat(sw->curves.y_list, string);
        }
         
    	sw->curves.data = 
          (CurveStruct *)XtMalloc(sizeof(CurveStruct) * nCurves);
    	sw->curves.store_data = 
          (CurveStruct *)XtMalloc(sizeof(CurveStruct) * nCurves);
        store_curve = sw->curves.store_data;
        curve=sw->curves.data;
        for(i=0; i<nCurves; curve++, store_curve++, curves++, i++) {
          len = strlen(curves->variable_name);
          curve->variable_name = (char *)XtMalloc(sizeof(char)*len+1);
          strcpy(curve->variable_name, curves->variable_name);
          curve->num_point = curves->num_point;
          curve->options = curves->options;
          store_curve->variable_name = (char *)XtMalloc(sizeof(char)*len+1);
          strcpy(store_curve->variable_name, curves->variable_name);
          store_curve->num_point = curves->num_point;
          store_curve->options = curves->options;
          if (max_point < curve->num_point)
            max_point = curve->num_point;
          if (curve->num_point ==  0) {
            curve->points = NULL;
            store_curve->points = NULL;
          }
          else {
            sw->curves.isEmptyData = False;
            curve->points = (float *)XtMalloc(sizeof(float)*curves->num_point);
            store_curve->points = 
                     (float *)XtMalloc(sizeof(float)*curves->num_point);
            for (j = 0; j < curves->num_point; j++) {
              curve->points[j] = curves->points[j];
              store_curve->points[j] = curves->points[j];
            }
          }
        }

	/* Update the mark and line styles */
    	if (sw->curves.updateMarkLine == NULL) {
          sw->curves.updateMarkLine = (int *)XtMalloc(sizeof(int)*nCurves*2);
          for (j = 0, i = 0; i < nCurves; i++) {
            if ((sw->curves.data+i)->options == CURVE_NO_OPTIONS) {
              if ( j > XtNumber(lines) -2)
        	j = 0;
              sw->curves.updateMarkLine[i*2] = 
                                    (sw->curves.data+i)->mark_num = NUMMARK - 1;
              sw->curves.updateMarkLine[i*2+1] = 
                          (sw->curves.data+i)->line_num = j;
              j++;
            }
          }
          if (!isTimeSeries) 
            sw->curves.updateMarkLine[3] = (sw->curves.data+1)->line_num = 0; 
        }
        for (i = 0; i < nCurves; i++) {
          (sw->curves.data+i)->mark_num = sw->curves.updateMarkLine[i*2];
          (sw->curves.data+i)->line_num = sw->curves.updateMarkLine[i*2+1];   
        }
        
        /* Calculate the range of the data */
        sw->curves.max_point = max_point;
        DataRange(sw, &maxX, &minX, &maxY, &minY, True);
    } /* if nCurves == 0 */
    resize(sw);    
   
    if (maxX == minX) {maxX += 1.; minX -= 1.;}
    if (maxY == minY) {maxY += 1.; minY -= 1.;}

    /* recalculate scale and limits for widget */
    switch (rescale) {
      case CURVES_RESCALE:
        sw->curves.maxXData = sw->curves.maxXLim = maxX;
        sw->curves.minXData = sw->curves.minXLim = minX;
        sw->curves.maxYData = sw->curves.maxYLim = maxY;
        sw->curves.minYData = sw->curves.minYLim = minY;
        redrawArea = REDRAW_CONTENTS | REDRAW_H_AXIS | REDRAW_V_AXIS;
        break;
      case CURVES_NO_RESCALE :
        redrawArea = REDRAW_CONTENTS;
        if (maxX > sw->curves.maxXData) {
          sw->curves.maxXData = maxX;
          redrawArea |= REDRAW_H_AXIS;
        }
        if (minX < sw->curves.minXData) {
          sw->curves.minXData = minX;
          redrawArea |= REDRAW_H_AXIS;
        }
        if (maxY > sw->curves.maxYData) {
          sw->curves.maxYData = maxY;
          redrawArea |= REDRAW_V_AXIS;
        }
        if (minY < sw->curves.minYData) {
          sw->curves.minYData = minY;
          redrawArea |= REDRAW_V_AXIS;
        }
      case CURVES_RESCALE_AT_MAX :
	redrawArea = REDRAW_CONTENTS;
	minX = dMin(sw->curves.minXData, minX);
	maxX = dMax(sw->curves.maxXData, maxX);
	minY = dMin(sw->curves.minYData, minY);
	maxY = dMax(sw->curves.maxYData, maxY);
    	if (sw->curves.maxXData != maxX || sw->curves.minXData != minX)
    	    redrawArea |= REDRAW_H_AXIS;
    	if (sw->curves.maxYData != maxY || sw->curves.minYData != minY)
    	    redrawArea |= REDRAW_V_AXIS;
    	if (sw->curves.maxXData == sw->curves.maxXLim)
    	    sw->curves.maxXLim = maxX;
    	if (sw->curves.minXData == sw->curves.minXLim)
    	    sw->curves.minXLim = minX;
    	if (sw->curves.maxYData == sw->curves.maxYLim)
    	    sw->curves.maxYLim = maxY;
    	if (sw->curves.minYData == sw->curves.minYLim)
    	    sw->curves.minYLim = minY;
    	sw->curves.maxXData = maxX; sw->curves.minXData = minX;
   	sw->curves.maxYData = maxY; sw->curves.minYData = minY;
      } /* switch */

     if (sw->curves.scaleFunction == NULL) {
       if (nCurves != 0) {
         sw->curves.scaleFunction = (ScalePtr *)XtMalloc(sizeof(ScalePtr) * 
                                   nCurves);
         for (i = 0; i < nCurves; i++)
         *(sw->curves.scaleFunction+i) = NULL;
       }
    }
    else {
      if (nCurves != 0) {
        if (XtIsRealized(w)) {
          k = isTimeSeries ? 0 : 1;
          j = sw->curves.current_scaling;
          for (i = k; i < sw->curves.num_variable; i++) {
            sw->curves.current_scaling = i - k;
            if (sw->curves.scaleFormular != NULL)  
              draw_scale_formular(sw);
          }
          sw->curves.current_scaling = j;
          scaling_treat_apply(sw, False);
        }
        return;
      }
    }     
    
    /* if log scaling was requested, make sure data is log scaleable */
    verifyDataForLogScaling(sw);
    
    /* redraw the widget with the new data */
    if (XtIsRealized(w)) {
        redisplayContents(sw, X_SCREEN, redrawArea);
    }
} /* CurvesSetContents */

/*
** CurvesSetVisibleRange, CurvesGetVisibleRange
**
** Set (Get) the range of data that is visible.  minXLim, minYLim, maxXLim, and
** maxYLim specify the endpoints of the x and y axes.  CurvesSetVisibleRange
** unlike the widgets interactive rescaling routines, can zoom out past the
** actual minimum and maximum data points.
*/
void CurvesSetVisibleRange(Widget w, double minXLim, double minYLim,
			 double maxXLim, double maxYLim)
{
    CurvesWidget sw = (CurvesWidget)w;
    double minX, minY, maxX, maxY;
    
    /* calculate the actual range of the data */
    DataRange(sw, &maxX, &minX, &maxY, &minY, True);

    /* allow user to zoom beyond the range of the data */
    sw->curves.maxXData = dMax(maxXLim, maxX);
    sw->curves.minXData = dMin(minXLim, minX);
    sw->curves.maxYData = dMax(maxYLim, maxY);
    sw->curves.minYData = dMin(minYLim, minY);

    /* Set the range */
    sw->curves.minXLim = minXLim;
    sw->curves.maxXLim = maxXLim;
    sw->curves.minYLim = minYLim;
    sw->curves.maxYLim = maxYLim;
    
    /* if log scaling was requested, make sure new range is log scaleable */
    verifyDataForLogScaling(sw);

    /* redraw if the widget is realized */
    if (XtIsRealized(w)){
    	redisplayContents(sw, X_SCREEN,
    		REDRAW_V_AXIS | REDRAW_H_AXIS | REDRAW_CONTENTS);
    }
}
void CurvesGetVisibleRange(Widget w, double *minXLim, double *minYLim,
			 double *maxXLim, double *maxYLim)
{
    *minXLim = ((CurvesWidget)w)->curves.minXLim;
    *maxXLim = ((CurvesWidget)w)->curves.maxXLim;
    *minYLim = ((CurvesWidget)w)->curves.minYLim;
    *maxYLim = ((CurvesWidget)w)->curves.maxYLim;
}

/*
**
** Zoom in and out by ZOOM_FACTOR.  Zoom in is centered on the current
** center of the plot.
*/
void CurvesZoomOut(Widget w)
{
    CurvesWidget sw = (CurvesWidget)w;
    int xLogScaling=sw->curves.xLogScaling, yLogScaling=sw->curves.yLogScaling;
    double xOffset, yOffset, newMaxXLim, newMinXLim, newMaxYLim, newMinYLim;
    double minXLim, maxXLim, minYLim, maxYLim;
    int redrawArea = REDRAW_NONE;
    
    /* if log scaling was requested, express limits in log coordinates */
    minXLim = xLogScaling ? log10(sw->curves.minXLim) : sw->curves.minXLim;
    maxXLim = xLogScaling ? log10(sw->curves.maxXLim) : sw->curves.maxXLim;
    minYLim = yLogScaling ? log10(sw->curves.minYLim) : sw->curves.minYLim;
    maxYLim = yLogScaling ? log10(sw->curves.maxYLim) : sw->curves.maxYLim;

    /* Calculate a suitable offset to reverse a zoom in by ZOOM_FACTOR */
    xOffset = (maxXLim - minXLim) * (ZOOM_FACTOR/(1.-ZOOM_FACTOR)) / 2;
    yOffset = (maxYLim - minYLim) * (ZOOM_FACTOR/(1.-ZOOM_FACTOR)) / 2;

    /* widen the plotting limits by the offsets calculated above,
       stopping when the limits reach the limits of the data */
    newMaxXLim = dMin(sw->curves.maxXData,
    	    xLogScaling ? pow(10., maxXLim + xOffset) : maxXLim + xOffset);
    newMinXLim = dMax(sw->curves.minXData,
    	    xLogScaling ? pow(10., minXLim - xOffset) : minXLim - xOffset);
    newMaxYLim = dMin(sw->curves.maxYData,
    	    yLogScaling ? pow(10., maxYLim + yOffset) : maxYLim + yOffset);
    newMinYLim = dMax(sw->curves.minYData,
    	    yLogScaling ? pow(10., minYLim - yOffset) : minYLim - yOffset);
    
    /* Tell widget to redraw, and what parts, if limits have changed */
    if (newMaxXLim != maxXLim || newMinXLim != minXLim)
    	redrawArea |= REDRAW_H_AXIS | REDRAW_CONTENTS;
    if (newMaxYLim != maxYLim || newMinYLim != minYLim)
    	redrawArea |= REDRAW_V_AXIS | REDRAW_CONTENTS;
    
    /* Set the new limits */
    sw->curves.maxXLim = newMaxXLim;
    sw->curves.minXLim = newMinXLim;
    sw->curves.maxYLim = newMaxYLim;
    sw->curves.minYLim = newMinYLim;
    
    /* redraw if the widget is realized */
    if (XtIsRealized(w))
    	redisplayContents(sw, X_SCREEN, redrawArea);
}

void CurvesZoomIn(Widget w)
{
    CurvesWidget sw = (CurvesWidget)w;
    int xLogScaling = sw->curves.xLogScaling, yLogScaling = sw->curves.yLogScaling;
    double xOffset, yOffset;
    double minXLim, maxXLim, minYLim, maxYLim;

    /* if log scaling was requested, express limits in log coordinates */
    minXLim = xLogScaling ? log10(sw->curves.minXLim) : sw->curves.minXLim;
    maxXLim = xLogScaling ? log10(sw->curves.maxXLim) : sw->curves.maxXLim;
    minYLim = yLogScaling ? log10(sw->curves.minYLim) : sw->curves.minYLim;
    maxYLim = yLogScaling ? log10(sw->curves.maxYLim) : sw->curves.maxYLim;
    
    /* Calculate offsets for limits of displayed data to zoom by ZOOM_FACTOR */
    xOffset = (maxXLim - minXLim) * ZOOM_FACTOR / 2;
    yOffset = (maxYLim - minYLim) * ZOOM_FACTOR / 2;

    /* Narrow the plotting limits by the offsets calculated above */
    maxXLim -= xOffset;
    minXLim += xOffset;
    maxYLim -= yOffset;
    minYLim += yOffset;
    
    /* Set the new limits */
    sw->curves.maxXLim = xLogScaling ? pow(10.,maxXLim) : maxXLim;
    sw->curves.minXLim = xLogScaling ? pow(10.,minXLim) : minXLim;
    sw->curves.maxYLim = yLogScaling ? pow(10.,maxYLim) : maxYLim;
    sw->curves.minYLim = yLogScaling ? pow(10.,minYLim) : minYLim;
   
    /* redraw if the widget is realized */
    if (XtIsRealized(w))
    	redisplayContents(sw, X_SCREEN,
    		REDRAW_V_AXIS | REDRAW_H_AXIS | REDRAW_CONTENTS);
}

/*
** PrintContents
*/
void CurvesPrintContents(Widget w, char *psFileName)
{
    CurvesWidget sw = (CurvesWidget)w;

    PSFile = OpenPS(psFileName, sw->core.width, sw->core.height);
    PSHeight = sw->core.height;
    PSWidth = sw->core.width;
    if (PSFile != NULL) {
	redisplayContents(sw, PS_PRINTER, REDRAW_ALL);
	EndPS();
    }    
}


/* 
** when data is empty, draw axes
*/

static void  drawEmptyAxis(display, drawBuf, gc, outDevice, xOrigin, yOrigin, 
                           xEnd, yEnd)
           Display *display;
           Drawable drawBuf;
           GC gc;
           int outDevice, xOrigin, yOrigin, xEnd, yEnd;
{
  if (outDevice == X_SCREEN) {
    XDrawLine(display, drawBuf, gc, xOrigin, yOrigin, xEnd, yOrigin);
    XDrawLine(display, drawBuf, gc, xOrigin, yOrigin, xOrigin, yEnd);    
  }
  else {
    fprintf(PSFile, "%d %d moveto %d %d lineto %d %d lineto \n",
                  x_X2PS(xOrigin), y_X2PS(yEnd), x_X2PS(xOrigin), 
                  y_X2PS(yOrigin), x_X2PS(xEnd), y_X2PS(yOrigin));
  }
}
    
    
/*
** Redisplays the contents part of the widget, without the motif shadows and
** highlights.
*/
static void redisplayContents(CurvesWidget w, int outDevice, int redrawArea)
{
    Display *display = XtDisplay(w);
    GC gc = w->curves.gc;
    XFontStruct *fs = getFontStruct(w->curves.font);
    Drawable drawBuf;
    int x_method;
 
    /* Save some energy if the widget isn't visible or no drawing requested */
    if ((outDevice==X_SCREEN && !w->core.visible) || redrawArea == REDRAW_NONE)
        return;

    /* Set destination for drawing commands, offscreen pixmap or window */
    if (w->curves.doubleBuffer)
    	drawBuf = w->curves.drawBuffer;
    else
    	drawBuf = XtWindow(w);

    /* Clear the drawing buffer or window only in the areas that have
       changed.  The other parts are still redrawn, but the net effect
       is that the unchanged areas do not flicker */
    if (outDevice == X_SCREEN) {
	XSetForeground(display, gc, w->core.background_pixel);
	if (redrawArea == REDRAW_ALL) {
	    XFillRectangle(display, drawBuf, gc, w->curves.xMin, w->curves.yMin,
    		     w->curves.xMax - w->curves.xMin, 
                     w->curves.yMax - w->curves.yMin);
	} else {
    	    if (redrawArea & REDRAW_V_AXIS)
		XFillRectangle(display, drawBuf, gc, w->curves.axisLeft,
                        w->curves.axisTop,
                        w->curves.xOrigin - w->curves.axisLeft,
    			w->curves.axisBottom - w->curves.axisTop);
    	    if (redrawArea & REDRAW_H_AXIS)
    		XFillRectangle(display, drawBuf, gc, w->curves.axisLeft,
    	    		w->curves.yOrigin + 1, 
                        w->curves.axisRight - w->curves.axisLeft,
    	    		w->curves.axisBottom - w->curves.yOrigin+1);
    	    if (redrawArea & REDRAW_CONTENTS)
    		XFillRectangle(display, drawBuf, gc, w->curves.xOrigin + 1,
    	   		w->curves.yEnd, w->curves.xEnd - w->curves.xOrigin,
    	   		w->curves.yOrigin - w->curves.yEnd);
            if (w->curves.showLegend && 
                (redrawArea & REDRAW_LEGEND))
                XFillRectangle(display, drawBuf, gc, w->curves.xMin + 1,
                        w->curves.axisBottom + X_LABEL_MARGIN + GAP * 2,
                        w->curves.xMax - w->curves.xMin,
                        w->curves.legendHeight);
	}
    }
    
    /* Draw the axes */
    XSetForeground(display, gc, w->primitive.foreground);
    x_method = (w->curves.isTimeSeries)? 
               (int)(w->curves.maxXData - w->curves.minXData) : 0;
    if (w->curves.isEmptyData) 
      drawEmptyAxis(display, drawBuf, gc, outDevice, w->curves.xOrigin,
         w->curves.yOrigin, w->curves.xEnd, w->curves.yEnd);
    else {
      DrawHorizontalAxis(display, drawBuf, gc, fs, outDevice,
	      w->curves.yOrigin, w->curves.xOrigin, w->curves.xEnd,
	      w->curves.minXData, w->curves.maxXData, w->curves.minXLim,
	      w->curves.maxXLim, w->curves.xLogScaling, x_method);
      DrawVerticalAxis(display, drawBuf, gc, fs, outDevice, w->curves.xOrigin,
      	      w->curves.xMin, w->curves.yOrigin, w->curves.yEnd,
              w->curves.minYData, w->curves.maxYData, w->curves.minYLim,
              w->curves.maxYLim, w->curves.yLogScaling);
    }
    /* Draw the axis labels */
    if (w->curves.xAxisLabel != NULL )
    	if (outDevice == X_SCREEN)
    	    XmStringDraw(display, drawBuf, w->curves.font, w->curves.xAxisLabel,
		gc, w->curves.xOrigin, w->curves.axisBottom + X_LABEL_MARGIN,
		w->curves.xEnd - w->curves.xOrigin, XmALIGNMENT_CENTER,
	     	XmSTRING_DIRECTION_L_TO_R, NULL);
    	else
    	  PSDrawXmString(display, drawBuf, w->curves.font, w->curves.xAxisLabel,
		gc, w->curves.xOrigin, w->curves.axisBottom + X_LABEL_MARGIN,
		w->curves.xEnd - w->curves.xOrigin, XmALIGNMENT_CENTER);
    if (w->curves.yAxisLabel != NULL)
      if (outDevice == X_SCREEN)
        XmStringDraw(display, drawBuf, w->curves.font, w->curves.yAxisLabel, gc,
    	  w->curves.xOrigin + Y_LABEL_MARGIN, w->curves.yMin + TOP_MARGIN,
	  w->curves.xEnd - w->curves.xOrigin, XmALIGNMENT_BEGINNING,
	     	XmSTRING_DIRECTION_L_TO_R, NULL);
    	else
    	  PSDrawXmString(display, drawBuf, w->curves.font, w->curves.yAxisLabel,
    	    gc, w->curves.xOrigin + Y_LABEL_MARGIN, w->curves.yMin + TOP_MARGIN,
	    w->curves.xEnd - w->curves.xOrigin, XmALIGNMENT_BEGINNING);
    /* Draw the contents of the plot */
    drawCurvesPlot(w, drawBuf, outDevice);
    /* For double buffering, now copy offscreen pixmap to screen */
    if (w->curves.doubleBuffer && outDevice == X_SCREEN)
    	XCopyArea(display, drawBuf, XtWindow(w), gc, 0, 0,
    		  w->core.width, w->core.height, 0, 0);
    
    /* Call the redisplay callback so an application which draws on the curves
       widget can refresh it's graphics */
    if (XtIsRealized((Widget)w) && outDevice == X_SCREEN)
    	XtCallCallbacks((Widget) w, XmNredisplayCallback, NULL);
}

/* 
** draw square 
*/

static void drawSquare(int x, int y) {
  int disp_x, disp_y;
  
  disp_x = x - ps_scale/2;
  disp_y = y + ps_scale/2;
  
  fprintf(PSFile, "%d %d translate\n", x_X2PS(disp_x), y_X2PS(disp_y));
  fprintf(PSFile, "0 0 moveto 0 %d lineto %d %d lineto %d 0 lineto ", 
                   ps_scale, ps_scale, ps_scale, ps_scale);
  fprintf(PSFile, "0 0 lineto\nclosepath stroke\n");
  fprintf(PSFile, "%d %d translate\n", -x_X2PS(disp_x), -y_X2PS(disp_y));
}

/*
** draw star
*/

static void drawStar(int x, int y) {
  float scale;
  
  scale = (float)ps_scale/2 *0.86;
  
  fprintf(PSFile, "%d %d translate\n", x_X2PS(x), y_X2PS(y));
  fprintf(PSFile, "%.3f 0 moveto %.3f 0 lineto ", -(float)ps_scale/2.,
                  (float)ps_scale/2.);
  fprintf(PSFile, "%.3f %.3f moveto %.3f %.3f lineto ", -(float)ps_scale/4., 
                  scale, (float)ps_scale/4., -scale);
  fprintf(PSFile, "%.3f %.3f moveto %.3f %.3f lineto stroke\n",
                  -(float)ps_scale/4., -scale, (float)ps_scale/4., scale);
  fprintf(PSFile, "0 0 moveto\n"); 
  fprintf(PSFile, "%d %d translate\n", -x_X2PS(x), -y_X2PS(y));
}

/*
** draw triangle
*/

static void drawTriangle(int x, int y) {
  int disp_x, disp_y;
  
  disp_x = x - ps_scale/2;
  disp_y = y + ps_scale/2;
  
  fprintf(PSFile, "%d %d translate\n", x_X2PS(disp_x), y_X2PS(disp_y));
  fprintf(PSFile, "0 0 moveto ");
  fprintf(PSFile,
          "%.3f %.3f lineto %.3f 0 lineto 0 0 lineto closepath fill stroke\n",
          (float)ps_scale/2., (float)ps_scale*0.86, (float)ps_scale);
  fprintf(PSFile, "%d %d translate\n", -x_X2PS(disp_x), -y_X2PS(disp_y));
}

/*
** draw circle
*/

static void drawCircle(int x, int y) {
 
  fprintf(PSFile, "%d %d translate\n", x_X2PS(x), y_X2PS(y));
  fprintf(PSFile, "%d 0 moveto\n", x_X2PS(ps_scale/2));
  fprintf(PSFile, "0 0 %d 0 360 arc stroke\n", ps_scale/2);
  fprintf(PSFile, "%d %d translate\n", -x_X2PS(x), -y_X2PS(y));
}


/*
** draw solid circle
*/

static void drawSolidCircle(int x, int y) {

  fprintf(PSFile, "%d %d translate\n", x_X2PS(x), y_X2PS(y));
  fprintf(PSFile, "%d 0 moveto\n", x_X2PS(ps_scale/2));
  fprintf(PSFile, "0 0 %d 0 360 arc fill stroke\n", ps_scale/2);
  fprintf(PSFile, "%d %d translate\n", -x_X2PS(x), -y_X2PS(y));
}

/*
** draw cross
*/

static void drawCross(int x, int y) {
  float scale;

  scale = (float)ps_scale / 2 * 0.707;
  fprintf(PSFile, "%d %d translate\n", x_X2PS(x), y_X2PS(y)); 
  fprintf(PSFile, "%.2f %.2f moveto %.2f %.2f lineto %.2f %.2f moveto \
                   %.2f %.2f lineto stroke\n",
                   -scale, -scale, scale, scale, -scale, scale, scale, -scale);
  fprintf(PSFile, "%d %d translate\n", -x_X2PS(x), -y_X2PS(y));   
}

/* 
** Obtain X Window related drawing parameters
** and massage them for PostScript printers
*/
static void getXParms(Display *display, GC gc,
		      unsigned short *red_ptr, unsigned short *green_ptr, 
      		      unsigned short *blue_ptr, double *linewidth)
{ 
    XGCValues values_ret;
    XColor ret_color;

#ifdef MOTIF10
    *red_ptr   = 0;
    *green_ptr = 0;
    *blue_ptr  = 0;
    *linewidth = .5;
#else
    XGetGCValues(display,gc,GCForeground | GCLineWidth ,&values_ret);
    /*XGetWindowAttributes(display,w,window_attributes);*/
    ret_color.pixel = values_ret.foreground;
    ret_color.flags = DoRed | DoGreen | DoBlue ;
    XQueryColor(display,/*window_attributes.colormap*/   /* Get color rgb */
        	DefaultColormap(display,0),&ret_color);
    *red_ptr   = ret_color.red;
    *green_ptr = ret_color.green;
    *blue_ptr  = ret_color.blue;
    *linewidth = values_ret.line_width + 0.5;  /* width = 0 dangerous for PS */
#endif
}


/*
** draw marks by using PostScript 
*/

static void drawMarks(display, gc, x, y, num_mark)
       Display *display;
       GC gc;
       int x, y, num_mark;
{
  double lw;
  unsigned short red, green, blue;
  
  /* set image drawing parameters from the contents of X graphics context */
  getXParms(display, gc, &red, &green, &blue, &lw);
  fprintf(PSFile, "%.2f %.2f %.2f setrgbcolor\n", (float)red/COLOR_FACTOR, 
                   (float)green/COLOR_FACTOR, (float)blue/COLOR_FACTOR);
  fprintf(PSFile, "%.2f setlinewidth\n", lw);
  
  /* draw mark */
  switch (num_mark) {
    case 0 : drawSquare(x, y); break;
    case 1 : drawStar(x, y); break;
    case 2 : drawTriangle(x, y); break;
    case 3 : drawCircle(x, y); break;
    case 4 : drawSolidCircle(x, y); break;
    case 5 : drawCross(x, y); break;
  }
}
               
    
/*
** draw one curve.
*/

static void drawOneCurve(CurvesWidget w, Drawable drawBuf, int outDevice,
                         float *x_data, float *y_data, int y_num_point,
                         int mark_num, int line_num) {
  int i, num_point;
  XSegment *segments; 
  int nSegs, lastPoint;
  float x1, y1, x2, y2;
  int line_index;
  GC gc = w->curves.contentsGC;
  Display *display = XtDisplay(w);
  int xLogScaling = w->curves.xLogScaling, yLogScaling = w->curves.yLogScaling;
  double minXData, minYData, minXLim, minYLim, maxXLim, maxYLim;
  double xScale, yScale, minXPix, maxYPix;

  /* if log scaling was requested, express limits in log coordinates */
  if (w->curves.xLogScaling) {
      minXData = log10(w->curves.minXData);
      minXLim = log10(w->curves.minXLim); maxXLim = log10(w->curves.maxXLim);
  } else {
      minXData = w->curves.minXData;
      minXLim = w->curves.minXLim; maxXLim = w->curves.maxXLim;
  }
  if (w->curves.yLogScaling) {
      minYData = log10(w->curves.minYData);
      minYLim = log10(w->curves.minYLim); maxYLim = log10(w->curves.maxYLim);
  } else {
      minYData = w->curves.minYData;
      minYLim = w->curves.minYLim; maxYLim = w->curves.maxYLim;
  }
  xScale = (w->curves.xEnd - w->curves.xOrigin) / (maxXLim - minXLim);
  yScale = (w->curves.yOrigin - w->curves.yEnd) / (maxYLim - minYLim);
  minXPix = w->curves.xOrigin - (minXLim - minXData) * xScale;
  maxYPix = w->curves.yOrigin + (minYLim - minYData) * yScale;

  num_point = (w->curves.x_num_point < y_num_point) ?
              w->curves.x_num_point : y_num_point;
  segments = (XSegment *)XtMalloc(sizeof(XSegment)*num_point);

  for (nSegs = 0, i = 0; i < num_point - 1; i++) {
    line_index = w->curves.index[i];
    x1 = xLogScaling ? log10(x_data[line_index]) : x_data[line_index];
    y1 = yLogScaling ? log10(y_data[line_index]) : y_data[line_index];
    line_index = w->curves.index[i+1];
    x2 = xLogScaling ? log10(x_data[line_index]) : x_data[line_index];
    y2 = yLogScaling ? log10(y_data[line_index]) : y_data[line_index];
    if (!((x1<minXLim && x2<minXLim) || (x1>maxXLim && x2>maxXLim) ||
    	  (y1<minYLim && y2<minYLim) || (y1>maxYLim && y2>maxYLim))) {
	segments[nSegs].x1 = (int)(minXPix + (x1 - minXData) * xScale);
	segments[nSegs].y1 = (int)(maxYPix - (y1 - minYData) * yScale);
	segments[nSegs].x2 = (int)(minXPix + (x2 - minXData) * xScale);
	segments[nSegs++].y2 = (int)(maxYPix - (y2 - minYData) * yScale);
    }
  }

  if (num_point == 1) {
    line_index = w->curves.index[0];
    x1 = xLogScaling ? log10(x_data[line_index]) : x_data[line_index];
    y1 = yLogScaling ? log10(y_data[line_index]) : y_data[line_index];
    segments[0].x1 = (int)(minXPix + (x1 - minXData) * xScale);
    segments[0].y1 = (int)(maxYPix - (y1 - minYData) * yScale);
    segments[0].x2 = (int)(minXPix + (x1 - minXData) * xScale);
    segments[0].y2 = (int)(maxYPix - (y1 - minYData) * yScale);
  }    
  if (outDevice == X_SCREEN) {
    if (mark_num < NUMMARK - 1) {
      for (i = 0; i < nSegs; i++)
        XCopyArea(display, w->curves.markTile[mark_num], drawBuf, gc, 0, 0,
                  symbol_width, symbol_height, segments[i].x1 - symbol_width/2,
                  segments[i].y1 - symbol_height / 2);
      lastPoint = (nSegs == 0) ? 0 : (nSegs-1);
      XCopyArea(display, w->curves.markTile[mark_num], drawBuf, gc, 0, 0,
              symbol_width,symbol_height,segments[lastPoint].x2-symbol_width/2,
              segments[lastPoint].y2 - symbol_height / 2);
    }
    
  if (line_num < NUMLINE - 1)
    XDrawSegments(display, drawBuf, w->curves.linesGC[line_num],
                  segments, nSegs);
  }
  else {
    if (mark_num < NUMMARK - 1) {
      for (i = 0; i < nSegs; i++)
        drawMarks(display, gc, segments[i].x1, segments[i].y1, mark_num);
    drawMarks(display, gc, segments[nSegs-1].x2, segments[nSegs-1].y2,
                mark_num);

    }
    if (line_num < NUMLINE - 1) 
      PSDrawDashedSegments(display, drawBuf, gc, segments, nSegs, 
                             lines[line_num].line_dashes_list, 0);
  }
  XtFree((char *)segments);
}

static void drawErrorBars(CurvesWidget w, Drawable drawBuf, int outDevice,
                         float *x_data, float *y_top_data,
                         float *y_bottom_data, int y_num_point) {
  int i, num_point;
  XSegment *segments; 
  int nSegs;
  float x, y1, y2;
  int line_index;
  GC gc = w->curves.contentsGC;
  Display *display = XtDisplay(w);
  int xLogScaling = w->curves.xLogScaling, yLogScaling = w->curves.yLogScaling;
  double minXData, minYData, minXLim, minYLim, maxXLim, maxYLim;
  double xScale, yScale, minXPix, maxYPix;
  int sx, sy1, sy2;

  /* if log scaling was requested, express limits in log coordinates */
  if (w->curves.xLogScaling) {
      minXData = log10(w->curves.minXData);
      minXLim = log10(w->curves.minXLim); maxXLim = log10(w->curves.maxXLim);
  } else {
      minXData = w->curves.minXData;
      minXLim = w->curves.minXLim; maxXLim = w->curves.maxXLim;
  }
  if (w->curves.yLogScaling) {
      minYData = log10(w->curves.minYData);
      minYLim = log10(w->curves.minYLim); maxYLim = log10(w->curves.maxYLim);
  } else {
      minYData = w->curves.minYData;
      minYLim = w->curves.minYLim; maxYLim = w->curves.maxYLim;
  }
  xScale = (w->curves.xEnd - w->curves.xOrigin) / (maxXLim - minXLim);
  yScale = (w->curves.yOrigin - w->curves.yEnd) / (maxYLim - minYLim);
  minXPix = w->curves.xOrigin - (minXLim - minXData) * xScale;
  maxYPix = w->curves.yOrigin + (minYLim - minYData) * yScale;

  num_point = (w->curves.x_num_point < y_num_point) ?
              w->curves.x_num_point : y_num_point;
  segments = (XSegment *)XtMalloc(sizeof(XSegment)*3*num_point);

  for (nSegs = 0, i = 0; i < num_point; i++) {
    line_index = w->curves.index[i];
    x = xLogScaling ? log10(x_data[line_index]) : x_data[line_index];
    y1 = yLogScaling ? log10(y_top_data[line_index]) : y_top_data[line_index];
    y2 = yLogScaling ? log10(y_bottom_data[line_index]) :
    		       y_bottom_data[line_index];
    if (!(x<minXLim || x>maxXLim ||
    	  (y1<minYLim && y2<minYLim) || (y1>maxYLim && y2>maxYLim))) {
	sx = (int)(minXPix + (x - minXData) * xScale);
	sy1 = (int)(maxYPix - (y1 - minYData) * yScale);
	sy2 = (int)(maxYPix - (y2 - minYData) * yScale);
	segments[nSegs].x1 = sx;
	segments[nSegs].y1 = sy1;
	segments[nSegs].x2 = sx;
	segments[nSegs++].y2 = sy2;
	segments[nSegs].x1 = sx - ERROR_BAR_WIDTH/2;
	segments[nSegs].y1 = sy1;
	segments[nSegs].x2 = sx + ERROR_BAR_WIDTH/2;
	segments[nSegs++].y2 = sy1;
	segments[nSegs].x1 = sx - ERROR_BAR_WIDTH/2;
	segments[nSegs].y1 = sy2;
	segments[nSegs].x2 = sx + ERROR_BAR_WIDTH/2;
	segments[nSegs++].y2 = sy2;

    }
  }

  if (outDevice == X_SCREEN) {
    XDrawSegments(display, drawBuf, w->curves.linesGC[PLAIN_LINE],
                  segments, nSegs);
  }
  else {
    PSDrawDashedSegments(display, drawBuf, gc, segments, nSegs, 
                             lines[PLAIN_LINE].line_dashes_list, 0);
  }
  XtFree((char *)segments);
}

/*
** draw legend
*/
static void drawLegend(CurvesWidget w, Drawable drawBuf, int outDevice) {
  int legendWidth;
  Display *display = XtDisplay(w);
  GC gc = w->curves.legendGC;
  int i, text_x, mark_x, line_x, line_x_end, y, n, current_y, maxLength;
  CurveStruct *curve;
  XFontStruct *fs = getFontStruct(w->curves.font);
  int gc_index;
  XSegment segment[1];
  XmString legendName;
  
  legendWidth = w->curves.xMin + GAP;
  y = w->curves.legendHeight;
  n = strlen(w->curves.y_list);
  
  for (i = 0; i < n; i++) {
    current_y = (int)(w->curves.y_list[i] - 'A');
    curve = w->curves.data+current_y;
    if (curve->options != CURVE_NO_OPTIONS)
      continue;
    maxLength = strlen(curve->variable_name);
    maxLength = XTextWidth(fs, curve->variable_name, maxLength);
    text_x = legendWidth;
    mark_x = text_x + maxLength + GAP;
    line_x = mark_x + GAP*2;
    line_x_end = line_x + 70;
    if (line_x_end > (w->curves.xMax - GAP)) {
      text_x = w->curves.xMin + GAP;
      mark_x = text_x + maxLength + GAP;
      line_x = mark_x + GAP*2;
      line_x_end = line_x + 70;
      legendWidth = text_x;
      y += LEGEND_HEIGHT;
    }
    legendWidth = line_x_end + LEGEND_HEIGHT;
      
    if (outDevice == X_SCREEN) {
      XDrawString(display, drawBuf, gc,
                  text_x, y + fs->ascent/2 - 3, 
                  curve->variable_name,
                  strlen(curve->variable_name));
      gc_index = curve->mark_num;
      if (gc_index < NUMMARK - 1) {
        XCopyArea(display, w->curves.markTile[gc_index], drawBuf,
                  gc, 0, 0, symbol_width, symbol_height,
                  mark_x, y - symbol_height / 2);
      }
      gc_index = curve->line_num;
      if (gc_index < NUMLINE - 1) {
        XDrawLine(display, drawBuf, w->curves.legendLineGC[gc_index], 
                  line_x, y, line_x_end, y);
                
      }
    }
    else { 
      legendName = XmStringCreateSimple(curve->variable_name); 
      PSDrawXmString(display, drawBuf, w->curves.font, legendName, gc, 
                     text_x, y-fs->descent-fs->ascent/2, 
                     XmStringLength(legendName), XmALIGNMENT_BEGINNING);
      if (curve->mark_num < NUMMARK - 1) 
         drawMarks(display, gc, mark_x, y, curve->mark_num);
      if (curve->line_num < NUMLINE - 1) {
         segment[0].x1 = line_x;
         segment[0].x2 = line_x_end;
         segment[0].y1 = segment[0].y2 = y;
         PSDrawDashedSegments(display, drawBuf, gc, segment, 1, 
                              lines[curve->line_num].line_dashes_list, 0);
      }
    }
  }
} /* draw legend */

           
/*
** draw curves
*/

static void drawCurvesPlot(CurvesWidget w, Drawable drawBuf, int outDevice){
  int i, current_y;
  int xMin = x_X2PS(w->curves.xOrigin), yMin = y_X2PS(w->curves.yEnd);
  int xMax = x_X2PS(w->curves.xEnd), yMax = y_X2PS(w->curves.yOrigin);
  
  if (w->curves.showLegend)
    drawLegend(w, drawBuf, outDevice);
  if (outDevice != X_SCREEN)
    fprintf(PSFile, "newpath %d %d moveto %d %d lineto %d %d lineto %d %d \
                     lineto %d %d lineto closepath clip newpath\n", xMin, 
                     yMax, xMin, yMin, xMax, yMin, xMax, yMax, xMin, yMax);
  for (i = 0; i < strlen(w->curves.y_list); i++){
    current_y = w->curves.y_list[i] - 'A';
    if ((w->curves.data+current_y)->num_point > 0) {
      if ((w->curves.data+current_y)->options == CURVE_NO_OPTIONS) {
	drawOneCurve(w, drawBuf, outDevice, w->curves.x_data,
                   (w->curves.data+current_y)->points,
                   (w->curves.data+current_y)->num_point,
                   (w->curves.data+current_y)->mark_num,
                   (w->curves.data+current_y)->line_num);                 
      } else if ((w->curves.data+current_y)->options == CURVE_TOP_ERROR) {
        drawErrorBars(w, drawBuf, outDevice, w->curves.x_data,
                   (w->curves.data+current_y)->points,
                   (w->curves.data+current_y+1)->points,
                   (w->curves.data+current_y)->num_point);
      }
    }
  }
} /* drawCurvesPlot */



/*
** For smoother animation, it is possible to allocate an offscreen pixmap
** and draw to that rather than directly into the window.  Unfortunately,
** it is too slow on many machines, so we have to give the user the choice
** whether to use it or not.  This routine reallocates the offscreen
** pixmap buffer when the window is resized, and when the widget is created
*/
static void updateBufferAllocation(CurvesWidget w)
{ 
    if (w->curves.drawBuffer)
    	XFreePixmap(XtDisplay(w), w->curves.drawBuffer);
    if (w->curves.doubleBuffer) {
    	w->curves.drawBuffer = XCreatePixmap(XtDisplay(w),
		DefaultRootWindow(XtDisplay(w)), w->core.width, w->core.height,
    	 	DefaultDepthOfScreen(XtScreen(w)));   
    } else {
    	w->curves.drawBuffer = 0;
    }
}

/*
** Get the XFontStruct that corresponds to the default (first) font in
** a Motif font list.  Since Motif stores this, it saves us from storing
** it or querying it from the X server.
*/
static XFontStruct *getFontStruct(XmFontList font)
{
#ifdef MOTIF10
    return font->font;
#else
    XFontStruct *fs;
    XmFontContext context;
    XmStringCharSet charset;

    XmFontListInitFontContext(&context, font);
    XmFontListGetNextFont(context, &charset, &fs);
    XmFontListFreeFontContext(context);
    XtFree(charset);
    return fs;
#endif
}

/*
** Checks that log scaling is possible with current data and current settings
** of xLogScaling and yLogScaling.  If it is not possible, this routine
** resets x or yLogScaling to False and takes no further action.  If the
** caller wants to take further action, it should test these flags.
*/
static void verifyDataForLogScaling(CurvesWidget w)
{
    if (w->curves.xLogScaling && w->curves.minXData <= 0.)
	w->curves.xLogScaling = False;
    if (w->curves.yLogScaling && w->curves.minYData <= 0.)
	w->curves.yLogScaling = False;
}

/* minimum and maximum of two doubles */
static double dMin(double d1, double d2)
{
    if (d2 < d1)
    	return d2;
    return d1;
}
static double dMax(double d1, double d2)
{
    if (d2 > d1)
    	return d2;
    return d1;
}

/*
** Motion compression means collecting MotionNotify events together to
** process only the event containing the current position of the
** mouse rather than all of the intermediate positions.
**
** Normally, motion compression is done automatically by X but for some
** mysterious reason that we have been unable to determine, something that
** this widget does disagrees with it and it has to be done explicitly.
*/
static void compressMotionEvents(Display *display, XEvent *event)
{
    while (XCheckTypedEvent(display, MotionNotify, event));
}

/*
** mark selection call back function
*/

static void mark_selection(w, data, callData)
    Widget w;
    CurvesWidget data;
    XmToggleButtonCallbackStruct *callData;
{
  int call_num, n, y, delta_y, current_y;
  Arg wargs[5];
  Dimension height, width;
  
  if (!callData->set) return;
  
    n = 0;
    XtSetArg(wargs[n], XtNwidth, &width); n++;
    XtSetArg(wargs[n], XtNheight, &height); n++;
    XtGetValues(data->curves.sample[0], wargs, n);
    XtSetArg(wargs[0], XmNuserData, &call_num);
    XtGetValues(w, wargs, 1);
    
    current_y = (int)(data->curves.y_list[data->curves.current_variable]-'A');
    data->curves.tempMarkLine[current_y*2]=call_num;
    delta_y = height / strlen(data->curves.y_list);
    y = (data->curves.current_variable) * delta_y + GAP/2; 
    XCopyArea(XtDisplay(data->curves.sample[0]), 
                      data->curves.view_markTile[call_num], 
                      XtWindow(data->curves.sample[0]),
                      data->curves.shellGC, 0, 0, 
                      view_symbol_width, view_symbol_height,
                      (width - view_symbol_width)/2, y);
}

/*
** line selection call back function
*/

static void line_selection(w, data, callData)
    Widget w;
    CurvesWidget data;
    XmToggleButtonCallbackStruct *callData;
{
  int call_num, n, y, delta_y, current_y;
  Arg wargs[5];
  Dimension height, width;
  
  if (!callData->set) return;
  
    n = 0;
    XtSetArg(wargs[n], XtNwidth, &width); n++;
    XtSetArg(wargs[n], XtNheight, &height); n++;
    XtGetValues(data->curves.sample[1], wargs, n);
    XtSetArg(wargs[0], XmNuserData, &call_num);
    XtGetValues(w, wargs, 1);
   
    current_y = (int)(data->curves.y_list[data->curves.current_variable]-'A');
    data->curves.tempMarkLine[current_y*2+1]=call_num;
    delta_y = height / strlen(data->curves.y_list);
    y = (data->curves.current_variable) * delta_y + GAP/2; 
    XCopyArea(XtDisplay(data->curves.sample[1]), 
                      data->curves.lineTile[call_num], 
                      XtWindow(data->curves.sample[1]),
                      data->curves.shellGC, 0, 0, 
                      lines_width, lines_height,
                      (width - lines_width)/2, y);
}

/*
** selection call back function
*/

static void select_variable (w, data, call_data)
     Widget w;
     CurvesWidget data;
     XmToggleButtonCallbackStruct *call_data;
  {
    int variable, current_y, num;
    Arg wargs[1];

    if (!call_data->set) return;
    
    current_y=(int)(data->curves.y_list[data->curves.current_variable]-'A');
    num = data->curves.tempMarkLine[current_y*2];
    XtSetArg(wargs[0], XmNset, False);
    XtSetValues(data->curves.markButton[num], wargs, 1);
    num = data->curves.tempMarkLine[current_y*2+1];
    XtSetArg(wargs[0], XmNset, False);
    XtSetValues(data->curves.lineButton[num], wargs, 1);
    XtSetArg(wargs[0], XmNuserData, &variable);
    XtGetValues(w, wargs, 1);
    data->curves.current_variable = variable;
    current_y = (int)(data->curves.y_list[variable] - 'A');
    num = data->curves.tempMarkLine[current_y*2];
    XtSetArg(wargs[0], XmNset, True);
    XtSetValues(data->curves.markButton[num], wargs, 1);
    num = data->curves.tempMarkLine[current_y*2+1];
    XtSetArg(wargs[0], XmNset, True);
    XtSetValues(data->curves.lineButton[num], wargs, 1);
}

/*
** sample expose
*/

static void sample_expose(w, data, call_data)
     Widget w; 
     CurvesWidget data;
     XmDrawingAreaCallbackStruct *call_data;
{
  Arg wargs[5];
  int i, y, n, index, current_y, curve_num;
  Dimension width, height;

  if (call_data->reason != XmCR_EXPOSE) 
    return;

  n = 0;
  XtSetArg(wargs[n], XtNwidth, &width); n++;
  XtSetArg(wargs[n], XtNheight, &height); n++;
  XtGetValues(data->curves.sample[0], wargs, n);

  curve_num = 0;
  for (i = 0; i < strlen(data->curves.y_list); i++) {
    current_y = (int)(data->curves.y_list[i] - 'A');
    if ((data->curves.data+current_y)->options != CURVE_NO_OPTIONS)
    	continue;
    y = curve_num * height / n_drawable_curves(data) + GAP / 2;
    curve_num++;
    index =  data->curves.tempMarkLine[current_y*2];
    XCopyArea(XtDisplay(data->curves.sample[0]), 
                        data->curves.view_markTile[index],
                        XtWindow(data->curves.sample[0]),
                      data->curves.shellGC, 0, 0,
                      view_symbol_width, view_symbol_height,
                      (width - view_symbol_width)/2, y);
  }
  
  n = 0;
  XtSetArg(wargs[n], XtNwidth, &width); n++;
  XtSetArg(wargs[n], XtNheight, &height); n++;
  XtGetValues(data->curves.sample[1], wargs, n);

  curve_num = 0;
  for (i = 0; i < strlen(data->curves.y_list); i++) {
    current_y = (int)(data->curves.y_list[i] - 'A');
    if ((data->curves.data+current_y)->options != CURVE_NO_OPTIONS)
    	continue;
    y = curve_num * height / n_drawable_curves(data) + GAP / 2;
    curve_num++;
    index =  data->curves.tempMarkLine[current_y*2+1];
    XCopyArea(XtDisplay(data->curves.sample[1]), data->curves.lineTile[index],
                      XtWindow(data->curves.sample[1]),
                      data->curves.shellGC, 0, 0,
                      lines_width, lines_height,
                      (width - lines_width)/2, y);
   }
}

/*
** close the setting widget 
*/

static void closeShell(CurvesWidget data) {
int i;

  if (data->curves.tempMarkLine != NULL) {
    for (i = 0; i < data->curves.num_variable; i++) {
      data->curves.updateMarkLine[i*2] = data->curves.tempMarkLine[i*2];
      data->curves.updateMarkLine[i*2+1] = data->curves.tempMarkLine[i*2+1];
    }
    XtFree((char *)data->curves.tempMarkLine);
    XtFree((char *)data->curves.storeMarkLine);
  }
  data->curves.isShellSet = False;
  XtUnmanageChild(data->curves.shell); 
}


/*
** apply function
*/

static void treat_apply(CurvesWidget data, int state) {
  int i;

  for (i = 0; i < data->curves.num_variable; i++) {
    (data->curves.data+i)->mark_num = data->curves.tempMarkLine[i*2];
    (data->curves.data+i)->line_num = data->curves.tempMarkLine[i*2+1];
    data->curves.updateMarkLine[i*2] = data->curves.tempMarkLine[i*2];
    data->curves.updateMarkLine[i*2+1] = data->curves.tempMarkLine[i*2+1]; 
 }

  redisplayContents(data, X_SCREEN, REDRAW_ALL);
  if (state == 0)
    closeShell(data);
}

 
/* 
** cancel the current setting of mark and symbol
*/

static void draw_cancel(CurvesWidget data) {
  int i;
  
  for (i = 0; i < data->curves.num_variable; i++) {
    data->curves.tempMarkLine[i*2] = data->curves.storeMarkLine[i*2];
    data->curves.tempMarkLine[i*2+1] = data->curves.storeMarkLine[i*2+1];
  }
  treat_apply(data, 0);
}
    
  
  
/*
** command call back function
*/

static void treat_command(w, data, call_data)
     Widget w;
     CurvesWidget data;
     XmPushButtonCallbackStruct *call_data;
  {
    Arg wargs[1];
    int n;

    if (call_data->reason != XmCR_ACTIVATE) return;
  
    XtSetArg(wargs[0], XmNuserData, &n);
    XtGetValues(w, wargs, 1);

    switch (n) {
      case 0 :
      case 1 : treat_apply(data, n);
               break;
      case 2 : closeShell(data);
               break;
      case 3 : draw_cancel(data);
               break;
    }
  }

/*
** sample area
*/

static void create_sample_area(Widget parent, CurvesWidget data) {
  int i, n, x, width, height;
  Arg wargs[5];
  
  n = n_drawable_curves(data);
  width = SAMPLE_WIDTH / 5;
  height = (int)(data->curves.selectionFrameHeight * n * 1.1); 

  x = 0;
  for (i = 0; i < 2; i++) {
    n = 0;
    XtSetArg(wargs[n], XmNx, x); n++; 
    XtSetArg(wargs[n], XmNy, 0); n++; 
    XtSetArg(wargs[n], XmNwidth, width); n++;
    XtSetArg(wargs[n], XmNheight, height); n++;
    data->curves.sample[i] = XtCreateManagedWidget("sample", 
                                      xmDrawingAreaWidgetClass,
                                      parent, wargs, n);
    XtAddCallback(data->curves.sample[i],
                  XmNexposeCallback,
                  (XtCallbackProc) sample_expose, data);
    x = width + GAP;
    width = SAMPLE_WIDTH - width;
  }
}


/*
** command area
*/

static void create_command_area(Widget parent, CurvesWidget data) {
  Widget w; 
  int i;
  Arg wargs[1];
  
  for (i = 0; i < XtNumber(CURVES_COMMAND); i++) {
    XtSetArg(wargs[0], XmNuserData, i);
    w = XtCreateManagedWidget(CURVES_COMMAND[i], xmPushButtonWidgetClass, 
                       parent, wargs, 1);
    XtAddCallback(w, XmNactivateCallback,
                  (XtCallbackProc) treat_command, data);
  } /* for */

}


/* 
** create celection area in the seetting widget
*/

static void create_selection_area(Widget parent, CurvesWidget data) {
  Widget selection;
  Widget selection_com, w;
  int i, n, n1, current_y;
  Arg wargs[10];
  Dimension height;
  
  selection = XtCreateManagedWidget("values",
                                     xmRowColumnWidgetClass,
                                     parent, NULL, 0);
 
  n = 0;
  XtSetArg(wargs[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetValues(selection, wargs, n);

  n = 0;
  XtSetArg(wargs[n], XmNentryClass, xmToggleButtonWidgetClass);n++;
  selection_com = XmCreateRadioBox(selection, "Select", wargs, n);
  XtManageChild(selection_com);

  /* create sub_widgets for selection */
  n = strlen(data->curves.y_list);
  for (i = 0; i < n; i++) {
    current_y = (int)(data->curves.y_list[i]-'A');
    if ((data->curves.data+current_y)->options != CURVE_NO_OPTIONS)
      continue;
    n1 = 0;
    XtSetArg(wargs[n1], XmNuserData, i); n1 ++;
    if (i == 0) {
      XtSetArg(wargs[n1], XmNset, True); n1++;
      data->curves.current_variable = 0;
    }
    w = XtCreateManagedWidget((data->curves.data+current_y)->variable_name,
                              xmToggleButtonWidgetClass,
                              selection_com, wargs, n1);
    XtAddCallback(w, XmNvalueChangedCallback,
                 (XtCallbackProc) select_variable, data);
  } /* for */
  XtSetArg(wargs[0], XmNheight, &height);
  XtGetValues(w, wargs, 1);
  data->curves.selectionFrameHeight = height;

}


/*
** create mark area in the setting widget
*/

static void create_mark_area(Widget parent, CurvesWidget data) {
  Widget mark_com;
  int i, n;
  Arg wargs[10];
  int current_y = (int)(data->curves.y_list[0]-'A');
  int mark_num = (data->curves.data+current_y)->mark_num;
  
  n = 0;
  XtSetArg(wargs[n], XmNentryClass, xmToggleButtonWidgetClass);n++;
  mark_com = XmCreateRadioBox(parent, "Mark", wargs, n);
  XtManageChild(mark_com);

  /* create buttons */
  for (i = 0; i < XtNumber(symbol); i++) {
    data->curves.markButton[i] = 
               XtCreateManagedWidget("mark_widget", xmToggleButtonWidgetClass,
                                     mark_com, NULL, 0);
    n = 0;
    XtSetArg(wargs[n], XmNlabelType, XmPIXMAP); n++;
    XtSetArg(wargs[n], XmNlabelPixmap, data->curves.view_markTile[i]); n++;
    XtSetArg(wargs[n], XmNuserData, i); n++;
    if (i == mark_num) {
      XtSetArg(wargs[n], XmNset, True); n++;
    }
    XtSetValues(data->curves.markButton[i], wargs, n);
    XtAddCallback(data->curves.markButton[i], XmNvalueChangedCallback,
                  (XtCallbackProc) mark_selection, data);
  } /* for */
}
                                                                                
/*
** create line area in the setting widget
*/

static void create_line_area(Widget parent, CurvesWidget data) {
  Widget line_com;
  int i, n;
  Arg wargs[10];
  int current_y = (int)(data->curves.y_list[0]-'A');
  int line_num = (data->curves.data+current_y)->line_num;
  
  n = 0;
  XtSetArg(wargs[n], XmNentryClass, xmToggleButtonWidgetClass);n++;
  line_com = XmCreateRadioBox(parent, "Line", wargs, n);
  XtManageChild(line_com);

  /* create buttons */
  for (i = 0; i < XtNumber(lines); i++) {
    data->curves.lineButton[i] = 
            XtCreateManagedWidget("line_widget", xmToggleButtonWidgetClass,
                                  line_com, NULL, 0);
    n = 0;
    XtSetArg(wargs[n], XmNlabelType, XmPIXMAP); n++;
    XtSetArg(wargs[n], XmNlabelPixmap, data->curves.lineTile[i]); n++;
    XtSetArg(wargs[n], XmNuserData, i); n++;
    if (i == line_num) {
      XtSetArg(wargs[n], XmNset, True); n++;
    }
    XtSetValues(data->curves.lineButton[i], wargs, n);
    XtAddCallback(data->curves.lineButton[i], XmNvalueChangedCallback,
                  (XtCallbackProc) line_selection, data);
  } /* for */
}

/*
** create the setting widget
*/

static void setup_setting_widget(Widget w) {
  CurvesWidget sw = (CurvesWidget)w;
  Widget topshell;
  Widget selection_area, sample_area, mark_area;
  Widget line_style_area,command_area;
  int i, n;
  Arg wargs[10];
  XGCValues values;
  Display *dpy = XtDisplay(sw);

  if (sw->curves.isShellSet)
    return;
  
  /* create topshell widget */
  n = 0;
  XtSetArg(wargs[n], XmNtitle, "Setting Mark & Line Style"); n++;
  XtSetArg(wargs[n], XmNautoUnmanage, False); n++;
  topshell = XmCreateFormDialog (w, "lineStyleDialog", wargs, n);
  XtVaSetValues(topshell, XmNshadowThickness, 0, 0);
  sw->curves.shell = topshell;
  /* create frame widget */
  selection_area = XtCreateManagedWidget("selection",
                                        xmFormWidgetClass,
                                        topshell, NULL, 0);
  command_area = XtCreateManagedWidget("command",
                                      xmRowColumnWidgetClass,
                                      topshell, NULL, 0);
  sample_area = XtCreateManagedWidget("sample",
                                      xmBulletinBoardWidgetClass,
                                      topshell, NULL, 0);
  mark_area = XtCreateManagedWidget("mark",
                                    xmRowColumnWidgetClass,
                                    topshell, NULL, 0);
  line_style_area = XtCreateManagedWidget("line_style",
                                               xmRowColumnWidgetClass,
                                               topshell, NULL, 0);
  n = 0;
  XtSetArg(wargs[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetValues(mark_area, wargs, n);

  n = 0;
  XtSetArg(wargs[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(wargs[n], XmNleftWidget, mark_area); n++;
  XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetValues(line_style_area, wargs, n);

  n = 0;
  XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(wargs[n], XmNleftWidget, line_style_area); n++;
  XtSetArg(wargs[n], XmNorientation, XmHORIZONTAL); n++;
  XtSetValues(command_area, wargs, n);

  n = 0;
  XtSetArg(wargs[n], XmNtopAttachment, XmATTACH_FORM); n++;
  
  XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(wargs[n], XmNleftWidget, line_style_area); n++;
  XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(wargs[n], XmNbottomWidget, command_area); n++;
 
  XtSetValues(selection_area, wargs, n);

  n = 0;
  XtSetArg(wargs[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(wargs[n], XmNleftWidget, selection_area); n++;
  XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(wargs[n], XmNbottomWidget, command_area); n++;
  XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetValues(sample_area, wargs, n);

  sw->curves.tempMarkLine = 
                      (int *)XtMalloc(sizeof(int)*sw->curves.num_variable*2);
  sw->curves.storeMarkLine = 
                      (int *)XtMalloc(sizeof(int)*sw->curves.num_variable*2);
  for (i = 0; i < sw->curves.num_variable; i++) {
    sw->curves.tempMarkLine[i*2] = (sw->curves.data+i)->mark_num;
    sw->curves.tempMarkLine[i*2+1] = (sw->curves.data+i)->line_num;
    sw->curves.storeMarkLine[i*2] = (sw->curves.data+i)->mark_num;
    sw->curves.storeMarkLine[i*2+1] = (sw->curves.data+i)->line_num;
  }
  
  n = 0;
  XtSetArg(wargs[n], XtNforeground, &(values.foreground)); n++;
  XtSetArg(wargs[n], XtNbackground, &(values.background)); n++;
  XtGetValues(topshell, wargs, n);  
  values.font = getFontStruct(sw->curves.font)->fid;
  sw->curves.shellGC = XCreateGC(dpy, XDefaultRootWindow(dpy), GCForeground |
                       GCBackground | GCFont, &values); 
  if (!(sw->curves.isPixmapRegisted)) {
    sw->curves.isPixmapRegisted = True;
    for (i = 0; i < NUMMARK; i++) {
      register_pattern(view_symbol[i].name, view_symbol[i].bitmap,
                         view_symbol_width, view_symbol_height);
      sw->curves.view_markTile[i] = XmGetPixmap(XtScreen(sw), 
                                            view_symbol[i].name,
                                            values.foreground,
                                            values.background);
    }
    for (i = 0; i < NUMLINE; i++) {
      register_pattern(lines[i].name, lines[i].bitmap, lines_width, 
                      lines_height);
      sw->curves.lineTile[i] = XmGetPixmap(XtScreen(sw), lines[i].name,
                                          values.foreground, values.background);
    }
  }
  create_mark_area(mark_area, sw);
  create_line_area(line_style_area, sw);
  create_command_area(command_area, sw);
  create_selection_area(selection_area, sw);
  create_sample_area(sample_area, sw);
  sw->curves.isShellSet = True;
  XtManageChild(topshell);                                  
}

/*
** rescale scaling widget
*/

static void rescale_scale(CurvesWidget sw, Boolean sortX) {
    double minX, minY, maxX, maxY;
    
    DataRange(sw, &maxX, &minX, &maxY, &minY, sortX);
    if (maxY == minY) {maxY += 1.; minY -= 1.;}
    resize(sw);
    sw->curves.maxYData = sw->curves.maxYLim = maxY;
    sw->curves.minYData = sw->curves.minYLim = minY;
    if (sortX) {
      sw->curves.maxXData = sw->curves.maxXLim = maxX;
      sw->curves.minXData = sw->curves.minXLim = minX;
    }
} /* rescale_scale */


/*
** scale expose
*/

static void scale_expose(w, data, call_data) 
     Widget w;
     CurvesWidget data;
     XmDrawingAreaCallbackStruct *call_data;
{
  Arg wargs[5];
  int i, y, n, current_y, curve_num;
  Dimension width, height;
  char *string;
  XFontStruct *fs = getFontStruct(data->curves.font);
  
  XClearWindow(XtDisplay(w), XtWindow(data->curves.scaleFormular));

  n = 0;
  XtSetArg(wargs[n], XtNwidth, &width); n++;
  XtSetArg(wargs[n], XtNheight, &height); n++;
  XtGetValues(data->curves.scaleFormular, wargs, n);

  curve_num = 0;
  for (i = 0; i < strlen(data->curves.y_list); i++) {
    current_y = (int)(data->curves.y_list[i]-'A');
    if ((data->curves.data+current_y)->options != CURVE_NO_OPTIONS)
    	continue;
    y = curve_num * height / n_drawable_curves(data) + fs->ascent + fs->descent;
    curve_num++;
    string = (data->curves.data+current_y)->variable_name;      
    XDrawString(XtDisplay(data->curves.scaleFormular), 
                XtWindow(data->curves.scaleFormular), data->curves.scaleGC,
                0, y, string, strlen(string));
  }
}


/*
** draw formular
*/

static void draw_scale_formular(CurvesWidget data) {
  int n, current_y, len;
  Dimension width, height;
  XGCValues values; 
  Display *display;
  GC gc = data->curves.scaleGC; 
  Arg wargs[5];
  char *string, *tempName, *op;
  ScalePtr tempPtr;

  if (data->curves.isScaleSet) {
    display =  XtDisplay(data->curves.scaleFormular);
    XGetGCValues(display,gc,GCForeground | GCBackground ,&values);
  }
  n = 0;
  XtSetArg(wargs[n], XtNwidth, &width); n++;
  XtSetArg(wargs[n], XtNheight, &height); n++;
  if (data->curves.isScaleSet)
    XtGetValues(data->curves.scaleFormular, wargs, n);
  current_y = (int)(data->curves.y_list[data->curves.current_scaling]-'A');
  tempPtr = *(data->curves.scaleFunction+current_y);
  len = strlen((data->curves.store_data+current_y)->variable_name);
  tempName = (char *)XtMalloc(sizeof(char)*len+1);
  strcpy(tempName, (data->curves.store_data+current_y)->variable_name);
  while (tempPtr != NULL) {
    len = strlen(tempName);
    string = (char *)XtMalloc(sizeof(char) * len +1);
    strcpy(string, tempName);
    XtFree((char *)tempName);
    switch (tempPtr->function_num) {
      case 0 : tempName = (char *)XtMalloc(sizeof(char) * len + 7);
               strcpy(tempName, "Ln(|");
               strcat(tempName, string);
               strcat(tempName, "|)");
               break;
      case 1 : tempName = (char *)XtMalloc(sizeof(char) * len + 8);
               strcpy(tempName, "Log(|");
               strcat(tempName, string);
               strcat(tempName, "|)");
               break;
      case 2 : tempName = (char *)XtMalloc(sizeof(char) * len + 6);
               strcpy(tempName, "exp(");
               strcat(tempName, string);
               strcat(tempName, ")");
               break;
      case 3 : tempName = (char *)XtMalloc(sizeof(char) * len + 6);
               strcpy(tempName, "10^(");
               strcat(tempName, string);
               strcat(tempName, ")");
               break;
      case 4 : tempName = (char *)XtMalloc(sizeof(char) * len + 5);
               strcpy(tempName, "1/(");
               strcat(tempName, string);
               strcat(tempName, ")");
               break;
      case 5 : tempName = (char *)XtMalloc(sizeof(char) * len + 5);
               strcpy(tempName, "(");
               strcat(tempName, string);
               strcat(tempName, ")^2");
               break;
      case 6 : tempName = (char *)XtMalloc(sizeof(char) * len + 3);
               strcpy(tempName, "|");
               strcat(tempName, string);
               strcat(tempName, "|");
               break;
      case 7 : tempName = (char *)XtMalloc(sizeof(char) * len + 7);
               strcpy(tempName, "sqrt(");
               strcat(tempName, string);
               strcat(tempName, ")");
               break;
      case 9 : tempName = (char *)XtMalloc(sizeof(char) * len + 20);
      	       op = (tempPtr->value < 0.) ? "-" : "+";
               sprintf(tempName, "%s %s %g", string, op,
               		(double)fabs(tempPtr->value));
               break;
      case 10: tempName = (char *)XtMalloc(sizeof(char) * len + 22);
               sprintf(tempName, "%g * (%s)", tempPtr->value, string);
               break;    
    }
    XtFree((char *)string);
    tempPtr = tempPtr->next;
  }
  string = (data->curves.data+current_y)->variable_name;  
  XtFree((char *)(data->curves.data+current_y)->variable_name);
  ((data->curves.data+current_y)->variable_name) = tempName;
  scale_expose((Widget)data, data, NULL);
}

/* 
** natural log scale 
*/

static void natural_log_scale(CurvesWidget data, CurveStruct *curve) {
  int i;
  
  for (i = 0; i < curve->num_point; i++) {
    if (curve->points[i] < 1.0e-32) {
      DialogF(DF_WARN, data->curves.scale, 1, 
      "There are ZEROs or negative numbers in the data.\nCan not take Ln operation", 
      "Acknowledged");
      data->curves.scaleValid = False;
      break;
    }
    curve->points[i] = (float)log(curve->points[i]);
  }
}

/* 
** base 10 log scale 
*/

static void log_scale(CurvesWidget data, CurveStruct *curve) {
  int i;
  
  for (i = 0; i < curve->num_point; i++) {
    if (curve->points[i] < 1.0e-32) {
      DialogF(DF_WARN, data->curves.scale, 1, 
      "There are ZEROs or negative numbers in the data.\nCan not take Log operation", 
      "Acknowledged");
      data->curves.scaleValid = False;
      break;
    }
    curve->points[i] = (float)log10(curve->points[i]);
  }
}


/* 
** exponent scale 
*/

static void exponent_scale(CurvesWidget data, CurveStruct *curve) {
  int i;
  
  for (i = 0; i < curve->num_point; i++) {
    if ((double)(curve->points[i]) > 32.) {
      DialogF(DF_WARN, data->curves.scale, 1, 
      "OVERFLOW During EXP operation\nCancelled", 
      "Acknowledged");
      data->curves.scaleValid = False;
      break;
    }    
    curve->points[i] = (float)exp((double)curve->points[i]);
  }
}


/* 
** 10 base power scale 
*/

static void exp_10_scale(CurvesWidget data, CurveStruct *curve) {
  int i;
  
  for (i = 0; i < curve->num_point; i++) {
    if ((double)(curve->points[i]) > 16.) {
      DialogF(DF_WARN, data->curves.scale, 1, 
      "OVERFLOW During 10^X operation\nCancelled", 
      "Acknowledged");
      data->curves.scaleValid = False;
      break;
    }
    curve->points[i] = (float)pow(10.0, (double)curve->points[i]);
  }
}

/* 
** inverse scale 
*/

static void inverse_scale(CurvesWidget data, CurveStruct *curve) {
  int i;
  
  for (i = 0; i < curve->num_point; i++) {
    if ((double)fabs(curve->points[i]) < 1.0e-32) {
      DialogF(DF_WARN, data->curves.scale, 1, 
      "There are ZEROs in the data.\nCan not take 1/X operation", 
      "Acknowledged");
      data->curves.scaleValid = False;
      break;
    }
    curve->points[i] = 1.0/curve->points[i];
  }
}

/* 
** square scale 
*/

static void square_scale(CurvesWidget data, CurveStruct *curve) {
  int i;
  
  for (i = 0; i < curve->num_point; i++) {
    if ((double)fabs(curve->points[i]) > 1.0e16) {
      DialogF(DF_WARN, data->curves.scale, 1, 
      "OVERFLOW During X^2 operation\nCancelled", 
      "Acknowledged");
      data->curves.scaleValid = False;
      break;
    }
    curve->points[i] = (curve->points[i]) * (curve->points[i]);
  }
}

/* 
** abs scale 
*/

static void abs_scale(CurveStruct *curve) {
  int i;
  
  for (i = 0; i < curve->num_point; i++)
    curve->points[i] = (float)fabs((double)(curve->points[i]));
}

/* 
** sqrt scale 
*/

static void sqrt_scale(CurvesWidget data, CurveStruct *curve) {
  int i;
  
  for (i = 0; i < curve->num_point; i++) {
    if (curve->points[i] < 0.) {
      DialogF(DF_WARN, data->curves.scale, 1, 
      "There are NEGATIVE numbers in the data.\nCan not take SQRT operation", 
      "Acknowledged");
      data->curves.scaleValid = False;
      break;
    }
    curve->points[i] = (float)sqrt((double)(curve->points[i]));
  }
}


/* 
** add value
**/

static void add_value_scale(CurvesWidget data, CurveStruct *curve, 
                            double value) {
  int i;
  
  for (i = 0; i < curve->num_point; i++)
    curve->points[i] = curve->points[i] + value;
}


/* 
** multiply value
**/

static void multi_value_scale(CurvesWidget data, CurveStruct *curve, 
                              double value) {
  int i;

  for (i = 0; i < curve->num_point; i++)
    curve->points[i] = curve->points[i] * value;
}

/* 
** undo scale
*/

static void undo_scale(ScalePtr *ptr, int *numToRemove) {
  if ((*ptr == NULL) || (*numToRemove == 0))
    return;
  undo_scale(&(*ptr)->next, numToRemove);
  if (*numToRemove == 0 )
    return;
  XtFree((char *)*ptr);
  *ptr = NULL;
  (*numToRemove)--;
}

/*
** svaeFunction
*/

static void saveFunction(ScalePtr *ptr, int num, double value) {
  if (*ptr == NULL) {
    *ptr = (ScalePtr)XtMalloc(sizeof(struct CurvesScaleFunction));
    (*ptr)->function_num = num;
    (*ptr)->value = value;
    (*ptr)->next = NULL;
     return;
   }
   saveFunction(&((*ptr)->next), num, value);
}
  
/*
** set scale
*/
 
static void set_scale(w, data, call_data) 
     Widget w;
     CurvesWidget data;
     XmPushButtonCallbackStruct *call_data;
{
    Arg wargs[2];
    int userData, result, scale_error_bars, current_y, i;
    double value;

    if (call_data->reason != XmCR_ACTIVATE) return;
    
    /* get the function type to insert */
    XtSetArg(wargs[0], XmNuserData, &userData);
    XtGetValues(w, wargs, 1);
    
    /* check if error bar variables need to be scaled also */
    current_y = (int)(data->curves.y_list[data->curves.current_scaling]-'A');
    scale_error_bars = current_y+1 < data->curves.num_variable &&
      	(data->curves.data+current_y+1)->options == CURVE_TOP_ERROR;
    
    /* process undo and reset */
    if (userData == 8) { /* undo */
    	i = 1; undo_scale(data->curves.scaleFunction+current_y, &i);
    	if (scale_error_bars) {
    	    i = 1; undo_scale(data->curves.scaleFunction+current_y+1, &i);
    	    i = 1; undo_scale(data->curves.scaleFunction+current_y+2, &i);
    	}
        draw_scale_formular(data);
	return;
    } else if (userData ==  11) { /* reset */
      	freeScalePtr(data->curves.scaleFunction+current_y);
      	if (scale_error_bars) {
      	    freeScalePtr(data->curves.scaleFunction+current_y+1);
      	    freeScalePtr(data->curves.scaleFunction+current_y+2);
      	}
        draw_scale_formular(data);
	return;
    }
    
    /* get any constant values that go along with that function type */
    if (userData == 9) { /* add constant */
      if (GetFloatTextWarn(data->curves.textArea[0], &value,
      				"constant", True)!= TEXT_READ_OK)
        return;
    } else if (userData == 10) { /* mult constant */
      if (GetFloatTextWarn(data->curves.textArea[1], &value,
      				"constant", True)!= TEXT_READ_OK)
        return;
    } 
    
    /* test-scale the data and make sure the new function works */
    test_scale(data, userData, current_y, value);
    if (scale_error_bars && data->curves.scaleValid)
      test_scale(data, userData, current_y+1, value);
    if (scale_error_bars && data->curves.scaleValid)
      test_scale(data, userData, current_y+2, value);
    
    /* if it worked, insert the new scaling function */
    if (data->curves.scaleValid) {
      saveFunction(data->curves.scaleFunction+current_y, userData, value);
      if (scale_error_bars) {
      	saveFunction(data->curves.scaleFunction+current_y+1, userData, value);
      	saveFunction(data->curves.scaleFunction+current_y+2, userData, value);
      }
    }
    data->curves.scaleValid = True;    
    
    /* display the new formula in the scaling dialog */
    draw_scale_formular(data);
}

static void test_scale(CurvesWidget data, int scale_fn, int current_y,
	double value) 
  {
    int i;
    CurveStruct *curve;
    ScalePtr tempPtr;
    
    curve = (CurveStruct *)XtMalloc(sizeof(CurveStruct)); 
    curve->num_point = (data->curves.store_data+current_y)->num_point; 
    if (curve->num_point == 0)
      curve->points = NULL;
    else {
      curve->points = (float *)XtMalloc(sizeof(float)*
                     (data->curves.store_data+current_y)->num_point); 
      for (i = 0; i < (data->curves.store_data+current_y)->num_point; i++)
        curve->points[i] = (data->curves.store_data+current_y)->points[i];
    }
    tempPtr = *(data->curves.scaleFunction+current_y);   
    while (tempPtr != NULL) {
      switch (tempPtr->function_num) {
        case 0 : natural_log_scale(data, curve); break;
        case 1 : log_scale(data, curve); break;
        case 2 : exponent_scale(data, curve); break;
        case 3 : exp_10_scale(data, curve); break;
        case 4 : inverse_scale(data, curve); break;
        case 5 : square_scale(data, curve); break;
        case 6 : abs_scale(curve); break;
        case 7 : sqrt_scale(data, curve); break;
        case 9 : add_value_scale(data, curve, tempPtr->value); break;
        case 10: multi_value_scale(data, curve, tempPtr->value); break;
      }
      tempPtr = tempPtr->next;
    }
    switch (scale_fn) {
      case 0 : natural_log_scale(data, curve); break;
      case 1 : log_scale(data, curve); break;
      case 2 : exponent_scale(data, curve); break;
      case 3 : exp_10_scale(data, curve); break;
      case 4 : inverse_scale(data, curve); break;
      case 5 : square_scale(data, curve); break;
      case 6 : abs_scale(curve); break;
      case 7 : sqrt_scale(data, curve); break;
      case 9 : add_value_scale(data, curve, value); break;
      case 10: multi_value_scale(data, curve, value); break;
    }  
    if (data->curves.yLogScaling) {
      for (i = 0; i < curve->num_point; i++) {
	if (curve->points[i] < 1.0e-32) {
	  DialogF(DF_WARN, data->curves.scale, 1, 
	    "Operation results in ZEROS or NEGATIVE numbers.\n\
Can not display results on a log plot.", "Acknowledged");
	  data->curves.scaleValid = False;
	  break;
	}
      }
    }

    if (curve->points != NULL)
      XtFree((char *)curve->points);
    XtFree((char *)curve);
} 

/*
** scale variable call back function
*/

static void scale_variable(w, data, call_data)
     Widget w;
     CurvesWidget data;
     XmToggleButtonCallbackStruct *call_data;
  {
    Arg wargs[1];
    int user_data;
    
    if (!call_data->set) return;
    
    XtSetArg(wargs[0], XmNuserData, &user_data);
    XtGetValues(w, wargs, 1);
    data->curves.current_scaling = user_data;      
  }


/*
** close the setting widget 
*/

static void scalingCloseShell(CurvesWidget data) {
  int i;

  if (data->curves.tempFunction != NULL) {
    for (i = 0; i < data->curves.num_variable; i++) {
      freeScalePtr(data->curves.tempFunction+i);
    }
    XtFree((char *)data->curves.tempFunction);
  }
  if (data->curves.tempName != NULL) {
    for (i = 0; i < data->curves.num_variable; i++) {
      XtFree((char *)*(data->curves.tempName+i));
    }
    XtFree((char *)data->curves.tempName);
  }
  data->curves.isScaleSet = False;
  XtUnmanageChild(data->curves.scale); 
}


/*
** scaling apply function
*/

static void scaling_treat_apply(CurvesWidget data, int rescale) {
  int i, j, current_y;
  CurveStruct *curve, *store_curve;
  XmString label;
  ScalePtr tempPtr;

  if (data->curves.xAxisLabel != NULL) {
    label = XmStringCreateSimple(data->curves.data->variable_name); 
    XmStringFree(data->curves.xAxisLabel);
    data->curves.xAxisLabel = XmStringCopy(label);
    XmStringFree(label);
  }
  if (data->curves.yAxisLabel != NULL) {
    i = (int)(data->curves.y_list[0] - 'A');
    label = XmStringCreateSimple((data->curves.data+i)->variable_name); 
    XmStringFree(data->curves.yAxisLabel);
    data->curves.yAxisLabel = XmStringCopy(label);
    XmStringFree(label);
  }
  for (i = 0; i < strlen(data->curves.y_list); i++) {
    current_y = (int)(data->curves.y_list[i] - 'A');
    curve = data->curves.data + current_y;
    store_curve = data->curves.store_data + current_y;
    for (j = 0; j < store_curve->num_point; j++)
      curve->points[j] = store_curve->points[j];
    curve->num_point = store_curve->num_point;
    tempPtr = *(data->curves.scaleFunction + current_y);
    while (tempPtr != NULL) {
      switch(tempPtr->function_num) {
        case 0 : natural_log_scale(data, curve); break;
        case 1 : log_scale(data, curve); break;
        case 2 : exponent_scale(data, curve); break;
        case 3 : exp_10_scale(data, curve); break;
        case 4 : inverse_scale(data, curve); break;
        case 5 : square_scale(data, curve); break;
        case 6 : abs_scale(curve); break;
        case 7 : sqrt_scale(data, curve); break;
        case 9 : add_value_scale(data, curve, tempPtr->value); break;
        case 10: multi_value_scale(data, curve, tempPtr->value); break;
      }
      tempPtr = tempPtr->next;   
    }
  }
  if (rescale)  
    rescale_scale(data, False);
  redisplayContents(data, X_SCREEN, REDRAW_ALL);
}


/* 
** free ScalePtr
*/

static void freeScalePtr(ScalePtr *ptr) {
  if (*ptr == NULL)
    return;
  freeScalePtr(&((*ptr)->next));
  XtFree((char *)*ptr);
  *ptr = NULL;
}


/* 
** scaling cancel
*/

static void scaling_cancel(CurvesWidget data) {
  int i;

  for (i = 0; i < data->curves.num_variable; i++) {
     freeScalePtr(data->curves.scaleFunction+i);
     *(data->curves.scaleFunction+i) = *(data->curves.tempFunction+i);
     (data->curves.data+i)->variable_name = *(data->curves.tempName+i);
  }
  data->curves.tempFunction = NULL;
  data->curves.tempName = NULL;
  scaling_treat_apply(data, True);
  scalingCloseShell(data);
}


/*
** scaling command call back function
*/

static void scaling_treat_command(w, data, call_data)
     Widget w;
     CurvesWidget data;
     XmPushButtonCallbackStruct *call_data;
  {
    Arg wargs[1];
    int n;

    if (call_data->reason != XmCR_ACTIVATE) return;
  
    XtSetArg(wargs[0], XmNuserData, &n);
    XtGetValues(w, wargs, 1);

    switch (n) {
      case 0 :
      case 1 : scaling_treat_apply(data, True);
               break;
      case 2 : scalingCloseShell(data);
               break;
      case 3 : scaling_cancel(data);
    }
  }

    /* 
** create scaling area
*/

static void create_scaling_area(Widget parent, CurvesWidget data) {
  Widget area_1, area_2;
  Widget text_area, w;
  int i, n;
  Arg wargs[10];
  Dimension height;
  Position y;
  XFontStruct *fs = getFontStruct(data->curves.font);
  
  area_1 = XtCreateManagedWidget("values", xmRowColumnWidgetClass, parent, 
                                 NULL, 0);
  area_2 = XtCreateManagedWidget("add", xmRowColumnWidgetClass, parent, 
                                 NULL, 0); 
  text_area = XtCreateManagedWidget("text", xmFormWidgetClass, parent, 
                                 NULL, 0); 
  n = 0;
  XtSetArg(wargs[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetArg(wargs[n], XmNpacking, XmPACK_COLUMN); n++;
  XtSetArg(wargs[n], XmNorientation, XmHORIZONTAL); n++;
  XtSetArg(wargs[n], XmNnumColumns, 3); n++;
  XtSetValues(area_1, wargs, n);

  n = 0;
  XtSetArg(wargs[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(wargs[n], XmNleftWidget, area_1); n++;
  XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetValues(area_2, wargs, n);
 
  n = 0;
  XtSetArg(wargs[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(wargs[n], XmNleftWidget, area_2); n++;
  XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetValues(text_area, wargs, n);
    
  /* create sub_widgets */
  for (i = 0; i < XtNumber(scaleName); i++) {
    XtSetArg(wargs[0], XmNuserData, i);
    if (i <= XtNumber(scaleName) - 4) 
      w = XtCreateManagedWidget(scaleName[i], xmPushButtonWidgetClass, 
                                area_1, wargs, 1);
    else
      w = XtCreateManagedWidget(scaleName[i], xmPushButtonWidgetClass, 
                                area_2, wargs, 1); 
    XtAddCallback(w, XmNactivateCallback,
                  (XtCallbackProc) set_scale, data);
  } /* for */
  XtSetArg(wargs[0], XmNheight, &height);
  XtGetValues(w, wargs, 1);
  data->curves.scalingFrameHeight = height;
  height += fs->ascent/2;
  /* create text area */
  
  y = 0;
  for (i = 0; i < 2; i++) {
    n = 0;
    XtSetArg(wargs[n], XmNx, 0); n++; 
    XtSetArg(wargs[n], XmNy, y); n++; 
    XtSetArg(wargs[n], XmNwidth, SAMPLE_WIDTH); n++;
    XtSetArg(wargs[n], XmNheight, height); n++;
    data->curves.textArea[i] = XtCreateManagedWidget("text", xmTextWidgetClass,
                                       text_area, wargs, n);
    y += height;
  }
}

/*
** create variable widgets
*/

static void create_variable_area(Widget parent, CurvesWidget data) {
  int i, n, current_y, height;
  Arg wargs[10];
  Widget variable_area;
  Widget variable_com, common;
  char *variableName;
  
  height=(data->curves.scalingFrameHeight+3) * n_drawable_curves(data); 
  n = 0;
  XtSetArg(wargs[n], XmNx, 0); n++; 
  XtSetArg(wargs[n], XmNy, 0); n++; 
  XtSetArg(wargs[n], XmNwidth, FORMULAR_WIDTH); n++;
  XtSetArg(wargs[n], XmNheight, height); n++;
  data->curves.scaleFormular = XtCreateManagedWidget("formular", 
                                      xmDrawingAreaWidgetClass,
                                      parent, wargs, n);
  XtAddCallback(data->curves.scaleFormular,
                  XmNexposeCallback,
                  (XtCallbackProc) scale_expose, data); 
  variable_area = XtCreateManagedWidget("variableName", xmFormWidgetClass,
                                        parent, NULL, 0);
  n = 0;
  XtSetArg(wargs[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetValues(variable_area, wargs, n);

  n = 0;
  XtSetArg(wargs[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(wargs[n], XmNtopOffset, 4); n++;
  XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(wargs[n], XmNleftWidget, variable_area); n++;
  XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetValues(data->curves.scaleFormular, wargs, n);  

  XtSetArg(wargs[0], XmNentryClass, xmToggleButtonWidgetClass);
  variable_com = XmCreateRadioBox(variable_area, "VariableName", wargs, 1);
  XtManageChild(variable_com);

  /* create variable buttons */
  for (i = 0; i < strlen(data->curves.y_list); i++) {
    current_y = (int)(data->curves.y_list[i] - 'A');
    if ((data->curves.data+current_y)->options != CURVE_NO_OPTIONS)
      continue;
    variableName = (char *)XtMalloc(sizeof(char) *
                 strlen((data->curves.store_data+current_y)->variable_name)+5);
    strcpy(variableName, (data->curves.store_data+current_y)->variable_name);
    strcat(variableName, "* =");
    n = 0;
    XtSetArg(wargs[n], XmNuserData, i); n++;
    if (i == 0) {
      XtSetArg(wargs[n], XmNset, True); n++;
    }
    common = XtCreateManagedWidget(variableName, xmToggleButtonWidgetClass,
                                  variable_com, wargs, n);
    XtAddCallback(common, XmNvalueChangedCallback,
                  (XtCallbackProc)  scale_variable, data); 
    XtFree((char *)variableName);
  } /* for */
}
  

/*
** create scaling command area
*/

static void create_scaling_command_area(Widget parent, CurvesWidget data) {
  Widget w; 
  int i;
  Arg wargs[1];
  
  for (i = 0; i < XtNumber(CURVES_COMMAND); i++) {
    XtSetArg(wargs[0], XmNuserData, i);
    w = XtCreateManagedWidget(CURVES_COMMAND[i], xmPushButtonWidgetClass, 
                       parent, wargs, 1);
    XtAddCallback(w, XmNactivateCallback,
                  (XtCallbackProc) scaling_treat_command, data);
  } /* for */

}


/*
** set up scaling widget
*/

static void setup_scaling_widget(Widget w) {
  CurvesWidget sw = (CurvesWidget)w;
  Widget topshell;
  Widget variable_area, scaling_area, command_area;
  int i, n;
  Arg wargs[10];
  XGCValues values;
  Display *dpy = XtDisplay(sw);
  ScalePtr tempPtr, ptr;

  if (sw->curves.isScaleSet)
    return;
  sw->curves.isScaleSet = True;
  sw->curves.scaleValid = True;
  sw->curves.current_scaling = 0;

  sw->curves.tempFunction = (ScalePtr *) XtMalloc(sizeof(ScalePtr) *
                                  (sw->curves.num_variable));
  sw->curves.tempName = (char **)XtMalloc(sizeof(char *) *
                           (sw->curves.num_variable));      
  for (i = 0; i < sw->curves.num_variable; i++) {
    *(sw->curves.tempFunction+i) = NULL;
    if ((ptr = *(sw->curves.scaleFunction+i)) != NULL) {
      tempPtr = (ScalePtr)XtMalloc(sizeof(struct CurvesScaleFunction));
      tempPtr->function_num = ptr->function_num;
      tempPtr->value = ptr->value;
      *(sw->curves.tempFunction+i) = tempPtr;    
      while (ptr->next != NULL) {
        tempPtr->next = (ScalePtr)XtMalloc(sizeof(struct CurvesScaleFunction));
        tempPtr->next->function_num = ptr->next->function_num;
        tempPtr->next->value = ptr->next->value;
        tempPtr = tempPtr->next;
        ptr = ptr->next;
      }
      tempPtr->next = NULL;
    }
    *(sw->curves.tempName+i) = (char *)XtMalloc(sizeof(char) *
                      strlen((sw->curves.data+i)->variable_name)+1);
    strcpy(*(sw->curves.tempName+i), (sw->curves.data+i)->variable_name);
  }
  /* create topshell widget */
  n = 0;
  XtSetArg(wargs[n], XmNautoUnmanage, False); n++;
  XtSetArg(wargs[n], XmNtitle, "Scaling Variables"); n++;
  topshell = XmCreateFormDialog (w, "scaleVariables", wargs, n);
  sw->curves.scale = topshell;
  /* create frame widget */
  variable_area = XtCreateManagedWidget("selection", xmFormWidgetClass,
                                        topshell, NULL, 0);
  scaling_area = XtCreateManagedWidget("symbol", xmFormWidgetClass,
                                      topshell, NULL, 0);
  command_area = XtCreateManagedWidget("command", xmRowColumnWidgetClass,
                                      topshell, NULL, 0);
                                      
  n = 0;
  XtSetArg(wargs[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetValues(variable_area, wargs, n);

  n = 0;
  XtSetArg(wargs[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(wargs[n], XmNtopWidget, variable_area); n++;
  XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetValues(scaling_area, wargs, n);
  
  n = 0;
  XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(wargs[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(wargs[n], XmNtopWidget, scaling_area); n++;
  XtSetArg(wargs[n], XmNorientation, XmHORIZONTAL); n++;
  XtSetValues(command_area, wargs, n);
  
  n = 0;
  XtSetArg(wargs[n], XtNforeground, &(values.foreground)); n++;
  XtSetArg(wargs[n], XtNbackground, &(values.background)); n++;
  XtGetValues(topshell, wargs, n);  
  values.font = getFontStruct(sw->curves.font)->fid;
  sw->curves.scaleGC = XCreateGC(dpy, XDefaultRootWindow(dpy), GCForeground |
                       GCBackground | GCFont, &values); 
  create_scaling_area(scaling_area, sw);                       
  create_variable_area(variable_area, sw);
  create_scaling_command_area(command_area, sw);
  XtManageChild(topshell);                                  
}

/*
** call back function for setting mark and line style 
*/

void CurvesSetMarkLineStyle(Widget w)
{
   setup_setting_widget(w);
}

/*
** call back function for showing legend
*/

void CurvesShowLegend(Widget w, Boolean showLegend) {
  CurvesWidget sw = (CurvesWidget)w;
  
  sw->curves.showLegend = showLegend;
  resize(sw);
  redisplayContents(sw, X_SCREEN, REDRAW_ALL);
}

/* 
** reset zoom callback function
*/

void CurvesResetZoom(Widget w) {
  CurvesWidget sw = (CurvesWidget)w;
  
  DataRange(sw, &sw->curves.maxXData, &sw->curves.minXData,
  	&sw->curves.maxYData, &sw->curves.minYData, False);
  sw->curves.minXLim = sw->curves.minXData;
  sw->curves.minYLim = sw->curves.minYData;
  sw->curves.maxXLim = sw->curves.maxXData;
  sw->curves.maxYLim = sw->curves.maxYData;
  redisplayContents(sw, X_SCREEN, REDRAW_ALL);
}

/*
** scale variables
*/

void CurvesScaleVariables(Widget w) 
{
  setup_scaling_widget(w);
}

/*
** Set Mark and line styles explicitly
*/
void CurvesSetCurveStyle(Widget w, int index, int markNum, int lineNum)
{
    CurvesWidget sw = (CurvesWidget)w;
    
    (sw->curves.data+index)->mark_num = markNum;
    (sw->curves.data+index)->line_num = lineNum;
    sw->curves.updateMarkLine[index*2] = markNum;
    sw->curves.updateMarkLine[index*2+1] = lineNum; 
}

/*
** return the number of curves which are actually drawable (not parts of 
** error bars
*/
static int n_drawable_curves(CurvesWidget data)
{
  int n_curves = strlen(data->curves.y_list), n = 0;
  int i, current_y;
  
  for (i = 0; i < n_curves; i++) {
    current_y = (int)(data->curves.y_list[i]-'A');
    if ((data->curves.data+current_y)->options == CURVE_NO_OPTIONS)
      n++;
  }
  return n;
}

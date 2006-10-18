/*******************************************************************************
*									       *
* XY.c -- General Purpose Plot Widget					       *
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
* May 28, 1992								       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#if XmVersion == 1002
#include <Xm/PrimitiveP.h>
#endif
#include "../util/psUtils.h"
#include "drawAxes.h"
#include "dragAxes.h"
#include "XYP.h"

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
#define LEGEND_TOP_MARGIN 5	/* space between legend and y axis label */
#define LEGEND_LEFT_MARGIN 5	/* space to the left of margin */
#define LEGEND_RIGHT_MARGIN 5	/* space to the right of margin */
#define LEGEND_LINE_LEN 40	/* width of example lines in plot legend */
#define LEGEND_NAME_SPACING 5	/* space between sample line and name */
#define LEGEND_COLUMN_MARGIN 20	/* space between columns in legend */
#define MAX_AXIS_PERCENT 22	/* maximum percentage of widget width that may
				   be used for drawing axes */
#define TINY_MARK_SIZE 2	/* width/height for small marker symbols */
#define SMALL_MARK_SIZE 4	/* width/height for small marker symbols */
#define MED_MARK_SIZE 6		/* width/height for medium marker symbols */
#define LARGE_MARK_SIZE 8	/* width/height for large marker symbols */
#define ERROR_MARKER_RATIO 1.25	/* ratio of error bar end to marker size */
#define SPACING_MASK 0xff   	/* mask for spacing component of fill style */
#define H_SLOPE_MASK 0xff00  	/* " " horizontal lines slope component " " */
#define V_SLOPE_MASK 0xff0000 	/* " " vertical lines slope component " " */
#define LINE_STYLE_MASK 0xff000000 /* " " line style component " " */
#define H_SLOPE_OFFSET 8    	/* offset for extracting horiz. slope */
#define V_SLOPE_OFFSET 16   	/* offset for extracting vert. slope */
#define LINE_STYLE_OFFSET 24	/* offset for extracting line style */

/* header of a list for accumulating figures (XRectangles, XArcs, XSegments)
   for drawing markers independent of their composition */
typedef struct {
    int style;
    int nFigures;
    char *list;
    char *fillPtr;
} markStruct;

/* header for a list for acumulating line segments in connected groups so that
   they can be drawn with a continuous dash pattern, and independent of their
   destination (FloatPoints for PSFloatDrawLines and XPoints for XDrawLines) */
typedef struct {
    int outDevice;
    XPoint *points;
    FloatPoint *floatPoints;
    int *lineCounts;
    int *curLineCount;
    XPoint *fillPtr;
    FloatPoint *fFillPtr;
} lineStruct;

/* legend is drawn differently to save space when user doesn't have
   lines and/or markers on any of the curves */
enum legendStyles {NO_MARKERS, NO_LINES, FULL_LEGEND};

static void motionAP(XYWidget w, XEvent *event, char *args, int n_args);
static void btnUpAP(XYWidget w, XEvent *event, char *args, int n_args);
static void btn2AP(XYWidget w, XEvent *event, char *args, int n_args);
static void btn3AP(XYWidget w, XEvent *event, char *args, int n_args);
static void initialize(XYWidget request, XYWidget new);
static void redisplay(XYWidget w, XEvent *event, Region region);
static void redisplayContents(XYWidget w, int outDevice, int redrawArea);
static void destroy(XYWidget w);
static void resize(XYWidget w);
static void resizeParts(XYWidget w);
static void resetContentsGCClipping(XYWidget w);
static Boolean setValues(XYWidget current, XYWidget request,XYWidget new);
static void setCurves(Widget w, XYCurve *curves, int nCurves, int rescaleMode,
    	int dataOnly, int redisplay);
static void setHistograms(Widget w, XYHistogram *histograms, int nHistograms,
    	int rescaleMode, int dataOnly, int redisplay);
static int resetDataAndViewRanges(XYWidget w, int oldNBins, int rescaleMode);
static void drawCurve(XYWidget w, Drawable drawBuf, int outDevice, XYCurve
	*curve);
static void drawHistogram(XYWidget w, Drawable drawBuf, int outDevice,
    	XYHistogram *histo);
static void drawHistOutline(XYWidget w, Drawable drawBuf, int outDevice,
    	XYHistogram *histo);
static void drawHistMarkers(XYWidget w, Drawable drawBuf, int outDevice,
    	XYHistogram *histo);
static void drawHistErrBars(XYWidget w, Drawable drawBuf, int outDevice,
    	XYHistogram *histo);
static void fillHistogram(XYWidget w, Drawable drawBuf, int outDevice,
    	XYHistogram *histo);
static void drawFill(Display *display, Drawable drawBuf, GC gc,
	int outDevice, int xMin, int yMin, int xMax, int yMax,
	unsigned fillStyle, Pixel fillPixel);
static void startLineList(lineStruct *lineList, int outDevice, int maxSegs);
static void addToLineList(lineStruct *lineList, float x, float y, int restart,
	int round);
static void drawLineList(Display *display, Drawable drawBuf, GC gc,
	int outDevice, lineStruct *lineList, int lineStyle, Pixel linePixel);
static void setLineStyle(Display *display, GC gc, int style, Pixel pixel);
static void startMarkList(markStruct *markList, int markerStyle, int maxLength);
static void addToMarkList(markStruct *markList, int x, int y, int size);
static void drawMarkList(Display *display, Drawable drawBuf, GC gc,
	int outDevice, markStruct *markList, int size, int style, Pixel color);
static int tryLegendLayout(XYWidget w, int nRows, int *width, int *nColumns);
static int legendColumnStyle(XYCurve *curves, int nCurves, XYHistogram *hists,
    	int nHists, int colNum, int nRows);
static void layoutLegend(XYWidget w, int *nRows, int *legendHeight,
	int *columnWidth, int *leftMargin);
static void drawLegend(XYWidget w, Drawable drawBuf, int outDevice);
static void drawMarker(Display *display, Drawable drawBuf, GC gc, int outDevice,
	int size, int style, Pixel color, int x, int y);
static void drawLine(Display *display, Drawable drawBuf, GC gc, int outDevice,
	int lineStyle, Pixel linePixel, int x1, int y1, int x2, int y2);
static void updateBufferAllocation(XYWidget w);
static XFontStruct *getFontStruct(XmFontList font);
static void calcDataRange(XYWidget w, double *xMin, double *xMax, double *yMin,
	double *yMax, double *xPosMin, double *yPosMin);
static int oneCurveDataRange(XYWidget w, XYCurve *curve, double *xMin,
	double *xMax, double *yMin, double *yMax, double *xPosMin,
	double *yPosMin);
static void calcHistDataRange(XYWidget w, XYHistogram *hist, double *xMin,
	double *xMax, double *yMin, double *yMax, double *xPosMin,
	double *yPosMin);
static void copyCurveData(XYCurve *fromCurve, XYCurve *toCurve, int dataOnly);
static void copyHistogramData(XYHistogram *fromHisto, XYHistogram *toHistogram,
    	int dataOnly);
static void copyCurveStyle(XYCurve *fromCurve, XYCurve *toCurve);
static void copyHistogramStyle(XYHistogram *fromHisto, XYHistogram *toHisto);
static void freeCurveData(XYCurve *curve, int dataOnly);
static void freeHistogramData(XYHistogram *histo, int dataOnly);
static int widgetHasData(XYWidget w);
static void expandLogRange(double *min, double *max, double fraction);
static double dMin(double d1, double d2);
static double dMax(double d1, double d2);

/* Line dash styles (defined as strings for convenience, but note that the
   numbers are in octal) */
static char *DashLists[] = {
    "\01" /* XY_NO_LINE (not used) */,
    "\01" /* XY_PLAIN_LINE (used w/dashes off) */,
    "\01\01" /* XY_FINE_DASH */,
    "\02\02" /* XY_MED_FINE_DASH */,
    "\04\04" /* XY_DASH */,
    "\06\06" /* XY_LONG_DASH */,
    "\016\06" /* XY_X_LONG_DASH */,
    "\020\03\02\03" /* XY_1_DOT_DASH */,
    "\016\03\02\03\02\03" /* XY_2_DOT_DASH */,
    "\013\03\02\03\02\03\02\03" /* XY_3_DOT_DASH */,
    "\011\03\02\03\02\03\02\03\02\03" /* XY_4_DOT_DASH */,
    "\01" /* XY_THICK_LINE (used w/dashes off) */,
    "\01" /* XY_X_THICK_LINE (used w/dashes off) */
};

/* Table to translate mark width index to actual width in pixels */
static char MarkWidths[] = {TINY_MARK_SIZE, SMALL_MARK_SIZE,
	MED_MARK_SIZE, LARGE_MARK_SIZE};

/* Table to translate additional histogram "line" styles into marker types
   and sizes for marker-style histograms */
static struct {
    char style;
    char size;
} HistMarkerStyles[XY_N_HIST_LINE_STYLES - XY_N_LINE_STYLES] = {
	{XY_SQUARE_MARK, XY_MEDIUM}, {XY_CIRCLE_MARK, XY_MEDIUM},
	{XY_X_MARK, XY_MEDIUM}, {XY_SOLID_SQUARE_MARK, XY_MEDIUM},
	{XY_SOLID_CIRCLE_MARK, XY_MEDIUM}
};

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
      XtOffset(XYWidget, xy.doubleBuffer), XmRString, "False"},
    {XmNshowLegend, XmCShowLegend, XmRBoolean, sizeof(Boolean),
      XtOffset(XYWidget, xy.showLegend), XmRString, "True"},
    {XmNxLogScaling, XmCXLogScaling, XmRBoolean, sizeof(Boolean),
      XtOffset(XYWidget, xy.xLogScaling), XmRString, "False"},
    {XmNyLogScaling, XmCYLogScaling, XmRBoolean, sizeof(Boolean),
      XtOffset(XYWidget, xy.yLogScaling), XmRString, "False"},
    {XmNmarginPercent, XmCMarginPercent, XmRInt, sizeof(int),
      XtOffset(XYWidget, xy.marginPercent), XmRString, "4"},
    {XmNfontList, XmCFontList, XmRFontList, sizeof(XmFontList),
      XtOffset(XYWidget, xy.font), XmRImmediate, NULL},
    {XmNxAxisLabel, XmCXAxisLabel, XmRXmString, sizeof (XmString), 
      XtOffset(XYWidget, xy.xAxisLabel), XmRString, NULL},
    {XmNyAxisLabel, XmCYAxisLabel, XmRXmString, sizeof (XmString), 
      XtOffset(XYWidget, xy.yAxisLabel), XmRString, NULL},
    {XmNresizeCallback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (XYWidget, xy.resize), XtRCallback, NULL},
    {XmNbtn2Callback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (XYWidget, xy.btn2), XtRCallback, NULL},
    {XmNbtn3Callback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (XYWidget, xy.btn3), XtRCallback, NULL},
    {XmNredisplayCallback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (XYWidget, xy.redisplay), XtRCallback, NULL},
};

XYClassRec  xyClassRec = {
     /* CoreClassPart */
  {
    (WidgetClass) &xmPrimitiveClassRec,  /* superclass       */
    "XY",                         /* class_name            */
    sizeof(XYRec),                /* widget_size           */
    NULL,                           /* class_initialize      */
    NULL,                           /* class_part_initialize */
    FALSE,                          /* class_inited          */
    (XtInitProc)initialize,         /* initialize            */
    NULL,                           /* initialize_hook       */
    XtInheritRealize,               /* realize               */
    actionsList,                    /* actions               */
    XtNumber(actionsList),          /* num_actions           */
    resources,                      /* resources             */
    XtNumber(resources),            /* num_resources         */
    NULLQUARK,                      /* xrm_class             */
    TRUE,                           /* compress_motion       */
    TRUE,                           /* compress_exposure     */
    TRUE,                           /* compress_enterleave   */
    TRUE,                           /* visible_interest      */
    (XtWidgetProc)destroy,          /* destroy               */
    (XtWidgetProc)resize,           /* resize                */
    (XtExposeProc)redisplay,        /* expose                */
    (XtSetValuesFunc)setValues,     /* set_values            */
    NULL,                           /* set_values_hook       */
    XtInheritSetValuesAlmost,       /* set_values_almost     */
    NULL,                           /* get_values_hook       */
    NULL,                           /* accept_focus          */
    XtVersion,                      /* version               */
    NULL,                           /* callback private      */
    defaultTranslations,            /* tm_table              */
    NULL,                           /* query_geometry        */
    NULL,                           /* display_accelerator   */
    NULL,                           /* extension             */
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
  /* XY class part */
  {
    0,                              	/* ignored	                */
  }
};

WidgetClass xyWidgetClass = (WidgetClass)&xyClassRec;

/*
** Widget initialize method
*/
static void initialize(XYWidget request, XYWidget new)
{
    XGCValues values;
    Display *display = XtDisplay(new);
   
    /* Make sure the window size is not zero. The Core 
       initialize() method doesn't do this. */
    if (request->core.width == 0)
    	new->core.width = 500;
    if (request->core.height == 0)
   	new->core.height = 400;

    /* Make a local copy of the fontlist,
       or get the default if not specified */
    if (new->xy.font == NULL)
    	new->xy.font =
    	    XmFontListCopy(_XmGetDefaultFontList(
    	    	    (Widget) new, XmLABEL_FONTLIST));
    else
        new->xy.font = XmFontListCopy(new->xy.font);

    /* Make local copies of the XmStrings */
    if (new->xy.xAxisLabel != NULL)
    	new->xy.xAxisLabel = XmStringCopy(new->xy.xAxisLabel);
    if (new->xy.yAxisLabel != NULL)
    	new->xy.yAxisLabel = XmStringCopy(new->xy.yAxisLabel);
     
    /* Create graphics contexts for drawing in the widget */
    values.font = getFontStruct(new->xy.font)->fid;
    values.foreground = new->primitive.foreground;
    values.background = new->core.background_pixel;
    new->xy.gc = XCreateGC(display, XDefaultRootWindow(display),
    	    GCForeground|GCBackground|GCFont, &values);
    new->xy.contentsGC = XCreateGC(display, XDefaultRootWindow(display),
    	    GCForeground|GCBackground|GCFont, &values);
    
    /* Initialize various fields */
    ResetAxisDragging(&new->xy.dragState);
    new->xy.curves = NULL;
    new->xy.nCurves = 0;
    new->xy.strings = NULL;
    new->xy.nStrings = 0;
    new->xy.histograms = NULL;
    new->xy.nHistograms = 0;
    new->xy.drawBuffer = 0;
    new->xy.xEnd = 0;
    new->xy.xOrigin = 0;
    new->xy.yOrigin = 0;
    new->xy.yEnd = 0;
    new->xy.toRedraw = REDRAW_NONE;
    
    /* Default plotting boundaries */
    setCurves((Widget)new, NULL, 0, XY_RESCALE, False, False);

    /* Set size dependent items */
    resize(new);
}

/*
** Widget destroy method
*/
static void destroy(XYWidget w)
{
    int i;
    
    XFreeGC(XtDisplay(w), w->xy.gc);
    XFreeGC(XtDisplay(w), w->xy.contentsGC);
    if (w->xy.font != NULL)
    	XmFontListFree(w->xy.font);
    if (w->xy.drawBuffer)
    	XFreePixmap(XtDisplay(w), w->xy.drawBuffer);
    XtRemoveAllCallbacks ((Widget)w, XmNresizeCallback);
    XtRemoveAllCallbacks ((Widget)w, XmNbtn2Callback);
    XtRemoveAllCallbacks ((Widget)w, XmNbtn3Callback);
    XtRemoveAllCallbacks ((Widget)w, XmNredisplayCallback);
    if (w->xy.xAxisLabel != NULL)
    	XmStringFree(w->xy.xAxisLabel);
    if (w->xy.yAxisLabel != NULL)
    	XmStringFree(w->xy.yAxisLabel);
    if (w->xy.curves != NULL) {
    	for (i=0; i<w->xy.nCurves; i++)
    	    freeCurveData(&w->xy.curves[i], False);
    	XtFree((char *) w->xy.curves);
    }
    if (w->xy.histograms != NULL) {
    	for (i=0; i<w->xy.nHistograms; i++)
    	    freeHistogramData(&w->xy.histograms[i], False);
    	XtFree((char *) w->xy.histograms);
    }
    for (i=0; i<w->xy.nStrings; i++) {
    	XmFontListFree(w->xy.strings[i].font);
    	XmStringFree(w->xy.strings[i].string);
    }
    if (w->xy.nStrings != 0)
    	XtFree((char *)w->xy.strings);
}

/*
** Widget resize method.  Called when the total size of the widget changes
*/
static void resize(XYWidget w)
{
    /* resize the drawing buffer, an offscreen pixmap for smoother animation */
    updateBufferAllocation(w);
    
    /* change size dependent values in the widget */
    resizeParts(w);
   
    /* call the resize callback */
    if (XtIsRealized((Widget)w)) {
	XYCallbackStruct cbStruct;
	XYTransform xform;
	ComputeTransform(w, &xform);
	cbStruct.reason = XmCR_RESIZE;
	cbStruct.event = NULL;
	cbStruct.xform = &xform;
	XtCallCallbacks((Widget)w, XmNresizeCallback, (XtPointer)&cbStruct);
    }
}

/*
** Internal resize procedure, called when the window is resized, or when
** something within the widget changes size (such as when labels are added or
** removed, or the legend changes size).  Re-calculates the sizes of the
** various part of the widget (highlights, shadows, axis labels, plot
** contents, plot legend, margins, etc.) and the size dependent elements
** in the widget instance data structure.
*/
static void resizeParts(XYWidget w)
{
    XFontStruct *fs = getFontStruct(w->xy.font);
    int legendHeight, borderWidth =
    	    w->primitive.shadow_thickness + w->primitive.highlight_thickness;
    XRectangle clipRect;

    /* calculate the area of the widget where contents can be drawn */
    w->xy.xMin = borderWidth;
    w->xy.yMin = borderWidth;
    w->xy.xMax = w->core.width - borderWidth;
    w->xy.yMax = w->core.height - borderWidth;

    /* calculate how the space within the drawable area should be divided
       horizontally (layoutLegend below uses this information) */
    w->xy.axisLeft = w->xy.xMin + LEFT_MARGIN;
    w->xy.xOrigin = w->xy.axisLeft + VAxisWidth(fs);
    if (w->xy.xOrigin > (w->xy.xMax - w->xy.xMin) * MAX_AXIS_PERCENT/100)
    	w->xy.xOrigin = (w->xy.xMax - w->xy.xMin) * MAX_AXIS_PERCENT/100;
    w->xy.axisRight = w->xy.xMax - RIGHT_MARGIN;
    w->xy.xEnd = w->xy.axisRight - HAxisEndClearance(fs);
    if (w->xy.xMax - w->xy.xEnd > w->xy.xOrigin - w->xy.xMin)
    	w->xy.xEnd = w->xy.xMax - (w->xy.xOrigin - w->xy.xMin);

    /* find out how high the legend will need to be and how many rows it has */
    layoutLegend(w, &w->xy.legendRows, &legendHeight,
    	    &w->xy.legendColumnSpacing, &w->xy.legendLeftMargin);
    
    /* calculate how the space within the drawable area should be divided
       vertically, depending the sizes of the labels and plot legend */
    w->xy.legendTop = w->xy.yMax - BOTTOM_MARGIN - legendHeight;
    if (w->xy.yAxisLabel != NULL)
    	w->xy.yEnd = w->xy.yMin + fs->ascent + fs->descent + TOP_MARGIN;
    else
    	w->xy.yEnd = VAxisEndClearance(fs) + fs->ascent/2 + TOP_MARGIN;
    w->xy.axisTop = w->xy.yEnd - VAxisEndClearance(fs);
    w->xy.axisBottom = w->xy.yMax - BOTTOM_MARGIN - legendHeight -
    	    (legendHeight == 0 ? 0 : LEGEND_TOP_MARGIN) -
    	    (w->xy.xAxisLabel == NULL ? fs->ascent/2 :
    	     fs->ascent + fs->descent + X_LABEL_MARGIN);
    w->xy.yOrigin = w->xy.axisBottom - HAxisHeight(fs);
    
    /* set plot contents gc to clip drawing at the edges */
    resetContentsGCClipping(w);

    /* set drawing gc to clip drawing before motif shadow and highlight */
    clipRect.x = borderWidth;
    clipRect.y = borderWidth;
    clipRect.width = w->core.width - 2 * borderWidth;
    clipRect.height = w->core.height - 2 * borderWidth;
    XSetClipRectangles(XtDisplay(w), w->xy.gc, 0, 0, &clipRect, 1, Unsorted);
}

/*
** Set clipping on the plot contents BG to the full data (non-axis) area of
** the widget.  (Actually, sets the right edge one pixel beyond the computed
** boundary to make drawing simpler for histograms, since rounding can
** sometimes place the important closing line on the right off the display).
*/
static void resetContentsGCClipping(XYWidget w)
{
    XRectangle clipRect;
    
    clipRect.x = w->xy.xOrigin;
    clipRect.y = w->xy.yEnd;
    clipRect.width = w->xy.xEnd - w->xy.xOrigin + 1;
    clipRect.height = w->xy.yOrigin - w->xy.yEnd;
    XSetClipRectangles(XtDisplay(w), w->xy.contentsGC, 0, 0, &clipRect,
    	    1, Unsorted);
}

/*
** Widget redisplay method
*/
static void redisplay(XYWidget w, XEvent *event, Region region)
{
    /* Draw the Motif required shadows and highlights */
    if (w->primitive.shadow_thickness > 0) {
	_XmDrawShadow (XtDisplay(w), XtWindow(w), w->primitive.bottom_shadow_GC,
		w->primitive.top_shadow_GC, w->primitive.shadow_thickness,
		w->primitive.highlight_thickness,
		w->primitive.highlight_thickness,
		w->core.width - 2 * w->primitive.highlight_thickness,
		w->core.height-2 * w->primitive.highlight_thickness);
    }
    if (w->primitive.highlighted)
	_XmHighlightBorder((Widget)w);
    else if (_XmDifferentBackground((Widget)w, XtParent((Widget)w)))
	_XmUnhighlightBorder((Widget)w);
    
    /* Now draw the contents of the xy widget */
    redisplayContents(w, X_SCREEN, REDRAW_ALL);
}

/*
** Widget setValues method
*/
static Boolean setValues(XYWidget current, XYWidget request, XYWidget new)
{
    Boolean redraw = False, doResize = False;
    Display *display = XtDisplay(new);

    /* If the background color has changed, change the GCs */
    if (new->core.background_pixel !=current->core.background_pixel) {
    	XSetBackground(display, new->xy.gc, new->core.background_pixel);
    	XSetBackground(display,new->xy.contentsGC,new->core.background_pixel);
    	redraw = TRUE;
    }
    
    /* if the foreground color has changed, redraw */
    if (new->primitive.foreground != current->primitive.foreground)
    	redraw = TRUE;

    /* if double buffering changes, allocate or deallocate offscreen pixmap */
    if (new->xy.doubleBuffer != current->xy.doubleBuffer) {
    	updateBufferAllocation(new);
    	redraw = TRUE;
    }

    /* if legend is turned on or off, resize everything */
    if (new->xy.showLegend != current->xy.showLegend)
    	doResize = TRUE;

    /* if log scaling changes, re-calculate data range if the plot has a
       margin, verify data (and set back to linear if data can't handle
       log scaling), reset dragging, and redraw */
    if   (new->xy.xLogScaling != current->xy.xLogScaling ||
    	  new->xy.yLogScaling != current->xy.yLogScaling) {
    	resetDataAndViewRanges((XYWidget)new, 0, XY_NO_RESCALE);
    	ResetAxisDragging(&new->xy.dragState);
    	redraw = TRUE;
    }

    /* if labels are changed, free the old ones and copy the new ones */
    if (new->xy.xAxisLabel != current->xy.xAxisLabel) {
    	if (current->xy.xAxisLabel != NULL)
    	    XmStringFree(current->xy.xAxisLabel);
    	new->xy.xAxisLabel = XmStringCopy(new->xy.xAxisLabel);
    	doResize = TRUE;
    }
    if (new->xy.yAxisLabel != current->xy.yAxisLabel) {
    	if (current->xy.yAxisLabel != NULL)
    	    XmStringFree(current->xy.yAxisLabel);
    	new->xy.yAxisLabel = XmStringCopy(new->xy.yAxisLabel);
    	doResize = TRUE;
    }
    
    /* if highlight thickness or shadow thickness changed, resize and redraw */
    if  ((new->primitive.highlight_thickness != 
    	    current->primitive.highlight_thickness) ||
         (new->primitive.shadow_thickness !=
    	    current->primitive.shadow_thickness)) {
    	doResize = TRUE;
    }
    if (doResize) {
    	resize(new);
    	return TRUE;
    }
    return redraw; 
} 

/*
** Button press and button motion action proc.
*/
static void motionAP(XYWidget w, XEvent *event, char *args, int n_args)
{
    int chgdArea, redrawArea = REDRAW_NONE;

    if (event->type == ButtonPress)
    	XmProcessTraversal((Widget)w, XmTRAVERSE_CURRENT);
    
    chgdArea = ExtDragAxes(event, w->xy.xOrigin, w->xy.xEnd, w->xy.yOrigin,
    	w->xy.yEnd, w->xy.axisLeft, w->xy.axisTop, w->xy.axisBottom,
    	w->xy.axisRight, w->xy.minXData, w->xy.maxXData, w->xy.minYData,
    	w->xy.maxYData, w->xy.xLogScaling, w->xy.yLogScaling, &w->xy.minXLim,
    	&w->xy.maxXLim, &w->xy.minYLim, &w->xy.maxYLim, &w->xy.minXDragStop,
    	&w->xy.maxXDragStop, &w->xy.minYDragStop, &w->xy.maxYDragStop,
    	&w->xy.dragState, &w->xy.xDragStart, &w->xy.yDragStart);
 
    if (chgdArea & DA_REDRAW_H_AXIS) redrawArea |= REDRAW_H_AXIS;
    if (chgdArea & DA_REDRAW_V_AXIS) redrawArea |= REDRAW_V_AXIS;
    if (chgdArea & DA_REDRAW_CONTENTS) redrawArea |= REDRAW_CONTENTS;

    redisplayContents(w, X_SCREEN, redrawArea);
}
/*
** Button up action proc.
*/
static void btnUpAP(XYWidget w, XEvent *event, char *args, int n_args)
{
    ResetAxisDragging(&w->xy.dragState);
}

static void btn2AP(XYWidget w, XEvent *event, char *args, int n_args)
{
    XYCallbackStruct cbStruct;
    XYTransform xform;

    XmProcessTraversal((Widget)w, XmTRAVERSE_CURRENT);

    /* Compute constants for user coordinate transformation */
    ComputeTransform(w, &xform);
    
    /* Call the callback */
    cbStruct.reason = XmCR_INPUT;
    cbStruct.event = event;
    cbStruct.xform = &xform;
    XtCallCallbacks((Widget)w, XmNbtn2Callback, (XtPointer)&cbStruct);
}

static void btn3AP(XYWidget w, XEvent *event, char *args, int n_args)
{    
    XYCallbackStruct cbStruct;
    XYTransform xform;

    XmProcessTraversal((Widget)w, XmTRAVERSE_CURRENT);

    /* Compute constants for user coordinate transformation */
    ComputeTransform(w, &xform);

    /* Just call the callback */
    cbStruct.reason = XmCR_INPUT;
    cbStruct.event = NULL;
    cbStruct.xform = &xform;
    XtCallCallbacks((Widget)w, XmNbtn3Callback, (XtPointer)&cbStruct);
}

/*
** XYSetCurves
**
** Specify points and/or lines to be displayed by the xy widget in the form
** of sets of points in 2D space.  Each set has a distinct marker and/or
** connecting line style, and possibly associated error bars (horizontal
** and/or vertical).  The data and appearance information for each set is
** called a curve.  XYSetContents takes an array of XYCurve data structures
** which contain all of this data, and passes it along to the XY widget to
** display.
**
** Parameters
**
** 	w		An xy widget
**	curves		An array of curves to display
**	nCurves		The number of curves specified in curves
**	rescaleMode	One of: XY_NO_RESCALE, XY_RESCALE, or
**			XY_GROW_ONLY.  Tells the widget how to change
**			its display to incorporate the new contents.
**  	redisplay   	True to refresh display, False to hold screen update 
**  	    	    	until a later call (to prevent unnecessary flashing).
*/
void XYSetCurves(Widget w, XYCurve *curves, int nCurves, int rescaleMode,
    	int redisplay)
{
    setCurves(w, curves, nCurves, rescaleMode, False, redisplay);
}

/*
** XYCurveUpdateStyles
**
** Update the style information (marker style, line style, color, and marker
** size) for the curves currently displayed by the widget, keeping the
** existing data, and ignoring the data fields in the curves structures.
** The number of curves must be the same as the currently displayed data
**
** Parameters
**
** 	w		An xy widget
**	curves		An array of curve data structures containing the
**			new style information, with the same number of 
**			elements as were passed originally in the nCurves
**			argument to XYSetContents.
**  	redisplay   	True to refresh display, False to hold screen update 
**  	    	    	until a later call (to prevent unnecessary flashing).
*/
void XYUpdateCurveStyles(Widget w, XYCurve *curves, int redisplay)
{
    int i;
    
    for (i=0; i<((XYWidget)w)->xy.nCurves; i++) {
    	XmStringFree(((XYWidget)w)->xy.curves[i].name);
    	copyCurveStyle(&curves[i], &((XYWidget)w)->xy.curves[i]);
    }
    if (redisplay && XtIsRealized(w))
    	redisplayContents((XYWidget)w, X_SCREEN, REDRAW_ALL);
    else
    	((XYWidget)w)->xy.toRedraw |= REDRAW_ALL;
}

/*
** XYUpdateCurveData
**
** Update the curves displayed by the widget, using the new data in the
** curves structure, ignoring style, color, and name information.
** The number of curves must be the same as the currently displayed data
**
** Parameters
**
** 	w		An xy widget
**	curves		An array of curve data structures containing the
**			new data to display, with the same number of 
**			elements as were passed originally in the nCurves
**			argument to XYSetContents.
**	rescaleMode	One of: XY_NO_RESCALE, XY_RESCALE, or
**			XY_GROW_ONLY.  Tells the widget how to change
**			its display to incorporate the new contents.
**  	redisplay   	True to refresh display, False to hold screen update 
**  	    	    	until a later call (to prevent unnecessary flashing).
*/
void XYUpdateCurveData(Widget w, XYCurve *curves, int rescaleMode,
    	int redisplay)
{
    setCurves(w, curves, ((XYWidget)w)->xy.nCurves, rescaleMode, False,
    	    redisplay);
}

static void setCurves(Widget w, XYCurve *curves, int nCurves,
    	int rescaleMode, int dataOnly, int redisplay)
{
    XYWidget xyW = (XYWidget)w;
    int i;
    int doResize = nCurves != xyW->xy.nCurves && xyW->xy.showLegend;

    /* Free the existing data (but preserve curve structures in dataOnly
       case to save appearance information) */
    if (xyW->xy.nCurves != 0) {
    	if (dataOnly) {
    	    for (i=0; i<xyW->xy.nCurves; i++)
    		freeCurveData(&xyW->xy.curves[i], True);
    	} else {
    	    for (i=0; i<xyW->xy.nCurves; i++)
    		freeCurveData(&xyW->xy.curves[i], False);
    	    XtFree((char *)xyW->xy.curves);
    	}
    }
    
    /* Copy in the user's data */
    if (nCurves != 0) {
    	if (!dataOnly) {
    	    xyW->xy.curves = (XYCurve *)XtMalloc(sizeof(XYCurve) * nCurves);
    	    xyW->xy.nCurves = nCurves;
    	}
    	for (i=0; i<nCurves; i++)
	    copyCurveData(&curves[i], &xyW->xy.curves[i], dataOnly);
    } else {
    	xyW->xy.nCurves = 0;
    	xyW->xy.curves = NULL;
    }
    
    /* if the number of curves changed, the legend may have changed size,
       so recalculate the size-dependent parts of the widget */
    if (doResize) {
    	resizeParts(xyW);
    	xyW->xy.toRedraw |= REDRAW_ALL;
    }
    
    /* Scan the data and calculate the new data and view ranges */
    xyW->xy.toRedraw |= resetDataAndViewRanges(xyW, 0, rescaleMode);
    
    /* redraw the widget with the new data, unless inhibited by redisplay */
    if (redisplay && XtIsRealized(w))
    	redisplayContents(xyW, X_SCREEN, xyW->xy.toRedraw);
}
/*
** XYSetHistograms
**
** Specify both data and appearance informations for one or more histograms
** presented in the form of XYHistogram data structures.
**
** Parameters
**
** 	w		An xy widget
**	histograms	An array of histograms to display
**	nHistograms	The number of histograms specified in histograms
**	rescaleMode	One of: XY_NO_RESCALE, XY_RESCALE, or XY_GROW_ONLY.
**			Tells the widget how to change its display to
**			incorporate the new contents.
**  	redisplay   	True to refresh display, False to hold screen update 
**  	    	    	until a later call (to prevent unnecessary flashing).
*/
void XYSetHistograms(Widget w, XYHistogram *histograms, int nHistograms,
    	int rescaleMode, int redisplay)
{
    setHistograms(w, histograms, nHistograms, rescaleMode, False, redisplay);
}

/*
** XYUpdateHistogramStyles
**
** Update the style information (line style, fill style, line color, and fill
** color) for the histograms currently displayed by the widget, keeping the
** existing data, and ignoring the data fields in the histogram structures.
** The number of histograms must be the same as the currently displayed data
**
** Parameters
**
** 	w		An xy widget
**	histograms	An array of histogram data structures containing the
**			new style information, with the same number of
**			elements as were passed originally in the nHistograms
**			argument to XYSetHistograms.
**  	redisplay   	True to refresh display, False to hold screen update 
**  	    	    	until a later call (to prevent unnecessary flashing).
*/
void XYUpdateHistogramStyles(Widget w, XYHistogram *histograms, int redisplay)
{
    int i;
    
    for (i=0; i<((XYWidget)w)->xy.nHistograms; i++) {
    	XmStringFree(((XYWidget)w)->xy.histograms[i].name);
    	copyHistogramStyle(&histograms[i], &((XYWidget)w)->xy.histograms[i]);
    }
    if (redisplay && XtIsRealized(w))
    	redisplayContents((XYWidget)w, X_SCREEN, REDRAW_ALL);
    else
    	((XYWidget)w)->xy.toRedraw |= REDRAW_ALL;
}

/*
** XYUpdateHistogramData
**
** Update the histograms displayed by the widget, using the new data in the
** histogram structures, ignoring style, color, and name information.
** The number of histograms must be the same as the currently displayed data
**
** Parameters
**
** 	w		An xy widget
**	histograms	An array of histogram data structures containing the
**			new data to display, with the same number of 
**			elements as were passed originally in the nHistograms
**			argument to XYSetContents.
**	rescaleMode	One of: XY_NO_RESCALE, XY_RESCALE, or
**			XY_GROW_ONLY.  Tells the widget how to change
**			its display to incorporate the new contents.
**  	redisplay   	True to refresh display, False to hold screen update 
**  	    	    	until a later call (to prevent unnecessary flashing).
*/
void XYUpdateHistogramData(Widget w, XYHistogram *histograms, int rescaleMode,
    	int redisplay)
{
    setHistograms(w, histograms, ((XYWidget)w)->xy.nHistograms, rescaleMode,
    	    False, redisplay);
}

static void setHistograms(Widget w, XYHistogram *histograms, int nHistograms,
    	int rescaleMode, int dataOnly, int redisplay)
{
    XYWidget xyW = (XYWidget)w;
    int i, oldNBins;
    int doResize = nHistograms != xyW->xy.nHistograms && xyW->xy.showLegend;

    /* If we are re-binning a single histogram, save the old number of bins
       to be used for guessing a new Y scale in automatic rescaling */
    oldNBins = rescaleMode == XY_REBIN_MODE && xyW->xy.nHistograms == 1 ?
    	    xyW->xy.histograms->nBins : 0;
    	    
    /* Free the existing data (but preserve histogram structures in dataOnly
       case to save appearance information) */
    if (xyW->xy.nHistograms != 0) {
    	if (dataOnly) {
    	    for (i=0; i<xyW->xy.nHistograms; i++)
    		freeHistogramData(&xyW->xy.histograms[i], True);
    	} else {
    	    for (i=0; i<xyW->xy.nHistograms; i++)
    		freeHistogramData(&xyW->xy.histograms[i], False);
    	    XtFree((char *)xyW->xy.histograms);
    	}
    }
    
    /* Copy in the user's data */
    if (nHistograms != 0) {
    	if (!dataOnly) {
    	    xyW->xy.histograms =
    	    	    (XYHistogram *)XtMalloc(sizeof(XYHistogram) * nHistograms);
    	    xyW->xy.nHistograms = nHistograms;
    	}
    	for (i=0; i<nHistograms; i++)
	    copyHistogramData(&histograms[i], &xyW->xy.histograms[i], dataOnly);
    } else {
    	xyW->xy.nHistograms = 0;
    	xyW->xy.histograms = NULL;
    }
    
    /* if the number of histograms changed, the legend may have changed size,
       so recalculate the size-dependent parts of the widget */
    if (doResize) {
    	resizeParts(xyW);
    	xyW->xy.toRedraw |= REDRAW_ALL;
    }
    
    /* Scan the data and calculate the new data and view ranges */
    xyW->xy.toRedraw |= resetDataAndViewRanges(xyW, oldNBins, rescaleMode);
    
    /* redraw the widget with the new data, unless inhibited by redisplay */
    if (redisplay && XtIsRealized(w))
    	redisplayContents(xyW, X_SCREEN, xyW->xy.toRedraw);
}

/*
** Examine the data currently stored in the widget, and reset both the
** maximum viewable data range (minXData, minYData, maxXData, maxYData),
** and the current visible range (minXLim, minYLim, maxXLim, maxYLim),
** according to the specified rescale mode.  The parameter "oldNBins" is
** used only in XY_REBIN_MODE for guessing a new Y scale for a single
** histogram being re-binned.  Returns which areas of the widget have
** changed and must be redrawn to display the changes.
*/
static int resetDataAndViewRanges(XYWidget w, int oldNBins, int rescaleMode)
{    
    double minX, minY, maxX, maxY, posMinX, posMinY;
    int redrawArea = REDRAW_NONE;
    
    /* Examine the data to find the min and max values */
    if (w->xy.nCurves == 0 && w->xy.nHistograms == 0) {
    	if (rescaleMode == XY_RESCALE) {
    	    minX = minY = 0.;
    	    maxX = maxY = 1.;
    	    posMinX = posMinY = .5;
    	} else {
    	    minX = w->xy.minXData; minY = w->xy.minYData;
    	    maxX = w->xy.maxXData; maxY = w->xy.maxYData;
    	    posMinX = dMax(DBL_MIN, minX); posMinY = dMax(DBL_MIN, minY); 
    	}
    } else
	calcDataRange(w, &minX, &maxX, &minY, &maxY, &posMinX, &posMinY);
    
    /* recalculate plot limits from the new data (full rescale) */
    if (rescaleMode == XY_RESCALE) {
	if (maxX == minX) {maxX += 1.; minX -= 1.;}  /* fix zero range data */
	if (maxY == minY) {maxY += 1.; minY -= 1.;}
    	w->xy.maxXDragStop = maxX; w->xy.maxXLim = maxX;
    	w->xy.minXDragStop = minX; w->xy.minXLim = minX;
    	w->xy.maxYDragStop = maxY; w->xy.maxYLim = maxY;
    	w->xy.minYDragStop = minY; w->xy.minYLim = minY;
    	redrawArea |= REDRAW_CONTENTS | REDRAW_H_AXIS | REDRAW_V_AXIS;
    
    /* recalculate data limits but don't change visible range (no rescale) */
    } else if (rescaleMode == XY_NO_RESCALE) {
	redrawArea |= REDRAW_CONTENTS;
	if (w->xy.maxXDragStop <= w->xy.maxXData)
	    w->xy.maxXDragStop = dMax(maxX, w->xy.maxXLim);
	if (w->xy.minXDragStop <= w->xy.minXData)
	    w->xy.minXDragStop = dMin(minX, w->xy.minXLim);
	if (w->xy.maxYDragStop <= w->xy.maxYData)
	    w->xy.maxYDragStop = dMax(maxY, w->xy.maxYLim);
	if (w->xy.minYDragStop <= w->xy.minYData)
	    w->xy.minYDragStop = dMin(minY, w->xy.minYLim);
    
    /* recalculate limits for rescale-at-max (normal data-update mode) */
    } else if (rescaleMode == XY_RESCALE_AT_MAX) {
	redrawArea |= REDRAW_CONTENTS;
    	if (w->xy.maxXData != maxX || w->xy.minXData != minX)
    	    redrawArea |= REDRAW_H_AXIS;
    	if (w->xy.maxYData != maxY || w->xy.minYData != minY)
    	    redrawArea |= REDRAW_V_AXIS;
	if (w->xy.maxXDragStop <= w->xy.maxXData)
	    w->xy.maxXDragStop = maxX;
	if (w->xy.minXDragStop <= w->xy.minXData)
	    w->xy.minXDragStop = minX;
	if (w->xy.maxYDragStop <= w->xy.maxYData)
	    w->xy.maxYDragStop = maxY;
	if (w->xy.minYDragStop <= w->xy.minYData)
	    w->xy.minYDragStop = minY;
     	if (w->xy.maxXData == w->xy.maxXLim)
    	    w->xy.maxXLim = maxX;
    	if (w->xy.minXData == w->xy.minXLim)
    	    w->xy.minXLim = minX;
    	if (w->xy.maxYData == w->xy.maxYLim)
    	    w->xy.maxYLim = maxY;
    	if (w->xy.minYData == w->xy.minYLim)
    	    w->xy.minYLim = minY;
    
    /* recalculate limits for grow-only mode (like rescale-at-max, but no
       shrinking allowed) */
    } else if (rescaleMode == XY_GROW_ONLY) {
	redrawArea |= REDRAW_CONTENTS;
    	if (w->xy.maxXData != maxX || w->xy.minXData != minX)
    	    redrawArea |= REDRAW_H_AXIS;
    	if (w->xy.maxYData != maxY || w->xy.minYData != minY)
    	    redrawArea |= REDRAW_V_AXIS;
	if (w->xy.maxXDragStop <= w->xy.maxXData)
	    w->xy.maxXDragStop = maxX;
	if (w->xy.minXDragStop <= w->xy.minXData)
	    w->xy.minXDragStop = minX;
	if (w->xy.maxYDragStop <= w->xy.maxYData)
	    w->xy.maxYDragStop = maxY;
	if (w->xy.minYDragStop <= w->xy.minYData)
	    w->xy.minYDragStop = minY;
    	if (w->xy.maxXData == w->xy.maxXLim)
    	    w->xy.maxXLim = dMax(w->xy.maxXDragStop, maxX);
    	if (w->xy.minXData == w->xy.minXLim)
    	    w->xy.minXLim = dMin(w->xy.minXDragStop, minX);
    	if (w->xy.maxYData == w->xy.maxYLim)
    	    w->xy.maxYLim = dMax(w->xy.maxYDragStop, maxY);
    	if (w->xy.minYData == w->xy.minYLim)
    	    w->xy.minYLim = dMin(w->xy.minYDragStop, minY);

    /* recalculate limits for histogram-rebin mode (like rescale-at-max, but
       if the range is not at max, and there's a single histogram displayed,
       make a guess as to how the y scale will be changed by the re-binning */
    } else if (rescaleMode == XY_REBIN_MODE) {
	redrawArea |= REDRAW_ALL;
	if (w->xy.maxXDragStop <= w->xy.maxXData)
	    w->xy.maxXDragStop = maxX;
	if (w->xy.minXDragStop <= w->xy.minXData)
	    w->xy.minXDragStop = minX;
	if (w->xy.maxYDragStop <= w->xy.maxYData)
	    w->xy.maxYDragStop = maxY;
	if (w->xy.minYDragStop <= w->xy.minYData)
	    w->xy.minYDragStop = minY;
    	if (w->xy.maxXData == w->xy.maxXLim)
    	    w->xy.maxXLim = maxX;
    	if (w->xy.minXData == w->xy.minXLim)
    	    w->xy.minXLim = minX;
    	if (w->xy.maxYData == w->xy.maxYLim)
    	    w->xy.maxYLim = maxY;
    	else if (w->xy.nHistograms == 1 && w->xy.nCurves == 0 && oldNBins != 0
    	    	&& w->xy.histograms->edges == NULL)
    	    w->xy.maxYLim =  dMin(maxY, w->xy.maxYLim *
    	    	    (double)oldNBins/(double)w->xy.histograms->nBins);
    	if (w->xy.minYData == w->xy.minYLim)
    	    w->xy.minYLim = minY;
    }

    /* Reset the data range to values calculated by scanning the data */
    w->xy.maxXData = maxX; w->xy.minXData = minX;
    w->xy.maxYData = maxY; w->xy.minYData = minY;
    
    /* If log scaling is turned on, force visible limits to show only
       the positive portion of the data. */
    if (w->xy.xLogScaling && w->xy.minXLim <= 0.) {
	if (w->xy.maxXLim > 0.)
	    w->xy.minXLim = posMinX;
	else {
	    w->xy.minXLim = .1;
	    w->xy.maxXLim = 1.;
	}
    }
    if (w->xy.yLogScaling && w->xy.minYLim <= 0.) {
	if (w->xy.maxYLim > 0.)
	    w->xy.minYLim = posMinY;
	else {
	    w->xy.minYLim = .1;
	    w->xy.maxYLim = 1.;
	}
    }
    return redrawArea;
}

/*
** XYSetStrings
**
** Display a set of text strings in the plot area of the widget.  Calling this
** routine removes any strings which were displayed previously and replaces
** them with the ones passed here.  The strings are in the form of XYString
** data structures, which contain position, font, color, and alignment, for
** the string, in addition to the text string itself.  In the XYString
** structure, font can be passed as NULL, and will default to the widget's
** fontList resource.
**
** Parameters
**
** 	w		An xy widget
**	strings		Array of XYStrings data structures containing string,
**			position, font, color, and alignment information
**	nStrings	The number of XYString data structures contained in
**			strings above
**	
*/
void XYSetStrings(Widget w, XYString *strings, int nStrings)
{
    XYWidget xyW = (XYWidget)w;
    int i;
    
    /* free the old string data */
    for (i=0; i<xyW->xy.nStrings; i++) {
    	XmFontListFree(xyW->xy.strings[i].font);
    	XmStringFree(xyW->xy.strings[i].string);
    }
    if (nStrings != 0)
    	XtFree((char *)xyW->xy.strings);
    
    /* copy in the new data */
    xyW->xy.nStrings = nStrings;
    if (nStrings == 0)
    	xyW->xy.strings = NULL;
    else
    	xyW->xy.strings = (XYString *)XtMalloc(sizeof(XYString) * nStrings);
    for (i=0; i<nStrings; i++) {
    	xyW->xy.strings[i] = strings[i];
    	xyW->xy.strings[i].font = XmFontListCopy(strings[i].font);
    	xyW->xy.strings[i].string = XmStringCopy(strings[i].string);
    }
    
    /* update the display */
    if (XtIsRealized(w))
    	redisplayContents(xyW, X_SCREEN, REDRAW_CONTENTS);
}

/*
** XYSetVisibleRange, XYGetVisibleRange
**
** Set (Get) the range of data that is visible.  minXLim, minYLim, maxXLim, and
** maxYLim specify the endpoints of the x and y axes.  XYSetVisibleRange,
** unlike the widgets interactive rescaling routines, can zoom out past the
** actual minimum and maximum data points.
*/
void XYSetVisibleRange(Widget w, double minXLim, double minYLim,
			 double maxXLim, double maxYLim)
{
    XYWidget xyW = (XYWidget)w;
    double minX, minY, maxX, maxY, posMinX, posMinY, dumMax;
    
    /* calculate the actual range of the data */
    calcDataRange(xyW, &minX, &maxX, &minY, &maxY, &posMinX, &posMinY);

    /* If log scaling was requested, make sure new range is log scaleable.    */
    /* Set lower visible limit to the minimum data limit, and adjust slightly */
    /* to allow  markers to be entirely displayed and histogram bottoms to be */
    /* clearly seen.  							      */
    if (xyW->xy.xLogScaling && minXLim <= 0.) {
	minXLim = posMinX;
	dumMax = maxXLim;
	expandLogRange(&minXLim, &dumMax, xyW->xy.marginPercent / 100.);
    }
    if (xyW->xy.yLogScaling && minYLim <= 0.) {
	minYLim = posMinY;
	dumMax = maxYLim;
	expandLogRange(&dumMax, &minYLim, xyW->xy.marginPercent / 100.);
    }

    /* allow user to zoom beyond the range of the data */
    xyW->xy.maxXDragStop = dMax(maxXLim, maxX);
    xyW->xy.minXDragStop = dMin(minXLim, minX);
    xyW->xy.maxYDragStop = dMax(maxYLim, maxY);
    xyW->xy.minYDragStop = dMin(minYLim, minY);

    /* Set the range */
    xyW->xy.minXLim = minXLim;
    xyW->xy.maxXLim = maxXLim;
    xyW->xy.minYLim = minYLim;
    xyW->xy.maxYLim = maxYLim;

    /* redraw if the widget is realized */
    if (XtIsRealized(w))
    	redisplayContents(xyW, X_SCREEN,
    		REDRAW_V_AXIS | REDRAW_H_AXIS | REDRAW_CONTENTS);
}
void XYGetVisibleRange(Widget w, double *minXLim, double *minYLim,
    	double *maxXLim, double *maxYLim)
{
    *minXLim = ((XYWidget)w)->xy.minXLim;
    *maxXLim = ((XYWidget)w)->xy.maxXLim;
    *minYLim = ((XYWidget)w)->xy.minYLim;
    *maxYLim = ((XYWidget)w)->xy.maxYLim;
}

/*
** XYZoomOut, XYZoomIn, XYResetZoom
**
** Zoom in and out by ZOOM_FACTOR.  Zoom in is centered on the current
** center of the plot.
*/
void XYZoomOut(Widget w)
{
    XYWidget xyW = (XYWidget)w;
    int xLogScaling = xyW->xy.xLogScaling, yLogScaling = xyW->xy.yLogScaling;
    double xOffset, yOffset, newMaxXLim, newMinXLim, newMaxYLim, newMinYLim;
    double minXLim, maxXLim, minYLim, maxYLim;
    int redrawArea = REDRAW_NONE;
    
    /* if log scaling was requested, express limits in log coordinates */
    minXLim = xLogScaling ? log10(xyW->xy.minXLim) : xyW->xy.minXLim;
    maxXLim = xLogScaling ? log10(xyW->xy.maxXLim) : xyW->xy.maxXLim;
    minYLim = yLogScaling ? log10(xyW->xy.minYLim) : xyW->xy.minYLim;
    maxYLim = yLogScaling ? log10(xyW->xy.maxYLim) : xyW->xy.maxYLim;

    /* Calculate a suitable offset to reverse a zoom in by ZOOM_FACTOR */
    xOffset = (maxXLim - minXLim) * (ZOOM_FACTOR/(1.-ZOOM_FACTOR)) / 2;
    yOffset = (maxYLim - minYLim) * (ZOOM_FACTOR/(1.-ZOOM_FACTOR)) / 2;

    /* widen the plotting limits by the offsets calculated above,
       stopping when the limits reach the drag stop limits */
    newMaxXLim = dMin(xyW->xy.maxXDragStop,
    	    xLogScaling ? pow(10., maxXLim + xOffset) : maxXLim + xOffset);
    newMinXLim = dMax(xyW->xy.minXDragStop, xLogScaling ?
    	    dMax(DBL_MIN, pow(10., minXLim - xOffset)) : minXLim - xOffset);
    newMaxYLim = dMin(xyW->xy.maxYDragStop,
    	    yLogScaling ? pow(10., maxYLim + yOffset) : maxYLim + yOffset);
    newMinYLim = dMax(xyW->xy.minYDragStop, yLogScaling ?
    	    dMax(DBL_MIN, pow(10., minYLim - yOffset)) : minYLim - yOffset);
    
    /* Tell widget to redraw, and what parts, if limits have changed */
    if (newMaxXLim != maxXLim || newMinXLim != minXLim)
    	redrawArea |= REDRAW_H_AXIS | REDRAW_CONTENTS;
    if (newMaxYLim != maxYLim || newMinYLim != minYLim)
    	redrawArea |= REDRAW_V_AXIS | REDRAW_CONTENTS;
    
    /* Set the new limits */
    xyW->xy.maxXLim = newMaxXLim;
    xyW->xy.minXLim = newMinXLim;
    xyW->xy.maxYLim = newMaxYLim;
    xyW->xy.minYLim = newMinYLim;

    /* redraw if the widget is realized */
    if (XtIsRealized(w))
    	redisplayContents(xyW, X_SCREEN, redrawArea);
}
void XYZoomIn(Widget w)
{
    XYWidget xyW = (XYWidget)w;
    int xLogScaling = xyW->xy.xLogScaling, yLogScaling = xyW->xy.yLogScaling;
    double xOffset, yOffset;
    double minXLim, maxXLim, minYLim, maxYLim;

    /* if log scaling was requested, express limits in log coordinates */
    minXLim = xLogScaling ? log10(xyW->xy.minXLim) : xyW->xy.minXLim;
    maxXLim = xLogScaling ? log10(xyW->xy.maxXLim) : xyW->xy.maxXLim;
    minYLim = yLogScaling ? log10(xyW->xy.minYLim) : xyW->xy.minYLim;
    maxYLim = yLogScaling ? log10(xyW->xy.maxYLim) : xyW->xy.maxYLim;
    
    /* Calculate offsets for limits of displayed data to zoom by ZOOM_FACTOR */
    xOffset = (maxXLim - minXLim) * ZOOM_FACTOR / 2;
    yOffset = (maxYLim - minYLim) * ZOOM_FACTOR / 2;

    /* Narrow the plotting limits by the offsets calculated above */
    maxXLim -= xOffset;
    minXLim += xOffset;
    maxYLim -= yOffset;
    minYLim += yOffset;
    
    /* Set the new limits */
    xyW->xy.maxXLim = xLogScaling ? pow(10.,maxXLim) : maxXLim;
    xyW->xy.minXLim = xLogScaling ? pow(10.,minXLim) : minXLim;
    xyW->xy.maxYLim = yLogScaling ? pow(10.,maxYLim) : maxYLim;
    xyW->xy.minYLim = yLogScaling ? pow(10.,minYLim) : minYLim;
   
    /* redraw if the widget is realized */
    if (XtIsRealized(w))
    	redisplayContents(xyW, X_SCREEN,
    		REDRAW_V_AXIS | REDRAW_H_AXIS | REDRAW_CONTENTS);
}
void XYResetZoom(Widget w)
{
    XYWidget xyW = (XYWidget)w;
    double minX, minY, maxX, maxY, posMinX, posMinY;

    /* Set the limits to the actual range of the data */
    calcDataRange(xyW, &minX, &maxX, &minY, &maxY, &posMinX, &posMinY);
    xyW->xy.minXLim = xyW->xy.minXDragStop = minX;
    xyW->xy.minYLim = xyW->xy.minYDragStop = minY;
    xyW->xy.maxXLim = xyW->xy.maxXDragStop = maxX;
    xyW->xy.maxYLim = xyW->xy.maxYDragStop = maxY;
    
    /* Adjust the limits if there are zeros or negative values and the
       scale is log */
    if (xyW->xy.xLogScaling && xyW->xy.minXLim <= 0.) {
	if (xyW->xy.maxXLim > 0.) {
	    xyW->xy.minXLim = posMinX;
	    xyW->xy.minXDragStop = DBL_MIN;
	} else {
	    xyW->xy.minXLim = .1;
	    xyW->xy.maxXLim = xyW->xy.maxXDragStop = 1.;
	    xyW->xy.minXDragStop = DBL_MIN;
	}
    }
    if (xyW->xy.yLogScaling && xyW->xy.minYLim <= 0.) {
	if (xyW->xy.maxYLim > 0.) {
	    xyW->xy.minYLim = posMinY;
	    xyW->xy.minYDragStop = DBL_MIN;
	} else {
	    xyW->xy.minYLim = .1;
	    xyW->xy.maxYLim = xyW->xy.maxYDragStop = 1.;
	    xyW->xy.minYDragStop = DBL_MIN;
	}
    }

    if (XtIsRealized(w))
	redisplayContents(xyW, X_SCREEN,
  		REDRAW_V_AXIS | REDRAW_H_AXIS | REDRAW_CONTENTS);
}

/*
** XYPrintContents
**
** Prints the contents XY widget to a PostScript file.
**
** Parameters
**
**	w		A xy widget
**	psFileName	Name for the PostScript file that will be created
**	
*/
void XYPrintContents(Widget w, char *psFileName)
{
    FILE * ps;
    XYWidget xyW = (XYWidget)w;

    ps = OpenPS(psFileName, xyW->core.width, xyW->core.height);
    if (ps != NULL) {
	redisplayContents(xyW, PS_PRINTER, REDRAW_ALL);
	EndPS();
    }    
}

/*
** XYWritePS
**
** Writes out PostScript drawing commands to draw the contents of the widget
** to an open file.  The PostScript code written does not stand alone.  It
** depends on the preamble definitions and scaling normally provided by
** OpenPS in the utilities directory.  To get the bounding rectangle, use
** XGetGeometry on the XtWindow of the widget to get the width and height,
** the plot is drawn between 0,0 and the width and height of the widget.
**
** Parameters
**
**	w	An XY widget
**	fp	File pointer for an open file in which to
**		write the drawing commands
*/
void XYWritePS(Widget w, FILE *fp)
{
    FILE *oldFP;
    
    oldFP = PSGetFile();
    PSSetFile(fp);
    redisplayContents((XYWidget)w, PS_PRINTER, REDRAW_ALL);
    PSSetFile(oldFP);   
}

/*
** XYDrawMarker
**
** Draw a marker in an external window (for use with external dialogs for
** setting marker styles).
**
** Parameters
**
**	display		X display
**	drawBuf		X window to draw in
**	gc		X graphics context to use for drawing.  Color, line
**			thickness and line style will be altered.
**	size		One of: XY_TINY, XY_SMALL, XY_MEDIUM, or XY_LARGE
**	style		One of the valid marker styles as listed in the
**			description of the curves data structure above.
**	color		Pixel value for the marker
**	x, y		Position in window coordinates of the center of the
**			marker
*/
void XYDrawMarker(Display *display, Drawable drawBuf, GC gc, int size,
	int style, Pixel color, int x, int y)
{
    drawMarker(display, drawBuf, gc, X_SCREEN, size, style, color, x, y);
}

/*
** XYDrawHistMarker
**
** Draw a marker in an external window (for use with external dialogs for
** setting marker styles), using a histogram "line" style, rather than the
** standard marker size and marker styles used by XYDrawMarker.  The
** lineStyle field in an XYHistogram can contain additional values (beyond
** the line styles used in an XYCurve) which cause the widget to draw markers
** instead of histogram outlines.
**
** Parameters
**
**	display		X display
**	drawBuf		X window to draw in
**	gc		X graphics context to use for drawing.  Color, line
**			thickness and line style will be altered.
**	style		One of the valid histogram line styles, beyond
**  	    	    	XY_N_LINE_STYLES
**	color		Pixel value for the marker
**	x, y		Position in window coordinates of the center of the
**			marker
*/
void XYDrawHistMarker(Display *display, Drawable drawBuf, GC gc,
	int style, Pixel color, int x, int y)
{
    if (style < XY_N_LINE_STYLES || style >= XY_N_HIST_LINE_STYLES)
    	return;
    drawMarker(display, drawBuf, gc, X_SCREEN,
    	    HistMarkerStyles[style - XY_N_LINE_STYLES].size,
    	    HistMarkerStyles[style - XY_N_LINE_STYLES].style, color, x, y);
}

/*
** XYDrawLine
**
** Draw a line in an external window (for use with external dialogs for
** setting line styles).
**
** Parameters
**
**	display		X display
**	drawBuf		X window to draw in
**	gc		X graphics context to use for drawing.  Color, line
**			thickness and line style will be altered.
**	style		One of the valid marker styles as listed in the
**			description of the curves data structure above.
**	color		Pixel value for the marker
**	x1, y1		Position in window coordinates of the start of the line
**	x2, y2		Position in window coordinates of the end of the line
*/
void XYDrawLine(Display *display, Drawable drawBuf, GC gc, int style,
	Pixel color, int x1, int y1, int x2, int y2)
{
    drawLine(display, drawBuf, gc, X_SCREEN, style, color, x1, y1, x2, y2);
}

/*
** XYDrawFill
**
** Draw a histogram fill pattern in an external window (for use with external
** dialogs for setting line styles).
**
** Parameters
**
**	display		X display
**	drawBuf		X window to draw in
**	gc		X graphics context to use for drawing.  Color, line
**			thickness and line style will be altered.
**	style		One of the valid fill styles as listed in the
**			description of the histogram data structure above.
**	color		Pixel value for the fill pattern
**	xMin, yMin	top left corner ot the rectangle to fill
**	xMax, yMax	bottom right corner ot the rectangle to fill
*/
void XYDrawFill(Display *display, Drawable drawBuf, GC gc,
	unsigned style, Pixel color, int xMin, int yMin, int xMax, int yMax)
{
    drawFill(display, drawBuf, gc, X_SCREEN, xMin, yMin, xMax, yMax, style,
    	    color);
}

/*
** Obsolete Interface Routines (for compatability with V1.0 release):
**
**    XYSetContents, XYUpdateStyles, XYUpdateData
*/
void XYSetContents(Widget w, XYCurve *curves, int nCurves, int rescaleMode)
{
    setCurves(w, curves, nCurves, rescaleMode, False, True);
}
void XYUpdateStyles(Widget w, XYCurve *curves)
{
    int i;
    
    for (i=0; i<((XYWidget)w)->xy.nCurves; i++) {
    	XmStringFree(((XYWidget)w)->xy.curves[i].name);
    	copyCurveStyle(&curves[i], &((XYWidget)w)->xy.curves[i]);
    }
    if (XtIsRealized(w))
    	redisplayContents((XYWidget)w, X_SCREEN, REDRAW_ALL);
}
void XYUpdateData(Widget w, XYCurve *curves, int rescaleMode)
{
    setCurves(w, curves, ((XYWidget)w)->xy.nCurves, rescaleMode, False, True);
}

/*
** Redisplays the contents part of the widget, (everything but the motif
** shadows and highlights).
*/
static void redisplayContents(XYWidget w, int outDevice, int redrawArea)
{
    Display *display = XtDisplay(w);
    GC gc = w->xy.gc;
    XGCValues gcValues;
    XFontStruct *fs = getFontStruct(w->xy.font);
    Drawable drawBuf;
    int i, stringWidth, stringX, alignment;
    XmFontList stringFont;
    Pixel lastColor;
    XYString *string;
    XYTransform xform;
 
    /* Save some energy if the widget isn't visible or no drawing requested */
    if ((outDevice==X_SCREEN && !w->core.visible) || redrawArea == REDRAW_NONE)
        return;

    /* Set destination for drawing commands, offscreen pixmap or window */
    if (w->xy.doubleBuffer)
    	drawBuf = w->xy.drawBuffer;
    else
    	drawBuf = XtWindow(w);

    /* Clear the drawing buffer or window only in the areas that have
       changed.  The other parts are still redrawn, but the net effect
       is that the unchanged areas do not flicker */
    if (outDevice == X_SCREEN) {
	XSetForeground(display, gc, w->core.background_pixel);
	if (redrawArea == REDRAW_ALL) {
	    XFillRectangle(display, drawBuf, gc, w->xy.xMin, w->xy.yMin,
    		     w->xy.xMax - w->xy.xMin, w->xy.yMax - w->xy.yMin);
	} else {
    	    if (redrawArea & REDRAW_V_AXIS)
		XFillRectangle(display, drawBuf, gc, w->xy.axisLeft,
	   		w->xy.axisTop, w->xy.xOrigin - w->xy.axisLeft,
    			w->xy.axisBottom - w->xy.axisTop);
    	    if (redrawArea & REDRAW_H_AXIS)
    		XFillRectangle(display, drawBuf, gc, w->xy.axisLeft,
    	    		w->xy.yOrigin + 1, w->xy.axisRight-w->xy.axisLeft,
    	    		w->xy.axisBottom - w->xy.yOrigin + 1);
    	    if (redrawArea & REDRAW_CONTENTS)
    		XFillRectangle(display, drawBuf, gc, w->xy.xOrigin + 1,
    	   		w->xy.yEnd, w->xy.xEnd - w->xy.xOrigin,
    	   		w->xy.yOrigin - w->xy.yEnd);
	}
    }
    
    /* Draw the axes */
    gcValues.line_width = 0;
    gcValues.line_style = LineSolid;
    gcValues.foreground = w->primitive.foreground;
    XChangeGC(display, gc, GCLineWidth|GCLineStyle|GCForeground, &gcValues);
    if (!widgetHasData(w)) {
        /* empty of data, just draw axis lines */
    	XSegment segs[2];
    	segs[0].x1 = segs[0].x2 = segs[1].x1 = w->xy.xOrigin;
    	segs[0].y1 = segs[1].y1 = segs[1].y2 = w->xy.yOrigin;
    	segs[1].x2 = w->xy.xEnd; segs[0].y2 = w->xy.yEnd;
	if (outDevice == X_SCREEN)
    	    XDrawSegments(display, drawBuf, gc, segs, 2);
	else /* PS_PRINTER */
    	    PSDrawSegments(display, drawBuf, gc, segs, 2);
    } else {
	DrawHorizontalAxis(display, drawBuf, gc, fs, outDevice,
    		w->xy.yOrigin, w->xy.xOrigin, w->xy.xEnd,
    		dMin(w->xy.minXDragStop, w->xy.minXData),
    		dMax(w->xy.maxXDragStop, w->xy.maxXData),
    		w->xy.minXLim, w->xy.maxXLim, w->xy.xLogScaling, 0);
	DrawVerticalAxis(display, drawBuf, gc, fs, outDevice, w->xy.xOrigin,
		w->xy.xMin, w->xy.yEnd, w->xy.yOrigin,
		dMin(w->xy.minYDragStop, w->xy.minYData),
		dMax(w->xy.maxYDragStop, w->xy.maxYData),
		w->xy.minYLim, w->xy.maxYLim, w->xy.yLogScaling);
    }
    
    /* Draw the axis labels */
    if (w->xy.xAxisLabel != NULL)
    	if (outDevice == X_SCREEN)
    	    XmStringDraw(display, drawBuf, w->xy.font, w->xy.xAxisLabel,
		    gc, w->xy.xOrigin, w->xy.axisBottom + X_LABEL_MARGIN,
		    w->xy.xEnd - w->xy.xOrigin, XmALIGNMENT_CENTER,
		    XmSTRING_DIRECTION_L_TO_R, NULL);
    	else
    	    PSDrawXmString(display, drawBuf, w->xy.font, w->xy.xAxisLabel,
		    gc, w->xy.xOrigin, w->xy.axisBottom + X_LABEL_MARGIN,
		    w->xy.xEnd - w->xy.xOrigin, XmALIGNMENT_CENTER);
    if (w->xy.yAxisLabel != NULL)
    	if (outDevice == X_SCREEN)
    	    XmStringDraw(display, drawBuf, w->xy.font, w->xy.yAxisLabel, gc,
    		    w->xy.xOrigin + Y_LABEL_MARGIN, w->xy.yMin + TOP_MARGIN,
		    w->xy.xEnd - w->xy.xOrigin, XmALIGNMENT_BEGINNING,
	     	    XmSTRING_DIRECTION_L_TO_R, NULL);
    	else
    	    PSDrawXmString(display, drawBuf, w->xy.font, w->xy.yAxisLabel,
    		    gc, w->xy.xOrigin + Y_LABEL_MARGIN, w->xy.yMin + TOP_MARGIN,
		    w->xy.xEnd - w->xy.xOrigin, XmALIGNMENT_BEGINNING);

    /* Draw the legend */
    if (w->xy.showLegend && redrawArea == REDRAW_ALL)
    	drawLegend(w, drawBuf, outDevice);
    
    /* Clipping information can't be read from the GCs by the PS translation
       code, so it has to be set by hand before drawing */
    if (outDevice == PS_PRINTER)
    	PSSetClipRectangle(w->xy.xOrigin, w->xy.yEnd, w->xy.xEnd,w->xy.yOrigin);
    
    /* Draw the contents of the plot */
    for (i=0; i<w->xy.nHistograms; i++)
    	drawHistogram(w, drawBuf, outDevice, &w->xy.histograms[i]);
    for (i=0; i<w->xy.nCurves; i++)
    	drawCurve(w, drawBuf, outDevice, &w->xy.curves[i]);

    /* Draw the plotted strings */
    lastColor = w->primitive.foreground;
    XSetForeground(display, w->xy.contentsGC, lastColor);
    ComputeTransform(w, &xform);
    for (i=0, string=w->xy.strings; i<w->xy.nStrings; i++, string++) {
	if (string->color != lastColor) {
	    XSetForeground(display, w->xy.contentsGC, string->color);
	    lastColor = string->color;
	}
	stringFont = string->font == NULL ? w->xy.font : string->font;
	stringWidth = XmStringWidth(stringFont, string->string);
	if (string->alignment == XY_LEFT) {
	    stringX = XYDataToWindowX(&xform, string->x);
	    alignment = XmALIGNMENT_BEGINNING;
    	} else if (string->alignment == XY_CENTER) {
    	    stringX = XYDataToWindowX(&xform, string->x) - stringWidth/2;
    	    alignment = XmALIGNMENT_CENTER;
    	} else { /* XY_RIGHT */
    	    stringX = XYDataToWindowX(&xform, string->x) - stringWidth;
    	    alignment = XmALIGNMENT_END;
    	}
    	if (outDevice == X_SCREEN)
    	    XmStringDraw(display, drawBuf, stringFont, string->string,
    		    w->xy.contentsGC, stringX,
    		    (int)XYDataToWindowY(&xform, string->y), stringWidth,
    		    alignment, XmSTRING_DIRECTION_L_TO_R, NULL);
    	else
    	    PSDrawXmString(display, drawBuf, stringFont, string->string,
    		    w->xy.contentsGC, stringX,
    		    (int)XYDataToWindowY(&xform, string->y),
    		    stringWidth, alignment);
    }
    
    /* For double buffering, now copy offscreen pixmap to screen */
    if (w->xy.doubleBuffer && outDevice == X_SCREEN)
    	XCopyArea(display, drawBuf, XtWindow(w), gc, 0, 0,
    		w->core.width, w->core.height, 0, 0);
    
    /* Call the redisplay callback so an application which draws on the xy
       widget can refresh it's graphics */
    if (XtIsRealized((Widget)w) && outDevice == X_SCREEN) {
	XYCallbackStruct cbStruct;
	XYTransform xform;
	ComputeTransform(w, &xform);
	cbStruct.reason = XmCR_EXPOSE;
	cbStruct.event = NULL;
	cbStruct.xform = &xform;
	XtCallCallbacks((Widget)w, XmNredisplayCallback, (XtPointer)&cbStruct);
    }
    
    /* Mark the stale areas that were redrawn as up-to-date */
    w->xy.toRedraw &= ~redrawArea;
}

/*
** Draw one curve.  Note that drawing is done using the cumbersome method
** of accumulating all of the segments and marker figures for a curve
** in a counted array of X data structures and using the multiple figure
** call, such as XDrawSegments or XDrawRectangles, rather than simply making
** the call to draw the figure on the spot.  This is done because in some
** circumstances there is a 10 to 1 speed difference between these two
** methods.
**
** For the most part the coordinate system used for drawing in this widget is
** the X integer coordinate system of the widget's window.  For PostScript
** output, the coordinates are the same, but we sometimes try to acheive
** higher resolution by using floating point numbers to place points within
** the grid of the 72 dpi screen coordinates.  Here, only lines without
** markers are drawn this way.  Improving the print quality of all of the
** widgets is an ongoing process.
**
** The X coordinate system, being based on 16 bit integers also causes
** trouble with clipping.  If the user zooms out, it is very easy to
** exceed the limits of a short integer.  This widget really needs to
** do its own clipping of lines, but at the moment, it only eliminates those
** lines which are obviously outside of the boundaries of the window.  Lines
** may still have endpoints far outside of the window, and at high zoom
** factors, things happen.
*/
static void drawCurve(XYWidget w, Drawable drawBuf, int outDevice, XYCurve
	*curve)
{
    int hasLines = curve->lineStyle != XY_NO_LINE;
    int hasMarkers = curve->markerStyle != XY_NO_MARK;
    int hasHErrs = curve->horizBars != NULL, hasVErrs = curve->vertBars != NULL;
    Display *display = XtDisplay(w);
    GC gc = w->xy.contentsGC;
    XGCValues gcValues;
    int xMin = w->xy.xOrigin, yMin = w->xy.yEnd;
    int xMax = w->xy.xEnd, yMax = w->xy.yOrigin;
    int markWidth = hasMarkers ? MarkWidths[curve->markerSize] : 0;
    int errWidth = (hasMarkers?markWidth:MED_MARK_SIZE) * ERROR_MARKER_RATIO;
    int xMarkMin = xMin - markWidth/2, yMarkMin = yMin - markWidth/2;
    int xMarkMax = xMax + markWidth/2, yMarkMax = yMax + markWidth/2;
    int xErrMin = xMin - errWidth/2, yErrMin = yMin - errWidth/2;
    int xErrMax = xMax + errWidth/2, yErrMax = yMax + errWidth/2;
    XYErrorBar *hErr, *vErr;
    markStruct markList;
    lineStruct lineList;
    XYPoint *point;
    XYTransform xform;
    XSegment *seg, *segs;
    int nSegs;
    double x, y, lastX, lastY, barMin, barMax;
    int i, lastPointDrawn;
    
    /* Compute constants for data to window coordinate transformation */
    ComputeTransform(w, &xform);
    
    /* Allocate memory for points, segments and markers */
    if (hasLines)
    	startLineList(&lineList, outDevice, curve->nPoints);
    if (hasHErrs || hasVErrs)
    	segs = (XSegment *)XtMalloc(sizeof(XSegment) * curve->nPoints *
    		((hasHErrs ? 4 : 0) + (hasVErrs ? 4 : 0)));
    if (hasMarkers)
    	startMarkList(&markList, curve->markerStyle, curve->nPoints);
    	
    /* Loop through all of the data converting the data coordinates to
       X coordinates and accumulating line segments and markers to draw. */
    seg = segs;
    nSegs = 0;
    lastPointDrawn = False;
    for (i=0, point=curve->points, hErr=curve->horizBars, vErr=curve->vertBars;
    	    i<curve->nPoints; i++, point++, hErr++, vErr++) {
    	x = XYDataToWindowX(&xform, point->x);
    	y = XYDataToWindowY(&xform, point->y);
        if (hasLines) {
            if (i!=0 && !((lastX<xMin && x<xMin) || (lastX>xMax && x>xMax) ||
    	  	    (lastY<yMin && y<yMin) || (lastY>yMax && y>yMax))) {
        	if (!lastPointDrawn)
         	    addToLineList(&lineList, lastX, lastY, True, hasMarkers);
        	if (x<xMin || x>xMax || y<yMin || y>yMax) {
        	    /* point was clipped */
        	    addToLineList(&lineList, x, y, False, hasMarkers);
        	    lastPointDrawn = False;
        	} else {
        	    addToLineList(&lineList, x, y, False, hasMarkers);
        	    lastPointDrawn = True;
        	}
            }
            lastX = x;
            lastY = y;
        }
        if (hasHErrs && y>yErrMin && y<yErrMax) {
    	    barMin = XYDataToWindowX(&xform, hErr->min);
    	    barMax = XYDataToWindowX(&xform, hErr->max);
    	    if (!((barMin<xMin&&barMax<xMin) || (barMin>xMax&&barMax>xMax))) {
    		seg->x1 = barMin; seg->x2 = x-markWidth/2;
    		seg->y1 = seg->y2 = y;
    		seg++; nSegs++;
    		seg->x1 = barMax; seg->x2 = x+markWidth/2;
    		seg->y1 = seg->y2 = y;
    		seg++; nSegs++;
    		seg->x1 = seg->x2 = barMin;
    		seg->y1 = y-errWidth/2; seg->y2 = y+errWidth/2;
    		seg++; nSegs++;
    		seg->x1 = seg->x2 = barMax;
    		seg->y1 = y-errWidth/2; seg->y2 = y+errWidth/2;
    		seg++; nSegs++;
    	    }
        }
        if (hasVErrs && x>xErrMin && x<xErrMax) {
    	    barMin = XYDataToWindowY(&xform, vErr->min);
    	    barMax = XYDataToWindowY(&xform, vErr->max);
    	    if (!((barMin<yMin&&barMax<yMin) || (barMin>yMax&&barMax>yMax))) {
    		seg->x1 = seg->x2 = x; seg->y1 = barMin;
    		seg->y2 = y+markWidth/2;
    		seg++; nSegs++;
     		seg->x1 = seg->x2 = x; seg->y1 = barMax;
     		seg->y2 = y-markWidth/2;
    		seg++; nSegs++;
   		seg->x1 = x-errWidth/2; seg->x2 = x+errWidth/2;
    		seg->y1 = seg->y2 = barMin;
    		seg++; nSegs++;
    		seg->x1 = x-errWidth/2; seg->x2 = x+errWidth/2;
    		seg->y1 = seg->y2 = barMax;
    		seg++; nSegs++;
    	    }
        }
        if (hasMarkers && x>xMarkMin && x<xMarkMax && y>yMarkMin && y<yMarkMax)
            addToMarkList(&markList, (int)x, (int)y, curve->markerSize);
    }
    
    /* Draw the line segments for the connecting lines */
    if (hasLines)
	drawLineList(display, drawBuf, gc, outDevice, &lineList,
	    	curve->lineStyle, curve->linePixel);
    
    /* Draw error bars */
    if (hasHErrs || hasVErrs) {
	gcValues.line_width = 0;
	gcValues.line_style = LineSolid;
	gcValues.foreground = curve->linePixel;
	XChangeGC(display, gc, GCLineWidth|GCLineStyle|GCForeground, &gcValues);
	if (outDevice == X_SCREEN)
    	    XDrawSegments(display, drawBuf, gc, segs, nSegs);
	else
    	    PSDrawSegments(display, drawBuf, gc, segs, nSegs);
        XtFree((char *)segs);
    }
    
    /* Draw markers */
    if (hasMarkers)
    	drawMarkList(display, drawBuf, gc, outDevice, &markList,
    	    	curve->markerSize, curve->markerStyle, curve->markerPixel);
}

/*
** Draw a single histogram.  
*/
static void drawHistogram(XYWidget w, Drawable drawBuf, int outDevice,
    	XYHistogram *histo)
{
    /* draw all of the parts.  Fill is drawn first so solid fill doesn't
       obscure outline or details */
    fillHistogram(w, drawBuf, outDevice, histo);
    drawHistOutline(w, drawBuf, outDevice, histo);
    drawHistMarkers(w, drawBuf, outDevice, histo);
    drawHistErrBars(w, drawBuf, outDevice, histo);
}

static void drawHistOutline(XYWidget w, Drawable drawBuf, int outDevice,
    	XYHistogram *histo)
{
    Display *display = XtDisplay(w);
    GC gc = w->xy.contentsGC;
    int xMin = w->xy.xOrigin, yMin = w->xy.yEnd;
    int xMax = w->xy.xEnd, yMax = w->xy.yOrigin;
    double minXLim = w->xy.minXLim, maxXLim = w->xy.maxXLim;
    double minYLim = w->xy.minYLim;
    double maxYLim = w->xy.maxYLim;
    XSegment *xSeg, *xSegs; int nSegs = 0; 
    double binWidth, x1, x2, y1, y2;
    float *bin;
    int hx1, hy1, hx2, hy2, sep, lastY, i, yZero;
    XYTransform xform;
   
    /* don't bother if there's no data or if outline is not requested */
    if (histo->bins == NULL || histo->lineStyle == XY_NO_LINE ||
    	    histo->lineStyle >= XY_N_LINE_STYLES)
    	return;
    
    /* set up the information for transforming coordinates */
    ComputeTransform(w, &xform);
    yZero = (int)XYDataToWindowY(&xform, 0.);
    binWidth = (histo->xMax - histo->xMin) / (double) (histo->nBins);
    
    /* Allocate memory for an array of XSegment structures for drawing lines. 
       Enough must be allocated for 3 segs and triple lines for overflow, as
       well as an additional baseline. */
    xSegs = (XSegment *)XtMalloc(sizeof(XSegment)*((histo->nBins)*5+2));

    /* loop through all of the data converting the data coordinates to
       X coordinates and drawing left and top segments of each bin */
    lastY = yZero;
    xSeg = xSegs;
    for (i=0, bin=histo->bins; i<histo->nBins; i++, bin++) {

	/* calculate left and right edges of bin in data coordinates, or
	   use the edges array, if supplied, to draw an adaptive histogram */
	if (histo->edges) {
	    x1 = histo->edges[i];
	    x2 = histo->edges[i+1];
	} else {
	    x1 = histo->xMin + i*binWidth;
	    x2 = x1 + binWidth;
	}
	
	/* skip any bins that are completely outside of plot area */
	if ((x1 < minXLim && x2 < minXLim) || (x1 > maxXLim && x2 > maxXLim))
            continue;

	/* calculate left and right edges of bin in window coordinates */
	hx1 = (int)XYDataToWindowX(&xform, x1);
	hx2 = (int)XYDataToWindowX(&xform, x2);

	/* if barSeparation is nonzero, adjust the bins by separation factor
	   of % of their width.  If this puts bin off display, skip it,
	   otherwise rounding below will put it back on inappropriately */
	if (histo->barSeparation > 0.) {
	    sep = ((hx2 - hx1) * histo->barSeparation) / 2;
	    hx1 += sep;
	    hx2 -= sep;
	    if (hx1 > xMax)
	    	continue;
	}
	
	/* calculate the top and bottom of the sides of the bin (if bins
	   are not separated, the top and bottom of the left leg of the bin) */
	y2 = *bin;
	hy1 = histo->barSeparation <= 0 ? lastY : yZero;
	hy2 = (int)XYDataToWindowY(&xform, dMax(minYLim, dMin(y2, maxYLim)));
	
	/* re-apply clipping in window coordinates to catch rounding errors */
	if (hx1 > xMax) hx1 = xMax; if (hx1 < xMin) hx1 = xMin;
	if (hx2 > xMax) hx2 = xMax; if (hx2 < xMin) hx2 = xMin;
	if (hy1 > yMax) hy1 = yMax; if (hy1 < yMin) hy1 = yMin;
	if (hy2 > yMax) hy2 = yMax; if (hy2 < yMin) hy2 = yMin;

	/* draw left and top sides of the current bin, and right if separated */
	xSeg->x1 = hx1; xSeg->x2 = hx2; xSeg->y1 = xSeg->y2 = hy2;
	xSeg++; nSegs++;
	xSeg->x1 = xSeg->x2 = hx1; xSeg->y1 = hy1; xSeg->y2 = hy2;
	xSeg++; nSegs++;
	if (histo->barSeparation > 0) {
	    xSeg->x1 = xSeg->x2 = hx2; xSeg->y1 = hy1; xSeg->y2 = hy2;
	    xSeg++; nSegs++;
	}
	
	/* add additional "shadow" segments to show overflow */
	if (y2 > maxYLim) {
            xSeg->x1 = hx1; xSeg->x2 = hx2; xSeg->y1 = xSeg->y2 = hy2 + 1;
            xSeg++; nSegs++;       
            xSeg->x1 = hx1; xSeg->x2 = hx2; xSeg->y1 = xSeg->y2 = hy2 + 2;
            xSeg++; nSegs++;       
	} else if (y2 < minYLim) {
            xSeg->x1 = hx1; xSeg->x2 = hx2; xSeg->y1 = xSeg->y2 = hy2 - 1;
            xSeg++; nSegs++;       
            xSeg->x1 = hx1; xSeg->x2 = hx2; xSeg->y1 = xSeg->y2 = hy2 - 2;
            xSeg++; nSegs++;       
	}
	lastY = hy2;
    }

    /* finish the right edge of the last bin if bins were not drawn separated */
    if (histo->barSeparation <= 0.) {
	hx1 = (int)XYDataToWindowX(&xform, histo->xMax);
	hy1 = (int)XYDataToWindowY(&xform, histo->bins[histo->nBins-1]);
	if (hx1 >= xMin && hx1 <= xMax && !(hy1 > yMax && yZero > yMax) &&
	    	!(hy1 < yMin && yZero < yMin)) {
            xSeg->x1 = xSeg->x2 = hx1; /* clipping isn't perfect here, could
            	    	    	    	  allow X's coordinates to overflow */
            xSeg->y1 = hy1;
            xSeg->y2 = yZero;
            xSeg++; nSegs++;
        }
    }
    
    /* Draw baseline at zero */
    hx1 = (int)XYDataToWindowX(&xform, histo->xMin - binWidth/4);
    hx2 = (int)XYDataToWindowX(&xform, histo->xMax + binWidth/4);
    if (hx1 <= xMax && hx2 >= xMin && yZero >= yMin && yZero <= yMax) {
    	xSeg->x1 = hx1 <= xMin ? xMin : hx1;
    	xSeg->x2 = hx2 >= xMax ? xMax : hx2;
    	xSeg->y1 = xSeg->y2 = yZero;
    	xSeg++; nSegs++;
    }
    
    /* draw the collected segments to the screen or post script file */
    setLineStyle(display, gc, histo->lineStyle, histo->linePixel);
    if (nSegs != 0) {
    	if (outDevice == X_SCREEN)
    	    XDrawSegments(display, drawBuf, gc, xSegs, nSegs);
    	else
            PSDrawDashedSegments(display, drawBuf, gc, xSegs, nSegs,
            	    DashLists[histo->lineStyle], 0);
    }
    XtFree((char *)xSegs);
}

static void drawHistMarkers(XYWidget w, Drawable drawBuf, int outDevice,
    	XYHistogram *histo)
{
    Display *display = XtDisplay(w);
    GC gc = w->xy.contentsGC;
    int xMin = w->xy.xOrigin, yMin = w->xy.yEnd;
    int xMax = w->xy.xEnd, yMax = w->xy.yOrigin;
    int hx, hy, i;
    int markStyle = HistMarkerStyles[histo->lineStyle-XY_N_LINE_STYLES].style;
    int markSize = HistMarkerStyles[histo->lineStyle-XY_N_LINE_STYLES].size;
    double minXLim = w->xy.minXLim, maxXLim = w->xy.maxXLim;
    double minYLim = w->xy.minYLim;
    double maxYLim = w->xy.maxYLim;
    double binWidth, x, y;
    float *bin;
    XYTransform xform;
    markStruct markList;
   
    /* don't bother if there's no data or if markers are not requested */
    if (histo->bins == NULL || histo->lineStyle < XY_N_LINE_STYLES ||
    	    histo->lineStyle >= XY_N_HIST_LINE_STYLES)
    	return;
    
    /* set up the information for transforming coordinates */
    ComputeTransform(w, &xform);
    binWidth = (histo->xMax - histo->xMin) / (double) (histo->nBins);
    
    /* Begin a list of markers to draw */
    startMarkList(&markList, markStyle, histo->nBins);

    /* loop through all of the data converting the data coordinates to
       X coordinates and drawing left and top segments of each bin */
    for (i=0, bin=histo->bins; i<histo->nBins; i++, bin++) {

	/* calculate the x coordinate of the of bin center in data coords */
	if (histo->edges)
	    x = (histo->edges[i] + histo->edges[i+1]) / 2;
	else
	    x = histo->xMin + i*binWidth + binWidth/2;
	
	/* skip any bins that are completely outside of plot area */
	if (x < minXLim || x > maxXLim || *bin < minYLim || *bin > maxYLim)
            continue;

	/* convert x and y (top of the bin) to window coordinates */
	hx = (int)XYDataToWindowX(&xform, x);
	hy = (int)XYDataToWindowY(&xform, *bin);
	
	/* re-apply clipping in window coordinates to catch rounding errors */
	if (hx > xMax) hx = xMax; if (hx < xMin) hx = xMin;
	if (hy > yMax) hy = yMax; if (hy < yMin) hy = yMin;

	/* draw a marker at the top/center of the bin */
	addToMarkList(&markList, hx, hy, markSize);
    }
        
    /* draw the collected segments to the screen or post script file */
    drawMarkList(display, drawBuf, gc, outDevice, &markList, markSize,
    	    markStyle, histo->linePixel);
}

/*
** Draw error bars from a histogram data structure.  
*/
static void drawHistErrBars(XYWidget w, Drawable drawBuf, int outDevice,
    	XYHistogram *histo)
{
    Display *display = XtDisplay(w);
    GC gc = w->xy.contentsGC;
    double minXLim = w->xy.minXLim, maxXLim = w->xy.maxXLim;
    double minYLim = w->xy.minYLim;
    double maxYLim = w->xy.maxYLim;
    XSegment *xSeg, *xSegs; int nSegs = 0; 
    double binWidth, x, y1, y2;
    float *bin;
    XYErrorBar *errBar;
    int hx1, hy1, hx2, hy2, sep, lastY, i;
    int halfErrWidth = (MED_MARK_SIZE * ERROR_MARKER_RATIO) / 2;
    XYTransform xform;
   
    /* don't bother if there's no data, or no error bars */
    if (histo->bins == NULL || histo->errorBars == NULL)
    	return;
    
    /* set up the information for transforming coordinates */
    ComputeTransform(w, &xform);
    binWidth = (histo->xMax - histo->xMin) / (double) (histo->nBins);
    
    /* Allocate memory for an array of XSegment structures for drawing lines. 
       Enough must be allocated for 3 segs and triple lines for overflow */
    xSegs = (XSegment *)XtMalloc(sizeof(XSegment)*((histo->nBins)*7));

    xSeg = xSegs;
    
    /* draw error bars */
    for (i=0,bin=histo->bins,errBar=histo->errorBars;
    	    i<histo->nBins; i++,bin++,errBar++) {

        /* Compute vertical part of bar and place in segment */
	x = histo->xMin + i*binWidth + (binWidth/2.0); /* Middle of bin */
	hx1 = hx2 = (int)XYDataToWindowX(&xform, x);
        y1 = *bin - errBar->min;
	hy1 = (int)XYDataToWindowY(&xform, dMax(minYLim, dMin(y1, maxYLim)));
        y2 = *bin + errBar->max;
	hy2 = (int)XYDataToWindowY(&xform, dMax(minYLim, dMin(y2, maxYLim)));

	/* If the error bar is not within the drawing area, skip it */
	if (x + binWidth < minXLim || x - binWidth > maxXLim ||
	    	y1 > maxYLim ||  y2 < minYLim)
	    continue;

        /* draw the vertical part of the bar */
        xSeg->x1 = hx1; xSeg->x2 = hx2; xSeg->y1 = hy1; xSeg->y2 = hy2;
        xSeg++; nSegs++;

        /* Compute and draw the horizontal part.  This is fixed to
           the width that looks good with medium size markers, maybe a
           bit small for histograms, but acceptable */
	hx1 = (int)XYDataToWindowX(&xform, x) - halfErrWidth;
	hx2 = hx1 + halfErrWidth * 2;
        xSeg->x1 = hx1; xSeg->x2 = hx2; xSeg->y1 = xSeg->y2 = hy1;
        xSeg++; nSegs++;
        xSeg->x1 = hx1; xSeg->x2 = hx2; xSeg->y1 = xSeg->y2 = hy2;
        xSeg++; nSegs++;

	/* add additional "shadow" segments to show overflow */
	if (y2 > maxYLim) {
            xSeg->x1 = hx1; xSeg->x2 = hx2; xSeg->y1 = xSeg->y2 = hy2 + 1;
            xSeg++; nSegs++;       
            xSeg->x1 = hx1; xSeg->x2 = hx2; xSeg->y1 = xSeg->y2 = hy2 + 2;
            xSeg++; nSegs++;       
	} 
	if (y1 < minYLim) {
            xSeg->x1 = hx1; xSeg->x2 = hx2; xSeg->y1 = xSeg->y2 = hy1 - 1;
            xSeg++; nSegs++;       
            xSeg->x1 = hx1; xSeg->x2 = hx2; xSeg->y1 = xSeg->y2 = hy1 - 2;
            xSeg++; nSegs++;       
	}
    }
    
    /* draw the collected segments to the screen or PostScript file */
    setLineStyle(display, gc, XY_PLAIN_LINE, histo->linePixel);
    if (nSegs != 0) {
    	if (outDevice == X_SCREEN)
    	    XDrawSegments(display, drawBuf, gc, xSegs, nSegs);
    	else
            PSDrawSegments(display, drawBuf, gc, xSegs, nSegs);
    }
    XtFree((char *)xSegs);
}


/*
** Draw the interior (filled part) of a histogram.  
*/
static void fillHistogram(XYWidget w, Drawable drawBuf, int outDevice,
    	XYHistogram *histo)
{
    Display *display = XtDisplay(w);
    GC gc = w->xy.contentsGC;
    int xMin = w->xy.xOrigin, yMin = w->xy.yEnd;
    int xMax = w->xy.xEnd, yMax = w->xy.yOrigin;
    double minXLim = w->xy.minXLim, maxXLim = w->xy.maxXLim;
    double minYLim = w->xy.minYLim;
    double maxYLim = w->xy.maxYLim;
    double binWidth, x1, x2;
    float *bin;
    int hx1, hy1, hx2, hy2, sep, i, yZero;
    XYTransform xform;
    int nClipRects;
    XRectangle *clipRects, *rect;

    /* If histogram doesn't require filling, return */
    if (histo->fillStyle == XY_NO_FILL || histo->lineStyle >= XY_N_LINE_STYLES)
    	return;
    	
    /* set up the information for transforming coordinates */
    ComputeTransform(w, &xform);
    binWidth = (histo->xMax - histo->xMin) / (double) (histo->nBins);
    yZero = (int)XYDataToWindowY(&xform, 0.);
    
    /* Loop through the bins of the histogram, creating a list of clipping
       rectangles that define the interior of the histogram */
    clipRects = (XRectangle *)XtMalloc(sizeof(XRectangle)*histo->nBins);
    rect = clipRects;
    nClipRects = 0;
    for (i=0, bin=histo->bins; i<histo->nBins; i++, bin++) {

	/* calculate left and right edges of bin in data coordinates, or
	   use the edges array, if supplied, to draw an adaptive histogram */
	if (histo->edges) {
	    x1 = histo->edges[i];
	    x2 = histo->edges[i+1];
	} else {
	    x1 = histo->xMin + i*binWidth;
	    x2 = x1 + binWidth;
	}
	
	/* skip any bins that are completely outside of plot area */
	if ((x1 < minXLim && x2 < minXLim) || (x1 > maxXLim && x2 > maxXLim))
            continue;

	/* calculate left and right edges of bin in window coordinates */
	hx1 = (int)XYDataToWindowX(&xform, x1);
	hx2 = (int)XYDataToWindowX(&xform, x2);
	
	/* calculate the top and bottom of the bin in window coordinates */
	hy1 = (int)XYDataToWindowY(&xform, dMax(minYLim, dMin(*bin, maxYLim)));
	if (hy1 > yZero) {
	    hy2 = hy1;
	    hy1 = yZero;
	} else
	    hy2 = yZero;

	/* if barSeparation is nonzero, adjust the bins by separation factor
	   of % of their width.  If this puts bin off display, skip it,
	   otherwise rounding below will put it back on inappropriately */
	if (histo->barSeparation > 0.) {
	    sep = ((hx2 - hx1) * histo->barSeparation) / 2;
	    hx1 += sep;
	    hx2 -= sep;
	    if (hx1 > xMax)
	    	continue;
	}
	
	/* re-apply clipping in window coordinates to catch rounding errors */
	if (hx1 > xMax) hx1 = xMax; if (hx1 < xMin) hx1 = xMin;
	if (hx2 > xMax) hx2 = xMax; if (hx2 < xMin) hx2 = xMin;
	if (hy1 > yMax) hy1 = yMax; if (hy1 < yMin) hy1 = yMin;
	if (hy2 > yMax) hy2 = yMax; if (hy2 < yMin) hy2 = yMin;

	/* create the clip rectangle for the bin and add it to the list */
	rect->x = hx1;
	rect->y = hy1;
	rect->width = hx2 - hx1 + (outDevice == X_SCREEN ? 1 : 0);
	rect->height = hy2 - hy1 + (outDevice == X_SCREEN ? 1 : 0);
	rect++;
	nClipRects++;
    }
    
    /* If no rectangles were accumulated, no parts of the histogram are
       visible, so don't bother with filling */
    if (nClipRects == 0) {
        XtFree((char *)clipRects);
    	return;
    }
    
    /* Set the clipping area for the gc to be used to draw the filling */
    if (outDevice == PS_PRINTER) {
    	fprintf(PSGetFile(), "gsave\n");
        PSSetClipRectangles(clipRects, nClipRects);
    } else
    	XSetClipRectangles(display, w->xy.contentsGC, 0, 0, clipRects,
    	    	nClipRects, Unsorted);
    XtFree((char *)clipRects);

    /* Draw the filling */
    drawFill(display, drawBuf, gc, outDevice, xMin, yMin, xMax, yMax,
    	     histo->fillStyle, histo->fillPixel);
    
    /* Restore clipping */
    if (outDevice == PS_PRINTER)
    	fprintf(PSGetFile(), "grestore\n");
    else
    	resetContentsGCClipping(w);
}

/*
** Draw the fill pattern implied by fillStyle into a rectangular area,
** subject to colors and clipping of gc.
*/
static void drawFill(Display *display, Drawable drawBuf, GC gc,
	int outDevice, int xMin, int yMin, int xMax, int yMax,
	unsigned fillStyle, Pixel fillPixel)
{
    XSegment *segs, *seg;
    int x, y, yStart, yEnd, xStart, xEnd, offset, spacing, nSegs, x2, y2;
    int width = xMax-xMin, height = yMax-yMin;
    int horizSlope, vertSlope, lineStyle, maxSegs;

    /* Don't fill if style is XY_NO_FILL */
    if (fillStyle == XY_NO_FILL)
    	return;
    	
    /* Solid filling (XY_SOLID_FILL) just set the color and fill a rectangle */
    if (fillStyle == XY_SOLID_FILL) {
    	XSetForeground(display, gc, fillPixel);
    	if (outDevice == PS_PRINTER)
    	    PSFillRectangle(display, drawBuf, gc, xMin, yMin, width+1,height+1);
    	else
    	    XFillRectangle(display, drawBuf, gc, xMin, yMin, width+1, height+1);
    	return;
    }
    
    /* Decode the fill-style.  Fill styles are actually made up of four
       1-byte fields: a line style for the fill lines, a slope for lines
       between horizontal and 45 degrees, a slope for lines between 45
       degrees and vertical, and a line spacing.  A slope of zero means
       don't draw the lines, and usually one slope will be zero.  The
       exception to this rule is cross hatching, where you want to
       actually draw two sets of lines. */
    spacing = fillStyle & SPACING_MASK;
    horizSlope = (fillStyle & H_SLOPE_MASK) >> H_SLOPE_OFFSET;
    vertSlope = (fillStyle & V_SLOPE_MASK) >> V_SLOPE_OFFSET;
    lineStyle = (fillStyle & LINE_STYLE_MASK) >> LINE_STYLE_OFFSET;
    
    /* Protect from bad values.  A spacing of 0 would cause crash or hang,
       as would line styles beyond array boundaries */
    if (spacing == 0 || lineStyle <= 0 || lineStyle >= XY_N_LINE_STYLES)
    	return;
    
    /* set the fill style and color for drawing the lines */
    setLineStyle(display, gc, lineStyle, fillPixel);
    
    /* Allocate enough memory for collecting all of the line segments
       generated by the calls below */
    maxSegs = (horizSlope == 0 ? 0 :
    	    (height + abs(((horizSlope - 128) * width) / 64)) / spacing + 1) +
    	    (vertSlope == 0 ? 0 :
    	    (width + abs(((vertSlope - 128) * height) / 64)) / spacing + 1);
    segs = (XSegment *)XtMalloc(sizeof(XSegment) * maxSegs);
    
    /* Create the horizontal segments, if any */
    seg = segs;
    nSegs = 0;
    if (horizSlope != 0) {
    	offset = ((horizSlope - 128) * width) / 64;
    	yStart = yMin - (offset < 0 ? 0 : offset);
    	yEnd = yMax + (offset > 0 ? 0 : -offset);
    	for (y=yStart, y2=yStart+offset; y<=yEnd; y+=spacing, y2+=spacing) {
    	    if (y < yMin) {
    	    	seg->x1 = xMin + ((yMin-y) * 64 / (horizSlope-128));
    	    	seg->y1 = yMin;
    	    } else if (y <= yMax) {
    	    	seg->x1 = xMin;
    	    	seg->y1 = y;
    	    } else {
    	    	seg->x1 = xMin - ((y-yMax) * 64 / (horizSlope-128));
    	    	seg->y1 = yMax;
    	    }
    	    if (y2 < yMin) {
    	    	seg->x2 = xMax + ((yMin-y2) * 64 / (horizSlope-128));
    	    	seg->y2 = yMin;
    	    } else if (y2 <= yMax) {
    		seg->x2 = xMax; 
    		seg->y2 = y2;
    	    } else {
   		seg->x2 = xMax - ((y2-yMax) * 64 / (horizSlope-128)); 
    		seg->y2 = yMax;
    	    }
    	    seg++; nSegs++;
    	}
    }
    
    /* Create the vertical segments, if any */
    if (vertSlope != 0) {
    	offset = ((vertSlope - 128) * height) / 64;
    	xStart = xMin - (offset < 0 ? 0 : offset);
    	xEnd = xMax + (offset > 0 ? 0 : -offset);
    	for (x=xStart, x2=xStart+offset; x<=xEnd; x+=spacing, x2+=spacing) {
    	    if (x < xMin) {
    	    	seg->y1 = yMin + ((xMin-x) * 64 / (vertSlope-128));
    	    	seg->x1 = xMin;
    	    } else if (x <= xMax) {
    	    	seg->y1 = yMin;
    	    	seg->x1 = x;
    	    } else {
    	    	seg->y1 = yMin - ((x-xMax) * 64 / (vertSlope-128));
    	    	seg->x1 = xMax;
    	    }
    	    if (x2 < xMin) {
    	    	seg->y2 = yMax + ((xMin-x2) * 64 / (vertSlope-128));
    	    	seg->x2 = xMin;
    	    } else if (x2 <= xMax) {
    		seg->y2 = yMax; 
    		seg->x2 = x2;
    	    } else {
   		seg->y2 = yMax - ((x2-xMax) * 64 / (vertSlope-128)); 
    		seg->x2 = xMax;
    	    }
    	    seg++; nSegs++;
    	}
    }
    
    /* Draw the segments */
    if (outDevice == PS_PRINTER)
        PSDrawDashedSegments(display, drawBuf, gc, segs, nSegs,
            	DashLists[lineStyle], 0);
    else
        XDrawSegments(display, drawBuf, gc, segs, nSegs);
    XtFree((char *)segs);
}

/*
** Allocate memory and initialize a lineList data structure to begin
** accumulating groups of lines to draw.  Lines are accumulated either
** in XPoint form, for the screen, or FloatPoint form for higher resolution
** on a PostScript printer.  Used in conjunction with addToLineList and
** drawLineList.  drawLineList deallocates the memory allocated here
*/
static void startLineList(lineStruct *lineList, int outDevice, int maxSegs)
{
    if (outDevice == X_SCREEN)
    	lineList->fillPtr = lineList->points =
    		(XPoint *)XtMalloc(sizeof(XPoint) * maxSegs * 2);
    else
    	lineList->fFillPtr = lineList->floatPoints =
    		(FloatPoint *)XtMalloc(sizeof(FloatPoint) * maxSegs * 2);
    lineList->lineCounts = (int *)XtMalloc(sizeof(int) 
    					     * (maxSegs == 0 ? 1 : maxSegs));
    lineList->curLineCount = lineList->lineCounts;
    *lineList->curLineCount = 0;
    lineList->outDevice = outDevice;
}

/*
** Add a new point to an accumulating line to be drawn later.  See
** startLineList and drawLineList for more information.  restart tells the
** function to begin a new run of connected segments (i.e. don't draw
** a line from the last point to this one).  If round is true, rounds
** x and y to the nearest integer value.  This is used in the case of
** printer output to reduce the resolution, since markers are not yet drawn
** at the higher resolution, and the lines don't look good if they don't
** join the markers (esp. small ones) exactly.
*/
static void addToLineList(lineStruct *lineList, float x, float y, int restart,
	int round)
{
    if (restart && *lineList->curLineCount != 0) {
	lineList->curLineCount++;
	*lineList->curLineCount = 0;
    }
    if (lineList->outDevice == X_SCREEN) {
	lineList->fillPtr->x = x;
	lineList->fillPtr->y = y;
	lineList->fillPtr++;
    } else {
	lineList->fFillPtr->x = round ? (int)x : x;
	lineList->fFillPtr->y = round ? (int)y : y;
	lineList->fFillPtr++;
    }
    (*lineList->curLineCount)++;
}

/*
** drawLineList draws the lines accumulated in lineList (see startLineList
** and addToLineList).  The arguments "lineStyle" and "linePixel"supply
** the color and style information.
**
** Uses repeated calls to either XDrawLines or PSFloatDrawLines.  Also
** deallocates memory attached to lineList, allocated by startLineList.
*/
static void drawLineList(Display *display, Drawable drawBuf, GC gc,
	int outDevice, lineStruct *lineList, int lineStyle, Pixel linePixel)
{
    XPoint *pt;
    FloatPoint *fpt;
    int nLineCounts = (lineList->curLineCount - lineList->lineCounts) + 1;
    int i, *count;
    
    /* set up the graphics context.  the convention for GCs in this widget
       is that line style, width, and color are not preserved */
    setLineStyle(display, gc, lineStyle, linePixel);

    if (outDevice == X_SCREEN) {
        pt = lineList->points;
        for (i=0, count=lineList->lineCounts; i<nLineCounts; i++, count++) {
	    XDrawLines(display, drawBuf, gc, pt, *count, 0);
    	    pt += *count;
        }
        XtFree((char *)lineList->points);
    } else { /* outDevice == PS_PRINTER */
        fpt = lineList->floatPoints;
        for (i=0, count=lineList->lineCounts; i<nLineCounts; i++, count++) {
	    PSFloatDrawDashedLines(display, drawBuf, gc, fpt, *count,
		    DashLists[lineStyle], 0);
    	    fpt += *count;
        }
        XtFree((char *)lineList->floatPoints);
    }
    XtFree((char *)lineList->lineCounts);
}

/*
** Set up a graphics context for drawing using information from an
** XYLineStyles value and a pixel value: line width, dashing, and color.  
*/
static void setLineStyle(Display *display, GC gc, int style, Pixel pixel)
{
    XGCValues gcValues;

    /* the convention for GCs in this widget
       is that line style, width, and color are not preserved */
    gcValues.line_width = style == XY_THICK_LINE ? 2 :
    	    (style == XY_X_THICK_LINE ? 3 : 0);
    gcValues.foreground = pixel;
    gcValues.line_style = (style==XY_PLAIN_LINE || style==XY_THICK_LINE ||
    	    style==XY_X_THICK_LINE) ? LineSolid : LineOnOffDash;
    XChangeGC(display, gc, GCLineWidth|GCLineStyle|GCForeground, &gcValues);
    XSetDashes(display, gc, 0, DashLists[style], strlen(DashLists[style]));
}

/*
** allocate storage for segments, rectangles or arcs for drawing markers
*/
static void startMarkList(markStruct *markList, int markerStyle, int maxLength)
{
    int size;
    
    markList->style = markerStyle;
    switch (markerStyle) {
      case XY_NO_MARK:
      	size = 0;
      	break;
      case XY_SQUARE_MARK: case XY_SOLID_SQUARE_MARK: case XY_THICK_SQUARE_MARK:
       	size = sizeof(XRectangle);
        break;
      case XY_CIRCLE_MARK: case XY_SOLID_CIRCLE_MARK: case XY_THICK_CIRCLE_MARK:
      	size = sizeof(XArc);
        break;
      case XY_STAR_MARK:
      	size = sizeof(XSegment) * 4;
        break;
      case XY_X_MARK:
      	size = sizeof(XSegment) * 2;
        break;
      case XY_TRIANGLE_MARK:
      	size = sizeof(XSegment) * 3;
        break;
    }
    markList->list = (void *)XtMalloc(size * maxLength);
    markList->fillPtr = markList->list;
    markList->nFigures = 0;
}

/*
** Add the appropriate figure(s) to markList to eventually draw a marker at
** the location (x,y).  The list must have been allocated with startMarkList,
** and should be drawn and disposed of with drawMarkList.
*/
static void addToMarkList(markStruct *markList, int x, int y, int size)
{
    int markWidth = MarkWidths[size];

    switch (markList->style) {
      case XY_NO_MARK:
      	break;
      case XY_SQUARE_MARK: case XY_SOLID_SQUARE_MARK: case XY_THICK_SQUARE_MARK:
      	((XRectangle *)markList->fillPtr)->x = x - markWidth/2;
      	((XRectangle *)markList->fillPtr)->y = y - markWidth/2;
      	((XRectangle *)markList->fillPtr)->width = markWidth;
      	((XRectangle *)markList->fillPtr)->height = markWidth;
      	markList->fillPtr += sizeof(XRectangle);
      	markList->nFigures++;
      	break;
      case XY_CIRCLE_MARK: case XY_SOLID_CIRCLE_MARK: case XY_THICK_CIRCLE_MARK:
      	((XArc *)markList->fillPtr)->x = x - markWidth/2;
      	((XArc *)markList->fillPtr)->y = y - markWidth/2;
      	((XArc *)markList->fillPtr)->width = markWidth;
      	((XArc *)markList->fillPtr)->height = markWidth;
      	((XArc *)markList->fillPtr)->angle1 = 0;
      	((XArc *)markList->fillPtr)->angle2 = 360 * 64;
      	markList->fillPtr += sizeof(XArc);
      	markList->nFigures++;
        break;
      case XY_STAR_MARK:
     	((XSegment *)markList->fillPtr)->x1 = x;
      	((XSegment *)markList->fillPtr)->y1 = y - markWidth/2;
      	((XSegment *)markList->fillPtr)->x2 = x;
      	((XSegment *)markList->fillPtr)->y2 = y + markWidth/2;
      	markList->fillPtr += sizeof(XSegment);
      	((XSegment *)markList->fillPtr)->x1 = x - markWidth/2;
      	((XSegment *)markList->fillPtr)->y1 = y;
      	((XSegment *)markList->fillPtr)->x2 = x + markWidth/2;
      	((XSegment *)markList->fillPtr)->y2 = y;
      	markList->fillPtr += sizeof(XSegment);
      	markList->nFigures += 2;
      	/* no break, uses XY_X_MARK */
      case XY_X_MARK:
      	((XSegment *)markList->fillPtr)->x1 = x - markWidth/2;
      	((XSegment *)markList->fillPtr)->y1 = y - markWidth/2;
      	((XSegment *)markList->fillPtr)->x2 = x + markWidth/2;
      	((XSegment *)markList->fillPtr)->y2 = y + markWidth/2;
      	markList->fillPtr += sizeof(XSegment);
      	((XSegment *)markList->fillPtr)->x1 = x + markWidth/2;
      	((XSegment *)markList->fillPtr)->y1 = y - markWidth/2;
      	((XSegment *)markList->fillPtr)->x2 = x - markWidth/2;
      	((XSegment *)markList->fillPtr)->y2 = y + markWidth/2;
      	markList->fillPtr += sizeof(XSegment);
      	markList->nFigures += 2;
      	break;
      case XY_TRIANGLE_MARK:
      	((XSegment *)markList->fillPtr)->x1 = x;
      	((XSegment *)markList->fillPtr)->y1 = y - markWidth/2;
      	((XSegment *)markList->fillPtr)->x2 = x + markWidth/2;
      	((XSegment *)markList->fillPtr)->y2 = y + markWidth/2;
      	markList->fillPtr += sizeof(XSegment);
      	((XSegment *)markList->fillPtr)->x1 = x + markWidth/2;
      	((XSegment *)markList->fillPtr)->y1 = y + markWidth/2;
      	((XSegment *)markList->fillPtr)->x2 = x - markWidth/2;
      	((XSegment *)markList->fillPtr)->y2 = y + markWidth/2;
      	markList->fillPtr += sizeof(XSegment);
      	((XSegment *)markList->fillPtr)->x1 = x - markWidth/2;
      	((XSegment *)markList->fillPtr)->y1 = y + markWidth/2;
      	((XSegment *)markList->fillPtr)->x2 = x;
      	((XSegment *)markList->fillPtr)->y2 = y - markWidth/2;
      	markList->fillPtr += sizeof(XSegment);
      	markList->nFigures += 3;
        break;
    }
}

/*
** Draw the markers accumulated with addToMarkList and free the list
*/
static void drawMarkList(Display *display, Drawable drawBuf, GC gc,
	int outDevice, markStruct *markList, int size, int style, Pixel color)
{
    XGCValues gcValues;
    
    /* set up the graphics context.  the convention for GCs in this widget
       is that line style, width, and color are not preserved */
    if (style==XY_THICK_SQUARE_MARK || style==XY_THICK_CIRCLE_MARK)
    	gcValues.line_width = size==XY_SMALL ? 1 :
    	        (size==XY_MEDIUM || size==XY_LARGE ? 2 : 0);
    else
    	gcValues.line_width = 0;
    gcValues.line_style = LineSolid;
    gcValues.foreground = color;
    XChangeGC(display, gc, GCLineWidth|GCLineStyle|GCForeground, &gcValues);

    if (outDevice == X_SCREEN) {
	switch (style) {
	  case XY_NO_MARK:
      	    break;
	  case XY_SQUARE_MARK: case XY_THICK_SQUARE_MARK:
            XDrawRectangles(display, drawBuf, gc, (XRectangle *)markList->list,
        	    markList->nFigures);
            break;
	   case XY_SOLID_SQUARE_MARK:
            XFillRectangles(display, drawBuf, gc, (XRectangle *)markList->list,
        	    markList->nFigures);
            break;
	  case XY_CIRCLE_MARK: case XY_THICK_CIRCLE_MARK:
            XDrawArcs(display, drawBuf, gc, (XArc *)markList->list,
        	    markList->nFigures);
            break;
	  case XY_SOLID_CIRCLE_MARK:
            XFillArcs(display, drawBuf, gc, (XArc *)markList->list,
        	    markList->nFigures);
            break;
	  case XY_STAR_MARK: case XY_X_MARK: case XY_TRIANGLE_MARK:
            XDrawSegments(display, drawBuf, gc, (XSegment *)markList->list,
        	    markList->nFigures);
            break;
	}
    } else { /* PS_PRINTER */
	switch (style) {
	  case XY_NO_MARK:
      	    break;
	  case XY_SQUARE_MARK: case XY_THICK_SQUARE_MARK:
            PSDrawRectangles(display, drawBuf, gc, (XRectangle *)markList->list,
        	    markList->nFigures);
            break;
	   case XY_SOLID_SQUARE_MARK:
            PSFillRectangles(display, drawBuf, gc, (XRectangle *)markList->list,
        	    markList->nFigures);
            break;
	  case XY_CIRCLE_MARK: case XY_THICK_CIRCLE_MARK:
            PSDrawArcs(display, drawBuf, gc, (XArc *)markList->list,
        	    markList->nFigures);
            break;
	  case XY_SOLID_CIRCLE_MARK:
            PSFillArcs(display, drawBuf, gc, (XArc *)markList->list,
        	    markList->nFigures);
            break;
	  case XY_STAR_MARK: case XY_X_MARK: case XY_TRIANGLE_MARK:
            PSDrawSegments(display, drawBuf, gc, (XSegment *)markList->list,
        	    markList->nFigures);
            break;
	}
    }
    XtFree((char *)markList->list);
}

/*
** Find the best layout for a legend for the current set of named curves that
** the widget contains.  Returns the number of rows, height, column width, and
** left margin for the optimal layout.
*/
static void layoutLegend(XYWidget w, int *nRows, int *legendHeight,
	int *columnWidth, int *leftMargin)
{
    int xOrigin = w->xy.xOrigin, xEnd = w->xy.xEnd;
    int n, row, rows, colHeight, maxColHeight, width, nCols;
    XYCurve *curve;
    XYHistogram *hist;
        
    /* If the legend is hidden, or there are no curves to show */
    if (!w->xy.showLegend || w->xy.nCurves + w->xy.nHistograms == 0) {
    	*nRows = 0;
    	*legendHeight = 0;
    	*leftMargin = 0;
    	*columnWidth = 0;
    	return;
    }
    
    /* Find how many rows and columns will fit all of the curve names,
       and the width of that layout for centering later */
    for (rows=1; rows<=w->xy.nCurves+w->xy.nHistograms; rows++)
    	if (tryLegendLayout(w, rows, &width, &nCols))
    	    break;
    *nRows = rows;
    
    /* Find the height of the legend */
    maxColHeight = 0;
    row = 1;
    colHeight = 0;
    for (curve=w->xy.curves, n=0; n<w->xy.nCurves; curve++, n++) {
    	colHeight += XmStringHeight(w->xy.font, curve->name);
    	if (row >= rows) {
    	    maxColHeight = colHeight > maxColHeight ? colHeight : maxColHeight;
    	    row = 1;
    	    colHeight = 0;
    	} else
    	    row++;
    }
    for (hist=w->xy.histograms, n=0; n<w->xy.nHistograms; hist++, n++) {
    	colHeight += XmStringHeight(w->xy.font, hist->name);
    	if (row >= rows) {
    	    maxColHeight = colHeight > maxColHeight ? colHeight : maxColHeight;
    	    row = 1;
    	    colHeight = 0;
    	} else
    	    row++;
    }
    *legendHeight = maxColHeight;
    
    /* determine a pleasant vertical spacing (column spacing and margins),
       with the goal of centering the legend under the horizontal axis line
       with the widest possible column spacing, but ignoring centering to
       pack the legend in as few rows as possible. */
    if (width > xEnd - xOrigin +
    	    2 * ((w->xy.xMax - LEGEND_RIGHT_MARGIN) - w->xy.xEnd)) {
    	/* can't center, just move left margin */
    	*columnWidth = LEGEND_COLUMN_MARGIN;
    	*leftMargin = w->xy.xMax - LEGEND_RIGHT_MARGIN - width;
    	if (*leftMargin < w->xy.xMin + LEGEND_LEFT_MARGIN)
    	    *leftMargin = w->xy.xMin + LEGEND_LEFT_MARGIN;
    } else if (width > xEnd - xOrigin || nCols <= 1) {
    	/* can center, but use tight column spacing */
    	*columnWidth = LEGEND_COLUMN_MARGIN;
    	*leftMargin = xOrigin + ((xEnd - xOrigin) - width)/2;
    } else {
    	/* can center and widen column spacing */
    	*columnWidth = ((xEnd - xOrigin) - (width -
    		(LEGEND_COLUMN_MARGIN*(nCols-1)))) / (nCols-1);
    	*leftMargin = xOrigin;
    }
}

/*
** Draw the plot legend at the bottom of the widget
*/
static void drawLegend(XYWidget w, Drawable drawBuf, int outDevice)
{
    Display *display = XtDisplay(w);
    GC gc = w->xy.gc;
    int markerWidth, lineWidth, style;
    int n, top, left, row, col, maxNameWidth, nameWidth, nameHeight;
    int sampleY, sampleY1, sampleY2, sampleX1, sampleX2, nameX;
    XYCurve *curve;
    XYHistogram *hist;
    
    top = w->xy.legendTop;
    left = w->xy.legendLeftMargin;
    row = 1;
    col = 1;
    maxNameWidth = 0;
    style = legendColumnStyle(w->xy.curves, w->xy.nCurves, w->xy.histograms,
    	    w->xy.nHistograms, 1, w->xy.legendRows);
    for (curve=w->xy.curves, n=0; n<w->xy.nCurves; curve++, n++) {
    	markerWidth = (style==NO_MARKERS ? 0 : LARGE_MARK_SIZE);
    	lineWidth = (style==NO_LINES ? 0 : LEGEND_LINE_LEN);
    	nameHeight = XmStringHeight(w->xy.font, curve->name);
    	nameWidth = XmStringWidth(w->xy.font, curve->name);
    	sampleY = top + nameHeight/2;
    	sampleX1 = left + markerWidth/2;
    	sampleX2 = sampleX1 + lineWidth;
    	nameX = left + LEGEND_NAME_SPACING + markerWidth + lineWidth;
    	if (curve->lineStyle != XY_NO_LINE)
    	    drawMarker(display, drawBuf, gc, outDevice, curve->markerSize,
    	    	    curve->markerStyle, curve->markerPixel, sampleX1, sampleY);
    	drawMarker(display, drawBuf, gc, outDevice, curve->markerSize,
    	    	curve->markerStyle, curve->markerPixel, sampleX2, sampleY);
    	drawLine(display, drawBuf, gc, outDevice, curve->lineStyle,
    	    	curve->linePixel, sampleX1, sampleY, sampleX2, sampleY);
    	XSetForeground(display, gc, w->primitive.foreground);
    	if (outDevice == X_SCREEN)
    	    XmStringDraw(display, drawBuf, w->xy.font, curve->name, gc,
    		    nameX, top, nameWidth, XmALIGNMENT_BEGINNING,
	     	    XmSTRING_DIRECTION_L_TO_R, NULL);
    	else
    	    PSDrawXmString(display, drawBuf, w->xy.font, curve->name, gc,
    		    nameX, top, nameWidth, XmALIGNMENT_BEGINNING);
	maxNameWidth = nameWidth > maxNameWidth ? nameWidth : maxNameWidth;
	if (row >= w->xy.legendRows) {
	    top = w->xy.legendTop;
    	    left = nameX + maxNameWidth + w->xy.legendColumnSpacing;
    	    row = 1;
    	    col++;
    	    maxNameWidth = 0;
    	    style = legendColumnStyle(w->xy.curves, w->xy.nCurves,
    	    	    w->xy.histograms, w->xy.nHistograms, col, w->xy.legendRows);
	} else {
	    top += nameHeight;
	    row++;
	}
    }
    for (hist=w->xy.histograms, n=0; n<w->xy.nHistograms; hist++, n++) {
    	markerWidth = (style==NO_MARKERS ? 0 : LARGE_MARK_SIZE);
    	lineWidth = (style==NO_LINES ? 0 : LEGEND_LINE_LEN);
    	nameHeight = XmStringHeight(w->xy.font, hist->name);
    	nameWidth = XmStringWidth(w->xy.font, hist->name);
    	sampleY1 = top + 1;
    	sampleY2 = top + nameHeight - 1;
    	sampleX1 = left + markerWidth/2;
    	sampleX2 = sampleX1 + lineWidth;
    	nameX = left + LEGEND_NAME_SPACING + markerWidth + lineWidth;
    	if (hist->lineStyle < XY_N_LINE_STYLES)
    	    drawFill(display, drawBuf, gc, outDevice, sampleX1, sampleY1, 
    	    	    sampleX2, sampleY2, hist->fillStyle, hist->fillPixel);
    	XSetForeground(display, gc, w->primitive.foreground);
    	if (outDevice == X_SCREEN)
    	    XmStringDraw(display, drawBuf, w->xy.font, hist->name, gc,
    		    nameX, top, nameWidth, XmALIGNMENT_BEGINNING,
	     	    XmSTRING_DIRECTION_L_TO_R, NULL);
    	else
    	    PSDrawXmString(display, drawBuf, w->xy.font, hist->name, gc,
    		    nameX, top, nameWidth, XmALIGNMENT_BEGINNING);
    	if (hist->lineStyle!=XY_NO_LINE && hist->lineStyle<XY_N_LINE_STYLES) {
    	    setLineStyle(display, gc, hist->lineStyle, hist->linePixel);
    	    if (outDevice == X_SCREEN)
    		XDrawRectangle(display, drawBuf, gc, sampleX1, sampleY1,
        		sampleX2 - sampleX1, sampleY2 - sampleY1);
    	    else
    		PSDrawDashedRectangle(display, drawBuf, gc, sampleX1, sampleY1,
        		sampleX2 - sampleX1, sampleY2 - sampleY1,
        		DashLists[hist->lineStyle], 0);
	} else if (hist->lineStyle >= XY_N_LINE_STYLES) {
	    drawMarker(display, drawBuf, gc, outDevice,
	    	    HistMarkerStyles[hist->lineStyle - XY_N_LINE_STYLES].size,
	    	    HistMarkerStyles[hist->lineStyle - XY_N_LINE_STYLES].style,
	    	    hist->linePixel, sampleX2, (sampleY1 + sampleY2) / 2);
	}
	maxNameWidth = nameWidth > maxNameWidth ? nameWidth : maxNameWidth;
	if (row >= w->xy.legendRows) {
	    top = w->xy.legendTop;
    	    left = nameX + maxNameWidth + w->xy.legendColumnSpacing;
    	    row = 1;
    	    col++;
    	    maxNameWidth = 0;
    	    style = legendColumnStyle(w->xy.curves, w->xy.nCurves,
    	    	    w->xy.histograms, w->xy.nHistograms, col, w->xy.legendRows);
	} else {
	    top += nameHeight;
	    row++;
	}
    }
}

/*
** Find out if a layout using a given number of rows is possible, and if
** so, the minimum width and number of columns it would contain.
*/
static int tryLegendLayout(XYWidget w, int nRows, int *width, int *nColumns)
{
    int item, colStart, colWidth, row, fieldWidth, nCols, style;
    int maxWidth = (w->xy.xMax - LEGEND_RIGHT_MARGIN) -
    	    (w->xy.xMin + LEGEND_LEFT_MARGIN);
    
    item = 0;
    colStart = 0;
    nCols = 0;
    while (item < w->xy.nCurves + w->xy.nHistograms) {
        nCols++;
        style = legendColumnStyle(w->xy.curves, w->xy.nCurves,
    	    	w->xy.histograms, w->xy.nHistograms, nCols, nRows);
        colWidth = 0;
        for (row=1; row<=nRows; row++) {
            if (item < w->xy.nCurves) { /* curve */
        	fieldWidth = (style==NO_LINES ? 0 : LEGEND_LINE_LEN) +
            		(style==NO_MARKERS ? 0 : LARGE_MARK_SIZE) +
            		LEGEND_NAME_SPACING + (w->xy.curves[item].name == NULL ?
            		0 : XmStringWidth(w->xy.font, w->xy.curves[item].name));
            } else { /* histogram */
            	fieldWidth = (style==NO_LINES ? 0 : LEGEND_LINE_LEN) +
            		(style==NO_MARKERS ? 0 : LARGE_MARK_SIZE) +
            		LEGEND_NAME_SPACING +
            		(w->xy.histograms[item-w->xy.nCurves].name == NULL ? 0 :
            		XmStringWidth(w->xy.font,
            		w->xy.histograms[item-w->xy.nCurves].name));
            }
            colWidth = fieldWidth > colWidth ? fieldWidth : colWidth;
            if (++item >= w->xy.nCurves + w->xy.nHistograms)
            	break;
        }
        if (colStart+colWidth > maxWidth &&
            	nRows < w->xy.nCurves + w->xy.nHistograms) {
            *width = maxWidth;
            *nColumns = 1;
            return False;
        }
        colStart += colWidth + LEGEND_COLUMN_MARGIN;
    }
    *width = colStart - LEGEND_COLUMN_MARGIN;
    *nColumns = nCols;
    return True;
}

/*
** Given a column number and the number of rows in the legend, calculates
** how the column should appear: NO_LINES, NO_MARKERS, or FULL_LEGEND
*/
static int legendColumnStyle(XYCurve *curves, int nCurves, XYHistogram *hists,
    	int nHists, int colNum, int nRows)
{
    int i, hasMarkers = False, hasLines = False;
    
    for (i=(colNum-1)*nRows; i<nCurves+nHists && i<colNum*nRows; i++) {
    	if (i < nCurves) {
    	    if (curves[i].markerStyle != XY_NO_MARK)
    		hasMarkers = True;
    	    if (curves[i].lineStyle != XY_NO_LINE)
    		hasLines = True;
    	} else { /* histogram */
    	    if (hists[i-nCurves].lineStyle >= XY_N_LINE_STYLES &&
    	    	    hists[i-nCurves].fillStyle == XY_NO_FILL)
    	    	hasMarkers = True;
    	    else
    	    	hasLines = True;
    	}
    }
    if (hasMarkers && hasLines) return FULL_LEGEND;
    else if (hasLines) return NO_MARKERS;
    else return NO_LINES;
}

/*
** Draw a single marker of style defined by curve, centered on (x, y)
*/
static void drawMarker(Display *display, Drawable drawBuf, GC gc, int outDevice,
	int size, int style, Pixel color, int x, int y)
{
    markStruct markList;

    if (style == XY_NO_MARK)
    	return;
    startMarkList(&markList, style, 1);
    addToMarkList(&markList, x, y, size);
    drawMarkList(display, drawBuf, gc, outDevice, &markList, size, style,color);
}

/*
** Draw a single line of style defined by lineStyle in color linePixel,
** from (x1, y1) to (x2, y2)
*/
static void drawLine(Display *display, Drawable drawBuf, GC gc, int outDevice,
	int lineStyle, Pixel linePixel, int x1, int y1, int x2, int y2)
{
    if (lineStyle == XY_NO_LINE)
    	return;

    setLineStyle(display, gc, lineStyle, linePixel);

    if (outDevice == X_SCREEN)
        XDrawLine(display, drawBuf, gc, x1, y1, x2, y2);
    else /* outDevice == PS_PRINTER */
        PSDrawDashedLine(display, drawBuf, gc, x1, y1, x2, y2,
		DashLists[lineStyle], 0);
}

/*
** For smoother animation, it is possible to allocate an offscreen pixmap
** and draw to that rather than directly into the window.  Unfortunately,
** it is too slow on many machines, so we have to give the user the choice
** whether to use it or not.  This routine reallocates the offscreen
** pixmap buffer when the window is resized, and when the widget is created
*/
static void updateBufferAllocation(XYWidget w)
{ 
    if (w->xy.drawBuffer)
    	XFreePixmap(XtDisplay(w), w->xy.drawBuffer);
    if (w->xy.doubleBuffer) {
    	w->xy.drawBuffer = XCreatePixmap(XtDisplay(w),
		DefaultRootWindow(XtDisplay(w)), w->core.width, w->core.height,
    	 	DefaultDepthOfScreen(XtScreen(w)));   
    } else {
    	w->xy.drawBuffer = 0;
    }
}

/*
** Get the XFontStruct that corresponds to the default (first) font in
** a Motif font list.  Since Motif stores this, it saves us from storing
** it or querying it from the X server.
*/
static XFontStruct *getFontStruct(XmFontList font)
{
    XFontStruct *fs;
    XmFontContext context;
    XmStringCharSet charset;

    XmFontListInitFontContext(&context, font);
    XmFontListGetNextFont(context, &charset, &fs);
    XmFontListFreeFontContext(context);
    XtFree(charset);
    return fs;
}

/*
** Calculate the actual extent of the data in the widget.  As a side-effect,
** may reset xLogScaling or yLogScaling to false if data can not be displayed
** in log scaling mode.
*/
static void calcDataRange(XYWidget w, double *xMin, double *xMax, double *yMin,
	double *yMax, double *xPosMin, double *yPosMin)
{
    XYCurve *curve;
    XYHistogram *hist;
    double minX = DBL_MAX, minY = DBL_MAX, maxX = -DBL_MAX, maxY = -DBL_MAX;
    double posMinX = DBL_MAX, posMinY = DBL_MAX;
    double cMinX, cMaxX, cMinY, cMaxY, cPosMinX, cPosMinY, logMin, logMax;
    int n, hasData = False;
 
    /* Combine the ranges from all of the curves */
    for (n=0, curve=w->xy.curves; n<w->xy.nCurves; n++, curve++) {
	if (oneCurveDataRange(w, curve, &cMinX, &cMaxX, &cMinY, &cMaxY,
	    	&cPosMinX, &cPosMinY)) {
	    minX = dMin(minX, cMinX);
	    maxX = dMax(maxX, cMaxX);
	    minY = dMin(minY, cMinY);
	    maxY = dMax(maxY, cMaxY);
	    posMinX = dMin(posMinX, cPosMinX);
	    posMinY = dMin(posMinY, cPosMinY);
	    hasData = True;
	}
    }
    
    /* expand the range by plot margin percentage (used mostly to make
       room for markers and error bar tops) */
    if (hasData) {
	if (w->xy.xLogScaling) {
	    if (minX > 0.)
    		expandLogRange(&minX, &maxX, w->xy.marginPercent / 100.);
	    else
	    	expandLogRange(&posMinX, &maxX, w->xy.marginPercent / 100.);
	} else if (maxX >= minX) {
	    minX -= (maxX - minX) * w->xy.marginPercent / 100;
	    maxX += (maxX - minX) * w->xy.marginPercent / 100;
	}
	if (w->xy.yLogScaling) {
	    if (minY > 0.)
    		expandLogRange(&minY, &maxY, w->xy.marginPercent / 100.);
	    else
	    	expandLogRange(&posMinY, &maxY, w->xy.marginPercent / 100.);
	} else if (maxY >= minY) {
	    minY -= (maxY - minY) * w->xy.marginPercent / 100;
	    maxY += (maxY - minY) * w->xy.marginPercent / 100;
	}
    }
    
    /* Combine the ranges from all of the histograms */
    for (n=0, hist=w->xy.histograms; n<w->xy.nHistograms; n++, hist++) {
        calcHistDataRange(w, hist, &cMinX, &cMaxX, &cMinY, &cMaxY, &cPosMinX,
            	&cPosMinY);
        minX = dMin(minX, cMinX);
	maxX = dMax(maxX, cMaxX);
	minY = dMin(minY, cMinY);
	maxY = dMax(maxY, cMaxY);
	posMinX = dMin(posMinX, cPosMinX);
	posMinY = dMin(posMinY, cPosMinY);
	hasData = True;
    }
    
    /* If there was no data, just make something up */
    if (!hasData) {
	*xMin = *yMin = 0.;
	*xMax = *yMax = 1.;
	*xPosMin = *yPosMin = .5;
	return;
    }
    
    *xMin = minX; *yMin = minY; *xMax = maxX; *yMax = maxY;
    *xPosMin = posMinX; *yPosMin = posMinY;
}

/*
** Find the rectangular area covered by a curve, including error bars and
** decorations (markers and error bar ends).  Return False if the curve
** has no points.
*/
static int oneCurveDataRange(XYWidget w, XYCurve *curve, double *xMin,
	double *xMax, double *yMin, double *yMax, double *xPosMin,
	double *yPosMin)
{
    XYPoint *point;
    XYErrorBar *vert, *hor;
    double minX = DBL_MAX, minY = DBL_MAX, maxX = -DBL_MAX, maxY = -DBL_MAX;
    double posMinX = DBL_MAX, posMinY = DBL_MAX;
    int i;

    /* Make sure there's data */
    if (curve->nPoints == 0)
    	return False;
    	
    /* Loop through all of the data, recording minimum and maximum values */
    if (curve->vertBars != NULL && curve->horizBars != NULL) {
	for (i=0, point=curve->points, vert=curve->vertBars,
	    	hor=curve->horizBars; i<curve->nPoints;
	    	i++, point++, vert++, hor++) {
    	    if (point->x > maxX) maxX = point->x;
    	    if (hor->max > maxX) maxX = hor->max;
    	    if (point->y > maxY) maxY = point->y;
    	    if (vert->max > maxY) maxY = vert->max;
    	    if (point->x < minX) minX = point->x;
    	    if (hor->min < minX) minX = hor->min;
    	    if (point->y < minY) minY = point->y;
    	    if (vert->min < minY) minY = vert->min;
    	    if (point->x < posMinX && point->x > 0.) posMinX = point->x;
    	    if (point->y < posMinY && point->y > 0.) posMinY = point->y;
    	    if (hor->min < posMinX && hor->min > 0.) posMinX = hor->min;
    	    if (vert->min < posMinY && vert->min > 0.) posMinY = vert->min;
	}
    } else if (curve->vertBars != NULL) {
	for (i=0, point=curve->points, vert=curve->vertBars;
	    	i<curve->nPoints; i++, point++, vert++) {
    	    if (point->x > maxX) maxX = point->x;
    	    if (point->y > maxY) maxY = point->y;
    	    if (vert->max > maxY) maxY = vert->max;
    	    if (point->x < minX) minX = point->x;
    	    if (point->y < minY) minY = point->y;
    	    if (vert->min < minY) minY = vert->min;
    	    if (point->x < posMinX && point->x > 0.) posMinX = point->x;
    	    if (point->y < posMinY && point->y > 0.) posMinY = point->y;
    	    if (vert->min < posMinY && vert->min > 0.) posMinY = vert->min;
	}
    } else if (curve->horizBars != NULL) {
	for (i=0, point=curve->points, hor=curve->horizBars;
	    	i<curve->nPoints; i++, point++, hor++) {
    	    if (point->x > maxX) maxX = point->x;
    	    if (hor->max > maxX) maxX = hor->max;
    	    if (point->y > maxY) maxY = point->y;
    	    if (point->x < minX) minX = point->x;
    	    if (hor->min < minX) minX = hor->min;
    	    if (point->y < minY) minY = point->y;
    	    if (point->x < posMinX && point->x > 0.) posMinX = point->x;
    	    if (point->y < posMinY && point->y > 0.) posMinY = point->y;
    	    if (hor->min < posMinX && hor->min > 0.) posMinX = hor->min;
	}
    } else {
	for (i=0, point=curve->points; i<curve->nPoints; i++, point++) {
    	    if (point->x > maxX) maxX = point->x;
    	    if (point->y > maxY) maxY = point->y;
    	    if (point->x < minX) minX = point->x;
    	    if (point->y < minY) minY = point->y;
    	    if (point->x < posMinX && point->x > 0.) posMinX = point->x;
    	    if (point->y < posMinY && point->y > 0.) posMinY = point->y;
	}
    }
    
    *xMin = minX; *yMin = minY; *xMax = maxX; *yMax = maxY;
    *xPosMin = posMinX; *yPosMin = posMinY;
    return True;
}
 
/*
** Scans the histogram data stored in the widget & returns the minimum
** and maximum values appropriate for that data.
*/
static void calcHistDataRange(XYWidget w, XYHistogram *hist, double *xMin,
	double *xMax, double *yMin, double *yMax, double *xPosMin,
	double *yPosMin)
{
    double min, max, posMin;
    float *bin;
    int i;
    XYErrorBar *errBar;
    
    /* xMin and xMax are given directly in the histogram */
    *xMin = hist->xMin;
    *xMax = hist->xMax;
    if (hist->xMin > 0)
    	*xPosMin = hist->xMin;
    else
    	*xPosMin = dMax(DBL_MIN, pow(10.,
    	    	(int)log10(((hist->xMax-hist->xMin) / hist->nBins) / 10.)));
    
    /* Scan the data calculating min and max */
    min = DBL_MAX; max = -DBL_MAX, posMin = DBL_MAX;
    if (hist->errorBars == NULL) {
	for (i=0,bin=hist->bins; i<hist->nBins; i++, bin++) {
    	    if (*bin > max) max = *bin;
    	    if (*bin < min) min = *bin;
    	    if (*bin < posMin && *bin > 0.) posMin = *bin;
	}
    } else {
    	for (i=0, bin=hist->bins, errBar=hist->errorBars;
    		i<hist->nBins; i++, bin++, errBar++) {
    	    if (*bin + errBar->max > max) max = *bin + errBar->max;
    	    if (*bin - errBar->min < min) min = *bin - errBar->min;
    	    if (*bin - errBar->min < posMin && *bin - errBar->min > 0.)
    	    	posMin = *bin - errBar->min;
	}
    }
    
    /* Histograms implicitly start from zero (unless minimum is negative),
       rather than from the lowest bin value, even if there are no zeros. */
    min = dMin(min, 0.);

    /* if min is the same as max, bump up max */
    if (min == max)
    	max += 1.;
    
    /* return the values */
    *yMin = min; *yMax = max; *yPosMin = posMin;
}

/* Compute constants for data to window coordinate transformation */
void ComputeTransform(XYWidget w, XYTransform *xform)
{    
    double maxXLim, maxYLim;
    
    /* If log scaling was requested, express limits in log coordinates */
    if (w->xy.xLogScaling) {
    	xform->minXData = w->xy.minXData > 0. ? log10(w->xy.minXData): 0.;
    	xform->minXLim = w->xy.minXLim > 0. ? log10(w->xy.minXLim) : 0.;
    	maxXLim = w->xy.maxXLim > 0. ? log10(w->xy.maxXLim) : 0.;
    } else {
    	xform->minXData = w->xy.minXData;
    	xform->minXLim = w->xy.minXLim;
    	maxXLim = w->xy.maxXLim;
    }
    if (w->xy.yLogScaling) {
    	xform->minYData = w->xy.minYData > 0. ? log10(w->xy.minYData) : 0.;
    	xform->minYLim = w->xy.minYLim > 0. ? log10(w->xy.minYLim) : 0.;
    	maxYLim = w->xy.maxYLim > 0. ? log10(w->xy.maxYLim) : 0.;
    } else {
    	xform->minYData = w->xy.minYData;
    	xform->minYLim = w->xy.minYLim;
    	maxYLim = w->xy.maxYLim;
    }
    xform->xScale = (w->xy.xEnd - w->xy.xOrigin) /
    	    (maxXLim - xform->minXLim);
    xform->yScale = (w->xy.yOrigin - w->xy.yEnd) /
    	    (maxYLim - xform->minYLim);
    xform->minXPix = w->xy.xOrigin -
    	    (xform->minXLim - xform->minXData) * xform->xScale;
    xform->maxYPix = w->xy.yOrigin +
    	    (xform->minYLim - xform->minYData) * xform->yScale;
    xform->xLogScaling = w->xy.xLogScaling;
    xform->yLogScaling = w->xy.yLogScaling;
    xform->xOrigin = w->xy.xOrigin;
    xform->yOrigin = w->xy.yOrigin;
}

/* Convert X and Y values from data coordinates to window coordinates.
   Requires valid transformation data from ComputeTransform above */
double XYDataToWindowX(XYTransform *xform, double value)
{
    if (xform->xLogScaling) {
    	if (value > 0.)
	    return xform->minXPix +
	    	    (log10(value)-xform->minXData) * xform->xScale;
	else
	    return SHRT_MIN/4;
    } else
	return xform->minXPix + (value - xform->minXData) * xform->xScale;
}
double XYDataToWindowY(XYTransform *xform, double value)
{
    if (xform->yLogScaling) {
    	if (value > 0.)
	    return xform->maxYPix -
	    	    (log10(value)-xform->minYData) * xform->yScale;
	else
	    return SHRT_MAX/4;
	    
    } else
	return xform->maxYPix - (value - xform->minYData) * xform->yScale;
}
double XYWindowToDataX(XYTransform *xform, double value)
{
    return xform->minXLim + (double)(value - xform->xOrigin) / xform->xScale;
}
double XYWindowToDataY(XYTransform *xform, double value)
{
    return xform->minYLim + (double)(xform->yOrigin - value) / xform->yScale;
}

/* Copies data from one curve data structure to another */
static void copyCurveData(XYCurve *fromCurve, XYCurve *toCurve, int dataOnly)
{
    int pointDataSize, errDataSize;
    
    if (!dataOnly)
	copyCurveStyle(fromCurve, toCurve);
    pointDataSize = sizeof(XYPoint) * fromCurve->nPoints;
    toCurve->points = (XYPoint *)XtMalloc(pointDataSize);
    toCurve->nPoints = fromCurve->nPoints;
    memcpy(toCurve->points, fromCurve->points, pointDataSize);
    errDataSize = sizeof(XYErrorBar) * fromCurve->nPoints;
    if (fromCurve->horizBars != NULL) {
	toCurve->horizBars = (XYErrorBar *)XtMalloc(errDataSize);
	memcpy(toCurve->horizBars, fromCurve->horizBars, errDataSize);
    } else
	toCurve->horizBars = NULL;
    if (fromCurve->vertBars != NULL) {
	toCurve->vertBars = (XYErrorBar *)XtMalloc(errDataSize);
	memcpy(toCurve->vertBars, fromCurve->vertBars, errDataSize);
    } else
	toCurve->vertBars = NULL;
}

/* Copies data from one histogram data structure to another */
static void copyHistogramData(XYHistogram *fromHisto, XYHistogram *toHisto,
    	int dataOnly)
{
    int errDataSize;
    
    if (!dataOnly)
	copyHistogramStyle(fromHisto, toHisto);
    toHisto->xMin = fromHisto->xMin;
    toHisto->xMax = fromHisto->xMax;
    toHisto->bins = (float *)XtMalloc(sizeof(float) * fromHisto->nBins);
    toHisto->nBins = fromHisto->nBins;
    memcpy(toHisto->bins, fromHisto->bins, sizeof(float) * fromHisto->nBins);
    if (fromHisto->edges != NULL) {
    	toHisto->edges =
    	    	(float *)XtMalloc(sizeof(float) * (fromHisto->nBins + 1));
    	memcpy(toHisto->edges, fromHisto->edges,
    	    	sizeof(float) * (fromHisto->nBins + 1));
    } else
    	toHisto->edges = NULL;
    if (fromHisto->errorBars != NULL) {
 	errDataSize = sizeof(XYErrorBar) * fromHisto->nBins;
	toHisto->errorBars = (XYErrorBar *)XtMalloc(errDataSize);
	memcpy(toHisto->errorBars, fromHisto->errorBars, errDataSize);
    } else
	toHisto->errorBars = NULL;
}

/* Copies name and style information from one curve data structure to another */
static void copyCurveStyle(XYCurve *fromCurve, XYCurve *toCurve)
{
    toCurve->name = fromCurve->name == NULL ?
    	    NULL : XmStringCopy(fromCurve->name);
    toCurve->markerStyle = fromCurve->markerStyle;
    toCurve->markerSize = fromCurve->markerSize;
    toCurve->lineStyle = fromCurve->lineStyle;
    toCurve->markerPixel = fromCurve->markerPixel;
    toCurve->linePixel = fromCurve->linePixel;
}

/* Copies name and style information from one histo data structure to another */
static void copyHistogramStyle(XYHistogram *fromHisto, XYHistogram *toHisto)
{
    toHisto->name = fromHisto->name == NULL ?
    	    NULL : XmStringCopy(fromHisto->name);
    toHisto->lineStyle = fromHisto->lineStyle;
    toHisto->fillStyle = fromHisto->fillStyle;
    toHisto->linePixel = fromHisto->linePixel;
    toHisto->fillPixel = fromHisto->fillPixel;
    toHisto->barSeparation = fromHisto->barSeparation;
}

/* Free the data WITHIN a curve data structure (not the structure itself) */
static void freeCurveData(XYCurve *curve, int dataOnly)
{
    if (!dataOnly && curve->name != NULL)
    	XmStringFree(curve->name);
    if (curve->points != NULL)
    	XtFree((char *)curve->points);
    if (curve->horizBars != NULL)
    	XtFree((char *)curve->horizBars);
    if (curve->vertBars != NULL)
    	XtFree((char *)curve->vertBars);
}

/* Free the data WITHIN a histogram data structure (not the structure itself) */
static void freeHistogramData(XYHistogram *histo, int dataOnly)
{
    if (!dataOnly && histo->name != NULL)
    	XmStringFree(histo->name);
    if (histo->bins != NULL)
    	XtFree((char *)histo->bins);
    if (histo->edges != NULL)
    	XtFree((char *)histo->edges);
    if (histo->errorBars != NULL)
    	XtFree((char *)histo->errorBars);
}

/* Find out if there is anything for the widget to draw */
static int widgetHasData(XYWidget w)
{
    int i;
    
    if (w->xy.nStrings !=0)
    	return True;
    if (w->xy.nHistograms != 0)
    	return True;
    for (i=0; i<w->xy.nCurves; i++)
    	if (w->xy.curves[i].nPoints != 0)
    	    return True;
    return False;
}

/*
** Adjust a pair of values representing plotting limits in log mode, to
** expand the range between "*min" and "*max" by "fraction" in terms of
** the displayed coordinate system.  That is, expand the range in terms
** of screen space, leaving an equal sized margin on both ends of the range
*/
static void expandLogRange(double *min, double *max, double fraction)
{
    double logMin, logMax;
    
    logMin = log10(*min);
    logMax = log10(*max);
    *min = dMax(DBL_MIN, pow(10., logMin - 
    	    (logMax - logMin) * fraction));
    *max = dMax(DBL_MIN, pow(10., logMax +
    	    (logMax - logMin) * fraction));
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

/*******************************************************************************
*                                                                              *
* LineBox.c -- 1-D Histogram Widget                                                *
*                                                                              *
* Copyright (c) 1991 Universities Research Association, Inc.                   *
* All rights reserved.                                                         *
*                                                                              *
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
*                                                                              *
* Fermilab Nirvana GUI Library                                                 *
* Sep 24, 1992                                                                 *
*                                                                              *
* jck	10/04/92    original 			                               *
* jck   03/15/93    Error Bars                                                 *
* mwe   09/2/93     Adaptive Histograms                                        *
*                                                                              *
*******************************************************************************/
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#if XmVersion == 1002
#include <Xm/PrimitiveP.h>
#include <Xm/DrawP.h>
#endif
#include "../util/psUtils.h"
#include "drawAxes.h"
#include "dragAxes.h"
#include "LineBoxP.h"

#define REDRAW_NONE 0
#define REDRAW_H_AXIS 1
#define REDRAW_V_AXIS 2
#define REDRAW_CONTENTS 4
#define REDRAW_LABELS 8
#define REDRAW_ALL 15

#define ZOOM_FACTOR .25		/* (linear) fraction of currently displayed
				   data to place outside of current limits 
				   when user invokes zoom command */
#define LOG_MIN_LIMIT .5	/* artificial lower plot limit to be used when
				   log scaling is on and data limit < 0 */
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
#define MAX_AXIS_PERCENT 22	/* maximum percentage of widget width that may
				   be used for drawing axes */

static void destroy(Widget w);
static void resize(Widget w);
static void motionAP(Widget w, XEvent *event, char *args, int n_args);
static void btnUpAP(Widget w, XEvent *event, char *args, int n_args);
static void btn2AP(Widget w, XEvent *event, char *args, int n_args);
static void btn3AP(Widget w, XEvent *event, char *args, int n_args);
static void initialize(LineBoxWidget request, LineBoxWidget new);
static void redisplay(Widget w, XEvent *event, Region region);
static void redisplayContents(LineBoxWidget w, int outDevice,
			      int redrawArea);
static void drawLineBox(LineBoxWidget w, Drawable drawBuf, int outDevice);
static void drawPoints(LineBoxWidget w, Drawable drawBuf, int outDevice);
static Boolean setValues(LineBoxWidget current, LineBoxWidget request,LineBoxWidget new);
static void updateBufferAllocation(LineBoxWidget w);
static XFontStruct *getFontStruct(XmFontList font);
static void adjustLimitsForLogScaling(LineBoxWidget w);
static void calcYDataRange(LineBoxWidget w, double *minimum, double *maximum);
static double dMin(double d1, double d2);
static double dMax(double d1, double d2);

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
      XtOffset(LineBoxWidget, lineBox.doubleBuffer), XmRString, "False"},
    {XmNlogScaling, XmCLogScaling, XmRBoolean, sizeof(Boolean),
      XtOffset(LineBoxWidget, lineBox.logScaling), XmRString, "False"},
    {XmNbinEdgeLabeling, XmCBinEdgeLabeling, XmRBoolean, sizeof(Boolean),
      XtOffset(LineBoxWidget, lineBox.binEdgeLabeling), XmRString, "False"},
    {XmNfontList, XmCFontList, XmRFontList, sizeof(XmFontList),
      XtOffset(LineBoxWidget, lineBox.font), XmRImmediate, NULL},
    {XmNxAxisLabel, XmCXAxisLabel, XmRXmString, sizeof (XmString), 
      XtOffset(LineBoxWidget, lineBox.xAxisLabel), XmRString, NULL},
    {XmNyAxisLabel, XmCYAxisLabel, XmRXmString, sizeof (XmString), 
      XtOffset(LineBoxWidget, lineBox.yAxisLabel), XmRString, NULL},
    {XmNbarSeparation, XmCBarSeparation, XmRInt, sizeof(int), 
      XtOffset(LineBoxWidget, lineBox.barSepPercent), XmRString, "0"},
    {XmNresizeCallback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (LineBoxWidget, lineBox.resize), XtRCallback, NULL},
    {XmNbtn2Callback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (LineBoxWidget, lineBox.btn2), XtRCallback, NULL},
    {XmNbtn3Callback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (LineBoxWidget, lineBox.btn3), XtRCallback, NULL},
    {XmNredisplayCallback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (LineBoxWidget, lineBox.redisplay), XtRCallback, NULL},
};

LineBoxClassRec  lineBoxClassRec = {
     /* CoreClassPart */
  {
    (WidgetClass)&xmPrimitiveClassRec,/* superclass          */
    "LineBox",                            /* class_name            */
    sizeof(LineBoxRec),                   /* widget_size           */
    NULL,                             /* class_initialize      */
    NULL,                             /* class_part_initialize */
    FALSE,                            /* class_inited          */
    (XtInitProc)initialize,           /* initialize            */
    NULL,                             /* initialize_hook       */
    XtInheritRealize,                 /* realize               */
    actionsList,                      /* actions               */
    XtNumber(actionsList),            /* num_actions           */
    resources,                        /* resources             */
    XtNumber(resources),              /* num_resources         */
    NULLQUARK,                        /* xrm_class             */
    TRUE,                             /* compress_motion       */
    TRUE,                             /* compress_exposure     */
    TRUE,                             /* compress_enterleave   */
    TRUE,                             /* visible_interest      */
    destroy,                          /* destroy               */
    resize,                           /* resize                */
    redisplay,                        /* expose                */
    (XtSetValuesFunc)setValues,       /* set_values            */
    NULL,                             /* set_values_hook       */
    XtInheritSetValuesAlmost,         /* set_values_almost     */
    NULL,                             /* get_values_hook       */
    NULL,                             /* accept_focus          */
    XtVersion,                        /* version               */
    NULL,                             /* callback private      */
    defaultTranslations,              /* tm_table              */
    NULL,                             /* query_geometry        */
    NULL,                             /* display_accelerator   */
    NULL,                             /* extension             */
  },
  /* Motif primitive class fields */
  {
     (XtWidgetProc)_XtInherit, 		/* Primitive border_highlight   */
     (XtWidgetProc)_XtInherit,		/* Primitive border_unhighlight */
     XtInheritTranslations,		/* translations                 */
     (XtActionProc)motionAP,		/* arm_and_activate             */
     NULL,				/* get resources      		*/
     0,					/* num get_resources  		*/
     NULL,         			/* extension                    */
  },
  /* LineBox class part */
  {
    0,                              	/* ignored	                */
  }
};

WidgetClass lineBoxWidgetClass = (WidgetClass) &lineBoxClassRec;

/*
** Widget initialize method
*/
static void initialize(LineBoxWidget request, LineBoxWidget new)
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
    if (new->lineBox.font == NULL)
    	new->lineBox.font =
    	    XmFontListCopy(_XmGetDefaultFontList(
    	    			(Widget) new, XmLABEL_FONTLIST));
    else
        new->lineBox.font = XmFontListCopy(new->lineBox.font);

    /* Make local copies of the XmStrings */
    if (new->lineBox.xAxisLabel != NULL)
    	new->lineBox.xAxisLabel = XmStringCopy(new->lineBox.xAxisLabel);
    if (new->lineBox.yAxisLabel != NULL)
    	new->lineBox.yAxisLabel = XmStringCopy(new->lineBox.yAxisLabel);
     
    /* Create graphics contexts for drawing in the widget */
    values.font = getFontStruct(new->lineBox.font)->fid;
    values.foreground = new->primitive.foreground;
    values.background = new->core.background_pixel;
    new->lineBox.gc = XCreateGC(display, XDefaultRootWindow(display),
    	    GCForeground|GCBackground|GCFont, &values);
    new->lineBox.contentsGC = XCreateGC(display, XDefaultRootWindow(display),
    	    GCForeground|GCBackground, &values);
    
    /* Initialize various fields */
    ResetAxisDragging(&new->lineBox.dragState);
    new->lineBox.bins = NULL;
    new->lineBox.edges = NULL;
    new->lineBox.nBins = 0;
    new->lineBox.uppErr = NULL;
    new->lineBox.lowErr = NULL;
    
    /* Default plotting boundaries  */
    LineBoxSetContents((Widget)new, 0.0, 0.0, 0, NULL, NULL, NULL, LineBox_RESCALE);

    /* Set size dependent items */
    new->lineBox.drawBuffer = 0;
    resize((Widget)new);
}

/*
** Widget destroy method
*/
static void destroy(Widget w)
{
    LineBoxWidget hw = (LineBoxWidget)w;
    
    XFreeGC(XtDisplay(w), hw->lineBox.gc);
    XFreeGC(XtDisplay(w), hw->lineBox.contentsGC);
    if (hw->lineBox.font != NULL)
    	XmFontListFree(hw->lineBox.font);
    if (hw->lineBox.bins != NULL)
     	XtFree((char *)hw->lineBox.bins);
    if (hw->lineBox.edges != NULL)
     	XtFree((char *)hw->lineBox.edges);
    XtRemoveAllCallbacks (w, XmNresizeCallback);
    XtRemoveAllCallbacks (w, XmNbtn2Callback);
    XtRemoveAllCallbacks (w, XmNbtn3Callback);
    XtRemoveAllCallbacks (w, XmNredisplayCallback);
    if (hw->lineBox.xAxisLabel != NULL)
    	XmStringFree(hw->lineBox.xAxisLabel);
    if (hw->lineBox.yAxisLabel != NULL)
    	XmStringFree(hw->lineBox.yAxisLabel);
}

/*
** Widget resize method
*/
static void resize(Widget w)
{
    LineBoxWidget hw = (LineBoxWidget)w;
    
    XFontStruct *fs = getFontStruct(hw->lineBox.font);
    int borderWidth =
    	hw->primitive.shadow_thickness + hw->primitive.highlight_thickness;
    XRectangle clipRect;

    /* resize the drawing buffer, an offscreen pixmap for smoother animation */
    updateBufferAllocation(hw); 

    /* calculate the area of the widget where contents can be drawn */
    hw->lineBox.xMin = borderWidth;
    hw->lineBox.yMin = borderWidth;
    hw->lineBox.xMax = w->core.width - borderWidth;
    hw->lineBox.yMax = w->core.height - borderWidth;

    /* calculate positions for the axes and contents depending on whether
       axis labels are specified, and the measurements of the current font */
    if (hw->lineBox.yAxisLabel != NULL)
    	hw->lineBox.yEnd = hw->lineBox.yMin + fs->ascent + fs->descent + TOP_MARGIN;
    else
    	hw->lineBox.yEnd = VAxisEndClearance(fs) + fs->ascent/2 + TOP_MARGIN;
    hw->lineBox.axisTop = hw->lineBox.yEnd - VAxisEndClearance(fs);
    if (hw->lineBox.xAxisLabel != NULL)
    	hw->lineBox.axisBottom = hw->lineBox.yMax - BOTTOM_MARGIN - fs->ascent -
    				fs->descent - X_LABEL_MARGIN;
    else
    	hw->lineBox.axisBottom = hw->lineBox.yMax - fs->ascent/2 - BOTTOM_MARGIN;
    hw->lineBox.yOrigin = hw->lineBox.axisBottom - HAxisHeight(fs);
    hw->lineBox.axisLeft = hw->lineBox.xMin + LEFT_MARGIN;
    hw->lineBox.xOrigin = hw->lineBox.axisLeft + VAxisWidth(fs);
    if (hw->lineBox.xOrigin > (hw->lineBox.xMax - hw->lineBox.xMin) * MAX_AXIS_PERCENT/100)
    	hw->lineBox.xOrigin = (hw->lineBox.xMax - hw->lineBox.xMin) * MAX_AXIS_PERCENT/100;
    hw->lineBox.axisRight = hw->lineBox.xMax - RIGHT_MARGIN;
    hw->lineBox.xEnd = hw->lineBox.axisRight - HAxisEndClearance(fs);
    if (hw->lineBox.xMax - hw->lineBox.xEnd > hw->lineBox.xOrigin - hw->lineBox.xMin)
    	hw->lineBox.xEnd = hw->lineBox.xMax - (hw->lineBox.xOrigin - hw->lineBox.xMin);
    
    /* set plot contents gc to clip drawing at the edges */
    clipRect.x = hw->lineBox.xOrigin;
    clipRect.y = hw->lineBox.yEnd;
    clipRect.width = hw->lineBox.xEnd - hw->lineBox.xOrigin + 1;
    clipRect.height = hw->lineBox.yOrigin - hw->lineBox.yEnd;
    XSetClipRectangles(XtDisplay(w), hw->lineBox.contentsGC, 0, 0, &clipRect,
    		       1, Unsorted);

    /* set drawing gc to clip drawing before motif shadow and highlight */
    clipRect.x = borderWidth;
    clipRect.y = borderWidth;
    clipRect.width = w->core.width - 2 * borderWidth;
    clipRect.height = w->core.height - 2 * borderWidth;
    XSetClipRectangles(XtDisplay(w), hw->lineBox.gc, 0, 0, &clipRect, 1, Unsorted);
    
    /* call the resize callback */
    if (XtIsRealized(w))
    	XtCallCallbacks(w, XmNresizeCallback, NULL);
}

/*
** Widget redisplay method
*/
static void redisplay(Widget w, XEvent *event, Region region)
{
    LineBoxWidget hw = (LineBoxWidget)w;
    
    /* Draw the Motif required shadows and highlights */
    if (hw->primitive.shadow_thickness > 0) {
	_XmDrawShadow (XtDisplay(w), XtWindow(w), 
	      hw->primitive.bottom_shadow_GC, hw->primitive.top_shadow_GC,
              hw->primitive.shadow_thickness, hw->primitive.highlight_thickness,
              hw->primitive.highlight_thickness,
              hw->core.width - 2 * hw->primitive.highlight_thickness,
              hw->core.height-2 * hw->primitive.highlight_thickness);
    }
    if (hw->primitive.highlighted)
	_XmHighlightBorder(w);
    else if (_XmDifferentBackground(w, XtParent((Widget)w)))
	_XmUnhighlightBorder(w);
    
    /* Now draw the contents of the lineBox widget */
    redisplayContents((LineBoxWidget)w, X_SCREEN, REDRAW_ALL);
}

/*
** Widget setValues method
*/
static Boolean setValues(LineBoxWidget current, LineBoxWidget request, LineBoxWidget new)
{
    Boolean redraw = False, doResize = False;
    Display *display = XtDisplay(new);

    /* If the foreground or background color has changed, change the GCs */
    if (new->core.background_pixel !=current->core.background_pixel) {
    	XSetForeground(display, new->lineBox.gc, new->primitive.foreground);
    	XSetForeground(display, new->lineBox.contentsGC,new->primitive.foreground);
    	redraw = TRUE;
    }

    if (new->primitive.foreground != current->primitive.foreground) {
    	XSetBackground(display, new->lineBox.gc, new->core.background_pixel);
    	XSetBackground(display,new->lineBox.contentsGC,new->core.background_pixel);
    	redraw = TRUE;
    }

    /* if double buffering changes, allocate or deallocate offscreen pixmap */
    if (new->lineBox.doubleBuffer != current->lineBox.doubleBuffer) {
    	updateBufferAllocation(new);
    	redraw = TRUE;
    }

    /* if log scaling changes, adjust or restore data limits,
       reset dragging, and redraw */
    if (new->lineBox.logScaling != current->lineBox.logScaling) {
    	if (new->lineBox.logScaling)
    	    adjustLimitsForLogScaling(new);
    	else
	    calcYDataRange(new, &new->lineBox.minYData, &new->lineBox.maxYData);
    	ResetAxisDragging(&new->lineBox.dragState);
    	redraw = TRUE;
    }

    /* if labels are changed, free the old ones and copy the new ones */
    if (new->lineBox.xAxisLabel != current->lineBox.xAxisLabel) {
    	if (current->lineBox.xAxisLabel != NULL)
    	    XmStringFree(current->lineBox.xAxisLabel);
    	new->lineBox.xAxisLabel = XmStringCopy(new->lineBox.xAxisLabel);
    	doResize = TRUE;
    }
    if (new->lineBox.yAxisLabel != current->lineBox.yAxisLabel) {
    	if (current->lineBox.yAxisLabel != NULL)
    	    XmStringFree(current->lineBox.yAxisLabel);
    	new->lineBox.yAxisLabel = XmStringCopy(new->lineBox.yAxisLabel);
    	doResize = TRUE;
    }
    
    /* if highlight thickness or shadow thickness changed, resize and redraw */
    if  ((new->primitive.highlight_thickness != 
          current->primitive.highlight_thickness) ||
         (new -> primitive.shadow_thickness !=
          current->primitive.shadow_thickness)) {
    	redraw = TRUE;
    }
    
    /* if bin separation changes, redraw */
    if (new->lineBox.barSepPercent != current->lineBox.barSepPercent)
    	redraw = TRUE;
    	
    if (doResize)
    	resize((Widget)new);
    return redraw; 
} 

/*
** Button press and button motion action proc.
*/
static void motionAP(Widget w, XEvent *event, char *args, int n_args)
{
    LineBoxWidget hw = (LineBoxWidget)w;
    int chgdArea, redrawArea = REDRAW_NONE;

    if (event->type == ButtonPress)
    	XmProcessTraversal((Widget)w, XmTRAVERSE_CURRENT);
    
    chgdArea = DragAxes(event, hw->lineBox.xOrigin, hw->lineBox.xEnd, hw->lineBox.yOrigin,
    	hw->lineBox.yEnd, hw->lineBox.axisLeft, hw->lineBox.axisTop, hw->lineBox.axisBottom,
    	hw->lineBox.axisRight, hw->lineBox.minXData, hw->lineBox.maxXData, hw->lineBox.minYData,
    	hw->lineBox.maxYData, False, hw->lineBox.logScaling, &hw->lineBox.minXLim,
    	&hw->lineBox.maxXLim, &hw->lineBox.minYLim, &hw->lineBox.maxYLim,
    	&hw->lineBox.dragState, &hw->lineBox.xDragStart, &hw->lineBox.yDragStart);
    if (chgdArea & DA_REDRAW_H_AXIS) redrawArea |= REDRAW_H_AXIS;
    if (chgdArea & DA_REDRAW_V_AXIS) redrawArea |= REDRAW_V_AXIS;
    if (chgdArea & DA_REDRAW_CONTENTS) redrawArea |= REDRAW_CONTENTS;

    redisplayContents(hw, X_SCREEN, redrawArea);
}
/*
** Button up action proc.
*/
static void btnUpAP(Widget w, XEvent *event, char *args, int n_args)
{
    ResetAxisDragging(&((LineBoxWidget)w)->lineBox.dragState);
    drawLineBox((LineBoxWidget)w, XtWindow(w), X_SCREEN); 
    drawPoints((LineBoxWidget)w, XtWindow(w), X_SCREEN); 
}

static void btn2AP(Widget w, XEvent *event, char *args, int n_args)
{
    LineBoxCallbackStruct cbStruct;

#ifdef MOTIF10
    _XmProcessTraversal(w, XmTRAVERSE_CURRENT);
#else
    XmProcessTraversal(w, XmTRAVERSE_CURRENT);
#endif   

    /* Just call the callback */
    cbStruct.reason = XmCR_INPUT;
    cbStruct.event = event;
    XtCallCallbacks(w, XmNbtn2Callback, (char *)&cbStruct);
}

static void btn3AP(Widget w, XEvent *event, char *args, int n_args)
{    
    LineBoxCallbackStruct cbStruct;

#ifdef MOTIF10
    _XmProcessTraversal(w, XmTRAVERSE_CURRENT);
#else
    XmProcessTraversal(w, XmTRAVERSE_CURRENT);
#endif   

    /* Just call the callback */
    cbStruct.reason = XmCR_INPUT;
    cbStruct.event = event;
    XtCallCallbacks(w, XmNbtn3Callback, (char *)&cbStruct);
}

/*
** LineBoxSetContents
**
** Specify the contents of the lineBox widget in the form of an array of bins
** and optional arrays of error data
**
** Parameters
**
** 	w		A lineBox widget
**	xMin, xMax	Low edge of first bin, right edge of last bin
**	nBins		The number of bins in the histogram
**	bins		Bin contents, an array floats nBins long
**	uppErr, lowErr	Upper and lower error bar data stored as offsets
**			from top of bin, arrays of floats nBins long 
**	rescale		One of: LineBox_NO_RESCALE, LineBox_RESCALE, or
**			LineBox_GROW_ONLY.  Tells the widget how to change
**			its display to incorporate the new contents.
*/
void LineBoxSetContents(Widget w, float xMin, float xMax, int nBins, float *bins,
		    float *uppErr, float *lowErr, int rescale)
{
    LineBoxWidget hw = (LineBoxWidget)w;
    int oldNBins = hw->lineBox.nBins;
    int redrawArea, i;
    double minX, minY, maxX, maxY;
    
    /* Free the previous data */
    if (hw->lineBox.bins != NULL) {
    	XtFree((char *)hw->lineBox.bins);
        hw->lineBox.nBins = 0;
    }
    if (hw->lineBox.uppErr != NULL) {
    	XtFree((char *)hw->lineBox.uppErr);
    	hw->lineBox.uppErr = NULL;
    }
    if (hw->lineBox.lowErr != NULL) {
    	XtFree((char *)hw->lineBox.lowErr);
    	hw->lineBox.lowErr = NULL;
    }
    
    /* Copy in the new data, if any */
    if (bins == NULL || nBins == 0) {
    	hw->lineBox.bins = NULL;
    	bins = NULL;
    	nBins = 0;
    } else {
	hw->lineBox.bins = (float *)XtMalloc(sizeof (float) * nBins);
	memcpy (hw->lineBox.bins, bins, sizeof (float) * nBins);
    }
    hw->lineBox.nBins = nBins;
    if (uppErr != NULL && lowErr != NULL) {
	hw->lineBox.uppErr = (float *)XtMalloc(sizeof (float) * nBins);
	hw->lineBox.lowErr = (float *)XtMalloc(sizeof (float) * nBins);
	for (i=0; i<nBins; i++) {
	    hw->lineBox.uppErr[i] = fabs(uppErr[i]);
	    hw->lineBox.lowErr[i] = fabs(lowErr[i]);
	}
    }
    
    /* Calculate range of data, or fill in defaults if there's no data */
    minX = xMin; maxX = xMax;
    if (hw->lineBox.nBins == 0 && rescale == LineBox_RESCALE) {
    	minY = hw->lineBox.logScaling ? .5 : 0.;
    	maxY = 1.;
    } else
	calcYDataRange(hw, &minY, &maxY);

    /* If either range is 0, make up fake min and max values */
    if (maxX == minX) maxX += 1.;
    if (maxY == minY) maxY += 1.;
    
    /* recalculate scale, limits, and redraw area for the widget */
    redrawArea = oldNBins != nBins ? REDRAW_ALL : REDRAW_NONE;
    if (rescale == LineBox_RESCALE) {
    	hw->lineBox.maxXLim = maxX; hw->lineBox.minXLim = minX;
    	hw->lineBox.maxYLim = maxY; hw->lineBox.minYLim = minY;
    	hw->lineBox.maxYData = maxY; hw->lineBox.minYData = minY;
    	adjustLimitsForLogScaling(hw);
    	redrawArea = REDRAW_CONTENTS | REDRAW_H_AXIS | REDRAW_V_AXIS;
    } else if (rescale == LineBox_NO_RESCALE) {
	redrawArea |= REDRAW_CONTENTS;
	if (maxY > hw->lineBox.maxYData) {
	    hw->lineBox.maxYData = maxY;
	    redrawArea |= REDRAW_V_AXIS;
	}
	if (minY < hw->lineBox.minYData) {
	    hw->lineBox.minYData = minY;
	    redrawArea |= REDRAW_V_AXIS;
	}
    } else if (rescale == LineBox_RESCALE_AT_MAX || rescale == LineBox_GROW_ONLY) {
	redrawArea |= REDRAW_CONTENTS;
	if (rescale == LineBox_GROW_ONLY)
	    maxY = dMax(hw->lineBox.maxYData, maxY);
    	if (hw->lineBox.maxXData != maxX || hw->lineBox.minXData != minX)
    	    redrawArea |= REDRAW_H_AXIS;
    	if (hw->lineBox.maxYData != maxY || hw->lineBox.minYData != minY)
    	    redrawArea |= REDRAW_V_AXIS;
    	if (hw->lineBox.maxXData == hw->lineBox.maxXLim)
    	    hw->lineBox.maxXLim = maxX;
    	if (hw->lineBox.minXData == hw->lineBox.minXLim)
    	    hw->lineBox.minXLim = minX;
    	if (hw->lineBox.maxYData == hw->lineBox.maxYLim)
    	    hw->lineBox.maxYLim = maxY;
    	if (hw->lineBox.minYData == hw->lineBox.minYLim)
    	    hw->lineBox.minYLim = minY;
   	hw->lineBox.maxYData = maxY; hw->lineBox.minYData = minY;
    } else if (rescale == LineBox_REBIN_MODE) {
	redrawArea |= REDRAW_ALL;
    	if (hw->lineBox.maxXData == hw->lineBox.maxXLim)
    	    hw->lineBox.maxXLim = maxX;
    	if (hw->lineBox.minXData == hw->lineBox.minXLim)
    	    hw->lineBox.minXLim = minX;
    	if (hw->lineBox.maxYData == hw->lineBox.maxYLim)
    	    hw->lineBox.maxYLim = maxY;
    	else
    	    hw->lineBox.maxYLim =
		dMin(maxY, hw->lineBox.maxYLim * (double)oldNBins/(double)nBins);
    	if (hw->lineBox.minYData == hw->lineBox.minYLim)
    	    hw->lineBox.minYLim = minY;
	else if (!(hw->lineBox.logScaling && hw->lineBox.minYLim == LOG_MIN_LIMIT))
	    hw->lineBox.minYLim =
		dMax(minY, hw->lineBox.minYLim * (double)oldNBins/(double)nBins);
   	hw->lineBox.maxYData = maxY; hw->lineBox.minYData = minY;
     }
    hw->lineBox.maxXData = maxX; hw->lineBox.minXData = minX;

    /* redraw the widget with the new data */
    if (XtIsRealized(w))
    	redisplayContents(hw, X_SCREEN, redrawArea);
}

/*
** LineBoxSetContentsAdaptive
**
** Specify the contents of the widget as an adaptive histogram in the form
** an array of bins and an array of bin edges.  After calling this routine,
** you should not use LineBoxSetContents and cannot return the widget to being
** a plain histogram.
**
** Parameters
**
** 	w		A LineBox widget
**	nBins		The number of bins in the histogram
**	bins		Bin contents, an array floats nBins long
**	edges		Bin edges in data coordinates nBins + 1 long
**	rescale		One of: LineBox_NO_RESCALE, LineBox_RESCALE, or
**			LineBox_GROW_ONLY.  Tells the widget how to change
**			its display to incorporate the new contents.
*/
void LineBoxSetContentsAdaptive(Widget w, int nBins, float *bins, float *edges,
	int rescale)
{
    LineBoxWidget hw = (LineBoxWidget)w;
    float xMin, xMax;

    /* Free the previous edges data and copy in new edges data */
    if (hw->lineBox.edges != NULL)
    	XtFree((char *)hw->lineBox.edges);
    if (edges == NULL) {
    	hw->lineBox.edges = NULL;
    } else {
    	hw->lineBox.edges = (float *)XtMalloc(sizeof(float) * (nBins+1));
    	memcpy(hw->lineBox.edges, edges, sizeof (float) * (nBins+1));
    }
    
    /* Set the contents as if this was a regular histogram, using the first
       edge as a min value, and the last as a max value.  The drawing
       routines will use the edges array if it is supplied, to draw the
       histogram as adaptive. In LineBox_NO_RESCALE mode, leave the X axis as is */
    if (rescale == LineBox_NO_RESCALE) {
    	xMin = hw->lineBox.minXData;
    	xMax = hw->lineBox.maxXData;
    } else if (edges == NULL) {
    	xMin = 0;
    	xMax = 1;
    } else {
    	xMin = edges[0];
    	xMax = edges[nBins];
    }
    LineBoxSetContents(w, xMin, xMax, nBins, bins, NULL, NULL, rescale);
}

/*
** LineBoxSetVisibleRange, LineBoxGetVisibleRange
**
** Set (Get) the range of data that is visible.  minXLim, minYLim, maxXLim, and
** maxYLim specify the endpoints of the x and y axes.  LineBoxSetVisibleRange,
** unlike the widgets interactive rescaling routines, can zoom out past the
** actual minimum and maximum data points.
*/
void LineBoxSetVisibleRange(Widget w, double minXLim, double minYLim,
			double maxXLim, double maxYLim)
{
    LineBoxWidget hw = (LineBoxWidget)w;
    double minY, maxY;
    
    /* calculate the range necessary to fully display the data */
    calcYDataRange(hw, &minY, &maxY);

    /* allow user to zoom beyond the range of the data by artificially
       adjusting data limits */
    hw->lineBox.maxYData = dMax(maxYLim, maxY);
    hw->lineBox.minYData = dMin(minYLim, minY);

    /* Set the range */
    hw->lineBox.minXLim = dMax(minXLim, hw->lineBox.minXData);
    hw->lineBox.maxXLim = dMin(maxXLim, hw->lineBox.maxXData);
    hw->lineBox.minYLim = minYLim;
    hw->lineBox.maxYLim = maxYLim;
    
    /* if log scaling was requested and minYLim <= 0, adjust the new limits */
    if (minYLim <= 0)
    	adjustLimitsForLogScaling(hw);

    /* redraw if the widget is realized */
    if (XtIsRealized(w))
    	redisplayContents(hw, X_SCREEN,
    		REDRAW_V_AXIS | REDRAW_H_AXIS | REDRAW_CONTENTS);
}
void LineBoxGetVisibleRange(Widget w, double *minXLim, double *minYLim,
			double *maxXLim, double *maxYLim)
{
    *minXLim = ((LineBoxWidget)w)->lineBox.minXLim;
    *maxXLim = ((LineBoxWidget)w)->lineBox.maxXLim;
    *minYLim = ((LineBoxWidget)w)->lineBox.minYLim;
    *maxYLim = ((LineBoxWidget)w)->lineBox.maxYLim;
}

/*
** LineBoxZoomOut, LineBoxZoomIn, LineBoxResetZoom
**
** Zoom in and out by ZOOM_FACTOR.  Zoom in is centered on the current
** center of the plot.
*/
void LineBoxZoomOut(Widget w)
{
    LineBoxWidget hw = (LineBoxWidget)w;
    int logScaling = hw->lineBox.logScaling;
    double xOffset, yOffset, newMaxXLim, newMinXLim, newMaxYLim, newMinYLim;
    double minXLim, maxXLim, minYLim, maxYLim;
    int redrawArea = REDRAW_NONE;
    
    /* if log scaling was requested, express limits in log coordinates */
    minXLim = hw->lineBox.minXLim;
    maxXLim = hw->lineBox.maxXLim;
    minYLim = logScaling ? log10(hw->lineBox.minYLim) : hw->lineBox.minYLim;
    maxYLim = logScaling ? log10(hw->lineBox.maxYLim) : hw->lineBox.maxYLim;

    /* Calculate a suitable offset to reverse a zoom in by ZOOM_FACTOR */
    xOffset = (maxXLim - minXLim) * (ZOOM_FACTOR/(1.-ZOOM_FACTOR)) / 2;
    yOffset = (maxYLim - minYLim) * (ZOOM_FACTOR/(1.-ZOOM_FACTOR)) / 2;

    /* widen the plotting limits by the offsets calculated above,
       stopping when the limits reach the limits of the data */
    newMaxXLim = dMin(hw->lineBox.maxXData, maxXLim + xOffset);
    newMinXLim = dMax(hw->lineBox.minXData, minXLim - xOffset);
    if (logScaling) {
	newMaxYLim = dMin(hw->lineBox.maxYData, pow(10., maxYLim + yOffset));
	newMinYLim = dMax(.5, pow(10., minYLim - yOffset));
    } else {
    	newMaxYLim = dMin(hw->lineBox.maxYData, maxYLim + yOffset);
	newMinYLim = dMax(hw->lineBox.minYData, minYLim - yOffset);
    }
    /* Tell widget to redraw, and what parts, if limits have changed */
    if (newMaxXLim != maxXLim || newMinXLim != minXLim)
    	redrawArea |= REDRAW_H_AXIS | REDRAW_CONTENTS;
    if (newMaxYLim != maxYLim || newMinYLim != minYLim)
    	redrawArea |= REDRAW_V_AXIS | REDRAW_CONTENTS;
    
    /* Set the new limits */
    hw->lineBox.maxXLim = newMaxXLim;
    hw->lineBox.minXLim = newMinXLim;
    hw->lineBox.maxYLim = newMaxYLim;
    hw->lineBox.minYLim = newMinYLim;
    
    /* redraw if the widget is realized */
    if (XtIsRealized(w))
    	redisplayContents(hw, X_SCREEN, redrawArea);
}
void LineBoxZoomIn(Widget w)
{
    LineBoxWidget hw = (LineBoxWidget)w;
    int logScaling = hw->lineBox.logScaling;
    double xOffset, yOffset;
    double minXLim, maxXLim, minYLim, maxYLim;

    /* if log scaling was requested, express limits in log coordinates */
    minXLim = hw->lineBox.minXLim;
    maxXLim = hw->lineBox.maxXLim;
    minYLim = logScaling ? log10(hw->lineBox.minYLim) : hw->lineBox.minYLim;
    maxYLim = logScaling ? log10(hw->lineBox.maxYLim) : hw->lineBox.maxYLim;
    
    /* Calculate offsets for limits of displayed data to zoom by ZOOM_FACTOR */
    xOffset = (maxXLim - minXLim) * ZOOM_FACTOR / 2;
    yOffset = (maxYLim - minYLim) * ZOOM_FACTOR / 2;

    /* Narrow the plotting limits by the offsets calculated above */
    maxXLim -= xOffset;
    minXLim += xOffset;
    maxYLim -= yOffset;
    minYLim += yOffset;
    
    /* Set the new limits */
    hw->lineBox.maxXLim = maxXLim;
    hw->lineBox.minXLim = minXLim;
    hw->lineBox.maxYLim = logScaling ? pow(10.,maxYLim) : maxYLim;
    hw->lineBox.minYLim = logScaling ? pow(10.,minYLim) : minYLim;
   
    /* redraw if the widget is realized */
    if (XtIsRealized(w))
    	redisplayContents(hw, X_SCREEN,
    		REDRAW_V_AXIS | REDRAW_H_AXIS | REDRAW_CONTENTS);
}
void LineBoxResetZoom(Widget w)
{
    LineBoxWidget hw = (LineBoxWidget)w;

    calcYDataRange(hw, &hw->lineBox.minYData, &hw->lineBox.maxYData);
    hw->lineBox.minXLim = hw->lineBox.minXData;
    if (hw->lineBox.logScaling)
    	hw->lineBox.minYLim = LOG_MIN_LIMIT;
    else
    	hw->lineBox.minYLim = hw->lineBox.minYData;
    hw->lineBox.maxXLim = hw->lineBox.maxXData;
    hw->lineBox.maxYLim = hw->lineBox.maxYData;

    if (XtIsRealized(w))
	redisplayContents(hw, X_SCREEN,
  		REDRAW_V_AXIS | REDRAW_H_AXIS | REDRAW_CONTENTS);
}

/*
** LineBoxPrintContents
**
** Prints the contents LineBox widget to a PostScript file.
**
** Parameters
**
**	w		A lineBox widget
**	psFileName	Name for the PostScript file that will be created
**	
*/
void LineBoxPrintContents(Widget w, char *psFileName)
{
    FILE * ps;
    LineBoxWidget hw = (LineBoxWidget)w;

    ps = OpenPS(psFileName, hw->core.width, hw->core.height);
    if (ps != NULL) {
	redisplayContents(hw, PS_PRINTER, REDRAW_ALL);
	EndPS();
    }    
}

/*
** LineBoxWritePS
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
**	w	An h1d widget
**	fp	File pointer for an open file in which to
**		write the drawing commands
*/
void LineBoxWritePS(Widget w, FILE *fp)
{
    FILE *oldFP;
    
    oldFP = PSGetFile();
    PSSetFile(fp);
    redisplayContents((LineBoxWidget)w, PS_PRINTER, REDRAW_ALL);
    PSSetFile(oldFP);   
}

/*
** Redisplays the contents part of the widget, without the motif shadows and
** highlights.
*/
static void redisplayContents(LineBoxWidget w, int outDevice, int redrawArea)
{
    Display *display = XtDisplay(w);
    GC gc = w->lineBox.gc;
    XFontStruct *fs = getFontStruct(w->lineBox.font);
    Drawable drawBuf;
    XSegment segs[2];
 
    /* Save some energy if the widget isn't visible or no drawing requested */
    if ((outDevice==X_SCREEN && !w->core.visible) || redrawArea == REDRAW_NONE)
        return;

    /* Set destination for drawing commands, offscreen pixmap or window */
    if (w->lineBox.doubleBuffer)
    	drawBuf = w->lineBox.drawBuffer;
    else
    	drawBuf = XtWindow(w);

    /* Clear the drawing buffer or window only in the areas that have
       changed.  The other parts are still redrawn, but the net effect
       is that the unchanged areas do not flicker */
    if (outDevice == X_SCREEN) {
	XSetForeground(display, gc, w->core.background_pixel);
	if (redrawArea == REDRAW_ALL) {
	    XFillRectangle(display, drawBuf, gc, w->lineBox.xMin, w->lineBox.yMin,
    		     w->lineBox.xMax - w->lineBox.xMin, w->lineBox.yMax - w->lineBox.yMin);
	} else {
    	    if (redrawArea & REDRAW_V_AXIS)
		XFillRectangle(display, drawBuf, gc, w->lineBox.axisLeft,
	   		w->lineBox.axisTop, w->lineBox.xOrigin - w->lineBox.axisLeft,
    			w->lineBox.axisBottom - w->lineBox.axisTop);
    	    if (redrawArea & REDRAW_H_AXIS)
    		XFillRectangle(display, drawBuf, gc, w->lineBox.axisLeft,
    	    		w->lineBox.yOrigin + 1, w->lineBox.axisRight-w->lineBox.axisLeft,
    	    		w->lineBox.axisBottom - w->lineBox.yOrigin + 1);
	    if (redrawArea & REDRAW_CONTENTS)
    		XFillRectangle(display, drawBuf, gc, w->lineBox.xOrigin + 1,
    	   		w->lineBox.yEnd, w->lineBox.xEnd - w->lineBox.xOrigin,
    	   		w->lineBox.yOrigin - w->lineBox.yEnd);
	}  
    }
    
    /* Draw the axes */
    XSetForeground(display, gc, w->primitive.foreground);
    if (w->lineBox.bins == NULL) {
        /* empty of data, just draw axis lines */
    	segs[0].x1 = segs[0].x2 = segs[1].x1 = w->lineBox.xOrigin;
    	segs[0].y1 = segs[1].y1 = segs[1].y2 = w->lineBox.yOrigin;
    	segs[1].x2 = w->lineBox.xEnd; segs[0].y2 = w->lineBox.yEnd;
	if (outDevice == X_SCREEN)
    	    XDrawSegments(display, drawBuf, gc, segs, 2);
	else /* PS_PRINTER */
    	    PSDrawSegments(display, drawBuf, gc, segs, 2);
    } else {
	DrawHorizontalAxis(display, drawBuf, gc, fs, outDevice, w->lineBox.yOrigin,
	    w->lineBox.xOrigin, w->lineBox.xEnd, w->lineBox.minXData, w->lineBox.maxXData,
	    w->lineBox.minXLim ,w->lineBox.maxXLim, False,
	    (w->lineBox.binEdgeLabeling && w->lineBox.edges==NULL) ? w->lineBox.nBins : 0);
	DrawVerticalAxis(display, drawBuf, gc, fs, outDevice, w->lineBox.xOrigin,
	    w->lineBox.xMin, w->lineBox.yOrigin, w->lineBox.yEnd,w->lineBox.minYData,
    	    w->lineBox.maxYData, w->lineBox.minYLim, w->lineBox.maxYLim, w->lineBox.logScaling);
    }
    
    /* Draw the axis labels */
    if (w->lineBox.xAxisLabel != NULL)
    	if (outDevice == X_SCREEN)
    	    XmStringDraw(display, drawBuf, w->lineBox.font, w->lineBox.xAxisLabel,
		gc, w->lineBox.xOrigin, w->lineBox.axisBottom + X_LABEL_MARGIN,
		w->lineBox.xEnd - w->lineBox.xOrigin, XmALIGNMENT_CENTER,
	     	XmSTRING_DIRECTION_L_TO_R, NULL);
    	else
    	    PSDrawXmString(display, drawBuf, w->lineBox.font, w->lineBox.xAxisLabel,
		gc, w->lineBox.xOrigin, w->lineBox.axisBottom + X_LABEL_MARGIN,
		w->lineBox.xEnd - w->lineBox.xOrigin, XmALIGNMENT_CENTER);
    if (w->lineBox.yAxisLabel != NULL)
    	if (outDevice == X_SCREEN)
    	    XmStringDraw(display, drawBuf, w->lineBox.font, w->lineBox.yAxisLabel, gc,
    		w->lineBox.xOrigin + Y_LABEL_MARGIN, w->lineBox.yMin + TOP_MARGIN,
		w->lineBox.xEnd - w->lineBox.xOrigin, XmALIGNMENT_BEGINNING,
	     	XmSTRING_DIRECTION_L_TO_R, NULL);
    	else
    	    PSDrawXmString(display, drawBuf, w->lineBox.font, w->lineBox.yAxisLabel,
    		gc, w->lineBox.xOrigin + Y_LABEL_MARGIN, w->lineBox.yMin + TOP_MARGIN,
		w->lineBox.xEnd - w->lineBox.xOrigin, XmALIGNMENT_BEGINNING);
    /* Draw the contents of the plot */
    drawLineBox(w, drawBuf, outDevice);
    drawPoints(w, drawBuf, outDevice);
    
    
    /* For double buffering, now copy offscreen pixmap to screen */
    if (w->lineBox.doubleBuffer && outDevice == X_SCREEN)
    	XCopyArea(display, drawBuf, XtWindow(w), gc, 0, 0,
    		  w->core.width, w->core.height, 0, 0);
    
    /* Call the redisplay callback so an application which draws on the lineBox
       widget can refresh it's graphics */
    if (XtIsRealized((Widget)w) && outDevice == X_SCREEN)
    	XtCallCallbacks((Widget)w, XmNredisplayCallback, NULL);
}

/*	TODO list:
 *	Does this technique have issues in the broader usage of X? Across servers, etc?
*/

void set_color (Display *display, GC gc, char *color)
{
	static struct {
		Pixel pix;
		char *name;
	} color_set[] = {
		{-1, "red"},
		{-1, "black"},
		{-1, "white"},
		{-1, NULL}
	};
	static int first = 1;

	int i;
	XColor col;
	Colormap map;

	if (first) {
		int result;
		first = 0;
		map = DefaultColormap(display,DefaultScreen(display));
		for (i=0; color_set[i].name != NULL; i++) {
			result = XParseColor (display, map, color_set[i].name, &col); 
			result = XAllocColor (display, map, &col);
			color_set[i].pix = col.pixel;
		}
	}

	for (i=0; color_set[i].name != NULL; i++) {
		if (!strcmp (color, color_set[i].name)) {
			if (color_set[i].pix != -1) {
				XSetForeground (display, gc, color_set[i].pix);
			}
			break;
		}
	}
}

static void drawPoints(LineBoxWidget w, Drawable drawBuf, int outDevice)
{
	Display *display = XtDisplay(w);
	GC gc = w->lineBox.contentsGC;
	int logScaling = w->lineBox.logScaling;
	int xMin = w->lineBox.xOrigin, yMin = w->lineBox.yEnd;
	int xMax = w->lineBox.xEnd, yMax = w->lineBox.yOrigin;
	double minXData = w->lineBox.minXData, maxXData = w->lineBox.maxXData;
	double minXLim = w->lineBox.minXLim, maxXLim = w->lineBox.maxXLim;
	double minYData, minYLim, maxYLim, xScale, yScale, minXPix, maxYPix;
	XSegment *xSeg, *xSegs; int nSegs = 0; 
	double delX, x1, x2, y1, y2;
	float *bin;
	float *uppErr,*lowErr,maxUppErr = 0.0;	/* Error bar data */
	int hx1, hy1, hx2, hy2, sep, lastY, i;	/* Int type to prevent ovf */

	float points[][2] = {
		{0,0},
		{.2,.4},
		{.4,.2},
		{.6,.7},
		{40,40}
	};

	set_color (display, gc, "red");

	/* don't bother if there's no data */
	if (w->lineBox.bins == NULL)
		return;

	/* if log scaling was requested, express limits in log coordinates */
	if (logScaling) {
		minYData = log10(w->lineBox.minYData);
		minYLim = log10(w->lineBox.minYLim); maxYLim = log10(w->lineBox.maxYLim + maxUppErr);
	} else {
		minYData = w->lineBox.minYData;
		minYLim = w->lineBox.minYLim; maxYLim = w->lineBox.maxYLim + maxUppErr;
	}
	xScale = (w->lineBox.xEnd - w->lineBox.xOrigin) / (maxXLim - minXLim);
	yScale = (w->lineBox.yOrigin - w->lineBox.yEnd) / (maxYLim - minYLim);
	minXPix = w->lineBox.xOrigin - (minXLim - minXData) * xScale;
	maxYPix = w->lineBox.yOrigin + (minYLim - minYData) * yScale;

	nSegs = 0;
	xSegs = (XSegment *)XtMalloc(sizeof(XSegment)*4);
	xSeg = xSegs;
	/* draw the segments that join the 'points' array */
	for (i=0; i<4; i++) {
		/*
		xSeg->x1 = minXPix + (points[i][0] - minXData) * xScale;
		xSeg->y1 = minYPix + (points[i][1] - minYData) * yScale;
		xSeg->x2 = minXPix + (points[i+1][0] - minXData) * xScale;
		xSeg->y2 = minYPix + (points[i+1][1] - minYData) * yScale;
		*/

		xSeg->x1 = (float)xMin + (float)(xMax-xMin) * points[i][0];
		xSeg->y1 = (float)xMin + (float)(xMax-xMin) * points[i][1];
		xSeg->x2 = (float)xMin + (float)(xMax-xMin) * points[i+1][0];
		xSeg->y2 = (float)xMin + (float)(xMax-xMin) * points[i+1][1];
		xSeg++;
		nSegs++;
	}
	if (nSegs != 0) {
		if (outDevice == X_SCREEN)
			XDrawSegments(display, drawBuf, gc, xSegs, nSegs);
		else
			PSDrawSegments(display, drawBuf, gc, xSegs, nSegs);
	}
	XtFree((char *)xSegs);
	set_color (display, gc, "black");
}
    
/*
** Draw the histogram itself using the list of points stored in the
** widget.  
*/
static void drawLineBox(LineBoxWidget w, Drawable drawBuf, int outDevice)
{
    Display *display = XtDisplay(w);
    GC gc = w->lineBox.contentsGC;
    int logScaling = w->lineBox.logScaling;
    int xMin = w->lineBox.xOrigin, yMin = w->lineBox.yEnd;
    int xMax = w->lineBox.xEnd, yMax = w->lineBox.yOrigin;
    double minXData = w->lineBox.minXData, maxXData = w->lineBox.maxXData;
    double minXLim = w->lineBox.minXLim, maxXLim = w->lineBox.maxXLim;
    double minYData, minYLim, maxYLim, xScale, yScale, minXPix, maxYPix;
    XSegment *xSeg, *xSegs; int nSegs = 0; 
    double delX, x1, x2, y1, y2;
    float *bin;
    float *uppErr,*lowErr,maxUppErr = 0.0;	/* Error bar data */
    int hx1, hy1, hx2, hy2, sep, lastY, i;	/* Int type to prevent ovf */
   
    /* don't bother if there's no data */
    if (w->lineBox.bins == NULL)
    	return;

    /* if log scaling was requested, express limits in log coordinates */
    if (logScaling) {
    	minYData = log10(w->lineBox.minYData);
    	minYLim = log10(w->lineBox.minYLim); maxYLim = log10(w->lineBox.maxYLim + maxUppErr);
    } else {
    	minYData = w->lineBox.minYData;
    	minYLim = w->lineBox.minYLim; maxYLim = w->lineBox.maxYLim + maxUppErr;
    }
    xScale = (w->lineBox.xEnd - w->lineBox.xOrigin) / (maxXLim - minXLim);
    yScale = (w->lineBox.yOrigin - w->lineBox.yEnd) / (maxYLim - minYLim);
    minXPix = w->lineBox.xOrigin - (minXLim - minXData) * xScale;
    maxYPix = w->lineBox.yOrigin + (minYLim - minYData) * yScale;

    /* allocate memory for an array of XSegment structures for drawing lines 
       Enough must be allocated for 3 segs and triple lines for overflow.
       Additionally, error bars can account for 7 segments per bin */
    xSegs = (XSegment *)XtMalloc(sizeof(XSegment)*((w->lineBox.nBins)*12+5));
    if (xSegs == NULL) {
	fprintf(stderr, "LineBox Can't Allocate Memory for Drawing\n");
	return;
     }    

    /* loop through all of the data converting the data coordinates to
       X coordinates and drawing left and top segments of each bin */
    lastY = yMax;
    xSeg = xSegs;
    delX = (maxXData - minXData) / (double) (w->lineBox.nBins);

    for (i=0, bin=w->lineBox.bins; i<w->lineBox.nBins; i++, bin++) {

	/* calculate left and right edges of bin in data coordinates, or
	   use the edges array, if supplied, to draw an adaptive histogram */
	if (w->lineBox.edges) {
	    x1 = w->lineBox.edges[i];
	    x2 = w->lineBox.edges[i+1];
	} else {
	    x1 = minXData + i*delX;
	    x2 = x1 + delX;
	}
	
	/* skip any bins that are completely outside of plot area */
	if ((x1 < minXLim && x2 < minXLim) || (x1 > maxXLim && x2 > maxXLim))
            continue;

	/* calculate left and right edges of bin in window coordinates */
	hx1 = (int)(minXPix + (x1 - minXData) * xScale);
	hx2 = (int)(minXPix + (x2 - minXData) * xScale);

	/* if barSepPercent is nonzero, adjust the bins by separation factor
	   of % of their width.  If this puts bin off display, skip it,
	   otherwise rounding below will put it back on inappropriately */
	if (w->lineBox.barSepPercent != 0) {
	    sep = ((hx2 - hx1) * w->lineBox.barSepPercent/100) / 2;
	    hx1 += sep;
	    hx2 -= sep;
	    if (hx1 > xMax)
	    	continue;
	}
	
	/* calculate the top and bottom of the sides of the bin (if bins
	   are not separated, the top and bottom of the left leg of the bin) */
	y2 = logScaling ? log10(dMax(*bin, FLT_MIN)) : *bin;
	hy1 = (w->lineBox.barSepPercent == 0) ? lastY : yMax;
	hy2 = (int)(maxYPix-(dMax(minYLim,dMin(y2,maxYLim))-minYData) * yScale);
	
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
	if (w->lineBox.barSepPercent != 0) {
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
    if (maxXData == maxXLim && w->lineBox.barSepPercent == 0) {
        xSeg->x1 = xSeg->x2 = hx2; xSeg->y1 = hy2; xSeg->y2 = yMax;
        xSeg++; nSegs++;
    }
    
    /* Finally, apply error bars if applicable */
    if (w->lineBox.uppErr != NULL && w->lineBox.lowErr != NULL) {
	for (i=0,bin=w->lineBox.bins,uppErr=w->lineBox.uppErr,lowErr=w->lineBox.lowErr;
    		i<w->lineBox.nBins; i++,bin++,uppErr++,lowErr++) {

            /* Compute vertical part of bar and place in segment */
	    x1 = minXData + i*delX + (delX/2.0);	/* Middle of bin */
	    hx1 = hx2 = (int)(minXPix + (x1 - minXData) * xScale);
            y1 = logScaling ? log10(dMax(*bin - *lowErr, FLT_MIN)) :
            	    *bin - *lowErr;
	    hy1 = (int)(maxYPix-(dMax(minYLim,dMin(y1,maxYLim))-minYData) *
	    	    yScale);
            y2 = logScaling ? log10(dMax(*bin + *uppErr, FLT_MIN)) :
            	    *bin + *uppErr;
	    hy2 = (int)(maxYPix-(dMax(minYLim,dMin(y2,maxYLim))-minYData) *
	    	    yScale);
	    
	    /* If the error bar is not within the drawing area, skip it */
	    if (x1 + delX < minXLim || x1 - delX > maxXLim ||
	    	    y1 > maxYLim ||  y2 < minYLim)
	    	continue;
	    	    
            /* draw the vertical part of the bar */
            xSeg->x1 = hx1; xSeg->x2 = hx2; xSeg->y1 = hy1; xSeg->y2 = hy2;
            xSeg++; nSegs++;

            /* Compute and draw the horizontal part.  This is fixed to
               procede from the vertical part (center of the bin) outward
               at a length of 1/4 the width of the bin */
            x2 = x1 + (delX - delX*w->lineBox.barSepPercent/100.) / 4.0;
            x1 = x1 - (delX - delX*w->lineBox.barSepPercent/100.) / 4.0;
	    hx1 = (int)(minXPix + (x1 - minXData) * xScale);
	    hx2 = (int)(minXPix + (x2 - minXData) * xScale);
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
    }  

    /* draw the collected segments to the screen or post script file */
    if (nSegs != 0) {
    	if (outDevice == X_SCREEN)
    	    XDrawSegments(display, drawBuf, gc, xSegs, nSegs);
    	else
    	    PSDrawSegments(display, drawBuf, gc, xSegs, nSegs);
    }
    XtFree((char *)xSegs);
}

/*
** For smoother animation, it is possible to allocate an offscreen pixmap
** and draw to that rather than directly into the window.  Unfortunately,
** it is too slow on many machines, so we have to give the user the choice
** whether to use it or not.  This routine reallocates the offscreen
** pixmap buffer when the window is resized, and when the widget is created
*/
static void updateBufferAllocation(LineBoxWidget w)
{ 
    if (w->lineBox.drawBuffer)
    	XFreePixmap(XtDisplay(w), w->lineBox.drawBuffer);
    if (w->lineBox.doubleBuffer) {
    	w->lineBox.drawBuffer = XCreatePixmap(XtDisplay(w),
		DefaultRootWindow(XtDisplay(w)), w->core.width, w->core.height,
    	 	DefaultDepthOfScreen(XtScreen(w)));   
    } else {
    	w->lineBox.drawBuffer = 0;
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
** Adjusts the data and limit minimums to be greater than 0 for log scaling.
** To highlight zeros or negative numbers as underflows, it sets the data
** minimum to FLT_MIN and the lower plot limit to LOG_MIN_LIMIT (currently .5)
*/
static void adjustLimitsForLogScaling(LineBoxWidget w)
{
    if (!w->lineBox.logScaling)
    	return;
    
    /* set the new minimums */	
    w->lineBox.minYData = FLT_MIN;
    w->lineBox.minYLim = LOG_MIN_LIMIT;

    /* make sure new limits are still less than the max limits */
    if (w->lineBox.maxYLim <= LOG_MIN_LIMIT)
    	w->lineBox.maxYLim = w->lineBox.maxYData > LOG_MIN_LIMIT ?
    	    	w->lineBox.maxYData : LOG_MIN_LIMIT * 2;
    if (w->lineBox.maxYData <= LOG_MIN_LIMIT)
    	w->lineBox.maxYData = LOG_MIN_LIMIT * 2;
}

/*
** Scans the histogram data stored in the widget & returns the minimum
** and maximum values appropriate for that data.
*/
static void calcYDataRange(LineBoxWidget w, double *minimum, double *maximum)
{
    double min, max;
    float *bin, *upErr, *lowErr;
    int i;
    
    /* If there is no data, return existing data limits */
    if (w->lineBox.nBins == 0) {
   	*minimum = w->lineBox.minYData;
    	*maximum = w->lineBox.maxYData;
    	return;
    }
    
    /* Scan the data calculating min and max */
    min = FLT_MAX; max = -FLT_MAX;
    if (w->lineBox.uppErr == NULL || w->lineBox.lowErr == NULL) {
	for (i=0,bin=w->lineBox.bins; i<w->lineBox.nBins; i++, bin++) {
    	    if (*bin > max) max = *bin;
    	    if (*bin < min) min = *bin;
	}
    } else {
    	for (i=0,bin=w->lineBox.bins,upErr=w->lineBox.uppErr,lowErr=w->lineBox.lowErr;
    		i<w->lineBox.nBins; i++, bin++, upErr++, lowErr++) {
    	    if (*bin + *upErr > max) max = *bin + *upErr;
    	    if (*bin - *lowErr < min) min = *bin - *lowErr;
	}
    }
    
    /* For linear scaling, start histogram from zero unless minimum is
       negative.  For log scaling, set the data minimum greater than 0.
       but less than LOG_MIN_LIMIT to highlight zeros and negative numbers */
    if (w->lineBox.logScaling)
    	min = FLT_MIN;
    else
   	min = dMin(min, 0);

    /* if min is the same as max, bump up max */
    if (min == max)
    	max += 1.;
    
    /* return the values */
    *minimum = min; *maximum = max;
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

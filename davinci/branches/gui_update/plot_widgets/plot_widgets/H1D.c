/*******************************************************************************
*                                                                              *
* H1D.c -- 1-D Histogram Widget                                                *
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
#include "H1DP.h"

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
static void initialize(H1DWidget request, H1DWidget new);
static void redisplay(Widget w, XEvent *event, Region region);
static void redisplayContents(H1DWidget w, int outDevice,
			      int redrawArea);
static void drawH1D(H1DWidget w, Drawable drawBuf, int outDevice);
static Boolean setValues(H1DWidget current, H1DWidget request,H1DWidget new);
static void updateBufferAllocation(H1DWidget w);
static XFontStruct *getFontStruct(XmFontList font);
static void adjustLimitsForLogScaling(H1DWidget w);
static void calcYDataRange(H1DWidget w, double *minimum, double *maximum);
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
      XtOffset(H1DWidget, h1D.doubleBuffer), XmRString, "False"},
    {XmNlogScaling, XmCLogScaling, XmRBoolean, sizeof(Boolean),
      XtOffset(H1DWidget, h1D.logScaling), XmRString, "False"},
    {XmNbinEdgeLabeling, XmCBinEdgeLabeling, XmRBoolean, sizeof(Boolean),
      XtOffset(H1DWidget, h1D.binEdgeLabeling), XmRString, "False"},
    {XmNfontList, XmCFontList, XmRFontList, sizeof(XmFontList),
      XtOffset(H1DWidget, h1D.font), XmRImmediate, NULL},
    {XmNxAxisLabel, XmCXAxisLabel, XmRXmString, sizeof (XmString), 
      XtOffset(H1DWidget, h1D.xAxisLabel), XmRString, NULL},
    {XmNyAxisLabel, XmCYAxisLabel, XmRXmString, sizeof (XmString), 
      XtOffset(H1DWidget, h1D.yAxisLabel), XmRString, NULL},
    {XmNbarSeparation, XmCBarSeparation, XmRInt, sizeof(int), 
      XtOffset(H1DWidget, h1D.barSepPercent), XmRString, "0"},
    {XmNresizeCallback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (H1DWidget, h1D.resize), XtRCallback, NULL},
    {XmNbtn2Callback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (H1DWidget, h1D.btn2), XtRCallback, NULL},
    {XmNbtn3Callback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (H1DWidget, h1D.btn3), XtRCallback, NULL},
    {XmNredisplayCallback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (H1DWidget, h1D.redisplay), XtRCallback, NULL},
};

H1DClassRec  h1DClassRec = {
     /* CoreClassPart */
  {
    (WidgetClass)&xmPrimitiveClassRec,/* superclass          */
    "H1D",                            /* class_name            */
    sizeof(H1DRec),                   /* widget_size           */
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
  /* H1D class part */
  {
    0,                              	/* ignored	                */
  }
};

WidgetClass h1DWidgetClass = (WidgetClass) &h1DClassRec;

/*
** Widget initialize method
*/
static void initialize(H1DWidget request, H1DWidget new)
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
    if (new->h1D.font == NULL)
    	new->h1D.font =
    	    XmFontListCopy(_XmGetDefaultFontList(
    	    			(Widget) new, XmLABEL_FONTLIST));
    else
        new->h1D.font = XmFontListCopy(new->h1D.font);

    /* Make local copies of the XmStrings */
    if (new->h1D.xAxisLabel != NULL)
    	new->h1D.xAxisLabel = XmStringCopy(new->h1D.xAxisLabel);
    if (new->h1D.yAxisLabel != NULL)
    	new->h1D.yAxisLabel = XmStringCopy(new->h1D.yAxisLabel);
     
    /* Create graphics contexts for drawing in the widget */
    values.font = getFontStruct(new->h1D.font)->fid;
    values.foreground = new->primitive.foreground;
    values.background = new->core.background_pixel;
    new->h1D.gc = XCreateGC(display, XDefaultRootWindow(display),
    	    GCForeground|GCBackground|GCFont, &values);
    new->h1D.contentsGC = XCreateGC(display, XDefaultRootWindow(display),
    	    GCForeground|GCBackground, &values);
    
    /* Initialize various fields */
    ResetAxisDragging(&new->h1D.dragState);
    new->h1D.bins = NULL;
    new->h1D.edges = NULL;
    new->h1D.nBins = 0;
    new->h1D.uppErr = NULL;
    new->h1D.lowErr = NULL;
    
    /* Default plotting boundaries  */
    H1DSetContents((Widget)new, 0.0, 0.0, 0, NULL, NULL, NULL, H1D_RESCALE);

    /* Set size dependent items */
    new->h1D.drawBuffer = 0;
    resize((Widget)new);
}

/*
** Widget destroy method
*/
static void destroy(Widget w)
{
    H1DWidget hw = (H1DWidget)w;
    
    XFreeGC(XtDisplay(w), hw->h1D.gc);
    XFreeGC(XtDisplay(w), hw->h1D.contentsGC);
    if (hw->h1D.font != NULL)
    	XmFontListFree(hw->h1D.font);
    if (hw->h1D.bins != NULL)
     	XtFree((char *)hw->h1D.bins);
    if (hw->h1D.edges != NULL)
     	XtFree((char *)hw->h1D.edges);
    XtRemoveAllCallbacks (w, XmNresizeCallback);
    XtRemoveAllCallbacks (w, XmNbtn2Callback);
    XtRemoveAllCallbacks (w, XmNbtn3Callback);
    XtRemoveAllCallbacks (w, XmNredisplayCallback);
    if (hw->h1D.xAxisLabel != NULL)
    	XmStringFree(hw->h1D.xAxisLabel);
    if (hw->h1D.yAxisLabel != NULL)
    	XmStringFree(hw->h1D.yAxisLabel);
}

/*
** Widget resize method
*/
static void resize(Widget w)
{
    H1DWidget hw = (H1DWidget)w;
    
    XFontStruct *fs = getFontStruct(hw->h1D.font);
    int borderWidth =
    	hw->primitive.shadow_thickness + hw->primitive.highlight_thickness;
    XRectangle clipRect;

    /* resize the drawing buffer, an offscreen pixmap for smoother animation */
    updateBufferAllocation(hw); 

    /* calculate the area of the widget where contents can be drawn */
    hw->h1D.xMin = borderWidth;
    hw->h1D.yMin = borderWidth;
    hw->h1D.xMax = w->core.width - borderWidth;
    hw->h1D.yMax = w->core.height - borderWidth;

    /* calculate positions for the axes and contents depending on whether
       axis labels are specified, and the measurements of the current font */
    if (hw->h1D.yAxisLabel != NULL)
    	hw->h1D.yEnd = hw->h1D.yMin + fs->ascent + fs->descent + TOP_MARGIN;
    else
    	hw->h1D.yEnd = VAxisEndClearance(fs) + fs->ascent/2 + TOP_MARGIN;
    hw->h1D.axisTop = hw->h1D.yEnd - VAxisEndClearance(fs);
    if (hw->h1D.xAxisLabel != NULL)
    	hw->h1D.axisBottom = hw->h1D.yMax - BOTTOM_MARGIN - fs->ascent -
    				fs->descent - X_LABEL_MARGIN;
    else
    	hw->h1D.axisBottom = hw->h1D.yMax - fs->ascent/2 - BOTTOM_MARGIN;
    hw->h1D.yOrigin = hw->h1D.axisBottom - HAxisHeight(fs);
    hw->h1D.axisLeft = hw->h1D.xMin + LEFT_MARGIN;
    hw->h1D.xOrigin = hw->h1D.axisLeft + VAxisWidth(fs);
    if (hw->h1D.xOrigin > (hw->h1D.xMax - hw->h1D.xMin) * MAX_AXIS_PERCENT/100)
    	hw->h1D.xOrigin = (hw->h1D.xMax - hw->h1D.xMin) * MAX_AXIS_PERCENT/100;
    hw->h1D.axisRight = hw->h1D.xMax - RIGHT_MARGIN;
    hw->h1D.xEnd = hw->h1D.axisRight - HAxisEndClearance(fs);
    if (hw->h1D.xMax - hw->h1D.xEnd > hw->h1D.xOrigin - hw->h1D.xMin)
    	hw->h1D.xEnd = hw->h1D.xMax - (hw->h1D.xOrigin - hw->h1D.xMin);
    
    /* set plot contents gc to clip drawing at the edges */
    clipRect.x = hw->h1D.xOrigin;
    clipRect.y = hw->h1D.yEnd;
    clipRect.width = hw->h1D.xEnd - hw->h1D.xOrigin + 1;
    clipRect.height = hw->h1D.yOrigin - hw->h1D.yEnd;
    XSetClipRectangles(XtDisplay(w), hw->h1D.contentsGC, 0, 0, &clipRect,
    		       1, Unsorted);

    /* set drawing gc to clip drawing before motif shadow and highlight */
    clipRect.x = borderWidth;
    clipRect.y = borderWidth;
    clipRect.width = w->core.width - 2 * borderWidth;
    clipRect.height = w->core.height - 2 * borderWidth;
    XSetClipRectangles(XtDisplay(w), hw->h1D.gc, 0, 0, &clipRect, 1, Unsorted);
    
    /* call the resize callback */
    if (XtIsRealized(w))
    	XtCallCallbacks(w, XmNresizeCallback, NULL);
}

/*
** Widget redisplay method
*/
static void redisplay(Widget w, XEvent *event, Region region)
{
    H1DWidget hw = (H1DWidget)w;
    
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
    
    /* Now draw the contents of the h1D widget */
    redisplayContents((H1DWidget)w, X_SCREEN, REDRAW_ALL);
}

/*
** Widget setValues method
*/
static Boolean setValues(H1DWidget current, H1DWidget request, H1DWidget new)
{
    Boolean redraw = False, doResize = False;
    Display *display = XtDisplay(new);

    /* If the foreground or background color has changed, change the GCs */
    if (new->core.background_pixel !=current->core.background_pixel) {
    	XSetForeground(display, new->h1D.gc, new->primitive.foreground);
    	XSetForeground(display, new->h1D.contentsGC,new->primitive.foreground);
    	redraw = TRUE;
    }
    if (new->primitive.foreground != current->primitive.foreground) {
    	XSetBackground(display, new->h1D.gc, new->core.background_pixel);
    	XSetBackground(display,new->h1D.contentsGC,new->core.background_pixel);
    	redraw = TRUE;
    }

    /* if double buffering changes, allocate or deallocate offscreen pixmap */
    if (new->h1D.doubleBuffer != current->h1D.doubleBuffer) {
    	updateBufferAllocation(new);
    	redraw = TRUE;
    }

    /* if log scaling changes, adjust or restore data limits,
       reset dragging, and redraw */
    if (new->h1D.logScaling != current->h1D.logScaling) {
    	if (new->h1D.logScaling)
    	    adjustLimitsForLogScaling(new);
    	else
	    calcYDataRange(new, &new->h1D.minYData, &new->h1D.maxYData);
    	ResetAxisDragging(&new->h1D.dragState);
    	redraw = TRUE;
    }

    /* if labels are changed, free the old ones and copy the new ones */
    if (new->h1D.xAxisLabel != current->h1D.xAxisLabel) {
    	if (current->h1D.xAxisLabel != NULL)
    	    XmStringFree(current->h1D.xAxisLabel);
    	new->h1D.xAxisLabel = XmStringCopy(new->h1D.xAxisLabel);
    	doResize = TRUE;
    }
    if (new->h1D.yAxisLabel != current->h1D.yAxisLabel) {
    	if (current->h1D.yAxisLabel != NULL)
    	    XmStringFree(current->h1D.yAxisLabel);
    	new->h1D.yAxisLabel = XmStringCopy(new->h1D.yAxisLabel);
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
    if (new->h1D.barSepPercent != current->h1D.barSepPercent)
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
    H1DWidget hw = (H1DWidget)w;
    int chgdArea, redrawArea = REDRAW_NONE;

    if (event->type == ButtonPress)
    	XmProcessTraversal((Widget)w, XmTRAVERSE_CURRENT);
    
    chgdArea = DragAxes(event, hw->h1D.xOrigin, hw->h1D.xEnd, hw->h1D.yOrigin,
    	hw->h1D.yEnd, hw->h1D.axisLeft, hw->h1D.axisTop, hw->h1D.axisBottom,
    	hw->h1D.axisRight, hw->h1D.minXData, hw->h1D.maxXData, hw->h1D.minYData,
    	hw->h1D.maxYData, False, hw->h1D.logScaling, &hw->h1D.minXLim,
    	&hw->h1D.maxXLim, &hw->h1D.minYLim, &hw->h1D.maxYLim,
    	&hw->h1D.dragState, &hw->h1D.xDragStart, &hw->h1D.yDragStart);
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
    ResetAxisDragging(&((H1DWidget)w)->h1D.dragState);
    drawH1D((H1DWidget)w, XtWindow(w), X_SCREEN); 
}

static void btn2AP(Widget w, XEvent *event, char *args, int n_args)
{
    H1DCallbackStruct cbStruct;

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
    H1DCallbackStruct cbStruct;

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
** H1DSetContents
**
** Specify the contents of the h1D widget in the form of an array of bins
** and optional arrays of error data
**
** Parameters
**
** 	w		A h1D widget
**	xMin, xMax	Low edge of first bin, right edge of last bin
**	nBins		The number of bins in the histogram
**	bins		Bin contents, an array floats nBins long
**	uppErr, lowErr	Upper and lower error bar data stored as offsets
**			from top of bin, arrays of floats nBins long 
**	rescale		One of: H1D_NO_RESCALE, H1D_RESCALE, or
**			H1D_GROW_ONLY.  Tells the widget how to change
**			its display to incorporate the new contents.
*/
void H1DSetContents(Widget w, float xMin, float xMax, int nBins, float *bins,
		    float *uppErr, float *lowErr, int rescale)
{
    H1DWidget hw = (H1DWidget)w;
    int oldNBins = hw->h1D.nBins;
    int redrawArea, i;
    double minX, minY, maxX, maxY;
    
    /* Free the previous data */
    if (hw->h1D.bins != NULL) {
    	XtFree((char *)hw->h1D.bins);
        hw->h1D.nBins = 0;
    }
    if (hw->h1D.uppErr != NULL) {
    	XtFree((char *)hw->h1D.uppErr);
    	hw->h1D.uppErr = NULL;
    }
    if (hw->h1D.lowErr != NULL) {
    	XtFree((char *)hw->h1D.lowErr);
    	hw->h1D.lowErr = NULL;
    }
    
    /* Copy in the new data, if any */
    if (bins == NULL || nBins == 0) {
    	hw->h1D.bins = NULL;
    	bins = NULL;
    	nBins = 0;
    } else {
	hw->h1D.bins = (float *)XtMalloc(sizeof (float) * nBins);
	memcpy (hw->h1D.bins, bins, sizeof (float) * nBins);
    }
    hw->h1D.nBins = nBins;
    if (uppErr != NULL && lowErr != NULL) {
	hw->h1D.uppErr = (float *)XtMalloc(sizeof (float) * nBins);
	hw->h1D.lowErr = (float *)XtMalloc(sizeof (float) * nBins);
	for (i=0; i<nBins; i++) {
	    hw->h1D.uppErr[i] = fabs(uppErr[i]);
	    hw->h1D.lowErr[i] = fabs(lowErr[i]);
	}
    }
    
    /* Calculate range of data, or fill in defaults if there's no data */
    minX = xMin; maxX = xMax;
    if (hw->h1D.nBins == 0 && rescale == H1D_RESCALE) {
    	minY = hw->h1D.logScaling ? .5 : 0.;
    	maxY = 1.;
    } else
	calcYDataRange(hw, &minY, &maxY);

    /* If either range is 0, make up fake min and max values */
    if (maxX == minX) maxX += 1.;
    if (maxY == minY) maxY += 1.;
    
    /* recalculate scale, limits, and redraw area for the widget */
    redrawArea = oldNBins != nBins ? REDRAW_ALL : REDRAW_NONE;
    if (rescale == H1D_RESCALE) {
    	hw->h1D.maxXLim = maxX; hw->h1D.minXLim = minX;
    	hw->h1D.maxYLim = maxY; hw->h1D.minYLim = minY;
    	hw->h1D.maxYData = maxY; hw->h1D.minYData = minY;
    	adjustLimitsForLogScaling(hw);
    	redrawArea = REDRAW_CONTENTS | REDRAW_H_AXIS | REDRAW_V_AXIS;
    } else if (rescale == H1D_NO_RESCALE) {
	redrawArea |= REDRAW_CONTENTS;
	if (maxY > hw->h1D.maxYData) {
	    hw->h1D.maxYData = maxY;
	    redrawArea |= REDRAW_V_AXIS;
	}
	if (minY < hw->h1D.minYData) {
	    hw->h1D.minYData = minY;
	    redrawArea |= REDRAW_V_AXIS;
	}
    } else if (rescale == H1D_RESCALE_AT_MAX || rescale == H1D_GROW_ONLY) {
	redrawArea |= REDRAW_CONTENTS;
	if (rescale == H1D_GROW_ONLY)
	    maxY = dMax(hw->h1D.maxYData, maxY);
    	if (hw->h1D.maxXData != maxX || hw->h1D.minXData != minX)
    	    redrawArea |= REDRAW_H_AXIS;
    	if (hw->h1D.maxYData != maxY || hw->h1D.minYData != minY)
    	    redrawArea |= REDRAW_V_AXIS;
    	if (hw->h1D.maxXData == hw->h1D.maxXLim)
    	    hw->h1D.maxXLim = maxX;
    	if (hw->h1D.minXData == hw->h1D.minXLim)
    	    hw->h1D.minXLim = minX;
    	if (hw->h1D.maxYData == hw->h1D.maxYLim)
    	    hw->h1D.maxYLim = maxY;
    	if (hw->h1D.minYData == hw->h1D.minYLim)
    	    hw->h1D.minYLim = minY;
   	hw->h1D.maxYData = maxY; hw->h1D.minYData = minY;
    } else if (rescale == H1D_REBIN_MODE) {
	redrawArea |= REDRAW_ALL;
    	if (hw->h1D.maxXData == hw->h1D.maxXLim)
    	    hw->h1D.maxXLim = maxX;
    	if (hw->h1D.minXData == hw->h1D.minXLim)
    	    hw->h1D.minXLim = minX;
    	if (hw->h1D.maxYData == hw->h1D.maxYLim)
    	    hw->h1D.maxYLim = maxY;
    	else
    	    hw->h1D.maxYLim =
		dMin(maxY, hw->h1D.maxYLim * (double)oldNBins/(double)nBins);
    	if (hw->h1D.minYData == hw->h1D.minYLim)
    	    hw->h1D.minYLim = minY;
	else if (!(hw->h1D.logScaling && hw->h1D.minYLim == LOG_MIN_LIMIT))
	    hw->h1D.minYLim =
		dMax(minY, hw->h1D.minYLim * (double)oldNBins/(double)nBins);
   	hw->h1D.maxYData = maxY; hw->h1D.minYData = minY;
     }
    hw->h1D.maxXData = maxX; hw->h1D.minXData = minX;

    /* redraw the widget with the new data */
    if (XtIsRealized(w))
    	redisplayContents(hw, X_SCREEN, redrawArea);
}

/*
** H1DSetContentsAdaptive
**
** Specify the contents of the widget as an adaptive histogram in the form
** an array of bins and an array of bin edges.  After calling this routine,
** you should not use H1DSetContents and cannot return the widget to being
** a plain histogram.
**
** Parameters
**
** 	w		A H1D widget
**	nBins		The number of bins in the histogram
**	bins		Bin contents, an array floats nBins long
**	edges		Bin edges in data coordinates nBins + 1 long
**	rescale		One of: H1D_NO_RESCALE, H1D_RESCALE, or
**			H1D_GROW_ONLY.  Tells the widget how to change
**			its display to incorporate the new contents.
*/
void H1DSetContentsAdaptive(Widget w, int nBins, float *bins, float *edges,
	int rescale)
{
    H1DWidget hw = (H1DWidget)w;
    float xMin, xMax;

    /* Free the previous edges data and copy in new edges data */
    if (hw->h1D.edges != NULL)
    	XtFree((char *)hw->h1D.edges);
    if (edges == NULL) {
    	hw->h1D.edges = NULL;
    } else {
    	hw->h1D.edges = (float *)XtMalloc(sizeof(float) * (nBins+1));
    	memcpy(hw->h1D.edges, edges, sizeof (float) * (nBins+1));
    }
    
    /* Set the contents as if this was a regular histogram, using the first
       edge as a min value, and the last as a max value.  The drawing
       routines will use the edges array if it is supplied, to draw the
       histogram as adaptive. In H1D_NO_RESCALE mode, leave the X axis as is */
    if (rescale == H1D_NO_RESCALE) {
    	xMin = hw->h1D.minXData;
    	xMax = hw->h1D.maxXData;
    } else if (edges == NULL) {
    	xMin = 0;
    	xMax = 1;
    } else {
    	xMin = edges[0];
    	xMax = edges[nBins];
    }
    H1DSetContents(w, xMin, xMax, nBins, bins, NULL, NULL, rescale);
}

/*
** H1DSetVisibleRange, H1DGetVisibleRange
**
** Set (Get) the range of data that is visible.  minXLim, minYLim, maxXLim, and
** maxYLim specify the endpoints of the x and y axes.  H1DSetVisibleRange,
** unlike the widgets interactive rescaling routines, can zoom out past the
** actual minimum and maximum data points.
*/
void H1DSetVisibleRange(Widget w, double minXLim, double minYLim,
			double maxXLim, double maxYLim)
{
    H1DWidget hw = (H1DWidget)w;
    double minY, maxY;
    
    /* calculate the range necessary to fully display the data */
    calcYDataRange(hw, &minY, &maxY);

    /* allow user to zoom beyond the range of the data by artificially
       adjusting data limits */
    hw->h1D.maxYData = dMax(maxYLim, maxY);
    hw->h1D.minYData = dMin(minYLim, minY);

    /* Set the range */
    hw->h1D.minXLim = dMax(minXLim, hw->h1D.minXData);
    hw->h1D.maxXLim = dMin(maxXLim, hw->h1D.maxXData);
    hw->h1D.minYLim = minYLim;
    hw->h1D.maxYLim = maxYLim;
    
    /* if log scaling was requested and minYLim <= 0, adjust the new limits */
    if (minYLim <= 0)
    	adjustLimitsForLogScaling(hw);

    /* redraw if the widget is realized */
    if (XtIsRealized(w))
    	redisplayContents(hw, X_SCREEN,
    		REDRAW_V_AXIS | REDRAW_H_AXIS | REDRAW_CONTENTS);
}
void H1DGetVisibleRange(Widget w, double *minXLim, double *minYLim,
			double *maxXLim, double *maxYLim)
{
    *minXLim = ((H1DWidget)w)->h1D.minXLim;
    *maxXLim = ((H1DWidget)w)->h1D.maxXLim;
    *minYLim = ((H1DWidget)w)->h1D.minYLim;
    *maxYLim = ((H1DWidget)w)->h1D.maxYLim;
}

/*
** H1DZoomOut, H1DZoomIn, H1DResetZoom
**
** Zoom in and out by ZOOM_FACTOR.  Zoom in is centered on the current
** center of the plot.
*/
void H1DZoomOut(Widget w)
{
    H1DWidget hw = (H1DWidget)w;
    int logScaling = hw->h1D.logScaling;
    double xOffset, yOffset, newMaxXLim, newMinXLim, newMaxYLim, newMinYLim;
    double minXLim, maxXLim, minYLim, maxYLim;
    int redrawArea = REDRAW_NONE;
    
    /* if log scaling was requested, express limits in log coordinates */
    minXLim = hw->h1D.minXLim;
    maxXLim = hw->h1D.maxXLim;
    minYLim = logScaling ? log10(hw->h1D.minYLim) : hw->h1D.minYLim;
    maxYLim = logScaling ? log10(hw->h1D.maxYLim) : hw->h1D.maxYLim;

    /* Calculate a suitable offset to reverse a zoom in by ZOOM_FACTOR */
    xOffset = (maxXLim - minXLim) * (ZOOM_FACTOR/(1.-ZOOM_FACTOR)) / 2;
    yOffset = (maxYLim - minYLim) * (ZOOM_FACTOR/(1.-ZOOM_FACTOR)) / 2;

    /* widen the plotting limits by the offsets calculated above,
       stopping when the limits reach the limits of the data */
    newMaxXLim = dMin(hw->h1D.maxXData, maxXLim + xOffset);
    newMinXLim = dMax(hw->h1D.minXData, minXLim - xOffset);
    if (logScaling) {
	newMaxYLim = dMin(hw->h1D.maxYData, pow(10., maxYLim + yOffset));
	newMinYLim = dMax(.5, pow(10., minYLim - yOffset));
    } else {
    	newMaxYLim = dMin(hw->h1D.maxYData, maxYLim + yOffset);
	newMinYLim = dMax(hw->h1D.minYData, minYLim - yOffset);
    }
    /* Tell widget to redraw, and what parts, if limits have changed */
    if (newMaxXLim != maxXLim || newMinXLim != minXLim)
    	redrawArea |= REDRAW_H_AXIS | REDRAW_CONTENTS;
    if (newMaxYLim != maxYLim || newMinYLim != minYLim)
    	redrawArea |= REDRAW_V_AXIS | REDRAW_CONTENTS;
    
    /* Set the new limits */
    hw->h1D.maxXLim = newMaxXLim;
    hw->h1D.minXLim = newMinXLim;
    hw->h1D.maxYLim = newMaxYLim;
    hw->h1D.minYLim = newMinYLim;
    
    /* redraw if the widget is realized */
    if (XtIsRealized(w))
    	redisplayContents(hw, X_SCREEN, redrawArea);
}
void H1DZoomIn(Widget w)
{
    H1DWidget hw = (H1DWidget)w;
    int logScaling = hw->h1D.logScaling;
    double xOffset, yOffset;
    double minXLim, maxXLim, minYLim, maxYLim;

    /* if log scaling was requested, express limits in log coordinates */
    minXLim = hw->h1D.minXLim;
    maxXLim = hw->h1D.maxXLim;
    minYLim = logScaling ? log10(hw->h1D.minYLim) : hw->h1D.minYLim;
    maxYLim = logScaling ? log10(hw->h1D.maxYLim) : hw->h1D.maxYLim;
    
    /* Calculate offsets for limits of displayed data to zoom by ZOOM_FACTOR */
    xOffset = (maxXLim - minXLim) * ZOOM_FACTOR / 2;
    yOffset = (maxYLim - minYLim) * ZOOM_FACTOR / 2;

    /* Narrow the plotting limits by the offsets calculated above */
    maxXLim -= xOffset;
    minXLim += xOffset;
    maxYLim -= yOffset;
    minYLim += yOffset;
    
    /* Set the new limits */
    hw->h1D.maxXLim = maxXLim;
    hw->h1D.minXLim = minXLim;
    hw->h1D.maxYLim = logScaling ? pow(10.,maxYLim) : maxYLim;
    hw->h1D.minYLim = logScaling ? pow(10.,minYLim) : minYLim;
   
    /* redraw if the widget is realized */
    if (XtIsRealized(w))
    	redisplayContents(hw, X_SCREEN,
    		REDRAW_V_AXIS | REDRAW_H_AXIS | REDRAW_CONTENTS);
}
void H1DResetZoom(Widget w)
{
    H1DWidget hw = (H1DWidget)w;

    calcYDataRange(hw, &hw->h1D.minYData, &hw->h1D.maxYData);
    hw->h1D.minXLim = hw->h1D.minXData;
    if (hw->h1D.logScaling)
    	hw->h1D.minYLim = LOG_MIN_LIMIT;
    else
    	hw->h1D.minYLim = hw->h1D.minYData;
    hw->h1D.maxXLim = hw->h1D.maxXData;
    hw->h1D.maxYLim = hw->h1D.maxYData;

    if (XtIsRealized(w))
	redisplayContents(hw, X_SCREEN,
  		REDRAW_V_AXIS | REDRAW_H_AXIS | REDRAW_CONTENTS);
}

/*
** H1DPrintContents
**
** Prints the contents H1D widget to a PostScript file.
**
** Parameters
**
**	w		A h1D widget
**	psFileName	Name for the PostScript file that will be created
**	
*/
void H1DPrintContents(Widget w, char *psFileName)
{
    FILE * ps;
    H1DWidget hw = (H1DWidget)w;

    ps = OpenPS(psFileName, hw->core.width, hw->core.height);
    if (ps != NULL) {
	redisplayContents(hw, PS_PRINTER, REDRAW_ALL);
	EndPS();
    }    
}

/*
** H1DWritePS
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
void H1DWritePS(Widget w, FILE *fp)
{
    FILE *oldFP;
    
    oldFP = PSGetFile();
    PSSetFile(fp);
    redisplayContents((H1DWidget)w, PS_PRINTER, REDRAW_ALL);
    PSSetFile(oldFP);   
}

/*
** Redisplays the contents part of the widget, without the motif shadows and
** highlights.
*/
static void redisplayContents(H1DWidget w, int outDevice, int redrawArea)
{
    Display *display = XtDisplay(w);
    GC gc = w->h1D.gc;
    XFontStruct *fs = getFontStruct(w->h1D.font);
    Drawable drawBuf;
    XSegment segs[2];
 
    /* Save some energy if the widget isn't visible or no drawing requested */
    if ((outDevice==X_SCREEN && !w->core.visible) || redrawArea == REDRAW_NONE)
        return;

    /* Set destination for drawing commands, offscreen pixmap or window */
    if (w->h1D.doubleBuffer)
    	drawBuf = w->h1D.drawBuffer;
    else
    	drawBuf = XtWindow(w);

    /* Clear the drawing buffer or window only in the areas that have
       changed.  The other parts are still redrawn, but the net effect
       is that the unchanged areas do not flicker */
    if (outDevice == X_SCREEN) {
	XSetForeground(display, gc, w->core.background_pixel);
	if (redrawArea == REDRAW_ALL) {
	    XFillRectangle(display, drawBuf, gc, w->h1D.xMin, w->h1D.yMin,
    		     w->h1D.xMax - w->h1D.xMin, w->h1D.yMax - w->h1D.yMin);
	} else {
    	    if (redrawArea & REDRAW_V_AXIS)
		XFillRectangle(display, drawBuf, gc, w->h1D.axisLeft,
	   		w->h1D.axisTop, w->h1D.xOrigin - w->h1D.axisLeft,
    			w->h1D.axisBottom - w->h1D.axisTop);
    	    if (redrawArea & REDRAW_H_AXIS)
    		XFillRectangle(display, drawBuf, gc, w->h1D.axisLeft,
    	    		w->h1D.yOrigin + 1, w->h1D.axisRight-w->h1D.axisLeft,
    	    		w->h1D.axisBottom - w->h1D.yOrigin + 1);
	    if (redrawArea & REDRAW_CONTENTS)
    		XFillRectangle(display, drawBuf, gc, w->h1D.xOrigin + 1,
    	   		w->h1D.yEnd, w->h1D.xEnd - w->h1D.xOrigin,
    	   		w->h1D.yOrigin - w->h1D.yEnd);
	}  
    }
    
    /* Draw the axes */
    XSetForeground(display, gc, w->primitive.foreground);
    if (w->h1D.bins == NULL) {
        /* empty of data, just draw axis lines */
    	segs[0].x1 = segs[0].x2 = segs[1].x1 = w->h1D.xOrigin;
    	segs[0].y1 = segs[1].y1 = segs[1].y2 = w->h1D.yOrigin;
    	segs[1].x2 = w->h1D.xEnd; segs[0].y2 = w->h1D.yEnd;
	if (outDevice == X_SCREEN)
    	    XDrawSegments(display, drawBuf, gc, segs, 2);
	else /* PS_PRINTER */
    	    PSDrawSegments(display, drawBuf, gc, segs, 2);
    } else {
	DrawHorizontalAxis(display, drawBuf, gc, fs, outDevice, w->h1D.yOrigin,
	    w->h1D.xOrigin, w->h1D.xEnd, w->h1D.minXData, w->h1D.maxXData,
	    w->h1D.minXLim ,w->h1D.maxXLim, False,
	    (w->h1D.binEdgeLabeling && w->h1D.edges==NULL) ? w->h1D.nBins : 0);
	DrawVerticalAxis(display, drawBuf, gc, fs, outDevice, w->h1D.xOrigin,
	    w->h1D.xMin, w->h1D.yOrigin, w->h1D.yEnd,w->h1D.minYData,
    	    w->h1D.maxYData, w->h1D.minYLim, w->h1D.maxYLim, w->h1D.logScaling);
    }
    
    /* Draw the axis labels */
    if (w->h1D.xAxisLabel != NULL)
    	if (outDevice == X_SCREEN)
    	    XmStringDraw(display, drawBuf, w->h1D.font, w->h1D.xAxisLabel,
		gc, w->h1D.xOrigin, w->h1D.axisBottom + X_LABEL_MARGIN,
		w->h1D.xEnd - w->h1D.xOrigin, XmALIGNMENT_CENTER,
	     	XmSTRING_DIRECTION_L_TO_R, NULL);
    	else
    	    PSDrawXmString(display, drawBuf, w->h1D.font, w->h1D.xAxisLabel,
		gc, w->h1D.xOrigin, w->h1D.axisBottom + X_LABEL_MARGIN,
		w->h1D.xEnd - w->h1D.xOrigin, XmALIGNMENT_CENTER);
    if (w->h1D.yAxisLabel != NULL)
    	if (outDevice == X_SCREEN)
    	    XmStringDraw(display, drawBuf, w->h1D.font, w->h1D.yAxisLabel, gc,
    		w->h1D.xOrigin + Y_LABEL_MARGIN, w->h1D.yMin + TOP_MARGIN,
		w->h1D.xEnd - w->h1D.xOrigin, XmALIGNMENT_BEGINNING,
	     	XmSTRING_DIRECTION_L_TO_R, NULL);
    	else
    	    PSDrawXmString(display, drawBuf, w->h1D.font, w->h1D.yAxisLabel,
    		gc, w->h1D.xOrigin + Y_LABEL_MARGIN, w->h1D.yMin + TOP_MARGIN,
		w->h1D.xEnd - w->h1D.xOrigin, XmALIGNMENT_BEGINNING);
    /* Draw the contents of the plot */
    drawH1D(w, drawBuf, outDevice);
    
    
    /* For double buffering, now copy offscreen pixmap to screen */
    if (w->h1D.doubleBuffer && outDevice == X_SCREEN)
    	XCopyArea(display, drawBuf, XtWindow(w), gc, 0, 0,
    		  w->core.width, w->core.height, 0, 0);
    
    /* Call the redisplay callback so an application which draws on the h1D
       widget can refresh it's graphics */
    if (XtIsRealized((Widget)w) && outDevice == X_SCREEN)
    	XtCallCallbacks((Widget)w, XmNredisplayCallback, NULL);
}

/*
** Draw the histogram itself using the list of points stored in the
** widget.  
*/
static void drawH1D(H1DWidget w, Drawable drawBuf, int outDevice)
{
    Display *display = XtDisplay(w);
    GC gc = w->h1D.contentsGC;
    int logScaling = w->h1D.logScaling;
    int xMin = w->h1D.xOrigin, yMin = w->h1D.yEnd;
    int xMax = w->h1D.xEnd, yMax = w->h1D.yOrigin;
    double minXData = w->h1D.minXData, maxXData = w->h1D.maxXData;
    double minXLim = w->h1D.minXLim, maxXLim = w->h1D.maxXLim;
    double minYData, minYLim, maxYLim, xScale, yScale, minXPix, maxYPix;
    XSegment *xSeg, *xSegs; int nSegs = 0; 
    double delX, x1, x2, y1, y2;
    float *bin;
    float *uppErr,*lowErr,maxUppErr = 0.0;	/* Error bar data */
    int hx1, hy1, hx2, hy2, sep, lastY, i;	/* Int type to prevent ovf */
   
    /* don't bother if there's no data */
    if (w->h1D.bins == NULL)
    	return;

    /* if log scaling was requested, express limits in log coordinates */
    if (logScaling) {
    	minYData = log10(w->h1D.minYData);
    	minYLim = log10(w->h1D.minYLim); maxYLim = log10(w->h1D.maxYLim + maxUppErr);
    } else {
    	minYData = w->h1D.minYData;
    	minYLim = w->h1D.minYLim; maxYLim = w->h1D.maxYLim + maxUppErr;
    }
    xScale = (w->h1D.xEnd - w->h1D.xOrigin) / (maxXLim - minXLim);
    yScale = (w->h1D.yOrigin - w->h1D.yEnd) / (maxYLim - minYLim);
    minXPix = w->h1D.xOrigin - (minXLim - minXData) * xScale;
    maxYPix = w->h1D.yOrigin + (minYLim - minYData) * yScale;
    
    /* allocate memory for an array of XSegment structures for drawing lines 
       Enough must be allocated for 3 segs and triple lines for overflow.
       Additionally, error bars can account for 7 segments per bin */
    xSegs = (XSegment *)XtMalloc(sizeof(XSegment)*((w->h1D.nBins)*12+1));
    if (xSegs == NULL) {
	fprintf(stderr, "H1D Can't Allocate Memory for Drawing\n");
	return;
     }    

    /* loop through all of the data converting the data coordinates to
       X coordinates and drawing left and top segments of each bin */
    lastY = yMax;
    xSeg = xSegs;
    delX = (maxXData - minXData) / (double) (w->h1D.nBins);
    for (i=0, bin=w->h1D.bins; i<w->h1D.nBins; i++, bin++) {

	/* calculate left and right edges of bin in data coordinates, or
	   use the edges array, if supplied, to draw an adaptive histogram */
	if (w->h1D.edges) {
	    x1 = w->h1D.edges[i];
	    x2 = w->h1D.edges[i+1];
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
	if (w->h1D.barSepPercent != 0) {
	    sep = ((hx2 - hx1) * w->h1D.barSepPercent/100) / 2;
	    hx1 += sep;
	    hx2 -= sep;
	    if (hx1 > xMax)
	    	continue;
	}
	
	/* calculate the top and bottom of the sides of the bin (if bins
	   are not separated, the top and bottom of the left leg of the bin) */
	y2 = logScaling ? log10(dMax(*bin, FLT_MIN)) : *bin;
	hy1 = (w->h1D.barSepPercent == 0) ? lastY : yMax;
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
	if (w->h1D.barSepPercent != 0) {
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
    if (maxXData == maxXLim && w->h1D.barSepPercent == 0) {
        xSeg->x1 = xSeg->x2 = hx2; xSeg->y1 = hy2; xSeg->y2 = yMax;
        xSeg++; nSegs++;
    }
    
    /* Finally, apply error bars if applicable */
    if (w->h1D.uppErr != NULL && w->h1D.lowErr != NULL) {
	for (i=0,bin=w->h1D.bins,uppErr=w->h1D.uppErr,lowErr=w->h1D.lowErr;
    		i<w->h1D.nBins; i++,bin++,uppErr++,lowErr++) {

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
            x2 = x1 + (delX - delX*w->h1D.barSepPercent/100.) / 4.0;
            x1 = x1 - (delX - delX*w->h1D.barSepPercent/100.) / 4.0;
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
static void updateBufferAllocation(H1DWidget w)
{ 
    if (w->h1D.drawBuffer)
    	XFreePixmap(XtDisplay(w), w->h1D.drawBuffer);
    if (w->h1D.doubleBuffer) {
    	w->h1D.drawBuffer = XCreatePixmap(XtDisplay(w),
		DefaultRootWindow(XtDisplay(w)), w->core.width, w->core.height,
    	 	DefaultDepthOfScreen(XtScreen(w)));   
    } else {
    	w->h1D.drawBuffer = 0;
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
static void adjustLimitsForLogScaling(H1DWidget w)
{
    if (!w->h1D.logScaling)
    	return;
    
    /* set the new minimums */	
    w->h1D.minYData = FLT_MIN;
    w->h1D.minYLim = LOG_MIN_LIMIT;

    /* make sure new limits are still less than the max limits */
    if (w->h1D.maxYLim <= LOG_MIN_LIMIT)
    	w->h1D.maxYLim = w->h1D.maxYData > LOG_MIN_LIMIT ?
    	    	w->h1D.maxYData : LOG_MIN_LIMIT * 2;
    if (w->h1D.maxYData <= LOG_MIN_LIMIT)
    	w->h1D.maxYData = LOG_MIN_LIMIT * 2;
}

/*
** Scans the histogram data stored in the widget & returns the minimum
** and maximum values appropriate for that data.
*/
static void calcYDataRange(H1DWidget w, double *minimum, double *maximum)
{
    double min, max;
    float *bin, *upErr, *lowErr;
    int i;
    
    /* If there is no data, return existing data limits */
    if (w->h1D.nBins == 0) {
   	*minimum = w->h1D.minYData;
    	*maximum = w->h1D.maxYData;
    	return;
    }
    
    /* Scan the data calculating min and max */
    min = FLT_MAX; max = -FLT_MAX;
    if (w->h1D.uppErr == NULL || w->h1D.lowErr == NULL) {
	for (i=0,bin=w->h1D.bins; i<w->h1D.nBins; i++, bin++) {
    	    if (*bin > max) max = *bin;
    	    if (*bin < min) min = *bin;
	}
    } else {
    	for (i=0,bin=w->h1D.bins,upErr=w->h1D.uppErr,lowErr=w->h1D.lowErr;
    		i<w->h1D.nBins; i++, bin++, upErr++, lowErr++) {
    	    if (*bin + *upErr > max) max = *bin + *upErr;
    	    if (*bin - *lowErr < min) min = *bin - *lowErr;
	}
    }
    
    /* For linear scaling, start histogram from zero unless minimum is
       negative.  For log scaling, set the data minimum greater than 0.
       but less than LOG_MIN_LIMIT to highlight zeros and negative numbers */
    if (w->h1D.logScaling)
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

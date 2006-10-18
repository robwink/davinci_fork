/*******************************************************************************
*									       *
* Cell.c -- Rectangular Cell  Plot Widget, to represent a 2D histogram         *
*						       			       *
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
* Written by Paul Lebrun, based on Scat.c which was written by Mark Edel.      *
*									       *
*******************************************************************************/
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#if XmVersion >= 1002
#include <Xm/PrimitiveP.h>
#endif
#include "../util/psUtils.h"
#include "drawAxes.h"
#include "dragAxes.h"
#include "CellP.h"

#define REDRAW_NONE 0
#define REDRAW_H_AXIS 1
#define REDRAW_V_AXIS 2
#define REDRAW_CONTENTS 4
#define REDRAW_LABELS 8
#define REDRAW_ALL 15

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
#define MAX_AXIS_PERCENT 22	/* maximum percentage of widget width that may
				   be used for drawing axes */

static void motionAP(CellWidget w, XEvent *event, char *args, int n_args);
static void btnUpAP(CellWidget w, XEvent *event, char *args, int n_args);
static void btn2AP(CellWidget w, XEvent *event, char *args, int n_args);
static void btn3AP(CellWidget w, XEvent *event, char *args, int n_args);
static void initialize(CellWidget request, CellWidget new);
static void redisplay(CellWidget w, XEvent *event, Region region);
static void redisplayContents(CellWidget w, int outDevice, int redrawArea);
static void drawCellPlot(CellWidget w, Drawable drawBuf, int outDevice);
static void destroy(CellWidget w);
static void resize(CellWidget w);
static Boolean setValues(CellWidget current, CellWidget request,CellWidget new);
static void updateBufferAllocation(CellWidget w);
static XFontStruct *getFontStruct(XmFontList font);
static int compareRects(const void *rect1, const void *rect2);
static void calcDataRange(CellWidget w, double *xMin, double *xMax,
	double *yMin, double *yMax);
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
      XtOffset(CellWidget, cell.doubleBuffer), XmRString, "False"},
    {XmNfontList, XmCFontList, XmRFontList, sizeof(XmFontList),
      XtOffset(CellWidget, cell.font), XmRImmediate, NULL},
    {XmNxAxisLabel, XmCXAxisLabel, XmRXmString, sizeof (XmString), 
      XtOffset(CellWidget, cell.xAxisLabel), XmRString, NULL},
    {XmNyAxisLabel, XmCYAxisLabel, XmRXmString, sizeof (XmString), 
      XtOffset(CellWidget, cell.yAxisLabel), XmRString, NULL},
    {XmNresizeCallback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (CellWidget, cell.resize), XtRCallback, NULL},
    {XmNbtn2Callback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (CellWidget, cell.btn2), XtRCallback, NULL},
    {XmNbtn3Callback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (CellWidget, cell.btn3), XtRCallback, NULL},
    {XmNredisplayCallback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (CellWidget, cell.redisplay), XtRCallback, NULL},
};

CellClassRec  cellClassRec = {
     /* CoreClassPart */
  {
    (WidgetClass) &xmPrimitiveClassRec,  /* superclass       */
    "Cell",                         /* class_name            */
    sizeof(CellRec),                /* widget_size           */
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
  /* Cell class part */
  {
    0,                              	/* ignored	                */
  }
};

WidgetClass cellWidgetClass = (WidgetClass)&cellClassRec;

/*
** Widget initialize method
*/
static void initialize(CellWidget request, CellWidget new)
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
    if (new->cell.font == NULL)
    	new->cell.font = XmFontListCopy(_XmGetDefaultFontList((Widget)new,
    		XmLABEL_FONTLIST));
    else
        new->cell.font = XmFontListCopy(new->cell.font);

    /* Make local copies of the XmStrings */
    if (new->cell.xAxisLabel != NULL)
    	new->cell.xAxisLabel = XmStringCopy(new->cell.xAxisLabel);
    if (new->cell.yAxisLabel != NULL)
    	new->cell.yAxisLabel = XmStringCopy(new->cell.yAxisLabel);
     
    /* Create graphics contexts for drawing in the widget */
    values.font = getFontStruct(new->cell.font)->fid;
    values.foreground = new->primitive.foreground;
    values.background = new->core.background_pixel;
    new->cell.gc = XCreateGC(display, XDefaultRootWindow(display),
    			     GCForeground|GCBackground|GCFont, &values);
    new->cell.contentsGC = XCreateGC(display, XDefaultRootWindow(display),
    				     GCForeground|GCBackground, &values);
    
    /* Initialize various fields */
    ResetAxisDragging(&new->cell.dragState);
    new->cell.rects = NULL;
    new->cell.nRects = 0;
    new->cell.drawBuffer = 0;
    
    /* Default plotting boundaries */
    CellSetContents((Widget)new, NULL, 0, CELL_RESCALE);

    /* Set size dependent items */
    resize(new);
}

/*
** Widget destroy method
*/
static void destroy(CellWidget w)
{
    XFreeGC(XtDisplay(w), w->cell.gc);
    XFreeGC(XtDisplay(w), w->cell.contentsGC);
    if (w->cell.font != NULL)
    	XmFontListFree(w->cell.font);
    if (w->cell.drawBuffer)
    	XFreePixmap(XtDisplay(w), w->cell.drawBuffer);
    XtRemoveAllCallbacks ((Widget)w, XmNresizeCallback);
    XtRemoveAllCallbacks ((Widget)w, XmNbtn2Callback);
    XtRemoveAllCallbacks ((Widget)w, XmNbtn3Callback);
    XtRemoveAllCallbacks ((Widget)w, XmNredisplayCallback);
    if (w->cell.xAxisLabel != NULL)
    	XmStringFree(w->cell.xAxisLabel);
    if (w->cell.yAxisLabel != NULL)
    	XmStringFree(w->cell.yAxisLabel);
    if (w->cell.rects != NULL)
    	XtFree((char *)w->cell.rects);
}

/*
** Widget resize method
*/
static void resize(CellWidget w)
{
    XFontStruct *fs = getFontStruct(w->cell.font);
    int borderWidth =
    	w->primitive.shadow_thickness + w->primitive.highlight_thickness;
    XRectangle clipRect;

    /* resize the drawing buffer, an offscreen pixmap for smoother animation */
    updateBufferAllocation(w);

    /* calculate the area of the widget where contents can be drawn */
    w->cell.xMin = borderWidth;
    w->cell.yMin = borderWidth;
    w->cell.xMax = w->core.width - borderWidth;
    w->cell.yMax = w->core.height - borderWidth;

    /* calculate positions for the axes and contents depending on whether
       axis labels are specified, and the measurements of the current font */
    if (w->cell.yAxisLabel != NULL)
    	w->cell.yEnd = w->cell.yMin + fs->ascent + fs->descent + TOP_MARGIN;
    else
    	w->cell.yEnd = VAxisEndClearance(fs) + fs->ascent/2 + TOP_MARGIN;
    w->cell.axisTop = w->cell.yEnd - VAxisEndClearance(fs);
    if (w->cell.xAxisLabel != NULL)
    	w->cell.axisBottom = w->cell.yMax - BOTTOM_MARGIN - fs->ascent -
    				fs->descent - X_LABEL_MARGIN;
    else
    	w->cell.axisBottom = w->cell.yMax - fs->ascent/2 - BOTTOM_MARGIN;
    w->cell.yOrigin = w->cell.axisBottom - HAxisHeight(fs);
    w->cell.axisLeft = w->cell.xMin + LEFT_MARGIN;
    w->cell.xOrigin = w->cell.axisLeft + VAxisWidth(fs);
    if (w->cell.xOrigin > (w->cell.xMax - w->cell.xMin) * MAX_AXIS_PERCENT/100)
    	w->cell.xOrigin = (w->cell.xMax - w->cell.xMin) * MAX_AXIS_PERCENT/100;
    w->cell.axisRight = w->cell.xMax - RIGHT_MARGIN;
    w->cell.xEnd = w->cell.axisRight - HAxisEndClearance(fs);
    if (w->cell.xMax - w->cell.xEnd > w->cell.xOrigin - w->cell.xMin)
    	w->cell.xEnd = w->cell.xMax - (w->cell.xOrigin - w->cell.xMin);
    
    /* set plot contents gc to clip drawing at the edges */
    clipRect.x = w->cell.xOrigin;
    clipRect.y = w->cell.yEnd;
    clipRect.width = w->cell.xEnd - w->cell.xOrigin;
    clipRect.height = w->cell.yOrigin - w->cell.yEnd;
    XSetClipRectangles(XtDisplay(w), w->cell.contentsGC, 0, 0, &clipRect,
    		       1, Unsorted);

    /* set drawing gc to clip drawing before motif shadow and highlight */
    clipRect.x = borderWidth;
    clipRect.y = borderWidth;
    clipRect.width = w->core.width - 2 * borderWidth;
    clipRect.height = w->core.height - 2 * borderWidth;
    XSetClipRectangles(XtDisplay(w), w->cell.gc, 0, 0, &clipRect, 1, Unsorted);
   
    /* call the resize callback */
    if (XtIsRealized((Widget)w))
    	XtCallCallbacks((Widget)w, XmNresizeCallback, NULL);
}

/*
** Widget redisplay method
*/
static void redisplay(CellWidget w, XEvent *event, Region region)
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
    else if (_XmDifferentBackground((Widget)w, XtParent((Widget)w)))
	_XmUnhighlightBorder((Widget)w);
    
    /* Now draw the contents of the cell widget */
    redisplayContents(w, X_SCREEN, REDRAW_ALL);
}

/*
** Widget setValues method
*/
static Boolean setValues(CellWidget current, CellWidget request, CellWidget new)
{
    Boolean redraw = False, doResize = False;
    Display *display = XtDisplay(new);

    /* If the foreground or background color has changed, change the GCs */
    if (new->core.background_pixel !=current->core.background_pixel) {
    	XSetForeground(display, new->cell.gc, new->primitive.foreground);
    	XSetForeground(display, new->cell.contentsGC,new->primitive.foreground);
    	redraw = TRUE;
    }
    if (new->primitive.foreground != current->primitive.foreground) {
    	XSetBackground(display, new->cell.gc, new->core.background_pixel);
    	XSetBackground(display,new->cell.contentsGC,new->core.background_pixel);
    	redraw = TRUE;
    }

    /* if double buffering changes, allocate or deallocate offscreen pixmap */
    if (new->cell.doubleBuffer != current->cell.doubleBuffer) {
    	updateBufferAllocation(new);
    	redraw = TRUE;
    }

    /* if labels are changed, free the old ones and copy the new ones */
    if (new->cell.xAxisLabel != current->cell.xAxisLabel) {
    	if (current->cell.xAxisLabel != NULL)
    	    XmStringFree(current->cell.xAxisLabel);
    	new->cell.xAxisLabel = XmStringCopy(new->cell.xAxisLabel);
    	doResize = TRUE;
    }
    if (new->cell.yAxisLabel != current->cell.yAxisLabel) {
    	if (current->cell.yAxisLabel != NULL)
    	    XmStringFree(current->cell.yAxisLabel);
    	new->cell.yAxisLabel = XmStringCopy(new->cell.yAxisLabel);
    	doResize = TRUE;
    }
    
    /* if highlight thickness or shadow thickness changed, resize and redraw */
    if  ((new->primitive.highlight_thickness != 
          current->primitive.highlight_thickness) ||
         (new -> primitive.shadow_thickness !=
          current->primitive.shadow_thickness)) {
    	redraw = TRUE;
    }
    if (doResize)
    	resize(new);
    return redraw; 
} 

/*
** Button press and button motion action proc.
*/
static void motionAP(CellWidget w, XEvent *event, char *args, int n_args)
{
    int chgdArea, redrawArea = REDRAW_NONE;

    if (event->type == ButtonPress)
    	XmProcessTraversal((Widget)w, XmTRAVERSE_CURRENT);
    
    chgdArea = DragAxes(event, w->cell.xOrigin, w->cell.xEnd, w->cell.yOrigin,
    	w->cell.yEnd, w->cell.axisLeft, w->cell.axisTop, w->cell.axisBottom,
    	w->cell.axisRight, w->cell.minXData, w->cell.maxXData, w->cell.minYData,
    	w->cell.maxYData, False, False,
    	&w->cell.minXLim, &w->cell.maxXLim, &w->cell.minYLim, &w->cell.maxYLim,
    	&w->cell.dragState, &w->cell.xDragStart, &w->cell.yDragStart);
    if (chgdArea & DA_REDRAW_H_AXIS) redrawArea |= REDRAW_H_AXIS;
    if (chgdArea & DA_REDRAW_V_AXIS) redrawArea |= REDRAW_V_AXIS;
    if (chgdArea & DA_REDRAW_CONTENTS) redrawArea |= REDRAW_CONTENTS;

    redisplayContents(w, X_SCREEN, redrawArea);
}
/*
** Button up action proc.
*/
static void btnUpAP(CellWidget w, XEvent *event, char *args, int n_args)
{
    ResetAxisDragging(&w->cell.dragState);
}

static void btn2AP(CellWidget w, XEvent *event, char *args, int n_args)
{
    CellCallbackStruct cbStruct;

    XmProcessTraversal((Widget)w, XmTRAVERSE_CURRENT);

    /* Just call the callback */
    cbStruct.reason = XmCR_INPUT;
    cbStruct.event = event;
    XtCallCallbacks((Widget)w, XmNbtn2Callback, (XtPointer)&cbStruct);
}

static void btn3AP(CellWidget w, XEvent *event, char *args, int n_args)
{    
    CellCallbackStruct cbStruct;

    XmProcessTraversal((Widget)w, XmTRAVERSE_CURRENT);

    /* Just call the callback */
    cbStruct.reason = XmCR_INPUT;
    cbStruct.event = event;
    XtCallCallbacks((Widget)w, XmNbtn3Callback, (XtPointer)&cbStruct);
}

/*
** CellSetContents
**
** Specify the contents of the cell widget in the form of rects in 2D space.
** Takes an array of CellRect data structures which contain the X and Y
** coordinates of the rect.
**
** Parameters
**
** 	w		A cell widget
**	rects		An array of rectangles to display
**	nRects		The number of elements specified in rects
**	rescale		One of: CELL_NO_RESCALE, CELL_RESCALE, or
**			CELL_GROW_ONLY.  Tells the widget how to change
**			its display to incorporate the new contents.
*/
void CellSetContents(Widget w, CellRect *rects, int nRects, int rescale)
{
    CellWidget sw = (CellWidget)w;
    int redrawArea = REDRAW_NONE;
    double minX, minY, maxX, maxY;
    
    /* Free the previous data */
    if (sw->cell.nRects != 0) {
    	XtFree((char *)sw->cell.rects);
    	sw->cell.nRects = 0;
    }
    
    /* Copy in the new data, if any, and calculate the min and max values */
    sw->cell.nRects = nRects;
    if (nRects == 0) {
    	if (rescale == CELL_RESCALE) {
    	    minX = minY = 0.;
    	    maxX = maxY = 1.;
    	} else {
    	    minX = sw->cell.minXData; minY = sw->cell.minYData;
    	    maxX = sw->cell.maxXData; maxY = sw->cell.maxYData;
    	}
    } else {
    	/* allocate memory and copy the data */
    	sw->cell.rects = (CellRect *)XtMalloc(sizeof(CellRect) * nRects);
    	memcpy(sw->cell.rects, rects, sizeof(CellRect) * nRects);
	sw->cell.nRects = nRects;
	/* Sort by color so the drawing routines can batch drawing calls */
	qsort(sw->cell.rects, nRects, sizeof(CellRect), compareRects);
	/* Calculate the range of the data */
	calcDataRange(sw, &minX, &maxX, &minY, &maxY);
    }
    
    /* recalculate scale and limits for the widget */
    if (rescale == CELL_RESCALE) {
	if (maxX == minX) {maxX += 1.; minX -= 1.;}  /* fix zero range data */
	if (maxY == minY) {maxY += 1.; minY -= 1.;}
    	sw->cell.maxXData = maxX; sw->cell.maxXLim = maxX;
    	sw->cell.minXData = minX; sw->cell.minXLim = minX;
    	sw->cell.maxYData = maxY; sw->cell.maxYLim = maxY;
    	sw->cell.minYData = minY; sw->cell.minYLim = minY;
    	redrawArea = REDRAW_CONTENTS | REDRAW_H_AXIS | REDRAW_V_AXIS;
    } else if (rescale == CELL_NO_RESCALE) {
	redrawArea = REDRAW_CONTENTS;
	if (maxX > sw->cell.maxXData) {
	    sw->cell.maxXData = maxX;
	    redrawArea |= REDRAW_H_AXIS;
	}
	if (minX < sw->cell.minXData) {
	    sw->cell.minXData = minX;
	    redrawArea |= REDRAW_H_AXIS;
	}
	if (maxY > sw->cell.maxYData) {
	    sw->cell.maxYData = maxY;
	    redrawArea |= REDRAW_V_AXIS;
	}
	if (minY < sw->cell.minYData) {
	    sw->cell.minYData = minY;
	    redrawArea |= REDRAW_V_AXIS;
	}
    } else if (rescale == CELL_RESCALE_AT_MAX || rescale == CELL_GROW_ONLY) {
	redrawArea |= REDRAW_CONTENTS;
	if (CELL_GROW_ONLY) {
	    minX = dMin(sw->cell.minXData, minX);
	    maxX = dMax(sw->cell.maxXData, maxX);
	    minY = dMin(sw->cell.minYData, minY);
	    maxY = dMax(sw->cell.maxYData, maxY);
    	}
    	if (sw->cell.maxXData != maxX || sw->cell.minXData != minX)
    	    redrawArea |= REDRAW_H_AXIS;
    	if (sw->cell.maxYData != maxY || sw->cell.minYData != minY)
    	    redrawArea |= REDRAW_V_AXIS;
    	if (sw->cell.maxXData == sw->cell.maxXLim)
    	    sw->cell.maxXLim = maxX;
    	if (sw->cell.minXData == sw->cell.minXLim)
    	    sw->cell.minXLim = minX;
    	if (sw->cell.maxYData == sw->cell.maxYLim)
    	    sw->cell.maxYLim = maxY;
    	if (sw->cell.minYData == sw->cell.minYLim)
    	    sw->cell.minYLim = minY;
    	sw->cell.maxXData = maxX; sw->cell.minXData = minX;
   	sw->cell.maxYData = maxY; sw->cell.minYData = minY;
    }
    
    /* redraw the widget with the new data */
    if (XtIsRealized(w))
    	redisplayContents(sw, X_SCREEN, redrawArea);
}

/*
** CellSetVisibleRange, CellGetVisibleRange
**
** Set (Get) the range of data that is visible.  minXLim, minYLim, maxXLim, and
** maxYLim specify the endpoints of the x and y axes.  CellSetVisibleRange,
** unlike the widgets interactive rescaling routines, can zoom out past the
** actual minimum and maximum data points.
*/
void CellSetVisibleRange(Widget w, double minXLim, double minYLim,
			 double maxXLim, double maxYLim)
{
    CellWidget sw = (CellWidget)w;
    double minX, minY, maxX, maxY;
    
    /* calculate the actual range of the data */
    calcDataRange(sw, &minX, &maxX, &minY, &maxY);

    /* allow user to zoom beyond the range of the data */
    sw->cell.maxXData = dMax(maxXLim, maxX);
    sw->cell.minXData = dMin(minXLim, minX);
    sw->cell.maxYData = dMax(maxYLim, maxY);
    sw->cell.minYData = dMin(minYLim, minY);

    /* Set the range */
    sw->cell.minXLim = minXLim;
    sw->cell.maxXLim = maxXLim;
    sw->cell.minYLim = minYLim;
    sw->cell.maxYLim = maxYLim;

    /* redraw if the widget is realized */
    if (XtIsRealized(w))
    	redisplayContents(sw, X_SCREEN,
    		REDRAW_V_AXIS | REDRAW_H_AXIS | REDRAW_CONTENTS);
}
void CellGetVisibleRange(Widget w, double *minXLim, double *minYLim,
			 double *maxXLim, double *maxYLim)
{
    *minXLim = ((CellWidget)w)->cell.minXLim;
    *maxXLim = ((CellWidget)w)->cell.maxXLim;
    *minYLim = ((CellWidget)w)->cell.minYLim;
    *maxYLim = ((CellWidget)w)->cell.maxYLim;
}

/*
** CellZoomOut, CellZoomIn, CellResetZoom
**
** Zoom in and out by ZOOM_FACTOR.  Zoom in is centered on the current
** center of the plot.
*/
void CellZoomOut(Widget w)
{
    CellWidget sw = (CellWidget)w;
    double xOffset, yOffset, newMaxXLim, newMinXLim, newMaxYLim, newMinYLim;
    double minXLim = sw->cell.minXLim, maxXLim = sw->cell.maxXLim;
    double minYLim = sw->cell.minYLim, maxYLim = sw->cell.maxYLim;
    int redrawArea = REDRAW_NONE;
    
    /* Calculate a suitable offset to reverse a zoom in by ZOOM_FACTOR */
    xOffset = (maxXLim - minXLim) * (ZOOM_FACTOR/(1.-ZOOM_FACTOR)) / 2;
    yOffset = (maxYLim - minYLim) * (ZOOM_FACTOR/(1.-ZOOM_FACTOR)) / 2;

    /* widen the plotting limits by the offsets calculated above,
       stopping when the limits reach the limits of the data */
    newMaxXLim = dMin(sw->cell.maxXData, maxXLim + xOffset);
    newMinXLim = dMax(sw->cell.minXData, minXLim - xOffset);
    newMaxYLim = dMin(sw->cell.maxYData, maxYLim + yOffset);
    newMinYLim = dMax(sw->cell.minYData, minYLim - yOffset);
    
    /* Tell widget to redraw, and what parts, if limits have changed */
    if (newMaxXLim != maxXLim || newMinXLim != minXLim)
    	redrawArea |= REDRAW_H_AXIS | REDRAW_CONTENTS;
    if (newMaxYLim != maxYLim || newMinYLim != minYLim)
    	redrawArea |= REDRAW_V_AXIS | REDRAW_CONTENTS;
    
    /* Set the new limits */
    sw->cell.maxXLim = newMaxXLim;
    sw->cell.minXLim = newMinXLim;
    sw->cell.maxYLim = newMaxYLim;
    sw->cell.minYLim = newMinYLim;

    /* redraw if the widget is realized */
    if (XtIsRealized(w))
    	redisplayContents(sw, X_SCREEN, redrawArea);
}
void CellZoomIn(Widget w)
{
    CellWidget sw = (CellWidget)w;
    double xOffset, yOffset;
    double minXLim = sw->cell.minXLim, maxXLim = sw->cell.maxXLim;
    double minYLim = sw->cell.minYLim, maxYLim = sw->cell.maxYLim;
    
    /* Calculate offsets for limits of displayed data to zoom by ZOOM_FACTOR */
    xOffset = (maxXLim - minXLim) * ZOOM_FACTOR / 2;
    yOffset = (maxYLim - minYLim) * ZOOM_FACTOR / 2;

    /* Narrow the plotting limits by the offsets calculated above */
    maxXLim -= xOffset;
    minXLim += xOffset;
    maxYLim -= yOffset;
    minYLim += yOffset;
    
    /* Set the new limits */
    sw->cell.maxXLim = maxXLim;
    sw->cell.minXLim = minXLim;
    sw->cell.maxYLim = maxYLim;
    sw->cell.minYLim = minYLim;
   
    /* redraw if the widget is realized */
    if (XtIsRealized(w))
    	redisplayContents(sw, X_SCREEN,
    		REDRAW_V_AXIS | REDRAW_H_AXIS | REDRAW_CONTENTS);
}
void CellResetZoom(Widget w)
{
    CellWidget sw = (CellWidget)w;
    double minX, minY, maxX, maxY;

    calcDataRange(sw, &minX, &maxX, &minY, &maxY);
    sw->cell.minXLim = sw->cell.minXData = minX;
    sw->cell.minYLim = sw->cell.minYData = minY;
    sw->cell.maxXLim = sw->cell.maxXData = maxX;
    sw->cell.maxYLim = sw->cell.maxYData = maxY;

    if (XtIsRealized(w))
	redisplayContents(sw, X_SCREEN,
  		REDRAW_V_AXIS | REDRAW_H_AXIS | REDRAW_CONTENTS);
}

/*
** CellPrintContents
**
** Prints the contents Cell widget to a PostScript file.
**
** Parameters
**
**	w		A cell widget
**	psFileName	Name for the PostScript file that will be created
**	
*/
void CellPrintContents(Widget w, char *psFileName)
{
    FILE * ps;
    CellWidget sw = (CellWidget)w;

    ps = OpenPS(psFileName, sw->core.width, sw->core.height);
    if (ps != NULL) {
	redisplayContents(sw, PS_PRINTER, REDRAW_ALL);
	EndPS();
    }    
}

/*
** CellWritePS
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
**	w	A cell widget
**	fp	File pointer for an open file in which to
**		write the drawing commands
*/
void CellWritePS(Widget w, FILE *fp)
{
    FILE *oldFP;
    
    oldFP = PSGetFile();
    PSSetFile(fp);
    redisplayContents((CellWidget)w, PS_PRINTER, REDRAW_ALL);
    PSSetFile(oldFP);   
}

/*
** Redisplays the contents part of the widget, without the motif shadows and
** highlights.
*/
static void redisplayContents(CellWidget w, int outDevice, int redrawArea)
{
    Display *display = XtDisplay(w);
    GC gc = w->cell.gc;
    XFontStruct *fs = getFontStruct(w->cell.font);
    Drawable drawBuf;
 
    /* Save some energy if the widget isn't visible or no drawing requested */
    if (!w->core.visible || redrawArea == REDRAW_NONE)
        return;

    /* Set destination for drawing commands, offscreen pixmap or window */
    if (w->cell.doubleBuffer)
    	drawBuf = w->cell.drawBuffer;
    else
    	drawBuf = XtWindow(w);

    /* Clear the drawing buffer or window only in the areas that have
       changed.  The other parts are still redrawn, but the net effect
       is that the unchanged areas do not flicker */
    if (outDevice == X_SCREEN) {
	XSetForeground(display, gc, w->core.background_pixel);
	if (redrawArea == REDRAW_ALL) {
	    XFillRectangle(display, drawBuf, gc, w->cell.xMin, w->cell.yMin,
    		     w->cell.xMax - w->cell.xMin, w->cell.yMax - w->cell.yMin);
	} else {
    	    if (redrawArea & REDRAW_V_AXIS)
		XFillRectangle(display, drawBuf, gc, w->cell.axisLeft,
	   		w->cell.axisTop, w->cell.xOrigin - w->cell.axisLeft,
    			w->cell.axisBottom - w->cell.axisTop);
    	    if (redrawArea & REDRAW_H_AXIS)
    		XFillRectangle(display, drawBuf, gc, w->cell.axisLeft,
    	    		w->cell.yOrigin + 1, w->cell.axisRight-w->cell.axisLeft,
    	    		w->cell.axisBottom - w->cell.yOrigin + 1);
    	    if (redrawArea & REDRAW_CONTENTS)
    		XFillRectangle(display, drawBuf, gc, w->cell.xOrigin + 1,
    	   		w->cell.yEnd, w->cell.xEnd - w->cell.xOrigin,
    	   		w->cell.yOrigin - w->cell.yEnd);
	}
    }
    
    /* Draw the axes */
    XSetForeground(display, gc, w->primitive.foreground);
    if (w->cell.nRects == 0) {
        /* empty of data, just draw axis lines */
    	XSegment segs[2];
    	segs[0].x1 = segs[0].x2 = segs[1].x1 = w->cell.xOrigin;
    	segs[0].y1 = segs[1].y1 = segs[1].y2 = w->cell.yOrigin;
    	segs[1].x2 = w->cell.xEnd; segs[0].y2 = w->cell.yEnd;
	if (outDevice == X_SCREEN)
    	    XDrawSegments(display, drawBuf, gc, segs, 2);
	else /* PS_PRINTER */
    	    PSDrawSegments(display, drawBuf, gc, segs, 2);
    } else {
	DrawHorizontalAxis(display, drawBuf, gc, fs, outDevice,
    	    w->cell.yOrigin, w->cell.xOrigin, w->cell.xEnd, w->cell.minXData,
    	    w->cell.maxXData, w->cell.minXLim, w->cell.maxXLim, False, 0);
	DrawVerticalAxis(display, drawBuf, gc, fs, outDevice, w->cell.xOrigin,
	    w->cell.xMin, w->cell.yEnd, w->cell.yOrigin, w->cell.minYData,
    	    w->cell.maxYData, w->cell.minYLim, w->cell.maxYLim, False);
    }
    
    /* Draw the axis labels */
    if (w->cell.xAxisLabel != NULL)
    	if (outDevice == X_SCREEN)
    	    XmStringDraw(display, drawBuf, w->cell.font, w->cell.xAxisLabel,
		gc, w->cell.xOrigin, w->cell.axisBottom + X_LABEL_MARGIN,
		w->cell.xEnd - w->cell.xOrigin, XmALIGNMENT_CENTER,
	     	XmSTRING_DIRECTION_L_TO_R, NULL);
    	else
    	    PSDrawXmString(display, drawBuf, w->cell.font, w->cell.xAxisLabel,
		gc, w->cell.xOrigin, w->cell.axisBottom + X_LABEL_MARGIN,
		w->cell.xEnd - w->cell.xOrigin, XmALIGNMENT_CENTER);
    if (w->cell.yAxisLabel != NULL)
    	if (outDevice == X_SCREEN)
    	    XmStringDraw(display, drawBuf, w->cell.font, w->cell.yAxisLabel, gc,
    		w->cell.xOrigin + Y_LABEL_MARGIN, w->cell.yMin + TOP_MARGIN,
		w->cell.xEnd - w->cell.xOrigin, XmALIGNMENT_BEGINNING,
	     	XmSTRING_DIRECTION_L_TO_R, NULL);
    	else
    	    PSDrawXmString(display, drawBuf, w->cell.font, w->cell.yAxisLabel,
    		gc, w->cell.xOrigin + Y_LABEL_MARGIN, w->cell.yMin + TOP_MARGIN,
		w->cell.xEnd - w->cell.xOrigin, XmALIGNMENT_BEGINNING);

    /* Draw the contents of the plot */
    drawCellPlot(w, drawBuf, outDevice);

    /* For double buffering, now copy offscreen pixmap to screen */
    if (w->cell.doubleBuffer && outDevice == X_SCREEN)
    	XCopyArea(display, drawBuf, XtWindow(w), gc, 0, 0,
    		  w->core.width, w->core.height, 0, 0);
    
    /* Call the redisplay callback so an application which draws on the cell
       widget can refresh it's graphics */
    if (XtIsRealized((Widget)w) && outDevice == X_SCREEN)
    	XtCallCallbacks((Widget)w, XmNredisplayCallback, NULL);
}
/*
** Draw the Cell plot itself using the list of rectangles stored in the
** widget.
*/
static void drawCellPlot(CellWidget w, Drawable drawBuf, int outDevice)
{
    int nPts = 0;
    Display *display = XtDisplay(w);
    GC gc = w->cell.contentsGC;
    Pixel lastPixel = -1;
    int xMin = w->cell.xOrigin, yMin = w->cell.yEnd;
    int xMax = w->cell.xEnd, yMax = w->cell.yOrigin;
    double minXData = w->cell.minXData, minXLim = w->cell.minXLim;
    double maxXLim = w->cell.maxXLim, minYData = w->cell.minYData;
    double minYLim = w->cell.minYLim, maxYLim = w->cell.maxYLim;
    double xScale, yScale, minXPix, maxYPix;
    XRectangle *pt, *pts;
    CellRect *rect;
    int x, dx, y, dy, x1, x2, y1, y2, i;
    
    /* don't bother if there's no data */
    if (w->cell.nRects == 0)
    	return;
    
    /* calculate conversion factors from data to screen coordinates */
    xScale = (w->cell.xEnd - w->cell.xOrigin) / (maxXLim - minXLim);
    yScale = (w->cell.yOrigin - w->cell.yEnd) / (maxYLim - minYLim);
    
    /* find equivalents of minXData & minYData in screen coordinates */
    minXPix = w->cell.xOrigin - (minXLim - minXData) * xScale;
    maxYPix = w->cell.yOrigin + (minYLim - minYData) * yScale;
    
    /* allocate an array of XRectangle structures for drawing rects */
    pts = (XRectangle *)XtMalloc(sizeof(XRectangle)*(w->cell.nRects));
    
    /* loop through all of the data converting the data coordinates to
       X coordinates and drawing.  Accumulate runs of the same color
       (the rects were sorted by color in CellSetContents) in the array
       pts, and draw whenever the color changes. */
    pt = pts;
    for (i=0, rect=w->cell.rects; i<w->cell.nRects; i++, rect++) {
    	if (rect->pixel != lastPixel) {
    	    /* draw the rects in the current color, then switch */
    	    if (nPts != 0) {
    	    	XSetForeground(display, gc, lastPixel);
    	    	if (outDevice == X_SCREEN)
    		    XFillRectangles(display, drawBuf, gc, pts, nPts);
    		else
    		    PSFillRectangles(display, drawBuf, gc, pts, nPts);
    	    	nPts = 0;
    	    	pt = pts;
    	    }
    	    lastPixel = rect->pixel;
    	}
        x = (int)(minXPix + (rect->x - rect->dx/2 - minXData) * xScale);
        dx = (int)(xScale*rect->dx);
        y = (int)(maxYPix - (rect->y + rect->dy/2 - minYData) * yScale);
        dy = (int)(yScale*rect->dy);
   	if (x+dx > xMin && x < xMax && y+dy > yMin && y < yMax &&
    		dx > 0 && dy > 0) {
            pt->x = x;
            pt->y = y;
            pt->width = dx;
            pt->height = dy;
            pt++; nPts++;
        }
    }
    if (nPts != 0) {
    	XSetForeground(display, gc, lastPixel);
    	if (outDevice == X_SCREEN)
            XFillRectangles(display, drawBuf, gc, pts, nPts);
        else
    	    PSFillRectangles(display, drawBuf, gc, pts, nPts);
    }
    XtFree((char *)pts);
    
}

/*
** For smoother animation, it is possible to allocate an offscreen pixmap
** and draw to that rather than directly into the window.  Unfortunately,
** it is too slow on many machines, so we have to give the user the choice
** whether to use it or not.  This routine reallocates the offscreen
** pixmap buffer when the window is resized, and when the widget is created
*/
static void updateBufferAllocation(CellWidget w)
{ 
    if (w->cell.drawBuffer)
    	XFreePixmap(XtDisplay(w), w->cell.drawBuffer);
    if (w->cell.doubleBuffer) {
    	w->cell.drawBuffer = XCreatePixmap(XtDisplay(w),
		DefaultRootWindow(XtDisplay(w)), w->core.width, w->core.height,
    	 	DefaultDepthOfScreen(XtScreen(w)));   
    } else {
    	w->cell.drawBuffer = 0;
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

/* compare procedure for qsort for sorting rectangles by color */
static int compareRects(const void *rect1, const void *rect2)
{
    CellRect *pt1 = (CellRect *)rect1, *pt2 = (CellRect *)rect2;
    
    if (pt1->pixel < pt2->pixel)
    	return -1;
    else if (pt1->pixel == pt2->pixel)
    	return 0;
    else
    	return 1;
}

static void calcDataRange(CellWidget w, double *xMin, double *xMax,
	double *yMin, double *yMax)
{
    CellRect *rect;
    double minX = FLT_MAX, minY = FLT_MAX, maxX = -FLT_MAX, maxY = -FLT_MAX;
    int i;

    for (i=0, rect=w->cell.rects; i<w->cell.nRects; i++, rect++) {
    	if (rect->x + rect->dx/2 > maxX) maxX = rect->x + rect->dx/2;
    	if (rect->y + rect->dy/2 > maxY) maxY = rect->y + rect->dy/2;
    	if (rect->x - rect->dx/2 < minX) minX = rect->x - rect->dx/2;
    	if (rect->y - rect->dy/2 < minY) minY = rect->y - rect->dy/2;
    }
    *xMin = minX; *yMin = minY; *xMax = maxX; *yMax = maxY;
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

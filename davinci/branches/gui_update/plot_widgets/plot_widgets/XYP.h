/*******************************************************************************
*									       *
* XYP.h - General Purpose Plot Widget, Private Header File			       *
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
#ifndef XYP_H
#define XYP_H

#include "XY.h"
#include <Xm/XmP.h>
#if XmVersion >= 2000
#include <X11/CoreP.h>
#include <Xm/PrimitiveP.h>
#endif

/* Masks for toRedraw field denoting window areas in need of redraw */
#define REDRAW_NONE 0
#define REDRAW_H_AXIS 1
#define REDRAW_V_AXIS 2
#define REDRAW_CONTENTS 4
#define REDRAW_LABELS 8
#define REDRAW_ALL 15

typedef struct _XYClassPart{
    int ignore;
} XYClassPart;

typedef struct _XYClassRec{
    CoreClassPart  core_class;
    XmPrimitiveClassPart primitive_class;
    XYClassPart  xy_class;
} XYClassRec;

extern XYClassRec xyClassRec;

typedef struct _XYPart {
    GC gc;	   	       	/* Graphics context for axes & labels */
    GC contentsGC;		/* Graphics context for plot contents */
    Pixmap drawBuffer;		/* Double buffering for non-flashing draws */
    int xMin, yMin, xMax, yMax;	/* Boundaries of the drawable area of widget */
    XmFontList font;		/* Motif font list associated with widget */
    XtCallbackList resize;	/* Callbacks */
    XtCallbackList btn2;
    XtCallbackList btn3;
    XtCallbackList redisplay;
    Boolean doubleBuffer;	/* When set, draw first to offscreen pixmap */
    Boolean xLogScaling;	/* When set, plot X axis as log of X */
    Boolean yLogScaling;	/* When set, plot Y axis as log of Y */
    Boolean showLegend;		/* When set, show the plot legend */
    XmString xAxisLabel;	/* Compound string labels for axes */
    XmString yAxisLabel;
    int marginPercent;		/* Size of plot border in % of data range */
    int xOrigin, yOrigin;	/* The point where the axis lines meet */
    int xEnd, yEnd;		/* The ends of the x and y axis lines */
    int legendTop;		/* Y coord. of the top of the plot legend */
    int legendLeftMargin;	/* left edge of legend */
    int axisLeft, axisTop;	/* Along with xOrigin and yOrigin, define */
    int axisBottom, axisRight;	/*    the boundaries of the axis areas	  */
    int legendRows;		/* Number of rows in the plot legend */
    int legendColumnSpacing;	/* How far apart to space legend columns */
    int dragState;		/* Is the user currently dragging the mouse? */
    int toRedraw;   	    	/* Changed window areas not yet redrawn */
    double xDragStart;		/* X (data coord) position of start of drag */
    double yDragStart;		/* Y (data coord) position of start of drag */
    double minXData, maxXData;	/* Minimum and maximum x data values */
    double minYData, maxYData;	/* Minimum and maximum y data values */
    double minXLim, maxXLim;	/* Min and max x data actually displayed */
    double minYLim, maxYLim;	/* Min and max y data actually displayed */
    double minXDragStop;    	/* Limits for interactive dragging */
    double maxXDragStop;
    double minYDragStop;
    double maxYDragStop;
    XYCurve *curves;		/* Data to be displayed on the plot */
    int nCurves;		/* Number of curves in curves above */
    XYHistogram *histograms;	/* Histograms to be displayed */
    int nHistograms;
    XYString *strings;		/* Text strings to be displayed on the plot */
    int nStrings;		/* Number of text strings in strings above */
} XYPart;

typedef struct _XYRec {
   CorePart        core;
   XmPrimitivePart primitive;
   XYPart        xy;
} XYRec;

#endif

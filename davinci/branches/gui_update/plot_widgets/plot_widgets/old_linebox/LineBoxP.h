/*******************************************************************************
*									       *
* LineBoxP.h - 1-D Histogram Widget, Private Header File			       *
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
* Sep 24,1992								       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/
#ifndef LineBoxP_H
#define LineBoxP_H

#include "LineBox.h"
#include <Xm/XmP.h>
#if XmVersion >= 2000
#include <X11/CoreP.h>
#include <Xm/PrimitiveP.h>
#endif

typedef struct _LineBoxClassPart{
    int ignore;
} LineBoxClassPart;

typedef struct _LineBoxClassRec{
    CoreClassPart  core_class;
    XmPrimitiveClassPart primitive_class;
    LineBoxClassPart  lineBox_class;
} LineBoxClassRec;

extern LineBoxClassRec lineBoxClassRec;

typedef struct _LineBoxPart {
    GC gc;	   	       	/* graphics context for axes & labels */
    GC contentsGC;		/* graphics context for plot contents */
    Pixmap drawBuffer;		/* double buffering for non-flashing draws */
    int xMin, yMin, xMax, yMax;	/* boundaries of the drawable area of widget */
    XmFontList font;		/* motif font list associated with widget */
    XtCallbackList resize;	/* callbacks */
    XtCallbackList btn2;
    XtCallbackList btn3;
    XtCallbackList redisplay;
    Boolean doubleBuffer;	/* When set, draw first to offscreen pixmap */
    Boolean logScaling;		/* When set, plot Y axis as log of Y */
    Boolean binEdgeLabeling;	/* When set, try to label bin edges exactly */
    int barSepPercent;		/* Space between bars as % of bin width */
    XmString xAxisLabel;	/* Compound string labels for axes */
    XmString yAxisLabel;
    int xOrigin, yOrigin;	/* The point where the axis lines meet */
    int xEnd, yEnd;		/* The ends of the x and y axis lines */
    int axisLeft, axisTop;	/* Along with xOrigin and yOrigin, define */
    int axisBottom, axisRight;	/*    the boundaries of the axis areas	  */
    float *bins;		/* Local storage for histogram data */
    float *edges;		/* Bin edge data for adaptive histogram */
    int nBins;			/* Number of bins */
    float *uppErr,*lowErr;	/* Centers of error bars */
    int dragState;		/* Is the user currently dragging the mouse? */
    double xDragStart;		/* X (data coord) position of start of drag */
    double yDragStart;		/* Y (data coord) position of start of drag */
    double minXData, maxXData;	/* Minimum and maximum x data values */
    double minYData, maxYData;	/* Minimum and maximum y data values */
    double minXLim, maxXLim;	/* Min and max x data actually displayed */
    double minYLim, maxYLim;	/* Min and max y data actually displayed */
} LineBoxPart;

typedef struct _LineBoxRec {
   CorePart        core;
   XmPrimitivePart primitive;
   LineBoxPart        lineBox;
} LineBoxRec;

#endif

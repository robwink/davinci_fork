/*******************************************************************************
*									       *
* ScatP.h - Scatter Plot Widget, Private Header File			       *
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
#ifndef SCATP_H
#define SCATP_H

#include "Scat.h"
#include <Xm/XmP.h>
#if XmVersion >= 2000
#include <X11/CoreP.h>
#include <Xm/PrimitiveP.h>
#endif

typedef struct _ScatClassPart{
    int ignore;
} ScatClassPart;

typedef struct _ScatClassRec{
    CoreClassPart  core_class;
    XmPrimitiveClassPart primitive_class;
    ScatClassPart  scat_class;
} ScatClassRec;

extern ScatClassRec scatClassRec;

typedef struct _ScatPart {
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
    Boolean darkerPoints;	/* When set, draw points 2 pixels wide */
    Boolean xLogScaling;	/* When set, plot X axis as log of X */
    Boolean yLogScaling;	/* When set, plot Y axis as log of Y */
    XmString xAxisLabel;	/* Compound string labels for axes */
    XmString yAxisLabel;
    int xOrigin, yOrigin;	/* The point where the axis lines meet */
    int xEnd, yEnd;		/* The ends of the x and y axis lines */
    int axisLeft, axisTop;	/* Along with xOrigin and yOrigin, define */
    int axisBottom, axisRight;	/*    the boundaries of the axis areas	  */
    int dragState;		/* Is the user currently dragging the mouse? */
    double xDragStart;		/* X (data coord) position of start of drag */
    double yDragStart;		/* Y (data coord) position of start of drag */
    double minXData, maxXData;	/* Minimum and maximum x data values */
    double minYData, maxYData;	/* Minimum and maximum y data values */
    double minXLim, maxXLim;	/* Min and max x data actually displayed */
    double minYLim, maxYLim;	/* Min and max y data actually displayed */
    int isColor;		/* True when plot has more than one color */
    XImage *plotImage;		/* Image for dense plotting mode drawing */
    unsigned char *plotBits;	/* Bit array for plotImage above */
    ScatPoint *points;		/* Contents expressed as colored points */
    int nPoints;		/* Number of points */
} ScatPart;

typedef struct _ScatRec {
   CorePart        core;
   XmPrimitivePart primitive;
   ScatPart        scat;
} ScatRec;

#endif

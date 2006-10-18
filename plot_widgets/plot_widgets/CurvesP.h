/*******************************************************************************
*									       *
* CurvesP.h - Curves Plot Widget, Private Header File			       *
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
* Written by Baolin Ren							       *
*									       *
*******************************************************************************/
#ifndef CURVESP_H
#define CURVESP_H

#include "Curves.h"
#include <Xm/XmP.h>
#if XmVersion >= 2000
#include <X11/CoreP.h>
#include <Xm/PrimitiveP.h>
#endif

typedef struct _CurvesClassPart{
    int ignore;
} CurvesClassPart;

typedef struct _CurvesClassRec{
    CoreClassPart  core_class;
    XmPrimitiveClassPart primitive_class;
    CurvesClassPart  curves_class;
} CurvesClassRec;

extern CurvesClassRec curvesClassRec;

struct CurvesScaleFunction {
    int function_num;			/* scale function */
    double value;			/* used for add or mult */
    struct CurvesScaleFunction *next;	/* point to next struct */
};
typedef struct CurvesScaleFunction *ScalePtr;

typedef struct _CurvesPart {
    GC gc;	   	       	/* graphics context for axes & labels */
    GC contentsGC;		/* graphics context for plot contents */
    GC legendGC;                /* graphics context for legend area */
    GC linesGC[NUMLINE];        /* gc for individual line */
    GC legendLineGC[NUMLINE];   /* gc for individual legend line */
    GC shellGC;                 /* gc for set mark & line style dialog shell */
    Pixmap markTile[NUMMARK];   /* mark pixmap tile */
    Pixmap lineTile[NUMMARK];   /* line pixmap tile */
    Pixmap view_markTile[NUMMARK]; /* mark pixmap tile for settting shell */
    Pixmap drawBuffer;		/* double buffering for non-flashing draws */
    int xMin, yMin, xMax, yMax;	/* boundaries of the drawable area of widget */
    XmFontList font;		/* motif font list associated with widget */
    XtCallbackList resize;	/* callbacks */
    XtCallbackList btn2;
    XtCallbackList btn3;
    XtCallbackList redisplay;
    Boolean doubleBuffer;	/* When set, draw first to offscreen pixmap */
    Boolean xLogScaling;	/* When set, plot X axis as log of X */
    Boolean yLogScaling;	/* When set, plot Y axis as log of Y */
    XmString xAxisLabel;	/* Compound string labels for axes */
    XmString yAxisLabel;
    int xOrigin, yOrigin;	/* The point where the axis lines meet */
    int xEnd, yEnd;		/* The ends of the x and y axis lines */
    int axisLeft, axisTop;	/* Along with xOrigin and yOrigin, define */
    int axisBottom, axisRight;	/*    the boundaries of the axis areas	  */
    int dragState;		/* Is the user currently dragging the mouse? */
    int legendHeight;           /* used for drawing legend */
    int max_point;              /* maxmum number points of the data */
    int x_num_point;            /* number of points for x axis data */
    double xDragStart;		/* X (data coord) position of start of drag */
    double yDragStart;		/* Y (data coord) position of start of drag */
    double minXData, maxXData;	/* Minimum and maximum x data values */
    double minYData, maxYData;	/* Minimum and maximum y data values */
    double minXLim, maxXLim;	/* Min and max x data actually displayed */
    double minYLim, maxYLim;	/* Min and max y data actually displayed */
    CurveStruct *data;          /* single curve data structure */
    CurveStruct *store_data;    /* original copy of data */
    float *x_data;              /* x axis data for drawing */
    int num_variable;           /* number of variables */
    int current_x_axis;         /* current variavle number of x axis data */
    int current_variable;       /* variable for current setting */  
    int *index;                 /* used for sorting data */
    char *y_list;               /* store the y axis variables */
    Widget shell;               /* setting mark & line style widget */
    Widget sample[2];           /* widgets for drawing sample in shell widget */
    Widget markButton[NUMMARK]; /* widgets for mark button callbacks */
    Widget lineButton[NUMLINE]; /* widgets for line buttom callbacks */
    Boolean isShellSet;         /* when set there is already a shell exist */
    Dimension selectionFrameHeight; /* used to create sample widget */
    Dimension scalingFrameHeight; /* used to create scaling sample widget */
    int *tempMarkLine;          /* temppary storing for mark and lind number */
    int *storeMarkLine;         /* original copy of mark & line tile number */
    int *updateMarkLine;
    Boolean showLegend;         /* when set show the legend */
    Boolean isTimeSeries;       /* when set plot time series */
    Boolean isPixmapRegisted;   /* when set pixmaps have been registed */
    Boolean isScaleSet;         /* set if there is a scaling widget */
    Boolean isEmptyData;        /* set if the data is empty */
    Widget scale;               /* scale variables widget */
    GC scaleGC;                 /* gc for scale variable widget */
    Widget scaleFormular;       /* widget for drawing formula */
    Widget textArea[2];         /* widgets for input constants */
    int current_scaling;        /* current variable for scaling */
    ScalePtr *scaleFunction;    /* scaling functions */
    ScalePtr *tempFunction;     /* temp scaling functions */
    char **tempName;            /* temp scalied variable names */
    Boolean scaleValid;         /* set if scale function is valid */
} CurvesPart;

typedef struct _CurvesRec {
   CorePart        core;
   XmPrimitivePart primitive;
   CurvesPart      curves;
} CurvesRec;

#endif

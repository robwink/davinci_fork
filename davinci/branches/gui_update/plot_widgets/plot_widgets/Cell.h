/*******************************************************************************
*									       *
* Cell.h - Rectangular Cell Plot Widget, Public Header File	               *
*		Cloned from the Scatter Plot Widget.			       *
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

#ifndef  CELL_H
#define  CELL_H

enum CellRescaleModes {CELL_NO_RESCALE, CELL_RESCALE, CELL_RESCALE_AT_MAX,
	CELL_GROW_ONLY};

/* Resource strings */
#define XmNdoubleBuffer "doubleBuffer"
#define XmCDoubleBuffer "DoubleBuffer"
#ifndef XmNfontList
#define XmNfontList "fontList"
#define XmCFontList "FontList"
#endif
#ifndef XmNresizeCallback
#define XmNresizeCallback "resizeCallback"
#define XmCResizeCallback "ResizeCallback"
#endif
#define XmNbtn2Callback "btn2Callback"
#define XmCBtn2Callback "Btn2Callback"
#define XmNbtn3Callback "btn3Callback"
#define XmCBtn3Callback "Btn3Callback"
#define XmNxAxisLabel "xAxisLabel"
#define XmCXAxisLabel "XAxisLabel"
#define XmNyAxisLabel "yAxisLabel"
#define XmCYAxisLabel "YAxisLabel"
#define XmNredisplayCallback "redisplayCallback"
#define XmCRedisplayCallback "RedisplayCallback"

extern WidgetClass cellWidgetClass;

typedef struct _CellClassRec *CellWidgetClass;
typedef struct _CellRec *CellWidget;

typedef struct _CellRect {
    Pixel pixel;
    float x, y;
    float dx, dy;
} CellRect;

typedef struct {
    int     reason;
    XEvent *event;
} CellCallbackStruct;

void CellSetContents(Widget w, CellRect *points, int nPoints, int rescale);
void CellSetVisibleRange(Widget w, double minXLim, double minYLim,
			 double maxXLim, double maxYLim);
void CellGetVisibleRange(Widget w, double *minXLim, double *minYLim,
			 double *maxXLim, double *maxYLim);
void CellZoomOut(Widget w);
void CellZoomIn(Widget w);
void CellResetZoom(Widget w);
void CellPrintContents(Widget w, char *psFileName);
void CellWritePS(Widget w, FILE *fp);
#endif

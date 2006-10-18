/*******************************************************************************
*									       *
* Scat.h - Scatter Plot Widget, Public Header File			       *
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

#ifndef  SCAT_H
#define  SCAT_H

enum ScatRescaleModes {SCAT_NO_RESCALE, SCAT_RESCALE, SCAT_RESCALE_AT_MAX,
	SCAT_GROW_ONLY};

/* Resource strings */
#define XmNdoubleBuffer "doubleBuffer"
#define XmCDoubleBuffer "DoubleBuffer"
#define XmNdarkerPoints "darkerPoints"
#define XmCDarkerPoints "DarkerPoints"
#define XmNxLogScaling "xLogScaling"
#define XmCXLogScaling "XLogScaling"
#define XmNyLogScaling "yLogScaling"
#define XmCYLogScaling "YLogScaling"
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

extern WidgetClass scatWidgetClass;

typedef struct _ScatClassRec *ScatWidgetClass;
typedef struct _ScatRec *ScatWidget;

typedef struct _ScatPoint {
    Pixel pixel;
    float x, y;
} ScatPoint;

typedef struct {
    int     reason;
    XEvent *event;
} ScatCallbackStruct;

void ScatSetContents(Widget w, ScatPoint *points, int nPoints, int rescale);
void ScatSetVisibleRange(Widget w, double minXLim, double minYLim,
			 double maxXLim, double maxYLim);
void ScatGetVisibleRange(Widget w, double *minXLim, double *minYLim,
			 double *maxXLim, double *maxYLim);
void ScatZoomOut(Widget w);
void ScatZoomIn(Widget w);
void ScatResetZoom(Widget w);
void ScatPrintContents(Widget w, char *psFileName);
void ScatWritePS(Widget w, FILE *fp);
#endif

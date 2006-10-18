/*******************************************************************************
*									       *
* LineBox.h - 1-D Histogram Widget, Public Header File			       *
*									       *
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
* Sep 24, 1992								       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/

#ifndef  LineBox_H
#define  LineBox_H

enum LineBoxRescaleModes {LineBox_RESCALE, LineBox_NO_RESCALE, LineBox_RESCALE_AT_MAX,
	LineBox_GROW_ONLY, LineBox_REBIN_MODE};

/* Resource strings */
#define XmNdoubleBuffer "doubleBuffer"
#define XmCDoubleBuffer "DoubleBuffer"
#define XmNlogScaling "logScaling"
#define XmCLogScaling "LogScaling"
#define XmNbinEdgeLabeling "binEdgeLabeling"
#define XmCBinEdgeLabeling "BinEdgeLabeling"
#define XmNbarSeparation "barSeparation"
#define XmCBarSeparation "BarSeparation"
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

extern WidgetClass lineBoxWidgetClass;

typedef struct _LineBoxClassRec *LineBoxWidgetClass;
typedef struct _LineBoxRec *LineBoxWidget;

typedef struct {
    int     reason;
    XEvent *event;
} LineBoxCallbackStruct;

void LineBoxSetContents(Widget w, float xMin, float xMax, int nBins, float *bins,
	float *uppErr, float *lowErr, int rescale);
void LineBoxSetContentsAdaptive(Widget w, int nBins, float *bins, float *edges,
	int rescale);
void LineBoxSetVisibleRange(Widget w, double minXLim, double minYLim,
	double maxXLim, double maxYLim);
void LineBoxGetVisibleRange(Widget w, double *minXLim, double *minYLim,
	double *maxXLim, double *maxYLim);
void LineBoxZoomOut(Widget w);
void LineBoxZoomIn(Widget w);
void LineBoxResetZoom(Widget w);
void LineBoxPrintContents(Widget w, char *psFileName);
void LineBoxWritePS(Widget w, FILE *fp);
#endif

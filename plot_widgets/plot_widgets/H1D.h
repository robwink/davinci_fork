/*******************************************************************************
*									       *
* H1D.h - 1-D Histogram Widget, Public Header File			       *
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

#ifndef  H1D_H
#define  H1D_H

enum H1DRescaleModes {H1D_RESCALE, H1D_NO_RESCALE, H1D_RESCALE_AT_MAX,
	H1D_GROW_ONLY, H1D_REBIN_MODE};

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

extern WidgetClass h1DWidgetClass;

typedef struct _H1DClassRec *H1DWidgetClass;
typedef struct _H1DRec *H1DWidget;

typedef struct {
    int     reason;
    XEvent *event;
} H1DCallbackStruct;

void H1DSetContents(Widget w, float xMin, float xMax, int nBins, float *bins,
	float *uppErr, float *lowErr, int rescale);
void H1DSetContentsAdaptive(Widget w, int nBins, float *bins, float *edges,
	int rescale);
void H1DSetVisibleRange(Widget w, double minXLim, double minYLim,
	double maxXLim, double maxYLim);
void H1DGetVisibleRange(Widget w, double *minXLim, double *minYLim,
	double *maxXLim, double *maxYLim);
void H1DZoomOut(Widget w);
void H1DZoomIn(Widget w);
void H1DResetZoom(Widget w);
void H1DPrintContents(Widget w, char *psFileName);
void H1DWritePS(Widget w, FILE *fp);
#endif

/*******************************************************************************
*                                                                              *
* Curves.h - Curves Plot Widget, Public Header File                            *
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
#ifndef  CURVES_H
#define  CURVES_H

enum DrawingRescaleModes { CURVES_NO_RESCALE, CURVES_RESCALE,
	CURVES_RESCALE_AT_MAX};
enum LineStyles {PLAIN_LINE, LINE_2, LINE_3, LINE_4, LINE_5, LINE_6,
	BLANK_LINE};
enum MarkStyles {SQUARE_MARK, STAR_MARK, TRIANGLE_MARK, CIRCLE_MARK,
	SOLID_CIRCLE_MARK, CROSS_MARK, TOP_ERROR_MARK, BOTTOM_ERROR_MARK,
	BLANK_MARK};
enum CurveOptions {CURVE_NO_OPTIONS, CURVE_TOP_ERROR, CURVE_BOTTOM_ERROR};

/* Resource strings */
#define XmNdoubleBuffer "doubleBuffer"
#define XmCDoubleBuffer "DoubleBuffer"
#define XmNxLogScaling "xLogScaling"
#define XmCXLogScaling "XLogScaling"
#define XmNyLogScaling "yLogScaling"
#define XmCYLogScaling "YLogScaling"
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

#define NUMMARK 7
#define NUMLINE 7

extern WidgetClass curvesWidgetClass;

typedef struct _CurvesClassRec *CurvesWidgetClass;
typedef struct _CurvesRec *CurvesWidget;

typedef struct {
     char *variable_name;      
     int num_point;             /* number of points for the variable */
     int mark_num;              /* mark tile number */
     int line_num;              /* line tile number */
     int options;		/* options: none, is top error, bottom error */
     float *points;             /* data */
} CurveStruct;

typedef struct {
     int reason;
     XEvent *event;
} CurvesCallbackStruct;

void CurvesSetContents(Widget w, CurveStruct *curves, int nCurves,
                 int rescale, Boolean isTimeSeries);
void CurvesSetVisibleRange(Widget w, double minXLim, double minYLim,
			 double maxXLim, double maxYLim);
void CurvesGetVisibleRange(Widget w, double *minXLim, double *minYLim,
			 double *maxXLim, double *maxYLim);
void CurvesZoomOut(Widget w);
void CurvesZoomIn(Widget w);
void CurvesResetZoom(Widget w);
void CurvesScaleVariables(Widget w);
void CurvesSetMarkLineStyle(Widget w);
void CurvesShowLegend(Widget w, Boolean showLegend);
void CurvesPrintContents(Widget w, char *psFileName);
void CurvesSetCurveStyle(Widget w, int index, int markNum, int lineNum);
#endif

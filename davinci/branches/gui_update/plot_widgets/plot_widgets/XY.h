/*******************************************************************************
*									       *
* XY.h - General Purpose Plot Widget, Public Header File		       *
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

#ifndef  XY_H
#define  XY_H

enum XYRescaleModes {XY_NO_RESCALE, XY_RESCALE, XY_RESCALE_AT_MAX,
	XY_GROW_ONLY, XY_REBIN_MODE};
#define XY_N_MARK_STYLES 10
enum XYMarkStyles {XY_NO_MARK, XY_SQUARE_MARK, XY_CIRCLE_MARK, XY_STAR_MARK,
	XY_X_MARK, XY_TRIANGLE_MARK, XY_SOLID_SQUARE_MARK, XY_SOLID_CIRCLE_MARK,
	XY_THICK_SQUARE_MARK, XY_THICK_CIRCLE_MARK};
#define XY_N_MARK_SIZES 4
enum XYMarkSizes {XY_TINY, XY_SMALL, XY_MEDIUM, XY_LARGE};
#define XY_N_LINE_STYLES 13
enum XYLineStyles {XY_NO_LINE, XY_PLAIN_LINE, XY_FINE_DASH, XY_MED_FINE_DASH,
	XY_DASH, XY_LONG_DASH, XY_X_LONG_DASH, XY_1_DOT_DASH, XY_2_DOT_DASH,
	XY_3_DOT_DASH, XY_4_DOT_DASH, XY_THICK_LINE, XY_X_THICK_LINE};

/* Histogram fill patterns */
#define XY_NO_FILL 0
#define XY_SOLID_FILL	         0x000000ff
#define XY_FINE_HORIZ_FILL       0x01008004
#define XY_COARSE_HORIZ_FILL     0x0100800b
#define XY_FINE_VERT_FILL        0x01800004
#define XY_COARSE_VERT_FILL      0x0180000b
#define XY_FINE_GRID_FILL        0x01808005
#define XY_COARSE_GRID_FILL      0x01808010
#define XY_FINE_X_FILL           0x01c04006
#define XY_COARSE_X_FILL         0x01c04010
#define XY_FINE_45DEG_FILL       0x01004005
#define XY_MED_45DEG_FILL        0x01004007
#define XY_COARSE_45DEG_FILL     0x01004010
#define XY_FINE_30DEG_FILL       0x01005506
#define XY_COARSE_30DEG_FILL     0x01005510
#define XY_FINE_60DEG_FILL       0x01550006
#define XY_COARSE_60DEG_FILL     0x01550010
#define XY_R_FINE_45DEG_FILL     0x0100c005
#define XY_R_MED_45DEG_FILL      0x0100c007
#define XY_R_COARSE_45DEG_FILL   0x0100c010
#define XY_R_FINE_30DEG_FILL     0x0100a506
#define XY_R_COARSE_30DEG_FILL   0x0100a510
#define XY_R_FINE_60DEG_FILL     0x01a50006
#define XY_R_COARSE_60DEG_FILL   0x01a50010
#define XY_L_FINE_HORIZ_FILL     0x02008004
#define XY_L_COARSE_HORIZ_FILL   0x0200800b
#define XY_L_FINE_VERT_FILL      0x02800004
#define XY_L_COARSE_VERT_FILL    0x0280000b
#define XY_L_FINE_GRID_FILL      0x02808005
#define XY_L_COARSE_GRID_FILL    0x02808010
#define XY_L_FINE_X_FILL         0x02c04006
#define XY_L_COARSE_X_FILL       0x02c04010
#define XY_L_FINE_45DEG_FILL     0x02004005
#define XY_L_MED_45DEG_FILL      0x02004007
#define XY_L_COARSE_45DEG_FILL   0x02004010
#define XY_L_FINE_30DEG_FILL     0x02005506
#define XY_L_COARSE_30DEG_FILL   0x02005510
#define XY_L_FINE_60DEG_FILL     0x02550006
#define XY_L_COARSE_60DEG_FILL   0x02550010

/* Additional histogram "line" styles, which make histogram appear as
   markers instead of lines */
#define XY_N_HIST_LINE_STYLES (XY_N_LINE_STYLES + 5)
enum XYHistLineStyles {XY_HIST_SQUARE_MARK=XY_N_LINE_STYLES,
    	XY_HIST_CIRCLE_MARK, XY_HIST_X_MARK, XY_HIST_SOLID_SQUARE_MARK,
    	XY_HIST_SOLID_CIRCLE_MARK};

enum XYStringAlignments {XY_LEFT, XY_CENTER, XY_RIGHT};
#define XY_N_COLORS 23

/* Resource strings */
#define XmNdoubleBuffer "doubleBuffer"
#define XmCDoubleBuffer "DoubleBuffer"
#define XmNshowLegend "showLegend"
#define XmCShowLegend "ShowLegend"
#define XmNxLogScaling "xLogScaling"
#define XmCXLogScaling "XLogScaling"
#define XmNyLogScaling "yLogScaling"
#define XmCYLogScaling "YLogScaling"
#define XmNmarginPercent "marginPercent"
#define XmCMarginPercent "MarginPercent"
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

extern WidgetClass xyWidgetClass;

typedef struct _XYClassRec *XYWidgetClass;
typedef struct _XYRec *XYWidget;

/* Constants for data to window coordinate transformation */
typedef struct {
    char xLogScaling, yLogScaling;
    double minXData, minYData, minXLim, minYLim;
    double xScale, yScale, minXPix, maxYPix;
    int xOrigin, yOrigin;
} XYTransform;

typedef struct {
    int reason;
    XEvent *event;
    XYTransform *xform;
} XYCallbackStruct;

typedef struct _XYPoint {
    float x, y;
} XYPoint;

typedef struct _XYErrorBar {
    float min, max;
} XYErrorBar;

typedef struct _XYString {
    float x, y;
    XmFontList font;
    Pixel color;
    int alignment;
    XmString string;
} XYString;

typedef struct _XYCurve {
    XmString name;
    char markerStyle;
    char markerSize;
    char lineStyle;
    Pixel markerPixel;
    Pixel linePixel;
    int nPoints;
    XYPoint *points;
    XYErrorBar *horizBars;
    XYErrorBar *vertBars;
} XYCurve;

typedef struct _XYHistogram {
    XmString name;
    char lineStyle;
    unsigned fillStyle;
    Pixel linePixel;
    Pixel fillPixel;
    float xMin, xMax;
    int nBins;
    float *bins;
    float *edges;
    float barSeparation;
    XYErrorBar *errorBars;
} XYHistogram;

void XYSetCurves(Widget w, XYCurve *curves, int nCurves, int rescaleMode,
    	int redisplay);
void XYSetHistograms(Widget w, XYHistogram *histograms, int nHistograms,
    	int rescaleMode, int redisplay);
void XYUpdateCurveStyles(Widget w, XYCurve *curves, int redisplay);
void XYUpdateHistogramStyles(Widget w, XYHistogram *histograms, int redisplay);
void XYUpdateCurveData(Widget w, XYCurve *curves, int rescaleMode,
    	int redisplay);
void XYUpdateHistogramData(Widget w, XYHistogram *histograms, int rescaleMode,
    	int redisplay);
void XYSetStrings(Widget w, XYString *strings, int nStrings);
void XYSetVisibleRange(Widget w, double minXLim, double minYLim,
	double maxXLim, double maxYLim);
void XYGetVisibleRange(Widget w, double *minXLim, double *minYLim,
	double *maxXLim, double *maxYLim);
void XYZoomOut(Widget w);
void XYZoomIn(Widget w);
void XYResetZoom(Widget w);
void XYPrintContents(Widget w, char *psFileName);
void XYWritePS(Widget w, FILE *fp);
void XYDrawMarker(Display *display, Drawable drawBuf, GC gc, int size,
	int style, Pixel color, int x, int y);
void XYDrawHistMarker(Display *display, Drawable drawBuf, GC gc,
	int style, Pixel color, int x, int y);
void XYDrawLine(Display *display, Drawable drawBuf, GC gc, int style,
	Pixel color, int x1, int y1, int x2, int y2);
void XYDrawFill(Display *display, Drawable drawBuf, GC gc,
	unsigned style, Pixel color, int xMin, int yMin, int xMax, int yMax);
double XYDataToWindowX(XYTransform *xform, double value);
double XYDataToWindowY(XYTransform *xform, double value);
double XYWindowToDataX(XYTransform *xform, double value);
double XYWindowToDataY(XYTransform *xform, double value);
void ComputeTransform(XYWidget w, XYTransform *xform);

/* Obsolete routines, for compatability with v1.0 */
void XYSetContents(Widget w, XYCurve *curves, int nCurves, int rescaleMode);
void XYUpdateStyles(Widget w, XYCurve *curves);
void XYUpdateData(Widget w, XYCurve *curves, int rescaleMode);
#endif

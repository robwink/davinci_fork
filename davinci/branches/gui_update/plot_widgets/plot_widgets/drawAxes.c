/*******************************************************************************
*									       *
* drawAxes.c -- Generic axis drawing routines for 2D graphs & plots	       *
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
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <float.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include "../util/psUtils.h"
#include "drawAxes.h"

/* prototypes of system routines missing from include files */
#ifdef VMS
#include "../util/vmsUtils.h"
#else
double rint(double);
#endif /*VMS*/

#define SHORT_TIC_LEN 4		/* length in pixels for short axis tic marks */
#define MED_TIC_LEN 6		/* length in pixels for medium axis tic marks */
#define LONG_TIC_LEN 8		/* length in pixels for long axis tic marks.
				   if too short, large fonts may collide at
				   the intersection of the v and h axes */
#define MIN_TIC_SPACING 4	/* minimum interval in pixels for axis tics */
#define MIN_LOG_TIC_SPACING 2	/* min spacing for axis tics on log plots */
#define BIN_TIC_THRESHOLD 4	/* minimum spacing of tics labeling bin edges
				   before ignoring bin edges in choosing tics */ 
#define MIN_V_LABEL_SPACING 1	/* vertical space to leave between labels in
				   (font dep.) units of character height */
#define MIN_V_LOG_LABEL_SPACING .5 /* "" for log plots */
#define MIN_H_LABEL_SPACING 1	/* horizontal space to leave between labels
				   in units of maximum character width */
#define MIN_H_LOG_LABEL_SPACING .5 /* "" for log plots */

/* table of multiples of 10, 2, and 5 for reference in figuring label spacing */
#define NMULTIPLIERS 15
static int Multipliers[NMULTIPLIERS] = {
    1, 2, 5, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000};

static void drawLinearVAxis(Display *display, Drawable window, GC gc,
	XFontStruct *fs, int outDevice, int x, int minX, int y1, int y2,
	double minData, double maxData, double minLimit, double maxLimit);  
static void drawLogVAxis(Display *display, Drawable window, GC gc,
	XFontStruct *fs, int outDevice, int x, int minX, int y1, int y2,
	double minData, double maxData, double minLimit, double maxLimit);  
static void drawLinearHAxis(Display *display, Drawable window, GC gc,
	XFontStruct *fs, int outDevice, int y, int x1, int x2,
	double minData, double maxData, double minLimit, double maxLimit);
static void drawDiscreteHAxis(Display *display, Drawable window, GC gc,
	XFontStruct *fs, int outDevice, int y, int x1, int x2, double minData,
	double maxData, double minLimit, double maxLimit, int nBins);
static void drawLogHAxis(Display *display, Drawable window, GC gc,
	XFontStruct *fs, int outDevice, int y, int x1, int x2,
	double minData, double maxData, double minLimit, double maxLimit);
static int calcDecimalOffset(char *labelStr, XFontStruct *fs);
static double calcLinTicInterval(double labelInterval, double scale, int factor);
static double calcLogTicInterval(int pixRange, double labelInterval, double min,
	double max, int factor);
static double calcTicInterval(double labelInterval, double minTicInterval,
	int factor);
static double calcLabelInterval(int pixRange, double dataRange, XFontStruct *fs,
 	int *factor);
static double calcLogLabelInterval(int pixRange, double min, double max,
	XFontStruct *fs, int *factor);
static double calcMinLogInterval(int pixRange, double min, double max,
	double narrowSpacing, double wideSpacing, int *factor);
static void drawHCenteredFloat(Display *display, Drawable window, GC gc,
	XFontStruct *fs, int outDevice, double value, int x, int y);
static void drawVCenteredFloat(Display *display, Drawable window, GC gc,
	XFontStruct *fs, int outDevice, double value, int x, int y,
	int maxWidth, int maxDecimalOffset);
static void measureHCenteredFloat(XFontStruct *fs, double value,
	int x, int *left, int *right);
static void genHAxisLineSegs(XSegment **segPtr, int *segCount, int x1, int x2,
	int y, int drawAxisLine, int drawLeftArrow, int drawRightArrow);
static void genVAxisLineSegs(XSegment **segPtr, int *segCount, int x, int y1,
	int y2, int drawAxisLine, int drawTopArrow, int drawBottomArrow);
static void fitDouble(char *string, double number, int maxWidth,
	XFontStruct *fs);

/*
** A note about rounding errors:
**
** Because this code deals with both an integer coordinate system and
** the floating point coordinate system of the data, it is sensitive
** to rounding errors.  Be careful when you make changes.  Think about
** how the data will be quantized.  Will precision be lost?  Will there
** be biases?
*/

/*
** HAxisHeight HAxisEndClearance VAxisWidth VAxisEndClearance
**
** Determine amount of space required to draw axes.  The calls which actually
** draw the axes, DrawHorizontalAxis and DrawVerticalAxis, draw from the
** desired coordinates of the axis lines, but use space around the lines
** for tic marks and scale text.  Use these routines to find out how much
** space the axis drawing routines will need.  The vertical axis drawing
** routines can draw in less space than VAxisWidth returns, but precision
** may be lost.
*/
int HAxisHeight(XFontStruct *fs)
{
    /* allow space for label below long tic separated by 2 pixels */
    return fs->ascent + LONG_TIC_LEN + 2;
}
int HAxisEndClearance(XFontStruct *fs)
{
    /* allow space for half of an 10 digit number to stick off end of axis */
    return XTextWidth(fs, "1234567890", 10) / 2;
}
int VAxisWidth(XFontStruct *fs)
{
    /* allow space for 10 character labels + axis tics (may have to change) */
    return XTextWidth(fs, "-1.23e-456", 10) + LONG_TIC_LEN + 1;
}
int VAxisEndClearance(XFontStruct *fs)
{
    /* allow space for half of a label to protrude above or below axis line */
    return fs->ascent / 2;
}

/*
** DrawHorizontalAxis
**
** Draw a horizontal axis for a plot or histogram.  For histograms, providing
** the routine with the number of bins in the histogram allows it to choose
** between labeling bin edges precisely, and drawing a general purpose 
** horizontal axis ignoring bin edges.  Specify the position of the axis
** by the x and y window coordinates where the actual axis line should be
** drawn.  Use HAxisHeight and HAxisEndClearance to find the amount of space
** that this routine will need to accomodate the tics and labels underneath
** and to either side of the axis line.
**
** Parameters
**
**	display		X display to draw to.
**	window		X window or pixmap to draw into.
**	gc		X graphics context to use in drawing labels and tics
**	fs		X font structure for font measurements.  Even though
**			gc specifies the font to be used to draw the string,
**			the caller has to supply this argument with the
**			XFontStruct corresponding to the font in the gc.
**			This avoids round trip overhead of querying the
**			server for the font data
**	outDevice	Output device, X_SCREEN, or PS_PRINTER
**	y, x1, and, x2	Where to draw the axis line itself
**			in window coordinates.
**	minData,	Horizontal range of the data to be plotted, even
**	    maxData	if some of it is outside the range displayed.
**	minLimit,	Limits in data coordinates on the data displayed.
**	    maxLimit	The part of the axis outside of minLimit and maxLimit
**			is not drawn, but arrows indicate that some data is
**			not shown.
**	logScaling	Use log scaling, if false, use linear scaling.
**	nBins		For histograms, the number of bins (for labeling
**			bin edges).  Pass nBins = 0 for a continuous axis.
*/
void DrawHorizontalAxis(Display *display, Drawable window, GC gc,
	XFontStruct *fs, int outDevice, int y, int x1, int x2,
	double minData, double maxData, double minLimit, double maxLimit,
	int logScaling, int nBins)
{
    double binWidth, nVisibleBins, nPixelsPerBin;
    
    /* reject dangerous values here (don't draw axis) rather than cause error */
    if (x2 <= x1 || maxData <= minData || maxLimit <= minLimit)
    	return;

    /* If log scaling is specified, use log axis routine regardless of
       whether the caller wanted to draw a discrete axis */
    if (logScaling) {
    	drawLogHAxis(display, window, gc, fs, outDevice, y, x1, x2,
    		minData, maxData, minLimit, maxLimit);
    	return;
    }
    
    /* Draw a discrete (edge labeled) axis if one was requested and there
       are enough pixels per bin to make it worthwhile */
    if (nBins != 0) {
    	binWidth = (maxData - minData) / nBins;
    	nVisibleBins = (maxLimit - minLimit) / binWidth;
    	nPixelsPerBin = (x2 - x1) / nVisibleBins;
    	if (nPixelsPerBin > BIN_TIC_THRESHOLD) {
    	    drawDiscreteHAxis(display, window, gc, fs, outDevice, y, x1, x2,
    		    minData, maxData, minLimit, maxLimit, nBins);
    	    return;
    	}
    }
    	  
    /* Otherwise, draw a plain old linear axis */
    drawLinearHAxis(display, window, gc, fs, outDevice, y, x1, x2,
    	minData, maxData, minLimit, maxLimit);
}

/*
** DrawVerticalAxis
**
** Draw a vertical axis for a plot or histogram.  Specify the position of
** the axis by the x and y window coordinates where the actual axis line should be
** drawn.  Use VAxisWidth and VAxisEndClearance to find the amount of space
** that this routine will need to accomodate the tics and labels to the left
** and above and below the axis line.
**
** Parameters
**
**	display		X display to draw to.
**	window		X window or pixmap to draw into.
**	gc		X graphics context to use in drawing labels and tics
**	fs		X font structure for font measurements.  Even though
**			gc specifies the font to be used to draw the string,
**			the caller has to supply this argument with the
**			XFontStruct corresponding to the font in the gc.
**			This avoids round trip overhead of querying the
**			server for the font data
**	outDevice	Output device, X_SCREEN, or PS_PRINTER
**	x, y1, and, y	Where to draw the axis line itself
**			in window coordinates.
**	minX		Clipping boundary.  Don't allow labels extend to the
**			left of this point.
**	minData,	Vertical range of the data to be plotted, even
**	    maxData	if some of it is outside the range displayed.
**	minLimit,	Limits in data coordinates on the data displayed.
**	    maxLimit	The part of the axis outside of minLimit and maxLimit
**			is not drawn, but arrows indicate that some data is
**			not shown.
*/
void DrawVerticalAxis(Display *display, Drawable window, GC gc, XFontStruct *fs,
	int outDevice, int x, int minX, int y1, int y2, double minData,
	double maxData, double minLimit, double maxLimit, int logScaling)
{
    int temp;
    
    /* reject bad data before it causes an error (just don't draw axis) */
    if (y1==y2 || maxData<=minData || maxLimit<=minLimit)
    	return;
    
    /* put y1 & y2 in a known order (y1 = top coord, y2 = bottom coord) */
    if (y2 < y1) {
    	temp = y2; y2 = y1; y1 = temp;
    }

    if (logScaling)
    	drawLogVAxis(display, window, gc, fs, outDevice, x, minX, y1, y2,
    		minData, maxData, minLimit, maxLimit);
    else
    	drawLinearVAxis(display, window, gc, fs, outDevice, x, minX, y1, y2,
    		minData, maxData, minLimit, maxLimit);
}

static void drawLinearVAxis(Display *display, Drawable window, GC gc,
	XFontStruct *fs, int outDevice, int x, int minX, int y1, int y2,
	double minData, double maxData, double minLimit, double maxLimit)    
{
    double dataRange = maxLimit - minLimit;
    int pixRange = y2 - y1;
    int maxDecimalOffset = 0;
    int maxLabelWidth = (x - minX) - (LONG_TIC_LEN + 1);
    char labelStr[16];
    double labelValue, label1Value, ticValue, tic1Value;
    double interval, ticInterval;
    double scale, maxTics;
    double ticY;
    int nSegs, nFSegs, maxSegs, factor, decimalOffset;
    int maxLabels, labelCenter;
    int ticLen, i, ticsBetweenLabels;
    XSegment *seg, *segs;
    FloatSegment *fSeg, *fSegs;
    
    /* calculate a scale to convert data coordinates to pixel coordinates */
    scale = pixRange/dataRange;

    /* determine how far apart to draw the labels and tics */
    interval = calcLabelInterval(pixRange, dataRange, fs, &factor);  
    ticInterval = calcLinTicInterval(interval, scale, factor);

    /* start the first label at near minLimit rounded to multiples of interval,
       start the first tic one label interval before first label */
    label1Value = rint(minLimit / interval) * interval;
    tic1Value = label1Value - interval;
    
    /* measure the length of text after the decimal point for all labels so
       they can aligned, but not padded & will work with proportional fonts */
    maxLabels = (int)(dataRange/interval) + 2;
    for (i=0; i<=maxLabels; i++) {
    	labelValue = label1Value + interval*i;
    	fitDouble(labelStr, labelValue, maxLabelWidth, fs);
    	decimalOffset = calcDecimalOffset(labelStr, fs);
    	if (decimalOffset > maxDecimalOffset)
    	    maxDecimalOffset = decimalOffset;
    }

    /* draw the labels using the interval, starting value,
       and string offsets calculated above */
    for (i=0; i<=maxLabels; i++) {
    	labelValue = label1Value + interval*i;
    	labelCenter = rint(y2 - (labelValue - minLimit) * scale);
    	if (labelCenter >= y1 && labelCenter <= y2) {
    	    drawVCenteredFloat(display, window, gc, fs, outDevice, labelValue,
    	    	   x - LONG_TIC_LEN - 1, labelCenter, maxLabelWidth,
    	    	   maxDecimalOffset);
    	}
    }

    /* allocate enough memory for X segments to draw axis lines and
       tics at minimum tic spacing: Maximum # of tics + last tic +
       axis line + 0-2 arrows of 2 segments each */
    maxSegs = (y2 - y1)/MIN_TIC_SPACING + 10;
    if (outDevice == X_SCREEN) {
    	segs = (XSegment *)XtMalloc(sizeof(XSegment) * maxSegs);
    	seg = segs;
    } else {
    	segs = (XSegment *)XtMalloc(sizeof(XSegment) * 5);
    	seg = segs;
    	fSegs = (FloatSegment *)XtMalloc(sizeof(FloatSegment) * maxSegs);
    	fSeg = fSegs;
    }
    nSegs = 0;
    nFSegs = 0;
    
    /* draw the tics using interval, ticInterval, and tic1Value from above */
    ticsBetweenLabels = interval/ticInterval;
    maxTics = dataRange/ticInterval + ticsBetweenLabels + 2;
    for (i=0; i<=maxTics; i++) {
    	ticValue = tic1Value + ticInterval * i;
    	ticY = y2 - (ticValue - minLimit) * scale;
    	if (ticY >= y1 && ticY <= y2) {
    	    if (i % ticsBetweenLabels == 0)
    		ticLen = LONG_TIC_LEN;
    	    else if ((i*2) % ticsBetweenLabels == 0)
    		ticLen = MED_TIC_LEN;
    	    else
    		ticLen = SHORT_TIC_LEN;
    	    if (outDevice == X_SCREEN) {
    	    	seg->x1=x; seg->x2=x-ticLen; seg->y1=seg->y2=rint(ticY);
    	    	seg++; nSegs++;
    	    } else {
    	    	fSeg->x1=x; fSeg->x2=x-ticLen; fSeg->y1=fSeg->y2=ticY;
    	    	fSeg++; nFSegs++;
    	    }
    	}
    }

    /* generate axis line and arrow heads to indicate data not shown */
    genVAxisLineSegs(&seg, &nSegs, x, y1, y2, True,
    		     maxData > maxLimit, minData < minLimit);
	
    /* draw the axis and tic lines */
    if (outDevice == X_SCREEN) {
    	XDrawSegments(display, window, gc, segs, nSegs);
    } else { /* PS_PRINTER */
    	PSFloatDrawSegments(display, window, gc, fSegs, nFSegs);
    	PSDrawSegments(display, window, gc, segs, nSegs);
    	XtFree((char *)fSegs);
    }
    XtFree((char *)segs);
}

static void drawLogVAxis(Display *display, Drawable window, GC gc,
	XFontStruct *fs, int outDevice, int x, int minX, int y1, int y2,
	double minData, double maxData, double minLimit, double maxLimit)    
{
    double logMinLimit, logMaxLimit;
    char labelStr[16];
    double label1Value, labelValue, logLabelValue, labelLimit;
    double powLabelValue, logPowLabelValue, logPowLabel1Value;
    double ticValue, tic1Value, ticLimit, ticY;
    double powTicValue, logPowTicValue, logPowTic1Value;
    double logPowInterval, logPowTicInterval;
    double interval, scaledInterval, ticInterval, scaledTicInterval;
    double logScale;
    int nSegs, nFSegs, maxSegs, factor, powFactor;
    int maxPowLabels, maxPowTics, labelCenter, powLabelCenter;
    int ticLen, i, j, ticCount, ticsBetweenLabels, powTicsBetweenLabels;
    int decimalOffset, maxDecimalOffset = 0;
    int maxLabelWidth = (x - minX) - (LONG_TIC_LEN + 1);
    XSegment *seg, *segs;
    FloatSegment *fSeg, *fSegs;
    
    /* reject dangerous values */
    if (minLimit <= 0. || (maxLimit-minLimit)/minLimit < 1e-6)
    	return;
    
    /* minData may be <= zero, but treat it as DBL_MIN if so */
    if (minData <= 0.)
    	minData = DBL_MIN;
    
    logMinLimit = log10(minLimit); logMaxLimit = log10(maxLimit);
    
    /* calculate a scale to convert log data coordinates to pixel coordinates */
    logScale = (y2-y1)/(logMaxLimit-logMinLimit);
    
    /* determine spacing for power of 10 labels and tics.  Use linear
       spacing algorithms, but limit to no finer than powers of 10 */
    logPowInterval = calcLabelInterval(y2-y1, logMaxLimit-logMinLimit, fs,
    				    &powFactor);
    if (logPowInterval < 1) {
    	logPowInterval = 1;
    	powFactor = 0;
    }
    logPowTicInterval = calcLinTicInterval(logPowInterval, logScale, powFactor);
    if (logPowTicInterval < 1)
    	logPowTicInterval = 1;
   
    /* determine spacing between power of 10 labels */
    interval = calcLogLabelInterval(y2-y1, minLimit, maxLimit, fs, &factor);
    ticInterval = calcLogTicInterval(y2-y1, interval, minLimit, maxLimit,
    				     factor);

    /* loop over each power of 10 in the axis range */
    maxPowLabels = (int)((logMaxLimit-logMinLimit)/logPowInterval) + 2;
    logPowLabel1Value = floor(logMinLimit / logPowInterval) * logPowInterval;
    for (i=0; i<=maxPowLabels; i++) {
    	logPowLabelValue = logPowLabel1Value + logPowInterval*i;
    	powLabelValue = pow(10., logPowLabelValue);

    	/* calculate the spacing of labels within the power-of-10 intervals */
	if (interval > 1.) {
	    /* if labels are at intervals of more than one, the log effect
	       strongly compresses the top of the interval, the values also
	       start at 1 rather than zero, so we just put labels at 2 and 5 */
	    scaledInterval = 3. * powLabelValue;
	    labelLimit = 6. * powLabelValue;
	    label1Value = 2. * powLabelValue;
	} else {
	    scaledInterval = interval * powLabelValue;
	    labelLimit = 10. * powLabelValue - scaledInterval/2;
	    if (minLimit > powLabelValue)
		label1Value = rint(minLimit/scaledInterval) * scaledInterval;
	    else
		label1Value = powLabelValue + scaledInterval;
	}
    	
   	/* measure the length of text after the decimal point for all labels */
    	fitDouble(labelStr, powLabelValue, maxLabelWidth, fs);
    	maxDecimalOffset = calcDecimalOffset(labelStr, fs);
	labelValue = label1Value;
	for (j=1; labelValue < labelLimit; j++) {
	    logLabelValue = log10(labelValue);
	    labelCenter = (int)rint(y2 - (logLabelValue-logMinLimit)*logScale);
	    if (labelCenter < y1)
	    	break;
   	    fitDouble(labelStr, labelValue, maxLabelWidth, fs);
    	    decimalOffset = calcDecimalOffset(labelStr, fs);
    	    if (decimalOffset > maxDecimalOffset)
    		maxDecimalOffset = decimalOffset;
	    labelValue = label1Value + j*scaledInterval;
	}
	
    	/* draw the power-of 10 label  */
    	powLabelCenter = rint(y2 - (logPowLabelValue - logMinLimit) * logScale);
    	if (powLabelCenter >= y1 && powLabelCenter <= y2)
    	    drawVCenteredFloat(display, window, gc, fs, outDevice,
    	    	    powLabelValue, x - LONG_TIC_LEN - 1, powLabelCenter,
    	    	    maxLabelWidth, maxDecimalOffset);
    	
    	/* draw the labels within the power-of-10 intervals */
	if (interval > 2.)
	    continue;
	labelValue = label1Value;
	for (j=1; labelValue < labelLimit; j++) {
	    logLabelValue = log10(labelValue);
	    labelCenter = (int)rint(y2 - (logLabelValue-logMinLimit)*logScale);
	    if (labelCenter < y1)
	    	break;
	    if (labelCenter <= y2)
    	    	drawVCenteredFloat(display, window, gc, fs, outDevice,
    	    		labelValue, x - LONG_TIC_LEN - 1, labelCenter,
    	    		maxLabelWidth, maxDecimalOffset);
	    labelValue = label1Value + j*scaledInterval;
    	}
    }

    /* allocate enough memory for X segments to draw axis lines and
       tics at minimum tic spacing: Maximum # of tics + last tic +
       axis line + 0-2 arrows of 2 segments each */
    maxSegs = (y2 - y1)/MIN_LOG_TIC_SPACING + 10;
    if (outDevice == X_SCREEN) {
    	segs = (XSegment *)XtMalloc(sizeof(XSegment) * maxSegs);
    	seg = segs;
    } else {
    	segs = (XSegment *)XtMalloc(sizeof(XSegment) * 5);
    	seg = segs;
    	fSegs = (FloatSegment *)XtMalloc(sizeof(FloatSegment) * maxSegs);
    	fSeg = fSegs;
    }
    nSegs = 0;
    nFSegs = 0;
    
    /* draw the tics */
    logPowTic1Value = logPowLabel1Value - logPowInterval;
    powTicsBetweenLabels = logPowInterval/logPowTicInterval;
    maxPowTics = (logMaxLimit - logMinLimit)/logPowTicInterval +
    	    powTicsBetweenLabels + 4;
    for (i=0; i<=maxPowTics; i++) {
    	logPowTicValue = logPowTic1Value + logPowTicInterval * i;
    	powTicValue = pow(10., logPowTicValue);
    	ticY = y2 - (logPowTicValue - logMinLimit) * logScale;
    	if (ticY >= y1 && ticY <= y2) {
    	    if (i % powTicsBetweenLabels == 0)
    		ticLen = LONG_TIC_LEN;
    	    else if ((i*2) % powTicsBetweenLabels == 0)
    		ticLen = MED_TIC_LEN;
    	    else
    		ticLen = SHORT_TIC_LEN;
    	    if (outDevice == X_SCREEN) {
    	    	seg->x1=x; seg->x2=x-ticLen; seg->y1=seg->y2=rint(ticY);
    	    	seg++; nSegs++;
    	    } else {
    	    	fSeg->x1=x; fSeg->x2=x-ticLen; fSeg->y1=fSeg->y2=ticY;
    	    	fSeg++; nFSegs++;
    	    }
    	}
    	/* draw the tics within the power-of-10 intervals */
	if (ticInterval > 1.)
	    continue;
	scaledTicInterval = ticInterval * powTicValue;
	ticsBetweenLabels = interval/ticInterval;
	ticLimit = pow(10., floor(logPowTicValue+1.)) - scaledTicInterval/2;
	if (minLimit > powTicValue)
	    tic1Value = floor(minLimit/(scaledTicInterval*10)) *
			     scaledTicInterval*10 + scaledTicInterval;
	else
	    tic1Value = powTicValue + scaledTicInterval;
	ticValue = tic1Value;
	for (j=1; ticValue < ticLimit; j++) {
	    ticValue = tic1Value + (j-1)*scaledTicInterval;
	    ticY = y2 - (log10(ticValue)-logMinLimit)*logScale;
	    ticCount = (minLimit<=powTicValue && ticInterval>=1.) ? j+1 : j;
	    if (ticY < y1)
	    	break;
	    if (ticY > y2)
	    	continue;
    	    if (interval>1. && interval<=2. && (ticCount==2 || ticCount==5))
    		ticLen = LONG_TIC_LEN;
    	    else if (interval>1. && ticCount==5)
    		ticLen = MED_TIC_LEN;
    	    else if (interval>1.)
    		ticLen = SHORT_TIC_LEN;
    	    else if (ticCount % ticsBetweenLabels == 0)
    		ticLen = LONG_TIC_LEN;
    	    else if ((ticCount*2) % ticsBetweenLabels == 0)
    		ticLen = MED_TIC_LEN;
    	    else
    		ticLen = SHORT_TIC_LEN;
    	    if (outDevice == X_SCREEN) {
    	    	seg->x1=x; seg->x2=x-ticLen; seg->y1=seg->y2=rint(ticY);
    	    	seg++; nSegs++;
    	    } else {
    	    	fSeg->x1=x; fSeg->x2=x-ticLen; fSeg->y1=fSeg->y2=ticY;
    	    	fSeg++; nFSegs++;
    	    }
    	}
    }

    /* generate axis line and arrow heads to indicate data not shown */
    genVAxisLineSegs(&seg, &nSegs, x, y1, y2, True,
    		     maxData > maxLimit, minData < minLimit);
    
    /* draw the axis and tic lines */
    if (outDevice == X_SCREEN) {
    	XDrawSegments(display, window, gc, segs, nSegs);
    } else { /* PS_PRINTER */
    	PSFloatDrawSegments(display, window, gc, fSegs, nFSegs);
    	PSDrawSegments(display, window, gc, segs, nSegs);
    	XtFree((char *)fSegs);
    }
    XtFree((char *)segs);
}

static void drawLinearHAxis(Display *display, Drawable window, GC gc,
	XFontStruct *fs, int outDevice, int y, int x1, int x2,
	double minData, double maxData, double minLimit, double maxLimit)
{
    double dataRange = maxData - minData;
    double scale = (x2 - x1)/(maxLimit - minLimit);
    double pixRange = dataRange * scale;
    double minPix = x1 - (minLimit - minData) * scale;
    double minLabelSpacing = (fs->max_bounds).width * MIN_H_LABEL_SPACING;
    double labelValue, label1Value, ticValue, tic1Value;
    double minInterval, interval, ticInterval, maxTics, ticX;
    int consumedSpace, nSegs, nFSegs, maxSegs, multIndex;
    int maxLabels, labelY, labelLeft, labelRight, labelCenter;
    int ticLen, i, ticsBetweenLabels;
    XSegment *seg, *segs;
    FloatSegment *fSeg, *fSegs;
    
    /* find the maximum number of tics that will fit across the whole axis */
    maxTics = pixRange/MIN_TIC_SPACING;

    /* begin with the rounded factor of 10 below where
       labels would be placed every MIN_TIC_SPACING */
    minInterval = pow(10., floor(log10(dataRange/(double)maxTics) - 1.));
    
    /* loop, increasing the interval between until all of the labels fit */
    for (multIndex=0; multIndex<NMULTIPLIERS; ++multIndex) {
    	interval = minInterval * Multipliers[multIndex];
    	/* place the first label at the first multiple of interval > minData */
    	labelValue = ((int)(minData / (interval))) * interval;
    	if (labelValue < minData)
    	    labelValue += interval;
    	label1Value = labelValue;
    	/* loop, checking the placement of all of the labels */
    	consumedSpace = INT_MIN;
    	while (labelValue <= maxData) {
    	    labelCenter = (int)(minPix + (labelValue - minData) * scale);
    	    measureHCenteredFloat(fs, labelValue, labelCenter, &labelLeft,
    	    			  &labelRight);
    	    if (labelLeft < consumedSpace)
    	    	break;			/* collision with previous label */
    	    consumedSpace = labelRight + minLabelSpacing;;
    	    labelValue += interval;
    	}
    	/* exit loop if all of the labels were laid out successfully */
    	if (labelValue > maxData)
    	    break;
    }
    if (multIndex == NMULTIPLIERS) {
    	fprintf(stderr, "Couldn't lay out axis labels.  Font problem?");
    	return;
    }
    
    /* draw the labels using interval and label1Value calculated above */
    labelY = y + HAxisHeight(fs);
    maxLabels = (int)(dataRange/interval + 1);
    for (i=0; i<=maxLabels; i++) {
    	labelValue = label1Value + interval*i;
    	labelCenter = (int)(minPix + (labelValue - minData) * scale);
    	if (labelCenter >= x1 && labelCenter <= x2) {
    	    drawHCenteredFloat(display, window, gc, fs, outDevice,
    	    	labelValue, labelCenter, labelY);
    	}
    }

    /* allocate enough memory for X segments to draw axis lines and
       tics at minimum tic spacing: Maximum # of tics + last tic +
       axis line + 0-2 arrows of 2 segments each */
    maxSegs = (x2 - x1)/MIN_TIC_SPACING + 10;
    if (outDevice == X_SCREEN) {
    	segs = (XSegment *)XtMalloc(sizeof(XSegment) * maxSegs);
    	seg = segs;
    } else {
    	segs = (XSegment *)XtMalloc(sizeof(XSegment) * 5);
    	seg = segs;
    	fSegs = (FloatSegment *)XtMalloc(sizeof(FloatSegment) * maxSegs);
    	fSeg = fSegs;
    }
    nSegs = 0;
    nFSegs = 0;
    
    /* determine how far apart to draw the tics */
    ticInterval = calcLinTicInterval(interval, scale, multIndex % 3);
    
    /* draw the tics */
    tic1Value = label1Value - interval;
    ticsBetweenLabels = interval/ticInterval;
    maxTics = dataRange/ticInterval + ticsBetweenLabels + 1;
    for (i=0; i<=maxTics; i++) {
    	ticValue = tic1Value + ticInterval * i;
    	ticX = minPix + (ticValue - minData) * scale;
    	if (ticX >= x1 && ticX <= x2) {
    	    if (i % ticsBetweenLabels == 0)
    		ticLen = LONG_TIC_LEN;
    	    else if ((i*2) % ticsBetweenLabels == 0)
    		ticLen = MED_TIC_LEN;
    	    else
    		ticLen = SHORT_TIC_LEN;
    	    if (outDevice == X_SCREEN) {
    	    	seg->x1=seg->x2=(int)ticX; seg->y1=y; seg->y2=y+ticLen;
    	    	seg++; nSegs++;
    	    } else {
    	    	fSeg->x1=fSeg->x2=ticX; fSeg->y1=y; fSeg->y2=y+ticLen;
    	    	fSeg++; nFSegs++;
    	    }
    	}
    }
    
    /* generate axis line and arrow heads to indicate data not shown */
    genHAxisLineSegs(&seg, &nSegs, x1, x2, y, True,
    		     minData < minLimit, maxData > maxLimit);
    
    /* draw the axis and tic lines */
    if (outDevice == X_SCREEN) {
    	XDrawSegments(display, window, gc, segs, nSegs);
    } else { /* PS_PRINTER */
    	PSFloatDrawSegments(display, window, gc, fSegs, nFSegs);
    	PSDrawSegments(display, window, gc, segs, nSegs);
    	XtFree((char *)fSegs);
    }
    XtFree((char *)segs);
}

static void drawLogHAxis(Display *display, Drawable window, GC gc,
	XFontStruct *fs, int outDevice, int y, int x1, int x2,
	double minData, double maxData, double minLimit, double maxLimit)
{
    double logMaxData = log10(maxData), logMinData;
    double logMaxLimit = log10(maxLimit), logMinLimit = log10(minLimit);
    double logLimitRange = logMaxLimit - logMinLimit;
    double logScale = (x2 - x1)/(logMaxLimit - logMinLimit);
    double minLabelSpacing = (fs->max_bounds).width * MIN_H_LABEL_SPACING;
    double min, max, logMin, logMax, logRange, pixRange, minPix;
    double powLabelValue, logPowLabelValue, powLabel1Value, logPowLabel1Value;
    double labelValue, label1Value, labelLimit;
    double powTicValue, logPowTicValue, logPowTic1Value;
    double ticValue, tic1Value, ticLimit, ticX;
    double minInterval, minPowInterval, powInterval, interval;
    double ticInterval, scaledTicInterval, powTicInterval;
    double scaledInterval, maxPowTics, maxTics;
    int consumedSpace, innerSpace, endSpace, nSegs, nFSegs, maxSegs;
    int multIndex, powMultIndex;
    int maxPowLabels, labelY, labelLeft, labelRight, labelCenter, ticCount;
    int ticLen, i, j, powTicsBetweenLabels, ticsBetweenLabels, factor;
    XSegment *seg, *segs;
    FloatSegment *fSeg, *fSegs;
    
    /* minData may be <= zero, but treat it as DBL_MIN if so */
    if (minData <= 0.)
    	minData = DBL_MIN;
    logMinData = log10(minData);
    
    /* Spacing tics and labels for the entire data range gets very slow when
       the plot is zoomed way in, and spacing it using only the visible
       (limit) range makes labels and tics flash on and off durring panning.
       The compromise is to calculate spacing using a range 5 plot-widths on
       either side of the visible range of the plot.  The interval is also
       limited to 10 orders of magnitude max on either side of the visible
       range, since the width of exponential number strings is nice and
       uniform, and it saves a lot of calculation. */
    logMin = logMinLimit - (logLimitRange > 2. ? 10. : logLimitRange * 5.);
    if (logMinData > logMin) { min = minData; logMin = logMinData; }
    else min = pow(10., logMin);
    logMax = logMaxLimit + (logLimitRange > 2. ? 10. : logLimitRange * 5.);
    if (logMaxData < logMax) { max = maxData; logMax = logMaxData; }
    else max = pow(10., logMax);
    logRange = logMax - logMin;
    pixRange = logRange * logScale;
    minPix = x1 - (logMinLimit - logMin) * logScale;
    
    /* find the maximum number of tics that will fit across the whole axis */
    maxTics = pixRange/MIN_LOG_TIC_SPACING;

    /* begin the spacing of power-of-10 labels at the rounded factor of
       10 below where labels would be placed every MIN_LOG_TIC_SPACING */
    minPowInterval = pow(10., floor(log10(logRange/(double)maxTics) - 1.));
    if (minPowInterval < 1)
    	minPowInterval = 1;
    
    /* begin the spacing of the labels within powers of 10 at the interval
       where MIN_LOG_TIC_SPACING would fit at the tightest point in the data */
    minInterval = calcMinLogInterval((int)pixRange, min, max,
		MIN_LOG_TIC_SPACING, MIN_TIC_SPACING, &factor);
    if (factor == 1)
    	minInterval /= 2;
    else if (factor == 2)
    	minInterval /= 5;
    
    /* loop, increasing the interval between until all of the labels fit */
    for (powMultIndex=0; powMultIndex<NMULTIPLIERS; ++powMultIndex) {
    	powInterval = minPowInterval * Multipliers[powMultIndex];
    	/* place the first label at the first multiple of interval < min */
    	logPowLabel1Value = floor(logMin/powInterval) * powInterval;
    	powLabel1Value = pow(10., logPowLabel1Value);
    	powLabelValue = powLabel1Value;
    	logPowLabelValue = logPowLabel1Value;
    	/* loop, checking the placement of all of the labels */
    	consumedSpace = INT_MIN;
    	multIndex = 0;
    	while (powLabelValue <= max) {
   	    /* place power of 10 label */
    	    labelCenter = (int)(minPix+(logPowLabelValue-logMin)*logScale);
    	    if (labelCenter >= x1) {
    		measureHCenteredFloat(fs, powLabelValue, labelCenter,
    				      &labelLeft, &labelRight);
    		if (labelLeft < consumedSpace)
    	    	    break;		/* collision with previous label */
    		consumedSpace = labelRight + minLabelSpacing;
	    }
	    /* temporarily create the next power of 10 label to define the
	       space for the labels within the power of 10 interval */
    	    labelCenter =
    	    	(int)(minPix + (logPowLabelValue+1.-logMin)*logScale);
    	    measureHCenteredFloat(fs, powLabelValue*10., labelCenter,
    	    			  &endSpace, &labelRight);
	    /* loop trying to place the labels within the power of 10 interval */
    	    while (multIndex<NMULTIPLIERS) {
    	    	interval = minInterval * Multipliers[multIndex];
    	    	if (interval > 5.)
    	    	    break;	/* labels don't fit within powers of 10 */
		if (interval > 1.) {
		    /* if labels are at intervals of more than one, just put
		       labels at 2 and 5 */
		    scaledInterval = 3. * powLabelValue;
		    labelLimit = 6. * powLabelValue;
		    label1Value = 2. * powLabelValue;
		} else {
    	    	    scaledInterval = interval * powLabelValue;
		    if (min > powLabelValue)
			label1Value = rint(min/scaledInterval) * scaledInterval;
		    else
			label1Value = powLabelValue + scaledInterval;
		    if (max < powLabelValue*10)
			labelLimit = max;
		    else
			labelLimit = powLabelValue*10 - scaledInterval/2;
		}
		labelValue = label1Value;
   	    	innerSpace = consumedSpace;
     	    	while (labelValue <= labelLimit) {
    	    	    labelCenter = (int)(minPix +
    	    	    		(log10(labelValue)-logMin)*logScale);
    	    	    measureHCenteredFloat(fs, labelValue, labelCenter,
    	    			  	  &labelLeft, &labelRight);
    		    if (labelLeft < innerSpace)
    	    		break;		/* collision with previous label */
    		    innerSpace = labelRight + minLabelSpacing;
    		    labelValue += scaledInterval;
    		}
    		if (labelValue > labelLimit && innerSpace < endSpace)
    		    break; /* labels laid out successfully */
    		multIndex++;
    	    }
    	    if (multIndex == NMULTIPLIERS) {
    		fprintf(stderr, "Couldn't lay out axis labels.  Font problem?");
    		return;
	    }
    	    logPowLabelValue += powInterval;
    	    powLabelValue = pow(10., logPowLabelValue);
    	}
    	/* exit loop if all of the labels were laid out successfully */
    	if (powLabelValue > max)
    	    break;
    }
    if (powMultIndex == NMULTIPLIERS) {
    	fprintf(stderr, "Couldn't lay out axis labels.  Font problem?");
    	return;
    }
    
    /* draw the labels using interval and label1Value calculated above */
    labelY = y + HAxisHeight(fs);
    maxPowLabels = (int)(logRange/powInterval + 1);
    for (i=0; i<=maxPowLabels; i++) {
    	logPowLabelValue = logPowLabel1Value + powInterval*i;
    	powLabelValue = pow(10., logPowLabelValue);
    	labelCenter = (int)(minPix + (logPowLabelValue-logMin)*logScale);
    	if (labelCenter >= x1 && labelCenter <= x2)
    	    drawHCenteredFloat(display, window, gc, fs, outDevice,
    	    	powLabelValue, labelCenter, labelY);
    	if (interval > 5.)
    	    continue;
	if (interval > 1.) {
	    /* if labels are at intervals of more than one, label at 2 and 5 */
	    scaledInterval = 3. * powLabelValue;
	    labelLimit = 6. * powLabelValue;
	    label1Value = 2. * powLabelValue;
	} else {
    	    scaledInterval = interval * powLabelValue;
	    if (min > powLabelValue)
		label1Value = rint(min/scaledInterval) * scaledInterval;
	    else
		label1Value = powLabelValue + scaledInterval;
	    if (max < powLabelValue*10)
		labelLimit = max;
	    else
		labelLimit = powLabelValue*10 - scaledInterval/2;
	}
    	labelValue = label1Value;
    	for (j=1; labelValue <= labelLimit; j++) {
    	    labelCenter = (int)(minPix +
    	    	    	(log10(labelValue)-logMin)*logScale);
    	    if (labelCenter >= x1 && labelCenter <= x2)
    		drawHCenteredFloat(display, window, gc, fs, outDevice,
    	    	    labelValue, labelCenter, labelY);
	    labelValue = label1Value + j*scaledInterval;
	}
     }

    /* allocate enough memory for X segments to draw axis lines and
       tics at minimum tic spacing: Maximum # of tics + last tic +
       axis line + 0-2 arrows of 2 segments each */
    maxSegs = (x2 - x1)*10/* /MIN_LOG_TIC_SPACING */ + 10;
    if (outDevice == X_SCREEN) {
    	segs = (XSegment *)XtMalloc(sizeof(XSegment) * maxSegs);
    	seg = segs;
    } else {
    	segs = (XSegment *)XtMalloc(sizeof(XSegment) * 5);
    	seg = segs;
    	fSegs = (FloatSegment *)XtMalloc(sizeof(FloatSegment) * maxSegs);
    	fSeg = fSegs;
    }
    nSegs = 0;
    nFSegs = 0;
    
    /* determine how far apart to draw the tics */
    powTicInterval = calcLinTicInterval(powInterval, logScale, powMultIndex % 3);
    if (powTicInterval < 1)
    	powTicInterval = 1;
    ticInterval = calcLogTicInterval((int)pixRange, interval, min,
    				     max, multIndex % 3);
    
    /* draw the tics */
    logPowTic1Value = logPowLabel1Value;
    powTicsBetweenLabels = powInterval/powTicInterval;
    maxPowTics = logRange/powTicInterval + powTicsBetweenLabels + 2;
    for (i=0; i<=maxPowTics; i++) {
    	logPowTicValue = logPowTic1Value + powTicInterval * i;
    	powTicValue = pow(10., logPowTicValue);
    	ticX = minPix + (logPowTicValue - logMin) * logScale;
    	if (ticX >= x1 && ticX <= x2) {
    	    if (i % powTicsBetweenLabels == 0)
    		ticLen = LONG_TIC_LEN;
    	    else if ((i*2) % powTicsBetweenLabels == 0)
    		ticLen = MED_TIC_LEN;
    	    else
    		ticLen = SHORT_TIC_LEN;
    	    if (outDevice == X_SCREEN) {
    	    	seg->x1=seg->x2=(int)ticX; seg->y1=y; seg->y2=y+ticLen;
    	    	seg++; nSegs++;
    	    } else {
    	    	fSeg->x1=fSeg->x2=ticX; fSeg->y1=y; fSeg->y2=y+ticLen;
    	    	fSeg++; nFSegs++;
    	    }
    	}
    	/* draw the tics within the power-of-10 intervals */
	if (ticInterval > 1.)
	    continue;
	scaledTicInterval = ticInterval * powTicValue;
	ticsBetweenLabels = interval/ticInterval;
	ticLimit = pow(10., floor(logPowTicValue+1.)) - scaledTicInterval/2;
	if (min > powTicValue)
	    tic1Value = floor(min/(scaledTicInterval*10)) *
			     scaledTicInterval*10 + scaledTicInterval;
	else
	    tic1Value = powTicValue + scaledTicInterval;
	ticValue = tic1Value;
	for (j=1; ticValue < ticLimit; j++) {
	    ticValue = tic1Value + (j-1)*scaledTicInterval;
	    ticX = minPix + (log10(ticValue)-logMin)*logScale;
	    ticCount = (min<=powTicValue && ticInterval>=1.) ? j+1 : j;
	    if (ticX < x1)
	    	continue;
	    if (ticX > x2)
	    	break;
    	    if (interval>1. && interval<=2. && (ticCount==2 || ticCount==5))
    		ticLen = LONG_TIC_LEN;
    	    else if (interval>1. && ticCount==5)
    		ticLen = MED_TIC_LEN;
    	    else if (interval>1.)
    		ticLen = SHORT_TIC_LEN;
    	    else if (ticCount % ticsBetweenLabels == 0)
    		ticLen = LONG_TIC_LEN;
    	    else if ((ticCount*2) % ticsBetweenLabels == 0)
    		ticLen = MED_TIC_LEN;
    	    else
    		ticLen = SHORT_TIC_LEN;
    	    if (outDevice == X_SCREEN) {
    	    	seg->x1=seg->x2=(int)ticX; seg->y1=y; seg->y2=y+ticLen;
    	    	seg++; nSegs++;
    	    } else {
    	    	fSeg->x1=fSeg->x2=ticX; fSeg->y1=y; fSeg->y2=y+ticLen;
    	    	fSeg++; nFSegs++;
    	    }
    	}
    }
    
    /* generate axis line and arrow heads to indicate data not shown */
    genHAxisLineSegs(&seg, &nSegs, x1, x2, y, True,
    		     minData < minLimit, maxData > maxLimit);
    
    /* draw the axis and tic lines */
    if (outDevice == X_SCREEN) {
    	XDrawSegments(display, window, gc, segs, nSegs);
    } else { /* PS_PRINTER */
    	PSFloatDrawSegments(display, window, gc, fSegs, nFSegs);
    	PSDrawSegments(display, window, gc, segs, nSegs);
    	XtFree((char *)fSegs);
    }
    XtFree((char *)segs);
}

static void drawDiscreteHAxis(Display *display, Drawable window, GC gc,
	XFontStruct *fs, int outDevice, int y, int x1, int x2, double minData,
	double maxData, double minLimit, double maxLimit, int nBins)
{
    double dataRange = maxData - minData;
    double scale = (x2 - x1)/(maxLimit - minLimit);
    int pixRange = dataRange * scale;
    int minPix = x1 - (minLimit - minData) * scale;
    double minLabelSpacing = (fs->max_bounds).width * MIN_H_LABEL_SPACING;
    int consumedSpace;
    double labelValue, increment;
    int i, l, nLabels, mult, labelLeft, labelRight, labelCenter, labelY, nSegs;
    int ticX, ticLen;
    XSegment *seg, *segs;
    
    /* determine the number of labels that will fit by simulating drawing
       them.  Iterate first trying to label every bin edge, then every other
       bin, then every 5th, every tenth, etc., until the labels fit. */
    for (i=0, mult=Multipliers[0]; i<NMULTIPLIERS; mult=Multipliers[++i]) {
    	nLabels = nBins/mult + 1;
    	increment = (dataRange / (double)nBins) * mult;
    	consumedSpace = INT_MIN;
    	for (l=0; l<nLabels; l++) {
    	    labelValue = minData + l*increment;
    	    labelCenter = minPix + l*increment*scale;
    	    measureHCenteredFloat(fs, labelValue, labelCenter,
    	    			  &labelLeft, &labelRight);
    	    if (labelLeft < consumedSpace)
    	    	break;
    	    consumedSpace = labelRight + minLabelSpacing;
    	}
    	if (l == nLabels)
    	    break;		/* all labels successfully laid out */
    }
    if (i == NMULTIPLIERS) {
    	fprintf(stderr, "Could not lay out axis labels!  Font problem?");
    	return;
    }
    
    /* draw the labels using the nLabels calculated above */
    labelY = y + HAxisHeight(fs);
    for (l=0; l<nLabels; l++) {
    	labelValue = minData + l*increment;
    	if (labelValue >= minLimit && labelValue <= maxLimit) {
    	    labelCenter = minPix + (labelValue - minData) * scale;
    	    drawHCenteredFloat(display, window, gc, fs, outDevice,
    	    		       labelValue, labelCenter, labelY);
    	}
    }
    
    /* allocate memory for segments to draw axis lines and tics, 1 tic per
       bin + last tic + axis line + 0-2 arrows of 2 segments each */
    segs = (XSegment *)XtMalloc(sizeof(XSegment) * (nBins + 10));
    seg = segs; nSegs = 0;
    
    /* generate the tic lines */
    for (i=0; i<=nBins; i++) {
    	ticX = minPix + (i * pixRange)/nBins;
    	if (ticX >= x1 && ticX <= x2) {
    	    if (i % mult == 0)
    		ticLen = LONG_TIC_LEN;
    	    else if ((i*2) % mult == 0)
    		ticLen = MED_TIC_LEN;
    	    else
    		ticLen = SHORT_TIC_LEN;
    	    seg->x1=seg->x2=ticX; seg->y1=y; seg->y2=y+ticLen; seg++; nSegs++;
    	}
    }
    
    /* generate axis line and arrow heads to indicate data not shown */
    genHAxisLineSegs(&seg, &nSegs, x1, x2, y, True,
    		     minData < minLimit, maxData > maxLimit);
    
    /* draw the axis and tic lines */
    if (outDevice == X_SCREEN)
    	XDrawSegments(display, window, gc, segs, nSegs);
    else /* PS_PRINTER */
    	PSDrawSegments(display, window, gc, segs, nSegs);
    XtFree((char *) segs);
}

/*
** RedrawHAxisArrows, RedrawVAxisArrows
**
** Arrows on the tips of the axes are used to indicate that data which is not
** shown exists beyond the edge of the plot.  These arrows protrude into
** the area where the plot is drawn, so RedrawHAxisArrows and RedrawVAxisArrows
** are provided to fix up the arrows if plot data has overwritten them.
*/
void RedrawHAxisArrows(Display *display, Drawable window, GC gc,
		       int y, int x1, int x2, double minData, double maxData,
		       double minLimit, double maxLimit)
{
    XSegment *seg, *segs;
    int nSegs = 0;

    seg = segs = (XSegment *)XtMalloc(sizeof(XSegment) * 5);
    genHAxisLineSegs(&seg, &nSegs, x1, x2, y, False,
    		     minData < minLimit, maxData > maxLimit);
    XDrawSegments(display, window, gc, segs, nSegs);
    XtFree((char *) segs);
}
void RedrawVAxisArrows(Display *display, Drawable window, GC gc,
		       int x, int y1, int y2, double minData, double maxData,
		       double minLimit, double maxLimit)
{
    XSegment *seg, *segs;
    int nSegs = 0;

    seg = segs = (XSegment *)XtMalloc(sizeof(XSegment) * 5);
    genVAxisLineSegs(&seg, &nSegs, x, y1, y2, False,
    		     maxData > maxLimit, minData < minLimit);
    XDrawSegments(display, window, gc, segs, nSegs);
    XtFree((char *) segs);
}

/* Calculate the number of pixels to the left of the decimal point given
   a string containing a number and an x font structure for measuring */
static int calcDecimalOffset(char *labelStr, XFontStruct *fs)
{
    char *decimalStr;

    if (strchr(labelStr, 'e') != NULL)
    	return 0;
    decimalStr = strchr(labelStr, '.');
    if (decimalStr == NULL)
    	return 0;
    return XTextWidth(fs, decimalStr, strlen(decimalStr));
}

/* Calculate interval (how far apart to draw in data coordinates) between
   tics given interval between labels, scale,  and whether the multiplier
   used was a factor of 10, 2, or 5.  Parameter factor==0: 10,
   factor==1: 2, factor==2: 5 */
static double calcLinTicInterval(double labelInterval, double scale, int factor)
{
    return calcTicInterval(labelInterval, MIN_TIC_SPACING/scale, factor);
}

static double calcLogTicInterval(int pixRange, double labelInterval, double min,
				 double max, int factor)
{
    double minTicInterval;
    int dummyFactor;
    
    minTicInterval = calcMinLogInterval(pixRange, min, max,
    			MIN_LOG_TIC_SPACING, MIN_TIC_SPACING, &dummyFactor);
    return calcTicInterval(labelInterval, minTicInterval, factor);
}

static double calcTicInterval(double labelInterval, double minTicInterval,
			      int factor)
{
    switch (factor) {
      case 0: /* labels placed at factors of 10 */
      	if (labelInterval/10 >= minTicInterval)
      	    return labelInterval/10;
      	else if (labelInterval/2 >= minTicInterval)
      	    return labelInterval/2;
      	else
      	    return labelInterval;
      case 1: /* labels placed at factors of 2 */
      	if (labelInterval/2 >= minTicInterval)
      	    return labelInterval/2;
      	else
      	    return labelInterval;
      case 2: /* labels placed at factors of 5 */
      	if (labelInterval/5 >= minTicInterval)
      	    return labelInterval/5;
      	else
      	    return labelInterval;
    }
}

/*
** Calculate label spacing in data coordinates, return the spacing in
** interval, and return the step factor as the function value.  The
** possible values are: 0: factor of 10, 1: factor of 2, 2: factor of 5.
*/
static double calcLabelInterval(int pixRange, double dataRange, XFontStruct *fs,
				int *factor)
{
    double interval, minInterval, maxTics;
    int minLabelSpacing = fs->ascent * (1 + MIN_V_LABEL_SPACING);
    int multIndex;
    
    /* find the maximum number of tics that will fit across the visible axis */
    maxTics = pixRange/(double)MIN_TIC_SPACING;

    /* begin with the rounded factor of 10 below where
       labels would be placed every MIN_TIC_SPACING */
    minInterval = pow(10., floor(log10(dataRange/maxTics) - 1.));
    
    /* loop, increasing the interval between labels until they all fit */
    for (multIndex=0; multIndex<NMULTIPLIERS; ++multIndex) {
   	interval = minInterval * Multipliers[multIndex];
   	if ((dataRange/interval) * minLabelSpacing <= pixRange)
    	    break;
    }

    if (multIndex == NMULTIPLIERS) {
    	fprintf(stderr, "Couldn't lay out V axis labels.  Font problem?");
    	return 0.;
    }
    
    *factor = multIndex % 3;
    return interval;
}

static double calcLogLabelInterval(int pixRange, double min, double max,
				   XFontStruct *fs, int *factor)
{
    return calcMinLogInterval(pixRange, min, max, 
    		fs->ascent * (1 + MIN_V_LOG_LABEL_SPACING),
    		fs->ascent * (1 + MIN_V_LABEL_SPACING), factor);
}

/*
** Calculate the minimum even (power of 2, 5, or 10) interval that will
** fit within one power of 10 between min and max.  The minimum spacing
** is given in pixels for both the wide and the narrow ends of the range such
** that narrowSpacing and wideSpacing pixels will fit at the narrowly spaced
** maximum end of and widely spaced minimum end of one power of 10.
*/
static double calcMinLogInterval(int pixRange, double min, double max,
			double narrowSpacing, double wideSpacing, int *factor)
{
    double interval, minInterval, minPow, maxPow, scale;
    double logNarrowSpacing, logWideSpacing;
    int multIndex;
    double logMin = log10(min);
    double logMax = log10(max);
    
    /* find the minimum tic & label spacing in the log coordinate system */
    scale = (logMax-logMin) / pixRange;
    logNarrowSpacing = narrowSpacing * scale;
    logWideSpacing = wideSpacing * scale;
    
    /* normalize the range to values between 1 and 10 (one power of 10) */
    minPow = (int)floor(logMin);
    maxPow = (int)ceil(logMax);
    if (maxPow - minPow > 1) {
    	min = 1.;
    	max = 10.;
    	logMin = 0.;
    	logMax = 1.;
    } else {
    	logMin -= minPow;
    	logMax -= minPow;
    	min = pow(10., logMin);
    	max = pow(10., logMax);
    }
    
    /* measure that distance down from the maximum range to get the minimum
       spacing in data coordinates for the closest objects in the range */
    minInterval = pow(10., logMax) - pow(10., logMax - logNarrowSpacing);

    /* round the interval size down to the nearest power of 10 */
    minInterval = pow(10., floor(log10(minInterval)));
    
    /* loop, increasing the interval between objects until they all fit */
    for (multIndex=0; multIndex<NMULTIPLIERS; ++multIndex) {
   	interval = minInterval * Multipliers[multIndex];
   	if (interval >= 10.)
   	    break;
   	if (logMax - log10(max-interval) >= logNarrowSpacing &&
   	    log10(min+interval) - logMin >= logWideSpacing)
    	    break;
    }

    if (multIndex == NMULTIPLIERS) {
    	fprintf(stderr, "Couldn't lay out V axis labels.  Font problem?");
    	return 0.;
    }
    
    *factor = multIndex % 3;
    return interval;
}

static void drawHCenteredFloat(Display *display, Drawable window, GC gc,
	XFontStruct *fs, int outDevice, double value, int x, int y)
{
    char valueStr[16];
    int textWidth, strLen;
    
    sprintf(valueStr, "%g", value);
    strLen = strlen(valueStr);
    textWidth = XTextWidth(fs, valueStr, strLen);
    if (outDevice == X_SCREEN)
    	XDrawString(display, window, gc, x-textWidth/2, y, valueStr, strLen);
    else
    	PSDrawString(display, window, gc, fs, x, y, PS_CENTER, valueStr);
}

static void measureHCenteredFloat(XFontStruct *fs, double value,
			   int x, int *left, int *right)
{
    char valueStr[16];
    int textWidth, strLen;
    
    sprintf(valueStr, "%g", value);
    strLen = strlen(valueStr);
    textWidth = XTextWidth(fs, valueStr, strLen);
    *left = x - textWidth/2;
    *right = x + textWidth/2;
}

static void genHAxisLineSegs(XSegment **segPtr, int *segCount, int x1, int x2,
		int y, int drawAxisLine, int drawLeftArrow, int drawRightArrow)
{
    XSegment * seg = *segPtr;
    int nSegs = 0;
    
    /* generate the axis line */
    if (drawAxisLine) {
    	seg->x1 = x1; seg->x2 = x2; seg->y1 = seg->y2 = y; seg++; nSegs++;
    }
    
    /* draw arrow heads to indicate data not shown */
    if (drawLeftArrow) {
    	seg->x1=x1; seg->x2=x1+6; seg->y1=y; seg->y2=y+2; seg++; nSegs++;
    	seg->x1=x1; seg->x2=x1+6; seg->y1=y; seg->y2=y-2; seg++; nSegs++;
    }
    if (drawRightArrow) {
    	seg->x1=x2; seg->x2=x2-6; seg->y1=y; seg->y2=y+2; seg++; nSegs++;
    	seg->x1=x2; seg->x2=x2-6; seg->y1=y; seg->y2=y-2; seg++; nSegs++;
    }

    *segPtr = seg;
    *segCount += nSegs;
}

static void genVAxisLineSegs(XSegment **segPtr, int *segCount, int x, int y1,
	int y2, int drawAxisLine, int drawTopArrow, int drawBottomArrow)
{
    XSegment * seg = *segPtr;
    int nSegs = 0;
    
    /* generate the axis line */
    if (drawAxisLine) {
    	seg->x1 = seg->x2 = x; seg->y1 = y1; seg->y2 = y2; seg++; nSegs++;
    }
    
    /* draw arrow heads to indicate data not shown */
    if (drawTopArrow) {
    	seg->y1=y1; seg->y2=y1+6; seg->x1=x; seg->x2=x+2; seg++; nSegs++;
    	seg->y1=y1; seg->y2=y1+6; seg->x1=x; seg->x2=x-2; seg++; nSegs++;
    }
    if (drawBottomArrow) {
    	seg->y1=y2; seg->y2=y2-6; seg->x1=x; seg->x2=x+2; seg++; nSegs++;
    	seg->y1=y2; seg->y2=y2-6; seg->x1=x; seg->x2=x-2; seg++; nSegs++;
    }
    
    *segPtr = seg;
    *segCount += nSegs;
}

/* Draw a vertical axis label representing "value" vertically centered on
   "y", right justified to "x", (but with decimal points for non-exponential
   numbers aligned, if possible).  "maxWidth" specifies the maximum width
   allowed in pixels to fit the number, and "maxDecimalOffset" represents
   the pixel offset to the decimal point of the number with the longest
   fractional part (where the decimal point should be placed if possible). */
static void drawVCenteredFloat(Display *display, Drawable window, GC gc,
	XFontStruct *fs, int outDevice, double value, int x, int y,
	int maxWidth, int maxDecimalOffset)
{
    char valueStr[16];
    int top, width, strLen, offset;

    fitDouble(valueStr, value, maxWidth, fs);
    strLen = strlen(valueStr);
    top = y + fs->ascent/2;
    width = XTextWidth(fs, valueStr, strLen);
    if (strchr(valueStr, 'e') != NULL)
    	offset = 0;
    else { /* line up decimal points at maxDecimalOffset */
    	offset =  maxDecimalOffset - calcDecimalOffset(valueStr, fs);
    	if (width + offset > maxWidth) /* ... unless value no longer fits */
    	    offset = 0;
    }
    if (outDevice == X_SCREEN)
    	XDrawString(display, window, gc, x-width-offset, top, valueStr, strLen);
    else
    	PSDrawString(display, window, gc, fs, x-offset, top, PS_RIGHT,valueStr);
}

/* Convert a number to a string with special attention paid to fitting the
   number into the maximum allowable width.  If fit is impossible, return
   ".." */
static void fitDouble(char *string, double number, int maxWidth,
	XFontStruct *fs)
{
    double mantissa, exponent;
    char mantString[16], expString[16], *s;
    int i, expLen, prec;
    
    /* Find the exponent for representing the number in exponential notation */
    if (fabs(number) < FLT_MIN)
    	exponent = 0.;
    else
	exponent = floor(log10(fabs(number)));
    
    /* Try to fit a non-exponential first (if it makes sense to do so).  Tries
       G format first (for supression of trailing zeros), then tries F format
       with lower and lower precision until the number fits */
    if ((exponent >= -4 && exponent < 6) || fabs(number) <= DBL_MIN) {
    	for (prec=8; prec>=0; prec--) {
    	    if (prec == 8)
    		sprintf(mantString, "%g", number);
    	    else
    		sprintf(mantString, "%.*f", prec, number);
    	    if (XTextWidth(fs, mantString, strlen(mantString)) <= maxWidth) {
    	    	strcpy(string, mantString);
    	    	return;
    	    }
    	}
    }

    /* Try exponential notation (note the strange adjustment below when
       rounding increases the mantissa to two digits (mantissa = 10 or -10),
       it is trimmed back to one and the exponent is increased to compensate) */
    mantissa = number/pow(10., exponent);
    sprintf(expString, "%g", exponent);
    expLen = strlen(expString);
    for (prec=6; prec>=0; prec--) {
    	if (prec == 6)
    	    sprintf(mantString, "%g", mantissa);
    	else
    	    sprintf(mantString, "%.*f", prec, mantissa);
	if (!strncmp(mantString, "10", 2) || !strncmp(mantString, "-10", 3)) {
	    sprintf(expString, "%g", exponent+1);
    	    expLen = strlen(expString);
    	    strcpy(mantString, mantissa>=0 ? "1" : "-1");
    	}
    	if (XTextWidth(fs, mantString, strlen(mantString)) + XTextWidth(fs,
    		expString, expLen) + XTextWidth(fs, "e", 1) <= maxWidth) {
    	    sprintf(string, "%se%s", mantString, expString);
    	    return;
       }
    }
    
    /* If it didn't fit in the available space, return ".." */
    strcpy(string, "..");
}

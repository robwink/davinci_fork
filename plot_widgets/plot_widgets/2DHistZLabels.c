/*******************************************************************************
*									       *
* drawAxes.c -- Z axis labeling routines for 2DHist widget		       *
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
* Modification (by Konstantine Iourcha) of generic routines		       *
* initially written by Mark Edel					       *
*									       *
*******************************************************************************/
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h> 
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#if XmVersion == 1002
#include <Xm/PrimitiveP.h>
#endif
#include <Xm/DrawingA.h>
#include <X11/Xutil.h>
#include <math.h>

#ifndef VMS
#include <string.h>
#endif /* VMS */

#include <limits.h>
/*?*/
#include <stdio.h>
#include "2DGeom.h"
#include "2DHistDefs.h"
#include "2DHist.h"
#include "uniLab.h"
#include "labels.h"		
#include "2DHistP.h"
#include "2DHistZLabels.h"
/* prototypes of system routines missing from include files */
double rint(double);

#define MIN_LOG_TIC_SPACING 2	/* min spacing for axis tics on log plots */
#define MIN_V_LOG_LABEL_SPACING .5 /* "" for log plots */
#define MIN_H_LOG_LABEL_SPACING .5 /* "" for log plots */

/* table of multiples of 10, 2, and 5 for reference in figuring label spacing */
#define NMULTIPLIERS 15
static int Multipliers[NMULTIPLIERS] = {
    1, 2, 5, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000};
static void makeVerticalAxis(XFontStruct *fs, int x, int y1, int y2, 
  double minLimit, double maxLimit, int logScaling,
  labelsToDraw ***lTD,XSegment **tics, int *nSeg);
static void makeLinearVAxis(XFontStruct *fs, int x, int y1, int y2, 
  double minLimit, double maxLimit, 
  labelsToDraw ***lTD,XSegment **tics, int *nSeg);
static void makeLogVAxis(XFontStruct *fs, int x, int y1, int y2, 
  double minLimit, double maxLimit, 
  labelsToDraw ***lTD,XSegment **tics, int *nSeg);    
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
static void makeZLabel(labelsToDraw ***lTD, 
			       XFontStruct *fs,   double value,
			       int x, int y, int maxDecimalOffset);
static void makeBackPlanes (Hist2DWidget w, objectsToDraw *oD);

/*
** A note about rounding errors:
**
** Because this code deals with both an integer coordinate system and
** the floating point coordinate system of the data, it is sensitive
** to rounding errors.  Be careful when you make changes.  Think about
** how the data will be quantized.  Will precision be lost?  Will there
** be biases?
*/

void makeZLabels (Hist2DWidget w, objectsToDraw *oD)
{
/* it is assumed that oD is already initialized */
    discreteMap *dMap;   
    labelsToDraw **lTD;
    if (oD ==NULL) return;    
    dMap = w->hist2D.discrMap;
    if (dMap == NULL) return;
    lTD = &(oD->zLabels);
    makeVerticalAxis(w->hist2D.fs,
      dMap->vertex.x - dMap->map.x.y,
      dMap->vertex.y - dMap->map.y.y,
      dMap->vertex.y - dMap->map.y.y + dMap->zFactor,
      w->hist2D.zVisibleStart,
      w->hist2D.zVisibleEnd,
      w->hist2D.zLogScaling, 
      &lTD, &(oD->zTics), &(oD->nZTics));
    if (w->hist2D.backPlanesOn)
     	makeBackPlanes ( w, oD);
}      
      
    

/*
** makeVerticalAxis
**
** make a vertical axis for 2D histogram.  Specify the position of
** the axis by the x and y window coordinates where the actual axis line should be
** drawn. 
**
** Parameters
**
**	fs		X font structure for font measurements.  
**	x, y1, and, y	Where to draw the axis line itself
**			in window coordinates.
**	minLimit,	Limits in data coordinates on the data displayed.
**	    maxLimit	The part of the axis outside of minLimit and maxLimit
**			is not drawn, but arrows indicate that some data is
**			not shown.
**	logScaling	Indicates log10 Z scaling (or linear if value is zero)
**	lTD		pointer to the address to which pointer to start of
**			labelsToDraw linked list shold be put after the return
**	tics		addres where pointer to tics array should be after
**			return
**	nTics		addres where actual number of tics should be after
**			return 
*/
static void makeVerticalAxis(XFontStruct *fs, int x, int y1, int y2, 
  double minLimit, double maxLimit, int logScaling,
  labelsToDraw ***lTD,XSegment **tics, int *nSeg)
{
    int temp;
    
    /* reject bad data before it causes an error (just don't make axis) */
    if (y1==y2 ||  maxLimit<=minLimit)
    	return;
    
    /* put y1 & y2 in a known order (y1 = top coord, y2 = bottom coord) */
    if (y2 < y1) {
    	temp = y2; y2 = y1; y1 = temp;
    }

    if (logScaling)
        makeLogVAxis(fs, x, y1, y2, minLimit, maxLimit, lTD,tics,nSeg);
    else
        makeLinearVAxis(fs, x, y1, y2, minLimit, maxLimit, lTD,tics,nSeg);
}

static void makeLinearVAxis(XFontStruct *fs, int x, int y1, int y2, 
  double minLimit, double maxLimit, 
  labelsToDraw ***lTD,XSegment **tics, int *nSegs)
{
    double dataRange = maxLimit - minLimit;
    int pixRange = y2 - y1;
    int maxDecimalOffset = 0;
    char labelStr[16];
    double labelValue, label1Value, ticValue, tic1Value;
    double interval, ticInterval;
    double scale, maxTics;
    int factor, decimalOffset;
    int maxLabels, labelCenter;
    int ticY, ticLen, i, ticsBetweenLabels;
    XSegment *seg;
    
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
    	sprintf(labelStr, "%g", labelValue);
    	decimalOffset = calcDecimalOffset(labelStr, fs);
    	if (decimalOffset > maxDecimalOffset)
    	    maxDecimalOffset = decimalOffset;
    }

    /* make the labels using the interval, starting value,
       and string offsets calculated above */
    for (i=0; i<=maxLabels; i++) {
    	labelValue = label1Value + interval*i;
    	labelCenter = rint(y2 - (labelValue - minLimit) * scale);
    	if (labelCenter >= y1 && labelCenter + fs->ascent / 2 < y2) {
	    makeZLabel(lTD, fs, labelValue,
		x - LONG_TIC_LEN - 1, labelCenter, maxDecimalOffset);
    	}
    **lTD = NULL;
    }

    /* allocate enough memory for X segments to make axis lines and
       tics at minimum tic spacing: Maximum # of tics + last tic */
    *tics = (XSegment *)XtMalloc(sizeof(XSegment) *
    				((y2 - y1)/MIN_TIC_SPACING + 10));
    seg = *tics; *nSegs = 0;
    
    /* make the tics using interval, ticInterval, and tic1Value from above */
    ticsBetweenLabels = interval/ticInterval;
    maxTics = dataRange/ticInterval + ticsBetweenLabels + 2;
    for (i=0; i<=maxTics; i++) {
    	ticValue = tic1Value + ticInterval * i;
    	ticY = rint(y2 - (ticValue - minLimit) * scale);
    	if (ticY >= y1 && ticY <= y2) {
    	    if (i % ticsBetweenLabels == 0)
    		ticLen = LONG_TIC_LEN;
    	    else if ((i*2) % ticsBetweenLabels == 0)
    		ticLen = MED_TIC_LEN;
    	    else
    		ticLen = SHORT_TIC_LEN;
    	    seg->x1=x; seg->x2=x-ticLen; seg->y1=seg->y2=ticY; seg++; (*nSegs)++;
    	}
    }

}

static void makeLogVAxis(XFontStruct *fs, int x, int y1, int y2, 
  double minLimit, double maxLimit, 
  labelsToDraw ***lTD,XSegment **tics, int *nSegs)    
{
    double logMinLimit, logMaxLimit;
    char labelStr[16];
    double label1Value, labelValue, logLabelValue, labelLimit;
    double powLabelValue, logPowLabelValue, logPowLabel1Value;
    double ticValue, tic1Value, ticLimit;
    double powTicValue, logPowTicValue, logPowTic1Value;
    double logPowInterval, logPowTicInterval;
    double interval, scaledInterval, ticInterval, scaledTicInterval;
    double logScale;
    int factor, powFactor;
    int maxPowLabels, maxPowTics, labelCenter, powLabelCenter;
    int ticY, ticLen, i, j, ticCount, ticsBetweenLabels, powTicsBetweenLabels;
    int decimalOffset, maxDecimalOffset = 0;
    XSegment *seg;
    
    /* reject dangerous values */
    if (minLimit <= 0.)
    	return;
    	
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
    	sprintf(labelStr, "%g", powLabelValue);
    	maxDecimalOffset = calcDecimalOffset(labelStr, fs);
	labelValue = label1Value;
	for (j=1; labelValue < labelLimit; j++) {
	    logLabelValue = log10(labelValue);
	    labelCenter = (int)rint(y2 - (logLabelValue-logMinLimit)*logScale);
	    if (labelCenter < y1)
	    	break;
   	    sprintf(labelStr, "%g", labelValue);
    	    decimalOffset = calcDecimalOffset(labelStr, fs);
    	    if (decimalOffset > maxDecimalOffset)
    		maxDecimalOffset = decimalOffset;
	    labelValue = label1Value + j*scaledInterval;
	}
	
    	/* make the power-of 10 label  */
    	powLabelCenter = rint(y2 - (logPowLabelValue - logMinLimit) * logScale);
    	if (powLabelCenter >= y1 && powLabelCenter + fs->ascent / 2 < y2)
	    makeZLabel(lTD, fs, powLabelValue,
		x - LONG_TIC_LEN - 1, powLabelCenter, maxDecimalOffset);
    	
    	/* make the labels within the power-of-10 intervals */
	if (interval > 2.)
	    continue;
	labelValue = label1Value;
	for (j=1; labelValue < labelLimit; j++) {
	    logLabelValue = log10(labelValue);
	    labelCenter = (int)rint(y2 - (logLabelValue-logMinLimit)*logScale);
	    if (labelCenter < y1)
	    	break;
	    if (labelCenter + fs->ascent / 2 < y2)
	    makeZLabel(lTD, fs,  
    	    			   labelValue, x - LONG_TIC_LEN - 1,
    	    			   labelCenter, maxDecimalOffset);
	    labelValue = label1Value + j*scaledInterval;
    	}
    }
    **lTD = NULL;
    /* allocate enough memory for X segments to make axis lines and
       tics at minimum tic spacing: Maximum # of tics + last tic  */
    *tics = (XSegment *)XtMalloc(sizeof(XSegment) *
    				((y2 - y1)/MIN_LOG_TIC_SPACING + 10));
    seg = *tics; *nSegs = 0;
    
    /* make the tics */
    logPowTic1Value = logPowLabel1Value - logPowInterval;
    powTicsBetweenLabels = logPowInterval/logPowTicInterval;
    maxPowTics = (logMaxLimit - logMinLimit)/logPowTicInterval + powTicsBetweenLabels + 2;
    for (i=0; i<=maxPowTics; i++) {
    	logPowTicValue = logPowTic1Value + logPowTicInterval * i;
    	powTicValue = pow(10., logPowTicValue);
    	ticY = rint(y2 - (logPowTicValue - logMinLimit) * logScale);
    	if (ticY >= y1 && ticY <= y2) {
    	    if (i % powTicsBetweenLabels == 0)
    		ticLen = LONG_TIC_LEN;
    	    else if ((i*2) % powTicsBetweenLabels == 0)
    		ticLen = MED_TIC_LEN;
    	    else
    		ticLen = SHORT_TIC_LEN;
    	    seg->x1=x; seg->x2=x-ticLen; seg->y1=seg->y2=ticY; seg++; (*nSegs)++;
    	}
    	/* make the tics within the power-of-10 intervals */
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
	    ticY = (int)rint(y2 - (log10(ticValue)-logMinLimit)*logScale);
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
    	    seg->x1=x; seg->x2=x-ticLen; seg->y1=seg->y2=ticY; seg++; (*nSegs)++;
    	}
    }
}


/* Calculate the number of pixels to the left of the decimal point, given
   a string containing a number, and an x font structure for measuring */
static int calcDecimalOffset(char *labelStr, XFontStruct *fs)
{
    char *decimalStr;

    decimalStr = strchr(labelStr, '.');
    if (decimalStr == NULL) {
    	decimalStr = strchr(labelStr, 'e');
    	if (decimalStr == NULL)
    	    return 0;
    }
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
      (double) MIN_LOG_TIC_SPACING, (double) MIN_TIC_SPACING, &dummyFactor);
    return calcTicInterval(labelInterval, minTicInterval, factor);
}

static double calcTicInterval(double labelInterval, double minTicInterval,
			      int factor)
{
    switch (factor) {
      case 0: /* labels placed at factors of 10 */
      	if (labelInterval/10 >= minTicInterval)
      	    return (labelInterval/10);
      	else if (labelInterval/2 >= minTicInterval)
      	    return (labelInterval/2);
      	else
      	    return (labelInterval);
      case 1: /* labels placed at factors of 2 */
      	if (labelInterval/2 >= minTicInterval)
      	    return (labelInterval/2);
      	else
      	    return (labelInterval);
      case 2: /* labels placed at factors of 5 */
      	if (labelInterval/5 >= minTicInterval)
      	    return (labelInterval/5);
      	else
      	    return (labelInterval);
    }
    return(labelInterval);  	    
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
    		(double)(fs->ascent * (1 + MIN_V_LOG_LABEL_SPACING)),
    		(double)(fs->ascent * (1 + MIN_V_LABEL_SPACING)), factor);
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


static void makeZLabel(labelsToDraw ***lTD, 
			       XFontStruct *fs,   double value,
			       int x, int y, int maxDecimalOffset)
{
    int decimalOffset;
    labelsToDraw *lT;
    MYMALLOC(labelsToDraw,lT,1);
    MYMALLOC(char,(lT->label),16);
    sprintf(lT->label, "%g", value);
    lT->length = strlen(lT->label);
    lT->XWidth = XTextWidth(fs, lT->label, lT->length);
    decimalOffset = calcDecimalOffset(lT->label, fs);
    lT->x = x - lT->XWidth - maxDecimalOffset + decimalOffset;
    lT->y = y + fs->ascent/2;
    lT->yCenter =y;
    **lTD = lT;
    *lTD = &(lT->next);
}

static void makeBackPlanes (Hist2DWidget w, objectsToDraw *oD)
{
    int i;
    vector v1,v2,v3;
    discreteMap *dMap = w->hist2D.discrMap;
    labelsToDraw *lT;
    XSegment *seg;
    oD->backPlanes = NULL;
    oD->nBackPlanes = 0 ;
    if (oD->zLabels == NULL) return;
    for ( i=0, lT = oD->zLabels; lT != NULL; i++, lT = lT->next);
    oD->nBackPlanes = 2 * i + 2;
    MYMALLOC(XSegment, oD->backPlanes, oD->nBackPlanes + 2);
    seg = oD->backPlanes;
    v1 =  dMap->vertex;
    v3 = subVectors(v1, xColomn(dMap->map));
    v1 = subVectors(v1, yColomn(dMap->map));	    
    v2 = subVectors(v1, xColomn(dMap->map));
    seg->x1=v2.x; seg->x2=v2.x; seg->y1=v2.y; seg->y2=v2.y+dMap->zFactor; seg++;
    seg->x1=v3.x; seg->x2=v3.x; seg->y1=v3.y; seg->y2=v3.y+dMap->zFactor; seg++;
    for (lT = oD->zLabels; lT != NULL;  lT = lT->next)
    {
        seg->x1=v1.x; seg->x2=v2.x;
        seg->y1=lT->yCenter; seg->y2=lT->yCenter - v1.y + v2.y; seg++;
        seg->x1=v2.x; seg->x2=v3.x;
        seg->y1=lT->yCenter - v1.y + v2.y;
        seg->y2=lT->yCenter - v1.y + v3.y; seg++;
    }
    if ( (seg-2)->y1 > v1.y + dMap->zFactor)
    {
        seg->x1=v1.x; seg->x2=v2.x;
        seg->y1=v1.y + dMap->zFactor; seg->y2=v2.y + dMap->zFactor; seg++;
        seg->x1=v2.x; seg->x2=v3.x;
        seg->y1=v2.y + dMap->zFactor; seg->y2=v3.y + dMap->zFactor; seg++;
        oD->nBackPlanes += 2;
    }
}    
    

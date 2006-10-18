/*******************************************************************************
*									       *
* 2DHist.c -- 2D histogram widget program interface (convinience functions)    *
*									       *
* Copyright (c) 1992 Universities Research Association, Inc.		       *
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
*      ,1992								       *
*									       *
* Written by Konstantine Iourcha					       *
*									       *
*******************************************************************************/
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h> 
#include <X11/Xutil.h>
#include <Xm/Xm.h>
#include <Xm/DrawingA.h>
#include <Xm/XmP.h>
#if XmVersion == 1002
#include <Xm/PrimitiveP.h>
#endif
#include <math.h>
#include <float.h>
#include <stdio.h>
#include "2DGeom.h"
#include "2DHistDefs.h"
#include "2DHist.h"
#include "uniLab.h"
#include "labels.h"		
#include "2DHistP.h"
#include "fltRange.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef enum _updateMode {HIST2D_SET, HIST2D_UPDATE} updateMode;

static void setZVisibleRange(Hist2DWidget w);
static void zoom1D (double *min, double *max, double factor);
static void setZMinMax(Hist2DWidget w);
static void copyBins(h2DHistSetup *hist,float *bins, float *topErrors, 
  float *bottomErrors, float **w_bins, float **w_topErrors, 
  float **w_bottomErrors, updateMode uMode);
static float aHistMin(aHistNode *aHist);
static float aHistMax(aHistNode *aHist);
static void switchToAHist(Hist2DWidget w);
static void switchToConvHist(Hist2DWidget w);
static int nodeNumber(aHistNode *aHist);
static void copyAHist(aHistStruct *aHist, aHistStruct **w_aHist);

static float aHistMin(aHistNode *aHist)
{
    float m1, m2;
    if (aHist->nextNodeOffset == 0) return (aHist->data.zData);
    m1 = aHistMin(aHist + 1);
    m2 = aHistMin(aHist + aHist->nextNodeOffset);
    return ( m1 > m2 ? m2 : m1);
}

static float aHistMax(aHistNode *aHist)
{
    float m1, m2;
    if (aHist->nextNodeOffset == 0) return (aHist->data.zData);
    m1 = aHistMax(aHist + 1);
    m2 = aHistMax(aHist + aHist->nextNodeOffset);
    return ( m1 < m2 ? m2 : m1);
}



/*
** Set min/max Z values
*/
static void setZMinMax(Hist2DWidget w)
{
    float zMin=0;
    float zMax=0;
    float *tmp;
    aHistNode *aTmp;
    if (w->hist2D.aHist!=NULL)
    {
        if (w->hist2D.aHist->aNode != NULL)
        {
            zMin = aHistMin(w->hist2D.aHist->aNode);
            zMax = aHistMax(w->hist2D.aHist->aNode);
        }
        else
            zMin = zMax = 0;    
    }
    else if (w->hist2D.sourceHistogram!=NULL)
    {
        int c=w->hist2D.sourceHistogram->nXBins * 
          w->hist2D.sourceHistogram->nYBins;
        int i;
        Boolean set = FALSE;
        if ((tmp=w->hist2D.bins)!=NULL)
        {
            i = c;
            zMin=zMax=(*tmp);
            set = TRUE;
            while (--i!=0)
            {
            	if (*++tmp<zMin) zMin=(*tmp);
            	if (*tmp>zMax) zMax=(*tmp);
            }
        }
        if ((tmp=w->hist2D.topErrors)!=NULL)
        {
            i = c;
            if (!set) 
                zMin=zMax=(*tmp);
            set = TRUE;
            while (--i!=0)
            {
            	if (*++tmp<zMin) zMin=(*tmp);
            	if (*tmp>zMax) zMax=(*tmp);
            }
        }
        if ((tmp=w->hist2D.bottomErrors)!=NULL)
        {
            i = c;
            if (!set) 
                zMin=zMax=(*tmp);
            set = TRUE;
            while (--i!=0)
            {
            	if (*++tmp<zMin) zMin=(*tmp);
            	if (*tmp>zMax) zMax=(*tmp);
            }
        }
    }
        
    if (zMin>0) zMin=0;
    setFRangeLimits(&zMin, &zMax);
    w->hist2D.zMin=zMin;
    w->hist2D.zMax=zMax;
}

static void switchToAHist(Hist2DWidget w)
{
    double xMin;
    double xMax;
    double yMin; 
    double yMax;
    double zMin; 
    double zMax;
    hist2DGetVisiblePart ((Widget) w, &xMin, &xMax, &yMin, &yMax, &zMin, &zMax);
    w->hist2D.templateDragging=FALSE;
    w->hist2D.motion= NONE;
/* set up "virtual" discrete range				*/	
    w->hist2D.xyHistoRange=makeRange2dFromRanges(
       makeRange(0,MAXPIC),
       makeRange(0,MAXPIC));
    w->hist2D.xyHistoRange.start=lShiftVector(w->hist2D.xyHistoRange.start,
        LOWBITS);
    w->hist2D.xyHistoRange.end=lShiftVector(w->hist2D.xyHistoRange.end,
        LOWBITS);
    hist2DSetVisiblePart ((Widget) w, xMin, xMax, yMin, yMax, zMin, zMax);
}    

static void switchToConvHist(Hist2DWidget w)
{
    double xMin;
    double xMax;
    double yMin; 
    double yMax;
    double zMin; 
    double zMax;
    hist2DGetVisiblePart ((Widget)w, &xMin, &xMax, &yMin, &yMax, &zMin, &zMax);
    w->hist2D.templateDragging=FALSE;
    w->hist2D.motion= NONE;
/* set up  discrete range				*/	
    w->hist2D.xyHistoRange=makeRange2dFromRanges(
       makeRange(0,w->hist2D.sourceHistogram->nXBins),
       makeRange(0,w->hist2D.sourceHistogram->nYBins));
    w->hist2D.xyHistoRange.start=lShiftVector(w->hist2D.xyHistoRange.start,
        LOWBITS);
    w->hist2D.xyHistoRange.end=lShiftVector(w->hist2D.xyHistoRange.end,
        LOWBITS);
    hist2DSetVisiblePart ((Widget)w, xMin, xMax, yMin, yMax, zMin, zMax);
}    
			  


/*
** Copy (update) new histogram data array according histogram description
*/
static void copyBins(h2DHistSetup *hist,float *bins, float *topErrors, 
  float *bottomErrors, float **w_bins, float **w_topErrors, 
  float **w_bottomErrors, updateMode uMode)
{
    if(uMode ==  HIST2D_SET)
    {
        MYFREE(*w_bins); *w_bins = NULL;
        MYFREE(*w_topErrors); *w_topErrors = NULL;
        MYFREE(*w_bottomErrors);  *w_bottomErrors = NULL;
    }
    if (hist==NULL)
    {
       return;
    }
    {
        float *sTmp, *rTmp, *dTmp;
        int c=hist->nXBins * hist->nYBins;
        int i;
/* recreate all private data arrays to allow change size of histogram
   in HIST2D_SET mode							*/    
        if (bins!=NULL)
        {
            if (*w_bins == NULL) MYMALLOC(float,*w_bins,c);  
            for (i=0, sTmp=bins, rTmp= *w_bins ; i < c ; i++, sTmp++, rTmp++)
                *rTmp = *sTmp;
        }
        if (*w_bins != NULL && topErrors!=NULL)
        {
            if (*w_topErrors == NULL) MYMALLOC(float,*w_topErrors,c);  
            for (i=0, sTmp=topErrors, rTmp= *w_topErrors, dTmp= *w_bins; 
              i < c ; i++, sTmp++, rTmp++, dTmp++)
/* use abs of an error value instead of the value			*/
                if (*sTmp > 0 )
                    *rTmp = *dTmp + *sTmp;
                else
                    *rTmp = *dTmp - *sTmp;  
        }  
        else 
        {
            MYFREE(*w_topErrors); *w_topErrors = NULL;    
        }
        if (*w_bins != NULL && bottomErrors!=NULL)
        {
            if (*w_bottomErrors == NULL) MYMALLOC(float,*w_bottomErrors,c);  
            for (i=0, sTmp=bottomErrors, rTmp= *w_bottomErrors, dTmp= *w_bins; 
              i < c ; i++, sTmp++, rTmp++, dTmp++)
/* use abs of an error value instead of the value			*/
                if (*sTmp < 0 )
                    *rTmp = *dTmp + *sTmp;
                else
                    *rTmp = *dTmp - *sTmp;
        }
        else
        {
            MYFREE(*w_bottomErrors); *w_bottomErrors = NULL;
        }
    }
}

static int nodeNumber(aHistNode *aHist)
{
    if (aHist == NULL) return(0);
    if (aHist->nextNodeOffset == 0) return (1);
    return (nodeNumber(aHist + 1) + 
      nodeNumber(aHist + aHist->nextNodeOffset) + 1);
}

static void copyAHist(aHistStruct *aHist, aHistStruct **w_aHist)
{
    int n;
    if (*w_aHist != NULL)
    {
        if ((*w_aHist)->aNode != NULL)
            MYFREE((*w_aHist)->aNode);
        MYFREE(*w_aHist);
    }        
    *w_aHist = NULL;
    if (aHist == NULL) return;
    MYMALLOC(aHistStruct,*w_aHist,1);
    **w_aHist = *aHist;
/* calculate number of nodes */
    n = nodeNumber(aHist->aNode);
    MYMALLOC(aHistNode,(*w_aHist)->aNode,n);
    for (n--;n >=0; n--)
        ((*w_aHist)->aNode)[n] = (aHist->aNode)[n];
}          
    

/*
** static void setZVisibleRange(Hist2DWidget w)
** 
** static function to set up initial values for
** w->hist2D.zVisibleStart and w->hist2D.zVisibleEnd
**
** uses w->hist2D.zMin,w->hist2D.zMax,w->hist2D.zLogScaling
**
*/
static void setZVisibleRange(Hist2DWidget w)
{
    w->hist2D.zVisibleStart = w->hist2D.zMin;
    w->hist2D.zVisibleEnd = w->hist2D.zMax;
    if (w->hist2D.zLogScaling)
         if ( w->hist2D.zMin < 0.5)
         {
             float zm = w->hist2D.zMin;
             w->hist2D.zMin = 0.5;
             w->hist2D.zLogScaling = setFRangeLogScale (&(w->hist2D.zVisibleStart), 
               &(w->hist2D.zVisibleEnd), w->hist2D.zMin, w->hist2D.zMax);
             w->hist2D.zMin = zm;
         }
         else
             w->hist2D.zLogScaling = setFRangeLogScale (&(w->hist2D.zVisibleStart), 
               &(w->hist2D.zVisibleEnd), w->hist2D.zMin, w->hist2D.zMax);
}    
/* 
** void hist2DSetHistogramm (Widget wg, h2DHistSetup *hist)
**
** 	wg	Specifies the widget
**	hist 	Specifies the gistogram for viewing
**
**		Copies all histogramm related data to the widget inner copy
**	of h2DHistSetup structure and starts to represent them with default 
**	representation.
**		hist.bins==NULL is allowed. In such cases as hist.nXBins==0 
**	no histogram is setted up.
**		View angles are setted up to defualt position, viewable
**	area is setted up to whole histogram area in X-Y axes and from
**	0 to MaxZ in Z.
**		If, for some reason, call occure during any kind of mouse
**	controlled dragging of previous object, the dragging mode will be
** 	locked, and to restart it mouse button should be realised and pressed
**	again.
*/
void hist2DSetHistogram (Widget wg, h2DHistSetup *hist)
{
    Hist2DWidget w = (Hist2DWidget) wg;
    /* make private copies of data					*/
    destroySrcHist(w->hist2D.sourceHistogram);
    MYFREE(w->hist2D.bins);
    w->hist2D.bins=NULL;
    if (w->hist2D.aHist!=NULL)
        MYFREE(w->hist2D.aHist->aNode);
    MYFREE(w->hist2D.aHist);
    w->hist2D.aHist=NULL;
    MYFREE(w->hist2D.topErrors);
    w->hist2D.topErrors=NULL;
    MYFREE(w->hist2D.bottomErrors);
    w->hist2D.bottomErrors=NULL;
    w->hist2D.fiViewAngle = M_PI * 5. /4.;
    w->hist2D.psiViewAngle = M_PI / 6.;
    adjustViewAngles(&w->hist2D.fiViewAngle, &w->hist2D.psiViewAngle);
    w->hist2D.zLogScaling = FALSE;
    w->hist2D.sourceHistogram=copyHistogram(hist);
    if (w->hist2D.sourceHistogram!=NULL)
    {
        w->hist2D.sourceHistogram->bins=NULL;
	if ((hist->nXBins==0)||(hist->nYBins==0)) 
	{
    	    XtWarning("Histogram with zero bins' number was not setted up");
            destroySrcHist(w->hist2D.sourceHistogram);
            w->hist2D.sourceHistogram=NULL;
	}
    }
    resetHist(w);
    if (w->hist2D.sourceHistogram==NULL)
    {
        w->hist2D.templateDragging=FALSE;
        w->hist2D.motion = NONE;
        makeAllGraphics(w);
    	redisplay((Widget)w,NULL,NULL);
 	return;
    }
    copyBins(hist, hist->bins, NULL, 
      NULL, &(w->hist2D.bins), &(w->hist2D.topErrors), 
      &(w->hist2D.bottomErrors), HIST2D_SET);
    if (w->hist2D.bins!=NULL)
    {
    /* Set up visible range according histogram 		*/
	setZMinMax(w);
	setZVisibleRange(w);
    }
/* Set up visible range according histogram 		*/
    setZMinMax(w);
    setZVisibleRange(w);
    w->hist2D.xyVisibleRange = w->hist2D.xyHistoRange;
    w->hist2D.templateDragging=FALSE;
    w->hist2D.motion = NONE;
    makeAllGraphics(w);
    redisplay((Widget)w,NULL,NULL);
}
/* 
** void hist2DClearHistogramData (Widget wg)
**
** 	wg	Specifies the widget
**
**	Removes histogram data end errors data from the widget
*/
void hist2DClearHistogramData (Widget wg)
{
    Hist2DWidget w = (Hist2DWidget) wg;
    copyBins(w->hist2D.sourceHistogram, NULL, NULL, NULL, 
      &(w->hist2D.bins), &(w->hist2D.topErrors), 
      &(w->hist2D.bottomErrors), HIST2D_SET);
    if (w->hist2D.aHist != NULL)
        MYFREE(w->hist2D.aHist->aNode);  
    MYFREE(w->hist2D.aHist);
    w->hist2D.aHist=NULL;
    setZMinMax(w);
    setZVisibleRange(w);
    w->hist2D.templateDragging=FALSE;
    w->hist2D.motion = NONE;
    makeAllGraphics(w);
    redisplay((Widget)w,NULL,NULL);
}
/* 
** void hist2DSetAdaptiveHistogramData (Widget wg, aHistStruct *aHist, 
**	scalingMode sMode)
**
** 	wg	Specifies the widget
**	aHist 	Specifies new histogramm data
**	sMode   Specifies how to handle visible range of histogram
**
**		Copies histogram  data tree to the widget inner 
**	copy. Source coordinate  XY range should be setted up first by
**	hist2DsetHistogram () call with possibble empty data and
**	arbitrary bin numbers NOT equal to 0. 
**		aHist==NULL is allowed 
**		If this call is isued before any hist2DSetHistogramm calls
**	it is ignored
**		
*/
void hist2DSetAdaptiveHistogramData (Widget wg, aHistStruct *aHist,
     scalingMode sMode)
{
    Hist2DWidget w = (Hist2DWidget) wg;
    float zMin, zMax;
    Boolean pb, pa;
    if (w->hist2D.sourceHistogram==NULL)
    {
        XtWarning("Bins data provided without histogram setted up");
        return;
    }
    pb = (w->hist2D.aHist != NULL || w->hist2D.bins != NULL);
    pa = (w->hist2D.aHist != NULL);
/* clear conventional histogram data if any */    
    copyBins(w->hist2D.sourceHistogram, NULL, NULL, NULL, 
      &(w->hist2D.bins), &(w->hist2D.topErrors), 
      &(w->hist2D.bottomErrors), HIST2D_SET);
    copyAHist(aHist, &(w->hist2D.aHist));
    if (w->hist2D.aHist == NULL) 
    {
        if (pa) 
        {
	    switchToConvHist(w);
	}
	else if (w->hist2D.motion == Z_LOW_SCALING || 
	    w->hist2D.motion == Z_HIGH_SCALING ||
	    w->hist2D.motion == Z_PANNING)
	{
	     w->hist2D.templateDragging=FALSE;
	     w->hist2D.motion= NONE;
	}
        if (w->hist2D.motion == NONE)
        {
            makeAllGraphics(w);
     	    redisplay((Widget)w,NULL,NULL);
     	}
 	return;
    }
    if (!pa)
    {
	switchToAHist(w);
    }
    if (pb)
    {
	zMin = w->hist2D.zMin;
	zMax = w->hist2D.zMax;
        setZMinMax(w);
    }
    else 
    {
	w->hist2D.templateDragging=FALSE;
	w->hist2D.motion= NONE;
        setZMinMax(w);
/* set some resonable values to z range and z visible range in case they were 
   not setted previously becase bins were NULL */
	zMin = w->hist2D.zMin;
	zMax = w->hist2D.zMax;
	setZVisibleRange(w);
    }
    switch (sMode) {
        case HIST2D_SCALING :
	    if (w->hist2D.motion == Z_LOW_SCALING || 
		w->hist2D.motion == Z_HIGH_SCALING ||
		w->hist2D.motion == Z_PANNING)
	    {
	        if (zMax > w->hist2D.zMax) w->hist2D.zMax = zMax;
	        if (zMin < w->hist2D.zMin) w->hist2D.zMin = zMin;
	    }
	    else
	        setZVisibleRange(w);
            break;
	case HIST2D_RESCALE_AT_MAX:
	    if ( (w->hist2D.zVisibleEnd==zMax) &&
	      !(w->hist2D.motion == Z_LOW_SCALING || 
		w->hist2D.motion == Z_HIGH_SCALING ||
		w->hist2D.motion == Z_PANNING))
   	        w->hist2D.zVisibleEnd = w->hist2D.zMax; 
	    else
   	        if (zMax > w->hist2D.zMax) w->hist2D.zMax = zMax;
	    if (( w->hist2D.zVisibleStart==zMin) &&
	      !(w->hist2D.motion == Z_LOW_SCALING || 
		w->hist2D.motion == Z_HIGH_SCALING ||
		w->hist2D.motion == Z_PANNING)) 
	    {
	        if (!w->hist2D.zLogScaling || (w->hist2D.zMin > 0))
	            w->hist2D.zVisibleStart = w->hist2D.zMin; 
	        else
	            w->hist2D.zVisibleStart = FLT_MIN; 
	    }
	    else 
		if (zMin < w->hist2D.zMin) w->hist2D.zMin = zMin;
	    break;
	case HIST2D_NO_SCALING :
	    if(zMin < w->hist2D.zMin)
	        w->hist2D.zMin=zMin;
	    if (zMax > w->hist2D.zMax)
	        w->hist2D.zMax = zMax;
	    break;
    }
    adjustFRange(&(w->hist2D.zVisibleStart), &(w->hist2D.zVisibleEnd), 
      w->hist2D.zMin, w->hist2D.zMax, w->hist2D.zLogScaling);
    if (w->hist2D.motion== NONE || (! w->hist2D.templateDragging))
    {
	makeAllGraphics(w);
	redisplay((Widget)w,NULL,NULL);
    }
    else  
        drawNewPicture(w);
    
    
}
/* 
** void hist2DUpdateHistogramData (Widget wg, float *data, 
** 	float *topErrors, float *bottomErrors, scalingMode sMode)
**
** 	wg	Specifies the widget
**	data 	Specifies new histogramm data
**      topErrors 	Specifies new top errors data
**      bottomErrors 	Specifies new bottom errors data
**	sMode   Specifies how to handle visible range of histogram
**      uMode   Specifies how to perform update if some of data
**		pointer are NULL
**
**		Copies histogram  data array to the widget inner 
**	copy according size parameters in the inner copy of h2DHistSetup structure
**	and starts to represent them with current type of representation.  
**		data==NULL is allowed
**		If this call is isued before any hist2DSetHistogramm calls
**	it is ignored
**		
*/
void hist2DUpdateHistogramData (Widget wg, float *data, 
  float *topErrors, float *bottomErrors, scalingMode sMode)
{
    Hist2DWidget w = (Hist2DWidget) wg;
    float zMin, zMax;
    Boolean pb;
    if (w->hist2D.sourceHistogram==NULL)
    {
        XtWarning("Bins data provided without histogram setted up");
        return;
    }
    pb = (w->hist2D.bins != NULL || w->hist2D.aHist != NULL);
    if (w->hist2D.aHist != NULL)
    {
        MYFREE(w->hist2D.aHist->aNode);
	MYFREE(w->hist2D.aHist);
	w->hist2D.aHist=NULL;
	switchToConvHist(w);
    }
    copyBins(w->hist2D.sourceHistogram, data, topErrors, 
      bottomErrors, &(w->hist2D.bins), &(w->hist2D.topErrors), 
      &(w->hist2D.bottomErrors), HIST2D_UPDATE);
    if (w->hist2D.bins == NULL) 
    {
	if (w->hist2D.motion == Z_LOW_SCALING || 
	    w->hist2D.motion == Z_HIGH_SCALING ||
	    w->hist2D.motion == Z_PANNING)
	{
	     w->hist2D.templateDragging=FALSE;
	     w->hist2D.motion= NONE;
	}
        if (w->hist2D.motion == NONE)
        {
            makeAllGraphics(w);
     	    redisplay((Widget)w,NULL,NULL);
     	}
 	return;
    }
    if (pb)
    {
	zMin = w->hist2D.zMin;
	zMax = w->hist2D.zMax;
        setZMinMax(w);
    }
    else 
    {
	w->hist2D.templateDragging=FALSE;
	w->hist2D.motion= NONE;
        setZMinMax(w);
/* set some resonable values to z range and z visible range in case they were 
   not setted previously becase bins were NULL */
	zMin = w->hist2D.zMin;
	zMax = w->hist2D.zMax;
	setZVisibleRange(w);
    }
    switch (sMode) {
        case HIST2D_SCALING :
	    if (w->hist2D.motion == Z_LOW_SCALING || 
		w->hist2D.motion == Z_HIGH_SCALING ||
		w->hist2D.motion == Z_PANNING)
	    {
	        if (zMax > w->hist2D.zMax) w->hist2D.zMax = zMax;
	        if (zMin < w->hist2D.zMin) w->hist2D.zMin = zMin;
	    }
	    else
	        setZVisibleRange(w);
            break;
	case HIST2D_RESCALE_AT_MAX:
	    if ( (w->hist2D.zVisibleEnd==zMax) &&
	      !(w->hist2D.motion == Z_LOW_SCALING || 
		w->hist2D.motion == Z_HIGH_SCALING ||
		w->hist2D.motion == Z_PANNING))
   	        w->hist2D.zVisibleEnd = w->hist2D.zMax; 
	    else
   	        if (zMax > w->hist2D.zMax) w->hist2D.zMax = zMax;
	    if (( w->hist2D.zVisibleStart==zMin) &&
	      !(w->hist2D.motion == Z_LOW_SCALING || 
		w->hist2D.motion == Z_HIGH_SCALING ||
		w->hist2D.motion == Z_PANNING)) 
	    {
	        if (!w->hist2D.zLogScaling || (w->hist2D.zMin > 0))
	            w->hist2D.zVisibleStart = w->hist2D.zMin; 
	        else
	            w->hist2D.zVisibleStart = FLT_MIN; 
	    }
	    else 
		if (zMin < w->hist2D.zMin) w->hist2D.zMin = zMin;
	    break;
	case HIST2D_NO_SCALING :
	    if(zMin < w->hist2D.zMin)
	        w->hist2D.zMin=zMin;
	    if (zMax > w->hist2D.zMax)
	        w->hist2D.zMax = zMax;
	    break;
    }
    adjustFRange(&(w->hist2D.zVisibleStart), &(w->hist2D.zVisibleEnd), 
      w->hist2D.zMin, w->hist2D.zMax, w->hist2D.zLogScaling);
    if (w->hist2D.motion== NONE || (! w->hist2D.templateDragging))
    {
	makeAllGraphics(w);
	redisplay((Widget)w,NULL,NULL);
    }
    else  
        drawNewPicture(w);
    
    
} 
/* 
** void hist2DSetRebinnedData (Widget wg, float *data, float *topErrors,
**			      float *bottomErrors, 
**			      int nXBins, int nYBins,
**			      float xMin, float xMax, float yMin, float yMax,
**			      scalingMode sMode)
**
** 	wg	Specifies the widget
**	data 	Specifies new histogramm data
**      topErrors 	Specifies new top errors data
**      bottomErrors 	Specifies new bottom errors data
**	nXBins	new number of bins along X axis
**	nYBins	new number of bins along Y axis
**	xMin, xMax range in coordinates of object which is represented
**		by current bins
**	yMin, yMax range in coordinates of object which is represented
**		by current bins
**	sMode   Specifies how to handle visible range of histogram
**
**		Copies histogram  data array to the widget inner 
**	copy of h2DHistSetup structure according size parameters in the inner
**	copy and starts to represent them with current type of representation.  
**		data==NULL is allowed
**		If this call is isued before any hist2DSetHistogramm calls
**	it is ignored
**		
*/
void hist2DSetRebinnedData (Widget wg, float *data, float *topErrors,
			      float *bottomErrors, int nXBins, int nYBins,
			      float xMin, float xMax, float yMin, float yMax,
			      scalingMode sMode)
{
    Hist2DWidget w = (Hist2DWidget) wg;
    float zMin, zMax;
    float xMinOld, xMaxOld;
    float yMinOld, yMaxOld;
    int nXBinsOld, nYBinsOld;
    Boolean pb;
    
    if (w->hist2D.sourceHistogram==NULL)
    {
        XtWarning("Bins data provided without histogram setted up");
        return;
    }
    if (w->hist2D.aHist!=NULL)
    {
        XtWarning("Impossible to set rebinned data in adaptive histogram mode");
        return;
    }

    xMinOld = w->hist2D.sourceHistogram->xMin;
    xMaxOld = w->hist2D.sourceHistogram->xMax;
    yMinOld = w->hist2D.sourceHistogram->yMin;
    yMaxOld = w->hist2D.sourceHistogram->yMax;
    nXBinsOld = w->hist2D.sourceHistogram->nXBins;
    nYBinsOld = w->hist2D.sourceHistogram->nYBins;
    w->hist2D.sourceHistogram->nXBins = nXBins;
    w->hist2D.sourceHistogram->nYBins = nYBins;
    w->hist2D.sourceHistogram->xMin = xMin;
    w->hist2D.sourceHistogram->xMax = xMax;
    w->hist2D.sourceHistogram->yMin = yMin;
    w->hist2D.sourceHistogram->yMax = yMax;
    if ((nXBins==0)||(nYBins==0)) 
    {
    	XtWarning("Histogram with zero bins' number was not setted up");
        destroySrcHist(w->hist2D.sourceHistogram);
        w->hist2D.sourceHistogram=NULL;
    }
    resetHist(w);
    pb =(w->hist2D.bins != NULL);
    copyBins(w->hist2D.sourceHistogram, data, topErrors, 
      bottomErrors, &(w->hist2D.bins), &(w->hist2D.topErrors), 
      &(w->hist2D.bottomErrors), HIST2D_SET);
    if ((w->hist2D.sourceHistogram==NULL)|| (data==NULL))
    {
        w->hist2D.templateDragging=FALSE;
        w->hist2D.motion = NONE;
	if (data==NULL)
            w->hist2D.xyVisibleRange = w->hist2D.xyHistoRange;
        makeAllGraphics(w);
    	redisplay((Widget)w,NULL,NULL);
 	return;
    }
    if (pb)
    {
	zMin = w->hist2D.zMin;
	zMax = w->hist2D.zMax;
        setZMinMax(w);
    }
    else 
    {
        setZMinMax(w);
	zMin = w->hist2D.zMin;
	zMax = w->hist2D.zMax;
	w->hist2D.zVisibleStart=zMin;
	w->hist2D.zVisibleEnd= zMax;
    }
    switch (sMode) {
        case HIST2D_SCALING :
            w->hist2D.xyVisibleRange=w->hist2D.xyHistoRange;
            setZVisibleRange(w);
            break;
        case HIST2D_NO_SCALING :
	case HIST2D_RESCALE_AT_MAX :
/* we use "proportional scaling during rebinning in this case to keep pictures
   "compatible" */
   	    {
   	        double zMult;
   	        double dtmp;
   	        zMult = (double)nXBins / ((double)xMax - (double)xMin)
   	            / (double)nXBinsOld * ((double)xMaxOld - (double)xMinOld) *
   	            (double)nYBins / ((double)yMax - (double)yMin)
   	            / (double)nYBinsOld * ((double)yMaxOld - (double)yMinOld);
		if (sMode == HIST2D_RESCALE_AT_MAX && w->hist2D.zVisibleEnd==zMax)
   	            w->hist2D.zVisibleEnd = w->hist2D.zMax;
		else
		{
   	            w->hist2D.zVisibleEnd /= zMult;
   	            if (zMax > w->hist2D.zMax)
	                w->hist2D.zMax = zMax;
	        }
		if (sMode == HIST2D_RESCALE_AT_MAX && w->hist2D.zVisibleStart==zMin)
		{
	            if (!w->hist2D.zLogScaling || (w->hist2D.zMin > 0))
	        	w->hist2D.zVisibleStart = w->hist2D.zMin;
	            else
	        	w->hist2D.zVisibleStart = FLT_MIN;
		}
		else 
		{
   	            w->hist2D.zVisibleStart /= zMult;
		    if (zMin < w->hist2D.zMin)
	        	w->hist2D.zMin = zMin;
		}
		if (w->hist2D.zVisibleEnd > w->hist2D.zMax)
	            w->hist2D.zMax = w->hist2D.zVisibleEnd;
		if (w->hist2D.zVisibleStart < w->hist2D.zMin)
	            w->hist2D.zMin = w->hist2D.zVisibleStart;
	        if (w->hist2D.xyVisibleRange.start.x != 0)
	        {
	            dtmp = (double)w->hist2D.xyVisibleRange.start.x / 
	              (double) nXBinsOld / (double)MAXPIC * 
	              (xMaxOld - xMinOld) + xMinOld;
	            if (dtmp < xMin) dtmp = xMin;
	            if (dtmp > xMax) dtmp = xMax;
	            w->hist2D.xyVisibleRange.start.x = (dtmp - xMin) /
	              (xMax - xMin) * (double)nXBins * (double) MAXPIC;
	        }
	        if (w->hist2D.xyVisibleRange.start.y != 0)
	        {
	            dtmp = (double)w->hist2D.xyVisibleRange.start.y /
	              (double) nYBinsOld / (double)MAXPIC * 
	              (yMaxOld - yMinOld) + yMinOld;
	            if (dtmp < yMin) dtmp = yMin;
	            if (dtmp > yMax) dtmp = yMax;
	            w->hist2D.xyVisibleRange.start.y = (dtmp - yMin) /
	              (yMax - yMin) * (double)nYBins * (double) MAXPIC;
	        }
	        if (w->hist2D.xyVisibleRange.end.x != nXBinsOld << LOWBITS)
	        {
	            dtmp = (double)w->hist2D.xyVisibleRange.end.x / 
	              (double) nXBinsOld / (double)MAXPIC * 
	              (xMaxOld - xMinOld) + xMinOld;
	            if (dtmp < xMin) dtmp = xMin;
	            if (dtmp > xMax) dtmp = xMax;
	            w->hist2D.xyVisibleRange.end.x = (dtmp - xMin) /
	              (xMax - xMin) * (double)nXBins * (double) MAXPIC;
	        }
	        else
	            w->hist2D.xyVisibleRange.end.x=w->hist2D.xyHistoRange.end.x;
	        if (w->hist2D.xyVisibleRange.end.y != nYBinsOld << LOWBITS)
	        {
	            dtmp = (double)w->hist2D.xyVisibleRange.end.y /
	              (double) nYBinsOld / (double)MAXPIC * 
	              (yMaxOld - yMinOld) + yMinOld;
	            if (dtmp < yMin) dtmp = yMin;
	            if (dtmp > yMax) dtmp = yMax;
	            w->hist2D.xyVisibleRange.end.y = (dtmp - yMin) /
	              (yMax - yMin) * (double)nYBins * (double) MAXPIC;
	        }
	        else
	            w->hist2D.xyVisibleRange.end.y=w->hist2D.xyHistoRange.end.y;
                w->hist2D.xyVisibleRange = 
                  adjustRange2dToRange2d(w->hist2D.xyVisibleRange,
                  w->hist2D.xyHistoRange,MAXPIC);
	    	break;
	    }
    }
    adjustFRange(&(w->hist2D.zVisibleStart), &(w->hist2D.zVisibleEnd), 
      w->hist2D.zMin, w->hist2D.zMax, w->hist2D.zLogScaling);
    w->hist2D.templateDragging=FALSE;
    w->hist2D.motion= NONE;
    makeAllGraphics(w); /* new Z ticmarks and labels are needed, 
makeAllGraphics  calls makeImage by itself  */
    redisplay((Widget)w,NULL,NULL);
}

/*
** void hist2DSetVisiblePart (Widget wg, 
**			    double xMin, double xMax,
**			    double yMin, double yMax,
**			    double zMin, double zMax
**			    )
**
** 	wg	Specifies the widget
**	xMin	Start of visible part along x axis if object coordinates
**	xMax	End of visible part along x axis if object coordinates
**	yMin	Start of visible part along y axis if object coordinates
**	yMax	End of visible part along y axis if object coordinates
**	zMin	Start of visible part along z axis if object coordinates
**	zMax	End of visible part along z axis if object coordinates
**
**		Specifies part of histogram to draw (and redraw this part)
**	It specifeied part in X-Y plane larger then that of histogram installed, 
**	the intersection of histogramm and specifeied square is used as visible 
**	part. 
**	
**		If specified part shorter than one bin (for X and Y axes,
**	it will be spread to one bin width
**		In cases of (zMin == zMax), zMax = zMin + 1  will be used
**	instead of requested zMax
**				   
*/

void hist2DSetVisiblePart (Widget wg, 
			    double xMin, double xMax,
			    double yMin, double yMax,
			    double zMin, double zMax
			    )
{
    Hist2DWidget w = (Hist2DWidget) wg;
    double dtmp;
    if (w->hist2D.zMin > zMin) w->hist2D.zMin = zMin;
    if (w->hist2D.zMax < zMax ) w->hist2D.zMax = zMax;
    if (!w->hist2D.zLogScaling || zMin > FLT_MIN)
        w->hist2D.zVisibleStart = zMin;
    else
        w->hist2D.zVisibleStart = 0.5;
    w->hist2D.zVisibleEnd = zMax;
    adjustFRange(&(w->hist2D.zVisibleStart), &(w->hist2D.zVisibleEnd), 
      w->hist2D.zMin, w->hist2D.zMax, w->hist2D.zLogScaling);
    w->hist2D.xyVisibleRange.start.x = 
      (xMin - w->hist2D.sourceHistogram->xMin) /
      (w->hist2D.sourceHistogram->xMax - w->hist2D.sourceHistogram->xMin) *
      (w->hist2D.xyHistoRange.end.x - w->hist2D.xyHistoRange.start.x);
    w->hist2D.xyVisibleRange.start.y = 
      (yMin - w->hist2D.sourceHistogram->yMin) /
      (w->hist2D.sourceHistogram->yMax - w->hist2D.sourceHistogram->yMin) * 
      (w->hist2D.xyHistoRange.end.y - w->hist2D.xyHistoRange.start.y);
    w->hist2D.xyVisibleRange.end.x = 
      (xMax - w->hist2D.sourceHistogram->xMin) /
      (w->hist2D.sourceHistogram->xMax - w->hist2D.sourceHistogram->xMin) * 
      (w->hist2D.xyHistoRange.end.x - w->hist2D.xyHistoRange.start.x);
    w->hist2D.xyVisibleRange.end.y = 
      (yMax - w->hist2D.sourceHistogram->yMin) /
      (w->hist2D.sourceHistogram->yMax - w->hist2D.sourceHistogram->yMin) * 
      (w->hist2D.xyHistoRange.end.y - w->hist2D.xyHistoRange.start.y);
    w->hist2D.xyVisibleRange = 
      adjustRange2dToRange2d(w->hist2D.xyVisibleRange,
      w->hist2D.xyHistoRange,MAXPIC);
    w->hist2D.templateDragging=FALSE;
    w->hist2D.motion= NONE;
    makeAllGraphics(w); /* new Z ticmarks and labels are needed, makeAllGraphics
    calls makeImage by itself  */
/*	    makeImage (w);	*/
    redisplay((Widget)w,NULL,NULL);
}

/*
** void hist2DResetView (Widget wg)
** 
** set view parameters accoding actual data to see the whole histogram
** (does not affect log/liner type of scaling)
*/
void hist2DResetView (Widget wg)
{
    Hist2DWidget w = (Hist2DWidget) wg;
    w->hist2D.fiViewAngle = M_PI * 5. /4.;
    w->hist2D.psiViewAngle = M_PI / 6.;
    adjustViewAngles(&w->hist2D.fiViewAngle, &w->hist2D.psiViewAngle);
    if (w->hist2D.sourceHistogram!=NULL) 
        w->hist2D.xyVisibleRange = w->hist2D.xyHistoRange;
    if ( w->hist2D.bins!=NULL || w->hist2D.aHist!=NULL)
    {
	setZMinMax(w);
	setZVisibleRange(w);
    }
        w->hist2D.templateDragging=FALSE;
    w->hist2D.motion = NONE;
    makeAllGraphics(w);
    redisplay((Widget)w,NULL,NULL);
}

/*
**
** static void zoom1D (double *min, double *max, double factor)
**
** make zoom in (factor > 1) or zoom out (factor < 1) for 1D range
**
*/
static void zoom1D (double *min, double *max, double factor)
{
    double d;
    d = (*min + *max)/2;
    *min -= d;
    *max -= d;
    *min *= factor;
    *max *= factor;
    *min += d;
    *max += d;
}


/*
**  void hist2DZoom(Widget wg, double factor)
**
**  make zoom in (factor < 1) or zoom out (factor > 1)
**
** 	wg	Specifies the widget
**	factor  Factor to "multiplay" or pull the picture from carrent
**		center of visual ranges. After zoom clipping to limit
**		ranges is applied
**
*/
void hist2DZoom(Widget wg, double factor)
{
    Hist2DWidget w = (Hist2DWidget) wg;
    double xMin, xMax, yMin, yMax, zMin, zMax;
    if (factor < 0) factor = -factor;
    hist2DGetVisiblePart ( wg, &xMin, &xMax, &yMin, &yMax, &zMin, &zMax);
    zoom1D(&xMin, &xMax, factor);
    zoom1D(&yMin, &yMax, factor);
    if (w->hist2D.zLogScaling)
    {
        Boolean ll = (zMin >= .5 && factor > 1.);
        zMin = log10(zMin);
        zMax = log10(zMax);
        zoom1D(&zMin, &zMax, factor);
        if (zMin < FLT_MIN_10_EXP)
            zMin = FLT_MIN_10_EXP;
        if (zMax > FLT_MAX_10_EXP)
            zMax = FLT_MAX_10_EXP;
        zMin = pow(10.,zMin);
        zMax = pow(10.,zMax);
        if (ll)
            if (zMin < 0.5 ) zMin =0.5;
    }
    else
        zoom1D(&zMin, &zMax, factor);
    if (zMin < w->hist2D.zMin) zMin = w->hist2D.zMin;
    if (zMax > w->hist2D.zMax) zMax = w->hist2D.zMax;
    hist2DSetVisiblePart (wg, xMin,  xMax, yMin,  yMax, zMin,  zMax);
}

/*
**  void hist2DZoomIn(Widget wg)
**
**  make zoom in with factor 2.
**
** 	wg	Specifies the widget
**
*/
void hist2DZoomIn(Widget wg)
{
    hist2DZoom(wg, 0.5);
}

/*
**  void hist2DZoomOut(Widget wg)
**
**  make zoom out -  with factor 2.
**
** 	wg	Specifies the widget
**
*/
void hist2DZoomOut(Widget wg)
{
    hist2DZoom(wg, 2.0);
}

/*
** void hist2DGetVisiblePart (Widget wg, 
**			    double *xMin, double *xMax,
**			    double *yMin, double *yMax,
**			    double *zMin, double *zMax
**			    )
**
** 	wg	Specifies the widget
**	xMin	Start of visible part along x axis if object coordinates
**	xMax	End of visible part along x axis if object coordinates
**	yMin	Start of visible part along y axis if object coordinates
**	yMax	End of visible part along y axis if object coordinates
**	zMin	Start of visible part along z axis if object coordinates
**	zMax	End of visible part along z axis if object coordinates
**
**	Returns current visible range of histogram in object 
**	coordinates				   
*/

 void hist2DGetVisiblePart (Widget wg, 
			    double *xMin, double *xMax,
			    double *yMin, double *yMax,
			    double *zMin, double *zMax
			    )
{
    Hist2DWidget w = (Hist2DWidget) wg;
    *xMin = (double)w->hist2D.xyVisibleRange.start.x / 
      (double) (w->hist2D.xyHistoRange.end.x - w->hist2D.xyHistoRange.start.x) *
      (w->hist2D.sourceHistogram->xMax - w->hist2D.sourceHistogram->xMin) +
      w->hist2D.sourceHistogram->xMin;
    *yMin = (double)w->hist2D.xyVisibleRange.start.y / 
      (double) (w->hist2D.xyHistoRange.end.y - w->hist2D.xyHistoRange.start.y) *
      (w->hist2D.sourceHistogram->yMax - w->hist2D.sourceHistogram->yMin) +
      w->hist2D.sourceHistogram->yMin;
    *xMax = (double)w->hist2D.xyVisibleRange.end.x / 
      (double) (w->hist2D.xyHistoRange.end.x - w->hist2D.xyHistoRange.start.x) *
      (w->hist2D.sourceHistogram->xMax - w->hist2D.sourceHistogram->xMin) +
      w->hist2D.sourceHistogram->xMin;
    *yMax = (double)w->hist2D.xyVisibleRange.end.y /
      (double) (w->hist2D.xyHistoRange.end.y - w->hist2D.xyHistoRange.start.y) *
      (w->hist2D.sourceHistogram->yMax - w->hist2D.sourceHistogram->yMin) +
      w->hist2D.sourceHistogram->yMin;
    *zMin = w->hist2D.zVisibleStart;
    *zMax = w->hist2D.zVisibleEnd;
}     

/* 	
** void hist2DSetViewAngles (Widget wg, double fi, double psi)
**
** 	wg	Specifies the widget
**	fi	Specifies the turn of view direction projection to XY plane
**		from X axe.
**	psi 	Specifies the angle between view direction and Z axe.
**
**		Set up view angles for the 3D histogram representation
**	and redraw a visible part
*/

void hist2DSetViewAngles (Widget wg, double fi, double psi)
{
    Hist2DWidget w = (Hist2DWidget) wg;
    w->hist2D.fiViewAngle = fi;
    w->hist2D.psiViewAngle = psi;
    adjustViewAngles(&w->hist2D.fiViewAngle, &w->hist2D.psiViewAngle);
    w->hist2D.templateDragging=FALSE;
    w->hist2D.motion= NONE;
    makeAllGraphics(w); /* new Z ticmarks and labels are needed, makeAllGraphics
    calls makeImage by itself  */
/*	    makeImage (w);	*/
    redisplay((Widget)w,NULL,NULL);
}

/* 	
** void hist2DGetViewAngles (Widget wg, double *fi, *double psi)
**
** 	wg	Specifies the widget
**	fi	Specifies the turn of view direction projection to XY plane
**		from X axe.
**	psi 	Specifies the angle between view direction and Z axe.
**
**		Get current view angles of the 3D histogram representation
*/

void hist2DGetViewAngles (Widget wg, double *fi, double *psi)
{
    Hist2DWidget w = (Hist2DWidget) wg;
    *fi = w->hist2D.fiViewAngle ;
    *psi = w->hist2D.psiViewAngle ;
}


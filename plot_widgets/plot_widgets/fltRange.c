/*******************************************************************************
*									       *
* fltRange.c -- functions to maintane floating range to sutisfy certain	       *
*  conditions								       *
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
#include <X11/Intrinsic.h>
#include <math.h>
#include <float.h>
#include "fltRange.h"
#define LIMIT (double) 1.0e-6
void setFRangeLimits(float *min, float *max)
{
/* we adjust only max to construct proper range */
    if ((double)*max < (double)*min + LIMIT) *max = *min + LIMIT;
    if ((double)*max < (double)*min * ((double) 1. + LIMIT)) 
        *max = (float)((double)*min * ((double) 1.+LIMIT));
    if ((double)*max < (double)*min * ((double) 1. - LIMIT))  
        *max = (float)((double) * min * ((double) 1. - LIMIT));
}


Boolean setFRangeLogScale (float *visibleStart , float *visibleEnd,
  float min, float max)
{
    float m = FLT_MIN;
    Boolean logScaling = TRUE;
/* don't set log scaling for the range with the "bad positive part" 	*/    
    if (min < m) 
    {
	if (max < m) logScaling = FALSE;
	if ((double)max <  (double)m * ((double) 1. + LIMIT))   
	    logScaling = FALSE;
	if ((double)max <  (double)m * ((double) 1. - LIMIT))   
	    logScaling = FALSE;
    }
    adjustFRange(visibleStart, visibleEnd, min, max, logScaling);
    return(logScaling);
}



void adjustFRange(float *start, float *end,float min, float max, 
    Boolean LogScale)
{
    double c = (double)*start + (double)*end;
    if ((double)*end < (double)*start + LIMIT)
    {
    	*start = (float) (((double)c -  LIMIT)/(double)2.);
    	*end = (float) (((double)c + LIMIT)/(double)2.);
    }
    if ((double)*end < (double)*start * ((double) 1. +(double) LIMIT))
    {
    	*start = (float) (c / ((double)2. +  LIMIT));
    	*end = (float) ((c * ((double) 1. + LIMIT) / ((double)2. + LIMIT)));
    }    	 
    if ((double)*end < (double)*start * ((double) 1. - (double)LIMIT))
    {
    	*start = (float) (c / ((double)2. - LIMIT));
    	*end = (float) ((c * ((double) 1. - LIMIT) / ((double)2. - LIMIT)));
    }
    if (LogScale) 
        if (min < FLT_MIN) min = FLT_MIN;
    if (*start < min)
    {
         *start = min;
         if ((double)*end < (double)*start * ((double) 1.- LIMIT)) 
             *end = (float) ((double)*start * ((double) 1.-LIMIT));
         if ((double)*end < (double)*start + LIMIT) 
             *end = (float)((double)*start + LIMIT);
         if ((double)*end < (double)*start * ((double) 1.+LIMIT))
              *end = *start *(float)((double) ((double) 1.+ LIMIT));
    }
    if (*end > max)
    {
         *end = max;
         if ((double)*end < (double)*start * ((double) 1.-LIMIT)) 
             *start = (float) ((double)*end / ((double) 1.-LIMIT));
         if ((double)*end < (double)*start + LIMIT) 
             *start = (float) ((double)*end - LIMIT);
         if ((double)*end < (double)*start * ((double) 1.+LIMIT)) 
             *start = (float) ((double)*end / ((double) 1.+ LIMIT));
    }
}


void pullFRangeStart(float *start, float *end, double d, float min, float max, 
    Boolean LogScale)
{
    float s = *start;
    if (LogScale)
    {
	d =  (log10((double)*end) - log10((double)*start)) * ( (double)1. - d);
	*start = (float) pow(10.,(log10((double)*start) + d));
	if (min < FLT_MIN) min = FLT_MIN;
    } 
    else
    {
	d =  ((double)*end - (double)*start) *((double) 1. - d);
	*start = (float)((double)*start + d);
    }
    if (d < 0)
    {
         if (*start < min) *start = min;
    }
    else
    {
        if ((double)*end < (double)*start + LIMIT) 
            *start = (float)((double)*end -  LIMIT);
        if ((double)*end < (double)*start * ((double) 1. + LIMIT)) 
            *start = (float)((double)*end / ((double) 1. + LIMIT));
        if ((double)*end < (double)*start * ((double) 1. - LIMIT)) 
            *start = (float)((double)*end / ((double) 1. - LIMIT));
    }
    if (LogScale)
        if(*start <= 0 ) 
            if (s > 0) *start = s;
            else *start = FLT_MIN;
}

void pullFRangeEnd(float *start, float *end, double d, float min, float max, 
    Boolean LogScale)
{
    if (LogScale)
    {
	d =  -(log10((double)*end) - log10((double)*start)) * ((double)1. - d);
	*end = (float)pow(10.,(log10((double)*end) + d));
	if (min < FLT_MIN) min = FLT_MIN;
    } 
    else
    {
	d =  -((double)*end - (double)*start) *((double)1. - d);
	*end = (float)((double)*end + d);
    }
    if (d > 0)
    {
         if (*end > max) *end = max;
    }
    else
    {
        if ((double)*end < (double)*start + LIMIT) 
            *end = (float)((double)*start + LIMIT);
        if ((double)*end < (double)*start * ((double) 1. + LIMIT)) 
            *end = (float)((double)*start * ((double) 1. + LIMIT));
        if ((double)*end < (double)*start * ((double) 1. - LIMIT)) 
            *end = (float)((double)*start * ((double) 1. - LIMIT));
    }
}

void moveFRange(float *start, float *end, double d, float min, float max, 
    Boolean LogScale)
{
    double s, e;
    if (LogScale)
    {
	d =  (log10((double)*end) - log10((double)*start)) *  d;
	d = pow(10., d);
	if (min < FLT_MIN) min = FLT_MIN;
	s = (double)*start * d;
	e = (double)*end * d;
	if (d > 1)
	{
            if (e > (double)max) 
            {
        	*start /= (float)((double)*start / 
        	  ((double)*end / (double)max) );
        	*end = max;
            }
            else
            {
                *start = (float)s;
                *end = (float)e;
            }
        }
	else
	{
            if (s < (double)min) 
                if ((double)min / (double)*start > d) 
                    d = (double)min / (double)*start;
            if (e < e + LIMIT)
                if (LIMIT /((double)*end - (double)*start) > d ) 
                    d = LIMIT /((double)*end - (double)*start);
            *start = (float)((double)*start * d);
            *end = (float)((double)*end * d);
        }
    } 
    else
    {
	d =  ((double)*end - (double)*start) * d;
	*start = (float)((double)*start + d);
	*end = (float)((double)*end + d);
	if (d > 0)
	{
            if (*end > max) 
            {
        	*start = (float)((double)*start - ((double)*end - (double)max));
        	*end = max;
            }
            else if ((double)*end < (double)*start * ((double) 1. + LIMIT))
            {
        	 *start = (float)(((double)*end - (double)*start) / LIMIT);
        	 *end = (float)((double)*start * ((double)1.+LIMIT));
            }
	}       
	else
	{
            if (*start < min) 
            {
        	*end = (float)((double)*end + ((double)min - (double)*start));
        	*start = min;
            }
            else if ((double)*end < (double)*start * ((double) 1. - LIMIT))
            {
        	*start = (float)(-((double)*end - (double)*start) / LIMIT);
        	*end =  (float)((double)*start * (1.-LIMIT));
            }
	}
    }
}

         

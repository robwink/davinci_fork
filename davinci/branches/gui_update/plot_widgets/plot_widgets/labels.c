/*******************************************************************************
*									       *
* labels.c -- 	axes labeling utilities to calculate labels and ticmarks       *
* 		placing (X & Y axes) for 2DHist widget			       *
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
#include <Xm/Xm.h>
#include <Xm/XmP.h>

#if XmVersion == 1002
#include <Xm/PrimitiveP.h>
#endif

#ifndef VMS
#include <string.h>
#endif /* VMS */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include "2DGeom.h"
#include "2DHistDefs.h"
#include "2DHist.h"
#include "uniLab.h" 
#include "labels.h"		
#include "2DHistP.h"
#include "imaggen.h"

typedef enum _ticsType{ IIIIIIIIII, iiiiiiiiii, IlIlIlIlIl, IllllIllll,
  Iiiiiliiii} ticsType;
static float nWeight(char *str, unsigned length, char format);
static int maxLabelXWidth(Hist2DWidget w);
static int maxLabelXHeight(Hist2DWidget w);
static labelTable *makeLableTable(double *value, unsigned n, lCS *labelCS);
static void calcXWidth (labelTable *lT, XFontStruct *fs);
static XSegment *makeTics(int *segNo, ticsType ttype, vector direction, 
  int pStart, int n, int step, vector *edge);
static ticStruct *makeTicStruct( ticsType ttype, vector bin, 
  int pStart, int n, int step, 
  range2d r2, discreteMap *dMap);
static labelsToDraw *makeXLabelsToDraw(ticLabel *curLabel, vector ticDirection,
  int pStart, int n, int step,  vector *edge, int XWidth, int ascent, 
  int charWidth, vector bin, vector limit);
static labelsToDraw *makeYLabelsToDraw(ticLabel *curLabel, vector ticDirection,
  int pStart, int n, int step,  vector *edge, int XWidth, int ascent, 
  int charWidth, vector bin, vector limit);
static void makeBaseLabels2 (XFontStruct *fs, discreteMap *dMap, 
  labelTable *lT, vector d,
  vector bin, range visible, range2d visible2d, vector limit, vector *edge,
  labelsToDraw * (*makeLabels) (ticLabel*, vector,
  int, int, int, vector*, int, int, int, vector, vector), 
  int *nTics, labelsToDraw ** labels, XSegment **tics, ticStruct **ts);
 

/* weigth function for evaluation of "quality" of label number */
static float nWeight(char *str, unsigned length, char format)
{
    float w;
    char *tmp;
    w=length;
    if (format=='e') 
    {
    	tmp=str+length-5;	/* last digit of the mantissa */
    	while (tmp>str && *tmp=='0')
    	{
    	    tmp--;
    	    w-=0.6;
    	}
    	if (tmp>str && *tmp=='.') tmp--;
    	if (*tmp=='0') w-=0.6;
    	else if (*tmp=='5') w-=.4;
    	else if (!(*tmp & 1)) w-=.15; /* even digit better then the odd one */
    	return (w*w);	        /* to calculate medium squars         */
    } else  if (format=='f') 			/* suppose format == 'f'	      */
    {
    	tmp=str+length-1;
    	while (tmp>str && *tmp=='0')
    	{
    	    tmp--;
    	    w-=0.6;
    	}
    	if (tmp>str && *tmp=='.') tmp--;
    	while (tmp > str && *tmp=='0')
    	{
    	    tmp--;
    	    w-=0.6;
    	}
	if (*tmp=='0') w-=.6;
        else if (*tmp=='5') w-=.4;
	else if (*tmp!=' ') if (!(*tmp & 1)) w-=.15;
	return (w*w);
    }
    else return (0);
}      	

/* "improved" rounding function for purpose of better represantation
** The matter of fact is that if we have a table of numbers as .45, .55, .65 
** .75, 1 etc., we could get next kind of results:
**      original table  (possible) float     rounded float	 what we'd like
**			 representation	   to one digit after    to have instead
**				           point
**      .45		   .4499..9	       	 .4		   .5
**      .55		   .5500..1	       	 .6		   .6
**      .65		   .6499..9		 .6		   .7
**      .75		   .7500..1		 .8		   .8
** this rounding function allow to have rather last colomn in such a case.
** Of cause it provides less rounding accuracy ( .502 * <value of last 
** significant digit instead of .5 for standard rounding). We consider this
** is not a point for presentation purposes. (And factor abouve could be
** ajusted)
*/
float smartRound(float a, int m)
{
#define MAGICFLOAT 4.98
    if (fmod((double) a, pow((double)10.,(double)m)) > 
     (double) MAGICFLOAT * pow((double)10.,(double)m) &&
     fmod((double) a, pow((double)10.,(double)m)) < 
     (double) 5 * pow((double)10.,(double)m))
       return (a+ .1*pow((double)10.,(double)m));
   else return(a);
}
static int maxLabelXWidth(Hist2DWidget w)
{
    XFontStruct *fs=w->hist2D.fs;
    char i='0';
    int j;
    int k=0;
    for (j=0;j<10;j++,i++) if (XTextWidth(fs,&i,1)>k) k=XTextWidth(fs,&i,1);
    k=k*(w->hist2D.baseLCS.defaultPrecision+3);
    if (XTextWidth(fs,"-",1)>XTextWidth(fs,"+",1)) k+=XTextWidth(fs,"-",1);
       else k+=XTextWidth(fs,"+",1);
    if (XTextWidth(fs,"-",1)>XTextWidth(fs," ",1)) k+=XTextWidth(fs,"-",1);
       else k+=XTextWidth(fs," ",1);
    return(k+XTextWidth(fs,".e",2));
}
static void sprintE(char* strE, int *lengthE, float value, 
  int mSDigit, int lSDigit, int maxLength, Boolean trailingZeros)		
{
    char *tmp;
    char str[80];
    if (mSDigit-lSDigit >= 0)
#ifdef PRINTF_ROUND		    
        sprintf(str, "%.*e",mSDigit-lSDigit, smartRound(value,lSDigit));
#else			  
        sprintf(str, "%.*e",mSDigit-lSDigit,smartRound(value,lSDigit)+
          pow((double)10.,(double)lSDigit)*0.5);
#endif			  
    else if (mSDigit-lSDigit == -1)
    {
	if (fabs(value)/ pow((double)10.,(double)lSDigit) > MAGICFLOAT)
	{
	    if (value<0)
	        sprintf(str, "%.*e",0,-pow((double)10.,(double)lSDigit)*1.);
	    else
	        sprintf(str, "%.*e",0,pow((double)10.,(double)lSDigit)*1.);
	}
	else
	    strcpy(str,"0e+00");
    }
    else
	strcpy(str,"0e+00");
    tmp=str+strspn(str," ");
    *lengthE=strlen(tmp);
    if (!trailingZeros)
    if (*lengthE>=6)
    {
	char *tmp1=tmp+*lengthE-5;
	char *tmp2=tmp1;
	while (tmp1>tmp && *tmp1=='0') 
	{   
	    tmp1--;
	    (*lengthE)--;
	}
/* Assume that sprintf produces at least one digit of mantissa in any case */			
	if (tmp1>tmp && *tmp1=='.')
	{   
	    tmp1--;
	    (*lengthE)--;
	}
	if (*tmp1=='+' || *tmp1=='-')
	{   
	    tmp1++;
	    (*lengthE)++;
	}
	*++tmp1 = *++tmp2;
	*++tmp1 = *++tmp2;
	*++tmp1 = *++tmp2;
	*++tmp1 = *++tmp2;
	*++tmp1 = *++tmp2;
    }
    if (*lengthE<maxLength)
	strcpy(strE,tmp);
    else
    { 
	*strE='\0';
	*lengthE=0;
    }
}

static void sprintF(char* strF, int *lengthF, float value, 
  int mSDigit, int lSDigit, int maxLength, Boolean trailingZeros)		
{
    char *tmp;
    char str[80];
    if (mSDigit-lSDigit>=0)
#ifdef PRINTF_ROUND		    
        sprintf(str, "%#.*f", lSDigit>=0 ? 0 : -lSDigit, 
          smartRound(value,lSDigit));
#else			  
        sprintf(str, "%#.*f", lSDigit>=0 ? 0 : (double)-lSDigit,
          smartRound(value,lSDigit)+pow((double)10.,(double)lSDigit)*0.5);
#endif			  
    else if (mSDigit-lSDigit == -1)
    {
	if (fabs(value)/ pow((double)10.,(double)lSDigit) > MAGICFLOAT)
	{
	    if (value<0)
	        sprintf(str, "%#.*f",lSDigit>=0 ? 0 : -lSDigit,
	          -pow((double)10.,(double)lSDigit)*1.);
	    else
	        sprintf(str, "%#.*f",lSDigit>=0 ? 0 : -lSDigit,
	          pow((double)10.,(double)lSDigit)*1.);
	}
	else
	    strcpy(str,"0");
    }
    else
	strcpy(str,"0");
    tmp=str+strspn(str," ");
    *lengthF=strlen(tmp);
    if (!trailingZeros && *lengthF>1)
    {
	char *tmp1=tmp+*lengthF-1;
	while (tmp1>tmp && *tmp1=='0') 
	{   
	    tmp1--;
	    (*lengthF)--;
	}
/* Assume that sprintf produces at least one digit of mantissa in any case */			
	if (tmp1>tmp && *tmp1=='.')
	{   
	    tmp1--;
            (*lengthF)--;
	}
	if (*tmp1=='+' || *tmp1=='-' || *tmp1=='.')
	{   
	    tmp1++;
	    (*lengthF)++;
	}
	*++tmp1 = '\0';
    }
    if (*lengthF<maxLength)
	strcpy(strF,tmp);
    else
    { 
	*strF='\0';
	*lengthF=0;
    }
}

static labelTable *makeLableTable(double *value, unsigned n, lCS *labelCS)
{
#define MAX_STR_LENGTH 40
    int oldStep,step;
    int oldStart;
    int stepFactor;
    int tenFlag;
    int i,j;
    labelTable *lT,**currentLT;
    ticLabel *currentLabel;
    char (*strE)[MAX_STR_LENGTH];
    char (*strF)[MAX_STR_LENGTH];
    char (*bestStr)[MAX_STR_LENGTH];
    int *mSDigit;
    int *lSDigitOld;
    int *lSDigitNew;
    int *lengthE;
    int *lengthF;
    int *bestLength;
    int maxMSDigit[5];  /* 5 is a max stepFactor */
    int minLSDigit[5];
    int maxLengthE[5];
    int maxLengthF[5];
    float weightE[5];
    float weightF[5];
    int minLength;
    int bestF;
    int bestE;
    if (n<=0) return (NULL);
    if (value==NULL) return(NULL);
    strE=(char(*)[MAX_STR_LENGTH]) XtMalloc(sizeof (char[MAX_STR_LENGTH])*n);
    strF=(char(*)[MAX_STR_LENGTH]) XtMalloc(sizeof (char[MAX_STR_LENGTH])*n);
    MYMALLOC(int,mSDigit,n);
    MYMALLOC(int,lSDigitOld,n);
    MYMALLOC(int,lSDigitNew,n);
    MYMALLOC(int,lengthF,n);
    MYMALLOC(int,lengthE,n);
/* assume that value[i] < value[j] if (i<j) */
/* The next code might not be portable because of floating point (exeptions)
** problems. Still it should work on most of the machines, because exeptions
** can apper if it get rather "odd" information as input 		 
*/
    for (i=0;i<n;i++)
    {
        if  (value[i]==0) mSDigit[i]=0;
        else mSDigit[i]=floor(log10(fabs(value[i])));
        if (labelCS->defaultPrecision > 0)
            lSDigitOld[i]=mSDigit[i]+ 1 - labelCS->defaultPrecision;
        else
            lSDigitOld[i]=mSDigit[i];
    }
    oldStart=0;
    oldStep=step=1;
    tenFlag=0;
    currentLT= &lT;
    stepFactor=1;
    while (n/step>=1)
    {
    	MYMALLOC(labelTable,*currentLT,1);
    	if ((labelCS->dynamicRounding) || (step==1))
    	{
	    for (i=oldStart;i<n-step;i+=oldStep)
	    {
        	if (value[i+step]-value[i]==0)
        	    lSDigitNew[i]=lSDigitOld[i+step] < lSDigitOld[i+step] ?
        	      lSDigitOld[i+step] : lSDigitOld[i+step];
		else 
        	    lSDigitNew[i]=floor(log10(fabs(value[i+step]-value[i])))+ 
        	      1 - labelCS->precision;
	    }
/*	assume <i> start value after previous loop.			*/
	    for (;i<step;i+=oldStep)
        	    lSDigitNew[i]=lSDigitOld[i];
	    for (;i<n;i+=oldStep)
        	lSDigitNew[i]=lSDigitNew[i-step];
	    for (i-=oldStep;i>=step;i-=oldStep)
        	if (lSDigitNew[i-step] < lSDigitNew[i])
        	    lSDigitNew[i]=lSDigitNew[i-step];
	    for (i=oldStart;i<n;i+=oldStep)
	    {
    		if (lSDigitNew[i]<lSDigitOld[i]) lSDigitNew[i]=lSDigitOld[i];
/* in fact we have only estimation for mSDigit and maxMSDigit, not an actual
** value we'll get during printing. This is because rounding errors and such
** cases as 9.999 which could be converted to 10.0 during rounding. 
** Attempt to improve this estimation.
** Next correction may happen at most once for each <i> during outmost cycling
** <while (n/step>1)..>.
** Because lSDigitNew[i] could only increase between that cycles the mSDigit
** value will be correct. 
*/
		if (mSDigit[i]>=lSDigitNew[i])
		    if (fabs(value[i])/pow((double)10.,(double)mSDigit[i])>9.)
	        	if (fabs(value[i])  >
	        	  pow((double)10.,(double)mSDigit[i]+1) - 
	        	  ((double)1.- MAGICFLOAT) * 
	        	  pow((double)10.,(double)lSDigitNew[i]))
	        	    mSDigit[i]++;
	    }
	}
	for (j=0;j<stepFactor;j++)
	{
	    if ((i=oldStart+j*oldStep)<n)
	    {
		maxMSDigit[j] = mSDigit[i];  
		minLSDigit[j] = lSDigitNew[i];  
	    }
	    for(i+=step;i<n;i+=step)
	    {
		if (maxMSDigit[j] < mSDigit[i]) 
		    maxMSDigit[j] = mSDigit[i];  
		if (minLSDigit[j] > lSDigitNew[i]) 
		    minLSDigit[j] = lSDigitNew[i];  
	    }
	}

	for (j=0;j<stepFactor;j++)
	{
	    int num=0;
	    weightE[j]=0;
	    weightF[j]=0;
	    maxLengthE[j]=0;
	    maxLengthF[j]=0;
	    for(i=oldStart+j*oldStep;i<n;i+=step)
	    {
		if (step==1||lSDigitNew[i]!=lSDigitOld[i])
		    sprintE(strE[i], lengthE+i, 
		      value[i] > (double)FLT_MAX ? FLT_MAX : 
		      (value[i] < -(double)FLT_MAX ? -FLT_MAX : (float)value[i]),  
		      mSDigit[i], 
		      lSDigitNew[i], MAX_STR_LENGTH,labelCS->trailingZeros);
		weightE[j]+=nWeight(strE[i],lengthE[i],'e');
                num++;
		if (maxLengthE[j] < lengthE[i])
		    maxLengthE[j] = lengthE[i];
	    }
	    if ( maxMSDigit[j] < labelCS->defaultPrecision+6 &&
		 minLSDigit[j] > -(labelCS->defaultPrecision+6))
		for(i=oldStart+j*oldStep;i<n;i+=step)
		{
		    sprintF(strF[i], lengthF+i, 
		      value[i] > (double)FLT_MAX ? FLT_MAX : 
		      (value[i] < -(double)FLT_MAX ? -FLT_MAX : (float)value[i]), 
		      mSDigit[i], lSDigitNew[i], MAX_STR_LENGTH, 
		      labelCS->trailingZeros);
		    weightF[j]+=nWeight(strF[i],lengthF[i],'f');
		    if (maxLengthF[j] < lengthF[i])
		        maxLengthF[j] = lengthF[i];
		}
            if (num!=0)
 	    {
                weightE[j]/= (double)num;
                weightF[j]/= (double)num;
            }
	}
/* printing and weigth/length calculations done, choose better subsecequnce
and format */
   	minLength=maxLengthE[0];
   	for (j=0;j<stepFactor;j++)
   	{
   	    if (maxLengthE[j] <= minLength)
   	        minLength = maxLengthE[j];
   	    if (maxLengthF[j]!=0 && maxLengthF[j] <= minLength)
   	        minLength= maxLengthF[j];
   	}
   	bestE= -1;
   	bestF= -1; /* marker */
   	for (j=0;j<stepFactor ;j++)
   	    if (minLength == maxLengthE[j])
   	    {
   	        bestE=j;
   	        break;
   	    }
   	for (j=0;j<stepFactor;j++)
   	    if (maxLengthF[j]!=0 /*  && minLength == maxLengthF[j] ttttt*/ ) 
   	    {
   	        bestF=j;
   	        break;
   	    }
        bestE=0;						/* tttt */
   	if (bestE!=-1)
   	    for (j=0;j<stepFactor;j++)
   /*tttt	        if (minLength == maxLengthE[j]) 		ttt*/
   	            if (weightE[bestE]>weightE[j])
   	                bestE=j;
   	if (bestF!=-1)
   	    for (j=0;j<stepFactor;j++)
   	        if (maxLengthF[j]!=0/* && minLength == maxLengthF[j] 	tttt*/ )
   	            if (weightF[bestF]>weightF[j])
   	                bestF=j;
	if (bestF!=-1) /*  prefer 'f' format if length is equal	*/
	{
            if (weightF[bestF]<weightE[bestE])
	    {
		j=bestF;
		bestLength=lengthF;
		bestStr=strF;
	    }
	    else
	    {
		j=bestE;
		bestLength=lengthE;
		bestStr=strE;
	    }
	}
	else
	{
	    j=bestE;
	    bestLength=lengthE;
	    bestStr=strE;
	}
/* shoice of subsequence is done,  make label table for this step	*/
	(*currentLT)->n=(n-1-(oldStart+j*oldStep))/step+1;
	(*currentLT)->start=oldStart+j*oldStep;
	(*currentLT)->step=step;
	MYMALLOC(ticLabel,(*currentLT)->labels,(*currentLT)->n);
	currentLabel=(*currentLT)->labels;
	for(i=oldStart+j*oldStep;i<n;i+=step)
	{

	    MYMALLOC(char,currentLabel->label,(bestLength[i]+1));
	    strcpy(currentLabel->label,bestStr[i]);
	    currentLabel->length=bestLength[i];
	    currentLabel++;  
	} 
/* correction for the first cycle				*/
	if (stepFactor==1) 
	{
	   stepFactor=2;
	   tenFlag=1;
	}
/* calculation of next subsequense step				*/
	if (stepFactor==2)
	{
	    if (tenFlag)
	    {
	    	tenFlag=0;
	    	oldStep=step;
	    	step=step*2;
	    	stepFactor=2;
	    	oldStart=(*currentLT)->start;
	    }
	    else
	    {
		stepFactor=5;
		step=step/2;
		step=step*5;
    /*	    oldStart & oldStep are the same	*/
	    }
	} else
	{
	    tenFlag=1;
	    oldStep=step;
	    step=step*2;
	    stepFactor=2;
	    oldStart=(*currentLT)->start;
	}
	{
	    int *tmp=lSDigitOld;
	    lSDigitOld=lSDigitNew;
	    lSDigitNew=tmp;
	}
	currentLT= &(*currentLT)->next;
    }
    *currentLT=NULL;
    MYFREE(mSDigit);
    MYFREE(lSDigitOld);
    MYFREE(lSDigitNew);
    MYFREE(lengthF);
    MYFREE(lengthE);
    MYFREE(strE);
    MYFREE(strF);
    return(lT);
}

void destroyLabelTable(labelTable *lT)
{
     int n;
     labelTable *tmp;
     ticLabel *ticTmp;
     tmp=lT;
     while (tmp!=NULL)
     {
     	ticTmp=tmp->labels;
     	n=tmp->n;
     	while (n>0)
     	{
     	    MYFREE(ticTmp->label);
     	    n--;
     	    ticTmp++;
     	}
     	MYFREE(tmp->labels);
     	lT=tmp;
     	tmp=tmp->next;
     	MYFREE(lT);
     }
}

static void calcXWidth (labelTable *lT, XFontStruct *fs)
{
    int maxXW=0;
    ticLabel *currentLabel;
    int i;
    while (lT!=NULL)
    {
	currentLabel=lT->labels;
        for(i=0;i<lT->n;i++,currentLabel++)
        {
            currentLabel->XShift=XTextWidth(fs,currentLabel->label,
              currentLabel->length);
            if (currentLabel->XShift > maxXW) 
            	maxXW=currentLabel->XShift;  
        }

	currentLabel=lT->labels;
	lT->XWidth=maxXW;            
        for(i=0;i<lT->n;i++,currentLabel++)
            currentLabel->XShift=maxXW-currentLabel->XShift;
        lT=lT->next;
    }
}

void makeXYLableTables(Hist2DWidget w)
{
    double *values;
    destroyLabelTable(w->hist2D.xLabels);
    destroyLabelTable(w->hist2D.yLabels);
    if (w->hist2D.sourceHistogram==NULL) return;
    MYMALLOC(double,values,(w->hist2D.sourceHistogram->nXBins+1));
    if (w->hist2D.sourceHistogram->xScaleType==H2D_LINEAR)
    {
     	double step=((double) w->hist2D.sourceHistogram->xMax - 
     	  (double)w->hist2D.sourceHistogram->xMin)/
     	  (double)w->hist2D.sourceHistogram->nXBins;
     	double *tmp=values;
     	double value=w->hist2D.sourceHistogram->xMin;
     	int n=w->hist2D.sourceHistogram->nXBins;
     	while (n>=0)
     	{
     	    *tmp++=value;
     	    value+=step;
     	    n--;
     	}
    }
    else if (w->hist2D.sourceHistogram->xScaleType==H2D_LOG)
    {
     	double b=w->hist2D.sourceHistogram->xScaleBase;
      	double m;
     	double c;
     	int n=w->hist2D.sourceHistogram->nXBins;
     	int i;
     	double *tmp=values;
      	if ((b <= 0)||(b==1))
     	{
     	    MYFREE(values);
     	    XtWarning("Bad Log base for X labeling");
     	} else
     	{
     	    m=(w->hist2D.sourceHistogram->xMax - 
     	      w->hist2D.sourceHistogram->xMin)/(pow(b,(double)n)-1);
     	    c=w->hist2D.sourceHistogram->xMin-m;
     	    for(i=0;i<=n;i++,tmp++)
     	        *tmp=m*pow(b,(double)i)+c;
     	}
    } else
    {
     	MYFREE(values);
     	XtWarning("Bad scale type for X labeling");
    }
    w->hist2D.xLabels=makeLableTable(values,
      w->hist2D.sourceHistogram->nXBins+1,
      &w->hist2D.baseLCS);
    calcXWidth (w->hist2D.xLabels, w->hist2D.fs);
    MYFREE(values);
    MYMALLOC(double,values,(w->hist2D.sourceHistogram->nYBins+1));
    if (w->hist2D.sourceHistogram->yScaleType==H2D_LINEAR)
    {
     	double step=((double) w->hist2D.sourceHistogram->yMax - 
     	  (double) w->hist2D.sourceHistogram->yMin)/
     	  (double) w->hist2D.sourceHistogram->nYBins;
     	double *tmp=values;
     	double value=w->hist2D.sourceHistogram->yMin;
     	int n=w->hist2D.sourceHistogram->nYBins;
     	while (n>=0)
     	{
     	    *tmp++=value;
     	    value+=step;
     	    n--;
     	}
    }
    else if (w->hist2D.sourceHistogram->yScaleType==H2D_LOG)
    {
     	double b=w->hist2D.sourceHistogram->yScaleBase;
      	double m;
     	double c;
     	int n=w->hist2D.sourceHistogram->nYBins;
     	int i;
     	double *tmp=values;
      	if ((b <= 0)||(b==1))
     	{
     	    MYFREE(values);
      	    XtWarning("Bad Log base for Y labeling");
     	} else
     	{
     	    m=(w->hist2D.sourceHistogram->yMax - 
     	      w->hist2D.sourceHistogram->yMin)/(pow(b,(double)n)-1);
     	    c=w->hist2D.sourceHistogram->yMin-m;
     	    for(i=0;i<=n;i++,tmp++)
     	        *tmp=m*pow(b,(double)i)+c;
     	}
    } else
    {
     	MYFREE(values);
     	XtWarning("Bad scale type for Y labeling");
    }
    w->hist2D.yLabels=makeLableTable(values,
      w->hist2D.sourceHistogram->nYBins+1,
      &w->hist2D.baseLCS);
    calcXWidth (w->hist2D.yLabels, w->hist2D.fs);
    MYFREE(values);
}

void initialazeBaseLCS(Hist2DWidget w)
{
    w->hist2D.baseLCS.defaultPrecision=DEFAULT_PRECISION;
    w->hist2D.baseLCS.precision=PRECISION;
    w->hist2D.baseLCS.dynamicRounding=TRUE;
    w->hist2D.baseLCS.trailingZeros=FALSE;
}


static int maxLabelXHeight(Hist2DWidget w)
{
    XFontStruct *fs=w->hist2D.fs;
    int ascent,descent;
    int dir;
    XCharStruct o;
    XTextExtents(fs,"0123456789.+-e",14,&dir,&ascent,&descent,&o);
    return (ascent+descent);
}

void makeMargins(Hist2DWidget w)
{
/*   top & bottom margins are used for axes labels (name), not for tic marks
**   labels only. So additional space for one string is needed and, probably
**   space between axes labels and tic marks labels in the bottom, according
**   MIN_H_LABEL_SPACING
*/ 
/*  add Motif Shadow size here				*/
    w->hist2D.topMargin=MARGIN;
    w->hist2D.bottomMargin=MARGIN;
    w->hist2D.leftMargin=MARGIN;
    w->hist2D.rightMargin=MARGIN;
    if (w->hist2D.sourceHistogram == NULL)
        return;
    if (w->hist2D.sourceHistogram->zLabel != NULL ) 
        w->hist2D.topMargin+=(w->hist2D.fs->ascent + 
          w->hist2D.fs->descent) * (1. + (double) MIN_V_LABEL_SPACING /4.);   
    w->hist2D.bottomMargin += maxLabelXHeight(w)+ LONG_TIC_LEN + 1;
    if (w->hist2D.sourceHistogram->xLabel != NULL || 
      w->hist2D.sourceHistogram->yLabel != NULL)   
	w->hist2D.bottomMargin += (w->hist2D.fs->ascent + 
          w->hist2D.fs->descent) * (1. + (double) MIN_V_LABEL_SPACING /4.);
}

static XSegment *makeTics(int *segNo, ticsType ttype, vector direction, 
  int pStart, int n, int step, vector *edge)
{
     XSegment *seg, *segTmp;
     int i,j;
     vector lTic;
     vector mTic;
     vector sTic;
     vector seq [10];
     lTic = setLength(direction,LONG_TIC_LEN); 
     mTic = setLength(direction,MED_TIC_LEN); 
     sTic = setLength(direction,SHORT_TIC_LEN); 
     switch (ttype) {
         case IIIIIIIIII:  
             for(i=0;i<10;i++) seq[i]=lTic;
             break;
         case iiiiiiiiii:
 	     for(i=0;i<10;i++) seq[i]=mTic;
             break; 
         case IlIlIlIlIl: 
             for(i=0;i<10;i+=2)
             {
              	 seq[i]=lTic;
              	 seq[i+1]=mTic;
             }
             break;
         case IllllIllll:
             for(i=0;i<10;i++) seq[i]=mTic;
             seq[0]=seq[5]=lTic;
             break;
         case Iiiiiliiii:        
             for(i=0;i<10;i++) seq[i]=sTic;
             seq[0]=lTic;
             seq[5]=mTic;
             break;
     }
     for (i=pStart,j=0;i>=0;i-=step,j=(j+9)%10);
     i += step;
     j = (j+1)%10;
     if (i>=n) return(NULL);
     *segNo=(n-i-1)/step + 1;
     MYMALLOC(XSegment,seg,(*segNo));
     segTmp=seg;
     for ( ;i<n;i+=step,j= (j+1)%10,segTmp++)
     {
          segTmp->x1=edge[i].x;
          segTmp->y1=edge[i].y;
          segTmp->x2=edge[i].x + seq[j].x;
          segTmp->y2=edge[i].y + seq[j].y;
     }
     return(seg);
}
/* parts from makeEdgePoints, file imaggen.c and makeTics, file labels.c */
static ticStruct *makeTicStruct( ticsType ttype, vector bin, 
  int pStart, int n, int step, 
  range2d r2, discreteMap *dMap)
    
{
    int v0;
    int nEdges;
    int Bar;
    int ticNo;
    ticStruct *ticTmp, *tics;
    range r;
    matrix2 map;
    int seq [10];
    int i,j;
    map = dMap->map;

/* culculate number of points */
    r2=transformRange2d(r2,dMap->rotMatr);
    /* actually bin.x allows to determine which axis is served "right" - virtual X if bin.x <=0
      or "left" - virtual Y if bin.x >0 */
    	if (bin.x> 0)
    	    r = yRange(r2);
    	else 
    	    r = xRange(r2);
    v0 = r.start;    	    
    if (v0 >= 0 )
	v0 /= MAXPIC;
    else 
    {
	v0 = -v0;
	v0 += MAXPIC - 1;
	v0 /= MAXPIC;
	v0 = - v0;
    }
    r = translateRange (r, v0 * -MAXPIC);
    nEdges =  (r.end ) / MAXPIC + 1;
    if (r.start != 0) nEdges --;
    if (r.end % MAXPIC !=0)
        Bar = 1;
    else
        Bar = 0;
    switch (ttype) {
         case IIIIIIIIII:  
             for(i=0;i<10;i++) seq[i]=LONG_TIC_LEN;
             break;
         case iiiiiiiiii:
 	     for(i=0;i<10;i++) seq[i]=MED_TIC_LEN;
             break; 
         case IlIlIlIlIl: 
             for(i=0;i<10;i+=2)
             {
              	 seq[i]=LONG_TIC_LEN;
              	 seq[i+1]=MED_TIC_LEN;
             }
             break;
         case IllllIllll:
             for(i=0;i<10;i++) seq[i]=MED_TIC_LEN;
             seq[0]=seq[5]=LONG_TIC_LEN;
             break;
         case Iiiiiliiii:        
             for(i=0;i<10;i++) seq[i]=SHORT_TIC_LEN;
             seq[0]=LONG_TIC_LEN;
             seq[5]=MED_TIC_LEN;
             break;
     }
     for (i=pStart,j=0;i>=0;i-=step,j=(j+9)%10);
     i += step;
     j = (j+1)%10;
     if (i>=n) return(NULL);
     ticNo=(n-i-1)/step + 1;
     MYMALLOC(ticStruct,tics,ticNo);
     ticTmp=tics;
     for ( ;i<n;i+=step,j= (j+1)%10,ticTmp++)
     {
          ticTmp->length = seq[j];
          if (v0 < 0 )
              ticTmp->bar = i + Bar;
          else
              ticTmp->bar = nEdges -1 - i + Bar;
     }
     return (tics);
}

static labelsToDraw *makeXLabelsToDraw(ticLabel *curLabel, vector ticDirection,
  int pStart, int n, int step,  vector *edge, int XWidth, int ascent, 
  int charWidth, vector bin, vector limit)
{
    vector lTic;
    int vShift;
    int hShift;
    int i;
    labelsToDraw *labelsStart, **lD;
    lTic = setLength(ticDirection,LONG_TIC_LEN); 
    lD= &(labelsStart);
    if (abs(bin.x) * ascent < bin.y * charWidth)
    /* side placing */
    {
	if (bin.y!=0)
            vShift= rint(((double)charWidth/2.+1.) * 
              (double)abs(bin.x) /(double)bin.y + (double)ascent/2.);
	else vShift=ascent/2;
	for(i=pStart;i< n;i+=step,curLabel++)
	{
	     {
		 MYMALLOC(labelsToDraw,*lD,1);
		 (**lD).x=edge[i].x + lTic.x + 1 + 1;
		 (**lD).y=edge[i].y + lTic.y + vShift;
/* make a private copy of ASCII label					*/
		 MYMALLOC(char,((**lD).label),(curLabel->length+1));
		 strcpy((**lD).label,curLabel->label);
		 (**lD).length=curLabel->length;
		 (**lD).sidePlacing = TRUE;
		 lD = &((**lD).next);
	     }
	}	 
    } else
/* down placing								*/    
    { 
	if(bin.y!=0)
            hShift= floor((double)(LONG_TIC_LEN>MED_TIC_LEN ?
              LONG_TIC_LEN - MED_TIC_LEN : 1)*
              (double)abs(bin.x)/(double)bin.y);
	else hShift= XWidth;
	for(i=pStart;i< n;i+=step,curLabel++)
	{
	    int center=(XWidth-curLabel->XShift)/2;
	    if (center > hShift) center=hShift;
	    if (edge[i].x + lTic.x - center > limit.x)
	    {
		MYMALLOC(labelsToDraw,*lD,1);
		(**lD).x=edge[i].x + lTic.x  -  center + 1;
		(**lD).y=edge[i].y + lTic.y + 1 + ascent;
/* make a private copy of ASCII label					*/
	 	MYMALLOC(char,((**lD).label),(curLabel->length+1));
		strcpy((**lD).label,curLabel->label);
		(**lD).length=curLabel->length;
		(**lD).sidePlacing = FALSE;
		lD = &((**lD).next);
	    }
	 }
    }
    *lD=NULL;
    return(labelsStart);
} 

static labelsToDraw *makeYLabelsToDraw(ticLabel *curLabel, vector ticDirection,
  int pStart, int n, int step,  vector *edge, int XWidth, int ascent, 
  int charWidth, vector bin, vector limit)
{
    vector lTic;
    int vShift;
    int hShift;
    int i;
    labelsToDraw *labelsStart, **lD;
    lTic = setLength(ticDirection,LONG_TIC_LEN); 
    lD= &(labelsStart);
    if (bin.x * ascent < bin.y * charWidth)
    /* side placing */
    {
	if (bin.y!=0)
            vShift= rint(((double)charWidth/2.+1.) * 
              (double) bin.x /(double)bin.y + (double)ascent/2.);
	else vShift=ascent/2;
	for(i=pStart;i< n;i+=step,curLabel++)
	{
	     if (edge[i].y-ascent+vShift > limit.y)
	     {
		 MYMALLOC(labelsToDraw,*lD,1);
		 (**lD).x=edge[i].x + lTic.x - 1 - XWidth + curLabel->XShift;
		 (**lD).y=edge[i].y + lTic.y + vShift;
/* make a private copy of ASCII label					*/
		 MYMALLOC(char,((**lD).label),(curLabel->length+1));
		 strcpy((**lD).label,curLabel->label);
		 (**lD).length=curLabel->length;
		 (**lD).sidePlacing = TRUE;
		 lD = &((**lD).next);
	     }
	}	 
    } else
/* down placing								*/    
    { 
	if(bin.y!=0)
            hShift=floor((double)(LONG_TIC_LEN>MED_TIC_LEN ?
              LONG_TIC_LEN - MED_TIC_LEN : 1)*
              (double)bin.x/(double)bin.y);
	else hShift=XWidth;
	for(i=pStart;i< n;i+=step,curLabel++)
	{
	    int center=(XWidth-curLabel->XShift)/2;
	    if (center > hShift) center=hShift;
	    if (edge[i].x + lTic.x + center < limit.x)
	    {
		MYMALLOC(labelsToDraw,*lD,1);
		(**lD).x=edge[i].x + lTic.x - XWidth + curLabel->XShift + 
		  center + 1;
		(**lD).y=edge[i].y + lTic.y + 1 + ascent;
/* make a private copy of ASCII label					*/
		MYMALLOC(char,((**lD).label),(curLabel->length+1));
		strcpy((**lD).label,curLabel->label);
		(**lD).length=curLabel->length;
		(**lD).sidePlacing = FALSE;
		lD = &((**lD).next);
	    }
	 }
    }
    *lD=NULL;
    return(labelsStart);
} 



/* Should be applyed to newly created oD only. Does not check
** if oD already has labels or not, and destory nothing in any case
*/
void makeBaseLabels (XFontStruct *fs, discreteMap *dMap, 
    labelTable *xlT, labelTable *ylT, range2d visible, objectsToDraw *oD)
{
    vector d;
    vector limit;
    range vRange;
    labelTable *lT;
    matrix2 matr;
    vector bin;
    vector l;
    vector *xEdges, *yEdges;
    oD->xTics=NULL; 
    oD->nXTics=0;
    oD->xLabels=NULL;
    oD->yTics=NULL; 
    oD->nYTics=0;
    oD->yLabels=NULL;
    if (fs==NULL) return;
    if (dMap==NULL) return; 
/*    matr=matrixMultMatrix(dMap->map,dMap->factor); */
    matr=dMap->map;
    
    limit=makeVector(dMap->vertex.x, subVectors(dMap->vertex,yColomn(dMap->map)).y);
    makeEdgePoints( visible, dMap, &l, &xEdges, &yEdges);
    d=matrixMultVector(transposeMatrix(dMap->rotMatr),makeVector(1,0));
    if (d.x==0)
    {
        lT=ylT;
        vRange=yRange(visible);
    }
    else
    {
        lT=xlT;
        vRange=xRange(visible);
    }
    vRange.start = (vRange.start + MAXPIC - 1) >> LOWBITS;
    vRange.end =  vRange.end  >> LOWBITS;
    bin = xColomn(matr);
    bin.x = -bin.x;
    bin.x <<= (2 * LOWBITS);
    bin.y <<= (2 * LOWBITS);
    bin.x /= l.x;
    bin.y /= l.x;
    if (bin.x % l.x !=0) bin.x ++;
    bin.x = -bin.x;
     
    makeBaseLabels2 (fs, dMap, lT, d, bin, vRange, visible,
    limit, xEdges, makeXLabelsToDraw, 
    &(oD->nXTics), &( oD->xLabels), &(oD->xTics), &(oD->xTicStruct) );

    d=matrixMultVector(transposeMatrix(dMap->rotMatr),makeVector(0,1));
    if (d.x==0)
    {
        lT=ylT;
        vRange=yRange(visible);
    }
    else
    {
        lT=xlT;
        vRange=xRange(visible);
    }
    vRange.start = (vRange.start + MAXPIC - 1) >> LOWBITS;
    vRange.end =  vRange.end  >> LOWBITS;
    bin = yColomn(matr);
    bin.x <<= (2 * LOWBITS);
    bin.y <<= (2 * LOWBITS);
    bin.x /= l.y;
    bin.y /= l.y;
    makeBaseLabels2 (fs, dMap, lT, d, bin, vRange, visible,
    limit, yEdges, makeYLabelsToDraw, 
    &(oD->nYTics), &( oD->yLabels), &(oD->yTics), &(oD->yTicStruct) );
    MYFREE(xEdges);
    MYFREE(yEdges);
}    
    
static void makeBaseLabels2 (XFontStruct *fs, discreteMap *dMap, 
  labelTable *lT, vector d,
  vector bin, range visible, range2d visible2d, vector limit, vector *edge,
  labelsToDraw * (*makeLabels) (ticLabel*, vector,
  int, int, int, vector*, int, int, int, vector, vector), 
  int *nTics, labelsToDraw ** labels, XSegment **tics, ticStruct **ts )
{    
    int ascent,descent;
    int dir;
    XCharStruct o;
    unsigned charWidth;
    int vStep;
    int ticStep;
    int ticFactor;
    vector ticDirection;
    int vSpacing;
    int i;
    int visibleLength;
    *tics=NULL; 
    *nTics=0;
    *labels=NULL;
    if (fs==NULL) return;
    if (dMap==NULL) return; 
    if (lT==NULL) return;
    XTextExtents(fs,"0123456789.+-e",14,&dir,&ascent,&descent,&o);

/* what charWidt to use better - "0" or maximum to make decision where
** place labels (side or buttom) and to calculate horizontal spacing?? 
** Use "0" now								
*/
    charWidth=XTextWidth(fs,"0",1);
    vSpacing=(ascent + descent) * (1 + MIN_V_LABEL_SPACING);
    if (bin.y !=0)
        vStep=((vSpacing << LOWBITS)+bin.y-1)/bin.y;
    else vStep=lT->n+1;
    if (vectorLength(bin) >= 1./(double)MAXPIC)
        ticStep=ceil((double)MIN_TIC_SPACING *(double)MAXPIC /
          vectorLength(bin));
    else ticStep=lT->n+1;

    while ((lT!=NULL)&&
      (abs(lT->step * bin.x) >> LOWBITS < lT->XWidth + charWidth * MIN_H_LABEL_SPACING)&&
      (lT->step < vStep))
    	lT=lT->next;
    {
    	ticsType ttype;
/* choose tics tipe			   				
**          	IIIIIIIIII
**              IlIlIlIlIl
**              IllllIllll
**          	Iiiiiliiii
**    I - long tic mark; l - medium tic mark; i -short tic mark
*/
    /* actually bin.x allows to determine which axis is served "right" - virtual X if bin.x <=0
      or "left" - virtual Y if bin.x >0 */
    	if (bin.x<=0)
    	    ticDirection=ortogonal(bin);
    	else 
    	    ticDirection=mulVector(ortogonal(bin),-1);
    	visibleLength=visible.end - visible.start;
	if (lT!=NULL)
	{
    /* overwise no labeling is possible at all */
    	    ticFactor=  (lT->step / ticStep);
    	    if (ticFactor >= 10)
    		ticFactor = 10;
    	    else if (ticFactor >= 5)
    		ticFactor = 5;
    	    else if (ticFactor >= 2)
    		ticFactor = 2;
    	    else
    		ticFactor = 1;
            if (lT->step < 10)
        	ticFactor = lT->step <= ticFactor ? lT->step : 1;
            switch (ticFactor)
            {
        	case 10: ttype=Iiiiiliiii; break;
        	case  5: ttype=IllllIllll; break;
        	case  2: ttype=IlIlIlIlIl; break;
        	case  1: ttype=IIIIIIIIII; break;
            }
	    {
    		int visibleLabelsStart;
		ticLabel *curLabel;
    		visibleLabelsStart=(lT->step + lT->start - 
    		  visible.start % lT->step) % lT->step;
		*tics=makeTics(nTics, ttype, ticDirection, 
		  visibleLabelsStart, visibleLength +1, lT->step / ticFactor,edge);
		*ts = makeTicStruct( ttype, bin, visibleLabelsStart, visibleLength +1, 
		  lT->step / ticFactor,  visible2d, dMap);  
                curLabel=lT->labels+(visible.start + visibleLabelsStart -
                  lT->start)/lT->step;
		*labels = makeLabels(curLabel, ticDirection,
  	          visibleLabelsStart, visibleLength + 1, lT->step, edge, 
  	          lT->XWidth, ascent, charWidth, bin, limit);
  	    }
	}
	else 
	{
            ttype=iiiiiiiiii;
	    *tics=makeTics(nTics, ttype, ticDirection, 
	      0, visibleLength+1, ticStep,edge);
	    *ts = makeTicStruct( ttype, bin, 0, visibleLength +1, 
		ticStep,  visible2d, dMap);
	}
    }
}






 

labelsToDraw *makeNameLabels (char *name, XFontStruct *fs)
{
    char *tmp;
    char *term="...";
    labelsToDraw *chain, *current;
    if (name==NULL) return (NULL);
    if (strlen(name)==0) return (NULL); 
    if (fs==NULL) return (NULL);
    chain=NULL;
    tmp=name;
    MYMALLOC(labelsToDraw, current,1);
    current->next=chain;
    chain=current;
    current->length=3;
    MYMALLOC(char, current->label ,current->length+1);
    strcpy (current->label,term);
    current->XWidth=XTextWidth(fs,current->label,current->length);
    while (tmp!=NULL)
    {
        MYMALLOC(labelsToDraw, current,1);
	current->next=chain;
	chain=current;
        tmp=strchr(tmp,' ');
        if (tmp!=NULL)
        {
            current->length=tmp-name+1+3;
            MYMALLOC(char, current->label ,current->length+1);
            strncpy (current->label,name,tmp-name+1);
            strcpy (current->label+(tmp-name)+1,term);
        }
        else
        {
            current->length=strlen(name);
            MYMALLOC(char, current->label ,current->length+1);
            strcpy (current->label,name);
        }
        current->XWidth=XTextWidth(fs,current->label,current->length);
        if (tmp!=NULL) tmp++;
    }
    return(chain);
}

void destroyLabelsToDraw (labelsToDraw *lTD) /* full destruction */
{
    labelsToDraw *tmp;
    while (lTD!=NULL)	
    {
        MYFREE(lTD->label);
        tmp=lTD;
        lTD=lTD->next;
        MYFREE(tmp);
    }
}
void makeMessageToDraw (Hist2DWidget w, objectsToDraw *oD)
{
    labelsToDraw *tmp;
    char *msg1 = "Press CTRL for\0";
    char *msg2 = "Continious Display\0";
    int l1,l2;
    int w1,w2;
    int wid;
    int h;
    int ascent,descent;
    int dir;
    XCharStruct o;
    XFontStruct *fs;
    oD->message = NULL;
    if (w->hist2D.discrMap==NULL) return;
    if (w->hist2D.fs==NULL) return;
    if (!w->hist2D.templateDragging) return;
    fs = w->hist2D.fs;
    l1 = strlen(msg1);
    l2 = strlen(msg2);
    w1 = XTextWidth(fs,msg1,l1);
    w2 = XTextWidth(fs,msg1,l2);
    wid = (w1 > w2) ? w1 : w2;
    XTextExtents(fs,msg1,l1,&dir,&ascent,&descent,&o);
    h = ascent + descent;
    XTextExtents(fs,msg2,l2,&dir,&ascent,&descent,&o);
    h += ascent + descent;
    h += (fs->ascent + fs->descent) * (MIN_V_LABEL_SPACING/4);
    if (wid > w->hist2D.discrMap->cubeSize) return;
    if (h > (double)w->hist2D.discrMap->cubeSize * .71) return;
    MYMALLOC(labelsToDraw,oD->message,1);
    tmp = oD->message;
    MYMALLOC(char,tmp->label,l1+1);
    strcpy(tmp->label,msg1);
    tmp->length = l1;
    tmp->x = w->hist2D.discrMap->vertex.x - 
      (w->hist2D.discrMap->map.x.y + w->hist2D.discrMap->map.x.x )/2
      - w1/2;
    XTextExtents(fs,msg1,l1,&dir,&ascent,&descent,&o);
    h = descent;
    XTextExtents(fs,msg2,l2,&dir,&ascent,&descent,&o);
    h += ascent + descent;
    h += (fs->ascent + fs->descent) * (MIN_V_LABEL_SPACING/4);
    tmp->y = (w->core.height - w->hist2D.bottomMargin - w->hist2D.topMargin)/2 
      + w->hist2D.topMargin - h;
    MYMALLOC(labelsToDraw,tmp->next,1);
    tmp=tmp->next;
    MYMALLOC(char,tmp->label,l2+1);
    strcpy(tmp->label,msg2);
    tmp->length = l2;
    tmp->x = w->hist2D.discrMap->vertex.x - 
      (w->hist2D.discrMap->map.x.y + w->hist2D.discrMap->map.x.x )/2
       - w2/2;
    XTextExtents(fs,msg2,l2,&dir,&ascent,&descent,&o);
    h =  descent;
    tmp->y = (w->core.height - w->hist2D.bottomMargin - w->hist2D.topMargin)/2 
      + w->hist2D.topMargin - h;
    tmp->next=NULL;
}

    
    


void makeNameLabelsToDraw (Hist2DWidget w, objectsToDraw *oD)
{
    vector v;
    vector u;
    int x;
    labelsToDraw *tmp;
    oD->xName=NULL;
    oD->yName=NULL;
    oD->zName=NULL;
    if (w->hist2D.discrMap==NULL) return;
    if (w->hist2D.fs==NULL) return;
    
    tmp=w->hist2D.zNames;
    while (tmp!=NULL)
    {
        if (tmp->XWidth < w->core.width - 2 * MARGIN)
            break;
    	tmp=tmp->next;
    }            
    if (tmp!=NULL)
    {
        v=w->hist2D.discrMap->vertex;
        v=subVectors(v,yColomn(w->hist2D.discrMap->map));
        if (tmp->XWidth /2 + MARGIN < v.x)
            tmp->x= v.x - tmp->XWidth /2;
        else 
            tmp->x= MARGIN;
        u=subVectors(v,xColomn(w->hist2D.discrMap->map));
        tmp->y = u.y + w->hist2D.discrMap->zFactor  - 
            w->hist2D.fs->descent - (w->hist2D.fs->ascent + 
          w->hist2D.fs->descent) * ((double) MIN_V_LABEL_SPACING /4.);
        MYMALLOC(labelsToDraw,oD->zName,1);
        *(oD->zName)= *tmp;
    }
    if (w->hist2D.discrMap->rotMatr.x.x!=0)
        tmp=w->hist2D.yNames;
    else 
        tmp=w->hist2D.xNames;
    while (tmp!=NULL)
    {
        if (tmp->XWidth < w->hist2D.discrMap->vertex.x - 2 * MARGIN)
            break;
    	tmp=tmp->next;
    }            
    if (tmp!=NULL)
    {
        if (tmp->XWidth + 2 * MARGIN <w->hist2D.discrMap->map.x.y)
            tmp->x= w->hist2D.discrMap->vertex.x - 
              w->hist2D.discrMap->map.x.y /2
              - tmp->XWidth /2;
        else 
            tmp->x = w->hist2D.discrMap->vertex.x - MARGIN - tmp->XWidth;
        tmp->y = w->hist2D.discrMap->vertex.y + w->hist2D.bottomMargin
         - w->hist2D.fs->descent;
        MYMALLOC(labelsToDraw,oD->yName,1);
        *(oD->yName)= *tmp;
    }
    if (w->hist2D.discrMap->rotMatr.x.x!=0)
        tmp=w->hist2D.xNames;
    else 
        tmp=w->hist2D.yNames;
    while (tmp!=NULL)
    {
        if (tmp->XWidth < w->core.width - w->hist2D.discrMap->vertex.x - 
          2 * MARGIN)
            break;
    	tmp=tmp->next;
    }    
    if (tmp!=NULL)
    {
        v=w->hist2D.discrMap->vertex;
        v=subVectors(v,yColomn(w->hist2D.discrMap->map));
        
        if (tmp->XWidth + 2 * MARGIN < -  w->hist2D.discrMap->map.x.x)
            tmp->x= w->hist2D.discrMap->vertex.x - 
              w->hist2D.discrMap->map.x.x /2
              - tmp->XWidth /2;
        else 
            tmp->x = w->hist2D.discrMap->vertex.x + MARGIN ;
        tmp->y = w->hist2D.discrMap->vertex.y + w->hist2D.bottomMargin
         - w->hist2D.fs->descent;
        MYMALLOC(labelsToDraw,oD->xName,1);
        *(oD->xName)= *tmp;
    }
}            

void makeArrows (Hist2DWidget w, objectsToDraw *oD)
{
    vector v;
    vector u;
    XSegment *seg;
    discreteMap *dMap;
    int x;
    labelsToDraw *tmp;
    matrix2 ind;
    range2d rv, rh;
    oD->nArrows=0;
    oD->arrows=NULL;
    if (w->hist2D.discrMap==NULL) return;
    if (w->hist2D.sourceHistogram == NULL) return;
    MYMALLOC (XSegment,oD->arrows,12);
    seg = oD->arrows;
    dMap= w->hist2D.discrMap;
    if ((w->hist2D.bins != NULL || w->hist2D.aHist != NULL) 
      && dMap->zFactor < -12)
    {
    	vector a1,a2;
    	vector p1,p2;
    	vector vtmp;
    	vtmp = makeVector(0,200);
    	a1.x = (int) ( cos(ARROW_ANGLE) * vtmp.x + sin(ARROW_ANGLE) * vtmp.y);
    	a1.y = (int) ( -sin(ARROW_ANGLE) * vtmp.x + cos(ARROW_ANGLE) * vtmp.y);
    	a2.x = (int) ( cos(ARROW_ANGLE) * vtmp.x  - sin(ARROW_ANGLE) * vtmp.y);
    	a2.y = (int) ( sin(ARROW_ANGLE) * vtmp.x + cos(ARROW_ANGLE) * vtmp.y);
    	v = subVectors(dMap->vertex,yColomn(dMap->map));
    	a1 = setLength(a1,ARROW_LENGTH);
    	a2 = setLength(a2,ARROW_LENGTH);
    	p1 = subVectors(v,a1);
    	p2 = subVectors(v,a2);
        if (w->hist2D.zMin < w->hist2D.zVisibleStart)
        {
            seg->x1=v.x; seg->x2=p1.x; seg->y1=v.y; seg->y2=p1.y; seg++; oD->nArrows++;
            seg->x1=v.x; seg->x2=p2.x; seg->y1=v.y; seg->y2=p2.y; seg++; oD->nArrows++;
        }
        v = addVectors(v, makeVector(0,dMap->zFactor));
	p1 = addVectors(v,a1);
    	p2 = addVectors(v,a2);
        if (w->hist2D.zMax > w->hist2D.zVisibleEnd)
        {
            seg->x1=v.x; seg->x2=p1.x; seg->y1=v.y; seg->y2=p1.y; seg++; oD->nArrows++;
            seg->x1=v.x; seg->x2=p2.x; seg->y1=v.y; seg->y2=p2.y; seg++; oD->nArrows++;
        }
    }
    rv = transformRange2d (w->hist2D.xyVisibleRange, dMap->rotMatr);
    rh = transformRange2d (w->hist2D.xyHistoRange, dMap->rotMatr);
    if (vectorLength(xColomn(dMap->map)) > 12)
    {
    	vector a1,a2;
    	vector p1,p2;
    	vector vtmp;
    	vtmp = xColomn(dMap->map);
    	a1.x = (int) ( cos(ARROW_ANGLE) * vtmp.x + sin(ARROW_ANGLE) * vtmp.y);
    	a1.y = (int) ( -sin(ARROW_ANGLE) * vtmp.x + cos(ARROW_ANGLE) * vtmp.y);
    	a2.x = (int) ( cos(ARROW_ANGLE) * vtmp.x  - sin(ARROW_ANGLE) * vtmp.y);
    	a2.y = (int) ( sin(ARROW_ANGLE) * vtmp.x + cos(ARROW_ANGLE) * vtmp.y);
    	v = dMap->vertex;
    	a1 = setLength(a1,ARROW_LENGTH);
    	a2 = setLength(a2,ARROW_LENGTH);
    	p1 = subVectors(v,a1);
    	p2 = subVectors(v,a2);
        if (rv.end.x != rh.end.x)
        {
            seg->x1=v.x; seg->x2=p1.x; seg->y1=v.y; seg->y2=p1.y; seg++; oD->nArrows++;
            seg->x1=v.x; seg->x2=p2.x; seg->y1=v.y; seg->y2=p2.y; seg++; oD->nArrows++;
        }
	v = subVectors(v,xColomn(dMap->map));
	p1 = addVectors(v,a1);
    	p2 = addVectors(v,a2);
    	if (rv.start.x!=rh.start.x)
        {
            seg->x1=v.x; seg->x2=p1.x; seg->y1=v.y; seg->y2=p1.y; seg++; oD->nArrows++;
            seg->x1=v.x; seg->x2=p2.x; seg->y1=v.y; seg->y2=p2.y; seg++; oD->nArrows++;
        }
    }
    if (vectorLength(yColomn(dMap->map)) > 12)
    {
    	vector a1,a2;
    	vector p1,p2;
    	vector vtmp;
    	vtmp = yColomn(dMap->map);
    	a1.x = (int) ( cos(ARROW_ANGLE) * vtmp.x + sin(ARROW_ANGLE) * vtmp.y);
    	a1.y = (int) ( -sin(ARROW_ANGLE) * vtmp.x + cos(ARROW_ANGLE) * vtmp.y);
    	a2.x = (int) ( cos(ARROW_ANGLE) * vtmp.x  - sin(ARROW_ANGLE) * vtmp.y);
    	a2.y = (int) ( sin(ARROW_ANGLE) * vtmp.x + cos(ARROW_ANGLE) * vtmp.y);
    	v = dMap->vertex;
    	a1 = setLength(a1,ARROW_LENGTH);
    	a2 = setLength(a2,ARROW_LENGTH);
    	p1 = subVectors(v,a1);
    	p2 = subVectors(v,a2);
        if (rv.end.y!=rh.end.y)
        {
            seg->x1=v.x; seg->x2=p1.x; seg->y1=v.y; seg->y2=p1.y; seg++; oD->nArrows++;
            seg->x1=v.x; seg->x2=p2.x; seg->y1=v.y; seg->y2=p2.y; seg++; oD->nArrows++;
        }
	v = subVectors(v,yColomn(dMap->map));
	p1 = addVectors(v,a1);
    	p2 = addVectors(v,a2);
    	if (rv.start.y!=rh.start.y)
        {
            seg->x1=v.x; seg->x2=p1.x; seg->y1=v.y; seg->y2=p1.y; seg++; oD->nArrows++;
            seg->x1=v.x; seg->x2=p2.x; seg->y1=v.y; seg->y2=p2.y; seg++; oD->nArrows++;
        }
    }
    if (oD->nArrows == 0)
    {
        MYFREE(oD->arrows);
        oD->arrows = NULL;
    }
}

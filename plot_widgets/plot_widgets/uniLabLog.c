#include <X11/Intrinsic.h> 
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
/* for interface function only	in test version				*/
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include "2DGeom.h"
/* below is that is really needed					*/
#include "uniLab.h"
#include "uniLabLog.h"

static double firstLogIntLabel( double baseLValue, double step,
    double minValue, double maxValue);
static void mkLogTicsGroup (double minValue, double maxValue, int ticsInGroup, 
    Boolean denseLables, Boolean labeledGroup, ticStruct *ticPrms,  
    double value, int *ticNo, labelsToDraw2 **lTmp);
static double minLogLabelsStep(XFontStruct *fs, double minLValue, double maxLValue,  
   double minLBase, double maxLBase, XSegment *axisImage, char **format);   
static int maxDigXWidth(XFontStruct *fs);
static int maxLabelXHeight(XFontStruct *fs);
static int maxDigXHeight(XFontStruct *fs);
static Boolean aboveMin(double stepValue, int subStepNo, int stepsInGroup,
    double minValue);



/* set values and print all labels					*/
/*
**  void mkLogLabelsValues ( double baseValue, double step, int stepFactor,
**  char *format,  labeledSegment *lS)
**
**  double baseValue 	the value chosen as a primary value to be labeled
**  double step		labels step in object coordinates
**  char *format	label format for <sprintf> fuction. Should be
**			a kind of "%.*e" to print one double value
**  labeledSegment *lS  this structure is used to
**			- get <lS->v0> and <lS->v1> values which should
**			be setted up
**			- return a pointer to a linked list of labels
**			in <lS->labels>
**
**  Calculates all labels values, their ASCII representation and their
**  lengths and return a pointer to labels list in <lS->labels>.
**  Don't check <lS->labels> on the possible presence of the list
**  upon function entry (and, hence, don't destroy it), just assume
**  that it does not exist.
*/	

void mkLogLabelsValues ( double baseValue, double step, char *format,
  labeledSegment *lS)
{
    labelsToDraw2 **curLab;
    char buf[80];
/* assume lS->labels is not allocated					*/
    double v;
    double minValue = (lS->v0 < lS->v1) ? lS->v0 : lS->v1;
    double maxValue = (lS->v0 < lS->v1) ? lS->v1 : lS->v0;
    int subStepNo;
/*  generate labels values						*/
    curLab = &(lS->labels); 
    
    v = firstLogIntLabel(baseValue, step, minValue, maxValue);
    if (v > minValue)
    	v -= step < 1. ?  1. : step;
    subStepNo = 0;
        	
    while (v + log10((double)(subStepNo * subStepNo) + 1.) <= maxValue)
/* (subStepNo * subStepNo) above is used to provide simple transfer
  0 -> 0, 1->1, 2->4  							*/          
    {
	if (aboveMin(v, subStepNo, 3, minValue))
	{
            *curLab = (labelsToDraw2*)XtMalloc(sizeof(labelsToDraw2));
            (*curLab)->lValue = v + log10((double)(subStepNo * subStepNo) + 1.);
            if (strchr (format, 'f') != NULL)
        	sprintf (buf,format, v <0 ? (int)(-v) : 0, pow(10., v + 
                    log10((double)(subStepNo * subStepNo) + 1.)));
            else
        	sprintf (buf,format, pow(10.,v + 
        	  log10((double)(subStepNo * subStepNo) + 1.)));    
    /* length of the label in chars						*/        
            (*curLab)->length = strlen(buf);
            (*curLab)->label = (char*)XtMalloc(sizeof(char)*((*curLab)->length+1));
	    strcpy((*curLab)->label,buf);
	    curLab = &((*curLab)->next);
        }	    
	if (step < 1.)
	{
	    subStepNo++;
	    if (subStepNo == 3)
	    {
	        subStepNo = 0;
	        v += 1.;
	    }
	}
	else	        
            v += step;
    }                
    *curLab = NULL;
}         

/* 
**
** returned firstLogIntLabel might be out of <minValue,   maxValue>
** range if (log) step < 1. In this case no exact power of ten
** (with integer log) might appear in the range, and returned value
** will be the closest one, smaller then minValue
*/
static double firstLogIntLabel( double baseLValue, double step,
    double minValue, double maxValue)
{
    int intBase;
    int intStep;
    
    if (step < 1.)
        intStep = 1;
    else
        intStep = (int) step; 
    intBase = (int) baseLValue;      
    if (intBase %  intStep != 0)
    {
        int rem = intBase %  intStep;
        if (rem < 0)
            rem = intStep + rem;
        if (intBase -  rem  >= minValue)
            intBase -= rem;
    }
    while (intBase - intStep   >= minValue)
        intBase -= intStep ;
    return((double)intBase);
}            	        	                        

/* calculate   tic values + make label-tic links		*/

void mkLogTicsValues (double baseValue, double step,  labeledSegment *lS)
{
    labelsToDraw2 *lTmp = lS->labels;
    double minTicStep;
    int ticsInGroup;
    int ticGroupsBetweenLabels;
    double minValue = (lS->v0 < lS->v1) ? lS->v0 : lS->v1;
    double maxValue = (lS->v0 < lS->v1) ? lS->v1 : lS->v0;
    double v;
    int i,j;

/* assume labels is setted up already					*/
    if ((lS->axisImage.x2 == lS->axisImage.x1) &&
      (lS->axisImage.y2 == lS->axisImage.y1))
        minTicStep = fabs(lS->v1 - lS->v0) * 2.;
    else
        minTicStep = (double)(MIN_TIC_SPACING) /
          (double)vectorLength(makeVector(lS->axisImage.x2 - lS->axisImage.x1,
          lS->axisImage.y2 - lS->axisImage.y1)) * fabs(lS->v1 - lS->v0);
    
    if ( minTicStep > 1.)
        ticGroupsBetweenLabels = 1;
    else if (step >= 1.)
        ticGroupsBetweenLabels = (int)step;
    else 
	ticGroupsBetweenLabels = 1;
	
/* recalculate minTicStep inside the group for "nonlinear" tic 
   placement, using norrower MIN_TIC_SPACING, so average tic density
   would be the same as in linear placement				*/
/* for  tics  placing with nonliner step in picture space use average 
   estimation instead of min. So, to place 9 tics in gorup 
   (the group length in <Log10 object space is 1>) next condition
   is used:								*/

    if ( minTicStep * 9. < 1.)
        ticsInGroup = 9;
    else if (minTicStep < log10(2.))
/*    
**	ticsInGroup = 3;
**	(like tick in values 1,2,5,10,20,50,100 with labels in 1 and 100)
**	This does not look very good (sometimes a bit confusion), so
**	the next strategy:
*/
	ticsInGroup = 1;
    else
	ticsInGroup = 1;
	 
/* consider labeling with log10(2) - log10(5) step as a labeling with step == 1 */	
    if (step < 1.)
        if(ticsInGroup < 3)
            ticsInGroup = 3;	                    
    
/* determine tics number (not very accurate, just estimation)		*/
/* estimate number of lables 						*/   
    if (step <1.) 
        i = ((int)floor(maxValue - minValue) + 2) * 3;
    else
        i = ((int)floor(maxValue - minValue) + 2) / (int)step + 1;    
/* estimate number of tics						*/
    i = i * ticGroupsBetweenLabels * ticsInGroup;        
        
    lS->ticPrms = (ticStruct*)XtMalloc(sizeof(ticStruct)*i);

    v = firstLogIntLabel(baseValue, step, minValue, maxValue);
/* move start value just below minValue					*/
    if (v > minValue)
    	v -= step < 1. ?  1. : step;
    i = 0;
    for (j = 0; v <= maxValue; j++)
    {
        mkLogTicsGroup (minValue,  maxValue,  ticsInGroup, 
          (Boolean) (step < 1.), (Boolean) (j % ticGroupsBetweenLabels == 0),
          lS->ticPrms, v, &i, &lTmp);
   	if (ticGroupsBetweenLabels > 1)
            v += 1;
	else if (step < 1.)
	    v += 1;
	else
	    v += step;	
    }
/* drop "extra" labels if tics and labels number does not match.
   (This should not happen ever, but some problems could be with the
   last label because of different step values for tics and labels,
   and, hence, different rounding errors				*/
    if (lTmp != NULL)
    {
	labelsToDraw2 **tmp = &(lS->labels);
	while (*tmp != lTmp) tmp = &((*tmp)->next);
	destroyLabelsToDraw2(lTmp);
	*tmp = NULL;
    }
    lS->nTics = i;
}    

static void mkLogTicsGroup (double minValue, double maxValue, int ticsInGroup, 
    Boolean denseLables, Boolean labeledGroup, ticStruct *ticPrms,  
    double v, int *ticNo, labelsToDraw2 **lTmp)
{
    int i;
    for (i = 0; 
      i < ticsInGroup && 
      v + log10((double)(ticsInGroup == 3 ? i*i : i) + 1.) <= maxValue; 
      i++)
    if (aboveMin(v, i, ticsInGroup, minValue))
    {
        ticPrms[*ticNo].value = v + 
          log10((double)(ticsInGroup == 3 ? i*i : i) + 1.);
/* (i * i) above is used to provide simple transfer
  0 -> 0, 1->1, 2->4 for the case of ticsInGroup == 3			*/          
/* ticsInGroup might has values 1, 3 and 9 only			*/    	        
        if (i == 0)
    	{
    	    if (labeledGroup && (*lTmp) != NULL)
    	    {
    		(*lTmp)->ticNumber = *ticNo;
    		ticPrms[*ticNo].label = (*lTmp);
    		(*lTmp) = (*lTmp)->next;
    	    }
    	    else
    		ticPrms[*ticNo].label = NULL;
            if (ticsInGroup == 1)
            {
                if (labeledGroup)    		
    	           ticPrms[*ticNo].length = LONG_TIC_LEN;
    	        else
    	           ticPrms[*ticNo].length = MED_TIC_LEN;   
	    }
	    else 
	        ticPrms[*ticNo].length = LONG_TIC_LEN;    	           
   	}
    	else if ((ticsInGroup == 3 ? i*i : i) == 4)
    	{
    	    if (denseLables)
    	    {
    		if (labeledGroup && (*lTmp) != NULL)
    		{
    		    (*lTmp)->ticNumber = *ticNo;
    		    ticPrms[*ticNo].label = (*lTmp);
    		    (*lTmp) = (*lTmp)->next;
    		}
    		else
    		    ticPrms[*ticNo].label = NULL;
    		ticPrms[*ticNo].length = LONG_TIC_LEN;
	    }    	            
    	    else 
    	    {
    	        ticPrms[*ticNo].label = NULL;   
    	        ticPrms[*ticNo].length = MED_TIC_LEN;
 	    }    	            
    	}
    	else if (i== 1)
    	{
    	    if (denseLables)
    	    {
    		if (labeledGroup && (*lTmp) != NULL)
    		{
    		    (*lTmp)->ticNumber = *ticNo;
    		    ticPrms[*ticNo].label = (*lTmp);
    		    (*lTmp) = (*lTmp)->next;
    		}
    		else
    		    ticPrms[*ticNo].label = NULL;
    		ticPrms[*ticNo].length = LONG_TIC_LEN;
	    }    	            
    	    else    
    	    {
    	        ticPrms[*ticNo].label = NULL;   
    	        ticPrms[*ticNo].length = SHORT_TIC_LEN;
 	    }    	            
	}
	else
    	{
    	    ticPrms[*ticNo].label = NULL;   
    	    ticPrms[*ticNo].length = SHORT_TIC_LEN;
 	}    	            
        (*ticNo)++;
    }
}    	

/*
** Check if the (current) value is above min range value.
** This separate function is designed to provide the same "cut-off"
** condition for labels and tics encountering to assure correct link biulding
** between tics and lables
*/
static Boolean aboveMin(double stepValue, int subStepNo, int stepsInGroup,
    double minValue)
{
    return (stepValue + log10((double)
      (stepsInGroup == 3 ? subStepNo * subStepNo : subStepNo) + 1.) >= minValue);
/* (i * i) above is used to provide simple transfer
  0 -> 0, 1->1, 2->4 for the case of ticsInGroup == 3			*/          
}                    	            

/* 
**
** returned baseLValue might be out of <minValue,   maxValue>
** range if (log) step < 1. In this case no exact power of ten
** (with integer log) might appear in the range, and returned value
** will be the closest one, smaller then minValue
*/
void mkLogLabelsStep(XFontStruct *fs, double minValue, double maxValue, 
   XSegment *axisImage, double *baseLValue, double *logStep, char **format)  
{
    double minVL = log10(minValue);
    double maxVL = log10(maxValue);
    double minLBase;
    double maxLBase;
    
    minLBase = (int)floor (minVL);
    while ((double)(minLBase - 1) >= minVL)
        minLBase --;
    while ((double)minLBase < minVL)
        minLBase ++;

/* correction for the case when no integers are between  minVL and maxVL  */
   if (minLBase > maxVL)
       minLBase --;        
        
    maxLBase = (int)floor (maxVL);
    while ((double)(maxLBase + 1) <= maxVL)
        maxLBase ++;
    while ((double)maxLBase > maxVL)
        maxLBase --;
    *logStep = minLogLabelsStep(fs, minVL, maxVL,  
       minLBase, maxLBase, axisImage, format);   

    if (*logStep < log10(2.))
/* labels length might be bigger by 1 when using logStep < 1 in some cases
   (actually, only log10(2.) and log10(5./2.) will be used).
   Make more accurate estimation for the case
*/    
    {
        if (minLBase - log10(2.) >= minVL)
        {
            minLBase--;
	    *logStep = minLogLabelsStep(fs, minVL, maxVL,  
	       minLBase, maxLBase, axisImage, format);   
            if (*logStep > log10(2.))
            { 
                *logStep = 1.;
/* restore format						*/
        	minLBase++;
		*logStep = minLogLabelsStep(fs, minVL, maxVL,  
		   minLBase, maxLBase, axisImage, format);   
	    }
	}	    
    }	                
    if (*logStep < log10(2.))
        *logStep = log10(2.);
    else if (*logStep != floor (*logStep))
        *logStep = floor(*logStep) + 1.;
    *baseLValue = maxLBase;        
}                
    
  

static double minLogLabelsStep(XFontStruct *fs, double minLValue, 
   double maxLValue,  
   double minLBase, double maxLBase, XSegment *axisImage, char **format)   
{
/* assume minValue > 0, maxValue > 0 					*/
    int XCharWidth = maxDigXWidth(fs);
    int XCharHeight = maxLabelXHeight(fs);
    int length;
    double minLogStep;
    double rangeLength = maxLValue - minLValue; 
    
/* "magic numbers below are from the format considerations. The next
   representation can be helpfull.
   		log10 length
   1e-05	-5	5
   1e-04	-4	5
   0.001	-3	5
   0.01		-2	4
   0.1		-1	3
   1		 0	1
   10		 1	2
   100		 2	3
   1000		 3	4
   10000	 4	5
   1e+05	 5	5
   1e+06	 6	5
  The function will use variable precision with "f" format, in opposite
  to the fixed one in the linear labeling case
*/              
    if (minLBase < 0)
    {
       	if(minLBase > -4)
        {
            if (maxLBase > -minLBase + 1)
                length = maxLBase;
            else 
                length = -minLBase + 1;     

	} 
	else
	{
            if (maxLBase > -minLBase)
                length = maxLBase;
            else 
                length = -minLBase;     
   	}
    }
    else
        length = maxLBase;
    if (length <5)
    {
	length++;
	*format = "%.*f";
    }	
    else 
    {
        if (length <100) 
	    length = 5;
        else
	    length = 6;
	*format = "%.0e";
    }	

/* determine minimal Labeling step using geometry constrains,
    for the case when it is not smaller then baseStep			*/

          
    if (axisImage->y2 == axisImage->y1)
        minLogStep = rangeLength * 2. ;  /* "infinity" in this context	*/
    else	
    {
        double normSize;
        normSize = (double)(MIN_V_LABEL_SPACING + 1) * (double) XCharHeight /
          (double)abs(axisImage->y2 - axisImage->y1);
        minLogStep = normSize * rangeLength ;    
    } 


    if (axisImage->x2 != axisImage->x1)
    {
        double normSize;
	normSize = (double)(MIN_H_LABEL_SPACING + length) * (double) XCharWidth /
          (double)abs(axisImage->x2 - axisImage->x1);
	if (minLogStep > normSize * rangeLength)
	    minLogStep = normSize * rangeLength ;    
    }    	                     
    return (minLogStep);              
}
	  	                 		   
static int maxDigXWidth(XFontStruct *fs)
{
    char i='0';
    int k=0;
    for (i='0';i<'9';i++) if (XTextWidth(fs,&i,1)>k) k=XTextWidth(fs,&i,1);
    if (XTextWidth(fs,"-",1)>k) k=XTextWidth(fs,"-",1);
    if (XTextWidth(fs,"+",1)>k) k=XTextWidth(fs,"-",1);
    if (XTextWidth(fs," ",1)>k) k=XTextWidth(fs,"-",1);
    if (XTextWidth(fs,".",1)>k) k=XTextWidth(fs,"-",1);
    if (XTextWidth(fs,"e",1)>k) k=XTextWidth(fs,"-",1);
    return(k);
}

static int maxLabelXHeight(XFontStruct *fs)
{
    int ascent,descent;
    int dir;
    XCharStruct o;
    XTextExtents(fs,"0123456789.+-e",14,&dir,&ascent,&descent,&o);
    return (ascent+descent);
}

static int maxDigXHeight(XFontStruct *fs)
{
    int ascent,descent;
    int dir;
    XCharStruct o;
    XTextExtents(fs,"0123456789.+-e",14,&dir,&ascent,&descent,&o);
    return (ascent);
}
                               


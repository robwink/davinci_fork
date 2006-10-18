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
#include "segment_name.h"


static labeledSegment* createLabeledSegment(void);
static void mkInitLinValues(double minValue, 
    double maxValue, double *baseValue, double *baseStep, int *LSDOrder);
static void mkLinBaseStep(double minValue, 
    double maxValue, double *baseStep, int *LSDOrder);
static void mkLinBaseValue(double minValue, 
  double maxValue, double baseStep, double *baseValue);
static void mkLinLabelsStep(XFontStruct *fs, double minValue, double maxValue, 
  XSegment *axisImage, Boolean logScale, double *baseValue, double *baseStep, 
  int *stepFactor, char **format);   
static double minLabelsStep(XFontStruct *fs, double minValue, double maxValue, 
  double baseValue, int presicion, XSegment *axisImage, Boolean logScale,
  char **format);
static int minASCIILength (double value, int LSDOrder, char **format);
static double firstLabel( double baseValue, double step, 
  int stepFactor, double minValue, double maxValue);
static void mkLabelsValues ( double baseValue, double step, int stepFactor,
  char *format,  labeledSegment *lS);
static void mkLinTicValues ( double baseValue, double step, int stepFactor,
  labeledSegment *lS);
static void mkTics(labeledSegment *lS);        
static void adjustLabels (XFontStruct *fs, labeledSegment *lS);
static void clipLabels(XFontStruct *fs, labeledSegment *lS);
static int maxDigXWidth(XFontStruct *fs);
static int maxLabelXHeight(XFontStruct *fs);
static int maxDigXHeight(XFontStruct *fs);

labeledSegment* makeLabeledSegment(XFontStruct *fs, XSegment *seg, 
  float v0, float v1,
  Boolean logScaling, Boolean binnedLabeling, clipType clip0, clipType clip1,
  char *name, int clipX0, int clipX1)
{
    labeledSegment*lS;
    double baseStep;
    double baseValue;
    double minValue;
    double maxValue;
    double dv0;
    double dv1;
    char *format;
    int stepFactor;
    
    lS = createLabeledSegment();
    lS->axisImage = *seg;
    lS->logScaling = logScaling;
    lS->binnedLabeling = binnedLabeling;
    lS->clip0 = clip0;
    lS->clip1 = clip1;
/* range with the exact lenght 0 is unsupported				*/
    if (v0 == v1) return (lS);
    if (seg->x1 == seg->x2 && seg->y1 == seg->y2)
        return(lS);
/* Exted object spase range by the value corresponding to one third of a pixel.
   This is a heuristic method used to avoid "fuzzy" labels at the
   end of segment in the case then initial range is descrabed by integers
   in the object space. Without this correction end integer labeles
   would sometimes (depending on lebeling step, etc) disappear because of 
   calculation inaccuracy.
       The loss of labels placement precision due to the method is not     
   essential								*/
    {
        int segL;
        segL = sqrt((double)((seg->x2 - seg->x1) * (seg->x2 - seg->x1) +
          (seg->y2 - seg->y1) * (seg->y2 - seg->y1)));
        if (logScaling)
        {
            dv0 = pow(10., log10((double) v0) - 
                (log10((double) v1) - log10((double)v0)) / (double) segL * 
              (double)0.3);
	    dv1 = pow(10., log10((double) v1) + 
                (log10((double) v1) - log10((double)v0)) / (double) segL * 
              (double)0.3); 
        }        	               
        else
        {
            dv0 = (double) v0 - ((double) v1 - (double)v0) / (double) segL * 
              (double)0.3;
            dv1 = (double) v1 + ((double) v1 - (double)v0) / (double) segL * 
              (double)0.3;
	}              
        if (dv0 < dv1)
        {
            minValue = dv0;
            maxValue = dv1;          
        }
        else
        {
            minValue = dv1;          
            maxValue = dv0;          
        }
    }

    lS->v0 =dv0;
    lS->v1 =dv1;


    if (logScaling)
    {  
        double logStep;
        double logBaseValue;
        char *logFormat;
/* choose linear of log labeling to use					*/          
        mkLinLabelsStep(fs, minValue, maxValue, seg,  logScaling,
          &baseValue, &baseStep, &stepFactor, &format);  
 	mkLogLabelsStep(fs, minValue, maxValue, seg, 
 	  &logBaseValue, &logStep, &logFormat);  
 	if ((log10(maxValue) - log10(minValue))/ logStep >
 	    (maxValue - minValue)/baseStep/stepFactor) 
/* log lableling							*/	             
	{
/* the order of operations in this block is essential 		*/
	    lS->v0 = log10(lS->v0);
	    lS->v1 = log10(lS->v1);
	    mkLogLabelsValues (logBaseValue, logStep, logFormat, lS);
	    mkLogTicsValues (logBaseValue, logStep, lS);
       	}
       	else
       	{
/* the order of operations in this block is essential either 	*/
            mkLabelsValues (baseValue, baseStep, stepFactor, format, lS);
            mkLinTicValues (baseValue, baseStep, stepFactor, lS);
	    lS->v0 = log10(lS->v0);
	    lS->v1 = log10(lS->v1);
       	}	    
    }
    else
    {
        mkLinLabelsStep(fs, minValue, maxValue, seg,  logScaling,
          &baseValue, &baseStep, &stepFactor, &format);  
        mkLabelsValues (baseValue, baseStep, stepFactor, format, lS);
        mkLinTicValues (baseValue, baseStep, stepFactor, lS);
    }        
    mkTics(lS);
    adjustLabels (fs,  lS);
    clipLabels(fs, lS);
    if (name != NULL )
         placeName (lS, fs, name, seg,  clipX0, clipX1,
        lS->tics, lS->nTics, lS->labels );
    return(lS);
}
/* create data structure and initialize "output" parameters (which
   should not be setted from the outside)				*/        				   				           
static labeledSegment* createLabeledSegment(void)
{
    labeledSegment* lS;
    lS = (labeledSegment*)XtMalloc(sizeof(labeledSegment));
    lS->nTics =0;
    lS->tics = NULL;
    lS->ticPrms = NULL;
    lS->labels = NULL;
    lS->name = NULL;
    return(lS);
}

void drawLabeledSegment(Display *d, Drawable dr, GC gc, labeledSegment* lS)
{
    labelsToDraw2 *lab;
    if (lS == NULL) return; 
    XDrawSegments(d, dr, gc, &(lS->axisImage), 1);
    if (lS->nTics != 0 && lS->tics != NULL)
        XDrawSegments(d, dr, gc, lS->tics, lS->nTics);
    for (lab = lS->labels; lab != NULL; lab = lab->next)
	XDrawString(d, dr, gc, lab->wX, lab->wY, lab->label,lab->length);
    for (lab = lS->name; lab != NULL; lab = lab->next)
	XDrawString(d, dr, gc, lab->wX, lab->wY, lab->label,lab->length);
}                    
     

void destroyLabeledSegment(labeledSegment* lS)
{
    if(lS != NULL)
    {
        XtFree((char*)lS->tics);
        XtFree((char*)lS->ticPrms);
        destroyLabelsToDraw2(lS->labels);
        destroyLabelsToDraw2(lS->name);
        XtFree((char*)lS);
    }
}

/* destructor for <labelToDraw>, destructs all list of
   labels, starting from current and DESTRUCTS character representation
   of the label (field <label> of the structure) also 			*/
void destroyLabelsToDraw2(labelsToDraw2 *labels)
{
    labelsToDraw2 *tmp;
    while (labels != NULL)
    {
        XtFree((char*)labels->label);
        tmp = labels;
        labels = labels->next;
        XtFree((char*)tmp);
    }
}

/*
**  static void mkInitLinValues(double minValue, 
**    double maxValue, double *baseValue, double *baseStep, int *LSDOrder)
**
**  minValue	minimal value in the range	(in)
**  maxValue    maximal velue int the range	(in)
**  *baseStep	"resonable" value to use as step between labels
**		which allows one to ten labels in the range.
**		Calculated regardless of geometry consraines (out)
**  *baseValue  the value inside the range which cosidered
**		as a "best" value to label		     (out)
**
**  *LSDOrder	less significant digit order of label values
**		using appropriate precision for labeling
**		with baseStep
**
**	calculates <*step> and <*baseValue> for linear labeling
** based on range parameters only, regardless geometry consraines.
*/

static void mkInitLinValues(double minValue, 
    double maxValue, double *baseValue, double *baseStep, int *LSDOrder)
{
    int maxNStep;
    int minNStep;
/* calculate  step in the form of exact power of 10 which fit the range */
    *LSDOrder = floor(log10(maxValue - minValue));
    *baseStep = pow((double)10.,(double)*LSDOrder);
/* try to eliminate the effect of possible rounding errors (in the 
    fuction <floor> mainly						*/
    while (*baseStep > (maxValue - minValue))
    {
	*baseStep /= (double)10.;
	(*LSDOrder) --;
    }
    while (*baseStep * (double) 10. <= (maxValue - minValue))    
    {
	*baseStep *= (double)10.;
	(*LSDOrder) ++;
    }   
/*
** choose the the number of form <integer> * (*baseStep) in the range
** which has  the longest representation   
*/     
    if (maxValue >= 0.)
    {
   	 maxNStep = floor(maxValue/ (*baseStep));
/* try to eliminate the effect of possible rounding errors		*/
         while (maxNStep * (*baseStep) > maxValue )
  	     maxNStep--;
         while ((maxNStep + 1) * (*baseStep) <= maxValue )    
	     maxNStep++;
    }	    
    if (minValue <= 0.)
    {
	minNStep = floor(minValue/ (*baseStep)) + 1;
/* try to eliminate the effect of possible rounding errors		*/
        while (minNStep * (*baseStep) < minValue )
  	    minNStep++;
        while ((minNStep - 1) * (*baseStep) >= minValue )    
	    minNStep--;
   }
   if (minValue <= 0.)
   {
       if (maxValue >= 0.)
       {
/* in this case minNStep <= 0 <= maxNStep; maxNStep - minNStep <= 10    */       
           if (-minNStep > 0)
               maxNStep = minNStep;
       }        
       else
           maxNStep = minNStep; 
    }
    *baseValue = maxNStep * (*baseStep);
}

/*
**  static void mkLinBaseStep(double minValue, 
**    double maxValue,  double *baseStep, int *LSDOrder)
**
**  minValue	minimal value in the range	(in)
**  maxValue    maximal velue int the range	(in)
**  *baseStep	"resonable" value to use as step between labels
**		which allows one to ten labels in the range.
**		Calculated regardless of geometry consraines (out)
**  *LSDOrder	less significant digit order of label values
**		using appropriate precision for labeling
**		with baseStep
**
**	calculates <*step>  for linear labeling
** based on range parameters only, regardless geometry consraines.
*/

static void mkLinBaseStep(double minValue, 
    double maxValue, double *baseStep, int *LSDOrder)
{
/* calculate  step in the form of exact power of 10 which fit the range */
    *LSDOrder = floor(log10(maxValue - minValue));
    *baseStep = pow((double)10.,(double)*LSDOrder);
/* try to eliminate the effect of possible rounding errors (in the 
    fuction <floor> mainly						*/
    while (*baseStep > (maxValue - minValue))
    {
	*baseStep /= (double)10.;
	(*LSDOrder) --;
    }
    while (*baseStep * (double) 10. <= (maxValue - minValue))    
    {
	*baseStep *= (double)10.;
	(*LSDOrder) ++;
    }   
}

/*
**  static void mkLinBaseValue(double minValue, 
**    double maxValue, double baseStep,  double *baseValue)
**
**  minValue	minimal value in the range	(in)
**  maxValue    maximal velue int the range	(in)
**  baseStep	"resonable" value to use as step between labels
**  *baseValue  the value inside the range with possible
**		longest ASCII representation		     (out)
**
**
**	calculates  <*baseValue> for linear labeling
** based on range parameters only, regardless geometry consraines.
*/

static void mkLinBaseValue(double minValue, 
  double maxValue, double baseStep, double *baseValue)
{
    int maxNStep;
    int minNStep;
   
/*
** choose the the number of form <integer> * (baseStep) in the range
** which has  the longest representation   
*/     
    if (maxValue >= 0.)
    {
   	 maxNStep = floor(maxValue/ (baseStep));
/* try to eliminate the effect of possible rounding errors		*/
         while ((double)maxNStep * baseStep > maxValue )
  	     maxNStep--;
         while ((double)(maxNStep + 1) * baseStep <= maxValue )    
	     maxNStep++;
    }	    
    if (minValue <= 0.)
    {
	minNStep = floor(minValue/ (baseStep)) + 1;
/* try to eliminate the effect of possible rounding errors		*/
        while ((double)minNStep * baseStep < minValue )
  	    minNStep++;
        while ((double)(minNStep - 1) *  baseStep >= minValue )    
	    minNStep--;
    }
    if (minValue < 0.)
    {
        if (maxValue >= 0.)
     	{
/* in this case minNStep <= 0 <= maxNStep; maxNStep - minNStep <= 100,
   make use of this, choosing the value with the longest representation   */ 
            if (-minNStep >= 10)
                maxNStep = minNStep;
            else if (maxNStep < 10)
            {
                if (minNStep < 0)
                    maxNStep = minNStep;
	    }                   
        }        
        else
            maxNStep = minNStep; 
    }
    *baseValue = (double)maxNStep * baseStep;
}

/* 
** static void mkLinLabelsStep(XFontStruct *fs, double minValue, double maxValue, 
**  XSegment *axisImage, Boolean logScale, double *baseValue, double *baseStep, 
**  int *stepFactor, char **format)   
**
**  fs 	 	pointer to a font structure for a font choosen to
**		print labels
**  minValue 	object space value, corresponding to the range beginning  
**  maxValue 	object space value, corresponding to the range end 
**  axisImage   window coordinates representation of the segment to label
**  *baseValue	value in the range [minValue, maxValue] with maximim
**		representaiton length choosen to be labeled.
**  *baseStep	base labels step, exact power of ten. 
**  *stepFactor	- output parameter - step for labeling choosen whith
**		geometry consideration is equal to <*baseStep> * <*stepFactor>.
**		Step factor is needed separately to calculate tics placing
**		during  further processing.  
**		
** calculate actual lables step taking into account geometry
** constrains 
*/
static void mkLinLabelsStep(XFontStruct *fs, double minValue, double maxValue, 
  XSegment *axisImage, Boolean logScale, double *baseValue, double *baseStep, 
  int *stepFactor, char **format)   
{
    int LSDOrder;
    double minStep;
    double oldBaseValue;
    double oldBaseStep;
    if (fs == NULL)
    {
        *stepFactor = 1;
        *format = NULL;
        return;
    }  
        
    mkLinBaseStep(minValue, maxValue,  baseStep, &LSDOrder);
  
    mkLinBaseValue(minValue, maxValue, *baseStep, baseValue);

/* determine minimal Labeling step using geometry constrains,
    for the case when it is not smaller then baseStep			*/

    minStep = minLabelsStep(fs, minValue,  maxValue, *baseValue, LSDOrder,
      axisImage, logScale, format);  
              
   if (minStep > *baseStep)
   {
       if(minStep < *baseStep * 2.)
           *stepFactor = 2; 
       else if(minStep < *baseStep * 5.)
           *stepFactor = 5;
       else if(minStep < *baseStep * 10.)
           *stepFactor = 10;
       else
           *stepFactor = 100; /* "infinity" in this context	*/    
    }
    else
/* the smaller step could be chosen, but the LSDOrder will be less by 1,
   so estimated length should be ajusted first				*/
    {
/* recalculate the step							*/
        oldBaseValue = *baseValue;
        oldBaseStep = *baseStep;
        *baseStep /= 10.;
        LSDOrder --;
        
     	mkLinBaseValue(minValue, maxValue, *baseStep,  baseValue);
               
       	minStep = minLabelsStep(fs, minValue, maxValue, *baseValue, LSDOrder,
          axisImage, logScale, format); 
           
       	if(minStep < *baseStep )
            *stepFactor = 1;
       	else if(minStep < *baseStep * 2.)
            *stepFactor = 2; 
       	else if(minStep < *baseStep * 5.)
            *stepFactor = 5;
       	else
       	{
/* impossible to have smaller step, restore previous values		*/       
           *baseValue = oldBaseValue;
           *baseStep = oldBaseStep;
           *stepFactor = 1;
           LSDOrder++;
/* restore proper format string						*/           
           (void) minASCIILength (*baseValue,  LSDOrder,  format);
       }    
    }
}

static double minLabelsStep(XFontStruct *fs, double minValue, double maxValue,  
  double baseValue, int presicion, XSegment *axisImage, 
  Boolean logScale, char **format)   
{
/* assume that the range parameters (minValue, maxValue) are correct!! */
    int XCharWidth = maxDigXWidth(fs);
    int XCharHeight = maxLabelXHeight(fs);
    double rangeLength;
    double normSize;
    double minStep;
    int length;
    
    rangeLength =  maxValue -  minValue;    

/* estimate the length of baseValue , this is longest
   possible label unless we use smaller step				*/
    length = minASCIILength (baseValue,  presicion,  format);
    
/* determine minimal Labeling step using geometry constrains,
    for the case when it is not smaller then baseStep			*/

          
    if (axisImage->y2 == axisImage->y1)
        minStep = (maxValue -  minValue) * 2. ;  /* "infinity" in 
this context	*/
    else	
    {
        normSize = (double)(MIN_V_LABEL_SPACING + 1) * (double) XCharHeight /
          (double)abs(axisImage->y2 - axisImage->y1);
        minStep = normSize * rangeLength ;    
        if (logScale)
        { 
/* in this case calculated <minStep> will provide the same "average" labeled
   density as it would in linear case. But because intervals between some of
   labels will be narrower, next check is needed to prevent labels
   collision								*/
/*  smaller MIN_LOG_V_LABEL_SPACING used to prevent cut off of too many labels */             	
            normSize = (double)(MIN_LOG_V_LABEL_SPACING + 1) * 
              (double) XCharHeight /
              (double)abs(axisImage->y2 - axisImage->y1);
	    if (minStep < baseValue - pow(10., log10(baseValue) - 
              normSize * (log10(maxValue) - log10(minValue))))
                minStep = baseValue - pow(10., log10(baseValue) - 
              normSize * (log10(maxValue) - log10(minValue)));	
        }
    } 

     
    if (axisImage->x2 != axisImage->x1)
    {
        normSize = (double)(MIN_H_LABEL_SPACING + length) * (double) XCharWidth /
          (double)abs(axisImage->x2 - axisImage->x1);
	if (minStep > normSize * rangeLength)
	{
            if (logScale)
            {
                double minStepHor = normSize * rangeLength;
/* use the same kind estimation of the narrowest interval as above	*/            
        	normSize = (double)(MIN_LOG_H_LABEL_SPACING + length) * 
        	  (double) XCharWidth /
        	  (double)abs(axisImage->x2 - axisImage->x1);
	        if (minStepHor < baseValue - pow(10., log10(baseValue) - 
              	  normSize * (log10(maxValue) - log10(minValue))))
                    minStepHor = baseValue - pow(10., log10(baseValue) - 
              	    normSize * (log10(maxValue) - log10(minValue)));
	    	if (minStepHor < minStep)
	    	    minStep = minStepHor;              	    	
	    }                	 
	    else
	            minStep = normSize * rangeLength ;    
    	} 
    }    	                     
    return (minStep);              
}



static int minASCIILength (double value, int LSDOrder, char **format)
{
    int vOrder;
    int ePrec;		/* precision for labeling in 'e' format		*/
    int fPrec;  	/* precision for labeling in 'f' format		*/
    int eLength;	/* label length (in char) in 'e' format		*/
    int fLength;	/* label length (in char) in 'f' format		*/
    int length;
    static char formatBuffer[80]; /* buffer to construct format string	*/
    char buffer[80];	  	 /* buffer to construct label string	*/

    if ((double)fabs(value) == (double) 0.)
        vOrder = 0;
    else
    {    
	vOrder = floor(log10((double)fabs(value)));
/* try to eliminate the effect of possible rounding errors		*/
	while (pow((double)10., (double)(vOrder)) > fabs(value))
  	    vOrder--;
	while (pow((double)10., (double)(vOrder+1)) <= fabs(value))    
	    vOrder++;
    }	   
    if (vOrder < LSDOrder)	
        vOrder = LSDOrder;
    if ((double)fabs(value) == (double) 0.) 
        ePrec = 0;
    else
        ePrec = vOrder - LSDOrder;
    fPrec = LSDOrder < 0 ? -LSDOrder : 0;
/* which format is shorter - e or f? calculate lengths for both and choose
    shortest one								*/  
    *format = formatBuffer; 
    sprintf(formatBuffer,"%%.%df",fPrec);
    sprintf(buffer,formatBuffer,value);
    fLength = strlen(buffer);
    sprintf(formatBuffer,"%%.%de",ePrec); 
    sprintf(buffer,formatBuffer,value);
    eLength = strlen(buffer);
    if (eLength < fLength)
       length = eLength;
    else
    {
/* restore format buffer 						*/
        sprintf(formatBuffer,"%%.%df",fPrec);
        length = fLength;
    }
    return (length);
}           

/* calculate lowerst value to label in the range, self evident code	*/
static double firstLabel( double baseValue, double step, 
  int stepFactor, double minValue, double maxValue)
{
    int i;
    
    i = (int)floor(baseValue/step);
    while (((double)i + .5) * step < baseValue)
        i++;
    while (((double)i - .5) * step >= baseValue)
        i--;
    if (stepFactor == 2)
    {
        if (i%2 != 0)
        {
            if (baseValue + step <= maxValue)
                baseValue += step;
            else if (baseValue - step >= minValue)
                baseValue -= step;
	}
    }
    else if (stepFactor == 5)
    {
	while (i%5 != 0)
	{
	    if (baseValue + step > maxValue) break;
	    i++;
	    baseValue += step;
	}
	while (i%5 != 0)
	{
	    if (baseValue - step < minValue) break;
	    i--;
	    baseValue -= step;
	}
    }	    	        
    while (baseValue - step * (double) stepFactor >= minValue)
        baseValue -= step * (double) stepFactor;
    return(baseValue);
}            	        	                        

/* set values and print all labels					*/
/*
** static void mkLabelsValues ( double baseValue, double step, int stepFactor,
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

static void mkLabelsValues ( double baseValue, double step, int stepFactor,
  char *format,  labeledSegment *lS)
{
    labelsToDraw2 **curLab;
    char buf[80];
/* assume lS->labels is not allocated					*/
    double v;
    double minValue = (lS->v0 < lS->v1) ? lS->v0 : lS->v1;
    double maxValue = (lS->v0 < lS->v1) ? lS->v1 : lS->v0;
    v = firstLabel(baseValue, step,  stepFactor, minValue, maxValue);          
/*  generate labels values						*/
    curLab = &(lS->labels);     
    while (v <= maxValue)
    {
        *curLab = (labelsToDraw2*)XtMalloc(sizeof(labelsToDraw2));
/* set geometry position of the labels					*/
        if (lS->logScaling)
            (*curLab)->lValue = log10(v);
        else    
            (*curLab)->lValue = v;
        sprintf (buf, format, v);    
/* length of the label in chars						*/        
        (*curLab)->length = strlen(buf);
        (*curLab)->label = (char*)XtMalloc(sizeof(char)*((*curLab)->length+1));
	strcpy((*curLab)->label,buf);
	curLab = &((*curLab)->next);
        v += step * (double)stepFactor;
    }                
    *curLab = NULL;
}         

/* calculate and print tic values + make label-tic links		*/

static void mkLinTicValues ( double baseValue, double step, int
    stepFactor, labeledSegment *lS)
{
    double minTicStep;
    int ticsBetweenLabels;
    double ticStep;
    double minValue = (lS->v0 < lS->v1) ? lS->v0 : lS->v1;
    double maxValue = (lS->v0 < lS->v1) ? lS->v1 : lS->v0;
    double v;
    double firstLab;
    double rangeLength;
    int i,j;
    rangeLength =  maxValue -  minValue;    
    firstLab = firstLabel(baseValue, step,  stepFactor, minValue, maxValue);          
/* assume labels is setted up already					*/
    if ((lS->axisImage.x2 == lS->axisImage.x1) &&
      (lS->axisImage.y2 == lS->axisImage.y1))
        minTicStep = rangeLength * 2.;
    else 
        minTicStep = (double)(MIN_TIC_SPACING) /
          (double)vectorLength(makeVector(lS->axisImage.x2 - lS->axisImage.x1,
          lS->axisImage.y2 - lS->axisImage.y1)) * rangeLength;
    if (lS->logScaling)
/* check if narrowest spacing OK					*/
        if (minTicStep <  maxValue - pow(10.,log10(maxValue) -
          (double)(MIN_LOG_TIC_SPACING) /
          (double)vectorLength(makeVector(lS->axisImage.x2 - lS->axisImage.x1,
          lS->axisImage.y2 - lS->axisImage.y1)) * 
          (log10(maxValue) - log10(minValue))))
            minTicStep = maxValue - pow(10.,log10(maxValue) -
              (double)(MIN_LOG_TIC_SPACING) /
              (double)vectorLength(makeVector(lS->axisImage.x2 - lS->axisImage.x1,
              lS->axisImage.y2 - lS->axisImage.y1)) *
              (log10(maxValue) - log10(minValue)));
/* use labels step if the last is smaler for any reason			*/
    if (minTicStep > step * (double)stepFactor)
        minTicStep = step * (double)stepFactor;
/* number of tics to place between labels including one of labeled tics */ 	
    ticsBetweenLabels = floor (step * (double)stepFactor / minTicStep);
/* check rounding 							*/        
    while (ticsBetweenLabels * minTicStep > step * (double)stepFactor)
        ticsBetweenLabels--;
    while ((ticsBetweenLabels + 1) * minTicStep <= step * (double)stepFactor)
        ticsBetweenLabels++; 
        
    if (ticsBetweenLabels >= 10)
        ticsBetweenLabels = 10;
    else if (ticsBetweenLabels >= 5)
        ticsBetweenLabels = 5;
    else if (ticsBetweenLabels >= 2)
        ticsBetweenLabels = 2;
    else
        ticsBetweenLabels = 1;        
/* choose the number of the tics between labels (and, hence, tic step)	*/
    switch (stepFactor) {	
        case 1: break; /* any possible tic number between labes is OK	*/
        case 2: if (ticsBetweenLabels == 5)
                    ticsBetweenLabels = 2;
        	break;
        case 5: if (ticsBetweenLabels == 10)
                    ticsBetweenLabels = 5;
        	if (ticsBetweenLabels == 2)
                    ticsBetweenLabels = 1;
                break;
        case 10: break;
        case 100: ticsBetweenLabels = 1;
        	break;                
    }
    ticStep = step * (double)stepFactor/ticsBetweenLabels;
    
/* determine tics number (not very accurate, just estimation)		*/
    i= floor((maxValue - minValue)/ticStep) + 2;
    lS->ticPrms = (ticStruct*)XtMalloc(sizeof(ticStruct)*i);
/* "syncronize" tics with label list to provide cross reference		*/
    v = firstLab ;
/* number of the tic corresponding to the first label in the tic array	*/    
    for (i = 0; v >= minValue ; v -= ticStep, i++);
/* make tics down from the first label					*/
    v = firstLab ;
    v -= ticStep;
    for (i-=2, j=1; i >=0;  v -= ticStep, i--, j++)
    {
        if (lS->logScaling)
     	    (lS->ticPrms)[i].value = log10(v);
     	else
     	    (lS->ticPrms)[i].value =  v;
     	(lS->ticPrms)[i].label = NULL;
    	if (j % ticsBetweenLabels == 0)
    	    (lS->ticPrms)[i].length = LONG_TIC_LEN;
    	else if ((j*2) % ticsBetweenLabels == 0)
    	    (lS->ticPrms)[i].length = MED_TIC_LEN;
    	else
    	    (lS->ticPrms)[i].length = SHORT_TIC_LEN;
    }
/* make tics up from the first label					*/
    {
        labelsToDraw2 *lTmp = lS->labels;
	i = j - 1;
	v = firstLab;
	for ( j=0; v <= maxValue;  v += ticStep, i++, j++)
	{
            if (lS->logScaling)
     		(lS->ticPrms)[i].value = log10(v);
     	    else
     		(lS->ticPrms)[i].value =  v;
    	    if (j % ticsBetweenLabels == 0)
    	    {
    	        if (lTmp != NULL)
    	        {
    	            lTmp->ticNumber = i;
    	            (lS->ticPrms)[i].label = lTmp;
    	            lTmp = lTmp->next;
    	        }
    	        else
    	            (lS->ticPrms)[i].label = NULL;
    		(lS->ticPrms)[i].length = LONG_TIC_LEN;
    	    }
    	    else 
    	    {
    	        (lS->ticPrms)[i].label = NULL;
    		if ((j*2) % ticsBetweenLabels == 0)
    		    (lS->ticPrms)[i].length = MED_TIC_LEN;
    		else
    		    (lS->ticPrms)[i].length = SHORT_TIC_LEN;
    	    }
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
    }
    lS->nTics = i;
}    

/* make X11 tics segments based on ticPrms setted in the labeledSegment 
   structure.        							*/
static void mkTics(labeledSegment *lS)        
{
/* tic direction							*/  
    vector axis;						 
    vector direction;
    vector tic;
    vector shortTic;
    vector mediumTic;
    vector longTic;
    vector ticApprox [5];
    double ticProd;
    int i,j,k; 
    double dX, dY, dS;
    int dd, dx, dy;
    int dax, day;
    int x, y;
    int scalarProduct2;
    axis = makeVector(lS->axisImage.x2 - lS->axisImage.x1,
      lS->axisImage.y2 - lS->axisImage.y1);
    direction = mulVector (ortogonal(axis), -1);
    if (lS->nTics==0 || lS->ticPrms == NULL)
        return;
    dx = direction.x > 0 ? 1 : (direction.x < 0 ? -1 : 0);
    dy = direction.y > 0 ? 1 : (direction.y < 0 ? -1 : 0);
    dax = axis.x > 0 ? 1 : (axis.x < 0 ? -1 : 0);
    day = axis.y > 0 ? 1 : (axis.y < 0 ? -1 : 0);
    dd = abs(direction.x) > abs(direction.y) ? 
      abs(direction.x) : abs(direction.y);
      
/* build tic vector approximation					*/      
    k = 0;  
    for (i=0; i<=2; i++)
        for (j=0; j<=2; j++)
            if ( abs(i * dx) == 2 || abs(j * dy) == 2)
                 ticApprox[k++] = makeVector(i * dx, j * dy);
    k--;
/* in regular cases <k> should not be 0                     */
    ticProd = fabs((double)scalarProduct(ticApprox[k], axis) / 
      vectorLength(ticApprox[k]));
    j = k;                  
    for (k-- ; k>=0; k--)
        if (fabs((double)scalarProduct(ticApprox[k], axis) / 
          vectorLength(ticApprox[k])) < ticProd)
        {
            ticProd = fabs((double)scalarProduct(ticApprox[k], axis) / 
              vectorLength(ticApprox[k]));
            j = k;
        }
    shortTic = mulVector(ticApprox[j],2);
    mediumTic = mulVector(ticApprox[j],3);
    longTic = mulVector(ticApprox[j],4);
      
    lS->tics = (XSegment*)XtMalloc(sizeof(XSegment)*(lS->nTics));
    for(i=0; i<lS->nTics; i++)
    {
/* assume, that visible range in the object coordinates is not too short
   to cause floating exeption. In the 2d hist widget it is supported by
   the widget processing						*/
/* try to eliminate some kind of aliasing				*/
	dX = ((double)lS->ticPrms[i].value - (double)lS->v0) /
            ((double)lS->v1 - (double)lS->v0) * 
            (double)(lS->axisImage.x2 - lS->axisImage.x1);
	dY = ((double)lS->ticPrms[i].value - (double)lS->v0) /
            ((double)lS->v1 - (double)lS->v0) * 
            (double)(lS->axisImage.y2 - lS->axisImage.y1);
        x = (int)floor(dX + (double)0.5);
        y = (int)floor(dY + (double)0.5);
        scalarProduct2 = 2 * (x * direction.x + y * direction.y);
        dS = dX * (double)axis.x + dY * (double)axis.y; 
        if (scalarProduct2 > dd)  
      	{
            int x1, x2;
            int y1, y2;
            x1 = x - dx;
            y1 = y;
            x2 = x;
            y2 = y - dy;
            dS = dX * (double)axis.x + dY * (double)axis.y; 
            if (fabs((double)(x1 *  axis.x +  y1 *  axis.y) - dS) <
              fabs((double)(x2 * axis.x + y2 * axis.y) - dS))
            {
                lS->tics[i].x1 = x1;
                lS->tics[i].y1 = y1;
            }
            else
            {
               	lS->tics[i].x1 = x2;
               	lS->tics[i].y1 = y2;
            } 
	} 
	else if(scalarProduct2 <= -dd)
      	{
            int x1, x2;
            int y1, y2;
            x1 = x + dx;
            y1 = y;
            x2 = x;
            y2 = y + dy;
            if (fabs((double)(x1 *  axis.x +  y1 *  axis.y) - dS) <
              fabs((double)(x2 * axis.x + y2 * axis.y) - dS))
            {
                lS->tics[i].x1 = x1;
                lS->tics[i].y1 = y1;
            }
            else
            {
               	lS->tics[i].x1 = x2;
               	lS->tics[i].y1 = y2;
            } 
	} 
	else 
	{
            int x1, y1;
	    if ((double)(x *  axis.x +  y *  axis.y) > dS)
	    {
    /* go opposite axis	direction					*/	
        	if (scalarProduct2 - 2 * dax * direction.x > -dd &&
        	  scalarProduct2 - 2 * dax * direction.x <= dd)
        	{
                    x1 = x - dax;
                    y1 = y;
        	}
        	else if (scalarProduct2 - 2 * day * direction.y > -dd &&
        	  scalarProduct2 - 2 * day * direction.y <= dd)
        	{
                    x1 = x;
                    y1 = y - day;
        	}  
		else
		{
                    x1 = x - dax;
                    y1 = y - day;
        	}
            }
            else
            {
    /* go along axis direction						*/	
        	if (scalarProduct2 + 2 * dax * direction.x > -dd &&
        	  scalarProduct2 + 2 * dax * direction.x <= dd)
        	{
                    x1 = x + dax;
                    y1 = y;
        	}
        	else if (scalarProduct2 + 2 * day * direction.y > -dd &&
        	  scalarProduct2 + 2 * day * direction.y <= dd)
        	{
                    x1 = x;
                    y1 = y + day;
        	}  
		else
		{
                    x1 = x + dax;
                    y1 = y + day;
        	}
            }
            if (fabs((double)(x1 *  axis.x +  y1 *  axis.y) - dS) <
              fabs((double)(x * axis.x + y * axis.y) - dS))
            {
                lS->tics[i].x1 = x1;
                lS->tics[i].y1 = y1;
            }
            else
            {
               	lS->tics[i].x1 = x;
               	lS->tics[i].y1 = y;
            }
        }
/* test code							       
**        scalarProduct2 = 2 * (lS->tics[i].x1 * direction.x + 
**          lS->tics[i].y1 * direction.y);
**        if (scalarProduct2 <= -dd || scalarProduct2 > dd  )
**            printf ("tic error\n");
*/            
/* end of test code						*/            
        lS->tics[i].x1 += lS->axisImage.x1;
        lS->tics[i].y1 += lS->axisImage.y1;  
        switch ( lS->ticPrms[i].length) {
            case SHORT_TIC_LEN : tic = shortTic; break;
            case MED_TIC_LEN : tic = mediumTic; break; 
            case LONG_TIC_LEN : tic = longTic; break;
            default:tic = shortTic; break;
        }            
/*	tic = setLength(direction,lS->ticPrms[i].length);		*/
        lS->tics[i].x2 = lS->tics[i].x1 + tic.x;
        lS->tics[i].y2 = lS->tics[i].y1 + tic.y;
    }
}    	                                     

/* adjust labels to the tics						*/ 
/* calculate X11 coordinates for labels printing.
   assume X11 representation of tics is built in the <lS> BEFORE
   this call								*/

static void adjustLabels (XFontStruct *fs, labeledSegment *lS)
{
    labelsToDraw2 *label;
    int digWidth;
    int digHeight;
    vector seg; 
    int vShift, hShift;
    int maxLabelWidth;
    
    if (fs == NULL) return;
    
    digWidth = maxDigXWidth(fs);
    digHeight = maxDigXHeight(fs);
    seg = makeVector(lS->axisImage.x2 - lS->axisImage.x1,
      lS->axisImage.y2 - lS->axisImage.y1);
    if (lS->labels == NULL)
       return;
    for(label = lS->labels, maxLabelWidth=0;label != NULL; label = label->next) 
        if (XTextWidth(fs,label->label, label->length) > maxLabelWidth)
            maxLabelWidth = XTextWidth(fs,label->label, label->length);
    if (seg.y > 0)
    {
        if (seg.x > 0)
        {
            if ( seg.x * digHeight < seg.y * digWidth)
/* side placing 							*/
    	    {	
    	        lS->sidePlacing = TRUE;
                vShift= floor(((double)digWidth/2.+1.) * 
                  (double) seg.x /(double)seg.y + (double)digHeight/2.);
                for(label = lS->labels;label != NULL; label = label->next) 
		{
		    label->XWidth = XTextWidth(fs,label->label, label->length);
		    label->wX = (lS->tics)[label->ticNumber].x2 - 1 - 
		      label->XWidth;
		    label->wY = (lS->tics)[label->ticNumber].y2 + vShift;
		}	 
             } else
/* down placing								*/    
             { 
    	        lS->sidePlacing = FALSE;
	        if(seg.y!=0)
        	    hShift=floor((double)(LONG_TIC_LEN>MED_TIC_LEN ?
        	      LONG_TIC_LEN - MED_TIC_LEN : 1)*
        	      (double)seg.x/(double)seg.y);
	        else 
	            hShift = maxLabelWidth;
                for(label = lS->labels;label != NULL; label = label->next) 
		{
		    int center;
		    label->XWidth = XTextWidth(fs,label->label, label->length);
		    center = label->XWidth/2;
		    if (center > hShift) 
		        center=hShift;
		    label->wX = (lS->tics)[label->ticNumber].x2 - 
		      label->XWidth + center + 1;
		    label->wY = (lS->tics)[label->ticNumber].y2 + 1 + digHeight;
		}
	    }
	}
	else /* seg.x <= 0 */
        {
            if ( - seg.x * digHeight < seg.y * digWidth)
/* side placing 							*/
    	    {	
    	        lS->sidePlacing = TRUE;
                vShift= floor(((double)digWidth/2.+1.) * 
                  (double) seg.x /(double)seg.y + (double)digHeight/2.);
                for(label = lS->labels;label != NULL; label = label->next) 
		{
		    label->XWidth = XTextWidth(fs,label->label, label->length);
		    label->wX = (lS->tics)[label->ticNumber].x2 - 1 - 
		      label->XWidth;
		    label->wY = (lS->tics)[label->ticNumber].y2 + vShift;
		}	 
            } else
/* down placing								*/    
            { 
    	        lS->sidePlacing = FALSE;
	        if(seg.y!=0)
        	    hShift=floor((double)(LONG_TIC_LEN>MED_TIC_LEN ?
        	      LONG_TIC_LEN - MED_TIC_LEN : 1)*
        	      (double)( - seg.x)/(double)seg.y);
	        else 
	            hShift = maxLabelWidth;
                for(label = lS->labels;label != NULL; label = label->next) 
		{
		    int center;
		    label->XWidth = XTextWidth(fs,label->label, label->length);
		    center = label->XWidth/2;
		    if (center > hShift) 
		        center=hShift;
		    label->wX = (lS->tics)[label->ticNumber].x2 - 
		      label->XWidth + center + 1;
		    label->wY = (lS->tics)[label->ticNumber].y2 - 1;
		}
	    }
	}
    }	
    else /* seg.y <= 0 */
    {
        if (seg.x > 0)
        {
            if (  seg.x * digHeight <  - seg.y * digWidth)
/* side placing 							*/
    	    {	
    	        lS->sidePlacing = TRUE;
                vShift= floor(((double)digWidth/2.+1.) * 
                  (double) seg.x /(double)( - seg.y) + (double)digHeight/2.);
                for(label = lS->labels;label != NULL; label = label->next) 
		{
		    label->XWidth = XTextWidth(fs,label->label, label->length);
		    label->wX = (lS->tics)[label->ticNumber].x2 + 1 + 1;
		    label->wY = (lS->tics)[label->ticNumber].y2 + vShift;
		    
		}	 
            } else
/* down placing								*/    
            { 
    	        lS->sidePlacing = FALSE;
	        if(seg.y!=0)
        	    hShift=floor((double)(LONG_TIC_LEN>MED_TIC_LEN ?
        	      LONG_TIC_LEN - MED_TIC_LEN : 1)*
        	      (double)seg.x/(double)( - seg.y));
	        else 
	            hShift = maxLabelWidth;
                for(label = lS->labels;label != NULL; label = label->next) 
		{
		    int center;
		    label->XWidth = XTextWidth(fs,label->label, label->length);
		    center = label->XWidth/2;
		    if (center > hShift) 
		        center=hShift;
		    label->wX = (lS->tics)[label->ticNumber].x2 - center + 1;
		    label->wY = (lS->tics)[label->ticNumber].y2 + 1 + digHeight;
		}
	    }
	}
	else /* seg.x <= 0 */
        {
            if ( - seg.x * digHeight < - seg.y * digWidth)
/* side placing 							*/
    	    {	
    	        lS->sidePlacing = TRUE;
                vShift= floor(((double)digWidth/2.+1.) * 
                  (double) seg.x /(double)( - seg.y) + (double)digHeight/2.);
                for(label = lS->labels;label != NULL; label = label->next) 
		{
		    label->XWidth = XTextWidth(fs,label->label, label->length);
		    label->wX = (lS->tics)[label->ticNumber].x2 + 1 + 1;
		    label->wY = (lS->tics)[label->ticNumber].y2 + vShift;
		}	 
            } else
/* down placing								*/    
            { 
    	        lS->sidePlacing = FALSE;
	        if(seg.y!=0)
        	    hShift=floor((double)(LONG_TIC_LEN>MED_TIC_LEN ?
        	      LONG_TIC_LEN - MED_TIC_LEN : 1)*
        	      (double)seg.x/(double)seg.y);
	        else 
	            hShift = maxLabelWidth;
                for(label = lS->labels;label != NULL; label = label->next) 
		{
		    int center;
		    label->XWidth = XTextWidth(fs,label->label, label->length);
		    center = label->XWidth/2;
		    if (center > hShift) 
		        center=hShift;
		    label->wX = (lS->tics)[label->ticNumber].x2 - center + 1;
		    label->wY = (lS->tics)[label->ticNumber].y2 - 1;
		}
	    }
	}
   }
}   	
    
/* check and remove if needed labels for labels list according to
   requared clipping 							*/
static void clipLabels(XFontStruct *fs, labeledSegment *lS)
{
    int digHeight;
    labelsToDraw2 *lT, **plT;
    
    if (fs == NULL) return;
    
    digHeight = maxDigXHeight(fs);
    plT = &(lS->labels);
    lT = *plT;
    if (lS->clip0 == CLIP_VERTICAL)
    {
        if (lS->axisImage.x1 < lS->axisImage.x2)
            while (lT!=NULL)
            {
                if (lT->wX < lS->axisImage.x1)
                {
/* drop the label							*/                 
    		    lS->ticPrms[lT->ticNumber].label = NULL;
    		    *plT = lT -> next;
    		    XtFree((char*)lT->label);
    		    XtFree((char*)lT);
    		}
    		else
    		    plT = &(lT->next);
    		lT = *plT;
    	    }
    	else if (lS->axisImage.x1 > lS->axisImage.x2)
            while (lT!=NULL)
            {
                if (lT->wX + lT->XWidth > lS->axisImage.x1)
                {
/* drop the label							*/                 
    		    lS->ticPrms[lT->ticNumber].label = NULL;
    		    *plT = lT -> next;
    		    XtFree((char*)lT->label);
    		    XtFree((char*)lT);
    		}
    		else
    		    plT = &(lT->next);
    		lT = *plT;
    	    }
    }
    else if (lS->clip0 == CLIP_HORIZONTAL)    	
    {
        if (lS->axisImage.y1 < lS->axisImage.y2)
            while (lT!=NULL)
            {
                if (lT->wY - digHeight < lS->axisImage.y1)
                {
/* drop the label							*/                 
    		    lS->ticPrms[lT->ticNumber].label = NULL;
    		    *plT = lT -> next;
    		    XtFree((char*)lT->label);
    		    XtFree((char*)lT);
    		}
    		else
    		    plT = &(lT->next);
    		lT = *plT;
    	    }
    	else if (lS->axisImage.y1 > lS->axisImage.y2)
            while (lT!=NULL)
            {
                if (lT->wY > lS->axisImage.y1)
                {
/* drop the label							*/                 
    		    lS->ticPrms[lT->ticNumber].label = NULL;
    		    *plT = lT -> next;
    		    XtFree((char*)lT->label);
    		    XtFree((char*)lT);
    		}
    		else
    		    plT = &(lT->next);
    		lT = *plT;
    	    }
    }
    plT = &(lS->labels);
    lT = *plT;
    if (lS->clip1 == CLIP_VERTICAL)
    {
        if (lS->axisImage.x2 < lS->axisImage.x1)
            while (lT!=NULL)
            {
                if (lT->wX < lS->axisImage.x2)
                {
/* drop the label							*/                 
    		    lS->ticPrms[lT->ticNumber].label = NULL;
    		    *plT = lT -> next;
    		    XtFree((char*)lT->label);
    		    XtFree((char*)lT);
    		}
    		else
    		    plT = &(lT->next);
    		lT = *plT;
    	    }
    	else if (lS->axisImage.x2 > lS->axisImage.x1)
            while (lT!=NULL)
            {
                if (lT->wX + lT->XWidth > lS->axisImage.x2)
                {
/* drop the label							*/                 
    		    lS->ticPrms[lT->ticNumber].label = NULL;
    		    *plT = lT -> next;
    		    XtFree((char*)lT->label);
    		    XtFree((char*)lT);
    		}
    		else
    		    plT = &(lT->next);
    		lT = *plT;
    	    }
    }
    else if (lS->clip1 == CLIP_HORIZONTAL)    	
    {
        if (lS->axisImage.y2 < lS->axisImage.y1)
            while (lT!=NULL)
            {
                if (lT->wY - digHeight < lS->axisImage.y2)
                {
/* drop the label							*/                 
    		    lS->ticPrms[lT->ticNumber].label = NULL;
    		    *plT = lT -> next;
    		    XtFree((char*)lT->label);
    		    XtFree((char*)lT);
    		}
    		else
    		    plT = &(lT->next);
    		lT = *plT;
    	    }
    	else if (lS->axisImage.y2 > lS->axisImage.y1)
            while (lT!=NULL)
            {
                if (lT->wY > lS->axisImage.y2)
                {
/* drop the label							*/                 
    		    lS->ticPrms[lT->ticNumber].label = NULL;
    		    *plT = lT -> next;
    		    XtFree((char*)lT->label);
    		    XtFree((char*)lT);
    		}
    		else
    		    plT = &(lT->next);
    		lT = *plT;
    	    }
    }    	    
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

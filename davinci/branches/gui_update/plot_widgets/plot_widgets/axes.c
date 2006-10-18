#include <X11/Intrinsic.h>
#include <math.h> 
#include <float.h>
#include <stdio.h>
#include "2DGeom.h"
#include "fltRange.h"
#include "uniLab.h"
#include "axes.h"

static void drawAxis(Display *d, Drawable dr, GC gc,
  labeledSegment *ls, XSegment *arrows, int nArrows);
static int makeArrows (float minLimit, float maxLimit, float visible0,
  float visible1, XSegment *axisImage, XSegment **arrows);

/*
** axis *createAxis(XFontStruct *fs, float minLimit, float maxLimit, 
**    float minVisible, float maxVisible, XSegment *seg, Boolean logSaling, 
**   clipType clip0, clipType clip1, Boolean reverse, Boolean mapped,
**   char *name, XRectangle *nameClipRectangle)
**
** minLimit	- min limit in object space (for dragging locking and
**		  arrow calcultion)
** maxLimit	- max limit in object space (for dragging locking and
**		  arrow calcultion)
** minVisible	- minimal value in the object space which is visible
** maxVisible	- maxmal value in the object space which is visible
** *seg		- image of the axis in the window. The lables and tics will be
**		  generated in the semi-plane which is
**		  on the right side while going from
**		  the first point to the second. Additional ordering
**		  information is controlled by <reverse> parameter.
** logsSaling	- use log scaling if TRUE, linear otherwise
** clip0	- labels clipping strategy around the first point of the <*seg>
**                (to avoid labels overlapping with other elements of the
**		  picture, i.e. with other axis labeles)
** clip1	- labels clipping strategy around the second point of the <*seg>,
**                as above
** reverse	- if FALSE the first point of the <*seg> corresponds to
**		  the <minVisible>, if TRUE the first point of the <*seg> corresponds to
**		  the <maxVisible>
** mapped	- if TRUE - will create drawing image and draw it during
**		  subsequent appropriate requests, if FALSE - will just
**		  manage parameters.
**
** name		- pointer to the axes name (ASCII)
**
** nameClipRectangle - pointer to rectangle to plce the name into (i.e. window)
**
**	Labels and tics will be placed in the semi-plane which is
**	on the right side of the <*seg> while going from the first
**	<*seg> point to the second
*/		  
axis *createAxis(XFontStruct *fs, float minLimit, float maxLimit, 
    float minVisible, float maxVisible, XSegment *seg, Boolean logScale, 
    clipType clip0, clipType clip1, Boolean reverse, Boolean mapped,
    char *name, XRectangle *nameClipRectangle )
{
    axis *a;
    a = (axis *)XtMalloc(sizeof(axis));
    a->fs = fs;
    a->minLimit = minLimit;
    a->maxLimit = maxLimit;
    a->mapped = mapped;
    a->axisImage = *seg;
    a->curSegment = NULL;
    a->newSegment = NULL;
    a->newArrows = NULL;
    a->newNArrows = 0;
    a->curArrows = NULL; 
    a->curNArrows = 0;
    a->action = A_NONE;
    a->logScale = logScale;
    a->clip0 = clip0;
    a->clip1 = clip1;
    if (reverse)
    {
        a->visible0 = &(a->maxVisible);
        a->visible1 = &(a->minVisible);
    }
    else
    {
        a->visible0 = &(a->minVisible);
        a->visible1 = &(a->maxVisible);
    }        
    setFRangeLimits(&(a->minLimit),&(a->maxLimit));
/* adjust visible range to a new limits	*/
    setAxisVisibleRange(a, minVisible, maxVisible);    
    if (logScale)
       a->logScale = setFRangeLogScale (&(a->minVisible) , &(a->maxVisible),
  	      a->minLimit, a->maxLimit);
    else 
       a->logScale = FALSE;
    a->changed = TRUE;
    if (name != NULL)
    {
        a->name = XtMalloc(sizeof(char) * (strlen(name) + 1));
        strcpy(a->name, name);
    }
    else
        a->name = NULL;
    if (nameClipRectangle != NULL)     
    {
        a->nameClipRectangle = (XRectangle*)XtMalloc(sizeof(XRectangle));
        *(a->nameClipRectangle) = *nameClipRectangle;
    }
    else
        a->nameClipRectangle = NULL;        
    setAxisDefaultNameClip(a);
    return(a);
}        

/*
** axis *createDummyAxis(void)
**
** just create the structure and fill it with the parameters which
** could be processed by all functions as "making sence" (i.e without crash)
** Useful for "step by step" axis management.
*/		  
axis *createDummyAxis(void)
{
    axis *a;
    a = (axis *)XtMalloc(sizeof(axis));
    a->minLimit = 0;
    a->maxLimit = 0;
    a->mapped = FALSE;
    a->axisImage.x1 = a->axisImage.y1 = a->axisImage.x2 = a->axisImage.y2 = 0;
    a->fs = NULL;
    a->curSegment = NULL;
    a->newSegment = NULL;
    a->newArrows = NULL;
    a->newNArrows = 0;
    a->curArrows = NULL; 
    a->curNArrows = 0;
    a->action = A_NONE;
    a->logScale = FALSE;
    a->clip0 = CLIP_NONE;
    a->clip1 = CLIP_NONE;
    setFRangeLimits(&(a->minLimit),&(a->maxLimit));
/* adjust visible range to a new limits	*/
    setAxisVisibleRange(a, 0., 0.);    
    a->changed = TRUE;
    a->name = NULL;
    a->nameClipRectangle = NULL;
    setAxisDefaultNameClip( a);
    return(a);
}        


void destroyAxis(axis *a)
{
    if (a==NULL) return;
    destroyLabeledSegment(a->newSegment);
    destroyLabeledSegment(a->curSegment);
    if (a->newArrows!=NULL) XtFree((char*)a->newArrows);
    if (a->curArrows!=NULL) XtFree((char*)a->curArrows);
    if (a->name!=NULL) XtFree((char*)a->name);
    if (a->nameClipRectangle!=NULL) XtFree((char*)a->nameClipRectangle);
    XtFree((char*)a);
}    

void setAxisLabelsFont(axis *a, XFontStruct *fs)
{
    if (a==NULL) return;
    a->fs = fs;
    a->changed = TRUE;
}    
       
void setAxisName (axis *a, char *name)
{
    if (a==NULL) return;
    if (a->name != NULL) XtFree((char*)a->name);
    if (name != NULL)
    {
        a->name = XtMalloc(sizeof(char) * (strlen(name) + 1));
        strcpy(a->name, name);
    }
    else
        a->name = NULL;
    a->changed = TRUE;
}    

void setAxisNameClipRectangle (axis *a, XRectangle *nameClipRectangle)
{
    if (a==NULL) return;
    if (a->nameClipRectangle!=NULL) XtFree((char*)a->nameClipRectangle);
    if (nameClipRectangle != NULL)     
    {
        a->nameClipRectangle = (XRectangle*)XtMalloc(sizeof(XRectangle));
        *(a->nameClipRectangle) = *nameClipRectangle;
    }
    else
        a->nameClipRectangle = NULL;
    a->changed = TRUE;
}    

void setAxisDefaultNameClip(axis *a)
{
    if (a==NULL) return;
    if (a->nameClipRectangle != NULL)  
    {
        a->clipX0 = a->nameClipRectangle->x;
        a->clipX1 = a->nameClipRectangle->x + a->nameClipRectangle->width;   
    }
    else
    {
        a->clipX0 = 0;
        a->clipX1 = 65535;
    }
}            

void setAxisLimitRange(axis *a, float min, float max)
{
    if (a==NULL) return;
    a->minLimit = min;
    a->maxLimit = max;
    setFRangeLimits(&(a->minLimit),&(a->maxLimit));
/* adjust visible range to a new limits	*/
    setAxisVisibleRange(a, min, max); 
    a->action = A_NONE;
    a->changed = TRUE;
} 

/* 
** Expand limit range so that it contanes the original range and 
** the requested panges both
** ("resize at max" without changing visible range)
*/
void expandAxisLimitRange(axis *a, float min, float max)
{
    if (a==NULL) return;
    if (min < a->minLimit)
        a->minLimit = min;
    if (max > a->maxLimit)    
        a->maxLimit = max;
    setFRangeLimits(&(a->minLimit),&(a->maxLimit));
/* adjust visible range to a new limits	*/
    setAxisVisibleRange(a, min, max); 
    a->action = A_NONE;
    a->changed = TRUE;
} 

/* 
** Set the limit range, and
** pull each of visible range ends to the corresponding limit 
** if it is at the limit before expansion.
** (visible range "resize at max")
*/
void setAxisLimitVisible (axis *a, float min, float max)
{
    int changeMinVisLimit = 0, changeMaxVisLimit = 0;

    if (a==NULL) return;
    if (a->minVisible == a->minLimit) {
        a->minVisible = min;
        changeMinVisLimit = 1;
    }
    a->minLimit = min;
    if (a->maxVisible == a->maxLimit) { 
        a->maxVisible = max;
        changeMaxVisLimit = 1;
    }
    a->maxLimit = max;
    setFRangeLimits(&(a->minLimit),&(a->maxLimit));
/* adjust visible range to a new limits	*/
    if (changeMinVisLimit)
	setAxisVisibleRange(a, min, a->maxVisible); 
    if (changeMaxVisLimit)
	setAxisVisibleRange(a, a->minVisible, max); 
    a->action = A_NONE;
    a->changed = TRUE;
} 

/* 
** Expand limit range so that it contanes the original range and 
** the requested panges both. 
** Pull each of visible range ends to the corresponding limit 
** if it is at the limit before expansion.
** ("resize at max" with visible range "resize at max")
*/
void expandAxisLimitVisible (axis *a, float min, float max)
{
    int changeMinVisLimit = 0, changeMaxVisLimit = 0;

    if (a==NULL) return;
    if (min < a->minLimit)
    {
        if (a->minVisible == a->minLimit) {
            a->minVisible = min;
            changeMinVisLimit = 1;
        }
        a->minLimit = min;
    }        
    if (max > a->maxLimit)  
    {  
        if (a->maxVisible == a->maxLimit) {
            a->maxVisible = max;
            changeMaxVisLimit = 1;
        }
        a->maxLimit = max;
    }    
    setFRangeLimits(&(a->minLimit),&(a->maxLimit));
/* adjust visible range to a new limits	*/
    if (changeMinVisLimit)
	setAxisVisibleRange(a, min, a->maxVisible); 
    if (changeMaxVisLimit)
	setAxisVisibleRange(a, a->minVisible, max); 
    a->action = A_NONE;
    a->changed = TRUE;
}

void setAxisMapped(axis *a)
{
    if (a==NULL) return;
    a->mapped = TRUE;
    a->changed = TRUE;
}
   
void setAxisUnMapped(axis *a)
{
    if (a==NULL) return;
    a->mapped = FALSE;
    a->action = A_NONE;
    a->changed = TRUE;
}
    
        
         
void setAxisVisibleRange(axis *a, float minVisible, float maxVisible)
{
    if (a==NULL) return;
    a->minVisible = minVisible;
    a->maxVisible = maxVisible;
    adjustFRange(&(a->minVisible),&(a->maxVisible),a->minLimit, a->maxLimit, 
      a->logScale);
    a->action = A_NONE;   
    a->changed = TRUE;
}

void setAxisImageSegment(axis *a, XSegment *seg)
{
    if (a==NULL) return;
    a->axisImage = *seg;
    a->visible0 = &(a->minVisible);
    a->visible1 = &(a->maxVisible);
    a->action = A_NONE;   
    a->changed = TRUE;
}

void setAxisImageSegmentReverse(axis *a, XSegment *seg)
{
    if (a==NULL) return;
    a->axisImage = *seg;
    a->visible0 = &(a->maxVisible);
    a->visible1 = &(a->minVisible);
    a->action = A_NONE;   
    a->changed = TRUE;
}

void setAxisClipping(axis *a, clipType clip0, clipType clip1)
{
    if (a==NULL) return;
    a->clip0 = clip0;
    a->clip1 = clip1;
    a->changed = TRUE;
}

void setAxisLinearScale(axis *a)
{
    if (a==NULL) return;
    a->logScale = FALSE;
    adjustFRange(&(a->minVisible),&(a->maxVisible),a->minLimit, a->maxLimit, 
      a->logScale);
    a->action = A_NONE;   
    a->changed = TRUE;
}

Boolean setAxisLogScale(axis *a)
{
    if (a==NULL) return(FALSE);
    a->logScale = setFRangeLogScale (&(a->minVisible),&(a->maxVisible),
      a->minLimit, a->maxLimit);
    a->action = A_NONE;   
    a->changed = TRUE;
    return (a->logScale);
}

/* 
** as above, but is trying to set up <minVisible> not less than <verge>
*/
Boolean setAxisLogScaleWVerge(axis *a, float verge)
{
    if (a==NULL) return(FALSE);
/* "minVisible lock"							*/
   if (a->minVisible < verge)
       a->minVisible = verge; 
/* might be overriden by the next call if the range goes
   wrong because of this. The resulting <a->minVisible> is hardly
   to predict in the last case, but it will be "reasonable"		*/      
   setAxisVisibleRange(a, a->minVisible, a->maxVisible);       
   return(setAxisLogScale(a));       
}

void getAxisLimits(axis *a, float *min, float *max)
{
    if (a==NULL) return;
    *min = a->minLimit;
    *max = a->maxLimit;
}

void getAxisVisibleRange(axis *a, float *minVisible, float *maxVisible)
{
    if (a==NULL) return;
    *minVisible = a->minVisible;
    *maxVisible = a->maxVisible;
}

/*
**  make zoom in (factor < 1) or zoom out (factor > 1) using zoom factor
*/
void zoomAxis(axis *a, float factor)
{
    float minVisible;
    float maxVisible;
    if (a==NULL) return;
    minVisible = a->minVisible;
    maxVisible = a->maxVisible;
    if (factor < 0) factor = -factor;
    if (a->logScale)
    {
/* lock for min don't go too close to zero (usually make no sence) if
   if was not too close to zero						*/
        minVisible = log10(minVisible);
        maxVisible = log10(maxVisible);
        a->minVisible = (minVisible + maxVisible)/2.0 -
          (maxVisible - minVisible) * factor /2.0;
        a->maxVisible = (minVisible + maxVisible)/2.0 +
          (maxVisible - minVisible) * factor /2.0;
        if (a->minVisible < FLT_MIN_10_EXP)
            a->minVisible = FLT_MIN_10_EXP;
        if (a->maxVisible > FLT_MAX_10_EXP)
            a->maxVisible = FLT_MAX_10_EXP;
        a->minVisible = pow(10.,a->minVisible);
        a->maxVisible = pow(10.,a->maxVisible);
    }
    else
    {
        a->minVisible = (minVisible + maxVisible)/2.0 -
          (maxVisible - minVisible) * factor /2.0;
        a->maxVisible = (minVisible + maxVisible)/2.0 +
          (maxVisible - minVisible) * factor /2.0;
    }          
    setAxisVisibleRange(a, a->minVisible, a->maxVisible);
    a->changed = TRUE;
}

/*
**  make zoom in (factor < 1) or zoom out (factor > 1) using zoom factor,
**  and trying to set up <minVisible> not less than <verge> in the
**  case   conditions below are satisfied:
**	a->logScale  == TRUE
**      a->minVisible >= verge  (i.e. minVisible was big enought)
**  The correction could appear only if  factor > 1 	(i.e. zoom out)
**  The reason is that in log case <minVisible> might go to very small
**  value (close to zero) wile doing regular zoom out that makes no
**  sense and looks fanny. 
**  This function may eliminate this problem
**
*/
void zoomAxisWVerge(axis *a, float factor, float verge)
{
    if (a==NULL) return;
    if (a->logScale)
    {
        if (a->minVisible >= verge)
        {
            zoomAxis(a,factor);
            if (a->minVisible < verge)
                setAxisVisibleRange(a, verge, a->maxVisible);
            return;
        }
    }
    zoomAxis(a,factor);
}              

void resetAxisView(axis *a)
{
    setAxisLinearScale(a);
    setAxisVisibleRange(a, a->minLimit, a->maxLimit);
}    

Boolean markAxisState(axis *a, XEvent *event)
{
    vector v1, v2, v;
    if (a==NULL) return(FALSE);
    if (event == NULL) return (FALSE);
    if (a->curSegment == NULL) return(FALSE);
    a->oldCursor=makeVector(event->xbutton.x,event->xbutton.y);
    a->markedMinVisible = a->minVisible;
    a->markedMaxVisible = a->maxVisible;
    v1 = makeVector(a->curSegment->axisImage.x1,a->curSegment->axisImage.y1);
    v2 = makeVector(a->curSegment->axisImage.x2,a->curSegment->axisImage.y2);
    v = subVectors(v2, v1);
    if ( dpointInBand(doubleVector(a->oldCursor),doubleVector(v1),
      dsetLength(dortogonal(doubleVector(v)),(double) -2 * LONG_TIC_LEN))
        && vectorLength(v) > 1)
    {
        double l=pointLocation(a->oldCursor,v1,v);
        if (l >= 0 && l <= 1)		      /* + or - here ^ ?  */
        {
	    a->action= A_SCALING;
            if (l > 1./2.)
                a->markedVector=v1;
            else 
                a->markedVector=v2;
            if (scalarProduct(subVectors(a->oldCursor,a->markedVector),v) == 0)
            {
                a ->action = A_NONE;
                return(FALSE);
            }
	    return(TRUE);                
        }
        return(FALSE);
    }
    if ( dpointInBand(doubleVector(a->oldCursor),doubleVector(v1),
      dsetLength(dortogonal(doubleVector(v)),(double) 2 * LONG_TIC_LEN))
        && vectorLength(v) > 1)
    {
        if (pointInBand(a->oldCursor,v1,v))
        {
	    a->action=A_PANNING;
	    return(TRUE);
	}                    
    }
    return(FALSE);
}     	    
            
        
Boolean dragAxis(axis *a, XEvent *event)
{
    vector v1, v2, v;
    vector cursor;
    if (a==NULL) return(FALSE);
    if (event == NULL) return (FALSE);
    if (a->curSegment == NULL) return(FALSE);
    if (a->action == A_NONE) return(FALSE);
    v1 = makeVector(a->curSegment->axisImage.x1,a->curSegment->axisImage.y1);
    v2 = makeVector(a->curSegment->axisImage.x2,a->curSegment->axisImage.y2);
    v = subVectors(v2, v1);
    cursor = makeVector(event->xbutton.x,event->xbutton.y);
    if (a->action == A_SCALING)
    {
        int d1 = scalarProduct(subVectors(a->oldCursor,a->markedVector),v);
        int d2 = scalarProduct(subVectors(cursor,a->markedVector),v);
        float d;
        a->minVisible = a->markedMinVisible;
        a->maxVisible = a->markedMaxVisible;
        if (eqVectors(a->markedVector, v2))
        {
            d1 = -d1;
            d2 = -d2;
        }
	if (d2 <= 0)
	    d = -1.0;
	else
	    d =  (float)d1 / (float)d2;    
        if (eqVectors(a->markedVector,v1) == (a->visible0 == &(a->minVisible)))
        {
            if ( d >=0 ) 
	        pullFRangeEnd (&(a->minVisible), &(a->maxVisible), d, 
	          a->minLimit, a->maxLimit, a->logScale);
	    else
	    {
	        a->maxVisible = a->maxLimit;
	        adjustFRange(&(a->minVisible), &(a->maxVisible), 
	          a->minLimit, a->maxLimit, a->logScale);
	    } 
	}         
 	else
 	{
            if ( d >=0 ) 
 	        pullFRangeStart (&(a->minVisible), &(a->maxVisible), d, 
 	          a->minLimit, a->maxLimit, a->logScale);
	    else
	    {
	        a->minVisible = a->minLimit;
	        adjustFRange(&(a->minVisible), &(a->maxVisible), 
	          a->minLimit, a->maxLimit, a->logScale);
	    } 
	}   
	a->changed = TRUE;      
	return(TRUE);	      
    }	      	      
    if (a->action == A_PANNING)
    {	 
        int d1 = scalarProduct(a->oldCursor ,v);
        int d2 = scalarProduct(cursor, v);
        float d;
        a->minVisible = a->markedMinVisible;
        a->maxVisible = a->markedMaxVisible;
	if (vectorLength(v) != 0)
	    d = (float)(d2 - d1) / (float)vectorLength(v) /
              (float)vectorLength(v); 
	else 
	    d = 0.0; 
	if( a->visible0 < a->visible1)         
            moveFRange(&(a->minVisible), &(a->maxVisible), -d, a->minLimit, 
	      a->maxLimit, a->logScale);
	else
	    moveFRange(&(a->minVisible), &(a->maxVisible), d, a->minLimit, 
	      a->maxLimit, a->logScale);
	a->changed = TRUE;      
	return(TRUE);	      
    }	      	      
    return(FALSE);
}
         
Boolean stopAxisDragging(axis *a, XEvent *event)
{
    if( a == NULL) return (FALSE);
    if (event == NULL) return (FALSE);
    if (a->action == A_NONE) return(FALSE);
    a->action = A_NONE;
    return(TRUE);
}    

void drawNewAxis(Display *d, Drawable dr, GC gc, axis *a)
{
    if (a==NULL) return;
    drawAxis(d, dr, gc, a->newSegment, a->newArrows, a->newNArrows);
}    

void drawCurAxis(Display *d, Drawable dr, GC gc, axis *a)
{
    if (a==NULL) return;
    drawAxis(d, dr, gc, a->curSegment, a->curArrows, a->curNArrows);
}    

static void drawAxis(Display *d, Drawable dr, GC gc,
  labeledSegment *ls, XSegment *arrows, int nArrows)
{
    if (ls == NULL) return;
    drawLabeledSegment(d, dr, gc, ls);
    if (nArrows != 0 && arrows != NULL)
        XDrawSegments(d,dr,gc,arrows,nArrows);
}        

void saveAxisImage(axis *a)
{
    if (a==NULL) return;
    destroyLabeledSegment(a->curSegment);
    a->curSegment = a->newSegment;
    a->newSegment = NULL;
    if (a->curArrows != NULL)
        XtFree((char*)a->curArrows);
    a->curArrows = a->newArrows;
    a->newArrows = NULL;
    a->curNArrows = a->newNArrows;
    a->newNArrows = 0;
    a->changed = FALSE;
}     

void makeNewAxisImage(axis *a)
{
    if (a==NULL) return;
    destroyLabeledSegment(a->newSegment);
    a->newSegment = NULL;
    if (a->newArrows != NULL)
    {
        XtFree((char*)a->newArrows);
        a->newArrows = NULL;
        a->newNArrows = 0;
    } 
    if (! (a->mapped) ) return;    
    a->newSegment = makeLabeledSegment(a->fs, &(a->axisImage), 
      *(a->visible0), *(a->visible1), a->logScale, FALSE, a->clip0, a->clip1,
      a->name, a->clipX0, a->clipX1); 
    a->newNArrows = makeArrows (a->minLimit, a->maxLimit, *(a->visible0),
      *(a->visible1), &(a->axisImage), &(a->newArrows));
} 
   
static int makeArrows (float minLimit, float maxLimit, float visible0,
  float visible1, XSegment *axisImage, XSegment **arrows)
{
    vector v1, v2, vseg;
    XSegment *seg;
    int nArrows;
    *arrows = (XSegment *)XtMalloc(sizeof(XSegment) *4);
    seg = *arrows;
    nArrows =0;
    v1 = makeVector(axisImage->x1,axisImage->y1);
    v2 = makeVector(axisImage->x2,axisImage->y2);
    vseg = subVectors (v2,v1);
    if (vectorLength(vseg) > 2 * ARROW_LENGTH )
    {
    	vector a1,a2;
    	vector p1,p2;
    	a1.x = (int) ( cos(ARROW_ANGLE) * vseg.x + sin(ARROW_ANGLE) * vseg.y);
    	a1.y = (int) ( -sin(ARROW_ANGLE) * vseg.x + cos(ARROW_ANGLE) * vseg.y);
    	a2.x = (int) ( cos(ARROW_ANGLE) * vseg.x  - sin(ARROW_ANGLE) * vseg.y);
    	a2.y = (int) ( sin(ARROW_ANGLE) * vseg.x + cos(ARROW_ANGLE) * vseg.y);
    	a1 = setLength(a1,ARROW_LENGTH);
    	a2 = setLength(a2,ARROW_LENGTH);
    	p1 = addVectors(v1,a1);
    	p2 = addVectors(v1,a2);
        if (visible0 != minLimit && visible0 != maxLimit)
        {
            seg->x1=v1.x; seg->x2=p1.x; seg->y1=v1.y; seg->y2=p1.y;
            seg++; nArrows++;
            seg->x1=v1.x; seg->x2=p2.x; seg->y1=v1.y; seg->y2=p2.y; 
            seg++; nArrows++;
        }
	p1 = subVectors(v2,a1);
    	p2 = subVectors(v2,a2);
        if (visible1 != minLimit && visible1 != maxLimit)
        {
            seg->x1=v2.x; seg->x2=p1.x; seg->y1=v2.y; seg->y2=p1.y; 
            seg++; nArrows++;
            seg->x1=v2.x; seg->x2=p2.x; seg->y1=v2.y; seg->y2=p2.y; 
            seg++; nArrows++;
        }
    }
    return (nArrows);
}

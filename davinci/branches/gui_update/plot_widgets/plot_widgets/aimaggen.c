/*******************************************************************************
*									       *
* aimaggen.c -- 2D adaptive histogram bit image generator		       *
* 		for 2DHist widget 					       *
*									       *
* Copyright (c) 1993 Universities Research Association, Inc.		       *
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
*      ,1993								       *
*									       *
* Written by Konstantine Iourcha					       *
*									       *
*******************************************************************************/
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h> 
#include <X11/Xmd.h>
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#if XmVersion == 1002
#include <Xm/PrimitiveP.h>
#endif
#include <Xm/DrawingA.h>
#include <X11/Xutil.h>
#include <math.h>
#include <stdlib.h>
/*?*/
#include <stdio.h>
#include "2DHist.h"
#include "2DGeom.h"
#include "2DHistDefs.h"
#include "uniLab.h"
#include "labels.h"		
#include "2DHistP.h"
#include "imaggen.h"
#include "aimaggen.h"

/*
** Next structure describe one column of image ( bitmap)
** it used to avoid bit & byte order problems too
** We will use an array of such structures,
** containing one instanse for each column.
*/
typedef struct _columnDescr 
{
    CARD32 pattn;	/* bitmask for current column 			*/    		
    int x;		/* index of CARD32 bitmap unit which contanes 
    	     		   current column in a row of the bitmap	*/ 
    CARD32 * hor;	/* current position of floating horizont 	*/
}   columnDescr;

typedef struct _lineStepDescr 
{
    int dx;		/* X shift to next line pixsel	(0 or +/- 1)	*/
    int dy;		/* Y shift to next line pixsel (actual dy), 
    			   multiplied by bitmap width (0 or +/- <width>)*/
    int dl;		/* Manhattan distance to next pixel, i.e
    			   abs(dx) + abs(<actual>dy), could be 0,1 or 2 */
} lineStepDescr;

typedef struct _fIndex
{
    lineStepDescr firstStep;
    lineStepDescr *regularStep;
} fIndex;

typedef struct _lineDescr
{
    int	f2Offset;	/* multiplied by 2 offset to make rounding by 
    			   floor operation, calculating initial f for index
    			   access					*/
    int a, b;		/* the source function coefficients, possible
    			   multiplayed by -1. To caculate intial value
    			   of f as a * x - b * y, which will be in
    			   consitents with fIdx				*/
    fIndex *fIdx;	/* index array, fIdx is setted up to the first 
    			   element					*/
    fIndex *f_0Idx;   	/* pointer to the same array, to an element		   
    			   corresponding  2 * f==0			*/
    lineStepDescr *lineStep;
} lineDescr;

#define _X 0
#define _Y 1
#define _START 0
#define _END 1

/* structure to describe sequence of binary tree walk. Fields of structure
   define displacement from the top of node stack for left and right
   subtree nodes. (Possible values are 0 and 1 and should be different.
   After putting left and right nodes on the stack, the algorithm 
   walk through subtree on the top. (stack is groing to positive indexies).
   So when left == 0 and right == 1, the algorithm will proceed with
   right to left walk, and left to right overvise		
*/
typedef struct _walkBranch {
    int left;
    int right;
} walkBranch;
/* 
   odd level nodes describe X splitting points and even level nodes - 
   Y splitting points. The walk sequences are different for X and Y
   splittings, so two element array is used to describe walk sequence.
   (To provide hidden line removing, the algorithm counts bins in such
   a sequence that all subsequent bins are situated futher from
   the viewer then current, either along X axis or along Y axis. 
*/ 

/* cell (range) for current bin, bin can be smaller due to
   interbin margins					  	*/

typedef struct _fcell {
    float c[2][2];
} fcell;    

typedef struct _walkStep {
    aHistNode *node;		/* node index		  	*/
    int walkIdx;		/* X split or Y split (0 or 1) on
    				   current level		*/
    fcell cellSize;
} walkStep;

/* in this file all mBaseNumber's have base == MAXPIC*MAXPIC to make
   processing faster ( but with slightly less accurate picture).
   So relevant structures are redefined to eliminate unnesessary
   feilds.
*/

typedef struct _mBaseNumber {
    long int highDig;
    long int firstDig;
} mBaseNumber;

typedef struct _binPrmA {
    mBaseNumber x;
    mBaseNumber y;
    mBaseNumber fx;
    mBaseNumber fy;
} binPrmA;

static columnDescr* makeColumnDescr(XImage *image);
static lineDescr *createLineDescr(void);
static void destroyLineDescr(lineDescr **lD);
static lineDescr *makeLineDescr(int a,int b, int width);
void makeAHistImage(Hist2DWidget w);
static void createImageData (XImage *image, Hist2DWidget w);
static void drawBin(walkStep *curCell);
static void aRescaleZ(float fData, discreteData *dData);
/* 
   variables to provide interface between makeAHistImage() drawBin() and
   aRescaleZ() functions. (Because drawBin() and  aRescaleZ() are called for
   each bin, passing all the paramerters each time could slows down 
   the processing. 
   This variables are setted up by makeAHistImage() and are used by
   functions calld from makeAHistImage()
*/

/*     initial visible range in source coordintates           	*/
static XImage *image;
static matrix2 map;
static float visible[2][2];
static float xScaleFactor;
static float yScaleFactor;
static matrix2 map2;
static int width;		/* width of the bitmap (in 32 bit units)   */
static columnDescr *columns;	/* array of elements corresponding to each
    				   column of the bitmap. Elemetns contain
    				   floating horizont position, offset in
    				   the row (in bitmap units) and bit mask
    				   (pattern) for given column              */
static int dfx_dx;
static int dfx_dy;
static int dfy_dx;
static int dfy_dy;
static int xBase;
static int yBase;
static int xLineBase;
static int yLineBase;
static lineDescr *xLine;
static lineDescr *yLine;
static Boolean zLogScaling;
static double zLinearScaleFactor;
static double zLogScaleFactor;
static double zLogStart;
static double zVisibleStart;
static double zVisibleEnd;
static int zFactor;
static int mapX;
static int mapY;
static int mapXStart;
static int mapYStart;
static int mapXEnd;
static int mapYEnd;
static int mapXStartValue;
static int mapYStartValue;
   
static lineDescr *createLineDescr(void)
{
    lineDescr *tmp;
    MYMALLOC(lineDescr,tmp,1);
    MYMALLOC(fIndex,(tmp->fIdx),2 * MAXPIC);
    MYMALLOC(lineStepDescr,(tmp->lineStep),2 * MAXPIC);
    tmp->f_0Idx = tmp->fIdx + MAXPIC;
    return (tmp);
}

static void destroyLineDescr(lineDescr **lD)
{
    if ((*lD) != NULL)
    {
	MYFREE((*lD)->fIdx);
	MYFREE((*lD)->lineStep);
    }
    MYFREE(*lD);
    *lD = NULL;
}

static lineDescr *makeLineDescr(int a,int b, int width)
/* for line a*x - b*y == f0, 
   f0 from the set {f| f==a*x-b*y, -1<x<=1, -1<y<=1}
*/   
{
    lineDescr *lD;
    lineStepDescr *sTmp;
    fIndex *fIdxTmp;
    int i;
    int maxk;
    int A, B, dLow, dHigh;
    int f, oldF;
    lineStepDescr xStep;
    lineStepDescr yStep;
    lineStepDescr xyStep;
    lineStepDescr minStep;
    lineStepDescr lowStep;
    lineStepDescr highStep;
    lineStepDescr noStep;

    xStep.dx  = 1; xStep.dy  = 0; xStep.dl  = 1;
    yStep.dx  = 0; yStep.dy  = -width; yStep.dl  = 1;
    xyStep.dx = 1; xyStep.dy  = -width; xyStep.dl  = 2;
    noStep.dx = 0; noStep.dy = 0; noStep.dl = 1; /* dl > 0 to avoid dead loops */
    
    lD = createLineDescr();

    for (i=0,fIdxTmp = lD->fIdx;i < 2 * MAXPIC; i++, fIdxTmp++)
         fIdxTmp->regularStep = NULL;
/* assume a >= 0 				*/
    A = a;
    B = - b;
/* change X direction  				*/   
    if (b < 0)
    {
        A = - a;
    }
    else
    {
        xStep.dx  = - xStep.dx;
        xyStep.dx = - xyStep.dx; 
    }
/* A and B have the different  signs now	*/
/* setup line increments			*/    
    if ( abs(a) > abs(b) ) 
        minStep = yStep; 
    else if (b != 0)
    {
        B = A; 
        A = - b;
        minStep = xStep;
    }
    else  /*  a==0 && b == 0 */ 
        minStep = noStep; 
    
/* the next is needed to implement correct rounding,
   see math considerations				*/
    if (b < 0 || abs(b) < abs(a) )
    {
        A = -A;
        B = -B;
    }
    else
    {
        a = -a;
        b = -b;
    }
    
    maxk = abs (A);
    
    lD->f2Offset = maxk;
    lD->a = a;
    lD->b = b;
/* next means lD->f_0Idx = fIdx  - floor(-maxk/2) 	*/   
    lD->f_0Idx = lD->fIdx + maxk/2 + maxk%2;
/* f =  A * x + B * y, (or f =  B * x + A * y),
   0 <= f < |A|, f_0 = 0, dx = dy = -1 .	 	*/
    f = 0;
    for (i=0, fIdxTmp = lD->f_0Idx, sTmp = lD->lineStep;
      i < 2 * MAXPIC; i++, sTmp++)
    {
        oldF = f;
        f -= B;
        if (f < 0 || f >=maxk)
/* first condition for the case A <=0, B >=0; second for
   A >=0 , B <=0 , maxk == abs(A)			*/      
        {
            f -= A;
            *sTmp = xyStep;
        }
        else 
            *sTmp = minStep;
        if ((lD->f_0Idx + oldF)-> regularStep == NULL)
        {     
            (lD->f_0Idx + oldF)-> regularStep = sTmp + 1;
            (lD->f_0Idx + oldF)-> firstStep = *sTmp;
        }
    }


/* in the case A == B == 0 the structure will be initialised
   incorrectly, it will not be used in this case, actually
   no lines defined in this case			*/
   
   for (i=0, f=0, fIdxTmp = lD->f_0Idx; i < maxk; i++)
       if ((lD->f_0Idx + i)-> regularStep == NULL)
           *(lD->f_0Idx + i) = *(lD->f_0Idx + f); 
       else
          f = i;
          
   if ( A < 0 || B > 0)
   {
       dLow = A;
       dHigh = B;
       lowStep = xyStep;
       highStep = minStep;
       lowStep.dx -= minStep.dx;
       lowStep.dy -= minStep.dy;
       lowStep.dl -= minStep.dl;
   } else
   {
       dLow = B;
       dHigh = A;
       lowStep = minStep;
       highStep = xyStep;
       highStep.dx -= minStep.dx;
       highStep.dy -= minStep.dy;
       highStep.dl -= minStep.dl;
    }    
          
   for ( fIdxTmp = lD->fIdx ; fIdxTmp < lD->f_0Idx; fIdxTmp++)
       if ((fIdxTmp - dLow)-> regularStep != NULL) 
       {
           fIdxTmp-> regularStep = 
             (fIdxTmp - dLow)-> regularStep - 1;
           fIdxTmp-> firstStep = lowStep;  
       }
   for (fIdxTmp = lD->f_0Idx + maxk; 
     fIdxTmp < lD->fIdx + 2*MAXPIC; fIdxTmp++)
       if ((fIdxTmp - dHigh)-> regularStep != NULL) 
       {
           fIdxTmp-> regularStep = 
             (fIdxTmp - dHigh)-> regularStep - 1;
           fIdxTmp-> firstStep = highStep;  
       }
   return (lD);
}            
static void createImageData (XImage *image, Hist2DWidget w)
/* we use the widget to get geometry information only */
{
    if (image == NULL) return;
    if ( (image->width != w->core.width) || (image->height != w -> core.height) )
    {
        MYFREE( image->data);
        image->data = NULL;
	image->width = w->core.width;
	image->height = w -> core.height;
        image->bytes_per_line=((image->width+31)/32) * 4;
    }
    else if (image->data != NULL)
    {
        int i;
        int n=image->bytes_per_line/4 * image->height;
        int *ptr = (int*)image->data;
        for(i=0;i<n; i++)
            *ptr++=0;
    }
    if (image->data == NULL)
/*       MYCALLOC(CARD32,image->data,((image->width+31)/32) * image->height); */
        image->data = (char*) XtCalloc(((image->width+31)/32) * image->height, sizeof(CARD32));
}        

static columnDescr * makeColumnDescr(XImage *image)
{
    columnDescr *cd, *cTmp;
    CARD32 *tableStart, *tmp;
    char *tst;
    Boolean lsbf;
    int i,j,k;
/* next tables describes patterns for different byte and bit  orders */
    static CARD32 BBTable [4] [32]=
    {
    	{ 
    	    0x00000001, 0x00000002, 0x00000004, 0x00000008,
    	    0x00000010, 0x00000020, 0x00000040, 0x00000080, 
    	    0x00000100, 0x00000200, 0x00000400, 0x00000800, 
    	    0x00001000, 0x00002000, 0x00004000, 0x00008000, 
    	    0x00010000, 0x00020000, 0x00040000, 0x00080000, 
    	    0x00100000, 0x00200000, 0x00400000, 0x00800000, 
    	    0x01000000, 0x02000000, 0x04000000, 0x08000000, 
    	    0x10000000, 0x20000000, 0x40000000, 0x80000000
    	}, 
    	{ 
    	    0x01000000, 0x02000000, 0x04000000, 0x08000000, 
    	    0x10000000, 0x20000000, 0x40000000, 0x80000000,
    	    0x00010000, 0x00020000, 0x00040000, 0x00080000, 
    	    0x00100000, 0x00200000, 0x00400000, 0x00800000, 
    	    0x00000100, 0x00000200, 0x00000400, 0x00000800, 
    	    0x00001000, 0x00002000, 0x00004000, 0x00008000, 
    	    0x00000001, 0x00000002, 0x00000004, 0x00000008,
    	    0x00000010, 0x00000020, 0x00000040, 0x00000080 
    	}, 
    	{ 
    	    0x00000080, 0x00000040, 0x00000020, 0x00000010, 
    	    0x00000008, 0x00000004, 0x00000002, 0x00000001,   
    	    0x00008000, 0x00004000, 0x00002000, 0x00001000,  
    	    0x00000800, 0x00000400, 0x00000200, 0x00000100,  
    	    0x00800000, 0x00400000, 0x00200000, 0x00100000,  
    	    0x00080000, 0x00040000, 0x00020000, 0x00010000,  
    	    0x80000000, 0x40000000, 0x20000000, 0x10000000, 
    	    0x08000000, 0x04000000, 0x02000000, 0x01000000
    	}, 
    	{ 
    	    0x80000000, 0x40000000, 0x20000000, 0x10000000, 
    	    0x08000000, 0x04000000, 0x02000000, 0x01000000,  
    	    0x00800000, 0x00400000, 0x00200000, 0x00100000,  
    	    0x00080000, 0x00040000, 0x00020000, 0x00010000,  
    	    0x00008000, 0x00004000, 0x00002000, 0x00001000,  
    	    0x00000800, 0x00000400, 0x00000200, 0x00000100,  
    	    0x00000080, 0x00000040, 0x00000020, 0x00000010,  
    	    0x00000008, 0x00000004, 0x00000002, 0x00000001
    	}
    };
    tst=(char*) BBTable[0];
    lsbf=(*tst==1);
    i=0;
/* it should be
    if (!((image->byte_order==LSBFirst)&& lsbf)&&
      ((image->byte_order==LSBFirst)|| lsbf)) i=1; else i=0;
   but in fact it appeared that image->byte_order does not reflect image
   structure on DECStation and SGI (whil reflect byte order of the server)
   and for proper operation we need only:    
*/
    if (lsbf) i=0; else i=1;	
    if (image->bitmap_bit_order!=LSBFirst) i+=2;
    tableStart=BBTable[i];
    MYMALLOC(columnDescr,cd,image->width);
    for (i=0,j=(-1),k=0,cTmp=cd;i<image->width;i++,k--)
    {
    	if (k==0)
    	{
    	    tmp=tableStart;
    	    k=32;
    	    j++;
    	} 
    	cTmp->pattn= (*tmp++);
    	cTmp->x = j;
    	cTmp++;
    }
    return(cd);
}

/*
**  static void aRescaleZ(float fData, discreteData *dData)
**
**  float fData 		data to scale
**  discreteData *dData		pointer to a structure to put scaled data,
**				with clipping information
**
**  scales data according to information setted up in static (non
**  local) context. For the context description and set up procedure
**  see makeAHistImage()
**
*/

static void aRescaleZ(float fData, discreteData *dData)
{
    if (! zLogScaling)
    {
    	if (fData < zVisibleStart) 
    	{
      	    dData->data=0;
            dData->clipped=TRUE;
        }
        else if (fData > zVisibleEnd)
        {
            dData->data = zFactor;				            	
            dData->clipped = TRUE;
        }
        else
        {
            dData->data = ((double)fData - (double) zVisibleStart) *
               zLinearScaleFactor;
/* correction by 1 for possible rounding indefenite		*/
            if (dData->data < zFactor) dData->data = zFactor;
            dData->clipped=FALSE;
        }
    }
    else
    {
    	if (fData < zVisibleStart) 
    	{
      	    dData->data=0;
            dData->clipped=TRUE;
        }
        else if (fData > zVisibleEnd)
        {
            dData->data = zFactor;				            	
            dData->clipped = TRUE;
        }
        else
        {
            dData->data = (log10((double)fData) - zLogStart) * 
              zLogScaleFactor; 
/* correction by 1 for possible rounding indefenite		*/
            if (dData->data < zFactor) dData->data = zFactor;
            dData->clipped=FALSE;
        }
    }
    dData->data *= width;
}

/* next macros are valid in tne context of makeImage function only 
   (bit image drawing macros
*/
/* draw vertical line generic					*/
#define DRAW_V_LINE_GEN(XPTR, YPTR, SET_HOR, DOT_PUT)		\
    { 								\
        register columnDescr* xc = XPTR;			\
        register CARD32 *h = xc->hor;				\
        register CARD32 *p = YPTR;				\
        if (p < h) 						\
        {							\
            SET_HOR;						\
            p += xc->x;						\
            h += xc->x;						\
	    do							\
            {							\
                DOT_PUT						\
                p += width;					\
            }							\
       	    while(p < h);					\
	}							\
    }
    
/* draw solid vertical line without resetting the horizont	*/    
#define DRAW_V_LINE(XPTR, YPTR ) 				\
    DRAW_V_LINE_GEN(XPTR, YPTR,{}, {*p |= xc->pattn;})
    
/* draw solid vertical line with  resetting the horizont	*/    
#define DRAW_V_LINE_HOR(XPTR, YPTR ) 				\
    DRAW_V_LINE_GEN(XPTR, YPTR,{xc->hor = p; }, {*p |= xc->pattn;})
    
/* draw dotted vertical line without resetting the horizont	*/    
#define DRAW_V_LINE_DOT(XPTR, YPTR ) 				\
    DRAW_V_LINE_GEN(XPTR, YPTR,{}, 				\
    {*p |= xc->pattn & dotMask; dotMask = ~dotMask;})
    
/* draw dotted  vertical line with  resetting the horizont	*/    
#define DRAW_V_LINE_HOR_DOT(XPTR, YPTR ) 			\
    DRAW_V_LINE_GEN(XPTR, YPTR,{xc->hor = p; }, 		\
    {*p |= xc->pattn & dotMask; dotMask = ~dotMask;})
    

#define DRAW_V_SEG_GEN(XPTR, YPTR1, YPTR2, SET_HOR, DOT_PUT)	\
    { 								\
        register columnDescr* xc = XPTR;			\
        register CARD32 *h = xc->hor;				\
        register CARD32 *p = YPTR1;				\
        if (p < h) 						\
        {							\
            SET_HOR;						\
            p += xc->x;						\
            if (h > YPTR2) h = YPTR2 + width;			\
            h += xc->x;						\
	    do							\
            {							\
                DOT_PUT						\
                p += width;					\
            }							\
       	    while(p < h);					\
	}							\
    }
    
/* draw solid vertical segment without resetting the horizont	*/    
#define DRAW_V_SEG(XPTR, YPTR1, YPTR2 ) 			\
    DRAW_V_SEG_GEN(XPTR, YPTR1, YPTR2,{}, {*p |= xc->pattn;})
    
/* draw solid vertical segment with  resetting the horizont	*/    
#define DRAW_V_SEG_HOR(XPTR, YPTR1, YPTR2)			\
    DRAW_V_SEG_GEN(XPTR, YPTR1, YPTR2,				\
    {xc->hor = p; }, {*p |= xc->pattn;})
    
/* draw dotted vertical segment without resetting the horizont	*/    
#define DRAW_V_SEG_DOT(XPTR, YPTR1, YPTR2)  			\
    DRAW_V_SEG_GEN(XPTR, YPTR1, YPTR2,{}, 			\
    {*p |= xc->pattn & dotMask; dotMask = ~dotMask;})
    
/* draw dotted  vertical segment with  resetting the horizont	*/    
#define DRAW_V_SEG__DOT_HOR(XPTR, YPTR1, YPTR2)			\
    DRAW_V_SEG_GEN(XPTR, YPTR1, YPTR2,{xc->hor = p; }, 		\
    {*p |= xc->pattn & dotMask; dotMask = ~dotMask;})


#define PUT_C_POINT_HOR(XPTR, YPTR)				\
    { 								\
	if (YPTR < XPTR->hor)					\
	{							\
            *(YPTR + XPTR->x ) |= XPTR->pattn;			\
            XPTR->hor=YPTR;					\
	}							\
    }

#define PUT_C_DBL_POINT_V_HOR(XPTR, YPTR)			\
    { 								\
	if (YPTR < XPTR->hor)					\
	{							\
            *(YPTR + XPTR->x ) |= XPTR->pattn;			\
            if (YPTR + delta < XPTR->hor)			\
                 DRAW_V_LINE(XPTR, YPTR + delta);		\
            XPTR->hor=YPTR;					\
	}							\
    }

#define PUT_C_DBL_POINT_DOT(XPTR, YPTR)				\
    { 								\
	if (YPTR < XPTR->hor)					\
	{							\
            *(YPTR + XPTR->x ) |= XPTR->pattn;			\
            if (YPTR + delta < XPTR->hor)			\
                *(YPTR + delta + XPTR->x ) |= 			\
                  XPTR->pattn & dotMask;			\
	}							\
        dotMask = ~dotMask;					\
    }

#define PUT_C_DBL_POINT(XPTR, YPTR)				\
    { 								\
	if (YPTR < XPTR->hor)					\
	{							\
            *(YPTR + XPTR->x ) |= XPTR->pattn;			\
            if (YPTR + delta < XPTR->hor)			\
                *(YPTR + delta + XPTR->x ) |= 			\
                  XPTR->pattn;					\
	}							\
    }

#define PUT_C_POINT(XPTR, YPTR)					\
    { 								\
	if (YPTR < XPTR->hor)					\
	{							\
            *(YPTR + XPTR->x ) |= XPTR->pattn;			\
	}							\
    }
  	 		
#define PUT_C_POINT_DOT_HOR(XPTR, YPTR)				\
    { 								\
	if (YPTR < XPTR->hor)					\
	{							\
            *(YPTR + XPTR->x ) |= XPTR->pattn & dotMask;	\
            XPTR->hor=YPTR;					\
	}							\
        dotMask = ~dotMask;					\
    }

#define PUT_C_POINT_DOT(XPTR, YPTR)				\
    { 								\
	if (YPTR < XPTR->hor)					\
	{							\
            *(YPTR + XPTR->x ) |= XPTR->pattn & dotMask;	\
	}							\
        dotMask = ~dotMask;					\
    }
  	 		
#define PUT_POINT_HOR(XPTR, YPTR)				\
    { 								\
	*(YPTR + XPTR->x ) |= XPTR->pattn; 			\
	XPTR->hor=YPTR;						\
    }
  	 		

#define DRAW_LINE(F0, LINE_DESCR, PUT)				\
    { 								\
    fIdx = LINE_DESCR -> f_0Idx + F0;				\
    linePtr = &(fIdx -> firstStep);				\
    colXPtr += linePtr->dx;					\
    rowYPtr += linePtr->dy;					\
	md -= linePtr->dl;					\
    linePtr = fIdx -> regularStep;				\
    while (md >= 0)						\
	{							\
            PUT(colXPtr, rowYPtr);				\
	    colXPtr += linePtr->dx;				\
	    rowYPtr += linePtr->dy;				\
	    md -= linePtr->dl;					\
	    linePtr++;						\
	}							\
    }

/* Bin parameters calculating macros for drawBin() function
   Works in the context of static (nonlocal) variable
   declared for makeAHistImage() -- drawBin() interface
*/
   
#define MAXPIC_2 (MAXPIC * MAXPIC)
#define MAXPIC_2_1 (MAXPIC * MAXPIC * 2)
#define LOW_BITS_2_1 (LOWBITS * 2 + 1)
#define LOW_BIT_MASK_2_1 ((MAXPIC_2_1 - 1))
   	
#define MK_SHORT_MB_REP(NUMBER, MBNUMBER)			\
{								\
    long int number = NUMBER;					\
    MBNUMBER.highDig = number >> LOW_BITS_2_1;			\
    MBNUMBER.firstDig = number & LOW_BIT_MASK_2_1;		\
/* assume right shift is arithmetic, but below is correction	\
   for the case it is logical (for better portability)	*/	\
    if (number < 0 && MBNUMBER.highDig > 0)			\
    {								\
         MBNUMBER.highDig |= (~((long int)-1 >> LOW_BITS_2_1));	\
         printf("tratata\n");					\
    }    							\
}

/*
**  Macro to add mBNumbers (it's possible to change it to
**  a function, but it's better to have this operation 
**  faster).
**
**  MB_NUM_1, MB_NUM_2 - mBNumbers (type) to add, should have the
**  same firstBase, secondBase, factor to provide correct
**  result (no checks performing) 
**
**  OVF_ACTION should be a C statement (possible empty)
**
**  Calculates MB_NUM_1 = MB_NUM_1 + MB_NUM_2
**
**  If 'carry' appeares from firstDig to hightDig (which means,
**  that integer part of number get change from the addition
**  of fractional part), OVF_ACTION is executed
**
*/
#define PLUS_N(MB_NUM_1, MB_NUM_2, OVF_ACTION)				\
{									\
    (MB_NUM_1).highDig += (MB_NUM_2).highDig;				\
    (MB_NUM_1).firstDig += (MB_NUM_2).firstDig;				\
    if ((MB_NUM_1).firstDig >= MAXPIC_2_1 )				\
    {									\
        (MB_NUM_1).firstDig -=  MAXPIC_2_1;				\
        (MB_NUM_1).highDig ++;						\
        { OVF_ACTION }							\
    }									\
}

/*
** Same kind of macro, but for binPrm.
** Two OVF_ACTION should be provided for each of coordinates
*/
#define PLUS_B_X_Y(BIN_1, BIN_2 )						\
{									\
    PLUS_N (BIN_1.x, BIN_2.x,						\
        {BIN_1.fx.highDig -= dfx_dx; 					\
         BIN_1.fy.highDig -= dfy_dx;})					\
    PLUS_N (BIN_1.y, BIN_2.y,						\
        {BIN_1.fx.highDig -= dfx_dy;					\
         BIN_1.fy.highDig -= dfy_dy;})					\
    PLUS_N (BIN_1.fx, BIN_2.fx, {}) 					\
    PLUS_N (BIN_1.fy, BIN_2.fy,	{})					\
}

#define PLUS_B_X(BIN_1, BIN_2 )						\
{									\
    PLUS_N (BIN_1.x, BIN_2.x,						\
        {BIN_1.fx.highDig -= dfx_dx;})					\
    PLUS_N (BIN_1.y, BIN_2.y,						\
        {BIN_1.fx.highDig -= dfx_dy;})					\
    PLUS_N (BIN_1.fx, BIN_2.fx, {}) 					\
}

#define PLUS_B_Y(BIN_1, BIN_2 )						\
{									\
    PLUS_N (BIN_1.x, BIN_2.x,						\
        {BIN_1.fy.highDig -= dfy_dx;})					\
    PLUS_N (BIN_1.y, BIN_2.y,						\
        {BIN_1.fy.highDig -= dfy_dy;})					\
    PLUS_N (BIN_1.fy, BIN_2.fy,	{})					\
}

#define PLUS_B(BIN_1, BIN_2 )						\
{									\
    PLUS_N (BIN_1.x, BIN_2.x, { })					\
    PLUS_N (BIN_1.y, BIN_2.y, { })					\
}

/*
** Manhattan length between BIN_1 and BIN_2 (bin start/vertex points)
** for the case increment is (negative) and along virtual axis
*/
#define MANH(BIN_1, BIN_2, ML)						\
{									\
     ML = BIN_1.x.highDig  - BIN_2.x.highDig;				\
     if (ML < 0) ML = -ML;						\
     ML +=  (BIN_1.y.highDig - BIN_2.y.highDig);				\
}

#define MAKE_X_BIN_PRM_X_Y(X,BIN_PRM)					\
    {									\
 	MK_SHORT_MB_REP(X * map2.x.x + xBase, BIN_PRM.x);		\
 	MK_SHORT_MB_REP(X * map2.y.x + yBase, BIN_PRM.y);		\
 	MK_SHORT_MB_REP(BIN_PRM.x.firstDig *  dfx_dx + 			\
 	  BIN_PRM.y.firstDig *  dfx_dy + xLineBase, BIN_PRM.fx);	\
        MK_SHORT_MB_REP(BIN_PRM.x.firstDig *  dfy_dx + 			\
          BIN_PRM.y.firstDig *  dfy_dy + yLineBase, BIN_PRM.fy);	\
        BIN_PRM.x.firstDig += MAXPIC_2;					\
        if (BIN_PRM.x.firstDig >= MAXPIC_2_1 )				\
        {								\
            BIN_PRM.x.firstDig -=  MAXPIC_2_1;				\
            BIN_PRM.x.highDig ++;					\
            BIN_PRM.fx.highDig -= dfx_dx;				\
            BIN_PRM.fy.highDig -= dfy_dx;				\
        }								\
        BIN_PRM.y.firstDig += MAXPIC_2;					\
        if (BIN_PRM.y.firstDig >= MAXPIC_2_1 )				\
        {								\
            BIN_PRM.y.firstDig -=  MAXPIC_2_1;				\
            BIN_PRM.y.highDig ++;					\
            BIN_PRM.fx.highDig -= dfx_dy;				\
            BIN_PRM.fy.highDig -= dfy_dy;				\
        }								\
    }

#define MAKE_X_BIN_PRM_X(X,BIN_PRM)				 	\
    {									\
 	MK_SHORT_MB_REP(X * map2.x.x + xBase, BIN_PRM.x);		\
 	MK_SHORT_MB_REP(X * map2.y.x + yBase, BIN_PRM.y);		\
 	MK_SHORT_MB_REP(BIN_PRM.x.firstDig *  dfx_dx + 			\
 	  BIN_PRM.y.firstDig *  dfx_dy + xLineBase, BIN_PRM.fx);	\
        BIN_PRM.x.firstDig += MAXPIC_2;					\
        if (BIN_PRM.x.firstDig >= MAXPIC_2_1 )				\
        {								\
            BIN_PRM.x.firstDig -=  MAXPIC_2_1;				\
            BIN_PRM.x.highDig ++;					\
            BIN_PRM.fx.highDig -= dfx_dx;					\
        }								\
        BIN_PRM.y.firstDig += MAXPIC_2;					\
        if (BIN_PRM.y.firstDig >= MAXPIC_2_1 )				\
        {								\
            BIN_PRM.y.firstDig -=  MAXPIC_2_1;				\
            BIN_PRM.y.highDig ++;					\
            BIN_PRM.fx.highDig -= dfx_dy;				\
        }								\
    }

#define MAKE_Y_BIN_PRM_X_Y(Y,BIN_PRM) 					\
    {									\
 	MK_SHORT_MB_REP(Y * map2.x.y, BIN_PRM.x);			\
 	MK_SHORT_MB_REP(Y * map2.y.y, BIN_PRM.y);			\
 	MK_SHORT_MB_REP(BIN_PRM.x.firstDig * dfx_dx + 			\
 	  BIN_PRM.y.firstDig * dfx_dy, BIN_PRM.fx);			\
        MK_SHORT_MB_REP(BIN_PRM.x.firstDig * dfy_dx + 			\
          BIN_PRM.y.firstDig * dfy_dy, BIN_PRM.fy);			\
    } 

#define MAKE_Y_BIN_PRM_Y(Y,BIN_PRM) 					\
    {									\
 	MK_SHORT_MB_REP(Y * map2.x.y, BIN_PRM.x);			\
 	MK_SHORT_MB_REP(Y * map2.y.y, BIN_PRM.y);			\
        MK_SHORT_MB_REP(BIN_PRM.x.firstDig * dfy_dx + 			\
          BIN_PRM.y.firstDig * dfy_dy, BIN_PRM.fy);			\
    } 

/* make discrete representation of all for bin coordinates (xMin,yMin,
xMax, yMax). First argument is coordinates array, second - <binPrm>
array.
*/
#define MAKE_COORD_BIN_PRM(BIN_COORD, BIN_COORD_PRM)			\
    {									\
        MAKE_X_BIN_PRM_X(BIN_COORD[_X][_START],				\
          BIN_COORD_PRM[_X][_START]) 					\
        MAKE_X_BIN_PRM_X_Y(BIN_COORD[_X][_END],				\
          BIN_COORD_PRM[_X][_END]) 					\
        MAKE_Y_BIN_PRM_Y(BIN_COORD[_Y][_START],				\
          BIN_COORD_PRM[_Y][_START]) 					\
        MAKE_Y_BIN_PRM_X_Y(BIN_COORD[_Y][_END],				\
          BIN_COORD_PRM[_Y][_END]) 					\
    }

/* make discrete representation of all for bin corners 
from its coordinates representation 
*/
#define MAKE_CORNER_BIN_PRM(BIN_COORD_PRM, BIN_CORNER_PRM)		\
    {									\
        BIN_CORNER_PRM[0][0] = BIN_COORD_PRM[_X][_END];			\
        PLUS_B_X_Y(BIN_CORNER_PRM[0][0],BIN_COORD_PRM[_Y][_END]);	\
        BIN_CORNER_PRM[0][1] = BIN_COORD_PRM[_X][_END];			\
        PLUS_B_Y(BIN_CORNER_PRM[0][1],BIN_COORD_PRM[_Y][_START]);	\
        BIN_CORNER_PRM[1][0] = BIN_COORD_PRM[_X][_START];		\
        PLUS_B_X(BIN_CORNER_PRM[1][0],BIN_COORD_PRM[_Y][_END]);		\
        BIN_CORNER_PRM[1][1] = BIN_COORD_PRM[_X][_START];		\
        PLUS_B(BIN_CORNER_PRM[1][1],BIN_COORD_PRM[_Y][_START]);		\
    }

void makeAHistImage(Hist2DWidget w)
{
    discreteMap *dMap;
    vector vertex ;
    int md;
    fIndex *fIdx;
    lineStepDescr *linePtr;
    CARD32 *rowYPtr;			/* pointer to Y row in the bitmap   */
    columnDescr *colXPtr;		/* poiter to current X column in
    					   columnDescr structure	    */
    CARD32 dotMask;    					   
    int histoBase [2][2];
    binPrmA baseCoordPrm [2][2];
    binPrmA baseCornerPrm [2][2];
    walkStep *walkArr;
    walkStep *tmp; 
    int levNum;
    int walkIdx;
    aHistNode *curNode;
    walkBranch walkDescr[2];
    /* whole histogram X-Y range in source coordinates        	*/
    fcell aHistRange;
/* 
   Those variable are declared above as static (non-local) 
   to provide interface between makeAHistImage() drawBin() and
   aRescaleZ() functions. (Because drawBin() and  aRescaleZ() are called for
   each bin, passing all the paramerters each time could slows down 
   the processing. 
   This variables are setted up by makeAHistImage() and are used by
   functions calld from makeAHistImage()

     initial visible range in source coordintates           	
static float visible[2][2];
static float xScaleFactor;
static float yScaleFactor;
static float zScaleFactor;
static matrix2 map2;
static int width;		width of the bitmap (in 32 bit units)   
static columnDescr *columns;	array of elements corresponding to each
    				column of the bitmap. Elemetns contain
    				floating horizont position, offset in
    				the row (in bitmap units) and bit mask
    				(pattern) for given column              
static int dfx_dx;
static int dfx_dy;
static int dfy_dx;
static int dfy_dy;
static int xBase;
static int yBase;
static int xLineBase;
static int yLineBase;
static lineDescr *xLine;
static lineDescr *yLine;
static Boolean zLogScaling;
static double zLinearScaleFactor;
static double zLogScaleFactor;
static double zLogStart;
static double zVisibleStart;
static double zVisibleEnd;
static int zFactor;
static int mapX;
static int mapY;
static int mapXStart;
static int mapYStart;
static int mapXEnd;
static int mapYEnd;
static int mapXStartValue;
static int mapYStartValue;

*/
    
    
    if ((w->hist2D.discrMap==NULL)|| (w->hist2D.aHist==NULL))
    {
        if (w->hist2D.wireImage==NULL)
	    return;
        MYFREE(w->hist2D.wireImage->data);
        w->hist2D.wireImage->data=NULL;
        return;
    }
    image=w->hist2D.wireImage;
    dMap=w->hist2D.discrMap;
    vertex  =  dMap->vertex ;
    map = dMap->map;
    map2.x.x = 2 * map.x.x;
    map2.x.y = 2 * map.x.y;
    map2.y.x = 2 * map.y.x;
    map2.y.y = 2 * map.y.y;
    
    createImageData(image,w);
    width = image->bytes_per_line / 4;
    columns= makeColumnDescr ( image);
    yLine = makeLineDescr (map.y.x, map.x.x, width);
    xLine = makeLineDescr (map.y.y, map.x.y, width);
    dfx_dx =   xLine -> a;
    dfx_dy = - xLine -> b;
    dfy_dx =   yLine -> a;
    dfy_dy = - yLine -> b;
    
/* assume that discrete limit range is always MAXPIC * MAXPIC for X and Y */    
    xScaleFactor = ((double)MAXPIC * (double)MAXPIC * (double)MAXPIC * 
      (double)MAXPIC) /
      (double)(w->hist2D.xyVisibleRange.end.x - 
      w->hist2D.xyVisibleRange.start.x) / 
      (double)(w->hist2D.sourceHistogram->xMax - 
      w->hist2D.sourceHistogram->xMin);
    yScaleFactor = ((double)MAXPIC * (double)MAXPIC * (double)MAXPIC * 
      (double)MAXPIC) /
      (double)(w->hist2D.xyVisibleRange.end.y - 
      w->hist2D.xyVisibleRange.start.y) / 
      (double)(w->hist2D.sourceHistogram->yMax - 
      w->hist2D.sourceHistogram->yMin);
      
       
    aHistRange.c[_X][_START] = w->hist2D.sourceHistogram->xMin;
    aHistRange.c[_X][_END] = w->hist2D.sourceHistogram->xMax;
    aHistRange.c[_Y][_START] = w->hist2D.sourceHistogram->yMin;
    aHistRange.c[_Y][_END] = w->hist2D.sourceHistogram->yMax;
    
/* recalculate visible range in descrete units back to float
   source coordinates. xyHistoRange suppose to be
   [0 .. MAXPIC * MAXPIC] for each coordinate in the case of adaptive
   histogram, regardless of any histogram data
*/
    visible[_X][_START] = (aHistRange.c[_X][_END] - aHistRange.c[_X][_START]) *
      (float) w->hist2D.xyVisibleRange.start.x / (float) (MAXPIC_2) + 
      aHistRange.c[_X][_START];
    visible[_X][_END] = (aHistRange.c[_X][_END] - aHistRange.c[_X][_START]) *
      (float) w->hist2D.xyVisibleRange.end.x / (float) (MAXPIC_2) + 
      aHistRange.c[_X][_START];  
    visible[_Y][_START] = (aHistRange.c[_Y][_END] - aHistRange.c[_Y][_START]) *
      (float) w->hist2D.xyVisibleRange.start.y / (float) (MAXPIC_2) + 
      aHistRange.c[_Y][_START];
    visible[_Y][_END] = (aHistRange.c[_Y][_END] - aHistRange.c[_Y][_START]) *
      (float) w->hist2D.xyVisibleRange.end.y / (float) (MAXPIC_2) + 
      aHistRange.c[_Y][_START];  

    zFactor = dMap->zFactor;  
    zVisibleStart = w->hist2D.zVisibleStart;
    zVisibleEnd = w->hist2D.zVisibleEnd;
    zLogScaling = w->hist2D.zLogScaling;
    if (!zLogScaling)
        zLinearScaleFactor = (double) (dMap->zFactor) / 
          (zVisibleEnd - zVisibleStart);
    else 
    {         
	zLogScaleFactor = (double) (dMap->zFactor) / 
	  (log10( zVisibleEnd ) - log10( zVisibleStart ));
	zLogStart = log10((double) zVisibleStart);
    }
    xBase = (dMap->vertex.x - map.x.x - map.x.y) * MAXPIC_2_1;
    yBase = (dMap->vertex.y - map.y.x - map.y.y) * MAXPIC_2_1;
    xLineBase = xLine->f2Offset * MAXPIC_2;
    yLineBase = yLine->f2Offset * MAXPIC_2;   
    
    if (dMap->rotMatr.x.x == 0)
    {
        mapX = _Y;
        mapY = _X;
    }
    else
    {
        mapX = _X;
        mapY = _Y;
    } 
    if (dMap->rotMatr.x.x + dMap->rotMatr.y.x < 0)
    {
        xScaleFactor = -xScaleFactor;
        mapXStart = _END;
        mapXEnd = _START;
        mapXStartValue = MAXPIC_2;
        walkDescr[_X].left = 1;
        walkDescr[_X].right = 0;
    }
    else 
    {
        mapXStart = _START;
        mapXEnd = _END;
        mapXStartValue = 0;
        walkDescr[_X].left = 0;
        walkDescr[_X].right = 1;
    }
    if (dMap->rotMatr.x.y + dMap->rotMatr.y.y < 0)
    {
        yScaleFactor = -yScaleFactor;
        mapYStart = _END;
        mapYEnd = _START;
        mapYStartValue = MAXPIC_2;
        walkDescr[_Y].left = 1;
        walkDescr[_Y].right = 0;
    }
    else 
    {
        mapYStart = _START;
        mapYEnd = _END;
        mapYStartValue = 0;
        walkDescr[_Y].left = 0;
        walkDescr[_Y].right = 1;
    }

 /* init floating horizont and draw axes  */
    histoBase [_X][_START] = 0;
    histoBase [_X][_END] = MAXPIC_2;
    histoBase [_Y][_START] = 0;
    histoBase [_Y][_END] = MAXPIC_2;
    MAKE_COORD_BIN_PRM(histoBase, baseCoordPrm);
    MAKE_CORNER_BIN_PRM(baseCoordPrm,baseCornerPrm);
    rowYPtr = (CARD32*)image->data + 
      baseCornerPrm[0][0].y.highDig * width;
    colXPtr = columns +  baseCornerPrm[0][0].x.highDig;
    PUT_POINT_HOR(colXPtr,rowYPtr);
/* draw line to left 						*/
    MANH( baseCornerPrm[0][0],baseCornerPrm[0][1], md);
    DRAW_LINE( baseCornerPrm[0][0].fx.highDig,xLine,PUT_POINT_HOR);
/* draw line to right 						*/
    rowYPtr = (CARD32*)image->data + 
      baseCornerPrm[0][0].y.highDig * width;
    colXPtr = columns +  baseCornerPrm[0][0].x.highDig;
    MANH( baseCornerPrm[0][0],baseCornerPrm[1][0], md);
    DRAW_LINE( baseCornerPrm[0][0].fy.highDig,yLine,PUT_POINT_HOR);
/* draw left point 							*/
    rowYPtr = (CARD32*)image->data + 
      baseCornerPrm[0][1].y.highDig * width;
    colXPtr = columns +  baseCornerPrm[0][1].x.highDig;
    PUT_C_POINT_HOR(colXPtr, (rowYPtr));
/* draw right  point						*/
    rowYPtr = (CARD32*)image->data + 
      baseCornerPrm[1][0].y.highDig * width;
    colXPtr = columns +  baseCornerPrm[1][0].x.highDig;
    PUT_C_POINT_HOR(colXPtr, rowYPtr);

/* walk along adaptive histogram tree and draw bins		*/
/* set up the walk						*/
    levNum = levelsNumber (w->hist2D.aHist->aNode);
    MYMALLOC (walkStep, walkArr, levNum + 2);
    tmp = walkArr;
/* put extra node on the "stack" array, to provide stop condition for 
walking up							*/
    walkIdx =  _X;							   
    tmp->node = w->hist2D.aHist->aNode;
    tmp++;
    tmp->node = w->hist2D.aHist->aNode;
    tmp->cellSize.c[_X][_START] = w->hist2D.aHist->xMin;
    tmp->cellSize.c[_X][_END] = w->hist2D.aHist->xMax;
    tmp->cellSize.c[_Y][_START] = w->hist2D.aHist->yMin;
    tmp->cellSize.c[_Y][_END] = w->hist2D.aHist->yMax;
    tmp->walkIdx =  _X;
    curNode = tmp->node;
    if ( (w->hist2D.aHist->aNode)->nextNodeOffset != 0)
/* walk itself							*/    
	do 
	{    
     /* while non leaf put nonclipped of both subnodes on "top" of walkArr */
       	    while(curNode -> nextNodeOffset!= 0)
  	    {
     /* current (non-leaf) cell processing, current cell is pointed by tmp, 
	walkIdx correspond to the split of current cell */       
        	(tmp + walkDescr[walkIdx].left)->node = curNode + 1;
        	(tmp + walkDescr[walkIdx].right)->node = 
        	  curNode + curNode -> nextNodeOffset;
		(tmp + 1)->cellSize = tmp->cellSize;
    /* clip bins outside visible range	(drop node from "top" of walkArr */
		if (curNode->data.xySplit >= visible[walkIdx][_END])
		{
		   tmp->cellSize.c[walkIdx][_END] = 
                      curNode->data.xySplit;
                   tmp->node = (tmp + walkDescr[walkIdx].left) -> node;
                }   
        	else if (curNode->data.xySplit <= visible[walkIdx][_START])
        	{
                    tmp ->cellSize.c[walkIdx][_START] = 
                      curNode->data.xySplit;
                    tmp->node = (tmp + walkDescr[walkIdx].right) -> node;
                }   
        	else 
        	{
    /* no clipping happens, proceed with both  subtrees			*/
                    (tmp + walkDescr[walkIdx].left)->cellSize.c[walkIdx][_END] = 
                      curNode->data.xySplit;
                    (tmp + walkDescr[walkIdx].right)->cellSize.c[walkIdx][_START] = 
                      curNode->data.xySplit;
                     tmp->walkIdx = 1 - walkIdx;
	             tmp++;
		}
        	walkIdx = 1 - walkIdx;  
    /* set up node on the "top" of walkArr as current		*/
        	tmp->walkIdx = walkIdx;
        	curNode = tmp->node;
	    } 
	    do 
	    { 
    /*  node appeared to be a leaf, leaf cell processing 	*/
	        drawBin(tmp);
    /* drop node from the "top" of walkArr			*/
        	tmp--;
    /* and set up node on the "top" of walkArr as current - "walking up"*/
        	walkIdx = tmp->walkIdx;
        	curNode = tmp->node;
	    } while (curNode -> nextNodeOffset == 0);
	} while (tmp != walkArr);
    else
    {
        drawBin(tmp);
    }
/* draw back edges of the base (which might not be drawn yet if
   histogram domain smaller then visible area				*/
    {
/* draw  right edge							*/        
  	rowYPtr = (CARD32*)image->data + 
	  (baseCornerPrm[1][0].y.highDig) * width;
	colXPtr = columns +  baseCornerPrm[1][0].x.highDig;
	MANH( baseCornerPrm[1][0],baseCornerPrm[1][1], md);
	DRAW_LINE( baseCornerPrm[1][0].fx.highDig,xLine,PUT_C_POINT);
/* draw left edge							*/        
 	rowYPtr = (CARD32*)image->data + 
	  (baseCornerPrm[0][1].y.highDig) * width;
	colXPtr = columns +  baseCornerPrm[0][1].x.highDig;
	MANH( baseCornerPrm[0][1],baseCornerPrm[1][1], md);
	DRAW_LINE( baseCornerPrm[0][1].fy.highDig,yLine,PUT_C_POINT);
    }	    

/* draw backplanes if required						*/
    if (w->hist2D.backPlanesOn)
    {
        int level;
        labelsToDraw *lT = w->hist2D.currentPicture->zLabels;
        level = vertex.y - map.y.y + dMap->zFactor;
	do
        {
/* draw line on the right backplane				*/        
            dotMask = 0;
	    rowYPtr = (CARD32*)image->data + 
	      (baseCornerPrm[1][0].y.highDig + 
	      level - vertex.y + map.y.y) * width;
	    colXPtr = columns +  baseCornerPrm[1][0].x.highDig;
	    MANH( baseCornerPrm[1][0],baseCornerPrm[1][1], md);
	    DRAW_LINE( baseCornerPrm[1][0].fx.highDig,xLine,PUT_C_POINT_DOT);
/* draw line on the left backplane				*/        
            dotMask = 0;
	    rowYPtr = (CARD32*)image->data + 
	      (baseCornerPrm[0][1].y.highDig + 
	      level - vertex.y + map.y.y) * width;
	    colXPtr = columns +  baseCornerPrm[0][1].x.highDig;
	    MANH( baseCornerPrm[0][1],baseCornerPrm[1][1], md);
	    DRAW_LINE( baseCornerPrm[0][1].fy.highDig,yLine,PUT_C_POINT_DOT);
	    if (lT != NULL)
	    { 
	        level = lT->yCenter;
	        lT = lT->next;
	    }
	    else
	        break;
	}
	while (TRUE);
/* set pointers to right base vertex 	*/   
        rowYPtr = (CARD32*)image->data + 
	  (baseCornerPrm[1][0].y.highDig + 
/* lift to upper cube vertex   */	
          dMap->zFactor) * width; 
	    colXPtr = columns +  baseCornerPrm[1][0].x.highDig;
/* and draw vertical dotted line				*/	
        DRAW_V_LINE_DOT(colXPtr,rowYPtr);
/* set pointers to farmost base vertex and lift to upper cube vertex   */
        rowYPtr = (CARD32*)image->data + 
	  baseCornerPrm[1][1].y.highDig * width; 
	  colXPtr = columns +  baseCornerPrm[1][1].x.highDig;
	PUT_C_POINT_HOR(colXPtr,rowYPtr);
	rowYPtr += dMap->zFactor * width;
/* and draw vertical dotted line				*/	
        DRAW_V_LINE_DOT(colXPtr,rowYPtr);
/* not drawing left vertical - Z axis is there anyway		*/
    }
    MYFREE(walkArr);
    destroyLineDescr (&xLine);
    destroyLineDescr (&yLine);
    MYFREE(columns);
}

static void drawBin(walkStep *curCell)
{
    float bin [2][2];
    float margin;
    int binInt [2][2];
    int cellInt [2][2]; 
    binPrmA binCoordPrm[2][2];
    binPrmA cellCoordPrm[2][2]; 
    binPrmA binCornerPrm[2][2];
    binPrmA cellCornerPrm[2][2]; 
    int md;
    fIndex *fIdx;
    lineStepDescr *linePtr;
    CARD32 *rowYPtr;			/* pointer to Y row in the bitmap   */
    columnDescr *colXPtr;		/* poiter to current X column       */
    discreteData zData;
    float margFactor = MARG_FACTOR;

/* make parameters for bin (curculate cell margins) */    
    margin = (curCell->cellSize.c[_X][_END] - curCell->cellSize.c[_X][_START]) *
      margFactor;
    bin[_X][_START] = curCell->cellSize.c[_X][_START] + margin;
    bin[_X][_END] = curCell->cellSize.c[_X][_END] - margin;
    margin = (curCell->cellSize.c[_Y][_END] - curCell->cellSize.c[_Y][_START]) *
      margFactor;
    bin[_Y][_START] = curCell->cellSize.c[_Y][_START] + margin;
    bin[_Y][_END] = curCell->cellSize.c[_Y][_END] - margin;
/* make discrete parameters for bin and cell, make rotation map	    */
    if (bin[_X][_START] <= visible[_X][_START]) 
        binInt [mapX][mapXStart] = mapXStartValue; 
    else
        binInt [mapX][mapXStart] =   (bin[_X][_START] - visible[_X][_START]) 
          * xScaleFactor + mapXStartValue;
    if (bin[_X][_END] >= visible[_X][_END]) 
        binInt [mapX][mapXEnd] = (MAXPIC * MAXPIC) - mapXStartValue; 
    else
        binInt [mapX][mapXEnd] =   (bin[_X][_END] - visible[_X][_START]) 
          * xScaleFactor + mapXStartValue;
    if (bin[_Y][_START] <= visible[_Y][_START]) 
        binInt [mapY][mapYStart] = mapYStartValue; 
    else
        binInt [mapY][mapYStart] =   (bin[_Y][_START] - visible[_Y][_START]) 
          * yScaleFactor + mapYStartValue;
    if (bin[_Y][_END] >= visible[_Y][_END]) 
        binInt [mapY][mapYEnd] = (MAXPIC * MAXPIC) - mapYStartValue; 
    else
        binInt [mapY][mapYEnd] =   (bin[_Y][_END] - visible[_Y][_START]) 
          * yScaleFactor + mapYStartValue;

    if (curCell->cellSize.c[_X][_START] <= visible[_X][_START]) 
        cellInt [mapX][mapXStart] = mapXStartValue; 
    else
        cellInt [mapX][mapXStart] =   
          (curCell->cellSize.c[_X][_START] - visible[_X][_START]) 
          * xScaleFactor + mapXStartValue;
    if (curCell->cellSize.c[_X][_END] >= visible[_X][_END]) 
        cellInt [mapX][mapXEnd] = (MAXPIC * MAXPIC) - mapXStartValue; 
    else
        cellInt [mapX][mapXEnd] =   
          (curCell->cellSize.c[_X][_END] - visible[_X][_START]) 
          * xScaleFactor + mapXStartValue;
    if (curCell->cellSize.c[_Y][_START] <= visible[_Y][_START]) 
        cellInt [mapY][mapYStart] = mapYStartValue; 
    else
        cellInt [mapY][mapYStart] =   
          (curCell->cellSize.c[_Y][_START] - visible[_Y][_START]) 
          * yScaleFactor + mapYStartValue;
    if (curCell->cellSize.c[_Y][_END] >= visible[_Y][_END]) 
        cellInt [mapY][mapYEnd] = (MAXPIC * MAXPIC) - mapYStartValue; 
    else
        cellInt [mapY][mapYEnd] =   
          (curCell->cellSize.c[_Y][_END] - visible[_Y][_START]) 
          * yScaleFactor + mapYStartValue;
/* check range (probably optional, becase clipping is done, 
   but more safty to do this because of possible rounding errors).
   If range went bad, result can be "unpredictable" up to a crash */
    if (binInt[_X][_START] < 0) binInt[_X][_START] = 0;
    if (binInt[_X][_END] > (MAXPIC * MAXPIC)) 
        binInt[_X][_END]=(MAXPIC * MAXPIC);
    if (binInt[_Y][_START] < 0) binInt[_Y][_START] = 0;
    if (binInt[_Y][_END] > (MAXPIC * MAXPIC)) 
        binInt[_Y][_END]=(MAXPIC * MAXPIC);
        
    if (cellInt[_X][_START] < 0) cellInt[_X][_START] = 0;
    if (cellInt[_X][_END] > (MAXPIC * MAXPIC)) 
        cellInt[_X][_END]=(MAXPIC * MAXPIC);
    if (cellInt[_Y][_START] < 0) cellInt[_Y][_START] = 0;
    if (cellInt[_Y][_END] > (MAXPIC * MAXPIC))
        cellInt[_Y][_END]=(MAXPIC * MAXPIC);

    if (binInt[_X][_START] < binInt[_X][_END] && 
      binInt[_Y][_START] < binInt[_Y][_END])
    {  
/* make parameters for line generation					*/        
	MAKE_COORD_BIN_PRM(binInt, binCoordPrm);
	MAKE_CORNER_BIN_PRM(binCoordPrm,binCornerPrm);
	aRescaleZ(curCell->node->data.zData, &zData);
/* draw from cell end to bin end parts of farmost base edges if cell is
   on the boundary of the base						*/
	if (cellInt[_X][_START] == 0 || cellInt[_Y][_START] ==0)
	{
            MAKE_COORD_BIN_PRM(cellInt, cellCoordPrm);
            if (cellInt[_X][_START] == 0)
            {
        	cellCornerPrm[1][0] = cellCoordPrm[_X][_START];
        	PLUS_B_X(cellCornerPrm[1][0],cellCoordPrm[_Y][_END]);
        	cellCornerPrm[1][1] = cellCoordPrm[_X][_START];
        	PLUS_B_X(cellCornerPrm[1][1],binCoordPrm[_Y][_END]);
		rowYPtr = (CARD32*)image->data + 
		  cellCornerPrm[1][0].y.highDig * width;
		colXPtr = columns +  cellCornerPrm[1][0].x.highDig;
	/* draw line to left 						*/
		MANH( cellCornerPrm[1][0],cellCornerPrm[1][1], md);
		DRAW_LINE( cellCornerPrm[1][0].fx.highDig,xLine,PUT_C_POINT);
		cellCornerPrm[1][0] = cellCornerPrm[1][1];
	    }
            if (cellInt[_Y][_START] == 0)
            {
        	cellCornerPrm[0][1] = cellCoordPrm[_Y][_START];
        	PLUS_B_Y(cellCornerPrm[0][1],cellCoordPrm[_X][_END]);
        	cellCornerPrm[1][1] = cellCoordPrm[_Y][_START];
        	PLUS_B_Y(cellCornerPrm[1][1],binCoordPrm[_X][_END]);
		rowYPtr = (CARD32*)image->data + 
		  cellCornerPrm[0][1].y.highDig * width;
		colXPtr = columns +  cellCornerPrm[0][1].x.highDig;
	/* draw line to right 						*/
		MANH( cellCornerPrm[0][1],cellCornerPrm[1][1], md);
		DRAW_LINE( cellCornerPrm[0][1].fy.highDig,yLine,PUT_C_POINT);
		cellCornerPrm[0][1] = cellCornerPrm[1][1];
	    }
            cellCornerPrm[1][1] = cellCoordPrm[_Y][_START];
            PLUS_B(cellCornerPrm[1][1],cellCoordPrm[_X][_START]);
	}
/* draw a bin								*/    
/* draw bin base							*/
	rowYPtr = (CARD32*)image->data + 
	  binCornerPrm[0][0].y.highDig * width;
	colXPtr = columns +  binCornerPrm[0][0].x.highDig;
	PUT_C_POINT_HOR(colXPtr,rowYPtr);
    /* draw line to left 						*/
	MANH( binCornerPrm[0][0],binCornerPrm[0][1], md);
	DRAW_LINE( binCornerPrm[0][0].fx.highDig,xLine,PUT_C_POINT_HOR);
    /* draw line to right 						*/
	rowYPtr = (CARD32*)image->data + 
	  binCornerPrm[0][0].y.highDig * width;
	colXPtr = columns +  binCornerPrm[0][0].x.highDig;
	MANH( binCornerPrm[0][0],binCornerPrm[1][0], md);
	DRAW_LINE( binCornerPrm[0][0].fy.highDig,yLine,PUT_C_POINT_HOR);
    /* draw left point 							*/
	rowYPtr = (CARD32*)image->data + 
	  binCornerPrm[0][1].y.highDig * width;
	colXPtr = columns +  binCornerPrm[0][1].x.highDig;
	PUT_C_POINT_HOR(colXPtr, (rowYPtr));
	rowYPtr += zData.data;
    /* draw left vertical edge, 					*/ 
	DRAW_V_LINE_HOR(colXPtr, rowYPtr);
    /* draw right  point						*/
	rowYPtr = (CARD32*)image->data + 
	  binCornerPrm[1][0].y.highDig * width;
	colXPtr = columns +  binCornerPrm[1][0].x.highDig;
	PUT_C_POINT_HOR(colXPtr, rowYPtr);
	rowYPtr += zData.data;
    /* draw right vertical edge, 					*/ 
	DRAW_V_LINE_HOR(colXPtr, rowYPtr);

    /* draw fron part of main bar now					*/
    /* setup pointers to the nearmost vertex 				*/ 
	rowYPtr = (CARD32*)image->data + 
	  binCornerPrm[0][0].y.highDig * width + zData.data;
	colXPtr = columns +  binCornerPrm[0][0].x.highDig;
    /* draw central vertical edge 					*/    
        DRAW_V_LINE_HOR(colXPtr,rowYPtr);
    /* draw to left							*/
	MANH( binCornerPrm[0][0],binCornerPrm[0][1], md);
	DRAW_LINE( binCornerPrm[0][0].fx.highDig,xLine,PUT_C_POINT_HOR);
	rowYPtr = (CARD32*)image->data + 
	  binCornerPrm[0][0].y.highDig * width + zData.data;
	colXPtr = columns +  binCornerPrm[0][0].x.highDig;
    /* draw to right							*/
	MANH( binCornerPrm[0][0],binCornerPrm[1][0], md);
	DRAW_LINE( binCornerPrm[0][0].fy.highDig,yLine,PUT_C_POINT_HOR);
    /* draw  top edges of the bin. The order is essential, and
	next <if> is better to move outside the loop. (Let optimaser
	to do this if it can)						*/
	if (map.x.y < map.y.y)
	{
	    rowYPtr = (CARD32*)image->data + 
	      binCornerPrm[1][0].y.highDig * width + zData.data;
	    colXPtr = columns +  binCornerPrm[1][0].x.highDig;
	    MANH( binCornerPrm[1][0],binCornerPrm[1][1], md);
	    if (zData.clipped)
	    {
	        DRAW_LINE( binCornerPrm[1][0].fx.highDig,xLine,DRAW_V_LINE_HOR);
	    }
	    else 
	    {
	  	DRAW_LINE( binCornerPrm[1][0].fx.highDig,xLine,PUT_C_POINT_HOR);
	    }
	    rowYPtr = (CARD32*)image->data + 
	      binCornerPrm[0][1].y.highDig * width + zData.data;
	    colXPtr = columns +  binCornerPrm[0][1].x.highDig;
	    MANH( binCornerPrm[0][1],binCornerPrm[1][1], md);
	    if (zData.clipped)
	    {
	        DRAW_LINE( binCornerPrm[0][1].fy.highDig,yLine,DRAW_V_LINE_HOR);
	    }
	    else 
	    {
		DRAW_LINE( binCornerPrm[0][1].fy.highDig,yLine,PUT_C_POINT_HOR);
	    }
  	} else
	{
	    rowYPtr = (CARD32*)image->data + 
	      binCornerPrm[0][1].y.highDig * width + zData.data;
	    colXPtr = columns +  binCornerPrm[0][1].x.highDig;
	    MANH( binCornerPrm[0][1],binCornerPrm[1][1], md);
	    if (zData.clipped)
	    {
	        DRAW_LINE( binCornerPrm[0][1].fy.highDig,yLine,DRAW_V_LINE_HOR);
	    }
	    else 
	    {
		DRAW_LINE( binCornerPrm[0][1].fy.highDig,yLine,PUT_C_POINT_HOR);
	    }
	    rowYPtr = (CARD32*)image->data + 
	      binCornerPrm[1][0].y.highDig * width + zData.data;
	    colXPtr = columns +  binCornerPrm[1][0].x.highDig;
	    MANH( binCornerPrm[1][0],binCornerPrm[1][1], md);
	    if (zData.clipped)
	    {
	        DRAW_LINE( binCornerPrm[1][0].fx.highDig,xLine,DRAW_V_LINE_HOR);
	    }
	    else 
	    {
	  	DRAW_LINE( binCornerPrm[1][0].fx.highDig,xLine,PUT_C_POINT_HOR);
	    }
        }
	if (cellInt[_X][_START] == 0)
	{
	    rowYPtr = (CARD32*)image->data + 
	      cellCornerPrm[1][0].y.highDig * width;
	    colXPtr = columns +  cellCornerPrm[1][0].x.highDig;
    /* draw line to left 						*/
	    MANH( cellCornerPrm[1][0],cellCornerPrm[1][1], md);
	    DRAW_LINE( cellCornerPrm[1][0].fx.highDig,xLine,PUT_C_POINT);
	}
	if (cellInt[_Y][_START] == 0)
	{
	    rowYPtr = (CARD32*)image->data + 
	      cellCornerPrm[0][1].y.highDig * width;
	    colXPtr = columns +  cellCornerPrm[0][1].x.highDig;
    /* draw line to right 						*/
	    MANH( cellCornerPrm[0][1],cellCornerPrm[1][1], md);
	    DRAW_LINE( cellCornerPrm[0][1].fy.highDig,yLine,PUT_C_POINT);
	}
    }
    else
    {
        MAKE_COORD_BIN_PRM(cellInt, cellCoordPrm);
        MAKE_CORNER_BIN_PRM(cellCoordPrm,cellCornerPrm);
        if (cellInt[_X][_START] == 0)
        {
	    rowYPtr = (CARD32*)image->data + 
	      cellCornerPrm[1][0].y.highDig * width;
	    colXPtr = columns +  cellCornerPrm[1][0].x.highDig;
    /* draw line to left 						*/
	    MANH( cellCornerPrm[1][0],cellCornerPrm[1][1], md);
	    DRAW_LINE( cellCornerPrm[1][0].fx.highDig,xLine,PUT_C_POINT);
	}
        if (cellInt[_Y][_START] == 0)
        {
	    rowYPtr = (CARD32*)image->data + 
	      cellCornerPrm[0][1].y.highDig * width;
	    colXPtr = columns +  cellCornerPrm[0][1].x.highDig;
    /* draw line to right 						*/
	    MANH( cellCornerPrm[0][1],cellCornerPrm[1][1], md);
	    DRAW_LINE( cellCornerPrm[0][1].fy.highDig,yLine,PUT_C_POINT);
	}
    }	
}

int levelsNumber(aHistNode *aHist)
{
    int i1,i2;
    if (aHist->nextNodeOffset == 0) return (1);
    i1 = levelsNumber (aHist + 1);
    i2 = levelsNumber (aHist + aHist->nextNodeOffset);
    return ( i1 > i2 ? i1 + 1 : i2 + 1);
}



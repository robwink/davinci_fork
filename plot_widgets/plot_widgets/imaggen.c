/*******************************************************************************
*									       *
* imaggen.c -- 	2D histogramm bit image generator			       *
* 		for 2DHist widget 					       *
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
#include <X11/Xmd.h>
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#if XmVersion == 1002
#include <Xm/PrimitiveP.h>
#endif
#include <Xm/DrawingA.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <math.h>
/*?*/
#include <stdio.h>
#include "2DHist.h"
#include "2DGeom.h"
#include "2DHistDefs.h"
#include "uniLab.h"
#include "labels.h"		
#include "2DHistP.h"
#include "aimaggen.h"
#include "imaggen.h"

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
    

/*
    We use next representation of map from virtual 3D space to window 
    coordinates:
    
    map.x.x * virtualPoint.x * ly + map.x.y * virtualPoint.y * lx =
        windowPoint.x  * lx * ly + Rx
    0 <= Rx < lx * ly
    
    map.y.x * virtualPoint.x * l.y + map.y.y * virtualPoint.y * lx =
        windowPoint.y  * lx * ly + Ry
    0 <= Rx < lx * ly
    
    Rx = xqx * lx + xrx = xqy * ly + xry
    Ry = yqx * lx + yrx = yqy * ly + yry

    Above <map> is given matrix and lx & ly are lengths of visible part
    of histogram. Value of lx is <number of bins> * MAXPIC. So about ly
    An incremental technique is used below to calculate <windowPoints> which
    are mapped intersection points of bin edges and Z==0 plane.
    At the beginning two rows of points are calculated: one which satisfy to
    additional condition <virtualY> == <max visible virtual Y> - array
    XPoints and the second which satisfy to 
    <virtualX> == <max visuable virtual X> - array YPoints.
    Those points are situated on the nearmost edges of visible area of
    histogram. To use truncation instead of rounding we'll start with
    Rx=Ry=lx * ly /2
    
    To implement this representation mBaseNumber(multi-base number)
    is introduced below:
    
    structure mBaseNumber (multi-base number) is used to represent and 
    process rationsl numbers in an appropriative for
    algorithms way and precision.
    Actual value of a represented number is:
	VAL = ((highDig/factor) * firstBase * secondBase + 
           firstDig * secondBase + secondDig) / (firstBase * secondBase)
    valid representation should meet next conditions:
	0 <= secondDig < secondBase,
	0 <= firstDig < firstBase,
	highDig % factor == 0.
	Some notes about semantics. It is assumed that the represented
    rational number has denominator (firstBase * secondBase). In this
    case (highDig/factor) is just an integer part of the rational number.
    In the way we use this technique, the integer part is usually
    X or Y window coordinate of some point. Because of a bitmap processing,
    it's convinient to multiplay Y coordinate by the width of the bitmap
    (in bitmap units) to have a displacement to corresponding row of
    the bitmap instead of Y coordinate. So, factor is setted up to
    this width and highDig is desireable displacement for Y. It's
    possible to use factor==1 in other cases.
	Actually firstBase and secondBase will always be known in contex, i.e.,
    functions to add mBaseNumber(s) will be provided for numbers with the
    same firstBase and secondBase, but they will not perform any checks of
    that kind, firstBase and secondBase is assumed to be fixed "in the context". 
	So fields firstBase and secondBase are provided in the structure
    rather for convinience, than functionality.
	Finally note, that "digits" are numbered from most significant to
    less significant, and  the secondBase==1 => secondDig==0 - representation
    we will refer to as a "short" one.

    To inplement <windowPoint.x> from the equations

    map.x.x * virtualPoint.x * ly + map.x.y * virtualPoint.y * lx =
        windowPoint.x  * lx * ly + Rx
        
    0 <= Rx < lx * ly
    
    Rx = xqx * lx + xrx = xqy * ly + xry
    
    0<= xrx < lx , 0 <= xry < ly
    
    mBaseNumber  is used with
    
    	 factor     == 1
         highDig    == windowPoint.x,
         firstDig   == xqx
         secondDig  == xrx
         firstBase  == ly
         secondBase == lx
         
	or wiht bases changed:

    	 factor     == 1
         highDig    == windowPoint.x,
         firstDig   == xqy
         secondDig  == xry
         firstBase  == lx
         secondBase == ly
         
    Y case is similar but the difference is that
    
         factor     == <bitmap width> 
         highDig    == windowPoint.y * <bitmap width>

         
*/       
  
typedef struct _mBaseNumber {
    long int highDig;
    long int firstDig;
} mBaseNumber;

typedef struct _binPrm {
    mBaseNumber x;
    mBaseNumber y;
    mBaseNumber fx;
    mBaseNumber fy;
    int factor;
    int yOffset;
    int base;
    int goodBin;
    int dfx_dx;
    int dfx_dy;
    int dfy_dx;
    int dfy_dy;
    int ml;
} binPrm;

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



static columnDescr* makeColumnDescr(XImage *image);
static void mkShortMBRep(int number, int base, 
  mBaseNumber *mBNumber);
static void changeMBRep (mBaseNumber *mBNShort, int fBase1, int fBase2);
static void changeBinPrmBase (binPrm *bin, int newBase);
static void makeBinPrm(discreteMap *dMap, binPosDescr *binDescr, lineDescr *xLine,
    lineDescr *yLine, int width, binPrm **binStartArray, binPrm **dXArray);

static lineDescr *createLineDescr(void);
static void destroyLineDescr(lineDescr **lD);
static lineDescr *makeLineDescr(int a,int b, int width);
static void makeSimpleImage(Hist2DWidget w);
static void makeErrorImage(Hist2DWidget w);

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


/* mBaseNumber's arithmetics, part of functions are implemented as macros, 
   designed for this programm only, not complete arithmetics for general 
   purpose						
*/

/* make short representation of number 					
**	number - source number
**	base   - firstBase to use
**	mBNumber - pointer to mBaseNumber to construct
*/	
static void mkShortMBRep(int number, int base, 
  mBaseNumber *mBNumber)
{  
    mBNumber->highDig = number / base;
    mBNumber->firstDig = number - mBNumber -> highDig * base;
    if (mBNumber->firstDig< 0)
    {
        mBNumber->firstDig += base;
        mBNumber->highDig --;	
    }
}
/*
** convert a mBaseNumber representation in a short form (with secondBase == 1 
** secondDig == 0 ) and  to a mBaseNumber with the 
** 	mBNew->firstBase == fBase2,
** 	mBNew->secondBase == mBNShort->firstBase
** 	mBNew->factor == mBNShort->factor.
*/
static void changeMBRep (mBaseNumber *mBNShort, int fBase1, int fBase2)
{
/*  solve an equation
    mBNShort->firstDig * fBase2 = 
       mBNew->firstDig * mBNShort->firstBase + mBNew->secondDig
    where 0 <= mBNew->secondDig <= mBNew->secondBase == mBNShort->firstBase
    in a manner that avoid integer multiplication overlow 		*/
    
    unsigned  dh = mBNShort->firstDig >> LOWBITS; /* MAXPIC == 1 << LOWBITS 	*/ 
    unsigned  dl = mBNShort->firstDig %  MAXPIC;
    unsigned  firstDig;
    unsigned  secondDig;
    secondDig   = dh * fBase2;
    firstDig    = secondDig / fBase1;
    secondDig   = secondDig % fBase1;
    firstDig  <<= LOWBITS;
    secondDig <<= LOWBITS;
    firstDig   += secondDig / fBase1;
    secondDig  %= fBase1;
    firstDig   += (dl * fBase2) / fBase1;
    secondDig  += (dl * fBase2) % fBase1;
    if (secondDig >= fBase1 )
    {
        secondDig -= fBase1;
        firstDig++;

    }
    mBNShort->firstDig = firstDig;
}

static void changeBinPrmBase (binPrm *bin, int newBase)
{
     changeMBRep (&(bin->x), bin->base, newBase);
     changeMBRep (&(bin->y), bin->base, newBase);
     changeMBRep (&(bin->fx), bin->base, newBase);
     changeMBRep (&(bin->fy), bin->base, newBase);
     bin->base = newBase;
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

#define PLUS_N(MB_NUM_1, MB_NUM_2, BASE, OVF_ACTION)			\
{									\
    (MB_NUM_1).highDig += (MB_NUM_2).highDig;				\
    (MB_NUM_1).firstDig += (MB_NUM_2).firstDig;				\
    if ((MB_NUM_1).firstDig >= BASE )					\
    {									\
        (MB_NUM_1).firstDig -=  BASE;					\
        (MB_NUM_1).highDig ++;						\
        { OVF_ACTION }							\
    }									\
}

/*
** Same kind of macro, but for binPrm.
** Two OVF_ACTION should be provided for each of coordinates
*/
#define PLUS_B(BIN_1, BIN_2 )						\
{									\
    PLUS_N (BIN_1.x, BIN_2.x, BIN_1.base,				\
        {BIN_1.fx.highDig -= BIN_1.dfx_dx; 				\
         BIN_1.fy.highDig -= BIN_1.dfy_dx;})				\
    PLUS_N (BIN_1.y, BIN_2.y, BIN_1.base,				\
        {BIN_1.fx.highDig -= BIN_1.dfx_dy;				\
         BIN_1.fy.highDig -= BIN_1.dfy_dy;				\
         BIN_1.yOffset += BIN_1.factor;})				\
    PLUS_N (BIN_1.fx, BIN_2.fx, BIN_1.base,{}) 				\
    PLUS_N (BIN_1.fy, BIN_2.fy, BIN_1.base,{})				\
    BIN_1.yOffset += BIN_2.yOffset;					\
}
/*
** Manhattan length between BIN and BIN + DELTA (bin start/vertex points)
** for the case increment is (negative) and along virtual X axis
*/
#define MANH_X_DX(BIN, DELTA, ML)						\
{									\
     ML = DELTA->ml;							\
     if (BIN->x.firstDig + DELTA->x.firstDig >= BIN->base) ML++;	\
     if (BIN->y.firstDig + DELTA->y.firstDig >= BIN->base) ML--;	\
}
/*
** Manhattan length between BIN_1 and BIN_2 (bin start/vertex points)
** for the case increment is (negative) and along virtual Y axis
*/
#define MANH_Y(BIN_1, BIN_2, ML)						\
{									\
     ML = BIN_1->x.highDig + BIN_1->y.highDig - 			\
     BIN_2->x.highDig - BIN_2->y.highDig;				\
}

static void makeBinPrm(discreteMap *dMap, binPosDescr *binDescr, lineDescr *xLine,
    lineDescr *yLine, int width, binPrm **binStartArray, binPrm **dXArray)
{
    binPrm *pPtr;
    binPrm *bPtr;
    int *delta;
    int i;
    matrix2 map;
    map = dMap->map;
    MYMALLOC (binPrm, *dXArray, binDescr->nXPoints * 2 - 1);
    MYMALLOC (binPrm, *binStartArray, binDescr->nYPoints *2);
    for ( i=0, pPtr = *dXArray, delta = binDescr -> xSteps;
      i < binDescr->nXPoints * 2 - 1; i++, delta++, pPtr++)
    {
        pPtr->dfx_dx =   xLine -> a;
        pPtr->dfx_dy = - xLine -> b;
        pPtr->dfy_dx =   yLine -> a;
        pPtr->dfy_dy = - yLine -> b;
 	mkShortMBRep(-*delta * map.x.x * 2, binDescr->lx * 2, &(pPtr->x));
 	mkShortMBRep(-*delta * map.y.x * 2, binDescr->lx * 2, &(pPtr->y));
 	mkShortMBRep(pPtr->x.firstDig * pPtr->dfx_dx + pPtr->y.firstDig * pPtr->dfx_dy, 
 	    2 * binDescr->lx, &(pPtr->fx));
        mkShortMBRep(pPtr->x.firstDig * pPtr->dfy_dx + pPtr->y.firstDig * pPtr->dfy_dy, 
 	    2 * binDescr->lx, &(pPtr->fy));
 	pPtr->base = 2 * binDescr->lx;
 	pPtr->factor = width;
 	pPtr->yOffset = pPtr->y.highDig * pPtr->factor;
 	pPtr->goodBin = *delta;
 	pPtr->ml = abs(pPtr->x.highDig) + abs(pPtr->y.highDig);
    }
    for ( i=0, pPtr = (*binStartArray)+1, delta = binDescr -> ySteps;
      i < binDescr->nYPoints * 2 - 1; i++, delta++, pPtr++)
    {
        pPtr->dfx_dx =   xLine -> a;
        pPtr->dfx_dy = - xLine -> b;
        pPtr->dfy_dx =   yLine -> a;
        pPtr->dfy_dy = - yLine -> b;
 	mkShortMBRep(-*delta * map.x.y * 2, binDescr->ly * 2, &(pPtr->x));
 	mkShortMBRep(-*delta * map.y.y * 2, binDescr->ly * 2, &(pPtr->y));
 	mkShortMBRep(pPtr->x.firstDig * pPtr->dfx_dx + pPtr->y.firstDig * pPtr->dfx_dy, 
 	    2 * binDescr->ly, &(pPtr->fx));
        mkShortMBRep(pPtr->x.firstDig * pPtr->dfy_dx + pPtr->y.firstDig * pPtr->dfy_dy, 
 	    2 * binDescr->ly, &(pPtr->fy));
 	pPtr->base = 2 * binDescr->ly;
 	pPtr->factor = width;
 	pPtr->yOffset = pPtr->y.highDig * pPtr->factor;
 	pPtr->goodBin = *delta;
 	pPtr->ml = abs(pPtr->x.highDig) + abs(pPtr->y.highDig);
    } 
    bPtr = *binStartArray;
    mkShortMBRep((2 * dMap->vertex.x + 1) * binDescr->ly, 
      binDescr->ly * 2, &(bPtr->x));
    mkShortMBRep((2 * dMap->vertex.y + 1) * binDescr->ly, 
      binDescr->ly * 2, &(bPtr->y));
    mkShortMBRep(xLine->f2Offset * binDescr->ly, 
      binDescr->ly * 2, &(bPtr->fx));
    mkShortMBRep(yLine->f2Offset * binDescr->ly, 
      binDescr->ly * 2, &(bPtr->fy));
    bPtr->factor = width;
    bPtr->yOffset = bPtr->y.highDig * bPtr->factor;
    bPtr->dfx_dx =   xLine -> a;
    bPtr->dfx_dy = - xLine -> b;
    bPtr->dfy_dx =   yLine -> a;
    bPtr->dfy_dy = - yLine -> b;
    bPtr->base = 2 * binDescr->ly;
    /* ml is not used in binStartArray, just to init */
    bPtr->ml = 0; 
    for ( i=0, delta = binDescr -> ySteps;
      i < binDescr->nYPoints * 2 - 1; i++, delta++, bPtr++)
    {
        PLUS_B( (*(bPtr+1)),(*bPtr));
        changeBinPrmBase(bPtr, binDescr->lx * 2);
    }
    changeBinPrmBase(bPtr, binDescr->lx * 2);
}

void makeBinPosDescr (range2d r2, discreteMap *dMap, 
    int binWidth, binPosDescr *binDescr)
    
{
    vector v0;
    int* itmp;
    int i;
    int lM, rM;
    matrix2 map;
    Boolean inBin;
    map = dMap->map;
/* calculate number of points */
    r2=transformRange2d(r2,dMap->rotMatr);
    v0=r2.start;
    if (v0.x >= 0 )
	v0.x /= MAXPIC;
    else 
    {
	v0.x = -v0.x;
	v0.x += MAXPIC - 1;
	v0.x /= MAXPIC;
	v0.x = - v0.x;
    }
    if (v0.y >= 0 )
	v0.y /= MAXPIC;
    else 
    {
	v0.y = -v0.y;
	v0.y += MAXPIC - 1;
	v0.y /= MAXPIC;
	v0.y = - v0.y; 
    }
    r2 = translateRange2d (r2, mulVector (v0, -MAXPIC));
    binDescr->nXPoints =  (r2.end.x + MAXPIC - 1) / MAXPIC + 1;
    binDescr->nYPoints =  (r2.end.y + MAXPIC - 1) / MAXPIC + 1;
    MYMALLOC(int,(binDescr->xSteps),(2 * binDescr->nXPoints - 1));
    MYMALLOC(int,(binDescr->ySteps),(2 * binDescr->nYPoints - 1));
/* calculate bin margins */  
    lM = (MAXPIC - binWidth ) /2;
    rM = MAXPIC - binWidth - lM;
    itmp=binDescr->xSteps;
    inBin = TRUE;
    i = MAXPIC * (binDescr->nXPoints - 1)-rM;
    while (i > r2.end.x)
    {
       *itmp++ = 0;
       if (inBin) i -= binWidth;
       else i -= (lM + rM);
       inBin = ! inBin;
    }
    if (i <= r2.start.x)
    {
        *itmp++ = r2.end.x - r2.start.x;
        if (inBin)
        {
            if (i > 0)        /* did not do to a new cell */
            {
                *itmp++ = 0;
                *itmp   = 0;
            }
        }
        else 
        {
            *itmp = 0;
        }
    }
    else 
    {
        *itmp++ = r2.end.x - i;
	if (inBin) i -= binWidth;
	else i -= (lM + rM);
	inBin = ! inBin;
        while (i > r2.start.x)
        {
            if (inBin) 
            {
            	*itmp++ = lM + rM;
            	i -= binWidth;
            }
            else
            {
            	*itmp++ = binWidth;
            	i -= lM + rM;
            }
            inBin = ! inBin;
      	}
      	if (inBin)
     	{ 	    
	    *itmp++ = lM + rM  - r2.start.x + i; 
            if (i > 0)        /* did not do to a new cell */
            {
                *itmp++ = 0;
                *itmp   = 0;
            }
	}
	else 
	{   
	    *itmp++ = binWidth  - r2.start.x + i;
	    *itmp =0;
	}
    }
    
    itmp=binDescr->ySteps;
    inBin = TRUE;
    i = MAXPIC * (binDescr->nYPoints - 1) - rM;
    while (i > r2.end.y)
    {
       *itmp++ = 0;
       if (inBin) i -= binWidth;
       else i -= (lM + rM);
       inBin = ! inBin;
    }
    if (i <= r2.start.y)
    {
        *itmp++ = r2.end.y - r2.start.y;
        if (inBin)
        {
            if (i > 0)        /* did not do to a new cell */
            {
                *itmp++ = 0;
                *itmp   = 0;
            }
        }
        else 
        {
            *itmp = 0;
        }
    }
    else 
    {
        *itmp++ = r2.end.y - i;
	if (inBin) i -= binWidth;
	else i -= (lM + rM);
	inBin = ! inBin;
        while (i > r2.start.y)
        {
            if (inBin) 
            {
            	*itmp++ = lM + rM;
            	i -= binWidth;
            }
            else
            {
            	*itmp++ = binWidth;
            	i -= lM + rM;
            }
            inBin = ! inBin;
      	}
      	if (inBin)
     	{ 	    
	    *itmp++ = lM + rM  - r2.start.y + i; 
            if (i > 0)        /* did not do to a new cell */
            {
                *itmp++ = 0;
                *itmp   = 0;
            }
	}
	else 
	{   
	    *itmp++ = binWidth  - r2.start.y + i;
	    *itmp =0;
	}
    }
    binDescr->lx = r2.end.x - r2.start.x;
    binDescr->ly = r2.end.y - r2.start.y;
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

void createImageData (XImage *image, Hist2DWidget w)
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

/*
** discreteData * rescaleZ(float *fdata, int nYBins, discreteMap *dMap, 
**		range2d xyVisibleRange, float zVisibleStart,
**		float zVisibleEnd, Boolean zLogScaling, int width)
**
**	data	pointer to data array
**
**	nYBins	row length of data array - in our case this is
**		number of bins in the histogram along Y axis
**
**	dMap	discrete map to map the histogram into the window
**
**      xyVisibleRange
**		visible range of the histogram in XY plane 
**	
**	zVisibleStart
**		start of visible range along Z axis
**	
**	zVisibleEnd
**		end of visible range along Z axis
** 
**	zLogScaling
**		use log or linear scaling for data
**	
**	width	width of the bitmap to be generated with descrete data.
**		This parameter determine size of increment in discrete
**		data between neighbor rows of bitmap
**		To get just scaled data (i.e. for PostScript file)
**		width obviously should be setted up to 1
**
**	The function creates scaled discrete data for visible part of the
**	histogram according to parameters given, performing clipping if 
**	needed. 
**	The X (Y) size of discrete data array is number of bins in the visible
** 	range along X (Y) axis plus two. The function adds two rows and two
**	columns - first and last - filled with zero (unclipped) data.
*/

discreteData * rescaleZ(float *fdata, int nYBins, discreteMap *dMap, 
		range2d xyVisibleRange, float zVisibleStart,
		float zVisibleEnd, Boolean zLogScaling, int width)
{
    discreteData *data, *tmp;
    int 		i,j;        
    float 		*fStart, *fRow, *fTmp; 
    range2d  r;
    vector v0, v;
    vector dd; 
    int nXPoints, nYPoints;    
    if (fdata==NULL)
 	return(NULL);
/* Calculate index of coner bin in data array.				*/
    r=transformRange2d(xyVisibleRange,dMap->rotMatr);
    v0=r.start;
    if (v0.x >= 0 )
	v0.x /= MAXPIC;
    else 
    {
	v0.x = -v0.x;
	v0.x -=  1;	
	v0.x /= MAXPIC;
	v0.x = - v0.x;
    }
    if (v0.y >= 0 )
	v0.y /= MAXPIC;
    else 
    {
	v0.y = -v0.y;
	v0.y -=  1;
	v0.y /= MAXPIC;
	v0.y = - v0.y;
    }
    v0 = matrixMultVector(transposeMatrix(dMap->rotMatr),v0);
/* indexes of bin wich is mapped to farmost bin on the picture is calculated */
    fStart = fdata + v0.y + v0.x * nYBins;
/* data pointer increments cooresponding to one bin step in X & Y in source
** coordinates								     */    
    dd=matrixMultVector(dMap->rotMatr, makeVector( nYBins,1));
/* calculate number of bin edges (actually number of bins + 1) crossing
** X & Y axes in the xyVisibleRange					     */       
    v=r.start;
    if (v.x >= 0 )
	v.x /= MAXPIC;
    else 
    {
	v.x = -v.x;
	v.x +=  MAXPIC - 1;	
	v.x /= MAXPIC;
	v.x = - v.x;
    }
    if (v.y >= 0 )
	v.y /= MAXPIC;
    else 
    {
	v.y = -v.y;
	v.y +=  MAXPIC - 1;
	v.y /= MAXPIC;
	v.y = - v.y;
    }
    r = translateRange2d (r, mulVector (v, -MAXPIC));
    nXPoints =  (r.end.x + MAXPIC - 1) / MAXPIC + 1;
    nYPoints =  (r.end.y + MAXPIC - 1) / MAXPIC + 1;
    
/* make a symmetry transformation to provide output data in most convinient 
** order for scanning 							*/
    fStart += (nYPoints - 2 ) * dd.y + (nXPoints - 2 ) * dd.x;
    dd.x = -dd.x;
    dd.y = -dd.y;
/* allocate memory for generating discrete data				*/    
    MYMALLOC(discreteData, data, (nXPoints - 1) * (nYPoints - 1));
    tmp=data;
    fRow = fStart;
    for (i=1; i < nXPoints; i++)
    {
	if (! zLogScaling)
	{
	    double linearScale = (double) (dMap->zFactor) / 
	      ( zVisibleEnd - zVisibleStart);
	    fTmp = fRow;
            for (j=1; j < nYPoints; j++ )
 	    {
    		if (*fTmp < zVisibleStart) 
    		{
      		    tmp->data=0;
        	    tmp->clipped=TRUE;
        	}
        	else if (*fTmp > zVisibleEnd)
        	{
        	    tmp->data = dMap->zFactor;				            	
        	    tmp->clipped = TRUE;
        	}
        	else
        	{
        	    tmp->data = ((double)*fTmp - (double) zVisibleStart) *
        	       linearScale;
    /* correction by 1 for possible rounding indefenite		*/
        	    if (tmp->data < dMap->zFactor) tmp->data = dMap->zFactor;
        	    tmp->clipped=FALSE;
        	}
        	fTmp += dd.y;
	        tmp->data *= width;
	        tmp++;
	    }
	}
        else
     	{
	    double logScale = (double) (dMap->zFactor) / 
	      (log10( zVisibleEnd ) -
	      log10( zVisibleStart ));
	    double logStart = log10((double) zVisibleStart);
	    fTmp = fRow;
            for (j=1; j < nYPoints; j++ )
 	    {
    		if (*fTmp < zVisibleStart) 
    		{
      		    tmp->data=0;
        	    tmp->clipped=TRUE;
        	}
        	else if (*fTmp > zVisibleEnd)
        	{
        	    tmp->data = dMap->zFactor;				            	
        	    tmp->clipped = TRUE;
        	}
        	else
        	{
        	    tmp->data = (log10((double)*fTmp) - logStart) * logScale; 
    /* correction by 1 for possible rounding indefenite		*/
        	    if (tmp->data < dMap->zFactor) tmp->data = dMap->zFactor;
        	    tmp->clipped=FALSE;
        	}
        	fTmp += dd.y;
	        tmp->data *= width;
	        tmp++;
	    }
	}
       	fRow += dd.x;
    }
    return (data);
}









/* next macros are valid in tne context of makeImage function only */
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

void makeImage(Hist2DWidget w)
{
    if ((w->hist2D.discrMap==NULL)|| 
      (w->hist2D.bins==NULL && w->hist2D.aHist == NULL))
    {
        if (w->hist2D.wireImage==NULL)
	    return;
        MYFREE(w->hist2D.wireImage->data);
        w->hist2D.wireImage->data=NULL;
        return;
    }
    if (w->hist2D.aHist != NULL)
        makeAHistImage(w);
    else if (w->hist2D.errorBarsOn && (w->hist2D.bottomErrors !=NULL ||
      w->hist2D.topErrors !=NULL))
        makeErrorImage(w);
    else
        makeSimpleImage(w);
}        

static void makeSimpleImage(Hist2DWidget w)
{
    XImage *image;
    discreteMap *discrMap;
    matrix2 map;
    vector vertex ;
    int i,j;
    binPosDescr binPositions;	/* bin positions in virtual coordinates    */
    int width;			/* width of the bitmap (in 32 bit units)   */
    columnDescr *columns;	/* array of elements corresponding to each
    				   column of the bitmap. Elemetns contain
    				   floating horizont position, offset in
    				   the row (in bitmap units) and bit mask
    				   (pattern) for given column              */
    int zPtrDy ;
    int zPtrDx ;
    lineDescr *xLine;
    lineDescr *yLine;
    discreteData *data;		/* scaled data				   */
    int delta;
    int zColumn;		/* index of current data row		   */
    int zIdx;			/* index of current data                   */
    int md;
 
    fIndex *fIdx;
    lineStepDescr *linePtr;
    
    binPrm *dXArray;
    binPrm *binStartArray;	/* bins origin position in window coordinates
    				   and other pareameters to generate a bin */
    binPrm leftBaseVertex;    				   
    binPrm dXFull;

    binPrm *dXPtr;			/* pointer to sructure,representing
    					   bin width along virtual X */
    binPrm *binPtr;			/* pointer to      
    					   sructure, representing bin start */
    CARD32 *rowYPtr;			/* pointer to Y row in the bitmap   */
    columnDescr *colXPtr;		/* poiter to current X column in
    					   columnDescr structure	    */
    CARD32 dotMask;    					   
    int f0;
        
    image=w->hist2D.wireImage;
    discrMap=w->hist2D.discrMap;
    vertex  = w->hist2D.discrMap->vertex ;
    map = discrMap->map;
    createImageData(image,w);
    width = image->bytes_per_line / 4;
    columns= makeColumnDescr ( image);
    data = rescaleZ(w->hist2D.bins,w->hist2D.sourceHistogram->nYBins, 
      w->hist2D.discrMap, w->hist2D.xyVisibleRange, w->hist2D.zVisibleStart,
      w->hist2D.zVisibleEnd, w->hist2D.zLogScaling, width);
    yLine = makeLineDescr (map.y.x, map.x.x, width);
    xLine = makeLineDescr (map.y.y, map.x.y, width);
    makeBinPosDescr (w->hist2D.xyVisibleRange, discrMap, MAIN_BIN_WIDTH, 
      &binPositions);  
    makeBinPrm(discrMap, &binPositions,  xLine,
      yLine,  width, &binStartArray, &dXArray);
    { /* init floating horizont and draw axes  */
	dXFull = *dXArray;
	for (i=1, dXPtr = dXArray + 1; i < binPositions.nXPoints * 2 - 1;
	   i++, dXPtr++)
	{
            PLUS_B (dXFull, (*dXPtr));
	}    
 	dXFull.ml = abs(dXFull.x.highDig) + 
 	  abs(dXFull.y.highDig);
	binPtr = binStartArray;
	rowYPtr = ((CARD32*)image->data) + binPtr->yOffset;
	colXPtr = columns + binPtr->x.highDig;
        PUT_POINT_HOR(colXPtr,rowYPtr);
	MANH_X_DX(binPtr,(&dXFull), md);
	DRAW_LINE(binPtr->fy.highDig,yLine,PUT_POINT_HOR);
	rowYPtr = ((CARD32*)image->data) + binPtr->yOffset;
	colXPtr = columns + binPtr->x.highDig;
	MANH_Y(binPtr,(binPtr + binPositions.nYPoints * 2 - 1), md);
	DRAW_LINE(binPtr->fx.highDig,xLine,PUT_POINT_HOR);

	/* draw left point 					*/
	    rowYPtr = (CARD32*)image->data + 
	      ( binPtr + binPositions.nYPoints * 2 - 1)->yOffset;
	    colXPtr = columns + 
	      ( binPtr + binPositions.nYPoints * 2 - 1)->x.highDig;
	    PUT_POINT_HOR(colXPtr, (rowYPtr));
	/* draw right  point.
	   <binPtr> parameters still will be needed later.
	   So, they are not updated at this moment
	   to next vertex by macro <PLUS_B>, insted start parameters
	   to draw this vertical segment calculated directly.	   	*/
	   rowYPtr = (CARD32*)image->data + binPtr->yOffset +
	      dXFull.yOffset;
	   if (binPtr->y.firstDig + dXFull.y.firstDig >= binPtr->base)
	       rowYPtr += binPtr->factor;
	   colXPtr = columns + binPtr->x.highDig + dXFull.x.highDig;
	   if (binPtr->x.firstDig + dXFull.x.firstDig >= binPtr->base)
	       colXPtr++; 
	   PUT_POINT_HOR(colXPtr, rowYPtr);
    }	
/* save leftBaseVertex descriptor to draw backplanes later           */    
    leftBaseVertex = binPtr[binPositions.nYPoints * 2 - 1];
#if MAIN_BIN_WIDTH - MAXPIC
    rowYPtr = (CARD32*)image->data +  
      (binPtr + binPositions.nYPoints * 2 - 1)->yOffset ;
    colXPtr = columns +  (binPtr + binPositions.nYPoints * 2 - 1)->x.highDig;
    MANH_X_DX( (binPtr + binPositions.nYPoints * 2 - 1), dXArray, md); 
    DRAW_LINE( (binPtr + binPositions.nYPoints * 2 - 1)->fy.highDig,
      yLine,PUT_C_POINT);
#endif
    for (i=0, binPtr = binStartArray;
      i < binPositions.nYPoints * 2; i++, binPtr++ )
    {
        PLUS_B((*binPtr), (*dXArray));
    }   
    zPtrDy = 1;
    zPtrDx = binPositions.nYPoints - 1;
    zColumn = 0;
    dXPtr = dXArray + 1;
    dotMask = 0;
    for (i=0; i< binPositions.nXPoints -1; i++)
    {
	binPtr = binStartArray;
	PLUS_B((*binPtr), (*dXPtr));
	PLUS_B((*binPtr), (*(dXPtr+1)));
	binPtr++;
#if MAIN_BIN_WIDTH - MAXPIC
            if(i == binPositions.nXPoints - 2)
            {
                binPrm bTmp;
/* draw part of right far base edge while drawing last raw of bins  */            
	        rowYPtr = (CARD32*)image->data + (binPtr -1) ->yOffset ;
	        colXPtr = columns +  (binPtr -1)->x.highDig;
	        bTmp = *binPtr;
	        PLUS_B (bTmp, (*(dXPtr)));
   	    	PLUS_B (bTmp, (*(dXPtr+1)));
	        MANH_Y( (binPtr-1),( &bTmp), md);
	        DRAW_LINE( (binPtr-1)->fx.highDig,xLine,PUT_C_POINT);
	    }
#endif
	zIdx = zColumn;
	zColumn += zPtrDx;
	for (j =0; j < binPositions.nYPoints - 1 ; j++)
	{
	    if((binPtr+1)->goodBin != 0 && (dXPtr)->goodBin != 0)
	    {
	/* the elements drawing sequence is essential othervise
	   some elements might "shadow" other by floating
	   horizont in a wrong way.					*/
#if MAIN_BIN_WIDTH - MAXPIC
/* draw the base of main bin in this case				*/
	    rowYPtr = (CARD32*)image->data +  binPtr->yOffset;
	    colXPtr = columns +  binPtr->x.highDig;
	    PUT_C_POINT_HOR(colXPtr,rowYPtr);
	/* draw line to left 						*/
	    MANH_Y( binPtr,( binPtr+1), md);
	    DRAW_LINE( binPtr->fx.highDig,xLine,PUT_C_POINT_HOR);
	/* draw line to right 						*/
	    rowYPtr = (CARD32*)image->data +  binPtr->yOffset ;
	    colXPtr = columns +  binPtr->x.highDig;
	    MANH_X_DX( binPtr, dXPtr, md); 
	    DRAW_LINE( binPtr->fy.highDig,yLine,PUT_C_POINT_HOR);
	/* draw left point 					*/
	    rowYPtr = (CARD32*)image->data + ( binPtr+1)->yOffset;
	    colXPtr = columns + ( binPtr+1)->x.highDig;
	    PUT_C_POINT_HOR(colXPtr, (rowYPtr));
	/* draw right  point.
	   <binPtr> parameters still will be needed later to draw
	   top error bar. So, they are not updated at this moment
	   to next bin vertex by macro <PLUS_B>, insted start parameters
	   to draw this vertical segment calculated directly.	   	*/
	   rowYPtr = (CARD32*)image->data + binPtr->yOffset +
	      dXPtr->yOffset;
	   if (binPtr->y.firstDig + dXPtr->y.firstDig >= binPtr->base)
	       rowYPtr += binPtr->factor;
	   colXPtr = columns + binPtr->x.highDig + dXPtr->x.highDig;
	   if (binPtr->x.firstDig + dXPtr->x.firstDig >= binPtr->base)
	       colXPtr++; 
	   PUT_C_POINT_HOR(colXPtr, rowYPtr);
#endif        
	     
	/* draw fron part of main bar now				*/
        /* setup pointers to the nearmost vertex 		*/ 
	    rowYPtr = (CARD32*)image->data + binPtr->yOffset + data[zIdx].data;
	    colXPtr = columns + binPtr->x.highDig;
        /* draw central vertical edge 				*/    
            DRAW_V_LINE_HOR(colXPtr,rowYPtr);
	/* draw left vertical edge, using corresponding vertex descriptor 
	   (macro DRAW_V_LINE body refers its paramters only  once) */ 
	    DRAW_V_LINE_HOR(columns + (binPtr+1)->x.highDig, 
	      (CARD32*)image->data + (binPtr+1)->yOffset + data[zIdx].data);
	/* draw line to left 					*/
	    MANH_Y(binPtr,(binPtr+1), md);
	    DRAW_LINE(binPtr->fx.highDig,xLine,PUT_C_POINT_HOR);
	/* move binDescr to next vertex	and draw vertical and top right	edges */
	    rowYPtr = (CARD32*)image->data + binPtr->yOffset + data[zIdx].data;
	    colXPtr = columns + binPtr->x.highDig;
	    MANH_X_DX(binPtr,dXPtr, md); 
	    f0 = binPtr->fy.highDig;
	    PLUS_B ((*binPtr), (*dXPtr));
	    DRAW_V_LINE_HOR(columns + binPtr->x.highDig, 
	      (CARD32*)image->data + binPtr->yOffset + data[zIdx].data);
	    DRAW_LINE(f0,yLine,PUT_C_POINT_HOR);
	/* draw  top edge of the bin. The order is essential, and
	    next <if> is better to move outside the loop. (Let optimaser
	    to do this if it can)				*/
	    if (map.x.y < map.y.y)
	    {
	        int x0, y0, f0, md0;
	        binPtr++;
	        x0 = binPtr->x.highDig;
	        y0 = binPtr->yOffset;
	        f0 = binPtr->fy.highDig;
	        MANH_X_DX(binPtr,dXPtr, md0);
   	        PLUS_B ((*binPtr), (*dXPtr));
   	        binPtr--;
 	 	rowYPtr = (CARD32*)image->data + binPtr->yOffset + data[zIdx].data;
	        colXPtr = columns + binPtr->x.highDig;
	        MANH_Y(binPtr,(binPtr+1), md);
		if (data[zIdx].clipped)
		{
	            DRAW_LINE(binPtr->fx.highDig,xLine,DRAW_V_LINE_HOR);
		}
		else 
		{
	            DRAW_LINE(binPtr->fx.highDig,xLine,PUT_C_POINT_HOR);
		}
 	 	rowYPtr = (CARD32*)image->data + y0 + data[zIdx].data;
	        colXPtr = columns + x0;
	        md = md0; 
		if (data[zIdx].clipped)
		{
  	            DRAW_LINE(f0,yLine,DRAW_V_LINE_HOR);
		}
		else 
		{
  	            DRAW_LINE(f0,yLine,PUT_C_POINT_HOR);
		}
  	    } else
  	    {
  	        binPtr++;
  	        rowYPtr = (CARD32*)image->data + binPtr->yOffset + 
  	            data[zIdx].data;
	        colXPtr = columns + binPtr->x.highDig;
	        MANH_X_DX(binPtr,dXPtr, md); 
		if (data[zIdx].clipped)
		{
  	            DRAW_LINE(binPtr->fy.highDig,yLine,DRAW_V_LINE_HOR);
		}
		else 
		{
  	            DRAW_LINE(binPtr->fy.highDig,yLine,PUT_C_POINT_HOR);
		}
   	        PLUS_B ((*binPtr), (*dXPtr));
   	        binPtr--;
  	        rowYPtr = (CARD32*)image->data + binPtr->yOffset + data[zIdx].data;
	        colXPtr = columns + binPtr->x.highDig;
	        MANH_Y(binPtr,(binPtr+1), md);
		if (data[zIdx].clipped)
		{
  	            DRAW_LINE(binPtr->fx.highDig,xLine,DRAW_V_LINE_HOR);
		}
		else 
		{
  	            DRAW_LINE(binPtr->fx.highDig,xLine,PUT_C_POINT_HOR);
		}
	    }
	    }
	    else
	    {
   		PLUS_B ((*binPtr), (*dXPtr));
        	binPtr ++;
   		PLUS_B ((*binPtr), (*dXPtr));
   		binPtr --;
   	    }
	        
   	    PLUS_B ((*binPtr), (*(dXPtr+1)));
            binPtr ++;
   	    PLUS_B ((*binPtr), (*(dXPtr+1)));
   	    binPtr ++;
            zIdx += zPtrDy;
#if MAIN_BIN_WIDTH - MAXPIC
            if(i == binPositions.nXPoints - 2)
            {
                binPrm bTmp;
/* draw part of right far base edge while drawing last raw of bins  */            
	        rowYPtr = (CARD32*)image->data + (binPtr -2) ->yOffset ;
	        colXPtr = columns +  (binPtr -2)->x.highDig;
	        bTmp = *binPtr;
	        PLUS_B (bTmp, (*(dXPtr)));
   	    	PLUS_B (bTmp, (*(dXPtr+1)));
	        MANH_Y( (binPtr-2),( &bTmp), md);
	        DRAW_LINE( (binPtr-2)->fx.highDig,xLine,PUT_C_POINT);
	    }
#endif
        }
#if MAIN_BIN_WIDTH - MAXPIC
	rowYPtr = (CARD32*)image->data + binPtr->yOffset ;
	colXPtr = columns +  binPtr->x.highDig;
	MANH_X_DX( (binPtr), dXPtr, md); 
	DRAW_LINE( binPtr->fy.highDig,yLine,PUT_C_POINT);
#endif
	PLUS_B((*binPtr), (*dXPtr));
#if MAIN_BIN_WIDTH - MAXPIC
	rowYPtr = (CARD32*)image->data + binPtr->yOffset ;
	colXPtr = columns +  binPtr->x.highDig;
	MANH_X_DX( (binPtr), (dXPtr+1), md); 
	DRAW_LINE( binPtr->fy.highDig,yLine,PUT_C_POINT);
#endif
	PLUS_B((*binPtr), (*(dXPtr+1)));
        dXPtr += 2;
    }
    if (w->hist2D.backPlanesOn)
    {
        int level;
        labelsToDraw *lT = w->hist2D.currentPicture->zLabels;
        level = vertex.y - map.y.y + discrMap->zFactor;
        binPtr=binStartArray;
	do
        {
/* draw line on the right backplane				*/        
            dotMask = 0;
	    rowYPtr = ((CARD32*)image->data) + binPtr->yOffset + 
	      (level - vertex.y + map.y.y) * width;
	    colXPtr = columns + binPtr->x.highDig;
	    MANH_Y(binPtr,(binPtr + binPositions.nYPoints * 2 - 1), md);
	    DRAW_LINE(binPtr->fx.highDig,xLine,PUT_C_POINT_DOT);
/* draw line on the left backplane				*/        
            dotMask = 0;
	    rowYPtr = ((CARD32*)image->data) + leftBaseVertex.yOffset + 
	      (level - vertex.y + map.y.y) * width;
	    colXPtr = columns + leftBaseVertex.x.highDig;
	    MANH_X_DX((&leftBaseVertex),(&dXFull), md);
	    DRAW_LINE(leftBaseVertex.fy.highDig,yLine,PUT_C_POINT_DOT);
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
        rowYPtr = ((CARD32*)image->data) + binPtr->yOffset + 
          discrMap->zFactor * width; 
	colXPtr = columns + binPtr->x.highDig;
/* lift to upper cube vertex   */	
/* and draw vertical dotted line				*/	
        DRAW_V_LINE_DOT(colXPtr,rowYPtr);
/* set pointers to farmost base vertex and lift to upper cube vertex   */
        binPtr += binPositions.nYPoints * 2 - 1;
	rowYPtr = ((CARD32*)image->data) + binPtr->yOffset;
	colXPtr = columns + binPtr->x.highDig;
	PUT_C_POINT_HOR(colXPtr,rowYPtr);
	rowYPtr += discrMap->zFactor * width;
/* and draw vertical dotted line				*/	
        DRAW_V_LINE_DOT(colXPtr,rowYPtr);
/* not drawing left vertical - Z axis is there anyway		*/
    }
    destroyLineDescr (&xLine);
    destroyLineDescr (&yLine);
    MYFREE(binPositions.xSteps);
    MYFREE(binPositions.ySteps);
    MYFREE(dXArray);
    MYFREE(binStartArray);    
    MYFREE(data);
    MYFREE(columns);
    
}

static void makeErrorImage(Hist2DWidget w)
{
    XImage *image;
    discreteMap *discrMap;
    matrix2 map;
    vector vertex ;
    int i,j;
    binPosDescr binPositions;	/* bin positions in virtual coordinates    */
    binPosDescr er_binPositions;	/* bin positions in virtual coordinates    */
    int width;			/* width of the bitmap (in 32 bit units)   */
    columnDescr *columns;	/* array of elements corresponding to each
    				   column of the bitmap. Elemetns contain
    				   floating horizont position, offset in
    				   the row (in bitmap units) and bit mask
    				   (pattern) for given column              */
    int zPtrDy ;
    int zPtrDx ;
    lineDescr *xLine;
    lineDescr *yLine;
    discreteData *data;		/* scaled data				   */
    discreteData *topErrors;    /* scaled top error			   */
    discreteData *bottomErrors; /* scaled bottom error			   */
    int delta;
    int zColumn;		/* index of current data row		   */
    int zIdx;			/* index of current data                   */
    int md;
    int errorBinWidth = ERROR_BIN_WIDTH;
 
    fIndex *fIdx;
    lineStepDescr *linePtr;
    
    binPrm *dXArray;
    binPrm *binStartArray;	/* bins origin position in window coordinates
    				   and other pareameters to generate a bin */
    binPrm leftBaseVertex;    				   
    binPrm dXFull;
    binPrm *er_dXArray;
    binPrm *er_binStartArray;	/* bins origin position in window coordinates
    				   and other pareameters to generate a bin */


    binPrm *dXPtr;			/* pointer to sructure,representing
    					   bin width along virtual X */
    binPrm *binPtr;			/* pointer to      
    					   sructure, representing bin start */
/* the same for error bins						*/
    binPrm *er_dXPtr;			/* pointer to sructure,representing
    					   bin width along virtual X */
    binPrm *er_binPtr;			/* pointer to      
    					   sructure, representing bin start */
    CARD32 *rowYPtr;			/* pointer to Y row in the bitmap   */
    columnDescr *colXPtr;		/* poiter to current X column in
    					   columnDescr structure	    */
    CARD32 dotMask;    					   
    int f0;
        
    if ((w->hist2D.discrMap==NULL)|| (w->hist2D.bins==NULL))
    {
        if (w->hist2D.wireImage==NULL)
	    return;
        MYFREE(w->hist2D.wireImage->data);
        w->hist2D.wireImage->data=NULL;
        return;
    }
    image=w->hist2D.wireImage;
    discrMap=w->hist2D.discrMap;
    vertex  = w->hist2D.discrMap->vertex ;
    map = discrMap->map;
    createImageData(image,w);
    width = image->bytes_per_line / 4;
    columns= makeColumnDescr ( image);
    data = rescaleZ(w->hist2D.bins,w->hist2D.sourceHistogram->nYBins, 
      w->hist2D.discrMap, w->hist2D.xyVisibleRange, w->hist2D.zVisibleStart,
      w->hist2D.zVisibleEnd, w->hist2D.zLogScaling, width);
    if (w->hist2D.topErrors!=NULL)
        topErrors = rescaleZ(w->hist2D.topErrors,
        w->hist2D.sourceHistogram->nYBins, w->hist2D.discrMap, 
        w->hist2D.xyVisibleRange, w->hist2D.zVisibleStart,
        w->hist2D.zVisibleEnd, w->hist2D.zLogScaling, width);
    else
        topErrors = data;
    if (w->hist2D.bottomErrors!=NULL)
        bottomErrors = rescaleZ(w->hist2D.bottomErrors,
        w->hist2D.sourceHistogram->nYBins, w->hist2D.discrMap, 
        w->hist2D.xyVisibleRange, w->hist2D.zVisibleStart,
        w->hist2D.zVisibleEnd, w->hist2D.zLogScaling, width);
    else
        bottomErrors = data;

        
    yLine = makeLineDescr (map.y.x, map.x.x, width);
    xLine = makeLineDescr (map.y.y, map.x.y, width);
    makeBinPosDescr (w->hist2D.xyVisibleRange, discrMap, MAIN_BIN_WIDTH, 
      &binPositions);  
    if (errorBinWidth > MAIN_BIN_WIDTH) errorBinWidth = MAIN_BIN_WIDTH;   
    makeBinPosDescr (w->hist2D.xyVisibleRange, discrMap, errorBinWidth, 
      &er_binPositions);  
    makeBinPrm(discrMap, &binPositions,  xLine,
      yLine,  width, &binStartArray, &dXArray);
    makeBinPrm(discrMap, &er_binPositions,  xLine,
      yLine,  width, &er_binStartArray, &er_dXArray);
    { /* init floating horizont and draw axes  */
	dXFull = *dXArray;
	for (i=1, dXPtr = dXArray + 1; i < binPositions.nXPoints * 2 - 1;
	   i++, dXPtr++)
	{
            PLUS_B (dXFull, (*dXPtr));
	}    
 	dXFull.ml = abs(dXFull.x.highDig) + 
 	  abs(dXFull.y.highDig);
	binPtr = binStartArray;
	rowYPtr = ((CARD32*)image->data) + binPtr->yOffset;
	colXPtr = columns + binPtr->x.highDig;
        PUT_POINT_HOR(colXPtr,rowYPtr);
	MANH_X_DX(binPtr,(&dXFull), md);
	DRAW_LINE(binPtr->fy.highDig,yLine,PUT_POINT_HOR);
	rowYPtr = ((CARD32*)image->data) + binPtr->yOffset;
	colXPtr = columns + binPtr->x.highDig;
	MANH_Y(binPtr,(binPtr + binPositions.nYPoints * 2 - 1), md);
	DRAW_LINE(binPtr->fx.highDig,xLine,PUT_POINT_HOR);


	/* draw left point 					*/
	    rowYPtr = (CARD32*)image->data + 
	      ( binPtr + binPositions.nYPoints * 2 - 1)->yOffset;
	    colXPtr = columns + 
	      ( binPtr + binPositions.nYPoints * 2 - 1)->x.highDig;
	    PUT_POINT_HOR(colXPtr, (rowYPtr));
	/* draw right  point.
	   <binPtr> parameters still will be needed later.
	   So, they are not updated at this moment
	   to next vertex by macro <PLUS_B>, insted start parameters
	   to draw this vertical segment calculated directly.	   	*/
	   rowYPtr = (CARD32*)image->data + binPtr->yOffset +
	      dXFull.yOffset;
	   if (binPtr->y.firstDig + dXFull.y.firstDig >= binPtr->base)
	       rowYPtr += binPtr->factor;
	   colXPtr = columns + binPtr->x.highDig + dXFull.x.highDig;
	   if (binPtr->x.firstDig + dXFull.x.firstDig >= binPtr->base)
	       colXPtr++; 
	   PUT_POINT_HOR(colXPtr, rowYPtr);
    }	
/* save leftBaseVertex descriptor to draw backplanes later           */    
    leftBaseVertex = binPtr[binPositions.nYPoints * 2 - 1];
#if MAIN_BIN_WIDTH - MAXPIC
    rowYPtr = (CARD32*)image->data +  
      (binPtr + binPositions.nYPoints * 2 - 1)->yOffset ;
    colXPtr = columns +  (binPtr + binPositions.nYPoints * 2 - 1)->x.highDig;
    MANH_X_DX( (binPtr + binPositions.nYPoints * 2 - 1), dXArray, md); 
    DRAW_LINE( (binPtr + binPositions.nYPoints * 2 - 1)->fy.highDig,
      yLine,PUT_C_POINT);
#endif
    for (i=0, binPtr = binStartArray, er_binPtr = er_binStartArray;
      i < binPositions.nYPoints * 2; i++, binPtr++, er_binPtr++)
    {
        PLUS_B((*binPtr), (*dXArray));
        PLUS_B((*er_binPtr), (*er_dXArray)); 
    }   
    zPtrDy = 1;
    zPtrDx = binPositions.nYPoints - 1;
    zColumn = 0;
    dXPtr = dXArray + 1;
    er_dXPtr = er_dXArray + 1;
    dotMask = 0;
    for (i=0; i< binPositions.nXPoints -1; i++)
    {
	binPtr = binStartArray;
	PLUS_B((*binPtr), (*dXPtr));
	PLUS_B((*binPtr), (*(dXPtr+1)));
	binPtr++;
#if MAIN_BIN_WIDTH - MAXPIC
            if(i == binPositions.nXPoints - 2)
            {
                binPrm bTmp;
/* draw part of right far base edge while drawing last raw of bins  */            
	        rowYPtr = (CARD32*)image->data + (binPtr -1) ->yOffset ;
	        colXPtr = columns +  (binPtr -1)->x.highDig;
	        bTmp = *binPtr;
	        PLUS_B (bTmp, (*(dXPtr)));
   	    	PLUS_B (bTmp, (*(dXPtr+1)));
	        MANH_Y( (binPtr-1),( &bTmp), md);
	        DRAW_LINE( (binPtr-1)->fx.highDig,xLine,PUT_C_POINT);
	    }
#endif
	er_binPtr = er_binStartArray ;
	PLUS_B((*er_binPtr), (*er_dXPtr));
	PLUS_B((*er_binPtr), (*(er_dXPtr+1)));
	er_binPtr++;
	zIdx = zColumn;
	zColumn += zPtrDx;
	for (j =0; j < binPositions.nYPoints - 1 ; j++)
	{
	    if((binPtr+1)->goodBin != 0 && (dXPtr)->goodBin != 0)
	    {
	/* the elements drawing sequence is essential othervise
	   some elements might "shadow" other by floating
	   horizont in a wrong way.					*/
#if MAIN_BIN_WIDTH - MAXPIC
/* draw the base of main bin in this case				*/
	    rowYPtr = (CARD32*)image->data +  binPtr->yOffset;
	    colXPtr = columns +  binPtr->x.highDig;
	    PUT_C_POINT_HOR(colXPtr,rowYPtr);
	/* draw line to left 						*/
	    MANH_Y( binPtr,( binPtr+1), md);
	    DRAW_LINE( binPtr->fx.highDig,xLine,PUT_C_POINT_HOR);
	/* draw line to right 						*/
	    rowYPtr = (CARD32*)image->data +  binPtr->yOffset ;
	    colXPtr = columns +  binPtr->x.highDig;
	    MANH_X_DX( binPtr, dXPtr, md); 
	    DRAW_LINE( binPtr->fy.highDig,yLine,PUT_C_POINT_HOR);
	/* draw left point 					*/
	    rowYPtr = (CARD32*)image->data + ( binPtr+1)->yOffset;
	    colXPtr = columns + ( binPtr+1)->x.highDig;
	    PUT_C_POINT_HOR(colXPtr, (rowYPtr));
	/* draw right  vertical line.
	   <binPtr> parameters still will be needed later to draw
	   top error bar. So, they are not updated at this moment
	   to next bin vertex by macro <PLUS_B>, insted start parameters
	   to draw this vertical segment calculated directly.	   	*/
	   rowYPtr = (CARD32*)image->data + binPtr->yOffset +
	      dXPtr->yOffset;
	   if (binPtr->y.firstDig + dXPtr->y.firstDig >= binPtr->base)
	       rowYPtr += binPtr->factor;
	   colXPtr = columns + binPtr->x.highDig + dXPtr->x.highDig;
	   if (binPtr->x.firstDig + dXPtr->x.firstDig >= binPtr->base)
	       colXPtr++; 
	   PUT_C_POINT_HOR(colXPtr, rowYPtr);
#endif        
	    if((er_binPtr+1)->goodBin != 0 && (er_dXPtr)->goodBin != 0)
	    {
	/* draw  bottom error bin					*/
        /* setup pointers to the nearmost vertex 			*/ 
	    rowYPtr = (CARD32*)image->data + er_binPtr->yOffset +
	      data[zIdx].data;
	    colXPtr = columns + er_binPtr->x.highDig;
        /* draw central vertical edge 					*/    
            delta = bottomErrors[zIdx].data - data[zIdx].data;
	    DRAW_V_SEG_DOT(colXPtr, (rowYPtr), (rowYPtr + delta));
	/* draw line to left 						*/
	    dotMask = ~((CARD32)0);
	    MANH_Y(er_binPtr,(er_binPtr+1), md);
	    if (bottomErrors[zIdx].clipped)
	    {
	        DRAW_LINE(er_binPtr->fx.highDig,xLine,PUT_C_DBL_POINT);
	    }
	    else    
	    {
	        DRAW_LINE(er_binPtr->fx.highDig,xLine,PUT_C_DBL_POINT_DOT);
	    }
	/* draw line to right 						*/
	    rowYPtr = (CARD32*)image->data + er_binPtr->yOffset +
	      data[zIdx].data;
	    colXPtr = columns + er_binPtr->x.highDig;
	    dotMask = ~((CARD32)0);
	    MANH_X_DX(er_binPtr,er_dXPtr, md); 
	    if (bottomErrors[zIdx].clipped)
	    {
	        DRAW_LINE(er_binPtr->fy.highDig,yLine,PUT_C_DBL_POINT);
	    }
	    else    
	    {
	        DRAW_LINE(er_binPtr->fy.highDig,yLine,PUT_C_DBL_POINT_DOT);
	    }
	/* draw left vertical line 					*/
	    rowYPtr = (CARD32*)image->data + (er_binPtr+1)->yOffset +
	      data[zIdx].data;;
	    colXPtr = columns + (er_binPtr+1)->x.highDig;
	    DRAW_V_SEG_DOT(colXPtr, (rowYPtr), (rowYPtr + delta));
	/* draw right  vertical line.
	   <*er_binPtr> parameters still will be needed later to draw
	   top error bar. So, they are not updated at this moment
	   to next bin vertex by macro <PLUS_B>, insted start parameters
	   to draw this vertical segment calculated directly.	   	*/
	   rowYPtr = (CARD32*)image->data + er_binPtr->yOffset +
	      er_dXPtr->yOffset;
	   if (er_binPtr->y.firstDig + er_dXPtr->y.firstDig >= er_binPtr->base)
	       rowYPtr += er_binPtr->factor;
	   colXPtr = columns + er_binPtr->x.highDig + er_dXPtr->x.highDig;
	   if (er_binPtr->x.firstDig + er_dXPtr->x.firstDig >= er_binPtr->base)
	       colXPtr++; 
	   DRAW_V_SEG_DOT(colXPtr, (rowYPtr + data[zIdx].data), 
	     (rowYPtr + bottomErrors[zIdx].data));
	    } 
	/* draw fron part of main bar now				*/
        /* setup pointers to the nearmost vertex 		*/ 
	    rowYPtr = (CARD32*)image->data + binPtr->yOffset + data[zIdx].data;
	    colXPtr = columns + binPtr->x.highDig;
        /* draw central vertical edge 				*/    
            DRAW_V_LINE_HOR(colXPtr,rowYPtr);
	/* draw left vertical edge, using corresponding vertex descriptor 
	   (macro DRAW_V_LINE body refers its paramters only  once) */ 
	    DRAW_V_LINE_HOR(columns + (binPtr+1)->x.highDig, 
	      (CARD32*)image->data + (binPtr+1)->yOffset + data[zIdx].data);
	/* draw line to left 					*/
	    MANH_Y(binPtr,(binPtr+1), md);
	    DRAW_LINE(binPtr->fx.highDig,xLine,PUT_C_POINT_HOR);
	/* move binDescr to next vertex	and draw vertical and top right	edges */
	    rowYPtr = (CARD32*)image->data + binPtr->yOffset + data[zIdx].data;
	    colXPtr = columns + binPtr->x.highDig;
	    MANH_X_DX(binPtr,dXPtr, md); 
	    f0 = binPtr->fy.highDig;
	    PLUS_B ((*binPtr), (*dXPtr));
	    DRAW_V_LINE_HOR(columns + binPtr->x.highDig, 
	      (CARD32*)image->data + binPtr->yOffset + data[zIdx].data);
	    DRAW_LINE(f0,yLine,PUT_C_POINT_HOR);
	    
	/* now draw top error bar					*/
	    if((er_binPtr+1)->goodBin != 0 && (er_dXPtr)->goodBin != 0)
	    {
	    CARD32 *hl;
	    CARD32 *hr;
	    columnDescr *xHPtr;
        /* setup pointers to the nearmost vertex 		*/ 
            delta = data[zIdx].data - topErrors[zIdx].data;
	    rowYPtr = (CARD32*)image->data + er_binPtr->yOffset + 
	      topErrors[zIdx].data;
	    colXPtr = columns + er_binPtr->x.highDig;
        /* draw central vertical edge 				*/    
	    if (data[zIdx].clipped)
	    {
  	        DRAW_V_LINE_HOR(colXPtr,rowYPtr);
	    }
	    else
	    {
   	        DRAW_V_SEG_HOR(colXPtr,rowYPtr,rowYPtr + delta);
	    }
        /* save horizont in the vertex point to not shadow top edge of the main
           bar (in some angles)						*/
            hl = (columns + (er_binPtr+1)->x.highDig) -> hor;	        
	/* draw left vertical edge, using corresponding vertex descriptor 
	   (macro DRAW_V_SEG body refers its paramters only  once) */ 
	    DRAW_V_SEG(columns + (er_binPtr+1)->x.highDig, 
	      (CARD32*)image->data + (er_binPtr+1)->yOffset + 
	      topErrors[zIdx].data, 
	      (CARD32*)image->data + (er_binPtr+1)->yOffset + 
	      data[zIdx].data);
	/* draw line to left 					*/
	    MANH_Y(er_binPtr,(er_binPtr+1), md);
	    if (data[zIdx].clipped)
	    {
	         DRAW_LINE(er_binPtr->fx.highDig,xLine,PUT_C_DBL_POINT_V_HOR);
	    }
	    else
	    {
	         DRAW_LINE(er_binPtr->fx.highDig,xLine,PUT_C_POINT_HOR);
	    }
	/* move binDescr to next vertex	and draw vertical and top right	edges */
	    rowYPtr = (CARD32*)image->data + er_binPtr->yOffset + 
	      topErrors[zIdx].data;
	    colXPtr = columns + er_binPtr->x.highDig;
	    MANH_X_DX(er_binPtr,er_dXPtr, md); 
	    f0 = er_binPtr->fy.highDig;
	    PLUS_B ((*er_binPtr), (*er_dXPtr));
        /* save horizont in the vertex point to not shadow top edge of the main
           bar (in some angles)						*/
            hr = (columns + er_binPtr->x.highDig) -> hor;	        
	    DRAW_V_SEG(columns + er_binPtr->x.highDig, 
	      (CARD32*)image->data + er_binPtr->yOffset + topErrors[zIdx].data,
	      (CARD32*)image->data + er_binPtr->yOffset + data[zIdx].data);
	    if (data[zIdx].clipped)
	    {
	         DRAW_LINE(f0,yLine,PUT_C_DBL_POINT_V_HOR);
	    }
	    else
	    {
	        DRAW_LINE(f0,yLine,PUT_C_POINT_HOR);
	    }
	    
	/* draw top edges (plane) of the top error bar			*/ 
	    if (map.x.y < map.y.y)
	    {
	        int x0, y0, f0, md0;
	        er_binPtr++;
	        x0 = er_binPtr->x.highDig;
	        y0 = er_binPtr->yOffset;
	        f0 = er_binPtr->fy.highDig;
	        MANH_X_DX(er_binPtr,er_dXPtr, md0);
   	        PLUS_B ((*er_binPtr), (*er_dXPtr));
   	        er_binPtr--;
 	 	rowYPtr = (CARD32*)image->data + er_binPtr->yOffset + 
 	 	   topErrors[zIdx].data;
	        colXPtr = columns + er_binPtr->x.highDig;
	        MANH_Y(er_binPtr,(er_binPtr+1), md);
		if (topErrors[zIdx].clipped)
		{
	            DRAW_LINE(er_binPtr->fx.highDig,xLine,DRAW_V_LINE_HOR);
		}
		else 
		{
	            DRAW_LINE(er_binPtr->fx.highDig,xLine,PUT_C_POINT_HOR);
		}
 	 	rowYPtr = (CARD32*)image->data + y0 + topErrors[zIdx].data;
	        colXPtr = columns + x0;
	        md = md0; 
		if (topErrors[zIdx].clipped)
		{
  	            DRAW_LINE(f0,yLine,DRAW_V_LINE_HOR);
		}
		else 
		{
  	            DRAW_LINE(f0,yLine,PUT_C_POINT_HOR);
		}
    /* restore saved horizont. It could be lower then actual, but this is OK
       in the particular case, unless no setted points deleted furwer 
       (the algoritm sets points only)					*/
                (columns + er_binPtr->x.highDig) -> hor = hr;
                (columns + x0) -> hor = hl;
  	    } else
  	    {
  	        er_binPtr++;
  	        rowYPtr = (CARD32*)image->data + er_binPtr->yOffset + 
  	            topErrors[zIdx].data;
	        colXPtr = columns + er_binPtr->x.highDig;
    /* save X position to restore saved horizont. 			*/
                xHPtr = colXPtr;
	        MANH_X_DX(er_binPtr,er_dXPtr, md); 
		if (topErrors[zIdx].clipped)
		{
  	            DRAW_LINE(er_binPtr->fy.highDig,yLine,DRAW_V_LINE_HOR);
		}
		else 
		{
  	            DRAW_LINE(er_binPtr->fy.highDig,yLine,PUT_C_POINT_HOR);
		}
   	        PLUS_B ((*er_binPtr), (*er_dXPtr));
   	        er_binPtr--;
  	        rowYPtr = (CARD32*)image->data + er_binPtr->yOffset + 
  	          topErrors[zIdx].data;
	        colXPtr = columns + er_binPtr->x.highDig;
	        MANH_Y(er_binPtr,(er_binPtr+1), md);
		if (topErrors[zIdx].clipped)
		{
  	            DRAW_LINE(er_binPtr->fx.highDig,xLine,DRAW_V_LINE_HOR);
		}
		else 
		{
  	            DRAW_LINE(er_binPtr->fx.highDig,xLine,PUT_C_POINT_HOR);
		}
    /* restore saved horizont. It could be lower then actual, but this is OK
       in the particular case, unless no setted points deleted furwer 
       (the algoritm sets points only)	 				*/
                xHPtr -> hor = hl;
                (columns + er_binPtr->x.highDig) -> hor = hr;
	    } 
	    }   
	    else
	    {
   		PLUS_B ((*er_binPtr), (*er_dXPtr));
        	er_binPtr ++;
   		PLUS_B ((*er_binPtr), (*er_dXPtr));
   		er_binPtr--;
   	    }
	    
	    
	    
	/* draw  top edge of the bin. The order is essential, and
	    next <if> is better to move outside the loop. (Let optimaser
	    to do this if it can)				*/
	    if (map.x.y < map.y.y)
	    {
	        int x0, y0, f0, md0;
	        binPtr++;
	        x0 = binPtr->x.highDig;
	        y0 = binPtr->yOffset;
	        f0 = binPtr->fy.highDig;
	        MANH_X_DX(binPtr,dXPtr, md0);
   	        PLUS_B ((*binPtr), (*dXPtr));
   	        binPtr--;
 	 	rowYPtr = (CARD32*)image->data + binPtr->yOffset + data[zIdx].data;
	        colXPtr = columns + binPtr->x.highDig;
	        MANH_Y(binPtr,(binPtr+1), md);
		if (data[zIdx].clipped)
		{
	            DRAW_LINE(binPtr->fx.highDig,xLine,DRAW_V_LINE_HOR);
		}
		else 
		{
	            DRAW_LINE(binPtr->fx.highDig,xLine,PUT_C_POINT_HOR);
		}
 	 	rowYPtr = (CARD32*)image->data + y0 + data[zIdx].data;
	        colXPtr = columns + x0;
	        md = md0; 
		if (data[zIdx].clipped)
		{
  	            DRAW_LINE(f0,yLine,DRAW_V_LINE_HOR);
		}
		else 
		{
  	            DRAW_LINE(f0,yLine,PUT_C_POINT_HOR);
		}
  	    } else
  	    {
  	        binPtr++;
  	        rowYPtr = (CARD32*)image->data + binPtr->yOffset + 
  	            data[zIdx].data;
	        colXPtr = columns + binPtr->x.highDig;
	        MANH_X_DX(binPtr,dXPtr, md); 
		if (data[zIdx].clipped)
		{
  	            DRAW_LINE(binPtr->fy.highDig,yLine,DRAW_V_LINE_HOR);
		}
		else 
		{
  	            DRAW_LINE(binPtr->fy.highDig,yLine,PUT_C_POINT_HOR);
		}
   	        PLUS_B ((*binPtr), (*dXPtr));
   	        binPtr--;
  	        rowYPtr = (CARD32*)image->data + binPtr->yOffset + data[zIdx].data;
	        colXPtr = columns + binPtr->x.highDig;
	        MANH_Y(binPtr,(binPtr+1), md);
		if (data[zIdx].clipped)
		{
  	            DRAW_LINE(binPtr->fx.highDig,xLine,DRAW_V_LINE_HOR);
		}
		else 
		{
  	            DRAW_LINE(binPtr->fx.highDig,xLine,PUT_C_POINT_HOR);
		}
	    }
	    }
	    else
	    {
   		PLUS_B ((*binPtr), (*dXPtr));
        	binPtr ++;
   		PLUS_B ((*binPtr), (*dXPtr));
   		binPtr --;
   		PLUS_B ((*er_binPtr), (*er_dXPtr));
        	er_binPtr ++;
   		PLUS_B ((*er_binPtr), (*er_dXPtr));
   		er_binPtr--;
   	    }
	        
   	    PLUS_B ((*binPtr), (*(dXPtr+1)));
            binPtr ++;
   	    PLUS_B ((*binPtr), (*(dXPtr+1)));
   	    binPtr ++;
   	    PLUS_B ((*er_binPtr), (*(er_dXPtr+1)));
            er_binPtr ++;
   	    PLUS_B ((*er_binPtr), (*(er_dXPtr+1)));
   	    er_binPtr ++;
            zIdx += zPtrDy;
#if MAIN_BIN_WIDTH - MAXPIC
            if(i == binPositions.nXPoints - 2)
            {
                binPrm bTmp;
/* draw part of right far base edge while drawing last raw of bins  */            
	        rowYPtr = (CARD32*)image->data + (binPtr -2) ->yOffset ;
	        colXPtr = columns +  (binPtr -2)->x.highDig;
	        bTmp = *binPtr;
	        PLUS_B (bTmp, (*(dXPtr)));
   	    	PLUS_B (bTmp, (*(dXPtr+1)));
	        MANH_Y( (binPtr-2),( &bTmp), md);
	        DRAW_LINE( (binPtr-2)->fx.highDig,xLine,PUT_C_POINT);
	    }
#endif
        }
#if MAIN_BIN_WIDTH - MAXPIC
	rowYPtr = (CARD32*)image->data + binPtr->yOffset ;
	colXPtr = columns +  binPtr->x.highDig;
	MANH_X_DX( (binPtr), dXPtr, md); 
	DRAW_LINE( binPtr->fy.highDig,yLine,PUT_C_POINT);
#endif
	PLUS_B((*binPtr), (*dXPtr));
#if MAIN_BIN_WIDTH - MAXPIC
	rowYPtr = (CARD32*)image->data + binPtr->yOffset ;
	colXPtr = columns +  binPtr->x.highDig;
	MANH_X_DX( (binPtr), (dXPtr+1), md); 
	DRAW_LINE( binPtr->fy.highDig,yLine,PUT_C_POINT);
#endif
	PLUS_B((*binPtr), (*(dXPtr+1)));
	PLUS_B((*er_binPtr), (*er_dXPtr));
	PLUS_B((*er_binPtr), (*(er_dXPtr+1)));
        dXPtr += 2;
        er_dXPtr += 2;
    }
    if (w->hist2D.backPlanesOn)
    {
        int level;
        labelsToDraw *lT = w->hist2D.currentPicture->zLabels;
        level = vertex.y - map.y.y + discrMap->zFactor;
        binPtr=binStartArray;
	do
        {
/* draw line on the right backplane				*/        
            dotMask = 0;
	    rowYPtr = ((CARD32*)image->data) + binPtr->yOffset + 
	      (level - vertex.y + map.y.y) * width;
	    colXPtr = columns + binPtr->x.highDig;
	    MANH_Y(binPtr,(binPtr + binPositions.nYPoints * 2 - 1), md);
	    DRAW_LINE(binPtr->fx.highDig,xLine,PUT_C_POINT_DOT);
/* draw line on the left backplane				*/        
            dotMask = 0;
	    rowYPtr = ((CARD32*)image->data) + leftBaseVertex.yOffset + 
	      (level - vertex.y + map.y.y) * width;
	    colXPtr = columns + leftBaseVertex.x.highDig;
	    MANH_X_DX((&leftBaseVertex),(&dXFull), md);
	    DRAW_LINE(leftBaseVertex.fy.highDig,yLine,PUT_C_POINT_DOT);
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
        rowYPtr = ((CARD32*)image->data) + binPtr->yOffset + 
          discrMap->zFactor * width; 
	colXPtr = columns + binPtr->x.highDig;
/* lift to upper cube vertex   */	
/* and draw vertical dotted line				*/	
        DRAW_V_LINE_DOT(colXPtr,rowYPtr);
/* set pointers to farmost base vertex and lift to upper cube vertex   */
        binPtr += binPositions.nYPoints * 2 - 1;
	rowYPtr = ((CARD32*)image->data) + binPtr->yOffset;
	colXPtr = columns + binPtr->x.highDig;
	PUT_C_POINT_HOR(colXPtr,rowYPtr);
	rowYPtr += discrMap->zFactor * width;
/* and draw vertical dotted line				*/	
        DRAW_V_LINE_DOT(colXPtr,rowYPtr);
/* not drawing left vertical - Z axis is there anyway		*/
    }
    destroyLineDescr (&xLine);
    destroyLineDescr (&yLine);
    MYFREE(binPositions.xSteps);
    MYFREE(binPositions.ySteps);
    MYFREE(er_binPositions.xSteps);
    MYFREE(er_binPositions.ySteps);
    MYFREE(dXArray);
    MYFREE(binStartArray);    
    MYFREE(er_dXArray);
    MYFREE(er_binStartArray);
    if(topErrors != data) MYFREE(topErrors); 
    if(bottomErrors != data) MYFREE(bottomErrors);   
    MYFREE(data);
    MYFREE(columns);
    
}    

void makeEdgePoints 
    ( range2d r2, discreteMap *dMap, 
     vector *lengths,
    vector **xEdges, vector **yEdges)
{
    binPosDescr binPositions;
    binPrm *binStartArray;
    binPrm *dXArray;
    binPrm *bXTmp;
    binPrm *bYTmp;
    int dx, dy;
    binPrm *dXTmp;
    vector *pTmp;
    static lineDescr fakedLine = {0, 0, 0, NULL, NULL, NULL};
    vector v0;
    int i, ix, iy;
    makeBinPosDescr (r2, dMap, MAXPIC, 
      &binPositions);
    lengths->x = binPositions.lx;
    lengths->y = binPositions.ly;  
    makeBinPrm(dMap, &binPositions,  &fakedLine,
      &fakedLine,  1, &binStartArray, &dXArray);
    for(bXTmp = binStartArray, dXTmp = dXArray,i = 0; 
      i < 2 * binPositions.nXPoints - 1;
      i++, dXTmp ++)
    {
        PLUS_B ((*dXTmp),(*bXTmp));
        bXTmp = dXTmp;
    }  
/* determine if first plane is a bar plane or a cut plane		*/
    r2=transformRange2d(r2,dMap->rotMatr);
    v0 = r2.start;
    if (v0.x >= 0 )
	v0.x /= MAXPIC;
    else 
    {
	v0.x = -v0.x;
	v0.x += MAXPIC - 1;
	v0.x /= MAXPIC;
	v0.x = - v0.x;
    }
    if (v0.y >= 0 )
	v0.y /= MAXPIC;
    else 
    {
	v0.y = -v0.y;
	v0.y += MAXPIC - 1;
	v0.y /= MAXPIC;
	v0.y = - v0.y; 
    }
    if (r2.end.x > 0)
    {
        dx = -2;
        bXTmp = dXArray + 2 * binPositions.nXPoints - 2 ;
    }
    else 
    {
        dx = 2;
        bXTmp = dXArray;
    }
    if (r2.end.y > 0)
    {
        dy = -2;
        bYTmp = binStartArray + 2 * binPositions.nYPoints - 1 ;
    }
    else 
    {
        dy = 2;
        bYTmp = binStartArray + 1;
    }
    r2 = translateRange2d (r2, mulVector (v0, -MAXPIC));
/* and shift to first non-cut plane if needed		   */
    ix = iy =0;
    if (dx > 0)
    {
        if (r2.end.x % MAXPIC !=0)
            ix = 1;
    }
    else 
        if (r2.start.x % MAXPIC !=0)
            ix = 1;
    if (dy > 0)
    {
        if (r2.end.y % MAXPIC !=0)
            iy = 1;
    }
    else 
        if (r2.start.y % MAXPIC !=0)
            iy = 1;
    bXTmp += ix * dx;
    bYTmp += iy * dy;
    MYMALLOC(vector, *xEdges, binPositions.nXPoints);
    MYMALLOC(vector, *yEdges, binPositions.nYPoints);
    for( pTmp = *xEdges;  ix < binPositions.nXPoints ;
         ix++, pTmp++, bXTmp += dx)
    {
        pTmp->x = bXTmp->x.highDig;
        pTmp->y = bXTmp->y.highDig; 
    }
    for(   pTmp = *yEdges; iy < binPositions.nYPoints ;
         iy ++, pTmp++, bYTmp+= dy)
    {
        pTmp->x = bYTmp->x.highDig;
        pTmp->y = bYTmp->y.highDig; 
    }     
    MYFREE(binPositions.xSteps);
    MYFREE(binPositions.ySteps);
    MYFREE(dXArray);
    MYFREE(binStartArray);    
}      
    

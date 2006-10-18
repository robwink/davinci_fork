#include <X11/StringDefs.h>
#include <X11/Intrinsic.h> 
#include <X11/Xmd.h>
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#if XmVersion == 1002
#include <Xm/PrimitiveP.h>
#endif
#include <stdio.h>
#include "2DHist.h"
#include "adaptHist.h"
#include "fltRange.h"

/* Maximum depth (-1) of histogram tree */
#define MAX_SPLIT_NUMBER 20	

#ifndef _X
#define _X 0
#endif

#ifndef _Y
#define _Y 1
#endif

#define _START 0
#define _END 1


typedef struct _aHistTree aHistTree;

struct _aHistTree {
    aHistTree *right; 
    aHistTree *left;
    aHistNode *node;
} ;

static void mkTNode(aHistTree *aT, t* data, int nData, int Idx, 
  float cell[2][2], int nLimit,int strategy, int splitCounter, Boolean
  splitMade);
static int aTNodeNum(aHistTree *aT);
static void transAHist ( aHistTree *aT, aHistNode **aN);
static void destrAHistTree ( aHistTree *aT);
static void trans1DAHist ( aHistTree *aT, float xMin, float xMax, float **bins, 
  float **binEdges);
static void mk1DTNode(aHistTree *aT, float* data, int nData,  
  float cell[2], int nLimit,int strategy, int splitCounter);
/*
** Build an adaptive histogram data structure by binning the data from
** the array dataArray. ....
** dataArray is an array of floats alternating x,y,x,y,... etc. containing
** nPairs pairs of numbers.
** Use "natural" X-Y domain for binning/normalization
*/
aHistStruct *Bin2DAdaptHist(t* dataArray, int nPairs, int nLimit, 
  int strategy)
{
    int j;
    t *data =  dataArray;
    double xMin, xMax, yMin, yMax;
    /* calculate the minimus and maximums of the data */
    if (dataArray == NULL || nPairs == 0)
        return (NULL);
    xMin = xMax = data -> c[_X];
    yMin = yMax = data -> c[_Y];
    for (j =1; j < nPairs ; j++) {
        if ( xMin > data[j].c[_X]) xMin = data[j].c[_X];
        if ( xMax < data[j].c[_X]) xMax = data[j].c[_X];
        if ( yMin > data[j].c[_Y]) yMin = data[j].c[_Y];
        if ( yMax < data[j].c[_Y]) yMax = data[j].c[_Y];
    }
    return (Bin2DAdaptHistInDom(data, nPairs, xMin, xMax, yMin, yMax,
      nLimit, strategy));
}


/*
** Build an adaptive histogram data structure by binning the data from
** the array dataArray. ....
** dataArray is an array of floats alternating x,y,x,y,... etc. containing
** nPairs pairs of numbers.
** Use <xMin, xMax, yMin, yMax> range as the histogram domain for
** normalization.
** If the specified domain smaller than actual data domain the result
** will be incorrect.
*/
aHistStruct *Bin2DAdaptHistInDom(t* dataArray, int nPairs, double xMin,
	double xMax, double yMin, double yMax, int nLimit, int strategy)
{
    aHistTree *aT;
    t *data = (t *)dataArray;
    float cell [2][2];
    float fxMin = xMin; 
    float fxMax = xMax; 
    float fyMin = yMin; 
    float fyMax = yMax;
    aHistNode  *aHTmp;
    aHistStruct *aHist;

    /* assure the range is not singular, i.e. has non-zero lengths */
    setFRangeLimits(&fxMin,&fxMax);
    setFRangeLimits(&fyMin,&fyMax);
    aHist = (aHistStruct*) XtMalloc(sizeof(aHistStruct));
    aHist->xMin = fxMin;
    aHist->xMax = fxMax;
    aHist->yMin = fyMin;
    aHist->yMax = fyMax;
    
    /* make a starting node and call the recursive mkTNode routine to fill
       the histogram */
    cell[_X][_START] = aHist->xMin;
    cell[_X][_END] =   aHist->xMax;
    cell[_Y][_START] = aHist->yMin;
    cell[_Y][_END] =   aHist->yMax;
    if (dataArray == NULL || nPairs == 0)
    {
        aHist->aNode = NULL;
        return (aHist);
    }	        
    aT = (aHistTree *)XtCalloc(sizeof(aHistTree),1);
    mkTNode( aT, data, nPairs, _X, cell, nLimit,strategy, MAX_SPLIT_NUMBER,
    	    TRUE);
    
    /* translate the tree into a flat array of nodes, the form readable
       by the adaptive histogram widget, free the original tree and return */
    aHist->aNode = (aHistNode *)XtCalloc(sizeof(aHistNode),aTNodeNum(aT));
    aHTmp = aHist->aNode;
    transAHist (aT, &aHTmp);
    destrAHistTree(aT);
    return(aHist);
}


/*
** static void mkTNode(aHistTree *aT, t* data, int nData, int Idx, 
**  float cell[2][2], int nLimit,int strategy, int splitCounter, Boolean
**  splitMade)
**
**  aT  - 	Pointer to a current node to split of the constucting 
**		histogramm tree.
**  t   -	Pointer to a data points array (in the the form <X,Y>
**		corresponding to this node.
**  nData -	Number of points in the <t> array above
**  Idx   -	Designate axis to make a first split (X or Y).
**		If <Idx> == 0 (== _X) the first split will be across
**		X axis, if <Idx> == 1 (== _Y) the first split will be across
**		Y axis.
**  cell  -     rectangle region in <X,Y> plane corresponding to the
**		node (all data points should be in the region)
**  strategy -  how to split - if <strategy> == 0  the cell will be
**		splitted by half in X and Y in turn.
**		If <strategy> == 1 the cell will be splitted by gravity
**		center of the points in X and Y in turn.
**  nLimit -	Stop condition value. The spliting process will be
**		stoped for cells with <nData> <= <nLimit>
**  splitCounter - Another stop condition value. The tree starting
**		from this node will have a depth not more then 
**		<splitCounter> - 1. I.e. not more then <splitCounter>
**		splits will be performed along any split path starting
**		from this node, regardless of <nLimit> based condition.
**  splitMade - Value for additional stop condition in the case
**		of gravity center splitting (<strategy> == 1),
**		otherwise ignored.
**		If TRUE, that means, that successful split was done
**	        on previous step, i.e. the split wich reduced the
**		number of points in either of the two new cells, so 
**		both of the new cells are not empty.
**		The algorithm will stop further splitting attempts after
**		two subsequent unsuccessful splits, which means that
**		all remaning points have exactly the same value, so
**		further splitting has no useful effect anyway.
*/

static void mkTNode(aHistTree *aT, t* data, int nData, int Idx, 
  float cell[2][2], int nLimit,int strategy, int splitCounter, Boolean
  splitMade)
{
    int i;
    t *tmp1, *tmp2;
    t tmpDat;
    float split;
    float newCell[2][2];
    int nLeft;
    if (aT == NULL) return;
    if (nData <= nLimit || splitCounter == 0)
    {
/* stop condition fulfilled, no more splits				*/    
        aT->right = NULL;
        aT->left = NULL;
        aT->node = (aHistNode *)XtCalloc(sizeof(aHistNode),1);
        aT->node->nextNodeOffset = 0;
        aT->node->data.zData = nData/( (cell[_X][_END] - cell[_X][_START]) *
          (cell[_Y][_END] - cell[_Y][_START]));
        return;
    } else
    {
        if (strategy == 0)
            split = (cell[Idx][_END] - cell[Idx][_START])/2 + cell[Idx][_START];
        else
        {
            split = 0;
            for (i =0; i< nData; i++) split+=data[i].c[Idx];
            split/= nData;
        }
        for (tmp1=data, tmp2=data + nData - 1;tmp2 > tmp1;)
        {
            if (tmp1->c[Idx] >= split)
            {
 		tmpDat = *tmp1;
 		*tmp1 = *tmp2;
 		*tmp2 = tmpDat;
 		tmp2 --;
 	    }
 	    else
 	        tmp1 ++;
	}
	if (tmp1->c[Idx] >= split)
   	    nLeft = tmp1 - data;
        else 
       	    nLeft = tmp1 - data + 1;
       	if ((strategy == 1 && (!splitMade) && (nLeft == 0 || nLeft == nData)) ||
       	  (split - cell[Idx][_START])== 0. || (cell[Idx][_END] - split) == 0.)    
       	{
            aT->right = NULL;
            aT->left = NULL;
            aT->node = (aHistNode *)XtCalloc(sizeof(aHistNode),1);
            aT->node->nextNodeOffset = 0;
            aT->node->data.zData = nData/( (cell[_X][_END] - cell[_X][_START]) *
            (cell[_Y][_END] - cell[_Y][_START]));
            return;
        }
        splitMade = (nLeft != 0 && nLeft != nData);
        splitCounter--;
	aT->right = (aHistTree *)XtCalloc(sizeof(aHistTree),1);
        aT->left = (aHistTree *)XtCalloc(sizeof(aHistTree),1);
        aT->node = (aHistNode *)XtCalloc(sizeof(aHistNode),1);
        aT->node->nextNodeOffset = 1;
        aT->node->data.xySplit = split;

	newCell[0][0] = cell[0][0];
	newCell[0][1] = cell[0][1];
	newCell[1][0] = cell[1][0];
	newCell[1][1] = cell[1][1];
	newCell[Idx][_END] = split;
	mkTNode(aT->left, data, nLeft,1 - Idx, newCell,  nLimit, strategy,
	  splitCounter,splitMade);	
	cell[Idx][_START] = split;
	mkTNode(aT->right, data + nLeft, nData - nLeft,1 - Idx, cell,  nLimit,
	  strategy, splitCounter, splitMade);     	        
    }
} 	        
 	        
static int aTNodeNum(aHistTree *aT)
{
     int n = 1;
     if (aT == NULL) return (0);
     if (aT->left != NULL)
         n+=aTNodeNum(aT->left);
     if (aT->right != NULL)
         n+=aTNodeNum(aT->right);
     return (n);
}

static void transAHist ( aHistTree *aT, aHistNode **aN)
{
    if (aT == NULL) return;
    **aN = *(aT->node);
    if (aT->left != NULL) 
    {
        (*aN)-> nextNodeOffset = aTNodeNum(aT->left) + 1;
	(*aN) ++;
        transAHist(aT->left, aN);
        transAHist(aT->right, aN);        
    }
    else 
    {
        (*aN)-> nextNodeOffset = 0;
        (*aN) ++;
    }
}

static void destrAHistTree ( aHistTree *aT)
{
    if (aT == NULL) return;
    if (aT->node != NULL) XtFree((char *)aT->node);
    if (aT->left != NULL) destrAHistTree (aT->left);
    if (aT->right != NULL) destrAHistTree (aT->right);
    XtFree((char *)aT);
}
 
/*
** Build an adaptive 1D histogram data structure by binning the data from
** the array <data >. ....
** Use "natural" X  domain for binning/normalization
*/
void Bin1DAdaptHist(float* data, int nData, int nLimit, 
  int strategy, float **bins, float **binEdges, int* nBins)
{
    int j;
    double xMin, xMax;
    if (data == NULL || nData == 0)
    {
        *bins = NULL;
        *binEdges = NULL;
        nBins = 0;
        return;
    }        
    /* calculate the minimus and maximums of the data */
    xMin = xMax = data[0];
    for (j =1; j < nData ; j++) {
        if ( xMin > data[j]) xMin = data[j];
        if ( xMax < data[j]) xMax = data[j];
    }
    Bin1DAdaptHistInDom(data, nData, xMin, xMax, nLimit, strategy,
      bins, binEdges, nBins);
}


/*
** Build an 1D adaptive histogram data structure by binning the data from
** the array <data> . ....
** Use <xMin, xMax> range as the histogram domain for
** normalization.
** If the specified domain smaller than actual data domain the result
** will be incorrect.
*/
void Bin1DAdaptHistInDom(float* data, int nData,  double xMin,
	double xMax, int nLimit, int strategy, float **bins, float **binEdges, 
	int* nBins) 
{
    aHistTree *aT;
    float cell [2];
    float fxMin = xMin; 
    float fxMax = xMax; 
    float *binsTmp;
    float *binEdgesTmp;
    if (data == NULL || nData == 0)
    {
        *bins = NULL;
        *binEdges = NULL;
        nBins = 0;
        return;
    }        
    /* assure the range is not singular, i.e. has non-zero lengths */
    setFRangeLimits(&fxMin,&fxMax);
    /* make a starting node and call the recursive mkTNode routine to fill
       the histogram */
    cell[_START] = xMin;
    cell[_END] = xMax;
    if (data  == NULL || nData == 0)
    {
        *bins = NULL;
        *binEdges = NULL;
        nBins = 0;
        return;
    }	        
    aT = (aHistTree *)XtCalloc(sizeof(aHistTree),1);
    mk1DTNode( aT, data, nData, cell, nLimit,strategy, MAX_SPLIT_NUMBER);
       
/* translate the tree into a flat array of nodes, the form readable
       by the adaptive histogram widget, free the original tree and return */
/* number of leaf nodes in the tree (which corresponds to bins)		   */       
    *nBins = (aTNodeNum(aT) - 1)/2 +1;
    if (nBins != 0)
    {
	*bins = (float *)XtCalloc(sizeof(float),*nBins); 
	*binEdges = (float *)XtCalloc(sizeof(float),*nBins + 1); 
        binsTmp = *bins;
        binEdgesTmp = *binEdges;
        trans1DAHist (aT, xMin, xMax,  &binsTmp, &binEdgesTmp);
    }
    else
    {
        *bins = NULL;
        *binEdges = NULL;
    }  
    destrAHistTree(aT);
}
    
/*
** static void mk1DTNode(aHistTree *aT, float* data, int nData,  
**  float cell[2], int nLimit,int strategy, int splitCounter )
**
**  aT  - 	Pointer to a current node to split of the constucting 
**		histogramm tree.
**  data   -	Pointer to a data points array  
**		corresponding to this node.
**  nData -	Number of points in the <data> array above
**  cell  -     rectangle region in <X> axis corresponding to the
**		node (all data points should be in the region)
**  strategy -  how to split - if <strategy> == 0  the cell will be
**		splitted by half in X and Y in turn.
**		If <strategy> == 1 the cell will be splitted by gravity
**		center of the points in X and Y in turn.
**  nLimit -	Stop condition value. The spliting process will be
**		stoped for cells with <nData> <= <nLimit>
**  splitCounter - Another stop condition value. The tree starting
**		from this node will have a depth not more then 
**		<splitCounter> - 1. I.e. not more then <splitCounter>
**		splits will be performed along any split path starting
**		from this node, regardless of <nLimit> based condition.
*/

static void mk1DTNode(aHistTree *aT, float* data, int nData,  
  float cell[2], int nLimit,int strategy, int splitCounter)
{
    int i;
    float *tmp1, *tmp2;
    float tmpDat;
    float split;
    float newCell[2];
    int nLeft;
    if (data == NULL) return;
    if (nData <= nLimit || splitCounter == 0)
    {
/* stop condition fulfilled, no more splits				*/    
        aT->right = NULL;
        aT->left = NULL;
        aT->node = (aHistNode *)XtCalloc(sizeof(aHistNode),1);
        aT->node->nextNodeOffset = 0;
        aT->node->data.zData = nData/( (cell[_END] - cell[_START]) );
        return;
    } else
    {
        if (strategy == 0)
            split = (cell[_END] - cell[_START])/2 + cell[_START];
        else
        {
            split = 0;
            for (i =0; i< nData; i++) split+=data[i];
            split/= nData;
        }
        for (tmp1=data, tmp2=data + nData - 1;tmp2 > tmp1;)
        {
            if (*tmp1 >= split)
            {
 		tmpDat = *tmp1;
 		*tmp1 = *tmp2;
 		*tmp2 = tmpDat;
 		tmp2 --;
 	    }
 	    else
 	        tmp1 ++;
	}
	if (*tmp1 >= split)
   	    nLeft = tmp1 - data;
        else 
       	    nLeft = tmp1 - data + 1;
       	if ((strategy == 1 && (nLeft == 0 || nLeft == nData)) ||
       	  (split - cell[_START])== 0. || (cell[_END] - split) == 0.)    
       	{
            aT->right = NULL;
            aT->left = NULL;
            aT->node = (aHistNode *)XtCalloc(sizeof(aHistNode),1);
            aT->node->nextNodeOffset = 0;
            aT->node->data.zData = nData/( cell[_END] - cell[_START]);
            return;
        }
        splitCounter--;
	aT->right = (aHistTree *)XtCalloc(sizeof(aHistTree),1);
        aT->left = (aHistTree *)XtCalloc(sizeof(aHistTree),1);
        aT->node = (aHistNode *)XtCalloc(sizeof(aHistNode),1);
        aT->node->nextNodeOffset = 1;
        aT->node->data.xySplit = split;

	newCell[0] = cell[0];
	newCell[1] = cell[1];
	newCell[_END] = split;
	mk1DTNode(aT->left, data, nLeft, newCell,  nLimit, strategy,
	  splitCounter);	
	cell[_START] = split;
	mk1DTNode(aT->right, data + nLeft, nData - nLeft, cell,  nLimit,
	  strategy, splitCounter); 
    }	      	        
} 	        
  
static void trans1DAHist ( aHistTree *aT, float xMin, float xMax, float **bins, 
  float **binEdges)
{
    if (aT == NULL) return;
    if (aT->left != NULL) 
    {
        trans1DAHist(aT->left, xMin, aT->node->data.xySplit, bins, binEdges);
        trans1DAHist(aT->right, aT->node->data.xySplit, xMax, bins, binEdges);        
    }
    else 
    {
/* leaf encountered						*/  
        **bins = aT->node->data.zData;
        **binEdges = xMin;
        (*bins)++;
        (*binEdges)++;
/* the next <*binEdges> value in most cases will be overwritten once more 
   (with same value) during continuation of the  tree wallk, 
   but this operator is needed to fill the last entry in the <*binEdges>
   array 							*/        
        **binEdges = xMax;     
    }
}
         
         

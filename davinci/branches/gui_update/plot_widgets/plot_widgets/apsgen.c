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
/*?*/
#include <stdio.h>
#include <math.h>
#include "2DGeom.h"
#include "2DHistDefs.h"
#include "2DHist.h"
#include "uniLab.h"
#include "labels.h"		
#include "2DHistP.h"
#include "imaggen.h"
#include "aimaggen.h"
#include "apsgen.h"

#define _X 0
#define _Y 1
#define _START 0
#define _END 1
#define MAXPIC_2 (MAXPIC * MAXPIC)

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

static void drawBin(walkStep *curCell);
static void aRescaleZ(float fData, discreteData *dData);

/* 
   Those variable are declared above as static (non-local) 
   to provide interface between makeAHistPSImage() drawBin() and
   aRescaleZ() functions. (Because drawBin() and  aRescaleZ() are called for
   each bin, passing all the paramerters each time could slows down 
   the processing. 
   This variables are setted up by makeAHistPSImage() and are used by
   functions calld from makeAHistPSImage()
*/   
static float visible[2][2];
static float xScaleFactor;
static float yScaleFactor;
static float zScaleFactor;
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
static FILE *psfile;

void makeAHistPSImage(Hist2DWidget w, FILE *outPsfile)
{
    discreteMap *dMap;
    vector vertex ;
    walkStep *walkArr;
    walkStep *tmp; 
    int levNum;
    int walkIdx;
    aHistNode *curNode;
    walkBranch walkDescr[2];
    /* whole histogram X-Y range in source coordinates        	*/
    fcell aHistRange;
    matrix2 map;
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
        return;
    }
    dMap=w->hist2D.discrMap;
    vertex  =  dMap->vertex ;
    map = dMap->map;
    psfile = outPsfile;

    fprintf(psfile,"/ah_window_row_start   %%xobj => row_xw row_yw 	\n");
    fprintf(psfile,"%% xobj - X coordinates of bar row start in the	\n");
    fprintf(psfile,"%% object coordinate system, relatively to current	\n");
    fprintf(psfile,"%% visible region. 					\n");
    fprintf(psfile,"%% Corresponding Y coordinate of bar row start is 	\n");
    fprintf(psfile,"%% alwase 0 (relatively to current visible region	\n");
    fprintf(psfile,"%% so it is not used				\n");
    fprintf(psfile,"%% row_xw row_yw - window coordinates (multiplied 	\n");
    fprintf(psfile,"%% by MAXPIC for PostSript) of this object point	\n");
    fprintf(psfile,"%%  under current mapping				\n");
    fprintf(psfile,"{							\n");
    fprintf(psfile,"    dup     %% xobj  xobj 				\n");
    fprintf(psfile,"    %d mul	%% xobj  ..				\n", 
      map.x.x );
    fprintf(psfile,"    %d div	%% .. / mapLength.x			\n",
      MAXPIC );
    fprintf(psfile,"    %d add	%% xobj xw 				\n", 
      (vertex.x - map.x.x -map.x.y) << LOWBITS);
    fprintf(psfile,"    exch   		 				\n");
    fprintf(psfile,"    %d mul	%%  .. 					\n", 
      map.y.x );
    fprintf(psfile,"    %d div	%% .. / mapLength.x			\n",
      MAXPIC);
    fprintf(psfile,"    %d add	%%  yw					\n", 
      (vertex.y - map.y.x - map.y.y) << LOWBITS);
    fprintf(psfile,"} def						\n");

    fprintf(psfile,"/ah_window_bar_start 		%% yobj => xw yw	\n");
    fprintf(psfile,"%% yobj - Y coordinates of bar start in the		\n");
    fprintf(psfile,"%% object coordinate system, relatively to current	\n");
    fprintf(psfile,"%% visible region. 					\n");
    fprintf(psfile,"%% xw yw - window coordinates (multiplied by 	\n");
    fprintf(psfile,"%% MAXPIC for PostSript) of this object point under \n");
    fprintf(psfile,"%% current mapping					\n");
    fprintf(psfile,"%% THIS FUNCTION USE row_xw AND row_yw which assumed\n");
    fprintf(psfile,"%% to have a coorect value setted up by 		\n");
    fprintf(psfile,"%% window_row_start function			\n");
    fprintf(psfile,"{							\n");
    fprintf(psfile,"    dup     %% yobj  yobj 				\n");
    fprintf(psfile,"    %d mul	%% yobj  ..				\n", 
      map.x.y );
    fprintf(psfile,"    %d div	%% .. / mapLength.y			\n",
      MAXPIC);
    fprintf(psfile,"    row_xw add 	%% yobj xw 			\n"); 
    fprintf(psfile,"    exch	%% xw yobj  				\n");
    fprintf(psfile,"    %d mul	%% xw .. 				\n", 
      map.y.y );
    fprintf(psfile,"    %d div	%% .. / mapLength.y			\n",
      MAXPIC);
    fprintf(psfile,"    row_yw add	%% xw yw			\n"); 
    fprintf(psfile,"} def 						\n");
    
    fprintf(psfile,"/ah_set_bin_row_prms   %% x1obj x0obj => 		\n");
    fprintf(psfile,"%% set variables needed to draw a bin:	 	\n");
    fprintf(psfile,"%% X-edge vector for bin and error bars		\n");
    fprintf(psfile,"%% X-edge vector are setted once for row of bins    \n");
    fprintf(psfile,"%% Sets variables: x_x, x_y, 	 		\n");
    fprintf(psfile,"%% row_xw, row_yw					\n");
    fprintf(psfile,"{							\n");    
    fprintf(psfile,"	ah_window_row_start				\n");
    fprintf(psfile,"	3 2 roll					\n");
    fprintf(psfile,"	ah_window_row_start				\n");
    fprintf(psfile,"	2 index						\n");
    fprintf(psfile,"	sub						\n");
    fprintf(psfile,"	/x_y exch def					\n");
    fprintf(psfile,"	2 index						\n");
    fprintf(psfile,"	sub						\n");
    fprintf(psfile,"	/x_x exch def					\n");
    fprintf(psfile,"	/row_yw exch def				\n");
    fprintf(psfile,"	/row_xw exch def				\n");
    fprintf(psfile,"} def						\n");
 						   
    fprintf(psfile,"/ah_set_bin_prms	 %% y1obj y0obj  => 		\n");
    fprintf(psfile,"%% set variables needed to draw a bin:	 	\n");
    fprintf(psfile,"%% Y-edge vector for bin and error bars		\n");
    fprintf(psfile,"%% and shift for the main bar start point to the    \n");
    fprintf(psfile,"%% error bar start point.				\n"); 
    fprintf(psfile,"%% X-edge vector are setted once for row of bins    \n");
    fprintf(psfile,"%% Uses variables:			  		\n");
    fprintf(psfile,"%%  row_xw, row_yw					\n");
    fprintf(psfile,"%% Sets variables: y_x, y_y				\n");
    fprintf(psfile,"{							\n");    
    fprintf(psfile,"	ah_window_bar_start				\n");
    fprintf(psfile,"	3 2 roll					\n");
    fprintf(psfile,"	ah_window_bar_start				\n");
    fprintf(psfile,"	2 index						\n");
    fprintf(psfile,"	sub						\n");
    fprintf(psfile,"	/y_y exch def					\n");
    fprintf(psfile,"	2 index						\n");
    fprintf(psfile,"	sub						\n");
    fprintf(psfile,"	/y_x exch def					\n");
    fprintf(psfile,"} def						\n");

    fprintf(psfile,"/ah_b	 %% x0obj x1obj y0obj y1obj  => 	\n");
    fprintf(psfile,"%% draw a bin				 	\n");
    fprintf(psfile,"{							\n");    
    fprintf(psfile,"	ah_set_bin_row_prms				\n");
    fprintf(psfile,"    ah_set_bin_prms					\n");
    fprintf(psfile,"    bar_itself					\n");
    fprintf(psfile,"} def						\n");

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
/* reverse order in compare with the terminal version		*/        
        walkDescr[_X].left = 0;
        walkDescr[_X].right = 1;
    }
    else 
    {
        mapXStart = _START;
        mapXEnd = _END;
        mapXStartValue = 0;
/* reverse order in compare with the terminal version		*/        
        walkDescr[_X].left = 1;
        walkDescr[_X].right = 0;
    }
    if (dMap->rotMatr.x.y + dMap->rotMatr.y.y < 0)
    {
        yScaleFactor = -yScaleFactor;
        mapYStart = _END;
        mapYEnd = _START;
        mapYStartValue = MAXPIC_2;
/* reverse order in compare with the terminal version		*/        
        walkDescr[_Y].left = 0;
        walkDescr[_Y].right = 1;
    }
    else 
    {
        mapYStart = _START;
        mapYEnd = _END;
        mapYStartValue = 0;
/* reverse order in compare with the terminal version		*/        
        walkDescr[_Y].left = 1;
        walkDescr[_Y].right = 0;
    }

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
    MYFREE(walkArr);
}

static void drawBin(walkStep *curCell)
{
    float bin [2][2];
    float margin;
    int binInt [2][2];
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

/* check range (probably optional, becase clipping is done, 
   but more safty to do this because of possible rounding errors).
   If range went bad, result can be "unpredictable" up to a crash */
    if (binInt[_X][_START] < 0) binInt[_X][_START] = 0;
    if (binInt[_X][_END] > (MAXPIC * MAXPIC)) 
        binInt[_X][_END]=(MAXPIC * MAXPIC);
    if (binInt[_Y][_START] < 0) binInt[_Y][_START] = 0;
    if (binInt[_Y][_END] > (MAXPIC * MAXPIC)) 
        binInt[_Y][_END]=(MAXPIC * MAXPIC);

    if (binInt[_X][_START] < binInt[_X][_END] && 
      binInt[_Y][_START] < binInt[_Y][_END])
    {
	aRescaleZ(curCell->node->data.zData, &zData);
        fprintf(psfile,"%d %d ", zData .data, 
          (zData.clipped) ? 1 : 0);
        fprintf(psfile,"%d %d ",binInt[_Y][_START], binInt[_Y][_END]); 
        fprintf(psfile,"%d %d ",binInt[_X][_START], binInt[_X][_END]); 
        fprintf(psfile,"ah_b\n");
        
    }
    	
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
}


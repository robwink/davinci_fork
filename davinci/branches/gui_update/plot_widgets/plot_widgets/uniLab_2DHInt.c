#include <X11/Intrinsic.h> 
#include <math.h>
#include <stdio.h>
#include "2DHistMalloc.h"
/* for interface function only	in test version				*/
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#if XmVersion == 1002
#include <Xm/PrimitiveP.h>
#endif
#include "2DHist.h"
#include "2DGeom.h"
#include "2DHistDefs.h"
#include "2DHist.h"
#include "uniLab.h"
#include "labels.h"
#include "2DHistP.h"

void makeBaseUnbinnedLabels(XFontStruct *fs, discreteMap *dMap, 
     range2d xyVisible, range2d xyHistoRange,
     h2DHistSetup *srcHist, objectsToDraw *oD)
{
#define _X 0
#define _Y 1
#define _START 0
#define _END 1
#define MAXPIC_2 (MAXPIC * MAXPIC)
    vector d;
    XSegment segX;	/* right axis - virtual X		*/
    XSegment segY;	/* left axis - virtual Y		*/
    double objHistRange [2][2];
    double visible[2][2];
    labeledSegment *labSeg;
    labelsToDraw **lT;
    labelsToDraw2 *lT2;
    int vX, vY, vXStart, vXEnd, vYStart, vYEnd;
    
    oD->xTics=NULL; 
    oD->nXTics=0;
    oD->xLabels=NULL;
    oD->yTics=NULL; 
    oD->nYTics=0;
    oD->yLabels=NULL;
    if (fs==NULL) return;
    if (dMap==NULL) return; 
    objHistRange [_X][_START] = srcHist->xMin;
    objHistRange [_X][_END] = srcHist->xMax;
    objHistRange [_Y][_START] = srcHist->yMin;
    objHistRange [_Y][_END] = srcHist->yMax;
    
/* recalculate visible range in descrete units back to float
   source coordinates. xyHistoRange suppose to be
   [0 .. MAXPIC * MAXPIC] for each coordinate in the case of adaptive
   histogram, regardless of any histogram data
*/
    visible[_X][_START] = (objHistRange [_X][_END] - objHistRange [_X][_START])/
      (double)(xyHistoRange.end.x - xyHistoRange.start.x) *
      (double) xyVisible.start.x  +  objHistRange [_X][_START];
    visible[_X][_END] = (objHistRange [_X][_END] - objHistRange [_X][_START]) /
      (double)(xyHistoRange.end.x - xyHistoRange.start.x) * 
      (double) xyVisible.end.x + objHistRange [_X][_START];  
    visible[_Y][_START] = (objHistRange [_Y][_END] - objHistRange [_Y][_START])/
      (double)(xyHistoRange.end.y - xyHistoRange.start.y) *
      (double) xyVisible.start.y +  objHistRange [_Y][_START];
    visible[_Y][_END] = (objHistRange [_Y][_END] - objHistRange [_Y][_START]) /
      (double)(xyHistoRange.end.y - xyHistoRange.start.y) *
      (double) xyVisible.end.y + objHistRange [_Y][_START];
            
    d=matrixMultVector(transposeMatrix(dMap->rotMatr),makeVector(1,0));
    if (d.x==0)
    {
        vX = _Y;
        if (d.y == 1)
        {
            vXStart = _END;
            vXEnd = _START;
        }
        else
        /* d.y == -1 */
        {
            vXStart = _START;
            vXEnd = _END;
        }
    }
    else
    {
        vX = _X;
        if (d.x == 1)
        {
            vXStart = _END;
            vXEnd = _START;
        }
        else
        /* d.x == -1 */
        {
            vXStart = _START;
            vXEnd = _END;
        }
    }
    segX.x1 = dMap->vertex.x;
    segX.x2 = dMap->vertex.x - dMap->map.x.x;
    segX.y1 = dMap->vertex.y;
    segX.y2 = dMap->vertex.y - dMap->map.y.x;
    labSeg = makeLabeledSegment( fs, &segX,visible[vX][vXStart],
      visible[vX][vXEnd],
      FALSE, FALSE, CLIP_VERTICAL, CLIP_NONE, NULL, 0, 0);
/* move tic data for X drawing to oD->..., left data for PostScript
   in labSeg->...							*/      
    oD->nXTics = labSeg->nTics;
    oD->xTics  = labSeg->tics;
    labSeg->tics = NULL;
    oD->xTicStruct = NULL;
    lT2 = labSeg->labels;
    lT = &(oD->xLabels);
    while (lT2 != NULL)
    {
        MYMALLOC(labelsToDraw, (*lT),1);
	(**lT).x=lT2->wX;
	(**lT).y=lT2->wY;
/* make a private copy of ASCII label					*/
	MYMALLOC(char,((**lT).label),(lT2->length+1));
	strcpy((**lT).label,lT2->label);
	(**lT).length=lT2->length;
	(**lT).sidePlacing = labSeg->sidePlacing;
	lT = &((**lT).next);
        lT2 = lT2->next;
    }
    *lT = NULL;
    oD->xSegment = labSeg;

    d=matrixMultVector(transposeMatrix(dMap->rotMatr),makeVector(0,1));
    if (d.x==0)
    {
        vY = _Y;
        if (d.y == 1)
        {
            vYStart = _START;
            vYEnd = _END;
        }
        else
        /* d.y == -1 */
        {
            vYStart = _END;
            vYEnd = _START;
        }
    }
    else
    {
        vY = _X;
        if (d.x == 1)
        {
            vYStart = _START;
            vYEnd = _END;
        }
        else
        /* d.x == -1 */
        {
            vYStart = _END;
            vYEnd = _START;
        }
    }
    segY.x2 = dMap->vertex.x;
    segY.x1 = dMap->vertex.x - dMap->map.x.y;
    segY.y2 = dMap->vertex.y;
    segY.y1 = dMap->vertex.y - dMap->map.y.y;
    labSeg = makeLabeledSegment( fs, &segY,visible[vY][vYStart],
      visible[vY][vYEnd],
      FALSE, FALSE, CLIP_HORIZONTAL, CLIP_VERTICAL, NULL, 0, 0);
/* move tic data for X drawing to oD->..., left data for PostScript
   in labSeg->...							*/      
    oD->nYTics = labSeg->nTics;
    oD->yTics = labSeg->tics;
    labSeg->tics = NULL;
    oD->yTicStruct = NULL;
    lT2 = labSeg->labels;
    lT = &(oD->yLabels);
    while (lT2 != NULL)
    {
        MYMALLOC(labelsToDraw, (*lT),1);
	(**lT).x=lT2->wX;
	(**lT).y=lT2->wY;
/* make a private copy of ASCII label					*/
	MYMALLOC(char,((**lT).label),(lT2->length+1));
	strcpy((**lT).label,lT2->label);
	(**lT).length=lT2->length;
	(**lT).sidePlacing = labSeg->sidePlacing;
	lT = &((**lT).next);
        lT2 = lT2 ->next;
    }
    *lT = NULL;
    oD->ySegment = labSeg;
}

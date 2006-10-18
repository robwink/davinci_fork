#ifndef IMAGGEN_H
#define IMAGGEN_H
#define MAIN_BIN_WIDTH MAXPIC /* width of main bin. 		*/
/*
** Bins are drawing in cells MAXPIC x MAXPIC virtual units, 
** so it should be 0 < MAIN_BIN_WIDTH <= MAXPIC 
*/
#define ERROR_BIN_WIDTH MAXPIC/4 /* width of main bin. 		*/
/*
** Bins are drawing in cells MAXPIC x MAXPIC virtual units, 
** so it should be 0 < ERROR_BIN_WIDTH <= MAIN_BIN_WIDTH 
*/

typedef struct _discreteData{
    short data;			/* scaled bin height  			*/
    Boolean clipped;		/* TRUE if bin was clipped 		*/
} discreteData;

typedef struct _binPosDescr {
    int nXPoints;		/* number of bins vertexes along 
    				   virtual X axis (X after rotation) 	
    				   (number of bins in X row +1)		*/
    int nYPoints;		/* the same for Y axis			*/
    int *xSteps;		/* array with size 2 * nXPoints - 1, 
    				   keeps 
    				   distances between neighbouring
    				   bin vertexes  along virtual X axis,
    				   starting from the nearmost vertex of
    				   visible histogram base. One unit is
    				   <bin> /MAXPIC. Elements with even
    				   index (0, 2, 4 ...) correspond to
    				   distance between beens (possibly 0),
    				   and with odd - to bin widths		*/
    int *ySteps;		/* the same for Y axis			*/
    int lx;			/* length of visible part of virtual
    				   X axis in  <bin> /MAXPIC units	*/
    int ly;			/* the same for Y axis			*/
} binPosDescr;


void makeEdgePoints 
    ( range2d r2, discreteMap *dMap, 
     vector *lengths,
    vector **xEdges, vector **yEdges);
void makeBinPosDescr (range2d r2, discreteMap *dMap, 
    int binWidth, binPosDescr *binDescr);

void makeImage(Hist2DWidget w);
discreteData * rescaleZ(float *data, int nYBins, discreteMap *dMap, 
		range2d xyVisibleRange, float zVisibleStart,
		float zVisibleEnd, Boolean zLogScaling, int width);
#endif

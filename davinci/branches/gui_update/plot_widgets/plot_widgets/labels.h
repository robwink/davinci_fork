#ifndef LABELS_H
#define LABELS_H

#define ARROW_LENGTH 7
#define ARROW_ANGLE 0.4

#define SHORT_TIC_LEN 4		/* length in pixels for short axis tic marks */
#define MED_TIC_LEN 6		/* length in pixels for medium axis tic marks */
#define LONG_TIC_LEN 8		/* length in pixels for long axis tic marks.
				   if too short, large fonts may collide at
				   the intersection of the v and h axes */
#define MIN_TIC_SPACING 5	/* minimum interval in pixels for axis tics */
#define BIN_TIC_THRESHOLD 4	/* minimum spacing of tics labeling bin edges
				   before ignoring bin edges in choosing tics */ 
#define MIN_V_LABEL_SPACING 1	/* vertical space to leave between labels in
				   (font dep.) units of character height */
#define MIN_H_LABEL_SPACING 1	/* horizontal space to leave between labels
				   in units of maximum character width */
/* number of digits in which two neigbor values should deffer after rounding 
*/
#define PRECISION 2
/* number of valuable digits in a label if the algorithm fail to determaine it 
** by itsel,f i.e. if three neighbor values are exactly the same	
*/
#define DEFAULT_PRECISION 6
/* printf makes "usual" rounding rather then truncation	for float          */
#define PRINTF_ROUND

typedef struct _lCS {			/* Tic marks labels control 
					   structure 			*/
    short defaultPrecision;		/* Max number of valuable digits
    					   in a marks in ane case 	*/    
    short precision;			/* Number of digits in which 
    					   neigbor marks should deffer 	*/
    Boolean dynamicRounding;		/* TRUE - Lable numbers are rounding
    					   according current scale &
    					   <Precision> as short as possible,
    					   FALSE - The precision needed to
    					   differ two neigbor beans is
    					   used for any scale		*/
    Boolean trailingZeros;		/* TRUE - keep, FALSE -skip	*/
    Boolean dotAlignment;		/* TRUE - use alingment by decimal
    					   dot (as for colomn printing)
    					   FALSE - use center alingment */
} lCS;  

typedef struct _ticLabel 
{
    short XShift;	/* relative shift in pixels for alingment purposes, 
    			   (from the beginning of the field with lenght=
    			           , see <typedef labelTable>) 
    			          0<= Xshift<=				*/
    int length;         /* ASCII length 				*/
    char *label;
} ticLabel;

typedef struct _labelTable
{
    unsigned n;
    unsigned step;
    unsigned start;
    ticLabel *labels;
    short XWidth;	/* widest labelwidth in pixels, using current font */
    struct _labelTable *next;
} labelTable; 


typedef struct _labelToDraw {
    int XWidth;				/* for use in axes name labels only */
    char *label;
    int length; 			/* ASCII characters */
    int x;				/* origin coordinates in the window */
    int y;
    int yCenter;			/* is used for Z labels only to provide
    					   backplane generation             */
    Boolean sidePlacing;		/* side or down placing, for PS printing
    					   only      			    */
        					   
    struct  _labelToDraw *next;
} labelsToDraw;


typedef struct _objectsToDraw {
        labelTable *zLabelTable;
	XPoint *template;
	int templateSize;
	XSegment *zAxis;
	XSegment *xTics;
	ticStruct *xTicStruct;
	int nXTics;
	XSegment *yTics;
	ticStruct *yTicStruct;
	int nYTics;
	XSegment *zTics;
	ticStruct *zTicStruct;
	int nZTics;
	XSegment *arrows;
	int nArrows;
	labelsToDraw *xLabels;
	labelsToDraw *yLabels;
	labelsToDraw *zLabels;
	labeledSegment *xSegment;
	labeledSegment *ySegment;
	labelsToDraw *xName;	
	labelsToDraw *yName;
	labelsToDraw *zName;	
	XSegment *backPlanes;
	int nBackPlanes;
	labelsToDraw *message;
} objectsToDraw;


void destroyLabelTable(labelTable *lT);
void makeXYLableTables(Hist2DWidget w);
void initialazeBaseLCS(Hist2DWidget w);
void makeMargins(Hist2DWidget w);
void makeBaseLabels (XFontStruct *fs, discreteMap *dMap, 
    labelTable *xlT, labelTable *ylT, range2d visible, objectsToDraw *oD);
labelsToDraw *makeNameLabels (char *name, XFontStruct *fs);
void destroyLabelsToDraw (labelsToDraw *lTD); /* full destruction */
void makeNameLabelsToDraw (Hist2DWidget w, objectsToDraw *oD);
void makeMessageToDraw (Hist2DWidget w, objectsToDraw *oD);
void makeArrows (Hist2DWidget w, objectsToDraw *oD);
#endif

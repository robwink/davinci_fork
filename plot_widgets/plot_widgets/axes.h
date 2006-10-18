#ifndef _AXES_H
#define _AXES_H

#define ARROW_LENGTH 7
#define ARROW_ANGLE 0.4

typedef enum _actionType { A_NONE, A_SCALING,  A_PANNING}
   actionType;          

typedef struct _axis {
    float minLimit;		/* limits in object space, for use
    				   as dragging limits		 	*/
    float maxLimit;
    float minVisible;		/* beginning of visible range in the object
    				   space				*/
    float maxVisible;		/* beginning of visible range in the object
    				   space				*/
    float* visible0;		/* pointer to either <minVisible> or 
    				   <maxVisible> - to the
    				   value in the object space which corresponds 
    				   to the beginning (first point) of <seg> */
    float* visible1;		/* pointer to either <minVisible> or 
    				   <maxVisible> - to the 
    				   value in the object space which corresponds 
    				   to the end (second point) of <seg> 	*/
    XSegment axisImage;  	/* image of the axis in the window. 
    				   Should be in consistence with the 
    				   order of <visible0>,<visible1>,
				   see above. The lables and tics will be
				   generated in the semi-plane which is
				   on the right side while going from
				   the first point to the second	*/
    Boolean mapped;		/* auxilary flag to prohibit axis display
    				   (when FALSE) without removing the 
    				   structure				*/		   
    XFontStruct *fs;		/* font structure for lables		*/				   
    labeledSegment *curSegment; /* current state of labeled segment
    				   which has been drawn on the screen	*/
    labeledSegment *newSegment; /* new (requested) state of labeled segment,
    			           corresponding to the current state
    			           of axis parameters and is been drawing
    			           for picture update			*/
    XSegment *newArrows;	/* segments of axis ends arrows		*/
    int newNArrows;		/* number of segments in the array above*/
    XSegment *curArrows;	/* segments of axis ends arrows		*/
    int curNArrows;		/* number of segments in the array above*/
       			           
    vector oldCursor;		/* cursor position marked at the
    				   beginning of dragging		*/
    float markedMinVisible;       /* value of <visible1> sved at the
    				   beginning of the dragging cycle	*/
    float markedMaxVisible;       /* the same for <visible2>		*/
    actionType action;		/* current action is performing with 
    				   the axis				*/
    vector markedVector;	/* point in the windows corresponding to
    				   one of the <seg> ends, marked as 
    				   motionless at the beginning of the
    				   scaling dragging cycle		*/
    Boolean logScale;		/* Log or linear scaling		*/
    clipType clip0;		
    clipType clip1;		/* how to clip labels at the end 
    				   of the segment to avoid possible labels
    				   overlapping with other picture elements or
    				   other segment labels			*/
    int clipX0;			/* X interval to place one line label name */
    int clipX1;    				   
    Boolean changed;    	/* TRUE means, that some changes were
    				   made in the contents of the object,
    				   so it is no more in consistens with
    				   <curSegment>.
    				   Setted to FALSE by any "set" or
    				   "dragging"	function
    				   and to TRUE by <saveAxisImage>	*/ 
    char *name;			/* unformatted ASCII axis name		*/
    XRectangle *nameClipRectangle; /* area to place the name into (aside
    				   with the restrictions caused by axis
    				   position/axis labels)		*/     				   
    				   			   
} axis;    

axis *createAxis(XFontStruct *fs, float minLimit, float maxLimit, 
    float minVisible, float maxVisible, XSegment *seg, Boolean logScale, 
    clipType clip0, clipType clip1, Boolean reverse, Boolean mapped,
    char *name, XRectangle *nameClipRectangle );
axis *createDummyAxis(void);
void destroyAxis(axis *a);
    
void setAxisDefaultNameClip(axis *a);
void setAxisLabelsFont(axis *a, XFontStruct *fs);
void setAxisName (axis *a, char *name);
void setAxisNameClipRectangle (axis *a, XRectangle *nameClipRectangle);
void setAxisLimitRange(axis *a, float min, float max);     
/* "resize at max" without changing visible range */
void expandAxisLimitRange(axis *a, float min, float max);
/* "resize at max" with visible range "resize at max" */
void setAxisLimitVisible (axis *a, float min, float max);
/* "grow-only" with visible range "resize at max" */
void expandAxisLimitVisible (axis *a, float min, float max);
void setAxisMapped(axis *a);
void setAxisUnMapped(axis *a);
void setAxisVisibleRange(axis *a, float minVisible, float maxVisible);
void setAxisImageSegment(axis *a, XSegment *seg);
void setAxisImageSegmentReverse(axis *a, XSegment *seg);
void setAxisClipping(axis *a, clipType clip0, clipType clip1);
void setAxisLinearScale(axis *a);
Boolean setAxisLogScale(axis *a);
Boolean setAxisLogScaleWVerge(axis *a, float verge);

void getAxisLimits(axis *a, float *min, float *max);
void getAxisVisibleRange(axis *a, float *minVisible, float *maxVisible);

Boolean markAxisState(axis *a, XEvent *event);
Boolean dragAxis(axis *a, XEvent *event);
Boolean stopAxisDragging(axis *a, XEvent *event);

void zoomAxis (axis *a, float factor);
void zoomAxisWVerge(axis *a, float factor, float verge);
void resetAxisView(axis *a);

void drawNewAxis(Display *d, Drawable dr, GC gc, axis *a);
void drawCurAxis(Display *d, Drawable dr, GC gc, axis *a);
void saveAxisImage(axis *a);
void makeNewAxisImage(axis *a);

/* function from axes_ps.c file			*/
void makePSAxis (axis *a, FILE *psfile ); 
void makePSArrows (axis *a, FILE *psfile );
void makePSTics (labeledSegment *lS, FILE *psfile );
void makePSLabels (XFontStruct * fs, labeledSegment *lS, FILE *psfile );
void makePSName (XFontStruct * fs, labeledSegment *lS, FILE *psfile );
#endif

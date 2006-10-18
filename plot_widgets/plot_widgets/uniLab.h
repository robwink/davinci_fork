#ifndef _UNILAB_H
#define _UNILAB_H

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
#define MIN_LOG_TIC_SPACING 2	/* minimum interval in pixels for narrowest
				   interval between tics in nonlinear
				   placing 				*/

#define MIN_LOG_V_LABEL_SPACING 1 /* narrowest vertical space to leave between 
				    labels in (font dep.) units of character 
				    height in  nonlinear placing 	*/
#define MIN_LOG_H_LABEL_SPACING 1 /* narrowest horizontal space to leave between
				     labels in units of maximum character 
				     width */

typedef enum _clipType { CLIP_NONE, CLIP_VERTICAL, CLIP_HORIZONTAL} clipType;

    					     
typedef struct _ticStruct {
    double value;        /* value marked by the tic in object
    			   coordinates (for readjustment 
    			   with better   resolution - printing)  	*/
	
    int length;
    int bar;
    struct _labelsToDraw2 * label;	/* pointer to a label description	*/
} ticStruct;

typedef struct _labelsToDraw2 {
    char *label;		/* pointer to ASCII representation of the
    				   label				*/
    int length; 		/* in ASCII characters 			*/
    int XWidth;			/* width in window pixcels, using current
           			   font. Optional, for use in axes name 
    			           labels only 				*/
    double lValue;		/* can be used for tic/label readjustment 
        			   during resolution changes (printing)	*/
    int wX;			/* origin coordinates in the window 	*/
    int wY;
    int ticNumber;		/* index of parameters for corresponding
    				   tic in <labeledSegment>.ticPrms array*/
    struct  _labelsToDraw2 *next;
} labelsToDraw2;

/* ticStruct is defined in labels.h in the same way now			*/
/*typedef struct _ticStruct {*/
/*    float value;		*//* value marked by the tic in object*/
/*    				*//*    coordinates (for readjustment */
/*    				*//*    with better   resolution - printing)*/	
/*    int length;		*/	/* tic length in window pixcels	*/	
/*    int bar;			*//* number of a bar the tic is attached to*/
/*    				*//*    in the binned labeling (for readjustment*/
/*    				*/ /*   with better   resolution - printing)	*/
/*    labelsToDraw2 * label;	*//* pointer to a label description	*/
/*} ticStruct;			*/

typedef struct _labeledSegment {
    XSegment axisImage;		/* axis segment in windows coordinates  */
    double v0, v1;		/* in the case of linear scaling, v0 and v1 are
    				   values in the object space, corresponding 
    				   to the beginning and the end of
    				   <axisImage>.
    				   in the case of log scaling, v0 and v1 are
    				   log10's of the values in the object space, 
    				   corresponding to the beginning and the end 
    				   of <axisImage>. 			*/    
    Boolean logScaling;		/* Log or linear  scale			*/
    Boolean binnedLabeling;	/* binned or unbinned labeling, affects
    				   the way to make further tics 
    				   readjustment also.
    				   (using bar number of value)		*/ 
    clipType clip0, clip1;	/* how to clip labels at the end 
    				   of the segment to avoid possible labels
    				   overlapping with other picture elements or
    				   other segment labels			*/
    				   
				/* parameters above are input parameters
				   for label calculating functions,
				   parameters below are results of label
				   calculation				*/

    Boolean sidePlacing;	/* TRUE if labels are placed left or
     				   right from the tics (otherwise
     				   up or down placing			*/
    int nTics;			/* number of tics			*/
    XSegment *tics;		/* array of tics in windows coordinates,
    				   array representation is used for
    				   faster X drawing			 */
    ticStruct *ticPrms;		/* array of tics desription, this
    				   parameters might be useful for fine
    				   tics readjustment to avoid aliasing
    				   while printing with better resolution*/
    labelsToDraw2 *labels;	/* linked list of labels, each element has 
    				   additional information for further
    				   processing/readjustment if needed    */
    labelsToDraw2 *name;	/* linked list of ASCII lines with additional
    				   placement information, representing
    				   formatted segment (axis) name.
    				   Not all fields of the structure might
    				   be used. */ 
/* parameters for one element (one string) name PostScript
   readjustment								*/
   int nameSidePlacing;
   int ortDirection;
   int clipX0;
   int clipX1;
   int centerX;
} labeledSegment;   				   


labeledSegment* makeLabeledSegment(XFontStruct *fs, XSegment *seg, 
  float v0, float v1,
  Boolean logScaling, Boolean binnedLabeling, clipType clip0, clipType clip1,
  char *name, int clipX0, int clipX1);
void drawLabeledSegment(Display *d, Drawable dr, GC gc, labeledSegment* lS);
void destroyLabeledSegment(labeledSegment* lS);
void destroyLabelsToDraw2(labelsToDraw2 *labels);
#endif

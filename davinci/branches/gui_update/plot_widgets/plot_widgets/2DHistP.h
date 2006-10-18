#ifndef _2DHISTP_H
#define _2DHISTP_H

#define LOWBITS 10
#define MAXPIC (1<<LOWBITS)
#define MARGIN 5
#include "2DHistMalloc.h"
#if XmVersion >= 2000
#include <X11/CoreP.h>
#include <Xm/PrimitiveP.h>
#endif

typedef enum _motionType {ROTATION, X_SCALING, X_PANNING, 
    Y_SCALING, Y_PANNING, 
    Z_LOW_SCALING, Z_PANNING, Z_HIGH_SCALING, NONE} motionType;

/*
   The widget parameters determine object position in 3D space and associated 
   projection as
   
   WARNING: this is not true in the case of adaptive histogram,
   which works in different way!!
   
   X'= (cos(fi) * (x-xVisibleStart)/xVisibleLength - 
   	sin(fi) * (y-yVisibleStart)/yVisibleLength ) * cubeSize
   	
   -Y'= (-sin(psi) * (sin(fi) * (x-xVisibleStart)/xVisibleLength +
                    cos(fi) * (y-yVisibleStart)/yVisibleLength) ) + 
	cos(psi) * (z-zVisibleStart)/zVisibleLength) ) * cubeSize
	
   where X' and Y' is windows (relative) plane coordinates and x, y are
   mesured "in bins" and z - in original z units.
    xVisibleStart=xyVisibleRange.start.x;
    xVisibleLength=xyVisibleRange.end.x-xyVisibleRange.start.x+1;
    yVisibleStart=xyVisibleRange.start.y;
    yVisibleLength=xyVisibleRange.end.y-xyVisibleRange.start.y+1;
   To provide proper image layout image in X' & Y' is shifted so that
   the point (x - xVisibleStart + xVisibleLength/2, 
   	      y - yVisibleStart + yVisibleLength/2,
   	      x - zVisibleStart + zVisibleLength/2)
   lies in the center of rectangle which rests after "substracting" of
   left, right, top & bottom margins from the window.
   	All the parameters in above equation, but the "cubeSize", are considered
   as "given" and should be provided from the outside of the widget and/or
   are  results of the action routines calls.
   	The cubeSize is a member of discreteMap structure and is calculated
   in the way that the histogram image fit the window given.
   
   discreteMap determine projection and scaling of 3D object into window as:

   MAXPIC * X = a * XFactor * MMULTX(rotMatr,x,y) + 
                b * YFactor * MMULTY(rotMatr,x,y) +
                vertex.x - a * mapXLength - b * mapYLength; (shift in window)

   MAXPIC * Y = p * XFactor * MMULTX(rotMatr,x,y) + 
                q * YFactor * MMULTY(rotMatr,x,y) + 
                r * z  +
                vertex.y - p * mapXLength - q * mapYLength; (shift in window)

   where X,(-Y) - window coodinates, x,y,z - 3D coordinates (x,y - bins' edges 
   No, and z is a member of discreteHistogram->data array and is orignal z
   scaled by makeDiscreteHistogram function in such way that the segment
   [zVisibleStart, zVisibleStart+zVisibleLength] in original z coordinates
   is mapped to integer segment [0, MAXPIC-1]. 
   	All parameters in the last equations are members of discreteMap
   structure and are integers. 
      (vertex.x, vertex.y are the windows coordinates of the image of nearmost 
   (in 3D) vertex of the histogram base. This point is used to tie image
   in relative window coordinates to absolute window coordinates (determine
   shift). Manipulation of this point could be used to chandge the image
   position in the window.
      MMULTX(), MMMULTY()  is coordinates of 2 dimension rotMatr matrix and 
   (x,y) vector product.
   rotMatr can determine only rotation by 0, 90, 180 or 270 degrees in (x,y)
   plane. It is choosen in shuch a way that the zero point of coordinate
   system x~=mMultX(x,y) y~=mMultY(x,y) is in the farmost vertex of
   histogram base, and provides that 
   	a<=0; b>=0; p<=0; q<=0; r>=0; in all cases
   The last assumption is very convinient for plane image genration,
   beside this rotMatr is used is some more geometry calculation separately.
   	All the parameters of discreteMap structure are produced by 
   makeDiscreteMap function exclusevely from the given "source" parameters
   and are never changed by any other function. 
   	All other functions, which provide image generation and drawing, are
   based on the discreteMap parameters and don't use any source parameters.
   So the best way to change image layout is to chande makeDiscreteMap function.
    
*/



#define XDESTROYIMAGE(XIMAGE) if (XIMAGE!=NULL) XDestroyImage(XIMAGE)


typedef struct _hist2DClassPart{
    int ignore;
} hist2DClassPart;

typedef struct _hist2DClassRec{
    CoreClassPart  		core_class;
    XmPrimitiveClassPart 	primitive_class;
    hist2DClassPart  		hs2D_hist_class;
} Hist2DClassRec;

extern Hist2DClassRec hs2DhistClassRec;
        

typedef struct _hist2DPart {
    h2DHistSetup	*sourceHistogram; 	/* histogram to handle 		*/
    float	*bins;			/* Normally bins==SourceHistogram.bins,
    					   but we provide this field separately
    					   to have cooresponding Xm resource 
    					   for implementation "update" function 
    					   through SetValue interface   */
    float	*topErrors;		/* Histogram error datum, the array
    					   should have the same size as <bins>
    					   of be NULL			*/
    float	*bottomErrors;   	/* Same as above, but bottom valuse */
    aHistStruct	*aHist;			/* Pointer to adaptive histogram if
    					   adaptive histogram is setted up  */
    range2d     xyVisibleRange;    	/* visible x-y range  (in bins)	*/
    float	zVisibleStart;		/* start of visible z range and	*/
    float	zVisibleEnd;		/* end of visible z range (in z units)*/
    Boolean	zLogScaling;		/* Log z scale or linear	*/
    Boolean 	binEdgeLabeling;	/* put x & y labels at bin edges
    					   only or elsewhere to use more
    					   even numbers			 */
    Boolean doubleBuffer;	/* When set, draw first to offscreen pixmap */
    double 	fiViewAngle;		 
    double 	psiViewAngle;		
    Boolean 	backPlanesOn;		
    Boolean 	shadingOn; 		/* wireframe or shaded image	*/
    Boolean     errorBarsOn;
    Pixel	topPlaneColor; 		/* 				*/
    Pixel	leftPlaneColor; 	/* 				*/
    Pixel	rightPlaneColor;	/* colors for shaded model	*/
    Pixmap	leftPlanePixmap; 	/* 				*/
    Pixmap	rightPlanePixmap;	/* colors for shaded mod	*/
    Pixel	clippingColor;	/* 				*/ 	
    XmFontList 	font;
    
    /* all other fields are inner and do not accessible as resources 	*/
    Pixmap drawBuffer;		/* double buffering for non-flashing draws */
    range2d     xyHistoRange;    	/* Histogram x-y range (in bins)*/
    range2d     markedXYRange;		/* previous state of x-y visible range 
    					   saved for control purposes	*/
    double 	markedZVisibleStart;	/* previous start of visible z range 
    					   saved for control purposes		*/
    double 	markedZVisibleEnd;	/* previous end of visible z range 
    					   (in z units) */
    double 	markedFiViewAngle;		 
    double 	markedPsiViewAngle;		
    vector	markedBin;    	
    vector	markedVector;				   		   
    lCS	baseLCS;	/* label control structure for X & Y axes lables*/
    			/* we don't use a pointer to keep possibility to
    			   convert it to user controlled resorce	*/ 
    XFontStruct *fs;	/* current font structure 			*/
    int rightMargin;
    int leftMargin;
    int topMargin;
    int bottomMargin;
    discreteMap 	*discrMap;	/* preprocessed map 		*/
    labelTable *xLabels;
    labelTable *yLabels;
    labelsToDraw *xNames;    
    labelsToDraw *yNames;
    labelsToDraw *zNames;
    GC gc;
    GC inverseGc;
    GC leftGc;
    GC rightGc;
    GC topGc;
    GC clippingGc;
    GC backPlaneGc;
    GC inverseBackPlaneGc;
/* TMP, should be removed from final version */
    float mA, mB;
/* end TMP */   
    float      zMin;
    float      zMax;
    XImage     *wireImage;
    XImage     *leftImage;
    XImage     *rightImage;
    XImage     *topImage;
    XImage     *clippingImage;
    objectsToDraw *currentPicture;
    int	       xImageStart;
    int        yImageStart;
    Boolean    templateDragging;
    Boolean    badPicture;
    motionType motion;
    vector     oldCursor;
    XEvent     lastEvent;
    XtCallbackList resize;		/* callbacks 			*/
    XtCallbackList btn2;	
    XtCallbackList btn3;
    XtCallbackList redisplay;
} hist2DPart;

typedef struct _hist2DRec {
   CorePart        core;
   XmPrimitivePart primitive;
   hist2DPart    hist2D;
} hist2DRec;

h2DHistSetup *copyHistogram (h2DHistSetup *hist);
void makeAllGraphics(Hist2DWidget w);
void resetHist(Hist2DWidget w);
void redisplay(Widget wg, XEvent *event, Region region );
void destroySrcHist(h2DHistSetup *hist);
Boolean adjustViewAngles(double *fi, double *psi);
void  drawNewPicture (Hist2DWidget w);

/* prototypes of system routines missing from include files */
double rint(double);
 

#endif

#ifndef _3DSCATP_H
#define _3DSCATP_H
#if XmVersion >= 2000
#include <X11/CoreP.h>
#include <Xm/PrimitiveP.h>
#endif

typedef struct _scat3DClassPart{
    int ignore;
} scat3DClassPart;

typedef struct _scat3DClassRec{
    CoreClassPart  		core_class;
    XmPrimitiveClassPart 	primitive_class;
    scat3DClassPart  		scat3D_class;
} Scat3DClassRec;

extern Scat3DClassRec scat3DClassRec;
        

typedef struct _scat3DPart {
    Boolean     doubleBuffer;	/* When set, draw first to offscreen pixmap */
    Boolean     darkerPoints;	       /* When set, draw points 2 pixels wide 
    					   while not dragging, main parameter
    					   for external control*/
    					/* Internal derivative parameters 
    					   for pixels wide control:*/   
    Boolean     newDarkerPoints;	/* When set, draw points 2 pixels wide 
    					   in new state */
    Boolean     curDarkerPoints;	/* When set,  points is drawn 2 pixels 
    					   wide  in current state	*/
    Boolean	dragging;	       /* TRUE while in dragging cycle  */  					   
    Boolean     curUseBitmap;	       /* When set,  points is drawn using
    					  bitmap (so defferent procedure
    					  to erase them)		*/
    Boolean     newUseBitmap;	       /* When set,  draw points using
    					  bitmap in new state		*/
    Boolean 	bitmapStrategyAuto;    /* When set, the widget will switch
    					  between point drawing and
    					  bitmap drawing modes automatically,
    					  based on the amount of data and
    					  ignore explicit "useBitmap" 
    					  settings 			*/   					  
    XImage *	plotImage;		/* image in bitmap drawing mode  */    					  
    Boolean	xLogScaling;		/* Log x scale or linear	*/
    Boolean	yLogScaling;		/* Log y scale or linear	*/
    Boolean	zLogScaling;		/* Log z scale or linear	*/
    Pixel	dataColor; 		/* 				*/
    Pixel	axesColor;		/* 				*/ 	
    Pixel	labeledEdgeColor;	/* colors  			*/
    Pixel	unLabeledEdgeColor;	/* 				*/ 	
    Pixel	frontEdgeColor; 	/* 				*/
    Pixel	backEdgeColor; 		/* 				*/
    int	backEdgeDashes; 	/* for dashed back edges	*/
    int		backEdgeDashOffset; 	/* for dashed back edges	*/
    XmFontList 	font;
    
    /* all other fields are inner and do not accessible as resources 	*/
    XFontStruct *fs;	        /* current font structure        	*/
    int rightMargin;
    int leftMargin;
    int topMargin;
    int bottomMargin;
    Scat3DPoint *scatData;
    int nPoints;
    XPoint *curScaledData;
    int nCurPoints;
    XPoint *newScaledData;
    int nNewPoints;
    Pixmap drawBuffer;		/* double buffering for non-flashing draws */
    Boolean newStateTemplate;
    Boolean curStateTemplate;
    Boolean dataMapChanged;
    GC gc;
    GC eraseGc;
    GC dataGc;
    GC frontEdgeGc;
    GC backEdgeGc;
    GC labeledEdgeGc;
    GC unLabeledEdgeGc;
    GC axesGc;

    cubeDescr *cd;
    axis * axis [3] ;  
    XEvent     lastEvent;
    XtCallbackList resize;		/* callbacks 			*/
    XtCallbackList btn2;	
    XtCallbackList btn3;
    XtCallbackList redisplay;
} scat3DPart;

typedef struct _scat3DRec {
   CorePart        core;
   XmPrimitivePart primitive;
   scat3DPart      scat3D;
} scat3DRec;

#endif

/*******************************************************************************
*									       *
* 3DScat.c -- 3D coordinate scat3D control widget - main operations	       *
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
#include <X11/Xutil.h>
#include <Xm/Xm.h>
#include <Xm/DrawingA.h>
#include <Xm/XmP.h>
#if XmVersion == 1002
#include <Xm/PrimitiveP.h>
#include <Xm/DrawP.h>
#endif
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <stdio.h>
#include <time.h>
/* for whatever reason bstring.h does not exist on ULTRIX machine  
   workaround is used instead of bzero()           
#include <bstring.h>
*/
#include "2DGeom.h"
#include "3DGeom.h"
#include "3DRot.h"
#include "3DPara.h"
#include "3DRotCube.h"
#include "uniLab.h"
#include "axes.h"
#include "geometry.h"
#include "3DScat.h"
#include "3DScatP.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* minimal
   margins (in pixels) between rotating cube frame and Motif shadow frame 
   or top and bottom axis names lines (if any) 				*/
#define MARGIN 5
/* axis indices assignment						*/
#define _X 0
#define _Y 1
#define _Z 2
/* default visible minimum when using Log scale				*/
#define LOG_SCALE_VERGE 0.5
/* formula for deciding between sparse (XDrawPoints)
   and dense (bitmap) drawing modes */
#define USE_BITMAP(width, height, n) ((width)*(height)/8 < 4*(n)*sizeof(XPoint))
/* Amount of space to leave on the edge of the printer page in 72nds of an
   inch.  If this is too big, part of the page will be wasted, too small,
   and some (espescially color) printers will clip off part of the plot */
#define PAGE_MARGIN 18

#define EPSILON 1.0E-4

#define RADIANS(x)  (M_PI * 2.0 * (x) / 360.0)
#define DEGREES(x)  ((x) / (M_PI * 2.0) * 360.0)

/* Scat3DWidget methods  						*/ 
static void initialize(Widget Request, Widget New, ArgList args, 
  Cardinal*  num_args);
static Boolean setValues(Widget Current, Widget Request, 
  Widget New, ArgList args, Cardinal*  num_args);
static void destroy(Widget wg);
static void resize(Widget wg);
static void redisplay(Widget wg, XEvent *event, Region region );
/* action routines							*/
static void markGeneric(Scat3DWidget w, XEvent *event, char* args, int n_args);
static void mark(Scat3DWidget w, XEvent *event, char* args, int n_args);
static void markMod(Scat3DWidget w, XEvent *event, char* args, int n_args);
static void dragMotionGeneric(Scat3DWidget w, XEvent *event, char* args, 
  int n_args);
static void dragMotion(Scat3DWidget w, XEvent *event, char* args, int n_args);
static void dragMotionMod(Scat3DWidget w, XEvent *event, char* args, 
  int n_args);
static void stopDragging(Scat3DWidget w, XEvent *event, char* args, int n_args);
/* inner service routines						*/
static void findDataLimits(Scat3DPoint * data, int nPoints, 
  float *xMin, float *xMax, 
  float *yMin, float *yMax,
  float *zMin, float *zMax);
static Scat3DPoint * copyData(Scat3DPoint * data, int nPoints);
static void makeNewDataImage(Scat3DWidget w, paraState *cubeFrame);
static void saveDataImage(Scat3DWidget w);
static XPoint *mapData(Scat3DPoint * data, int nPoints, axis *axes[3],
  paraState *cube,  int *nXPoints);
static void bitMapData(Scat3DPoint * data, int nPoints, axis *axes [3],
  paraState *cube,  XImage* image, Boolean thiken);
static void mapAxes(Scat3DWidget w, paraState *cubeFrame);
static void updateBufferAllocation(Scat3DWidget w);
static void updateImageAllocation(Scat3DWidget w);
static int makeCubeSize(Scat3DWidget w);
static vector makeCubeCenter(Scat3DWidget w);
static void makeNewAxesState(Scat3DWidget w);
static Boolean resolveAxesNameCollision(XFontStruct *fs, axis *a1, axis *a2);
/* customized  drawing routines					*/
static void drawContents(Scat3DWidget w);
static void drawNewData(Scat3DWidget w);    
static void drawCurData(Scat3DWidget w);    
static void eraseData(Scat3DWidget w);    
static void drawData(Scat3DWidget w, XPoint *data, int nPoints, GC gc,
  Boolean useBitmap, Boolean darkerPoints);
static void drawNewCubeFrame (Scat3DWidget w);
static void drawCurCubeFrame (Scat3DWidget w);
static void eraseCubeFrame (Scat3DWidget w);
static void drawShadow(Scat3DWidget w);
static void finishDrawing(Scat3DWidget w);

static void makePSData(XPoint *data, int nPoints, FILE *psfile, XImage *image,
    Boolean useBitmap, Boolean darkPoints);
static void makePSDataFromImage(FILE *psfile, XImage *image);
static void eulerAnglesToMatrix(double alpha, double beta, double gamma,
			   double m[3][3]);
static void matrixToEulerAngles(double m[3][3], double *alpha,
			   double *beta, double *gamma);

/*
static void stopAP(Scat3DWidget w, XEvent *event, char *args, int n_args);
*/ 
static void btn2AP(Scat3DWidget w, XEvent *event, char *args, int n_args);
static void btn3AP(Scat3DWidget w, XEvent *event, char *args, int n_args);

static char defaultTranslations[] = 
    "Ctrl <Btn1Motion>: DragMotionMod()\n\
     <Btn1Motion>: DragMotion()\n\
     Ctrl <Btn1Down>: MarkMod()\n\
     <Btn1Down>: Mark()\n\
     <Btn1Up>: StopDragging()\n\
     <Btn2Down>: Btn2Press()\n\
     <Btn3Down>: Btn3Press()\n";

static XtActionsRec actionsList[] = {
    {"Btn3Press", (XtActionProc)btn3AP},
    {"Mark", (XtActionProc)mark}   ,
    {"MarkMod", (XtActionProc)markMod}   ,
    {"DragMotion", (XtActionProc)dragMotion},
    {"DragMotionMod", (XtActionProc)dragMotionMod},
    {"StopDragging", (XtActionProc)stopDragging},
    {"Btn2Press", (XtActionProc)btn2AP}
};


static XtResource resources[] = {
    {XmNdoubleBuffer, XmCDoubleBuffer, XmRBoolean, sizeof(Boolean),
      XtOffset(Scat3DWidget, scat3D.doubleBuffer), XmRString, "False"},
    {XmNdarkerPoints, XmCDarkerPoints, XmRBoolean, sizeof(Boolean),
      XtOffset(Scat3DWidget, scat3D.darkerPoints), XmRString, "True"},      
    {XmNuseBitmap, XmCUseBitmap, XmRBoolean, sizeof(Boolean),
      XtOffset(Scat3DWidget, scat3D.newUseBitmap), XmRString, "True"},
    {XmNbitmapStrategyAuto,XmCBitmapStrategyAuto, XmRBoolean, sizeof(Boolean),
      XtOffset(Scat3DWidget, scat3D.bitmapStrategyAuto), XmRString, "True"},         
    {XmNfontList, XmCFontList, XmRFontList, sizeof(XmFontList),
      XtOffset(Scat3DWidget, scat3D.font), XmRImmediate, NULL},
    {XmNxLogScaling, XmCXLogScaling,XmRBoolean, sizeof(Boolean), 
      XtOffset(Scat3DWidget, scat3D.xLogScaling ), XmRString, "False"}, 
    {XmNyLogScaling, XmCYLogScaling,XmRBoolean, sizeof(Boolean), 
      XtOffset(Scat3DWidget, scat3D.yLogScaling ), XmRString, "False"}, 
    {XmNzLogScaling, XmCZLogScaling,XmRBoolean, sizeof(Boolean), 
      XtOffset(Scat3DWidget, scat3D.zLogScaling ), XmRString, "False"}, 
    {XmNdataColor, XmCDataColor, XmRPixel, sizeof(Pixel),
      XtOffset(Scat3DWidget, scat3D.dataColor ), 
      XmRString,"Black"}, 
    {XmNaxesColor, XmCAxesColor,  XmRPixel, sizeof(Pixel), 
      XtOffset(Scat3DWidget, scat3D.axesColor ), 
      XmRString,"Black"}, 
    {XmNlabeledEdgeColor, XmCLabeledEdgeColor, XmRPixel, sizeof(Pixel),
      XtOffset(Scat3DWidget, scat3D.labeledEdgeColor ), 
      XmRString,"Black"}, 
    {XmNunLabeledEdgeColor, XmCUnLabeledEdgeColor,  XmRPixel, sizeof(Pixel), 
      XtOffset(Scat3DWidget, scat3D.unLabeledEdgeColor ), 
      XmRString,"Black"}, 
    {XmNfrontEdgeColor, XmCFrontEdgeColor, XmRPixel, sizeof(Pixel), 
      XtOffset(Scat3DWidget,scat3D.frontEdgeColor),
      XmRString,"Black"}, 
    {XmNbackEdgeColor, XmCBackEdgeColor, XmRPixel, sizeof(Pixel), 
      XtOffset(Scat3DWidget, scat3D.backEdgeColor), 
      XmRString,"Black"},
    {XmNbackEdgeDashes, XmCBackEdgeDashes, XmRInt, sizeof(int), 
      XtOffset(Scat3DWidget, scat3D.backEdgeDashes), 
      XmRString,"2"},
    {XmNbackEdgeDashOffset, XmCBackEdgeDashOffset, XmRInt, sizeof(int), 
      XtOffset(Scat3DWidget, scat3D.backEdgeDashOffset), 
      XmRString,"1"},
    {XmNresizeCallback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (Scat3DWidget, scat3D.resize), XtRCallback, NULL},
    {XmNbtn2Callback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (Scat3DWidget, scat3D.btn2), XtRCallback, NULL},
    {XmNbtn3Callback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (Scat3DWidget, scat3D.btn3), XtRCallback, NULL},
    {XmNredisplayCallback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (Scat3DWidget, scat3D.redisplay), XtRCallback, NULL},
};


Scat3DClassRec  scat3DClassRec = {
     /* CoreClassPart */
  {
    (WidgetClass) &xmPrimitiveClassRec,  /* superclass            */
    "Scat3D",                       /* class_name            */
    sizeof(scat3DRec),              /* widget_size           */
    NULL,                           /* class_initialize      */
    NULL,                           /* class_part_initialize */
    FALSE,                          /* class_inited          */
    initialize,       /* initialize            */
    NULL,                           /* initialize_hook       */
    XtInheritRealize,               /* realize               */
    actionsList,                    /* actions               */
    XtNumber(actionsList),          /* num_actions           */
    resources,                      /* resources             */
    XtNumber(resources),            /* num_resources         */
    NULLQUARK,                      /* xrm_class             */
    TRUE,                           /* compress_motion       */
    TRUE,                           /* compress_exposure     */
    TRUE,                           /* compress_enterleave   */
    TRUE,                           /* visible_interest      */
    destroy,                        /* destroy               */
    resize,                         /* resize                */
    redisplay,                      /* expose                */
    setValues,                      /* set_values            */
    NULL,                           /* set_values_hook       */
    XtInheritSetValuesAlmost,       /* set_values_almost     */
    NULL,                           /* get_values_hook       */
    NULL,                           /* accept_focus          */
    XtVersion,                      /* version               */
    NULL,                           /* callback private      */
    defaultTranslations,            /* tm_table              */
    NULL,                           /* query_geometry        */
    NULL,                           /* display_accelerator   */
    NULL,                           /* extension             */
  },
  /* Motif primitive class fields */
  {
     (XtWidgetProc) _XtInherit,  	/* Primitive border_highlight   */
     (XtWidgetProc) _XtInherit,   	/* Primitive border_unhighlight */
     XtInheritTranslations,		/* translations                 */
     NULL, /* motionAP,		*/		/* arm_and_activate             */
     NULL,				/* get resources      		*/
     0,					/* num get_resources  		*/
     NULL,         			/* extension                    */
  },
  /* Scat3D class part */
  {
    0,                              	/* ignored	                */
  }
};

WidgetClass scat3DWidgetClass = (WidgetClass) &scat3DClassRec;

static void btn2AP(Scat3DWidget w, XEvent *event, char *args, int n_args)
{
    XmDrawingAreaCallbackStruct  cbStruct;
    
#ifdef MOTIF10
    	_XmProcessTraversal((Widget)w, XmTRAVERSE_CURRENT);
#else
    	XmProcessTraversal((Widget)w, XmTRAVERSE_CURRENT);
#endif   

    /* Just call the callback */
    cbStruct.reason = XmCR_INPUT;
    cbStruct.event = event;
    XtCallCallbacks ((Widget)w, XmNbtn2Callback, (XtPointer) &cbStruct);
}

static void btn3AP(Scat3DWidget w, XEvent *event, char *args, int n_args)
{
    XmDrawingAreaCallbackStruct  cbStruct;
    
#ifdef MOTIF10
    	_XmProcessTraversal((Widget)w, XmTRAVERSE_CURRENT);
#else
    	XmProcessTraversal((Widget)w, XmTRAVERSE_CURRENT);
#endif   

    /* Just call the callback */
    cbStruct.reason = XmCR_INPUT;
    cbStruct.event = event;
    XtCallCallbacks ((Widget)w, XmNbtn3Callback, (XtPointer)&cbStruct);
}

/*
** Widget initialize method
*/

static void initialize(Widget Request, Widget New, ArgList args, 
  Cardinal*  num_args)
{
    Scat3DWidget request = (Scat3DWidget) Request;
    Scat3DWidget new = (Scat3DWidget) New;
    XGCValues values;
    XmFontContext context;
    XmStringCharSet charset;
    Display *display = XtDisplay(new);
    int i;
    /* Make sure the window size is not zero. The Core 
       initialize() method doesn't do this. */
    if (request->core.width == 0) 
   	new->core.width = 100;
    if (request->core.height == 0)
  	new->core.height = 100;
   	
    /* Make a local copy of the font list, or get the default if not specified */
    if (new->scat3D.font == NULL)
#ifdef MOTIF10
	new->scat3D.font = XmFontListCreate(
	    XLoadQueryFont(XtDisplay(new), "fixed"),
	    XmSTRING_DEFAULT_CHARSET);
#else
    	new->scat3D.font = XmFontListCopy(
    	      _XmGetDefaultFontList((Widget)new, XmLABEL_FONTLIST));
#endif                                      
    else
        new->scat3D.font = XmFontListCopy(new->scat3D.font);

#ifdef MOTIF10
    new->scat3D.fs = new->scat3D.font->font;
#else
    XmFontListInitFontContext(&context, new->scat3D.font);
    XmFontListGetNextFont(context, &charset, &new->scat3D.fs);
    XmFontListFreeFontContext(context);
    XtFree(charset);
#endif
    /* Create the graphics contexts */
    values.background = new->core.background_pixel;
    values.foreground = new->primitive.foreground;
    values.line_width = 0;
    values.line_style = LineSolid;
    values.font = new->scat3D.fs->fid;
    new->scat3D.gc = XCreateGC(display, XDefaultRootWindow(display),
      GCForeground|GCBackground|GCFont|
      GCLineWidth|GCLineStyle, &values);

    values.background = new->core.background_pixel;
    values.foreground = new->scat3D.dataColor;
    new->scat3D.dataGc = XCreateGC(display, XDefaultRootWindow(display),
      GCForeground|GCBackground|GCFont|
      GCLineWidth|GCLineStyle, &values);

    values.background = new->core.background_pixel;
    values.foreground = new->core.background_pixel;
    new->scat3D.eraseGc = XCreateGC(display, XDefaultRootWindow(display),
      GCForeground|GCBackground|GCFont|
      GCLineWidth|GCLineStyle, &values);

    values.background = new->core.background_pixel;
    values.foreground = new->scat3D.frontEdgeColor;
    new->scat3D.frontEdgeGc = XCreateGC(display, XDefaultRootWindow(display),
      GCForeground|GCBackground|GCFont|
      GCLineWidth|GCLineStyle /*|GCDashList */, &values);

    values.background = new->core.background_pixel;
    values.foreground = new->scat3D.labeledEdgeColor;
    new->scat3D.labeledEdgeGc = XCreateGC(display, XDefaultRootWindow(display),
      GCForeground|GCBackground|GCFont|
      GCLineWidth|GCLineStyle /*|GCDashList */, &values);

    values.background = new->core.background_pixel;
    values.foreground = new->scat3D.unLabeledEdgeColor;
    new->scat3D.unLabeledEdgeGc = XCreateGC(display, XDefaultRootWindow(display),
      GCForeground|GCBackground|GCFont|
      GCLineWidth|GCLineStyle /*|GCDashList */, &values);

    values.background = new->core.background_pixel;
    values.foreground = new->scat3D.axesColor;
    new->scat3D.axesGc = XCreateGC(display, XDefaultRootWindow(display),
      GCForeground|GCBackground|GCFont|
      GCLineWidth|GCLineStyle /*|GCDashList */, &values);

    values.background = new->core.background_pixel;
    values.foreground = new->scat3D.backEdgeColor;
/* setup dashed line for back edges drawing, if requested. Do it
   only once at startup and don't bother any more 			*/  
    if (new->scat3D.backEdgeDashes > 0)
    {   
	if (new->scat3D.backEdgeDashes > 255)
	    new->scat3D.backEdgeDashes = 255;
        values.line_style = LineOnOffDash;
        values.dash_offset = new->scat3D.backEdgeDashOffset;
        values.dashes = (unsigned char)new->scat3D.backEdgeDashes;
	new->scat3D.backEdgeGc = XCreateGC(display, XDefaultRootWindow(display),
	  GCForeground|GCBackground|GCFont|
	  GCLineWidth|GCLineStyle |GCDashList |GCDashOffset, &values);
    }
    else 
        new->scat3D.backEdgeGc = XCreateGC(display, XDefaultRootWindow(display),
	  GCForeground|GCBackground|GCFont|
	  GCLineWidth|GCLineStyle, &values);        

    new->scat3D.rightMargin = MARGIN;
    new->scat3D.leftMargin = MARGIN;
    new->scat3D.topMargin = MARGIN;
    new->scat3D.bottomMargin = MARGIN;
    new->scat3D.scatData = NULL;
    new->scat3D.nPoints = 0;
    new->scat3D.curScaledData = NULL;
    new->scat3D.nCurPoints = 0;
    new->scat3D.newScaledData = NULL;
    new->scat3D.nNewPoints = 0;
    new->scat3D.drawBuffer = 0;
    updateBufferAllocation(new);
    new->scat3D.plotImage = NULL;
    updateImageAllocation(new);
    new->scat3D.curStateTemplate = FALSE;
    new->scat3D.newStateTemplate = FALSE;
    new->scat3D.dataMapChanged = TRUE;
    new->scat3D.curDarkerPoints = new->scat3D.newDarkerPoints = new->scat3D.darkerPoints;
    new->scat3D.dragging = FALSE;
    new->scat3D.cd = createCubeDescr (makeCubeCenter(new), makeCubeSize(new),
      M_PI/4., -M_PI/3., 0.0, _X, _Y, _Z);
/* don't initialize axes now - there is no  data for this	   */      
    for (i=0; i< 3; i++)
        new->scat3D.axis[i] = NULL; 
/* axis initialization is provided by <scat3DSetLimits> function        */
    resize(New);
}



/*
** Widget setValues method
*/
static Boolean setValues(Widget Current,  Widget Request, 
  Widget New, ArgList args, Cardinal*  num_args)
{
    Scat3DWidget current = (Scat3DWidget) Current;
    Scat3DWidget request = (Scat3DWidget) Request;
    Scat3DWidget new = (Scat3DWidget) New;
    Boolean redraw;
#define DONT_CHANGE(A, M)			\
    if (new->A!= current->A)			\
    {						\
        new->A= current->A;			\
        XtWarning(M);				\
    }
    redraw=FALSE;
    DONT_CHANGE (scat3D.font,"Font can not be changed after initialization");
    DONT_CHANGE (core.background_pixel, 
      "Color can not be changed after initialization");
    DONT_CHANGE (primitive.foreground,
      "Color can not be changed after initialization");
    DONT_CHANGE (scat3D.dataColor,
      "Color can not be changed after initialization");
    DONT_CHANGE (scat3D.frontEdgeColor,
      "Color can not be changed after initialization");
    DONT_CHANGE (scat3D.backEdgeColor,
      "Color can not be changed after initialization");
    DONT_CHANGE (scat3D.backEdgeDashes,
      "Dash parameters can not be changed after initialization");
    DONT_CHANGE (scat3D.backEdgeDashOffset,
      "Dash parameters can not be changed after initialization");
    DONT_CHANGE (scat3D.labeledEdgeColor,
      "Color can not be changed after initialization");
    DONT_CHANGE (scat3D.unLabeledEdgeColor,
      "Color can not be changed after initialization");
    DONT_CHANGE (scat3D.axesColor,
      "Color can not be changed after initialization");
    /* if double buffering changes, allocate or deallocate offscreen pixmap */
    if (new->scat3D.doubleBuffer != current->scat3D.doubleBuffer) 
    	updateBufferAllocation(new);
    	
    if (new->scat3D.newUseBitmap != current->scat3D.newUseBitmap)
    {
        if (new->scat3D.bitmapStrategyAuto)
            new->scat3D.newUseBitmap = current->scat3D.newUseBitmap;
        else    
    	    updateImageAllocation(new); 
    }
        	    	
    if (new->scat3D.axis[_X] != NULL)
    {
        float min, max;
        if (new->scat3D.xLogScaling != current->scat3D.xLogScaling)
        {
             getAxisLimits(new->scat3D.axis[_X], &min, &max);
             if (new->scat3D.xLogScaling && min > 0.)
        	  new->scat3D.xLogScaling = setAxisLogScale(new->scat3D.axis[_X]);
             else
             {
        	  setAxisLinearScale(new->scat3D.axis[_X]); 
        	  new->scat3D.xLogScaling = FALSE;
             }	   
             redraw = TRUE;
        }                 
    }
    if (new->scat3D.axis[_Y] != NULL)
    {
        float min, max;
        if (new->scat3D.yLogScaling != current->scat3D.yLogScaling)
        {
             getAxisLimits(new->scat3D.axis[_Y], &min, &max);
             if (new->scat3D.yLogScaling && min > 0.)
        	  new->scat3D.yLogScaling = setAxisLogScale(new->scat3D.axis[_Y]);
             else
             {
        	  setAxisLinearScale(new->scat3D.axis[_Y]);
        	  new->scat3D.yLogScaling =  FALSE;
             }	   
             redraw = TRUE;
        }                 
    }
    if (new->scat3D.axis[_Z] != NULL)
    {
        float min, max;
        if (new->scat3D.zLogScaling != current->scat3D.zLogScaling)
        {
             getAxisLimits(new->scat3D.axis[_Z], &min, &max);
             if (new->scat3D.zLogScaling && min > 0.)
        	  new->scat3D.zLogScaling = setAxisLogScale(new->scat3D.axis[_Z]);
             else
             {
        	  setAxisLinearScale(new->scat3D.axis[_Z]); 
        	  new->scat3D.zLogScaling =  FALSE;
             }	   
             redraw = TRUE;
        }                 
    }
    if (new->scat3D.darkerPoints  != current->scat3D.darkerPoints)
        redraw = TRUE;
    return(redraw);   	
}
    
/*
** Widget destroy method
*/
static void destroy(Widget wg)
{
    int i;
    Scat3DWidget w =(Scat3DWidget) wg;
    XFreeGC(XtDisplay(w), w->scat3D.gc);
    XFreeGC(XtDisplay(w), w->scat3D.eraseGc);
    XFreeGC(XtDisplay(w), w->scat3D.dataGc);
    XFreeGC(XtDisplay(w), w->scat3D.frontEdgeGc);
    XFreeGC(XtDisplay(w), w->scat3D.labeledEdgeGc);
    XFreeGC(XtDisplay(w), w->scat3D.unLabeledEdgeGc);
    XFreeGC(XtDisplay(w), w->scat3D.axesGc);
    if (w->scat3D.font != NULL) XmFontListFree(w->scat3D.font);
    XtRemoveAllCallbacks (wg, XmNresizeCallback);
    XtRemoveAllCallbacks (wg, XmNbtn2Callback);
    XtRemoveAllCallbacks (wg, XmNbtn3Callback);
    XtRemoveAllCallbacks (wg, XmNredisplayCallback );
    if (w->scat3D.drawBuffer)
    {
        XFreePixmap(XtDisplay(w), w->scat3D.drawBuffer);
        w->scat3D.drawBuffer = 0;
    }        
    /* Free inner structures */
    destroyCubeDescr(w->scat3D.cd);
    for (i=0; i<3; i++)
         destroyAxis(w->scat3D.axis[i]);
    XtFree((char*)w->scat3D.scatData);
    XtFree((char*)w->scat3D.curScaledData); 
    XtFree((char*)w->scat3D.newScaledData);    
}

/*
** Widget resize method
** It is used to generate a new picture when some picture control 
** parameters are changed because the same calculation should be performed
** in this case
*/
static void resize(Widget wg)
{
    Scat3DWidget w = (Scat3DWidget) wg;
    int i;
    int borderWidth =
    	w->primitive.shadow_thickness + w->primitive.highlight_thickness;
    XRectangle clipRect;
    /* set drawing gc to clip drawing before motif shadow and highlight */
    /* resize the drawing buffer, an offscreen pixmap for smoother animation */
    if (w->scat3D.bitmapStrategyAuto)
        w->scat3D.newUseBitmap = 
          USE_BITMAP(w->core.width, w->core.height, w->scat3D.nPoints);
    updateImageAllocation(w); 
    clipRect.x = borderWidth;
    clipRect.y = borderWidth;
    clipRect.width = w->core.width - 2 * borderWidth;
    clipRect.height = w->core.height - 2 * borderWidth;
    XSetClipRectangles(XtDisplay(w), w->scat3D.gc, 0, 0, &clipRect, 1, Unsorted);
    XSetClipRectangles(XtDisplay(w), w->scat3D.eraseGc, 0, 0, &clipRect, 1, Unsorted);
    XSetClipRectangles(XtDisplay(w), w->scat3D.dataGc, 0, 0, &clipRect, 1, Unsorted);
    XSetClipRectangles(XtDisplay(w), w->scat3D.frontEdgeGc, 0, 0, &clipRect, 1, Unsorted);
    XSetClipRectangles(XtDisplay(w), w->scat3D.backEdgeGc, 0, 0, &clipRect, 1, Unsorted);
    XSetClipRectangles(XtDisplay(w), w->scat3D.labeledEdgeGc, 0, 0, &clipRect, 1, Unsorted);
    XSetClipRectangles(XtDisplay(w), w->scat3D.unLabeledEdgeGc, 0, 0, &clipRect, 1, Unsorted);
    XSetClipRectangles(XtDisplay(w), w->scat3D.axesGc, 0, 0, &clipRect, 1, Unsorted);
    updateBufferAllocation(w);
    setCubeSize (w->scat3D.cd, makeCubeSize(w));
    setCubeLocation(w->scat3D.cd, makeCubeCenter(w));
    setAxisNameClipRectangle(w->scat3D.axis[_X], &clipRect);
    setAxisNameClipRectangle(w->scat3D.axis[_Y], &clipRect);
    setAxisNameClipRectangle(w->scat3D.axis[_Z], &clipRect);
    /* call the resize callback */
    if (XtIsRealized((Widget)w))
    	XtCallCallbacks(wg, XmNresizeCallback, NULL);
}
    
/* redisplay and drawing routines					*/

static void redisplay(Widget wg, XEvent *event, Region region )
{
    Scat3DWidget w = (Scat3DWidget) wg;
    drawContents(w);
}

static void makeNewAxesState(Scat3DWidget w)
{
    int i, j;
    Boolean col;
    Boolean changed = FALSE;
    for (i=0; i<3; i++)
        if (w->scat3D.axis[i] != NULL) 
            if (w->scat3D.axis[i]->changed)
                changed = TRUE;
    if (changed)
	for (i=0; i<3; i++)
            if (w->scat3D.axis[i] != NULL) 
            {
		setAxisDefaultNameClip(w->scat3D.axis[i]);
                makeNewAxisImage(w->scat3D.axis[i]);
            }

    if (changed)
	do
	{
            col = FALSE;
            for (i=0; i<2; i++)
        	for (j=i+1; j<3; j++) 
                    col = col || resolveAxesNameCollision(w->scat3D.fs,
                      w->scat3D.axis[i],w->scat3D.axis[j] );
	}
	while (col);                          
}  

static Boolean resolveAxesNameCollision(XFontStruct *fs, axis *a1, axis *a2)
{
    vector ort1, ort2, ort;
    axis a;
    int hSpacing;
    int x11, x12, x21, x22;
    
    if (a1 == NULL || a2 == NULL) return (FALSE);
    if (!a1->mapped)   return (FALSE);
    if (!a2->mapped)   return (FALSE);  
    if (fs == NULL)   return (FALSE); 
    if (a1->newSegment == NULL)   return (FALSE);  
    if (a2->newSegment == NULL)   return (FALSE);  
    if (a1->newSegment->name == NULL)   return (FALSE);  
    if (a2->newSegment->name == NULL)   return (FALSE);  
    hSpacing = XTextWidth(fs, " ", 1); 
    if (abs(a1->newSegment->name->wY - a2->newSegment->name->wY) > 
        fs->ascent + fs->descent) return (FALSE);
    ort1 = mulVector(ortogonal(makeVector(a1->axisImage.x2 - a1->axisImage.x1,
      a1->axisImage.y2 - a1->axisImage.y1)), -1);
    ort2 = mulVector(ortogonal(makeVector(a2->axisImage.x2 - a2->axisImage.x1,
      a2->axisImage.y2 - a2->axisImage.y1)), -1);
    x11 = a1->newSegment->name->wX - hSpacing/2;
    x12 = a1->newSegment->name->wX + a1->newSegment->name->XWidth + hSpacing/2;      
    x21 = a2->newSegment->name->wX - hSpacing/2;
    x22 = a2->newSegment->name->wX + a2->newSegment->name->XWidth + hSpacing/2;      
    
    if (x11 >= x22 || x21 >= x12) return (FALSE);

    if ((ort1.x <= 0 && ort2.x <0) || (ort1.x < 0 && ort2.x <=0) ||
      (ort1.x >= 0 && ort2.x >0) || (ort1.x > 0 && ort2.x >=0)) 
    {
        int ox = ort1.x != 0 ? ort1.x : ort2.x;
        ox = ox > 0 ? 1 : -1;
        if (ox > 0)
        {
            if (x11 != x21)
            {
                if (x11 < x21)
                    a1->clipX1 = a2->newSegment->name->wX - hSpacing;
                else  
                    a2->clipX1 = a1->newSegment->name->wX - hSpacing;
            }
            else if (x12 != x22)
            {
                if (x12 < x22)
                    a1->clipX1 = a2->newSegment->name->wX - hSpacing;
                else  
                    a2->clipX1 = a1->newSegment->name->wX - hSpacing;
            }    
            else  if (abs(ort1.y * ort2.x) > abs(ort2.y * ort1.x))
               a2->clipX1 = a1->newSegment->name->wX - hSpacing;
            else  if (abs(ort1.y * ort2.x) < abs(ort2.y * ort1.x))
               a1->clipX1 = a2->newSegment->name->wX - hSpacing;
  	    else  if (ox * a1->axisImage.x1 >= ox * a1->axisImage.x2)
  	    	a2->clipX1 = a1->newSegment->name->wX - hSpacing;
  	    else
  	    	a1->clipX1 = a2->newSegment->name->wX - hSpacing;
	}
	else
	{
            if (x12 != x22)
            {
                if (x12 > x22)
                    a1->clipX0 = a2->newSegment->name->wX + a2->newSegment->name->XWidth + hSpacing;
                else  
                    a2->clipX0 = a1->newSegment->name->wX + a1->newSegment->name->XWidth + hSpacing;
            }
            else if (x11 != x21)
            {
                if (x11 > x21)
                    a1->clipX0 = a2->newSegment->name->wX + a2->newSegment->name->XWidth + hSpacing;
                else  
                    a2->clipX0 = a1->newSegment->name->wX + a1->newSegment->name->XWidth + hSpacing;
            }    
            else  if (abs(ort1.y * ort2.x) > abs(ort2.y * ort1.x))
                a2->clipX0 = a1->newSegment->name->wX + a1->newSegment->name->XWidth + hSpacing;
            else  if (abs(ort1.y * ort2.x) < abs(ort2.y * ort1.x))
                a1->clipX0 = a2->newSegment->name->wX + a2->newSegment->name->XWidth + hSpacing;
  	    else  if (ox * a1->axisImage.x1 >= ox * a1->axisImage.x2)
                a2->clipX0 = a1->newSegment->name->wX + a1->newSegment->name->XWidth + hSpacing;
  	    else
                a1->clipX0 = a2->newSegment->name->wX + a2->newSegment->name->XWidth + hSpacing;
	}
    }
    else if ( a1->axisImage.x1 < a2->axisImage.x1 || a1->axisImage.x2 < a2->axisImage.x2)
    {
         int _x1 = x21;
         int _x2 = x12;
         int _x = (_x1 + _x2) / 2;
         a1->clipX1 = _x - hSpacing/2 + 1;
         a2->clipX0 = _x + hSpacing/2 + 1;
    }
    else if ( a1->axisImage.x1 > a2->axisImage.x1 || a1->axisImage.x2 > a2->axisImage.x2)         
    {
         int _x1 = x11;
         int _x2 = x22;
         int _x = (_x1 + _x2) / 2;
         a2->clipX1 = _x - hSpacing/2 + 1;
         a1->clipX0 = _x + hSpacing/2 + 1;
    }
    else
    {
/* why might it happen ???			*/
         a1->clipX1 = a1->clipX0 + 1;
    }
    makeNewAxisImage(a1);
    makeNewAxisImage(a2);
    return (TRUE);
}                 
	 
/*  
    If the state was changed
    calculate and set all the information related to the new state
    and draw new contents (draw all objects anyway because they might 
    be corrupted by erase data) .
    Overwise just draw (redisplay) .
*/     
static void drawContents(Scat3DWidget w)
{
    int i;
    Drawable drawBuf = w->scat3D.doubleBuffer ? 
      w->scat3D.drawBuffer : XtWindow(w);
    Boolean somethingChanged = FALSE;

/* nothing to do if no cube frame					*/
    if (w->scat3D.cd == NULL) return;

/* make new objects  (data calculation part)				*/
    if (w->scat3D.dragging)
        w->scat3D.newDarkerPoints = FALSE;
    else
        w->scat3D.newDarkerPoints = w->scat3D.darkerPoints;            

    if (w->scat3D.curDarkerPoints != w->scat3D.newDarkerPoints)
        w->scat3D.dataMapChanged = TRUE;

    if (w->scat3D.cd->changed)
    {
        makeNewCubeImage(w->scat3D.cd); 
        mapAxes(w,w->scat3D.cd->newCube);
        w->scat3D.dataMapChanged = TRUE;
    }                   
    for (i=0; i<3; i++)
        if (w->scat3D.axis[i] != NULL) 
            if (w->scat3D.axis[i]->changed)
                w->scat3D.dataMapChanged = TRUE;
    makeNewAxesState( w);
    if (!w->scat3D.newStateTemplate)
    {
/* otherwise data will not be drawn anyway				*/       
        if (w->scat3D.dataMapChanged)
	{
            if (w->scat3D.cd->changed)
        	makeNewDataImage(w,w->scat3D.cd->newCube);
            else
        	makeNewDataImage(w,w->scat3D.cd->curCube);     
	}        
    } 
    
/* erase part								*/                 
    if (w->core.visible && XtIsRealized((Widget)w))
    {
	if ( !w->scat3D.curStateTemplate )
	    somethingChanged = somethingChanged || w->scat3D.newStateTemplate || 
	        w->scat3D.dataMapChanged;   
        for (i=0; i<3; i++)
            if (w->scat3D.axis[i] != NULL) 
        	somethingChanged = somethingChanged || w->scat3D.axis[i]->changed; 
	somethingChanged = somethingChanged || w->scat3D.cd->changed;

	if (somethingChanged)
	{
	    if (!w->scat3D.newUseBitmap || w->scat3D.newStateTemplate)
	    {
/* othewise no need to erase at all					*/
	        if (w->scat3D.curUseBitmap)
	        {
/* no possiblity to erase/restore data separately,  so total erase	*/
	            XFillRectangle(XtDisplay(w), drawBuf, w->scat3D.eraseGc, 
	            0, 0,w->core.width,w->core.height);
		}
		else
		{
/* data							*/    
		    if ( !w->scat3D.curStateTemplate && 
		      (w->scat3D.newStateTemplate || w->scat3D.dataMapChanged))
      		        eraseData(w); 
/* erase  axes								*/	   
        	    for (i=0; i<3; i++)
        		if (w->scat3D.axis[i] != NULL) 
        		{
        		    if (w->scat3D.axis[i]->changed) 
			        drawCurAxis(XtDisplay(w), drawBuf, 
			          w->scat3D.eraseGc, w->scat3D.axis[i]);
			}		         
		    if (w->scat3D.cd->changed)
		       eraseCubeFrame(w);
		}
	    }
	}	

/* drawing part								*/	
        if (!w->scat3D.newUseBitmap)
	{
/*   draw new / restore old objects					*/	   
	    if (w->scat3D.cd->changed)
	       drawNewCubeFrame(w);
	    else
	       drawCurCubeFrame(w);
/*   draw new / restore possibly corrupted axes				*/	   
            for (i=0; i<3; i++)
        	if (w->scat3D.axis[i] != NULL) 
        	{
        	    if (w->scat3D.axis[i]->changed) 
                       drawNewAxis(XtDisplay(w), drawBuf, w->scat3D.axesGc,
                	 w->scat3D.axis[i]);
		    else
		       drawCurAxis(XtDisplay(w), drawBuf, w->scat3D.axesGc, 
			 w->scat3D.axis[i]); 
		}		         
/* draw new / restore possibly corrupted objects			*/	     
	    if (!w->scat3D.newStateTemplate)
		if( w->scat3D.dataMapChanged)
      		   drawNewData(w);
      		else
      		   drawCurData(w);
	}
	else
	{
/* draw data bitmap first						*/
	    if (!w->scat3D.newStateTemplate)
		if( w->scat3D.dataMapChanged)
      		   drawNewData(w);
      		else
      		   drawCurData(w);
/*   draw new / restore old objects				*/	   
	    if (w->scat3D.cd->changed)
	       drawNewCubeFrame(w);
	    else
	       drawCurCubeFrame(w);
/*   draw new / restore possibly corrupted axes			*/	   
            for (i=0; i<3; i++)
        	if (w->scat3D.axis[i] != NULL) 
        	{
        	    if (w->scat3D.axis[i]->changed) 
                       drawNewAxis(XtDisplay(w), drawBuf, w->scat3D.axesGc,
                	 w->scat3D.axis[i]);
		    else
		       drawCurAxis(XtDisplay(w), drawBuf, w->scat3D.axesGc, 
			 w->scat3D.axis[i]); 
		}
	}		    		         
	finishDrawing(w);
    }	              

/* save new state of the objects					*/
/* IMPORTANT: all objects clear their <changed> flag performing
   <save...> calls 							*/
    if (w->scat3D.cd->changed)
        saveCubeImage(w->scat3D.cd);
    
    for (i=0; i<3; i++)
        if (w->scat3D.axis[i] != NULL) 
            if (w->scat3D.axis[i]->changed) 
            	saveAxisImage(w->scat3D.axis[i]);
             
    if (w->scat3D.dataMapChanged && !w->scat3D.newStateTemplate)
        saveDataImage(w); 
        
    w->scat3D.curStateTemplate = w->scat3D.newStateTemplate;
    w->scat3D.curUseBitmap = w->scat3D.newUseBitmap;
    w->scat3D.curDarkerPoints = w->scat3D.newDarkerPoints;
}

/* auxilary drawing functions,  are calling only from the fuction above */

static void drawNewData(Scat3DWidget w)    
{
    drawData(w, w->scat3D.newScaledData,w->scat3D.nNewPoints,w->scat3D.dataGc,
      w->scat3D.newUseBitmap, w->scat3D.newDarkerPoints);
}

static void drawCurData(Scat3DWidget w)    
{
    drawData(w, w->scat3D.curScaledData,w->scat3D.nCurPoints,w->scat3D.dataGc,
      w->scat3D.curUseBitmap, w->scat3D.curDarkerPoints);
}


static void eraseData(Scat3DWidget w)    
{   
    if (! w->scat3D.curUseBitmap)
        drawData(w, w->scat3D.curScaledData,
          w->scat3D.nCurPoints,w->scat3D.eraseGc, w->scat3D.curUseBitmap, 
          w->scat3D.curDarkerPoints);
}

static void drawData(Scat3DWidget w, XPoint *data, int nPoints, GC gc,
    Boolean bitmapDrawing, Boolean darkerPoints)
{
    Drawable drawBuf = w->scat3D.doubleBuffer ? 
      w->scat3D.drawBuffer : XtWindow(w);

    if (w->core.visible && XtIsRealized((Widget)w))
    if (bitmapDrawing)
    {
       if (w->scat3D.plotImage == NULL) return;
	XPutImage(XtDisplay(w), drawBuf, gc,
	  w->scat3D.plotImage, 
	  0,0,0,0 /* w->scat3D.plotImageStart, w->scat3D.plotImageEnd */,
	  w->scat3D.plotImage->width,w->scat3D.plotImage->height);
    }	  
    else
    {
        if (data == NULL || nPoints == 0) return;
        XDrawPoints(XtDisplay(w), drawBuf, gc, data, nPoints, CoordModeOrigin);
        if (darkerPoints)
        {
            int i;
            for (i = 0; i < nPoints; i++)
                data[i].x +=1;
	    XDrawPoints(XtDisplay(w), drawBuf, gc, data, nPoints, 
	      CoordModeOrigin); 
	    for (i = 0; i < nPoints; i++)
                data[i].x -=1;	                     
	}
    }	                
}
            
static void  drawNewCubeFrame (Scat3DWidget w)
{
    Drawable drawBuf = w->scat3D.doubleBuffer ? 
      w->scat3D.drawBuffer : XtWindow(w);

    if (w->core.visible && XtIsRealized((Widget)w))
    {
	drawNewCubeBackEdges (XtDisplay(w), drawBuf,  w->scat3D.backEdgeGc,  
  	  w->scat3D.cd);
	drawNewCubeUnLabeledEdges (XtDisplay(w), drawBuf, w->scat3D.unLabeledEdgeGc,
  	  w->scat3D.cd);
 	drawNewCubeLabeledEdges (XtDisplay(w), drawBuf, w->scat3D.labeledEdgeGc, 
  	  w->scat3D.cd);
 	drawNewCubeFrontEdges (XtDisplay(w), drawBuf, w->scat3D.frontEdgeGc, 
  	  w->scat3D.cd);
    }
}    	  

static void  drawCurCubeFrame (Scat3DWidget w)
{
    Drawable drawBuf = w->scat3D.doubleBuffer ? 
      w->scat3D.drawBuffer : XtWindow(w);
    if (w->core.visible && XtIsRealized((Widget)w))
    {
	drawCurCubeBackEdges (XtDisplay(w), drawBuf,  w->scat3D.backEdgeGc,  
  	  w->scat3D.cd);
	drawCurCubeUnLabeledEdges (XtDisplay(w), drawBuf, w->scat3D.unLabeledEdgeGc,
  	  w->scat3D.cd);
 	drawCurCubeLabeledEdges (XtDisplay(w), drawBuf, w->scat3D.labeledEdgeGc, 
  	  w->scat3D.cd);
 	drawCurCubeFrontEdges (XtDisplay(w), drawBuf, w->scat3D.frontEdgeGc, 
  	  w->scat3D.cd);
    }
}    	  

static void eraseCubeFrame (Scat3DWidget w)
{
    Drawable drawBuf = w->scat3D.doubleBuffer ? 
      w->scat3D.drawBuffer : XtWindow(w);

    if (w->core.visible && XtIsRealized((Widget)w))
    {
	drawCurCubeFrontEdges (XtDisplay(w), drawBuf, 
	  w->scat3D.eraseGc, w->scat3D.cd);
	drawCurCubeLabeledEdges (XtDisplay(w), drawBuf, 
	  w->scat3D.eraseGc, w->scat3D.cd);
	drawCurCubeUnLabeledEdges (XtDisplay(w), drawBuf, 
	  w->scat3D.eraseGc, w->scat3D.cd);
	drawCurCubeBackEdges (XtDisplay(w), drawBuf,  
	  w->scat3D.eraseGc, w->scat3D.cd);
    }
}  

/* event handling functions						*/

/* mark (start of BtnMotion) */    
static void markMod(Scat3DWidget w, XEvent *event, char* args, int n_args)
{
    markGeneric(w, event, args, n_args);
    w->scat3D.newStateTemplate = FALSE;
    drawContents(w);
}

static void mark(Scat3DWidget w, XEvent *event, char* args, int n_args)
{
    markGeneric(w, event, args, n_args);
    w->scat3D.newStateTemplate = TRUE;
    drawContents(w);
}

static void markGeneric(Scat3DWidget w, XEvent *event, char* args, int n_args)
{
    Boolean grabbed = FALSE;
    int i;
    for (i=0; i<3; i++)
        if (grabbed = markAxisState(w->scat3D.axis[i], event))
            break;
    if (!grabbed)
        grabbed = markCubeState(w->scat3D.cd,  event);
    w->scat3D.dragging = grabbed;    
}

static void dragMotionGeneric(Scat3DWidget w, XEvent *event, char* args, 
  int n_args)
{
/* Assume that only one instance of the only object was marked. So 
   processing sequence can differ from the one in <mark> function.
   Changes in cube apparently affects all axes				*/
    Boolean grabbed = FALSE;
    int i;
    for (i=0; i<3; i++)
        if (grabbed = dragAxis(w->scat3D.axis[i], event))
            break;
    if (!grabbed)
        dragCube(w->scat3D.cd,  event);
}	    

static void dragMotionMod(Scat3DWidget w, XEvent *event, char* args, int n_args)
{
    dragMotionGeneric( w,  event,  args, n_args);
    w->scat3D.newStateTemplate = FALSE;
    drawContents(w);
}

static void dragMotion(Scat3DWidget w, XEvent *event, char* args, int n_args)
{
    dragMotionGeneric( w,  event,  args, n_args);
    w->scat3D.newStateTemplate = TRUE;
    drawContents(w);
}


static void stopDragging(Scat3DWidget w, XEvent *event, char* args, int n_args)
{
/* Process last possible cursor position changes, if any */
    int i; 
    dragMotion(w, event, args, n_args);
    if (!stopCubeDragging(w->scat3D.cd,  event))
        for (i=0; i<3; i++)
            if (stopAxisDragging(w->scat3D.axis[i], event))
                break;
    w->scat3D.newStateTemplate = FALSE;
    w->scat3D.dragging = FALSE;
    drawContents(w);
}     


/* 
** widget set/get interface
*/

void Scat3DSetContents(Widget wg, Scat3DPoint *points, int nPoints,  
  Scat3DRescaleModes rescale)            
{
    Scat3DWidget w = (Scat3DWidget) wg;
    float axisMin[3];
    float axisMax[3];
    int i;
    if (w == NULL) return;
    if (w->scat3D.scatData != NULL) XtFree((char*) w->scat3D.scatData);
    w->scat3D.nPoints = 0;
    w->scat3D.scatData = NULL;
    w->scat3D.dataMapChanged = TRUE;
    if (points != NULL && nPoints != 0)
    {
	w->scat3D.scatData = copyData(points, nPoints);
	w->scat3D.nPoints = nPoints;
    }	
    if (w->scat3D.bitmapStrategyAuto)
    {
        Boolean useBitmap;
        useBitmap = USE_BITMAP(w->core.width, w->core.height, w->scat3D.nPoints);
	if (useBitmap != w->scat3D.newUseBitmap)
	{
            w->scat3D.newUseBitmap = useBitmap;
            updateImageAllocation(w); 
	}
    }			
    if (rescale != SCAT3D_NO_RESCALE)
    {
	findDataLimits(w->scat3D.scatData, w->scat3D.nPoints, 
	  axisMin + _X, axisMax+ _X, 
	  axisMin + _Y, axisMax+ _Y, 
	  axisMin + _Z, axisMax+ _Z);
        if (rescale == SCAT3D_RESCALE)
        {
            Scat3DSetLimits(wg, axisMin[_X], axisMax[_X],
              axisMin[_Y], axisMax[_Y], axisMin[_Z], axisMax[_Z]);
/* <Scat3DSetLimits> calls <drawContents> through <scat3DSetVisiblePart>,
    everything is done							    */
	    return;              
        }      
	else if (rescale == SCAT3D_RESCALE_AT_MAX)                
	    for (i=0; i<3; i++)
                setAxisLimitVisible(w->scat3D.axis[i], 
                  axisMin[i],axisMax[i]);
	else if (rescale == SCAT3D_GROW_ONLY)                
	    for (i=0; i<3; i++)
                expandAxisLimitVisible(w->scat3D.axis[i], 
                  axisMin[i],axisMax[i]);
    }
    drawContents(w);
}    
        
/* Initialize axis instances (if not done yet) and set object
** space limits
*/
void Scat3DSetLimits (Widget wg, 
			    float xMin, float xMax,
			    float yMin, float yMax,
			    float zMin, float zMax
			    )
{
    Scat3DWidget w = (Scat3DWidget) wg;
    if (w == NULL) return;
/* initialize axis if not done yet				*/    
    if (w->scat3D.axis[_X] == NULL)
    {
        w->scat3D.axis[_X] = createDummyAxis();
        setAxisLabelsFont(w->scat3D.axis[_X], w->scat3D.fs);
    }
    if (w->scat3D.axis[_Y] == NULL)
    {
        w->scat3D.axis[_Y] = createDummyAxis();
        setAxisLabelsFont(w->scat3D.axis[_Y], w->scat3D.fs);
    }
    if (w->scat3D.axis[_Z] == NULL)
    {
        w->scat3D.axis[_Z] = createDummyAxis();
        setAxisLabelsFont(w->scat3D.axis[_Z], w->scat3D.fs);
    }
    setAxisLimitRange(w->scat3D.axis[_X], xMin, xMax);
    setAxisLimitRange(w->scat3D.axis[_Y], yMin, yMax);
    setAxisLimitRange(w->scat3D.axis[_Z], zMin, zMax);
    if (w->scat3D.xLogScaling && xMin > 0.)
         w->scat3D.xLogScaling = setAxisLogScale(w->scat3D.axis[_X]);
    else
    {
         setAxisLinearScale(w->scat3D.axis[_X]); 
         w->scat3D.xLogScaling = FALSE;
    }	   
    if (w->scat3D.yLogScaling && yMin > 0.)
         w->scat3D.yLogScaling = setAxisLogScale(w->scat3D.axis[_Y]);
    else
    {
         setAxisLinearScale(w->scat3D.axis[_Y]); 
         w->scat3D.yLogScaling = FALSE;
    }	   
    if (w->scat3D.zLogScaling && zMin > 0.)
         w->scat3D.zLogScaling = setAxisLogScale(w->scat3D.axis[_Z]);
    else
    {
         setAxisLinearScale(w->scat3D.axis[_Z]); 
         w->scat3D.zLogScaling = FALSE;
    }	   
/* <scat3DSetVisiblePart> will make redraw as well		*/    
    Scat3DSetVisiblePart (wg, xMin, xMax, yMin, yMax, zMin, zMax);
}

/*
** set axis names
*/
void Scat3DSetAxesNames(Widget wg, char *xName, char *yName, char *zName)  
{
    int borderWidth;
    XRectangle clipRect;
    Scat3DWidget w = (Scat3DWidget) wg;
    if (w == NULL) return;

    borderWidth =
    	w->primitive.shadow_thickness + w->primitive.highlight_thickness;
    clipRect.x = borderWidth;
    clipRect.y = borderWidth;
    clipRect.width = w->core.width - 2 * borderWidth;
    clipRect.height = w->core.height - 2 * borderWidth;

/* initialize axis if not done yet				*/    
    if (w->scat3D.axis[_X] == NULL)
    {
        w->scat3D.axis[_X] = createDummyAxis();
        setAxisLabelsFont(w->scat3D.axis[_X], w->scat3D.fs);
        if (w->scat3D.xLogScaling)
             setAxisLogScale(w->scat3D.axis[_X]);
    }   
    if (w->scat3D.axis[_Y] == NULL)
    {
        w->scat3D.axis[_Y] = createDummyAxis();
        setAxisLabelsFont(w->scat3D.axis[_Y], w->scat3D.fs);
        if (w->scat3D.yLogScaling)
             setAxisLogScale(w->scat3D.axis[_Y]);
    }
    if (w->scat3D.axis[_Z] == NULL)
    {
        w->scat3D.axis[_Z] = createDummyAxis();
        setAxisLabelsFont(w->scat3D.axis[_Z], w->scat3D.fs);
        if (w->scat3D.zLogScaling)
             setAxisLogScale(w->scat3D.axis[_Z]);
    }
    setAxisName(w->scat3D.axis[_X], xName);
    setAxisName(w->scat3D.axis[_Y], yName);
    setAxisName(w->scat3D.axis[_Z], zName);
/* <scat3DSetVisiblePart> will make redraw as well		*/    
    setAxisNameClipRectangle(w->scat3D.axis[_X], &clipRect);
    setAxisNameClipRectangle(w->scat3D.axis[_Y], &clipRect);
    setAxisNameClipRectangle(w->scat3D.axis[_Z], &clipRect);
    drawContents(w);
}

void Scat3DSetVisiblePart (Widget wg, 
			    float xMin, float xMax,
			    float yMin, float yMax,
			    float zMin, float zMax
			    )
{
    Scat3DWidget w = (Scat3DWidget) wg;
    if (w == NULL) return;
    setAxisVisibleRange(w->scat3D.axis[_X], xMin, xMax);
    setAxisVisibleRange(w->scat3D.axis[_Y], yMin, yMax);
    setAxisVisibleRange(w->scat3D.axis[_Z], zMin, zMax); 
    drawContents(w);
}

void Scat3DGetVisiblePart (Widget wg, 
			    float *xMin, float *xMax,
			    float *yMin, float *yMax,
			    float *zMin, float *zMax
			    )
{
    Scat3DWidget w = (Scat3DWidget) wg;
    if (w == NULL) return;
    getAxisVisibleRange(w->scat3D.axis[_X], xMin, xMax);
    getAxisVisibleRange(w->scat3D.axis[_Y], yMin, yMax);
    getAxisVisibleRange(w->scat3D.axis[_Z], zMin, zMax); 
}    

void Scat3DResetView(Widget wg)
{
    Scat3DWidget w = (Scat3DWidget) wg;
    int i;
    if (w == NULL) return;
    setCubeViewAngles(w->scat3D.cd, M_PI/4., -M_PI/3., 0.0 );
    for (i=0; i<3; i++)
         resetAxisView(w->scat3D.axis[i]);
    drawContents(w);
}



/*
**  Scat3DZoom(    drawContents(w);
Widget wg, float factor);
**
**  make zoom in (factor < 1) or zoom out (factor > 1)
**
** 	wg	Specifies the widget
**	factor  Factor to "multiplay" or pull the picture from carrent
**		center of visual ranges. After zoom clipping to limit
**		ranges is applied
**
*/
void Scat3DZoom(Widget wg, float factor)
{
    int i;
    Scat3DWidget w = (Scat3DWidget) wg;
    for (i=0; i<3; i++)
        zoomAxisWVerge (w->scat3D.axis[i], factor, LOG_SCALE_VERGE);
    drawContents(w);
}

void Scat3DZoomIn(Widget wg)
{
    Scat3DZoom(wg, 0.5);
}

void Scat3DZoomOut(Widget wg)
{
    Scat3DZoom(wg, 2.0);
}

static void findDataLimits(Scat3DPoint * data, int nPoints, 
  float *xMin, float *xMax, 
  float *yMin, float *yMax,
  float *zMin, float *zMax)
{
    *xMin = *xMax = 0;
    *yMin = *yMax = 0;
    *zMin = *zMax = 0; 
    if(data == NULL && nPoints == 0) return;
    nPoints--;
    *xMin = *xMax = data[nPoints].x;
    *yMin = *yMax = data[nPoints].y;
    *zMin = *zMax = data[nPoints].z; 
    while (--nPoints >=0 )
    {
        if (*xMin > data[nPoints].x) *xMin = data[nPoints].x;
        if (*xMax < data[nPoints].x) *xMax = data[nPoints].x;
        if (*yMin > data[nPoints].y) *yMin = data[nPoints].y;
        if (*yMax < data[nPoints].y) *yMax = data[nPoints].y;
        if (*zMin > data[nPoints].z) *zMin = data[nPoints].z;
        if (*zMax < data[nPoints].z) *zMax = data[nPoints].z;
    }
}

static Scat3DPoint * copyData(Scat3DPoint * data, int nPoints)
{
    Scat3DPoint * d;
    Scat3DPoint *dd;
    if (data == NULL) return(NULL);
    if (nPoints == 0) return (NULL);
    dd = d = (Scat3DPoint *) XtMalloc(sizeof(Scat3DPoint) * nPoints); 
    while(--nPoints >=0)
        *dd++ = *data++;
    return(d);       
}


static void makeNewDataImage(Scat3DWidget w, paraState *cubeFrame)
{
    if (w->scat3D.newScaledData != NULL) XtFree((char*) w->scat3D.newScaledData);
    w->scat3D.newScaledData = NULL;
    w->scat3D.nNewPoints = 0;
    if (cubeFrame == NULL) return;
    if (!w->scat3D.newUseBitmap)
	w->scat3D.newScaledData = 
	  mapData(w->scat3D.scatData, w->scat3D.nPoints, w->scat3D.axis, 
	  cubeFrame, &(w->scat3D.nNewPoints));
    else
    {
        if (w->scat3D.plotImage == NULL)
             updateImageAllocation(w);
/* for whatever reason bstring.h does not exist on ULTRIX machine             
        bzero(w->scat3D.plotImage->data, 
          w->scat3D.plotImage->bytes_per_line * 
          w->scat3D.plotImage->height);
workaround :*/   
        {
            int i;
            for (i=0; i < w->scat3D.plotImage->bytes_per_line * 
              w->scat3D.plotImage->height; i++)
                w->scat3D.plotImage->data[i] = 0;
	}                         
 	bitMapData(w->scat3D.scatData, w->scat3D.nPoints, w->scat3D.axis,
  	  cubeFrame,  w->scat3D.plotImage, w->scat3D.newDarkerPoints);
    }  	  

/* through away data if empty					*/      
    if ((!w->scat3D.newUseBitmap) && w->scat3D.nNewPoints == 0)
    {
         XtFree((char*) w->scat3D.newScaledData);
         w->scat3D.newScaledData = NULL;
    } 
}

static void saveDataImage(Scat3DWidget w)
{
    if (w->scat3D.curScaledData != NULL) XtFree((char*) w->scat3D.curScaledData);
    w->scat3D.curScaledData =  w->scat3D.newScaledData;
    w->scat3D.newScaledData = NULL;
    w->scat3D.nCurPoints = w->scat3D.nNewPoints;
    w->scat3D.nNewPoints = 0;
    w->scat3D.dataMapChanged = FALSE;
}

static XPoint *mapData(Scat3DPoint * data, int nPoints, axis *axes [3],
  paraState *cube,  int *nXPoints)
{
    XPoint *xP;
    int i;
    XPoint *dst;
    Scat3DPoint *src;
    int wX0, wY0;
    int wXdx, wXdy, wXdz;
    int wYdx, wYdy, wYdz;   
    double fxx, fxy, fxz;
    double fyx, fyy, fyz;
/* visible region range					*/    
    float xMin, xMax, yMin, yMax, zMin, zMax;
    double xMinMod, xMaxMod, xLenMod, yMinMod, yMaxMod, yLenMod, zMinMod, 
      zMaxMod, zLenMod;
    Boolean xLog, yLog, zLog;
    
    if (data == NULL || nPoints == 0 || cube == NULL || axes[_X] == NULL ||
      axes[_Y] == NULL || axes[_Z] == NULL) 
    {
        *nXPoints = 0; 
        return(NULL);
    }
    getAxisVisibleRange(axes[_X], &xMin, &xMax);
    getAxisVisibleRange(axes[_Y], &yMin, &yMax);
    getAxisVisibleRange(axes[_Z], &zMin, &zMax);

    xLog = axes[_X]->logScale;
    yLog = axes[_Y]->logScale;
    zLog = axes[_Z]->logScale;

    if (xLog) {xMinMod = log10(xMin); xMaxMod = log10(xMax);}
    else {xMinMod =  xMin; xMaxMod =  xMax;}
    if (yLog) {yMinMod = log10(yMin); yMaxMod = log10(yMax);}
    else {yMinMod =  yMin; yMaxMod =  yMax;}
    if (zLog) {zMinMod = log10(zMin); zMaxMod = log10(zMax);}
    else {zMinMod =  zMin; zMaxMod =  zMax;}
    
    xLenMod = xMaxMod - xMinMod;
    yLenMod = yMaxMod - yMinMod;
    zLenMod = zMaxMod - zMinMod;
    
    wX0 = cube->vertex[0].point.x;
    wY0 = cube->vertex[0].point.y;
    
    wXdx = cube->vertex[
      cube->edge[cube->vertex[0].edge[_X]].vertex[1]
      ].point.x - wX0;
    wXdy = cube->vertex[
      cube->edge[cube->vertex[0].edge[_Y]].vertex[1]
      ].point.x - wX0;
    wXdz = cube->vertex[
      cube->edge[cube->vertex[0].edge[_Z]].vertex[1]
      ].point.x - wX0;
      
    wYdx = cube->vertex[
      cube->edge[cube->vertex[0].edge[_X]].vertex[1]
      ].point.y - wY0;
    wYdy = cube->vertex[
      cube->edge[cube->vertex[0].edge[_Y]].vertex[1]
      ].point.y - wY0;
    wYdz = cube->vertex[
      cube->edge[cube->vertex[0].edge[_Z]].vertex[1]
      ].point.y - wY0;
/* calculate float matrix factors (this is really transformation matrix) */      
    fxx = (double)wXdx / xLenMod;
    fxy = (double)wXdy / yLenMod;
    fxz = (double)wXdz / zLenMod; 
    
    fyx = (double)wYdx / xLenMod;
    fyy = (double)wYdy / yLenMod;
    fyz = (double)wYdz / zLenMod; 
    
#define T_LIN(A, B, C, D) double A = B - C;
#define T_LOG(A, B, C, D) double A = log10(B) - D; 
/* set point as XPoint						*/
#define X_POINT_SET							\
{									\
    dst->x = 								\
      (int) floor((double)(fxx * x + fxy * y + fxz * z + 0.5)) + wX0; 	\
    dst->y = 								\
      (int) floor((double)(fyx * x + fyy * y + fyz * z + 0.5)) + wY0; 	\
    dst++;								\
} 		
/* set point in the bitmap					*/	
#define BIT_POINT_SET							\
{									\
    int b_x;								\
    int b_y;								\
    b_x = 								\
      (int) floor((double)(fxx * x + fxy * y + fxz * z + 0.5)) + wX0; 	\
    b_y = 								\
      (int) floor((double)(fyx * x + fyy * y + fyz * z + 0.5)) + wY0; 	\
/* integer clipping to avoid wrong memory acsess is not usually needed,	\
   because bitmap size usually mach bigger then image size	*/	\
   bits[b_y * bytesPerLine + (b_x >> 3)] |= 0x80 >> (b_x & 0x7);	\
   if (thiken)								\
   {									\
       b_x += 1;								\
       bits[b_y * bytesPerLine + (b_x >> 3)] |= 0x80 >> (b_x & 0x7);	\
   }  									\
}   


#define TRANS_S3D_POINT(TRAN_X, TRAN_Y,  TRAN_Z, POINT_SET)		\
    {									\
        for (i = nPoints-1; i >= 0; i--, src++)				\
            if ( xMin <= src->x && src->x <= xMax &&			\
                 yMin <= src->y && src->y <= yMax &&			\
                 zMin <= src->z && src->z <= zMax )			\
            {								\
               TRAN_X(x, src->x, xMin, xMinMod)				\
               TRAN_Y(y, src->y, yMin, yMinMod)				\
               TRAN_Z(z, src->z, zMin, zMinMod)				\
               POINT_SET						\
	    } 								\
    }	

/* take max amount of memory. It is possible to decrease this amount
    if run clipping pass first, to count visible points		*/            
    xP = (XPoint *) XtMalloc(sizeof(XPoint) * nPoints); 
    dst = xP;	 
    src = data;	 
    if (xLog)
    {
        if (yLog)
        {
            if (zLog)
                TRANS_S3D_POINT(T_LOG, T_LOG, T_LOG, X_POINT_SET)
	    else
	        TRANS_S3D_POINT(T_LOG, T_LOG, T_LIN, X_POINT_SET)
	}
	else
	{
            if (zLog)
                TRANS_S3D_POINT(T_LOG, T_LIN, T_LOG, X_POINT_SET)
	    else
	        TRANS_S3D_POINT(T_LOG, T_LIN, T_LIN, X_POINT_SET)
	}
    }	
    else
    {
        if (yLog)
        {
            if (zLog)
                TRANS_S3D_POINT(T_LIN, T_LOG, T_LOG, X_POINT_SET)
	    else
	        TRANS_S3D_POINT(T_LIN, T_LOG, T_LIN, X_POINT_SET)
	}
	else
	{
            if (zLog)
                TRANS_S3D_POINT(T_LIN, T_LIN, T_LOG, X_POINT_SET)
	    else
	        TRANS_S3D_POINT(T_LIN, T_LIN, T_LIN, X_POINT_SET)
	}
    }
    *nXPoints = dst - xP;	    					 
    return(xP);
}    

static void bitMapData(Scat3DPoint * data, int nPoints, axis *axes [3],
  paraState *cube,  XImage* image, Boolean thiken)
{
    int i;
    char *bits =  image->data;
    int bytesPerLine = image->bytes_per_line;
    Scat3DPoint *src;

    int wX0, wY0;
    int wXdx, wXdy, wXdz;
    int wYdx, wYdy, wYdz;   
    double fxx, fxy, fxz;
    double fyx, fyy, fyz;
/* visible region range					*/    
    float xMin, xMax, yMin, yMax, zMin, zMax;
    double xMinMod, xMaxMod, xLenMod, yMinMod, yMaxMod, yLenMod, zMinMod, 
      zMaxMod, zLenMod;
    Boolean xLog, yLog, zLog;
    
    if (data == NULL || nPoints == 0 || cube == NULL || axes[_X] == NULL ||
      axes[_Y] == NULL || axes[_Z] == NULL) 
	return;
    getAxisVisibleRange(axes[_X], &xMin, &xMax);
    getAxisVisibleRange(axes[_Y], &yMin, &yMax);
    getAxisVisibleRange(axes[_Z], &zMin, &zMax);

    xLog = axes[_X]->logScale;
    yLog = axes[_Y]->logScale;
    zLog = axes[_Z]->logScale;

    if (xLog) {xMinMod = log10(xMin); xMaxMod = log10(xMax);}
    else {xMinMod =  xMin; xMaxMod =  xMax;}
    if (yLog) {yMinMod = log10(yMin); yMaxMod = log10(yMax);}
    else {yMinMod =  yMin; yMaxMod =  yMax;}
    if (zLog) {zMinMod = log10(zMin); zMaxMod = log10(zMax);}
    else {zMinMod =  zMin; zMaxMod =  zMax;}
    
    xLenMod = xMaxMod - xMinMod;
    yLenMod = yMaxMod - yMinMod;
    zLenMod = zMaxMod - zMinMod;
    
    wX0 = cube->vertex[0].point.x;
    wY0 = cube->vertex[0].point.y;
    
    wXdx = cube->vertex[
      cube->edge[cube->vertex[0].edge[_X]].vertex[1]
      ].point.x - wX0;
    wXdy = cube->vertex[
      cube->edge[cube->vertex[0].edge[_Y]].vertex[1]
      ].point.x - wX0;
    wXdz = cube->vertex[
      cube->edge[cube->vertex[0].edge[_Z]].vertex[1]
      ].point.x - wX0;
      
    wYdx = cube->vertex[
      cube->edge[cube->vertex[0].edge[_X]].vertex[1]
      ].point.y - wY0;
    wYdy = cube->vertex[
      cube->edge[cube->vertex[0].edge[_Y]].vertex[1]
      ].point.y - wY0;
    wYdz = cube->vertex[
      cube->edge[cube->vertex[0].edge[_Z]].vertex[1]
      ].point.y - wY0;
/* calculate float matrix factors (this is really transformation matrix) */      
    fxx = (double)wXdx / xLenMod;
    fxy = (double)wXdy / yLenMod;
    fxz = (double)wXdz / zLenMod; 
    
    fyx = (double)wYdx / xLenMod;
    fyy = (double)wYdy / yLenMod;
    fyz = (double)wYdz / zLenMod; 
    src = data;              

    if (xLog)
    {
        if (yLog)
        {
            if (zLog)
                TRANS_S3D_POINT(T_LOG, T_LOG, T_LOG, BIT_POINT_SET)
	    else
	        TRANS_S3D_POINT(T_LOG, T_LOG, T_LIN, BIT_POINT_SET)
	}
	else
	{
            if (zLog)
                TRANS_S3D_POINT(T_LOG, T_LIN, T_LOG, BIT_POINT_SET)
	    else
	        TRANS_S3D_POINT(T_LOG, T_LIN, T_LIN, BIT_POINT_SET)
	}
    }	
    else
    {
        if (yLog)
        {
            if (zLog)
                TRANS_S3D_POINT(T_LIN, T_LOG, T_LOG, BIT_POINT_SET)
	    else
	        TRANS_S3D_POINT(T_LIN, T_LOG, T_LIN, BIT_POINT_SET)
	}
	else
	{
            if (zLog)
                TRANS_S3D_POINT(T_LIN, T_LIN, T_LOG, BIT_POINT_SET)
	    else
	        TRANS_S3D_POINT(T_LIN, T_LIN, T_LIN, BIT_POINT_SET)
	}
    }
}    
    
/*
** static void mapAxes(Scat3DWidget w, paraState *cubeFrame)
** 
** Set widget axes images according to current cube position
** (so both object be in consistence)
**
*/
static void mapAxes(Scat3DWidget w, paraState *cubeFrame)
{
     int i;
     XSegment seg;
     paraEdge *pe;
     if (cubeFrame == NULL) return;
/* unmap all axis				*/
     for (i=0; i<3; i++)
         setAxisUnMapped(w->scat3D.axis[i]);
/* map ones from new labeledEdge list		*/
     for (pe = cubeFrame->labeledEdge; pe != NULL; pe = pe->next)
     {
        if ( cubeFrame->vertex[pe->vertex[0]].nextBnd == 
          &( cubeFrame->vertex[pe->vertex[1]]))
        {
/* if first vertex of the edge is a predessor of the second in 
   countclockwise ordered sequence of cube (2d) boundary verteces	*/
            seg.x1 = cubeFrame->vertex[pe->vertex[0]].point.x;
            seg.y1 = cubeFrame->vertex[pe->vertex[0]].point.y;
            seg.x2 = cubeFrame->vertex[pe->vertex[1]].point.x;
            seg.y2 = cubeFrame->vertex[pe->vertex[1]].point.y;
            setAxisImageSegment(w->scat3D.axis[pe->aIdx], &seg); 
	}
	else
        {
            seg.x1 = cubeFrame->vertex[pe->vertex[1]].point.x;
            seg.y1 = cubeFrame->vertex[pe->vertex[1]].point.y;
            seg.x2 = cubeFrame->vertex[pe->vertex[0]].point.x;
            seg.y2 = cubeFrame->vertex[pe->vertex[0]].point.y;
            setAxisImageSegmentReverse(w->scat3D.axis[pe->aIdx], &seg); 
	}
        setAxisMapped(w->scat3D.axis[pe->aIdx]);
    }
/* set labels clipping on axes						*/
    pe = cubeFrame->labeledEdge; 
    if (pe != NULL)
    {
        setAxisClipping(w->scat3D.axis[pe->aIdx], CLIP_NONE,CLIP_HORIZONTAL);
        pe = pe->next;
    }
    if (pe != NULL)
    {
        clipType clip2_3;
        
        if (pe->next == NULL)
            setAxisClipping(w->scat3D.axis[pe->aIdx], CLIP_HORIZONTAL, CLIP_NONE); 
	else 
	{
	    if  (( cubeFrame->vertex[pe->next->vertex[0]].nextBnd == 
                &( cubeFrame->vertex[pe->next->vertex[1]])) ==
                 (cubeFrame->vertex[pe->next->vertex[0]].point.y < 
                 cubeFrame->vertex[pe->next->vertex[1]].point.y))
/* if first point of the third labeled segment has smaller Y then the second point of the same
   segment											*/	
      	    {
      	        if (cubeFrame->vertex[pe->vertex[0]].point.x ==  
                 cubeFrame->vertex[pe->vertex[1]].point.x)
                     clip2_3 = CLIP_HORIZONTAL;
                else if (cubeFrame->vertex[pe->next->vertex[0]].point.y ==
                 cubeFrame->vertex[pe->next->vertex[1]].point.y)
                     clip2_3 = CLIP_VERTICAL;
                else if ((double) abs(cubeFrame->vertex[pe->vertex[0]].point.y - 
                  cubeFrame->vertex[pe->vertex[1]].point.y) /
                  (double) abs(cubeFrame->vertex[pe->vertex[0]].point.x - 
                  cubeFrame->vertex[pe->vertex[1]].point.x)  >= 
                  (double) abs(cubeFrame->vertex[pe->next->vertex[0]].point.x -
                  cubeFrame->vertex[pe->next->vertex[1]].point.x) /
                  (double) abs(cubeFrame->vertex[pe->next->vertex[0]].point.y -
                  cubeFrame->vertex[pe->next->vertex[1]].point.y))
                    clip2_3 = CLIP_HORIZONTAL;
                else
                    clip2_3 = CLIP_VERTICAL;    
	    }
	    else
	        clip2_3 = CLIP_VERTICAL;
      	    {
      	        setAxisClipping(w->scat3D.axis[pe->aIdx], CLIP_HORIZONTAL, clip2_3);
      	        setAxisClipping(w->scat3D.axis[pe->next->aIdx], clip2_3, CLIP_NONE);
	    }
	}	    
    }      	        
	                       
}                       




static void drawShadow(Scat3DWidget w)
{
	/* Draw the Motif required shadows and highlights */
	if (w->primitive.shadow_thickness > 0) 
	{
	    _XmDrawShadow (XtDisplay(w), XtWindow(w), 
			   w->primitive.bottom_shadow_GC,
			   w->primitive.top_shadow_GC,
                	   w->primitive.shadow_thickness,
                	   w->primitive.highlight_thickness,
                	   w->primitive.highlight_thickness,
                	   w->core.width - 2 * w->primitive.highlight_thickness,
                	   w->core.height-2 * w->primitive.highlight_thickness);
	}
	if (w->primitive.highlighted)
	    _XmHighlightBorder((Widget)w);
	else if (_XmDifferentBackground((Widget)w, XtParent((Widget)w)))
	    _XmUnhighlightBorder((Widget)w);
}

static void finishDrawing(Scat3DWidget w)
{
    if (w->core.visible && XtIsRealized((Widget)w))
    {  
        if (w->scat3D.doubleBuffer)
    	    XCopyArea(XtDisplay(w), w->scat3D.drawBuffer, 
    	    XtWindow(w), w->scat3D.gc, 0, 0,
            w->core.width, w->core.height, 0, 0);
	/* Draw the Motif required shadows and highlights */
 	drawShadow(w);
    } 
}    	    
			    
/*
** For smoother animation, it is possible to allocate an offscreen pixmap
** and draw to that rather than directly into the window.  Unfortunately,
** it is too slow on many machines, so we have to give the user the choice
** whether to use it or not.  This routine reallocates the offscreen
** pixmap buffer when the window is resized, and when the widget is created
*/
static void updateBufferAllocation(Scat3DWidget w)
{ 
    if (w->scat3D.drawBuffer)
    	XFreePixmap(XtDisplay(w), w->scat3D.drawBuffer);
    if (w->scat3D.doubleBuffer) 
    {
    	w->scat3D.drawBuffer = XCreatePixmap(XtDisplay(w),
	  DefaultRootWindow(XtDisplay(w)), w->core.width, w->core.height,
    	  DefaultDepthOfScreen(XtScreen(w)));
     	XFillRectangle(XtDisplay(w), w->scat3D.drawBuffer, w->scat3D.eraseGc, 
	  0, 0,w->core.width,w->core.height);
    	 	   
    } 
    else 
    {
    	w->scat3D.drawBuffer = 0;
    }
}			    

static void updateImageAllocation(Scat3DWidget w)
{ 
    if (w->scat3D.plotImage != NULL)
/* XDestroyImage suppose to destroy data pointed to by image structure */
        XDestroyImage(w->scat3D.plotImage);
    if (w->scat3D.newUseBitmap) 
    {
        w->scat3D.plotImage = XCreateImage(XtDisplay(w), 
           DefaultVisual(XtDisplay(w), DefaultScreen(XtDisplay(w))),  
           1, 
           XYBitmap, 
           0, 
           (char *)XtMalloc(((w->core.width + 7) >> 3) * w->core.height),
           w->core.width,
           w->core.height,
           8,
           ((w->core.width + 7) >> 3) );
         w->scat3D.plotImage->byte_order = LSBFirst;
         w->scat3D.plotImage->bitmap_bit_order = MSBFirst;
         w->scat3D.plotImage->bitmap_unit = 8;
         w->scat3D.plotImage->bitmap_pad = 8;
     }
     else
     {   
          w->scat3D.plotImage = NULL;
    }
}			    



			    
static int makeCubeSize(Scat3DWidget w)
{
    int width, height;
    int cs;
    width  = w->core.width - 
      w->scat3D.leftMargin - w->scat3D.rightMargin -
    	- 2 * ( w->primitive.shadow_thickness + 
    	w->primitive.highlight_thickness);
    height = w->core.height - 
      w->scat3D.bottomMargin - w->scat3D.topMargin - 
    	- 2 * (w->primitive.shadow_thickness + 
    	w->primitive.highlight_thickness);
/* reserve some space for top and down labels and axes names (approximate
   estimation)					*/		
/* to place tics				*/
    height -= 2 * LONG_TIC_LEN;	
    if (w->scat3D.fs != NULL)
    {
/* to place ticmarks				*/    
        height -= 2 * (w->scat3D.fs->ascent + w->scat3D.fs->descent + 1); 
/* to place axes names, the vert spacing  mighth  be up to 2 * ascent, not ascent + descent */
        if (w->scat3D.fs->ascent >= w->scat3D.fs->descent)
            height -= 2 * (2 * w->scat3D.fs->ascent + 1); 
	else
	    height -= 2 * (w->scat3D.fs->ascent + w->scat3D.fs->descent + 1); 
    }	      
/* the cube suppose to be placed in the middle (top and buttom labeling use the same
   amount of space				*/	                      	
    if (width < height )
	cs =  (int)((width-MARGIN)/sqrt(3.));
    else
	cs =  (int)((height-MARGIN)/sqrt(3.));
    if (cs < 0)
       return (0);
    else
       return (cs);   	
	
}    

static vector makeCubeCenter(Scat3DWidget w)
{
    int width, height;
    vector center;
/* the cube suppose to be placed in the middle (top and buttom labeling use the same
   amount of space				*/	                      	
    width  = w->core.width - 
      w->scat3D.leftMargin - w->scat3D.rightMargin;
    height = w->core.height - 
      w->scat3D.bottomMargin - w->scat3D.topMargin;
    center.x = w->scat3D.leftMargin + width/2;
    center.y = w->scat3D.topMargin + height/2; 
    return(center); 
}     

static void makePSData(XPoint *data, int nPoints, FILE *psfile, XImage *image,
    Boolean useBitmap, Boolean darkPoints)
{
    int i, j, k;
    /* if ((data == NULL || nPoints) == NULL && image == NULL) jmk 10/19/94 */
    if ((data == NULL || nPoints == 0) && image == NULL)
        return;

    fprintf(psfile,"save  						\n");

    fprintf(psfile,"/draw_point  	%% x, y =>			\n");
    fprintf(psfile,"{	  						\n");
    fprintf(psfile,"    newpath						\n");
    fprintf(psfile,"    0.5 0 360 arc					\n");
    fprintf(psfile,"    closepath					\n");
    fprintf(psfile,"    fill						\n");
    fprintf(psfile,"} def	  					\n");

    if (useBitmap)
        makePSDataFromImage( psfile,  image);
    else
    {            
	for (i = 0, j=0, k = nPoints; i < nPoints; i++, j++)
	{
	    if (j == 100)
	    {
	        j -= 100;
	        k -= 100;
	        if (darkPoints)
	            fprintf(psfile,"200 {draw_point} repeat		\n");
	        else
	            fprintf(psfile,"100 {draw_point} repeat		\n");
	    } 
            fprintf(psfile,"%d %d					\n",
              data[i].x, data[i].y);
            if (darkPoints)
                fprintf(psfile,"%d %d					\n",
                  data[i].x, data[i].y + 1);
        }   
	if (darkPoints)
            k *= 2;
        if (k != 0)      
	    fprintf(psfile,"%d {draw_point} repeat			\n",
              k);
    }          
    fprintf(psfile,"restore  						\n");
}

static void makePSDataFromImage(FILE *psfile, XImage *image)
{
    int i, j, k;
    char *ptr, *ptrL;
    if (image == NULL) return;
    if (image->data == NULL) return;
    ptrL = image->data;
    k = 0;
    for (i = 0; i < image->height; i++)
    {
        ptr = ptrL;
        for (j = 0; j < image->width; )
        {
            if (((*ptr) & (0x80 >> (j & 0x7))) != 0)
            {
                fprintf(psfile,"%d %d				\n",
                  j, i);
                k++;
            }
            j++;
            if ((j & 0x7) == 0)
                ptr++;
            if (k == 200)
            {
                k -= 200;
	        fprintf(psfile,"200 {draw_point} repeat		\n");
	    }    
                    
	}
	ptrL += image->bytes_per_line;
    }
    if (k != 0)
        fprintf(psfile,"%d {draw_point} repeat				\n",
          k);
}          
		                   

void Scat3DPrintContents(Widget wg, char *psFileName)
{
    Scat3DWidget w = (Scat3DWidget) wg;
    FILE *psfile;
    time_t tt;
    
    psfile = fopen(psFileName, "w");
    if (psfile == NULL)
        return;   
/* PostScript prefix							*/        
    fprintf(psfile, "%%!PS-Adobe-3.0 EPSF-3.0\n");
    fprintf(psfile, "%%%%BoundingBox: %d %d %d %d \n", PAGE_MARGIN, PAGE_MARGIN,
    	    w->core.width + PAGE_MARGIN, w->core.height + PAGE_MARGIN);
    fprintf(psfile, "%%%%Title: %s \n", psFileName);
    time(&tt);
    fprintf(psfile, "%%%%CreationDate: %s \n", ctime(&tt));
    
    fprintf(psfile,"%% Set up a small margin around the	page since most	\n");
    fprintf(psfile,"%% PostScript printers can't print to the edge.	\n");
    fprintf(psfile,"%d %d translate\n", PAGE_MARGIN, PAGE_MARGIN	   );

    Scat3DWritePS(wg, psfile);
    
    fprintf(psfile,"showpage\n");
    fclose (psfile);
}

void Scat3DWritePS(Widget wg, FILE *psfile)
{
    Scat3DWidget w = (Scat3DWidget) wg;
    int i;
    Boolean darkerPoints;    
    
    fprintf(psfile,"1 setlinecap	        			\n");
    fprintf(psfile,"1 setlinejoin					\n");
    fprintf(psfile,"0 setlinewidth					\n");
    fprintf(psfile,"/inch {72 mul} def					\n");
    fprintf(psfile,"%% make mirror transformation for Y to have the same\n");
    fprintf(psfile,"%% orintation of the coodinate system as in the 	\n");
    fprintf(psfile,"%% widget window (X11 style). With the origin in	\n");
    fprintf(psfile,"%% left upper corner and Y axis directed from top to\n");
    fprintf(psfile,"%% bottom.  					\n");
    fprintf(psfile,"0 %d  %% heigth of the widget window\n", w->core.height); 
    fprintf(psfile,"translate 						\n");
    fprintf(psfile,"1 -1 scale 						\n");

/* change mode to a single point plotting for printing			*/
    if (w->scat3D.curDarkerPoints)
    {
        darkerPoints = w->scat3D.darkerPoints;
        w->scat3D.darkerPoints = FALSE;
        drawContents(w);
    }
    else
        darkerPoints = FALSE;        

    for (i=0; i<3; i++)
        if (w->scat3D.axis[i] != NULL) 
               makePSAxis(w->scat3D.axis[i], psfile);
    fprintf(psfile,"gsave						\n");               

    fprintf(psfile,"[%d] %d setdash					\n",
      w->scat3D.backEdgeDashes, w->scat3D.backEdgeDashOffset);               
    makePSCubeBackEdges (w->scat3D.cd, psfile);
    fprintf(psfile,"grestore						\n");               

    makePSCubeUnLabeledEdges (w->scat3D.cd, psfile);
    makePSCubeFrontEdges (w->scat3D.cd, psfile);

    makePSData(w->scat3D.curScaledData, w->scat3D.nCurPoints, psfile, 
        w->scat3D.plotImage, w->scat3D.curUseBitmap, w->scat3D.curDarkerPoints);

/* change mode to initial						*/

    if (darkerPoints)
    {
        w->scat3D.darkerPoints = TRUE;
        drawContents(w);
    }
}
/*
** Utility routine to get and restore the Viewing Rotation.  Note that we work
** using the Euler representation, to make easier with the math. 
*/
void Scat3DGetViewEulerAngles (Widget wg, double *alpha,
                                double *beta, double *gamma )
{
    Scat3DWidget w = (Scat3DWidget) wg;
    matrixToEulerAngles(w->scat3D.cd->rotMatr, alpha, beta, gamma);    
}
                                
void Scat3DSetViewEulerAngles (Widget wg, double alpha, 
				double beta, double gamma)

{  
    Scat3DWidget w = (Scat3DWidget) wg;
    eulerAnglesToMatrix(alpha, beta, gamma, w->scat3D.cd->rotMatr);
    CopyM(w->scat3D.cd->rotMatr, w->scat3D.cd->markedRotMatr);
    w->scat3D.cd->changed = TRUE;
    w->scat3D.newStateTemplate = FALSE;
    w->scat3D.dragging = FALSE;
    drawContents(w);

}
static void eulerAnglesToMatrix(double alpha, double beta, double gamma,
			   double m[3][3])
{
    double cosAlpha, sinAlpha, cosBeta, sinBeta, cosGamma, sinGamma;
    
    sinAlpha = sin(RADIANS(alpha));
    sinBeta = sin(RADIANS(beta));
    sinGamma = sin(RADIANS(gamma));
    cosAlpha = cos(RADIANS(alpha));
    cosBeta = cos(RADIANS(beta));
    cosGamma = cos(RADIANS(gamma));
    
    m[0][0] = cosGamma*cosBeta*cosAlpha - sinGamma*sinAlpha;
    m[0][1] = 0. - sinGamma*cosBeta*cosAlpha - cosGamma*sinAlpha;
    m[0][2] = sinBeta*cosAlpha;
    m[1][0] = cosGamma*cosBeta*sinAlpha + sinGamma*cosAlpha;
    m[1][1] = 0. - sinGamma*cosBeta*sinAlpha + cosGamma*cosAlpha;
    m[1][2] = sinBeta*sinAlpha;
    m[2][0] = 0. - cosGamma*sinBeta;
    m[2][1] = sinGamma*sinBeta;
    m[2][2] = cosBeta;
}

/*
** Calculate current Euler rotation of a 3x3 matrix as three angles,
** alpha, beta, and gamma.  This routine solves the matrix
** equation 4.80 in "The Mathematical Methods of Physics" for
** alpha beta and gamma.
*/
static void matrixToEulerAngles(double m[3][3], double *alpha,
			   double *beta, double *gamma)
{
    double cosAlpha, sinAlpha, cosBeta, sinBeta, cosGamma, sinGamma;
    double alphaResult, betaResult, gammaResult, betaResults[2];
    double temp, testMatrix[3][3];
    int i, j, k, verified;
    
    /* try solutions for both beta = (+/-)acos(m[2][2]) */
    cosBeta = m[2][2];
    betaResults[0] = acos(cosBeta);
    betaResults[1] = 0. - betaResults[0];
    for (k=0; k<=1; k++) {
    	betaResult = betaResults[k];
    	sinBeta = sin(betaResult);
    	if (fabs(sinBeta) < EPSILON) {
    	    /* beta angle close to or at zero */
    	    *beta = 0.;
    	    *gamma = 0.;
    	    /* compute alpha */
    	    sinAlpha = m[1][0];
    	    cosAlpha = m[0][0];
    	    temp = acos(cosAlpha);
    	    if (fabs(sin(temp) - sinAlpha) < EPSILON)
    	    	*alpha = DEGREES(temp);
    	    else if (fabs(sinAlpha + sin(temp)) < EPSILON)
    	    	*alpha = DEGREES(0.-temp);
    	    else
    	    	printf("Internal Error, input not an Euler matrix");
    	    if (*alpha < 0) *alpha += 360;
    	    return;
    	}
    	/* compute gamma */
    	sinGamma = m[2][1] / sinBeta;
    	cosGamma = 0.-m[2][0] / sinBeta;
    	temp = acos(cosGamma);
    	if (fabs(sin(temp) - sinGamma) < EPSILON)
    	    gammaResult = temp;
    	else if (fabs(sin(temp) + sinGamma) < EPSILON)
    	    gammaResult = 0.-temp;
    	else
    	    gammaResult = EPSILON;

    	/* compute alpha */
    	sinAlpha = m[1][2] / sinBeta;
    	cosAlpha = m[0][2] / sinBeta;
    	temp = acos(cosAlpha);
    	if (fabs(sin(temp) - sinAlpha) < EPSILON)
    	    alphaResult = temp;
    	else if (fabs(sin(temp) + sinAlpha) < EPSILON)
    	    alphaResult = 0.-temp;
    	else
    	    alphaResult = EPSILON;

    	/* Verify and select solution */
    	eulerAnglesToMatrix(DEGREES(alphaResult), DEGREES(betaResult), 
    	    	       DEGREES(gammaResult), testMatrix);
    	verified = True;
    	for (i=0; i<=1; i++) {
    	    for (j=0; j<=1; j++) {
    	    	if (fabs(testMatrix[i][j] - m[i][j]) > EPSILON)
    	    	    verified = False;
    	    }
    	}
    	if (verified) {
    	    if (alphaResult < 0) alphaResult += 2*PI;
    	    if (betaResult < 0) betaResult += 2*PI;
    	    if (gammaResult < 0) gammaResult += 2*PI;
    	    *alpha = DEGREES(alphaResult);
    	    *beta = DEGREES(betaResult);
    	    *gamma = DEGREES(gammaResult);
    	    return;
    	}
    }
    printf("Internal error, not an Euler matrix\n");
}

/*******************************************************************************
*									       *
* 2DHist.c -- 2D histogramm viewer widget - main operations		       *
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
*  July 1994 : Upgrade to Encapsulated Postscript, Paul Lebrun		       *
*	File modified : 2DHistP.h, psgen.c 				       *
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
#endif
#include <stdlib.h>
#include <math.h>
#include <float.h>

#ifndef VMS
#include <string.h>
#endif /* VMS */

#include <stdio.h>
#include "2DGeom.h"
#include "2DHistDefs.h"
#include "2DHist.h"
#include "uniLab.h"
#include "labels.h"		
#include "2DHistP.h"
#include "imaggen.h"
#include "2DHistZLabels.h"
#include "fltRange.h"
#include "uniLab_2DHInt.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
 

/* XtResourceDefaultProc(s) for resources initialisation 		*/
static void setTopPlaneColor(Hist2DWidget w, int offset, XrmValue *value); 
static void setLeftPlaneColor(Hist2DWidget w, int offset, XrmValue *value); 
static void setRightPlaneColor(Hist2DWidget w, int offset, XrmValue *value); 
static void setLeftPlanePixmap(Hist2DWidget w, int offset, XrmValue *value); 
static void setRightPlanePixmap(Hist2DWidget w, int offset, XrmValue *value); 
static void setClippingColor(Hist2DWidget w, int offset, XrmValue *value); 
/* Hist2DWidget methods  						*/ 
static void initialize(Widget Request, Widget New, ArgList args, 
  Cardinal*  num_args);
static Boolean setValues(Widget Current, Widget Request, 
  Widget New, ArgList args, Cardinal*  num_args);
static void resize(Widget wg);
static void destroy(Widget wg);
/* Methods for inner objects						*/
static void makeDiscreteMap(Hist2DWidget w);
static void destroyDiscrMap(discreteMap *dMap);
/* inner service routines						*/
static char* myStrdup(char* src);
static objectsToDraw *createObjectsToDraw (void);
static objectsToDraw *makePicture (Hist2DWidget w);
static void destroyObjectsToDraw (objectsToDraw *oD);
static XPoint *makeTemplate(discreteMap *dMap, int *Size);
static XSegment *makeZAxis(discreteMap *dMap);
static void makeMarks (Hist2DWidget w, GC gc);
static void drawTemplate(Display *d,Drawable w, objectsToDraw *oD,GC gc,
  GC backPlaneGc);
static void redrawTemplate(Display *d, Drawable w, objectsToDraw *old, 
  objectsToDraw *new, motionType mT,GC oldInvGc, GC newGc,
  GC oldInvBpGc, GC newBpGc, Boolean zRedraw);
static void updateBufferAllocation(Hist2DWidget w);
static void drawTics(Display *d,Drawable w, objectsToDraw *oD,GC gc);
static void drawArrows(Display *d,Drawable w, objectsToDraw *oD,GC gc);
static void drawNames(Display *d, Drawable w, objectsToDraw *oD, GC gc);
static void drawShadow(Hist2DWidget w);
static void destroyHistStructures(Hist2DWidget w);

/* action routines							*/
static Bool noCtrlWaiting(Display *d, XEvent *e, char *arg);
static Bool pushBtnWaiting(Display *d, XEvent *e, char *arg);
static void mark(Hist2DWidget w, XEvent *event, char* args, int n_args);
static void markMod(Hist2DWidget w, XEvent *event, char* args, int n_args);
static void dragMotion(Hist2DWidget w, XEvent *event, char* args, int n_args);
static void dragMotionMod(Hist2DWidget w, XEvent *event, char* args, int n_args);
static void dragMotionGen(Hist2DWidget w, XEvent *event, Boolean tempDragOn,
  Boolean zRedraw);
static void stopDragging(Hist2DWidget w, XEvent *event, char* args, int n_args);
/*
static void stopAP(Hist2DWidget w, XEvent *event, char *args, int n_args);
*/
static void btn2AP(Hist2DWidget w, XEvent *event, char *args, int n_args);
static void btn3AP(Hist2DWidget w, XEvent *event, char *args, int n_args);

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
      XtOffset(Hist2DWidget, hist2D.doubleBuffer), XmRString, "False"},
    {XmNfontList, XmCFontList, XmRFontList, sizeof(XmFontList),
      XtOffset(Hist2DWidget, hist2D.font), XmRImmediate, NULL},
    {XmNzLogScaling, XmCZLogScaling,XmRBoolean, sizeof(Boolean), 
      XtOffset(Hist2DWidget, hist2D.zLogScaling ), XmRString, "False"}, 
    {XmNbinEdgeLabeling, XmCBinEdgeLabeling, XmRBoolean, sizeof(Boolean), 
      XtOffset(Hist2DWidget, hist2D.binEdgeLabeling ), XmRString, "False"},
    {XmNshadingOn, XmCShadingOn, XmRBoolean, sizeof(Boolean),
      XtOffset(Hist2DWidget, hist2D.shadingOn ), XmRString, "False"}, 
    {XmNerrorBarsOn, XmCErrorBarsOn, XmRBoolean, sizeof(Boolean),
      XtOffset(Hist2DWidget, hist2D.errorBarsOn ), XmRString, "True"}, 
    {XmNtopPlaneColor, XmCTopPlaneColor,XmRPixel, sizeof(Pixel), 
      XtOffset(Hist2DWidget,hist2D.topPlaneColor),
      XmRCallProc,(char*) setTopPlaneColor}, 
    {XmNleftPlaneColor, XmCLeftPlaneColor,XmRPixel, sizeof(Pixel), 
      XtOffset(Hist2DWidget, hist2D.leftPlaneColor), 
      XmRCallProc,(char*) setLeftPlaneColor}, 
    {XmNrightPlaneColor, XmCRightPlaneColor,XmRPixel, sizeof(Pixel),
      XtOffset(Hist2DWidget, hist2D.rightPlaneColor ), 
      XmRCallProc,(char*) setRightPlaneColor}, 
    {XmNleftPlanePixmap, XmCLeftPlanePixmap,XmRPixmap, sizeof(Pixmap), 
      XtOffset(Hist2DWidget, hist2D.leftPlanePixmap), 
      XmRCallProc,(char*) setLeftPlanePixmap}, 
    {XmNrightPlanePixmap, XmCRightPlanePixmap,XmRPixel, sizeof(Pixmap),
      XtOffset(Hist2DWidget, hist2D.rightPlanePixmap ), 
      XmRCallProc,(char*) setRightPlanePixmap}, 
    {XmNclippingColor, XmCClippingColor, XmRPixel, sizeof(Pixel), 
      XtOffset(Hist2DWidget, hist2D.clippingColor ), 
      XmRCallProc,(char*) setClippingColor}, 
    {XmNbackPlanesOn, XmCBackPlanesOn, XmRBoolean, sizeof(Boolean), 
      XtOffset(Hist2DWidget, hist2D.backPlanesOn ),XmRString,"True"}, 
    {XmNresizeCallback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (Hist2DWidget, hist2D.resize), XtRCallback, NULL},
    {XmNbtn2Callback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (Hist2DWidget, hist2D.btn2), XtRCallback, NULL},
    {XmNbtn3Callback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (Hist2DWidget, hist2D.btn3), XtRCallback, NULL},
    {XmNredisplayCallback, XmCCallback, XmRCallback, sizeof(caddr_t),
      XtOffset (Hist2DWidget, hist2D.redisplay), XtRCallback, NULL},
};


Hist2DClassRec  hist2DClassRec = {
     /* CoreClassPart */
  {
    (WidgetClass) &xmPrimitiveClassRec,  /* superclass            */
    "2DHist",                       /* class_name            */
    sizeof(hist2DRec),              /* widget_size           */
    NULL,                           /* class_initialize      */
    NULL,                           /* class_part_initialize */
    FALSE,                          /* class_inited          */
    initialize,                     /* initialize            */
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
     (XtWidgetProc) _XtInherit,         /* Primitive border_highlight   */
     (XtWidgetProc) _XtInherit,  	/* Primitive border_unhighlight */
     XtInheritTranslations,		/* translations                 */
     NULL, /* motionAP,		*/		/* arm_and_activate     */
     NULL,				/* get resources      		*/
     0,					/* num get_resources  		*/
     NULL,         			/* extension                    */
  },
  /* 2DHist class part */
  {
    0,                              	/* ignored	                */
  }
};

WidgetClass hist2DWidgetClass = (WidgetClass) &hist2DClassRec;

static void btn2AP(Hist2DWidget w, XEvent *event, char *args, int n_args)
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

static void btn3AP(Hist2DWidget w, XEvent *event, char *args, int n_args)
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
    XtCallCallbacks ((Widget)w, XmNbtn3Callback, (XtPointer) &cbStruct);
}

static void setTopPlaneColor(Hist2DWidget w, int offset, XrmValue *value)
{
    value->size=sizeof(Pixel); 
    value->addr=(caddr_t)(&(w->primitive.foreground));
}
static void setLeftPlaneColor(Hist2DWidget w, int offset, XrmValue *value) 
{
    value->size=sizeof(Pixel); 
    value->addr=(caddr_t)(&(w->primitive.top_shadow_color));
}
static void setRightPlaneColor(Hist2DWidget w, int offset, XrmValue *value) 
{
    value->size=sizeof(Pixel); 
    value->addr=(caddr_t)(&(w->primitive.bottom_shadow_color));
}
static void setLeftPlanePixmap(Hist2DWidget w, int offset, XrmValue *value) 
{
    value->size=sizeof(Pixmap); 
    value->addr=(caddr_t)(&(w->primitive.top_shadow_pixmap));
}
static void setRightPlanePixmap(Hist2DWidget w, int offset, XrmValue *value) 
{
    value->size=sizeof(Pixmap); 
    value->addr=(caddr_t)(&(w->primitive.bottom_shadow_pixmap));
}
static void setClippingColor(Hist2DWidget w, int offset, XrmValue *value) 
{
    value->size=sizeof(Pixel); 
    value->addr=(caddr_t)(&(w->primitive.foreground));
}
 
Boolean adjustViewAngles(double *fi, double *psi)
{
    float oldFi=(*fi);
    float oldPsi=(*psi);
    *fi-=floor(*fi/(2.*M_PI))*2.*M_PI;
    *psi-=floor(*psi/(2.*M_PI))*2.*M_PI;
    if (*psi>= M_PI) *psi=0;
    if (*psi> M_PI/2.) *psi=M_PI/2.;
    return((*fi==oldFi)&&(*psi==oldPsi));
}



/*
** Widget initialize method
*/
static void initialize(Widget Request, Widget New, ArgList args, 
  Cardinal*  num_args)
{
    Hist2DWidget request = (Hist2DWidget) Request;
    Hist2DWidget new = (Hist2DWidget) New;
    XGCValues values;
    XmFontContext context;
    XmStringCharSet charset;
    Display *display = XtDisplay(new);
    /* Make sure the window size is not zero. The Core 
       initialize() method doesn't do this. */
    if (request->core.width == 0) 
   	new->core.width = 100;
    if (request->core.height == 0)
  	new->core.height = 100;
   	
    /* Make a local copy of the font list, or get the default if not specified */
    if (new->hist2D.font == NULL)
#ifdef MOTIF10
	new->hist2D.font = XmFontListCreate(
	    XLoadQueryFont(XtDisplay(new), "fixed"),
	    XmSTRING_DEFAULT_CHARSET);
#else
    	new->hist2D.font = XmFontListCopy(
    	      _XmGetDefaultFontList(New, XmLABEL_FONTLIST));
#endif                                      
    else
        new->hist2D.font = XmFontListCopy(new->hist2D.font);

#ifdef MOTIF10
    new->hist2D.fs = new->hist2D.font->font;
#else
    XmFontListInitFontContext(&context, new->hist2D.font);
    XmFontListGetNextFont(context, &charset, &new->hist2D.fs);
    XmFontListFreeFontContext(context);
    XtFree(charset);
#endif
    /* Create the graphics contexts */
    values.background = new->core.background_pixel;
    values.foreground = new->primitive.foreground;
    values.line_width = 0;
    values.line_style = LineSolid;
    values.font = new->hist2D.fs->fid;
    new->hist2D.gc = XCreateGC(display, XDefaultRootWindow(display),
      GCForeground|GCBackground|GCFont|
      GCLineWidth|GCLineStyle, &values);

/*    values.dashes = 1;
    values.line_style = LineOnOffDash; */
    new->hist2D.backPlaneGc = XCreateGC(display, XDefaultRootWindow(display),
      GCForeground|GCBackground|GCFont|
      GCLineWidth|GCLineStyle /*|GCDashList */, &values);

    values.line_style = LineSolid;
    values.background = new->primitive.foreground;
    values.foreground = new->core.background_pixel;
    new->hist2D.inverseGc = XCreateGC(display, XDefaultRootWindow(display),
      GCForeground|GCBackground|GCFont|
      GCLineWidth|GCLineStyle, &values);
 
/*    values.line_style = LineOnOffDash; */
    new->hist2D.inverseBackPlaneGc = XCreateGC(display, XDefaultRootWindow(display),
      GCForeground|GCBackground|GCFont|
      GCLineWidth|GCLineStyle /*|GCDashList */, &values);

    values.line_style = LineSolid;
    values.foreground = new->hist2D.topPlaneColor;
    values.background = new->core.background_pixel;
    new->hist2D.topGc = XCreateGC(display, XDefaultRootWindow(display),
      GCForeground|GCBackground, 
       &values);
    
/* add leftPlanePixmap and rightPlanePixmap handling here          */
/* (The problem is that stiplling does not support during PutImage */

    values.background = new->core.background_pixel;
    values.foreground = new->hist2D.leftPlaneColor;
    new->hist2D.leftGc = XCreateGC(display, XDefaultRootWindow(display), 
      GCForeground|GCBackground,
      &values);

    values.background = new->core.background_pixel;
    values.foreground = new->hist2D.rightPlaneColor;
    new->hist2D.rightGc = XCreateGC(display, XDefaultRootWindow(display),
      GCForeground|GCBackground,
      &values);

    values.background = new->core.background_pixel;
    values.foreground = new->hist2D.clippingColor;
    new->hist2D.clippingGc = XCreateGC(display, XDefaultRootWindow(display),
      GCForeground|GCBackground,
      &values);

    new->hist2D.drawBuffer = 0;
    new->hist2D.discrMap=NULL;
    new->hist2D.xLabels=NULL;
    new->hist2D.yLabels=NULL;
    new->hist2D.currentPicture=NULL;      
    new->hist2D.templateDragging=FALSE;
    new->hist2D.leftImage=NULL;
    new->hist2D.rightImage=NULL;
    new->hist2D.topImage=NULL;
    new->hist2D.clippingImage=NULL;
    new->hist2D.xNames=NULL;
    new->hist2D.yNames=NULL;
    new->hist2D.zNames=NULL;
    new->hist2D.bins=NULL;
    new->hist2D.aHist=NULL;
    new->hist2D.topErrors=NULL;
    new->hist2D.bottomErrors=NULL;
    new->hist2D.sourceHistogram=NULL;
    new->hist2D.badPicture=TRUE;

    new->hist2D.fiViewAngle = 0.;
    new->hist2D.psiViewAngle = 0.;
    
    new->hist2D.wireImage=XCreateImage(XtDisplay(new),
      DefaultVisual(XtDisplay(new), DefaultScreen(XtDisplay(new))),
      1,XYBitmap,0,NULL,new->core.width,new->core.height,32,0);
/* All other images needed should be initialised here too 
** ..........
*/         
    initialazeBaseLCS(new);
    resize(New);
}

/*
** Widget setValues method
*/
static Boolean setValues(Widget Current, Widget Request, 
  Widget New, ArgList args, Cardinal*  num_args)
{
    Hist2DWidget current = (Hist2DWidget) Current; 
    Hist2DWidget request = (Hist2DWidget) Request;
    Hist2DWidget new = (Hist2DWidget) New;
    Boolean redraw;
#define DONT_CHANGE(A, M)			\
    if (new->A!= current->A)			\
    {						\
        new->A= current->A;			\
        XtWarning(M);				\
    }
    redraw=FALSE;
    DONT_CHANGE (hist2D.font,"Font can not be changed after initialization");
    DONT_CHANGE (core.background_pixel, 
      "Color can not be changed after initialization");
    DONT_CHANGE (primitive.foreground,
      "Color can not be changed after initialization");
    DONT_CHANGE (hist2D.topPlaneColor,
      "Color can not be changed after initialization");
    DONT_CHANGE (hist2D.leftPlaneColor,
      "Color can not be changed after initialization");
    DONT_CHANGE (hist2D.rightPlaneColor,
      "Color can not be changed after initialization");
    DONT_CHANGE (hist2D.clippingColor,
      "Color can not be changed after initialization");
/* add leftPlanePixmap and rightPlanePixmap handling here          */
    /* if double buffering changes, allocate or deallocate offscreen pixmap */
    if (new->hist2D.doubleBuffer != current->hist2D.doubleBuffer) 
    	updateBufferAllocation(new);
/* What about Z range ??? */
    if(new->hist2D.zLogScaling && (!current->hist2D.zLogScaling))
    {
        if (new->hist2D.zMin < 0.5)
            new->hist2D.zVisibleStart = 0.5;
        else 
            new->hist2D.zVisibleStart = new->hist2D.zMin;
        new->hist2D.zVisibleEnd = new->hist2D.zMax;
        new->hist2D.zLogScaling = setFRangeLogScale (&(new->hist2D.zVisibleStart), 
               &(new->hist2D.zVisibleEnd), new->hist2D.zMin, new->hist2D.zMax);
    }
    if(
	(new->hist2D.zLogScaling && (!current->hist2D.zLogScaling))||		
      ((!new->hist2D.zLogScaling) && current->hist2D.zLogScaling))
    {
        redraw=TRUE;
    }
    else if(
 	(new->hist2D.binEdgeLabeling && (!current->hist2D.binEdgeLabeling))||		
      ((!new->hist2D.binEdgeLabeling) && current->hist2D.binEdgeLabeling))
    {
        redraw=TRUE;
    }
    else if (		
      ((new->hist2D.backPlanesOn || current->hist2D.backPlanesOn)&&
      (!(new->hist2D.backPlanesOn && current->hist2D.backPlanesOn))))
    {
	redraw=TRUE;
    }
    else if (		
      ((new->hist2D.errorBarsOn || current->hist2D.errorBarsOn)&&
      (!(new->hist2D.errorBarsOn && current->hist2D.errorBarsOn))))
    {
	redraw=TRUE;
    }
    if (redraw)
        makeAllGraphics(new);
/* prohibit standard motion porcessing				*/
    new->hist2D.motion=NONE;
    new->hist2D.templateDragging=FALSE;
    return(redraw);
}

/*
** Source histogram destroy function
*/
void destroySrcHist(h2DHistSetup *hist)
{
    if (hist!=NULL)
    {
    	MYFREE(hist->bins);
    	MYFREE(hist->xLabel);
    	MYFREE(hist->yLabel);
    	MYFREE(hist->zLabel);
    	MYFREE(hist);
    }
}


/*
** Discrete map destroy function
*/
static void destroyDiscrMap(discreteMap *dMap)
{
    MYFREE(dMap);
}

static objectsToDraw *createObjectsToDraw (void)
{
    objectsToDraw *oD;
    MYMALLOC(objectsToDraw,oD,1);
    oD->template=NULL;
    oD->templateSize=0;
    oD->zAxis=NULL;
    oD->xTics=NULL;
    oD->xTicStruct = NULL;
    oD->nXTics=0;
    oD->yTics=NULL;
    oD->yTicStruct = NULL;
    oD->nYTics=0;
    oD->zTics=NULL;
    oD->zTicStruct = NULL;
    oD->nZTics=0;
    oD->arrows=NULL;
    oD->nArrows=0;
    oD->xLabels=NULL;
    oD->yLabels=NULL;
    oD->zLabels=NULL;
    oD->backPlanes=NULL;
    oD->message = NULL;
    oD->nBackPlanes=0;
    oD->xSegment = NULL;
    oD->ySegment = NULL;
    return(oD);
}

static void destroyObjectsToDraw (objectsToDraw *oD)
{
    labelsToDraw *tmp, *tmp1;
    if (oD!=NULL)
    {	
	MYFREE(oD->template);
	MYFREE(oD->zAxis);
	MYFREE(oD->xName);
	MYFREE(oD->yName);
	MYFREE(oD->zName);
	MYFREE(oD->xTics);
	MYFREE(oD->xTicStruct);
	MYFREE(oD->yTics);
	MYFREE(oD->yTicStruct);
	MYFREE(oD->zTics);
	MYFREE(oD->zTicStruct);
	MYFREE(oD->arrows);
	MYFREE(oD->backPlanes);
        destroyLabeledSegment(oD->xSegment);
        destroyLabeledSegment(oD->ySegment);
    	for (tmp=oD->xLabels; tmp!=NULL; tmp=tmp1)
	{
	    tmp1=tmp->next;
	    MYFREE(tmp->label);
	    MYFREE(tmp);
	}
	for (tmp=oD->yLabels; tmp!=NULL; tmp=tmp1)
	{
	    tmp1=tmp->next;
	    MYFREE(tmp->label);
	    MYFREE(tmp);
	}
	for (tmp=oD->zLabels; tmp!=NULL; tmp=tmp1)
	{
	    tmp1=tmp->next;
	    MYFREE(tmp->label);
	    MYFREE(tmp);
	}
	for (tmp=oD->message; tmp!=NULL; tmp=tmp1)
	{
	    tmp1=tmp->next;
	    MYFREE(tmp->label);
	    MYFREE(tmp);
	}
    }
    MYFREE(oD);
}

/*
** function to redraw axes/template picture while ratating or scaling, etc.
** To provide less flickering we use inverse GC drawing approach, so
** both objects and GCs should be provided - old whith inverse old
** GC(which is drawn, to erase it) and new to draw. 
*/
static void redrawTemplate(Display *d,Drawable w, objectsToDraw *old, 
  objectsToDraw *new, motionType mT,GC oldInvGc, GC newGc,
  GC oldInvBpGc, GC newBpGc, Boolean zRedraw)
{
    labelsToDraw *tmp;
    Boolean xChange,yChange,zChange;
    xChange=(! (mT==Y_SCALING || mT==Y_PANNING || 
      mT==Z_LOW_SCALING || mT==Z_PANNING || mT==Z_HIGH_SCALING));
    yChange=(! (mT==X_SCALING || mT==X_PANNING || 
      mT==Z_LOW_SCALING || mT==Z_PANNING || mT==Z_HIGH_SCALING));
    zChange=(! (mT==X_SCALING || mT==X_PANNING || 
      mT==Y_SCALING || mT==Y_PANNING ) || zRedraw);
    if (old!=NULL)
    {
    	if (mT==ROTATION)
            drawNames(d,w,old,oldInvGc);
    	if (xChange)
            for(tmp=old->xLabels;tmp!=NULL;tmp=tmp->next)
	    	XDrawString(d,w,oldInvGc,tmp->x,tmp->y,tmp->label,tmp->length);
    	if (yChange)
            for(tmp=old->yLabels;tmp!=NULL;tmp=tmp->next)
		XDrawString(d,w,oldInvGc,tmp->x,tmp->y,tmp->label,tmp->length);
    	if (zChange)  
            for(tmp=old->zLabels;tmp!=NULL;tmp=tmp->next)
		XDrawString(d,w,oldInvGc,tmp->x,tmp->y,tmp->label,tmp->length);
	if (xChange && old->xTics!=NULL && old->nXTics>0 )
	    XDrawSegments(d,w,oldInvGc,old->xTics,old->nXTics);
	if (yChange && old->yTics!=NULL && old->nYTics>0 )
	    XDrawSegments(d,w,oldInvGc,old->yTics,old->nYTics);
	if (zChange && old->zTics!=NULL && old->nZTics>0 )
	    XDrawSegments(d,w,oldInvGc,old->zTics,old->nZTics);
	if (old->arrows!=NULL && old->nArrows>0 )
	    XDrawSegments(d,w,oldInvGc,old->arrows,old->nArrows);
/*	if (zChange   && old->nBackPlanes > 0 && old->backPlanes != NULL)
	    XDrawSegments(d,w,oldInvBpGc,old->backPlanes,old->nBackPlanes);
*/	if (mT==ROTATION)
	{
	    if (old->template!=NULL && old->templateSize>0 )
		XDrawLines(d,w,oldInvGc,old->template,old->templateSize,
		  CoordModePrevious);
	    if (old->zAxis!=NULL)     
		XDrawSegments(d,w,oldInvGc,old->zAxis,1);
	}
    }	      
    if (new!=NULL)
    {
	if (xChange || yChange)
	    if (new->template!=NULL && new->templateSize>0 )
	        XDrawLines(d,w,newGc,new->template,new->templateSize,
	          CoordModePrevious);
	if (zChange)
	    if (new->zAxis!=NULL)     
	        XDrawSegments(d,w,newGc,new->zAxis,1);
/*	if (zChange   && new->nBackPlanes > 0 && new->backPlanes != NULL)
	    XDrawSegments(d,w,newBpGc,new->backPlanes,new->nBackPlanes);
*/	if (new->arrows!=NULL && new->nArrows>0 )
	    XDrawSegments(d,w,newGc,new->arrows,new->nArrows);
	if (xChange && new->xTics!=NULL && new->nXTics>0 )
	    XDrawSegments(d,w,newGc,new->xTics,new->nXTics);
	if (yChange && new->yTics!=NULL && new->nYTics>0 )
	    XDrawSegments(d,w,newGc,new->yTics,new->nYTics);
	if (zChange && new->zTics!=NULL && new->nZTics>0 )
	    XDrawSegments(d,w,newGc,new->zTics,new->nZTics);
    	if (xChange)
            for(tmp=new->xLabels;tmp!=NULL;tmp=tmp->next)
	    	XDrawString(d,w,newGc,tmp->x,tmp->y,tmp->label,tmp->length);
    	if (yChange)
            for(tmp=new->yLabels;tmp!=NULL;tmp=tmp->next)
		XDrawString(d,w,newGc,tmp->x,tmp->y,tmp->label,tmp->length);
    	if (zChange) 
            for(tmp=new->zLabels;tmp!=NULL;tmp=tmp->next)
		XDrawString(d,w,newGc,tmp->x,tmp->y,tmp->label,tmp->length);
        if (mT == ROTATION)
        {
            drawNames(d,w,new,newGc);
            for(tmp=new->message;tmp!=NULL;tmp=tmp->next)
		XDrawString(d,w,newGc,tmp->x,tmp->y,tmp->label,tmp->length);
	}
    }
}    	      
 	  
static void drawTemplate(Display *d,Drawable w, objectsToDraw *oD,GC gc,
  GC backPlaneGc)
{
    labelsToDraw *tmp;
    if (oD!=NULL)
    {
	{
	    if (oD->template!=NULL && oD->templateSize>0 )
	        XDrawLines(d,w,gc,oD->template,oD->templateSize,
	          CoordModePrevious);
	    if (oD->zAxis!=NULL)     
		XDrawSegments(d,w,gc,oD->zAxis,1);
/*	    if (oD->nBackPlanes > 0 && oD->backPlanes != NULL)
		XDrawSegments(d,w,backPlaneGc,oD->backPlanes,oD->nBackPlanes);
*/	}
	drawArrows(d,w,oD,gc);
	drawTics(d,w,oD,gc);
        drawNames (d,w,oD,gc);
        for(tmp=oD->message;tmp!=NULL;tmp=tmp->next)
	    XDrawString(d,w,gc,tmp->x,tmp->y,tmp->label,tmp->length);
    }
}

static void drawTics(Display *d,Drawable w, objectsToDraw *oD,GC gc)
{
    labelsToDraw *tmp;
    if (oD!=NULL)
    {
	if (oD->zAxis!=NULL)     
	    XDrawSegments(d,w,gc,oD->zAxis,1);
	if (oD->xTics!=NULL && oD->nXTics>0 )
	    XDrawSegments(d,w,gc,oD->xTics,oD->nXTics);
	if (oD->yTics!=NULL && oD->nYTics>0 )
	    XDrawSegments(d,w,gc,oD->yTics,oD->nYTics);
	if (oD->zTics!=NULL && oD->nZTics>0 )
	    XDrawSegments(d,w,gc,oD->zTics,oD->nZTics);
        for(tmp=oD->xLabels;tmp!=NULL;tmp=tmp->next)
	    XDrawString(d,w,gc,tmp->x,tmp->y,tmp->label,tmp->length);
        for(tmp=oD->yLabels;tmp!=NULL;tmp=tmp->next)
	    XDrawString(d,w,gc,tmp->x,tmp->y,tmp->label,tmp->length);
        for(tmp=oD->zLabels;tmp!=NULL;tmp=tmp->next)
	    XDrawString(d,w,gc,tmp->x,tmp->y,tmp->label,tmp->length);
    }
}    	      
static void drawArrows(Display *d,Drawable w, objectsToDraw *oD,GC gc)
{ 	  
    if (oD->arrows != NULL && oD->nArrows != 0)
	XDrawSegments(d,w,gc,oD->arrows,oD->nArrows);
}    
/*
** Widget destroy method
*/
static void destroy(Widget wg)
{
    Hist2DWidget w =(Hist2DWidget) wg;
    XFreeGC(XtDisplay(w), w->hist2D.gc);
    XFreeGC(XtDisplay(w), w->hist2D.backPlaneGc);
    XFreeGC(XtDisplay(w), w->hist2D.inverseGc);
    XFreeGC(XtDisplay(w), w->hist2D.inverseBackPlaneGc);
    XFreeGC(XtDisplay(w), w->hist2D.leftGc);
    XFreeGC(XtDisplay(w), w->hist2D.rightGc);
    XFreeGC(XtDisplay(w), w->hist2D.topGc);
    XFreeGC(XtDisplay(w), w->hist2D.clippingGc);
    if (w->hist2D.font != NULL) XmFontListFree(w->hist2D.font);
    XtRemoveAllCallbacks (wg, XmNresizeCallback);
    XtRemoveAllCallbacks (wg, XmNbtn2Callback);
    XtRemoveAllCallbacks (wg, XmNbtn3Callback);
    XtRemoveAllCallbacks (wg, XmNredisplayCallback);
    /* Free inner structures */
    if (w->hist2D.drawBuffer)
    {
    	XFreePixmap(XtDisplay(w), w->hist2D.drawBuffer);
    	w->hist2D.drawBuffer = 0;
    }    	
    XDESTROYIMAGE(w->hist2D.wireImage);
    XDESTROYIMAGE(w->hist2D.leftImage);
    XDESTROYIMAGE(w->hist2D.rightImage);
    XDESTROYIMAGE(w->hist2D.topImage);
    XDESTROYIMAGE(w->hist2D.clippingImage);
    destroySrcHist(w->hist2D.sourceHistogram);
    MYFREE(w->hist2D.bins);
    w->hist2D.bins=NULL;
    if (w->hist2D.aHist != NULL)
        MYFREE(w->hist2D.aHist->aNode);
    MYFREE(w->hist2D.aHist);
    w->hist2D.aHist=NULL;
    MYFREE(w->hist2D.topErrors);
    w->hist2D.topErrors=NULL;
    MYFREE(w->hist2D.bottomErrors);
    w->hist2D.bottomErrors=NULL;
    destroyHistStructures(w);
}

static void destroyHistStructures(Hist2DWidget w)
{
    destroyDiscrMap(w->hist2D.discrMap);
    w->hist2D.discrMap=NULL;
    destroyLabelTable(w->hist2D.xLabels);
    w->hist2D.xLabels=NULL;
    destroyLabelTable(w->hist2D.yLabels);
    w->hist2D.yLabels=NULL;
    destroyObjectsToDraw(w->hist2D.currentPicture);
    w->hist2D.currentPicture=NULL;
    destroyLabelsToDraw(w->hist2D.xNames);
    w->hist2D.xNames=NULL;
    destroyLabelsToDraw(w->hist2D.yNames);
    w->hist2D.yNames=NULL;
    destroyLabelsToDraw(w->hist2D.zNames);
    w->hist2D.zNames=NULL;
}
/* 
** myStrdup works like Strdup, but uses MYMALLOC instead malloc
** This allow to use the unique memory allocation procedure and to
** add additional memory allocation errors handling  simply
** by redefinition of MYMALLOC  (if needed)
*/

static char* myStrdup(char* src)
{
    char* copy;
    int len;
    if (src==NULL) return(NULL);
    len=strlen(src)+1;
    MYMALLOC(char,copy,len);
    return (strcpy(copy,src));
}
    

h2DHistSetup *copyHistogram (h2DHistSetup *hist)
{
    h2DHistSetup *newHist;
    if (hist==NULL) return(NULL);
    else if ((hist->nXBins==0)||(hist->nYBins==0)) return (NULL);
    MYMALLOC(h2DHistSetup,newHist,1); 
    *newHist=(*hist); /* copy non-pointer fields */
    newHist->xLabel=myStrdup(hist->xLabel);
    newHist->yLabel=myStrdup(hist->yLabel);
    newHist->zLabel=myStrdup(hist->zLabel);
    newHist->bins=NULL;
    return(newHist);
}
			    
			    

void resetHist(Hist2DWidget w)
{
    destroyHistStructures(w);
    if (w->hist2D.sourceHistogram!=NULL)
    {
        makeXYLableTables(w);
        w->hist2D.xyHistoRange=makeRange2dFromRanges(
           makeRange(0,w->hist2D.sourceHistogram->nXBins),
           makeRange(0,w->hist2D.sourceHistogram->nYBins));
        w->hist2D.xyHistoRange.start=lShiftVector(w->hist2D.xyHistoRange.start,
            LOWBITS);
        w->hist2D.xyHistoRange.end=lShiftVector(w->hist2D.xyHistoRange.end,
            LOWBITS);
	if (w->hist2D.sourceHistogram->xLabel!=NULL)
            w->hist2D.xNames=makeNameLabels(w->hist2D.sourceHistogram->xLabel,
        	w->hist2D.fs);
	if (w->hist2D.sourceHistogram->yLabel!=NULL)
            w->hist2D.yNames=makeNameLabels(w->hist2D.sourceHistogram->yLabel,
        	w->hist2D.fs);
	if (w->hist2D.sourceHistogram->zLabel!=NULL)
            w->hist2D.zNames=makeNameLabels(w->hist2D.sourceHistogram->zLabel,
        	w->hist2D.fs);
    }
    else 
       w->hist2D.xyHistoRange=makeRange2dFromRanges(
           makeRange(0,MAXPIC),
           makeRange(0,MAXPIC));
}  

/*
** Widget resize method
** It is used to generate a new picture when some picture control 
** parameters are changed because the same calculation should be performed
** in this case
*/
static void resize(Widget wg)
{
    Hist2DWidget w = (Hist2DWidget) wg;
    int borderWidth =
    	w->primitive.shadow_thickness + w->primitive.highlight_thickness;
    XRectangle clipRect;
    /* set drawing gc to clip drawing before motif shadow and highlight */
    /* resize the drawing buffer, an offscreen pixmap for smoother animation */
    updateBufferAllocation(w); 
    clipRect.x = borderWidth;
    clipRect.y = borderWidth;
    clipRect.width = w->core.width - 2 * borderWidth;
    clipRect.height = w->core.height - 2 * borderWidth;
    XSetClipRectangles(XtDisplay(w), w->hist2D.gc, 0, 0, &clipRect, 1, Unsorted);
    XSetClipRectangles(XtDisplay(w), w->hist2D.backPlaneGc, 0, 0, &clipRect, 1, Unsorted);
    XSetClipRectangles(XtDisplay(w), w->hist2D.inverseGc, 0, 0, &clipRect, 1, Unsorted);
    XSetClipRectangles(XtDisplay(w), w->hist2D.inverseBackPlaneGc, 0, 0, &clipRect, 1, Unsorted);
    XSetClipRectangles(XtDisplay(w), w->hist2D.leftGc, 0, 0, &clipRect, 1, Unsorted);
    XSetClipRectangles(XtDisplay(w), w->hist2D.rightGc, 0, 0, &clipRect, 1, Unsorted);
    XSetClipRectangles(XtDisplay(w), w->hist2D.topGc, 0, 0, &clipRect, 1, Unsorted);
    XSetClipRectangles(XtDisplay(w), w->hist2D.clippingGc, 0, 0, &clipRect, 1, Unsorted);
    updateBufferAllocation(w);
    makeAllGraphics(w);
    /* call the resize callback */
    if (XtIsRealized((Widget)w))
    	XtCallCallbacks(wg, XmNresizeCallback, NULL);
}


void makeAllGraphics(Hist2DWidget w)
{
    makeDiscreteMap(w);
    destroyObjectsToDraw(w->hist2D.currentPicture);
    w->hist2D.currentPicture=makePicture(w);
    w->hist2D.badPicture = FALSE;
    makeImage(w);
}

    
void redisplay(Widget wg, XEvent *event, Region region )
{
    Hist2DWidget w = (Hist2DWidget) wg;
    Drawable drawBuf;
    if (w->hist2D.doubleBuffer)
    	drawBuf = w->hist2D.drawBuffer;
    else
    	drawBuf = XtWindow(w);
    if (w->core.visible && XtIsRealized((Widget)w))
    {
        if(w->hist2D.shadingOn)
        {
/* add handling of shaded image */ 
	} else
	{ 
            if (!w->hist2D.templateDragging && 
              (w->hist2D.bins!=NULL || w->hist2D.aHist!=NULL))
	    {
		if (w->hist2D.wireImage!=NULL)      
	             if (w->hist2D.wireImage->data!=NULL)
	             {  
/*	        	 XClearWindow(XtDisplay(w),XtWindow(w)); */
	        	 XPutImage(XtDisplay(w),drawBuf,w->hist2D.gc,
	        	   w->hist2D.wireImage, 
	        	   0,0,0,0 /*
	        	   w->hist2D.ImageStart,
	        	   w->hist2D.ImageEnd */,
	        	   w->hist2D.wireImage->width,w->hist2D.wireImage->height)
	        	   ;
	                   drawArrows(XtDisplay(w),drawBuf,
	                     w->hist2D.currentPicture,w->hist2D.gc);
	        	   drawTics(XtDisplay(w),drawBuf,
	                     w->hist2D.currentPicture,w->hist2D.gc);
	                   drawNames(XtDisplay(w),drawBuf,
	                     w->hist2D.currentPicture,w->hist2D.gc);
                     }
             }
            else if (w->hist2D.templateDragging && !w->hist2D.badPicture)
            {
	        drawTemplate(XtDisplay(w),drawBuf,w->hist2D.currentPicture,
                  w->hist2D.gc, w->hist2D.backPlaneGc);
            }
            else 
	    {
	          XFillRectangle(XtDisplay(w), drawBuf, w->hist2D.inverseGc, 
	            0, 0,w->core.width,w->core.height);
                  destroyObjectsToDraw(w->hist2D.currentPicture);
                  w->hist2D.currentPicture=makePicture(w);
                  drawTemplate(XtDisplay(w),drawBuf,w->hist2D.currentPicture,
                  w->hist2D.gc, w->hist2D.backPlaneGc);
                  w->hist2D.badPicture = FALSE;
            }
	}
        if (w->hist2D.doubleBuffer)
    	    XCopyArea(XtDisplay(w), drawBuf, XtWindow(w), w->hist2D.gc, 0, 0,
            w->core.width, w->core.height, 0, 0);
	/* Draw the Motif required shadows and highlights */
 	drawShadow(w);
      }
}	 
	         	               
static void drawNames(Display *d, Drawable w, objectsToDraw *oD, GC gc)
{
    if (oD->xName!=NULL)
        XDrawString(d,w,gc,oD->xName->x,oD->xName->y,oD->xName->label,
          oD->xName->length);
    if (oD->yName!=NULL)
        XDrawString(d,w,gc,oD->yName->x,oD->yName->y,oD->yName->label,
          oD->yName->length);
    if (oD->zName!=NULL)
        XDrawString(d,w,gc,oD->zName->x,oD->zName->y,oD->zName->label,
          oD->zName->length);
}

static void makeDiscreteMap(Hist2DWidget w)
{
    discreteMap *dMap;
    int imageWidth;
    int imageHeight;
    double fi,psi;
    makeMargins(w);  
    fi=w->hist2D.fiViewAngle;
    psi=w->hist2D.psiViewAngle;
    adjustViewAngles(&fi,&psi);
    imageWidth  = w->core.width - 
      w->hist2D.leftMargin - w->hist2D.rightMargin;
    imageHeight = w->core.height - 
      w->hist2D.bottomMargin - w->hist2D.topMargin;
    if((w->hist2D.sourceHistogram==NULL)||
       (imageWidth<=MARGIN)||(imageHeight<=MARGIN))
    {
    	destroyDiscrMap(w->hist2D.discrMap);
    	w->hist2D.discrMap=NULL;
    	return;
    }
    if (w->hist2D.discrMap==NULL)
        MYMALLOC(discreteMap,w->hist2D.discrMap,1);
    dMap= w->hist2D.discrMap;       
    if ((imageWidth-MARGIN)/sqrt(2.)<(imageHeight-MARGIN)/sqrt(3.))
	dMap->cubeSize=(imageWidth-MARGIN)/sqrt(2.);
    else
	dMap->cubeSize=(imageHeight-MARGIN)/sqrt(3.);
    dMap->rotMatr=makeMatrix(makeVector(1,0),makeVector(0,1));
    while (fi>=M_PI/2.)
    {
    	fi-=M_PI/2.;
/* multiplication of dMap->rotMatr by 
**					/  0 1 \
**					\ -1 0 / 
**	 				          matrix
*/
	dMap->rotMatr=matrixMultMatrix(dMap->rotMatr,
	    makeMatrix(makeVector(0,1),makeVector(-1,0)));
    }
    dMap->mapLength = subVectors(
      range2dEnd(transformRange2d(w->hist2D.xyVisibleRange,dMap->rotMatr)) ,
      range2dStart(transformRange2d(w->hist2D.xyVisibleRange, dMap->rotMatr)));
    dMap->map.x.x = -sin(fi)  * (double)dMap->cubeSize;
    dMap->map.x.y =  cos(fi)  * (double)dMap->cubeSize;
    dMap->map.y.x =  - sin(psi) * -cos(fi) * (double)dMap->cubeSize;
    dMap->map.y.y =  - sin(psi) * -sin(fi) * (double) dMap->cubeSize;
/* z is scaled already to MAXPIC  */
    dMap->zFactor = - cos(psi) * (double) dMap->cubeSize; 
/* Next corrections could be performed only because of rounding errors.
** Next five conditions should be alwase false in "ideal geometry" case.
** Image generation function assume they are false, so explicit checking
** is important
*/
    if (dMap->map.x.x>0) dMap->map.x.x=0;
    if (dMap->map.x.y<=0) dMap->map.x.y=1;
    if (dMap->map.y.x<0) dMap->map.y.x=0;
    if (dMap->map.y.y<0) dMap->map.y.y=0;
    if ((abs(dMap->map.x.x)<abs(dMap->map.y.x))&&(abs(dMap->map.x.y)<abs(dMap->map.y.y)))
    	if (dMap->map.x.x==0)
    	    if (dMap->map.x.y==0)
    	        /* if fact it means an error     */
    	        if(abs(dMap->map.y.x)<abs(dMap->map.y.y))
    	            dMap->map.y.x=0;
    	        else
    	            dMap->map.y.y=0;
    	    else
    	    	dMap->map.y.y=(-dMap->map.x.y);
    	else
    	    if (dMap->map.x.y==0)
    	        dMap->map.y.x=dMap->map.x.x;
    	    else 
    	        if ((double)(abs(dMap->map.y.x)/(double)(abs(dMap->map.x.x))<
    	          (double)(abs(dMap->map.y.y)/(double)(abs(dMap->map.x.y)))))
    	            dMap->map.y.x=dMap->map.x.x;
    	        else
    	            dMap->map.y.y=(-dMap->map.x.y);
    if (dMap->zFactor>0) dMap->zFactor=0;
   dMap->vertex = addVectors(xColomn(dMap->map),yColomn(dMap->map));
    dMap->vertex = subVectors(dMap->vertex,makeVector(0,dMap->zFactor));
    dMap->vertex = divVector(dMap->vertex,2);
    dMap->vertex = addVectors(dMap->vertex,
      makeVector ((imageWidth)/2 + w->hist2D.leftMargin,
                  (imageHeight)/2 + w->hist2D.topMargin));
}

static void startMotionDispatch(discreteMap *dMap, vector cursor, motionType *motion,
  vector *markedBin, vector *markedVector,Boolean zExists)
{
    vector v;
    if (dMap==NULL) 
    {
        *motion = ROTATION;
        return ;
    }
    v=subVectors(dMap->vertex,yColomn(dMap->map));
    if ( pointInBand(cursor,v,makeVector(-2 * LONG_TIC_LEN,0))
        && (dMap->zFactor!=0) && zExists)
    {							
        double l=pointLocation(cursor,v,makeVector(0,dMap->zFactor));
        if (l>=0 && l<=1)  		     
        {
            if (l > 1./2.) 
            {                
                *motion=Z_HIGH_SCALING;
                *markedVector=v;
		return;
            }
            else 
            { 
                *motion=Z_LOW_SCALING;
                *markedVector=addVectors(v,makeVector(0,dMap->zFactor));
		return;
            }
        }
    }
    if ( pointInBand(cursor,v,makeVector(2 * LONG_TIC_LEN,0))
        && (dMap->zFactor!=0) && zExists)
    {
        if ( pointInBand(cursor,v,makeVector(0,dMap->zFactor)))
        {
            *motion=Z_PANNING;
	    return;
	}
    }
    if ( dpointInBand(doubleVector(cursor),doubleVector(dMap->vertex),
      dsetLength(
      dortogonal(doubleVector(xColomn(dMap->map))),(double) 2 * LONG_TIC_LEN))
        && vectorLength(xColomn(dMap->map)) > 1)
    {							
        double l=pointLocation(cursor,dMap->vertex,mulVector(xColomn(dMap->map),-1));
        if (l >= 0 && l <= 1)		      /* + or - here ^ ?  */
        {
	    *motion=X_SCALING;
            if (l > 1./2.)
            {
                *markedBin=dMap->mapLength;
                *markedVector=dMap->vertex;
		return;
            }
            else 
            { 
                *markedBin=makeVector(0,dMap->mapLength.y);
                *markedVector=addVectors(dMap->vertex,mulVector(xColomn(dMap->map),-1));
  		return;
            }
        }
    }
    if ( dpointInBand(doubleVector(cursor),doubleVector(dMap->vertex),
      dsetLength(
      dortogonal(doubleVector(xColomn(dMap->map))), (double) -2 * LONG_TIC_LEN))
      && vectorLength(xColomn(dMap->map)) > 1)
    {
        if ( pointInBand(cursor,dMap->vertex,mulVector(xColomn(dMap->map),-1)))
	{
            *motion=X_PANNING;
	    return;
	}
    }
    if ( dpointInBand(doubleVector(cursor),doubleVector(dMap->vertex),
      dsetLength(
      dortogonal(doubleVector(yColomn(dMap->map))), (double) -2 * LONG_TIC_LEN)) && vectorLength(yColomn(dMap->map)) > 1)
    {							
        double l=pointLocation(cursor,dMap->vertex,mulVector(yColomn(dMap->map),-1));
        if (l >= 0 && l <= 1)		      /* + or - here ^ ?  */
        {
	    *motion=Y_SCALING;
            if (l > 1./2.)
            {
                *markedBin=dMap->mapLength;
                *markedVector=dMap->vertex;
		return;
            }
            else   
            { 
                *markedBin=makeVector(dMap->mapLength.x,0);
                *markedVector=addVectors(dMap->vertex,mulVector(yColomn(dMap->map),-1));
		return;
            }
        }
    }
    if ( dpointInBand(doubleVector(cursor),doubleVector(dMap->vertex),
      dsetLength(
      dortogonal(doubleVector(yColomn(dMap->map))),(double) 2 * LONG_TIC_LEN))
      && vectorLength(yColomn(dMap->map)) > 1)
    {
        if ( pointInBand(cursor,dMap->vertex,mulVector(yColomn(dMap->map),-1)))
        {
            *motion=Y_PANNING;
	    return;
	}
    }
    *motion=ROTATION;
}


/* mark (start of BtnMotion) */    
static void markMod(Hist2DWidget w, XEvent *event, char* args, int n_args)
{
    w->hist2D.oldCursor=makeVector(event->xbutton.x,event->xbutton.y);
    w->hist2D.markedXYRange=w->hist2D.xyVisibleRange;
    w->hist2D.markedZVisibleStart=w->hist2D.zVisibleStart;
    w->hist2D.markedZVisibleEnd=w->hist2D.zVisibleEnd;
    w->hist2D.markedFiViewAngle=w->hist2D.fiViewAngle;
    w->hist2D.markedPsiViewAngle=w->hist2D.psiViewAngle;
    if (w->hist2D.bins!=NULL || w->hist2D.aHist!=NULL)
	startMotionDispatch(w->hist2D.discrMap,w->hist2D.oldCursor,
	  &w->hist2D.motion, &w->hist2D.markedBin,&w->hist2D.markedVector,TRUE);
    else	  
	startMotionDispatch(w->hist2D.discrMap,w->hist2D.oldCursor,
	  &w->hist2D.motion, &w->hist2D.markedBin,&w->hist2D.markedVector,FALSE);
    w->hist2D.templateDragging=FALSE;
}

static void mark(Hist2DWidget w, XEvent *event, char* args, int n_args)
{
    markMod(w, event, args, n_args);
    w->hist2D.templateDragging=TRUE;
    w->hist2D.badPicture=TRUE;
    redisplay((Widget)w,event,NULL);
}

static void drawShadow(Hist2DWidget w)
{
	/* Draw the Motif required shadows and highlights */
	if (w->primitive.shadow_thickness > 0) {
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




/* Draw new template while dragging if histogramm data has been updated */
void  drawNewPicture (Hist2DWidget w)
{
    if (w->hist2D.motion == NONE) return;
    if (w->hist2D.templateDragging == FALSE) return;
    dragMotionGen(w, &(w->hist2D.lastEvent), TRUE, TRUE);
}
    				         

static void dragMotion(Hist2DWidget w, XEvent *event, char* args, int n_args)
{
    if (w->hist2D.motion == NONE) return;
    dragMotionGen( w, event, TRUE, FALSE);
}

static void dragMotionMod(Hist2DWidget w, XEvent *event, char* args, int n_args)
{
    if (w->hist2D.motion == NONE) return;
    dragMotionGen( w, event, FALSE, FALSE);
}

static void dragMotionGen(Hist2DWidget w, XEvent *event, Boolean tempDragOn,
  Boolean zRedraw)
{
    vector cursor;
/* make private copy of event */    
    discreteMap *dMap=w->hist2D.discrMap;
    w->hist2D.lastEvent = *event;
    cursor=makeVector(event->xmotion.x,event->xmotion.y);
    if (w->hist2D.motion==X_SCALING || w->hist2D.motion==Y_SCALING)
    {
    	vector v;
    	range rx,ry;
    	range *cr;
    	int *mark;
    	int d;
    	vector tv;
	range2d r;
        int d0,d1,d2;
        double f;
	r=transformRange2d(w->hist2D.markedXYRange,dMap->rotMatr);
	tv=addVectors(r.start,w->hist2D.markedBin);
	r=translateRange2d(r,mulVector(tv, -1));
	rx=xRange(r);
	ry=yRange(r);
	if (w->hist2D.motion==X_SCALING)
	{
	    cr= &rx;
	    v=xColomn(dMap->map);
	}
	else
	{
	    cr= &ry;
	    v=yColomn(dMap->map);
	}
	if (cr->start==0)
	{ 
	    mark = &(cr->end);
	    d=  1;
        }	    
	else if (cr->end==0)
	{ 
	    mark = &(cr->start);
	    d= -1;
        }	    
        else 
            mark=NULL;
        d0=scalarProduct(v,w->hist2D.markedVector);
        d1=scalarProduct(v,w->hist2D.oldCursor);
        d2=scalarProduct(v,cursor);
        if (abs(d1-d0)>0) 
            f=(double)(d2-d0)/(double)(d1-d0);
        else
            f=MAXPIC;
        if (f>=1/(double)MAXPIC/2)
            *cr=rangeOf(intVector(ddivVector(doubleVector(vectorOf(*cr)),f)));
     	else
            *cr=rangeOf( mulVector(vectorOf(*cr),MAXPIC));
        if (cr->end - cr->start < MAXPIC/2)
            if (mark!=NULL)
                *mark+=d * (MAXPIC/2 - cr->end + cr->start);
        r=makeRange2dFromRanges(rx,ry);        
        r=translateRange2d(r,tv);
	r=transformRange2d(r,transposeMatrix(dMap->rotMatr));	
        w->hist2D.xyVisibleRange=adjustRange2dToRange2d(r,w->hist2D.xyHistoRange,
            MAXPIC/2);
    } 
    else if (w->hist2D.motion==X_PANNING || w->hist2D.motion==Y_PANNING)
    {
    	vector v;
    	vector tv;
	range2d r;
        double f;
	r=transformRange2d(w->hist2D.markedXYRange,dMap->rotMatr);
	tv= r.start;
	r=translateRange2d(r,mulVector(tv, -1));
	if (w->hist2D.motion==X_PANNING)
	    v=xColomn(dMap->map);
	else
	    v=yColomn(dMap->map);
	f=projectionLength(subVectors(cursor,w->hist2D.oldCursor),v);
	if (scalarProduct(v,v)!=0)
	{
	    f /= vectorLength(v);
            if (w->hist2D.motion==X_PANNING)
            {
                r.start.x = - r.end.x * f;
                r.end.x += r.start.x;
            }
            else
            {
                r.start.y = - r.end.y * f;
                r.end.y += r.start.y;
             }
         }
        r=translateRange2d(r,tv);
	r=transformRange2d(r,transposeMatrix(dMap->rotMatr));	
        w->hist2D.xyVisibleRange=moveRange2dToRange2d(r,w->hist2D.xyHistoRange);
    } else if  (w->hist2D.motion==Z_LOW_SCALING||
	w->hist2D.motion==Z_HIGH_SCALING)
    {
        int d1= w->hist2D.oldCursor.y - w->hist2D.markedVector.y;
        int d2= cursor.y - w->hist2D.markedVector.y;
        w->hist2D.zVisibleStart = w->hist2D.markedZVisibleStart;
        w->hist2D.zVisibleEnd = w->hist2D.markedZVisibleEnd;
        if (w->hist2D.motion==Z_LOW_SCALING)
        {
            if (d2 > 0 )
		pullFRangeStart(&(w->hist2D.zVisibleStart), 
		  &(w->hist2D.zVisibleEnd),
		  (double) d1 / (double) d2,
		  w->hist2D.zMin, w->hist2D.zMax,
		  w->hist2D.zLogScaling);
            else
            {            
                w->hist2D.zVisibleStart = w->hist2D.zMin;
                adjustFRange(&(w->hist2D.zVisibleStart), 
		  &(w->hist2D.zVisibleEnd),
		  w->hist2D.zMin, w->hist2D.zMax,
		  w->hist2D.zLogScaling);
            }
       	}
       	else if (w->hist2D.motion==Z_HIGH_SCALING)
        {
            if (d2 < 0 )
		pullFRangeEnd(&(w->hist2D.zVisibleStart), 
		  &(w->hist2D.zVisibleEnd),
		  (double) d1 / (double) d2,
		  w->hist2D.zMin, w->hist2D.zMax,
		  w->hist2D.zLogScaling);
	    else
            {
                w->hist2D.zVisibleEnd = w->hist2D.zMax;
                adjustFRange(&(w->hist2D.zVisibleStart), 
		  &(w->hist2D.zVisibleEnd),
		  w->hist2D.zMin, w->hist2D.zMax,
		  w->hist2D.zLogScaling);
            }
        }
    } 
    else if (w->hist2D.motion==Z_PANNING && w->hist2D.discrMap->zFactor !=0 )
    {
        double f = (double) cursor.y - w->hist2D.oldCursor.y;
        w->hist2D.zVisibleStart = w->hist2D.markedZVisibleStart;
        w->hist2D.zVisibleEnd = w->hist2D.markedZVisibleEnd;
        f /= (double) - w->hist2D.discrMap->zFactor;
        moveFRange(&(w->hist2D.zVisibleStart), 
	  &(w->hist2D.zVisibleEnd),
	  f,
	  w->hist2D.zMin, w->hist2D.zMax,
	  w->hist2D.zLogScaling);
    }
    else if (w->hist2D.motion==ROTATION)
    {
	w->hist2D.fiViewAngle = w->hist2D.markedFiViewAngle -
          (double)(cursor.x - w->hist2D.oldCursor.x) /
	  (double)(w->core.width)*M_PI;
	w->hist2D.psiViewAngle = w->hist2D.markedPsiViewAngle + 
          (double)(cursor.y - w->hist2D.oldCursor.y)/
	  (double)(w->core.height)*M_PI;
	adjustViewAngles(&w->hist2D.fiViewAngle,&w->hist2D.psiViewAngle);
    } 
    if (tempDragOn)
    {
    
/*  	if (XPeekIfEvent(XtDisplay(w), &evn, pushBtnWaiting, (char*) &win))
  	{
  	    w->hist2D.templateDragging=TRUE;
  	    w->hist2D.badPicture = TRUE;
  	}
  	else */ if (w->hist2D.templateDragging)
	{
	    Drawable drawBuf;
   	    objectsToDraw *oD;
	    if (w->hist2D.doubleBuffer)
    		drawBuf = w->hist2D.drawBuffer;
	    else
    		drawBuf = XtWindow(w);
            makeDiscreteMap(w);
            oD=makePicture(w);
            redrawTemplate(XtDisplay(w),drawBuf,w->hist2D.currentPicture,
            oD,w->hist2D.motion,w->hist2D.inverseGc,w->hist2D.gc, 
            w->hist2D.inverseBackPlaneGc, w->hist2D.backPlaneGc, zRedraw);
	    if (w->hist2D.doubleBuffer)
    		XCopyArea(XtDisplay(w), drawBuf, XtWindow(w), w->hist2D.gc, 0, 0,
    			  w->core.width, w->core.height, 0, 0);
	    drawShadow(w);
            destroyObjectsToDraw (w->hist2D.currentPicture);
            w->hist2D.currentPicture=oD;
	}
	else
	{
	    w->hist2D.templateDragging=TRUE;
	    w->hist2D.badPicture=TRUE;
	    redisplay((Widget)w,event,NULL);
	}
    }
    else
    {
        w->hist2D.templateDragging = FALSE;
/*  	if (XPeekIfEvent(XtDisplay(w), &evn, noCtrlWaiting, (char*) &win))
  	    w->hist2D.badPicture = TRUE;
  	else
*/    	    makeAllGraphics(w);
/*  	if (XPeekIfEvent(XtDisplay(w), &evn, noCtrlWaiting, (char*) &win))
  	    w->hist2D.badPicture = TRUE;
  	else
*/	    redisplay((Widget)w,event,NULL);
    }
}


static void stopDragging(Hist2DWidget w, XEvent *event, char* args, int n_args)
{
/* event has wrong type for next call, but event information will not be
really used */
   dragMotionGen( w, event, FALSE,FALSE); 
   w->hist2D.motion = NONE;
   w->hist2D.templateDragging = FALSE;
}
static XPoint *makeTemplate(discreteMap *dMap, int *Size)
{
    XPoint  *curPoint, *points;
    if (dMap==NULL) return(NULL); 
    MYMALLOC(XPoint,points,5);
    curPoint=points;	
    *curPoint++ = xPoint(subVectors(dMap->vertex,yColomn(dMap->map)));
    *curPoint++ = xPoint(mulVector(xColomn(dMap->map),-1));
    *curPoint++ = xPoint(yColomn(dMap->map));
    *curPoint++ = xPoint(xColomn(dMap->map));
    *curPoint++ = xPoint(mulVector(yColomn(dMap->map),-1));
    *Size=5;
    return(points);
}
static XSegment *makeZAxis(discreteMap *dMap)
{
    XSegment *seg;
    if (dMap==NULL) return (NULL);
    MYMALLOC(XSegment,seg,1);
    seg->x1=subVectors(dMap->vertex,yColomn(dMap->map)).x;
    seg->y1=subVectors(dMap->vertex,yColomn(dMap->map)).y;
    seg->x2=addVectors(subVectors(dMap->vertex,yColomn(dMap->map)),
      makeVector(0,dMap->zFactor)).x;
    seg->y2=addVectors(subVectors(dMap->vertex,yColomn(dMap->map)),
      makeVector(0,dMap->zFactor)).y;
    return(seg);
}

			    
static objectsToDraw *makePicture (Hist2DWidget w)
{
    objectsToDraw *oD=createObjectsToDraw();
    oD->template=makeTemplate(w->hist2D.discrMap,&(oD->templateSize));
    oD->zAxis=makeZAxis(w->hist2D.discrMap);
    if (w->hist2D.aHist == NULL && w->hist2D.binEdgeLabeling)
	makeBaseLabels (w->hist2D.fs, w->hist2D.discrMap, w->hist2D.xLabels, 
	  w->hist2D.yLabels, w->hist2D.xyVisibleRange, oD);
    else 
    {
  	makeBaseUnbinnedLabels(w->hist2D.fs, w->hist2D.discrMap, 
     	  w->hist2D.xyVisibleRange, w->hist2D.xyHistoRange,
     	  w->hist2D.sourceHistogram, oD);
    
    }
    if (w->hist2D.bins!=NULL || w->hist2D.aHist!=NULL)
	makeZLabels (w, oD);
    else
    {
        oD->nZTics=0;
        oD->zLabels=NULL;
        oD->zTics=NULL;
    }
    makeNameLabelsToDraw (w,  oD);
    makeArrows (w,  oD);
    makeMessageToDraw(w, oD);
    return(oD);
}
			    
/*
** For smoother animation, it is possible to allocate an offscreen pixmap
** and draw to that rather than directly into the window.  Unfortunately,
** it is too slow on many machines, so we have to give the user the choice
** whether to use it or not.  This routine reallocates the offscreen
** pixmap buffer when the window is resized, and when the widget is created
*/
static void updateBufferAllocation(Hist2DWidget w)
{ 
    if (w->hist2D.drawBuffer)
    	XFreePixmap(XtDisplay(w), w->hist2D.drawBuffer);
    if (w->hist2D.doubleBuffer) {
    	w->hist2D.drawBuffer = XCreatePixmap(XtDisplay(w),
		DefaultRootWindow(XtDisplay(w)), w->core.width, w->core.height,
    	 	DefaultDepthOfScreen(XtScreen(w)));   
     	XFillRectangle(XtDisplay(w), w->hist2D.drawBuffer, w->hist2D.inverseGc, 
	  0, 0,w->core.width,w->core.height);
    } else {
    	w->hist2D.drawBuffer = 0;
    }
}			    
			    
    

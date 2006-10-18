#include <X11/Intrinsic.h> 
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#if XmVersion == 1002
#include <Xm/PrimitiveP.h>
#endif
#include <Xm/DrawingA.h>
#include <Xm/RowColumn.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "2DGeom.h"
#include "uniLab.h"
#include "2DHistDefs.h"
#include "2DHist.h"
#include "labels.h"
#include "2DHistP.h"
#include "readhist.h"
#include "aHistRead.h"
static void setView(Widget w, caddr_t cData, 
  XmDrawingAreaCallbackStruct *call_data);
static void makePs(Widget w, caddr_t cData, 
  XmDrawingAreaCallbackStruct *call_data);
static void testDestroy(Widget w, caddr_t cData, 
  XmDrawingAreaCallbackStruct *call_data);
static void testUpdate(Widget w, caddr_t cData, 
  XmDrawingAreaCallbackStruct *call_data);

Widget toplevel, histW, rowcol;
char *fileName;
char fName[80];
int cnt = 0;
void main(argc, argv)
  unsigned   argc;
  char *argv[];
{
    h2DHistSetup *hist;
    int i,j;
    int strat;
    float *topErrors, *bottomErrors;
    float *tmp, *tmp1;
    aHistStruct aHIST;
    aHistStruct *aHISTP = &aHIST;
    aHistNode aHist[13];
    aHistStruct *aHistR;
    int nLimit; /* for adaptive histogram */
    aHIST.aNode = aHist;
    /*
     * Initialize the Intrinsics.
     */   
    toplevel = XtInitialize(argv[0], "HistTest", NULL, 
                            0, &argc, argv);
    /*
     * Create a widget and add a select callback.
     */
    if (argv[1]!=NULL)
    {
        strcpy(fName,argv[1]);
        fileName=fName;
    }
    else
        fileName=NULL;
        
    nLimit = 100;
    if (argc > 2)
        if (argv[2] != NULL)
            nLimit = atoi(argv[2]);
/*    if (nLimit < 10) nLimit = 10;   */ 
        
/* a */    hist=readHist(NULL,0);
    hist->xScaleType=H2D_LINEAR;
    hist->yScaleType=H2D_LINEAR;
    hist->xMin=3.25;
    hist->xMax=5.75;
    hist->xScaleBase=2.;
    hist->yScaleBase=2.;
    hist->xLabel="This is X axis";
    hist->yLabel="This is Y axis";
    hist->zLabel="This is Z axis";
    aHIST.xMin=0.;
    aHIST.xMax=1000.;
    aHIST.yMin=0.;
    aHIST.yMax=1000.;
    aHIST.aNode= aHist;
/* a */  
    if (argc > 3) strat = atoi(argv[3]);
    else strat =0;
    
    if (argc > 1) aHistR = readAHist(argv[1], nLimit,strat);
    else aHistR = NULL;
    if (aHistR != NULL)
        aHISTP = aHistR;
    hist->xMin = aHISTP->xMin;
    hist->xMax = aHISTP->xMax;
    hist->yMin = aHISTP->yMin;
    hist->yMax = aHISTP->yMax;    
    
/*    rowcol = XtCreateManagedWidget("rowcol", xmRowColumnWidgetClass,
    				     toplevel,NULL,0);
    for (i=0; i< 4; i++)
    {
    
*/    
    histW = XtCreateManagedWidget("hist2D", hist2DWidgetClass, 
                        	 toplevel, NULL, 0);
/*    XtFree(hist->bins);
    hist->bins=NULL;                        	 
*/    
    XtAddCallback(histW, XmNbtn3Callback, (XtCallbackProc) makePs, NULL); 
    hist2DSetHistogram(histW,hist);
    j=sizeof(float)* hist->nXBins * hist->nYBins;
    topErrors=tmp=(float*)XtMalloc(j);
    tmp1 = hist->bins;
    for(i=hist->nXBins;i>0;i--)
	for(j=hist->nYBins;j>0;j--)
    {
      	*tmp = sqrt (*tmp1);
      	tmp++;
      	tmp1++;
    }
    j=sizeof(float)* hist->nXBins * hist->nYBins;
    bottomErrors=tmp=(float*)XtMalloc(j);
    tmp1 = hist->bins;
    for(i=hist->nXBins;i>0;i--)
	for(j=hist->nYBins;j>0;j--)
    {
      	*tmp = - sqrt (*tmp1);
      	tmp++;
      	tmp1++;
      	
    }

/*    }
*/                        	 
    aHist[0].nextNodeOffset = 4;
    aHist[0].data.xySplit = 500.;
    aHist[1].nextNodeOffset = 2;
    aHist[1].data.xySplit = 500.;
    aHist[2].nextNodeOffset = 0;
    aHist[2].data.zData = 1.;
    aHist[3].nextNodeOffset = 0;
    aHist[3].data.zData = 2.;
    
    aHist[4].nextNodeOffset = 4;
    aHist[4].data.xySplit = 500.;
    aHist[5].nextNodeOffset = 2;
    aHist[5].data.xySplit = 750.;
    aHist[6].nextNodeOffset = 0;
    aHist[6].data.zData = 3.;
    aHist[7].nextNodeOffset = 0;
    aHist[7].data.zData = 4.;
    
    aHist[8].nextNodeOffset = 2;
    aHist[8].data.xySplit = 750.;
    aHist[9].nextNodeOffset = 0;
    aHist[9].data.zData = 5.;
    
    aHist[10].nextNodeOffset = 2;
    aHist[10].data.xySplit = 750.;
    aHist[11].nextNodeOffset = 0;
    aHist[11].data.zData = 6.;
    aHist[12].nextNodeOffset = 0;
    aHist[12].data.zData = 7.;
    hist2DSetAdaptiveHistogramData (histW, aHISTP, HIST2D_RESCALE_AT_MAX );
    XtRealizeWidget(toplevel);
    XtMainLoop();
}

static void setView(Widget w, caddr_t cData, XmDrawingAreaCallbackStruct *call_data)
{
   float fi,psi;
   int xS,yS,xL,yL;
   float zS,zL;
   int noerr=1;
   printf("\n\rxS  ");
   noerr&=scanf( "%u",&xS);
   printf("\n\rxL ");
   noerr&=scanf( "%u",&xL);
   printf("\n\ryS  ");
   noerr&=scanf( "%u",&yS);
   printf("\n\ryL ");
   noerr&=scanf( "%u",&yL);
   printf("\n\rzS  ");
   noerr&=scanf( "%e",&zS);
   printf("\n\rzL  ");
   noerr&=scanf( "%e",&zL);
   if (noerr)
   {
       hist2DSetVisiblePart(w,xS,xL,yS,yL,zS,zL);
   }
}
static void makePs(Widget w, caddr_t cData, XmDrawingAreaCallbackStruct *call_data)
{
    hist2DmakePsImage(w, "image.ps");
}

static void testDestroy(Widget w, caddr_t cData, XmDrawingAreaCallbackStruct *call_data)
{
    XtDestroyWidget(toplevel);
}

static void testUpdate(Widget w, caddr_t cData, XmDrawingAreaCallbackStruct *call_data)
{
    h2DHistSetup *hist;
    switch (cnt) {
    	case 0:  
	hist=readHist(fileName,6);
	hist2DUpdateHistogramData(w,hist->bins,NULL,NULL,HIST2D_NO_SCALING);
        destroySrcHist(hist);
	break;	
    	case 1: 
	hist2DUpdateHistogramData(w,NULL,NULL,NULL,HIST2D_NO_SCALING);
	break;	
    	case 2:
        hist2DSetHistogram(w,NULL);
        break;
        case 3:
	hist=readHist(NULL,3);
        hist2DSetHistogram(w,hist);
        destroySrcHist(hist);
        break;
        case 4:  
	hist2DUpdateHistogramData(w,NULL,NULL,NULL,HIST2D_NO_SCALING);
	break;
	case 5:  
	hist=readHist(NULL,5);
	hist2DUpdateHistogramData(w,hist->bins,NULL,NULL,HIST2D_NO_SCALING);
        destroySrcHist(hist);
	break;	
        case 6:  
	hist=readHist(fileName,6);
        hist2DSetHistogram(w,hist);
        destroySrcHist(hist);
	break;
	}
	cnt = (cnt+1)%7;


}


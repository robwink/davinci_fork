
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h> 
#include <Xm/Xm.h>
#include <Xm/DrawingA.h>
#include <X11/Xutil.h>
#include <math.h>
#include <stdio.h>
#include "2DHist.h"
#include "readhist.h"

h2DHistSetup * readHist(char* name,int n)
{
    float *tmp;
    unsigned i,j;
    h2DHistSetup * hist;
    FILE *ifil;
    
    if ((name != NULL) && (ifil=fopen(name, "r"))!=NULL)
    {
	hist=(h2DHistSetup *)XtCalloc(sizeof(h2DHistSetup ),1);
	hist->xLabel = NULL ;
	hist->yLabel = NULL;
	hist->zLabel = NULL;
        hist->xScaleType=H2D_LINEAR;
        hist->yScaleType=H2D_LINEAR;
        hist->xScaleBase=10.;
        hist->yScaleBase=10.;
	fscanf(ifil,"%u%u",&hist->nXBins,&hist->nYBins);
	    j=sizeof(float)* hist->nXBins * hist->nYBins;
	hist->bins=tmp=(float*)XtMalloc(j);

	for(i=hist->nXBins*hist->nYBins;i>0;i--)
	{
    	    fscanf(ifil,"%*e%*e%e",tmp);
    	    tmp++;
	}
	fclose(ifil);
	return (hist);
    } else
    {
	hist=(h2DHistSetup *)XtCalloc(sizeof(h2DHistSetup ),1);
	hist->xLabel = NULL ;
	hist->yLabel = NULL;
	hist->zLabel = NULL;
	hist->nXBins=hist->nYBins=10;
	if (n!=0) hist->nXBins *= n;
	j=sizeof(float)* hist->nXBins * hist->nYBins;
	hist->bins=tmp=(float*)XtMalloc(j);

	for(i=hist->nXBins;i>0;i--)
	    for(j=hist->nYBins;j>0;j--)
	{
/*    	    *tmp++=i+j; */
      	    *tmp++=0;
	}
	return (hist);
    }
    
}
         
    

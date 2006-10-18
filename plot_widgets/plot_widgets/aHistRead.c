#include <X11/StringDefs.h>
#include <X11/Intrinsic.h> 
#include <X11/Xmd.h>
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#if XmVersion == 1002
#include <Xm/PrimitiveP.h>
#endif
#include <stdio.h>
#include "2DHist.h"
#include "readhist.h"
#include "adaptHist.h"
#include "fltRange.h"

aHistStruct * readAHist(char* name, int nLimit,int strategy)
{
    FILE *ifil;
    int i,j;
    float x, y;
    t *data, *tmp;
    if ((name != NULL) && (ifil=fopen(name, "r"))!=NULL)
    {
        i = 0;
        while (	fscanf(ifil,"%f%f",&x,&y) == 2) i++;
	rewind (ifil);
	data=(t *)XtCalloc(sizeof(t),i+1);
	j = 0;
        while (	fscanf(ifil,"%f%f",&((data+j)->c[_X]),&((data+j)->c[_Y]))==2 )
            j++;
	if (i != j ) printf("data reading error\n");
        return ( Bin2DAdaptHist(data, i, nLimit, strategy));
    }
    else
        return(NULL);
}                

         
 	        
 	     
 		                
    

  

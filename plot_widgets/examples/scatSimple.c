#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <Xm/Xm.h>
#include "../plot_widgets/Scat.h"

#define NUM_POINTS 10000

main(int argc, char *argv[])
{
    XtAppContext appContext;
    Widget toplevel, scat; 
    XmString s1, s2; 
    int i;
    ScatPoint *point, points[NUM_POINTS];
    
    /* Initialize X and Xt */   
    toplevel = XtAppInitialize(&appContext, "ScatTest", NULL,
        0, &argc, argv, NULL, NULL, 0);
    
    /* Create a scat widget to display the points */
    scat = XtVaCreateManagedWidget("Scat", scatWidgetClass, toplevel,
            XmNxAxisLabel, s1=XmStringCreateSimple("X Axis Label"),
            XmNyAxisLabel, s2=XmStringCreateSimple("Y Axis Label"),
            XmNdarkerPoints, False,
	    NULL); 
    XmStringFree(s1);    	
    XmStringFree(s2);
    
    /* Create data for the plot and display it */
    srand(1);
    for (point = points, i=0; i<NUM_POINTS; i++, point++) {
        point->x = pow((float)rand() / RAND_MAX - .5, 3.);
        point->y = pow((float)rand() / RAND_MAX - .5, 3.);
        point->pixel = BlackPixelOfScreen(XtScreen(scat));
    }
    ScatSetContents(scat, points, NUM_POINTS, SCAT_RESCALE);
    
    /* Realize the widgets and call XtMainLoop to continuously process events */
    XtRealizeWidget(toplevel);
    XtAppMainLoop(appContext);
}


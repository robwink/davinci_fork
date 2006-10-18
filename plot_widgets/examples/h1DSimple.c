#include <stdio.h>
#include <Xm/Xm.h>
#include "../plot_widgets/H1D.h"

#define NUM_BINS   50
#define X_MIN      0.0
#define X_MAX      100.0

main(int argc, char *argv[])
{
    XtAppContext appContext;
    Widget toplevel, h1dWidget;
    XmString s1, s2;
    float bins[NUM_BINS], *bin;
    int i;
    
    /* Initialize the X and Xt and create a shell */
    toplevel = XtAppInitialize(&appContext, "H1DTest", NULL, 0, &argc, argv,
    	    NULL, NULL, 0);
    	
    /* Create a H1D widget */
    h1dWidget = XtVaCreateManagedWidget("h1dWidget",
    	    h1DWidgetClass, toplevel, 
    	    XmNlogScaling, FALSE,
    	    XmNxAxisLabel, s1=XmStringCreateSimple("X Axis Label"),
    	    XmNyAxisLabel, s2=XmStringCreateSimple("Y Axis Label"),
    	    NULL);
    XmStringFree(s1);
    XmStringFree(s2);
    
    /* Set the bin contents */
    for (i = 0, bin = bins; i < NUM_BINS; i++, bin++) {
        *bin = (float)i;
        if (i > NUM_BINS / 2) 
            *bin = NUM_BINS - *bin;
    }
    H1DSetContents(h1dWidget, X_MIN, X_MAX, NUM_BINS, bins, NULL, NULL,
    	    H1D_RESCALE);
    	
    /* Realize the top level widget and give control to the X event loop */
    XtRealizeWidget(toplevel);
    XtAppMainLoop(appContext);
}

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <Xm/Xm.h>
#include "../plot_widgets/3DScat.h"

#define NUM_POINTS 10000

main(int argc, char *argv[])
{
    XtAppContext appContext;
    Widget toplevel, scat3D;
    int i, c;
    Scat3DPoint *point, points[NUM_POINTS];
    
    /* Initialize X and Xt */   
    toplevel = XtAppInitialize(&appContext, "Scat3DTest", NULL,
            0, &argc, argv, NULL, NULL, 0);

    /* Create a Scat3D widget to display the points. */
    scat3D = XtVaCreateManagedWidget("Scat3D", scat3DWidgetClass, toplevel,
            XmNdarkerPoints, FALSE, NULL); 
    
    /* Label the axes */
    Scat3DSetAxesNames(scat3D, "X Axis", "Y Axis", "Z Axis");
    
    /* Create data for the plot */
    srand(1);
    for (point = points, i=0; i<NUM_POINTS; i++, point++) {
        point->x = pow((float)rand() / RAND_MAX - .5, 3.);
        point->y = pow((float)rand() / RAND_MAX - .5, 3.);
        point->z = pow((float)rand() / RAND_MAX - .5, 3.);
    }
    
    /* Display the data */
    Scat3DSetContents(scat3D, points, NUM_POINTS, SCAT3D_RESCALE);
    
    /* Realize the widgets and call XtMainLoop to continuously process events */
    XtRealizeWidget(toplevel);
    XtAppMainLoop(appContext);
}


#include <stdio.h>
#include <Xm/Xm.h>
#include "XY.h"

#define NUM_POINTS  10
static XYPoint Points[NUM_POINTS] = {{1,4}, {2,5}, {3,7}, {4,8}, {5,0},
	{6,3}, {7,9}, {8,6}, {9,2}, {10,1}};

main(int argc, char *argv[])
{
    XtAppContext app_context;
    Widget toplevel, xyW;
    XmString s1, s2;
    XYCurve curve;
    
    /* Initialize X and the X toolkit */
    toplevel = XtAppInitialize(&app_context, "CurvesTest", NULL,
	    0, &argc, argv, NULL, NULL, 0);    
    		
    /* Create a curve widget. */
    xyW = XtVaCreateManagedWidget("XYWidget", xyWidgetClass, toplevel, 
    	    XmNxAxisLabel, s1=XmStringCreateSimple("X Axis Label"),
    	    XmNyAxisLabel, s2=XmStringCreateSimple("Y Axis Label"),
    	    NULL);
    XmStringFree(s1);
    XmStringFree(s2);

    /* Set the contents */
    curve.name = XmStringCreateSimple("Variable 1");
    curve.markerStyle = XY_CIRCLE_MARK;
    curve.markerSize = XY_MEDIUM;
    curve.lineStyle = XY_PLAIN_LINE;
    curve.markerPixel = BlackPixelOfScreen(XtScreen(xyW));
    curve.linePixel = curve.markerPixel;
    curve.nPoints = NUM_POINTS;
    curve.points = Points;
    curve.horizBars = NULL;
    curve.vertBars = NULL;
    XYSetContents(xyW, &curve, 1, XY_RESCALE); 
    XmStringFree(curve.name);
  
    /* Realize widgets and call XtMainLoop to continuously process events */
    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}

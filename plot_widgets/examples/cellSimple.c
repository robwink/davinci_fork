#include <stdio.h>
#include <Xm/Xm.h>
#include "../plot_widgets/Cell.h"

#define NUM_X_BINS 50
#define NUM_Y_BINS 30
#define X_MIN 0.
#define X_MAX 20.
#define Y_MIN 0.
#define Y_MAX 10.

main(int argc, char *argv[])
{
    XtAppContext appContext;
    Widget toplevel, cell;
    float bins[NUM_X_BINS][NUM_Y_BINS];
    float minBinValue, maxBinValue, xBinWidth, yBinWidth, xScale, yScale;
    CellRect *rect, rects[NUM_X_BINS * NUM_Y_BINS];
    XmString s1, s2;
    int x, y;
    
    /* Initialize X and Xt */
    toplevel = XtAppInitialize(&appContext, "CellTest", NULL,
	    0, &argc, argv, NULL, NULL, 0);    	
    
    /* Create a Cell widget */
    cell = XtVaCreateManagedWidget("CellWidget", cellWidgetClass, toplevel,
	    XmNxAxisLabel, s1=XmStringCreateSimple("X Axis Label"),
            XmNyAxisLabel, s2=XmStringCreateSimple("Y Axis Label"),
	    NULL);
    XmStringFree(s1);
    XmStringFree(s2);
    
    /* Create sample data */
    for (x=0; x<NUM_X_BINS; x++) {
    	for (y=0; y<NUM_Y_BINS; y++) {
    	    bins[x][y] = (x < NUM_X_BINS/2 ? x : NUM_X_BINS - x) +
    	    	    (y < NUM_Y_BINS/2 ? y : NUM_Y_BINS - y);
    	}
    }

    /* Calculate constants used in creating rectangles for cell widget */
    minBinValue = 0;
    maxBinValue = NUM_X_BINS/2 + NUM_Y_BINS/2;
    xBinWidth = (X_MAX - X_MIN)/NUM_X_BINS;
    yBinWidth = (Y_MAX - Y_MIN)/NUM_Y_BINS;
    xScale = xBinWidth / (maxBinValue - minBinValue);
    yScale = yBinWidth / (maxBinValue - minBinValue);
    
    /* Create the rectangles for the cell widget to draw */
    rect = rects;
    for (x=0; x<NUM_X_BINS; x++) {
    	for (y=0; y<NUM_Y_BINS; y++) {
    	    rect->x = X_MIN + x*xBinWidth + xBinWidth/2;
    	    rect->y = Y_MIN + y*yBinWidth + yBinWidth/2;
    	    rect->dx = (bins[x][y] - minBinValue) * xScale;
    	    rect->dy = (bins[x][y] - minBinValue) * yScale;
    	    rect->pixel = BlackPixel(XtDisplay(cell), 0);
    	    rect++;
    	}
    }
    
    /* Give the rectangles to the cell widget to display */
    CellSetContents(cell, rects, NUM_X_BINS * NUM_Y_BINS, CELL_RESCALE);
    
    /* Realize widgets and call XtMainLoop to continuously process events */
    XtRealizeWidget(toplevel);
    XtAppMainLoop(appContext);
}


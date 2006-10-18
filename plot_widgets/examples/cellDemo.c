/***********************************************************************
*                                                                      *
* cellExample.c - Cell Plot Widget Example Program                     *
*                                                                      *
* This program demonstrates the capabilities of the Cell widget        *
*                                                                      *
* September 9, 1994 by George Dimas and Mark Edel                      *
*                                                                      *
***********************************************************************/
#include <limits.h>
#include <stdio.h>
#include <float.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include "../util/printUtils.h"
#include "../plot_widgets/Cell.h"
#include "../plot_widgets/2DHelp.xbm"
#include "demoUtils.h"

#define NUM_X_BINS 50
#define NUM_Y_BINS 50

static Widget Cell;

static void createRects(Widget Cell, CellRect **rects, int *nRects);
static float *createBins(void);
static void doubleBufferCB(Widget w, XtPointer clientD, XtPointer callD);
static void helpCallback(Widget w, XtPointer clientData, XtPointer callData);
static void setRangeCallback(Widget w, XtPointer clientD, XtPointer callD);
static void zoomCallback(Widget w, XtPointer clientD, XtPointer callD);
static void exitCallback(Widget w, XtPointer clientD, XtPointer callD);
static void printCallback(Widget w, XtPointer clientD, XtPointer callD);


main(int argc, char *argv[])
{
    XtAppContext appContext;
    Widget toplevel, form, rowColumn;
    XmString s1, s2;
    CellRect *rects;
    int nRects;
        
    /* Initialize X and the X Toolkit */   
    toplevel = XtAppInitialize(&appContext, "CellTest", NULL,
            0, &argc, argv, NULL, NULL, 0);
    
    /* Create a form widget as the container for rowcolumn, 
       Cell, and checkbox. */
    form = XtCreateManagedWidget("form", xmFormWidgetClass, toplevel, NULL, 0);

    /* Create a Cell widget */
    Cell = XtVaCreateManagedWidget("Cell", cellWidgetClass, form,
            XmNxAxisLabel,       s1 = XmStringCreateSimple("X Axis Label"),
            XmNyAxisLabel,       s2 = XmStringCreateSimple("Y Axis Label"),
            XmNtopAttachment,    XmATTACH_WIDGET,
    	    XmNleftAttachment,   XmATTACH_POSITION,
    	    XmNleftPosition,     26,
    	    XmNrightAttachment,  XmATTACH_WIDGET,
    	    XmNbottomAttachment, XmATTACH_WIDGET, NULL); 
    XmStringFree(s1);    	
    XmStringFree(s2);

    /* Create data for the Cell widget & display it */
    createRects(Cell, &rects, &nRects); 
    CellSetContents(Cell, rects, nRects, CELL_RESCALE);

    /* Create a rowColumn widget  to hold toggle and push buttons. */
    rowColumn = XtVaCreateManagedWidget("rowColumn", xmRowColumnWidgetClass,
    	form,
    	XmNtopAttachment,    XmATTACH_FORM,
    	XmNtopOffset,        10,
    	XmNleftAttachment,   XmATTACH_WIDGET,
    	XmNrightAttachment,  XmATTACH_POSITION,
    	XmNrightPosition,    25,
    	XmNbottomAttachment, XmATTACH_WIDGET, NULL);
    
    /* Create toggle and push buttons */
    createToggleButton(rowColumn, "doubleBuffer",
    	    "Double Buffer", doubleBufferCB, NULL);
    createButton(rowColumn, "controlsHelp", "Controls Help...",
	    helpCallback, NULL);
    createButton(rowColumn, "setVisibleRange", "Set Visible Range...",
	    setRangeCallback, NULL);
    createButton(rowColumn, "zoomIn", "Zoom In", zoomCallback, (XtPointer)1);
    createButton(rowColumn, "zoomOut", "Zoom Out", zoomCallback, (XtPointer)2);
    createButton(rowColumn, "resetZoom", "Reset Zoom",
    	    zoomCallback, (XtPointer)3);
    createButton(rowColumn, "printPlot", "Print Plot...",
	    printCallback, NULL);
    createButton(rowColumn, "exitProgram", "Exit Program",
	    exitCallback, NULL);
    
    /* Display all widgets and call XtMainLoop to continuously process
       all events. */
    XtRealizeWidget(toplevel);
    XtAppMainLoop(appContext);
}

static void createRects(Widget Cell, CellRect **rects, int *nRects)
{
    /* Create an array of CellRect structures for the Cell widget
       to display  */ 
       
    int i, j;
    float *bins;
    float xMin = -1.0, yMin = -1.0, xMax = 1.0, yMax = 1.0;
    float xBinWidth, yBinWidth, xScale, yScale, binValue;
    float minBinValue = FLT_MAX, maxBinValue = -FLT_MAX;
    CellRect *rect;

    /* Create data in the form an array of histogram bin values */
    bins = createBins();
    
    /* find the range of the data: minBinValue & maxBinValue */
    for (i=0; i<NUM_X_BINS; i++) {
    	for (j=0; j<NUM_Y_BINS; j++) {
    	    binValue = bins[i*NUM_Y_BINS + j];
    	    if (binValue < minBinValue) minBinValue = binValue;
    	    if (binValue > maxBinValue) maxBinValue = binValue;
	}
    }
    
    /* calculate constants used in creating rectangles for cell widget */
    xBinWidth = (xMax - xMin)/NUM_X_BINS;
    yBinWidth = (yMax - yMin)/NUM_Y_BINS;
    xScale = xBinWidth / (maxBinValue - minBinValue);
    yScale = yBinWidth / (maxBinValue - minBinValue);
    *nRects = NUM_X_BINS * NUM_Y_BINS;
    
    /* create the rectangles for the cell widget to draw */
    *rects = (CellRect *)XtMalloc(*nRects * sizeof(CellRect));
    rect = *rects;
    for (i=0; i<NUM_X_BINS; i++) {
    	for (j=0; j<NUM_Y_BINS; j++) {
    	    binValue = bins[i*NUM_Y_BINS + j];
    	    rect->x = xMin + i*xBinWidth + xBinWidth/2;
    	    rect->y = yMin + j*yBinWidth + yBinWidth/2;
    	    rect->dx = (binValue - minBinValue) * xScale;
    	    rect->dy = (binValue - minBinValue) * yScale;
    	    rect->pixel = BlackPixel(XtDisplay(Cell), 0);
    	    rect++;
    	}
    }
    XtFree((char *)bins);
}

static float *createBins()
{
    /* Create binned data for the cell widget to plot.
       The points will be arranged to display the FermiLab logo.
       First, a two dimensional array is created with twice the
       size of NUM_X_BINS by NUM_Y_BINS (bins2) and another two
       dimensional array of NUM_X_BINS by NUM_Y_BINS (bins). Second,
       four points will be averaged and placed in the bins array.
       A pointer to the beginning of the points is returned. */ 
       
    int i, j;
    float x, y, *bins, *logo, *bin, total, *logoRow1, *logoRow2;
    
    bins = (float *)XtMalloc(sizeof(float) * NUM_X_BINS * NUM_Y_BINS);
    logo = (float *)XtMalloc(sizeof(float) * NUM_X_BINS * NUM_Y_BINS * 4);   
    
    /* Create Fermi logo points. */	  
    bin = logo;	    
    for (i = 0; i < (NUM_X_BINS * 2); i++) {
    	for (j = 0; j < (NUM_Y_BINS * 2); j++) {
    	    x = -1.0 + (float)i / NUM_X_BINS;
    	    y = -1.0 + (float)j / NUM_Y_BINS; 
    	    if (fermiLogo(x,y)) 
    	        *bin++ = 1.0; 
    	    else
    	    	*bin++ = 0.0;
    	 }
    }

    /* Average the four points and place the average into bins. */
    logoRow1 = logo;
    logoRow2 = logo + NUM_Y_BINS * 2;
    bin = bins;
    for (i = 0; i < NUM_X_BINS; i++) {
    	for (j = 0; j < NUM_Y_BINS ; j++) { 
    	    total = 0.0;
    	    total += *logoRow1++;
    	    total += *logoRow1++;
    	    total += *logoRow2++;
    	    total += *logoRow2++;
    	    *bin++ = total / 4.;
    	}
    	logoRow1 += NUM_Y_BINS * 2;
    	logoRow2 += NUM_Y_BINS * 2;
    }
    		
    XtFree((char *)logo); 
    return (bins);
}

static void doubleBufferCB(Widget w, XtPointer clientD, XtPointer callD)
{  
    /* Turn on and off graphics buffering */
    
    XtVaSetValues(Cell, XmNdoubleBuffer, XmToggleButtonGetState(w), NULL);
}

static void helpCallback(Widget w, XtPointer clientData, XtPointer callData)
{ 
    /* Display a help message about using the interactive controls */
    
    displayHelpBitmap(w, "Widget Controls", Help2D_bits, Help2D_width,
    	    Help2D_height);
}

static void setRangeCallback(Widget w, XtPointer clientData, XtPointer callData)
{  
    /* Set the range of the data visible in the widget */

    double minXLim, maxXLim, minYLim, maxYLim;
    
    CellGetVisibleRange(Cell, &minXLim, &minYLim, &maxXLim, &maxYLim); 
    setRange(Cell, &minXLim, &minYLim, NULL, &maxXLim, &maxYLim, NULL);
    CellSetVisibleRange(Cell, minXLim, minYLim, maxXLim, maxYLim);          
}

static void zoomCallback(Widget w, XtPointer data, XtPointer callData)
{
   /* Zoom In, Zoom Out, or Reset View, depending on clientData. */
        
    switch ((int)data) {
        case 1 : CellZoomIn(Cell); 
                 break;
        case 2 : CellZoomOut(Cell);
                 break;
        case 3 : CellResetZoom(Cell);
                 break;
    } 
}

static void printCallback(Widget w, XtPointer clientD, XtPointer callD)
{
    /* Generate a postscript file and invoke the print dialog
       to prompt for a print queue and submit the print job */
       
    static char fileName[] = "cellDemo.ps";
	
    CellPrintContents(Cell, fileName); 
#ifdef VMS
    PrintFile(w, fileName, fileName, True);
#else
    PrintFile(w, fileName, fileName);
    remove(fileName);
#endif
}

static void exitCallback(Widget w, XtPointer clientData, XtPointer callData)
{ 
    exit(0);
}

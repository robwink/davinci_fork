/***********************************************************************
*                                                                      *
* scatExample.c - Scatter Plot Widget Example Program                  *
*                                                                      *
* This program demonstrates the capabilities of the Scat widget        *
*                                                                      *
* June 29, 1994 by George Dimas and Mark Edel                          *
*                                                                      *
***********************************************************************/
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include "../plot_widgets/Scat.h"
#include "../plot_widgets/2DHelp.xbm"
#include "../util/printUtils.h"
#include "demoUtils.h"

#define NUM_POINTS 10000

static Widget Scat;

static void setRangeCallback(Widget w, XtPointer clientD, XtPointer callD);
static void zoomCallback(Widget w, XtPointer clientD, XtPointer callD);
static void helpCallback(Widget w, XtPointer clientD, XtPointer callD);
static void xScalingCallback(Widget w, XtPointer clientD, XtPointer callD);
static void yScalingCallback(Widget w, XtPointer clientD, XtPointer callD);
static void printCallback(Widget w, XtPointer clientD, XtPointer callD);
static void darkerCallback(Widget w, XtPointer clientD, XtPointer callD);
static void doubleBufferCB(Widget w, XtPointer clientD, XtPointer callD);
static void exitCallback(Widget w, XtPointer clientD, XtPointer callD);


main(int argc, char *argv[])
{
    XtAppContext appContext;
    Widget toplevel, form, rowColumn; 
    XmString s1, s2; 
    int numPoints = 0;
    float x, y;
    ScatPoint *point, *points;
    
    
    /* Initialize the Intrinsics. */   
    toplevel = XtAppInitialize(&appContext, "ScatTest", NULL,
            0, &argc, argv, NULL, NULL, 0);
    
    /* Create a form widget as the container for rowcolumn, 
       Scat, and checkbox. */
    form = XtCreateManagedWidget("form", xmFormWidgetClass, toplevel, NULL, 0);

    /* Create a scat widget to display the points. */
    Scat = XtVaCreateManagedWidget("Scat", scatWidgetClass, form,
        XmNxAxisLabel,       s1=XmStringCreateSimple("X Axis Label"),
        XmNyAxisLabel,       s2=XmStringCreateSimple("Y Axis Label"),
        XmNdarkerPoints,     False,
        XmNtopAttachment,    XmATTACH_FORM,
    	XmNleftAttachment,   XmATTACH_POSITION,
    	XmNleftPosition,     21,
    	XmNrightAttachment,  XmATTACH_FORM,
    	XmNbottomAttachment, XmATTACH_FORM, NULL); 
    XmStringFree(s1);    	
    XmStringFree(s2);
    
    /* create the points to put in the plot and display them. */
    points = (ScatPoint *)XtMalloc(sizeof(ScatPoint) * NUM_POINTS);
    point = points;
    srand(1);
    while (numPoints < NUM_POINTS) {
        x = rand() / (RAND_MAX / 2.) - 1.;
        y = rand() / (RAND_MAX / 2.) - 1.;
        if (fermiLogo(x, y)) {
            point->x = x + 1.;
            point->y = y + 1.;
            point->pixel = BlackPixelOfScreen(XtScreen(Scat));
            point++;
            numPoints++;
        }
    }
    ScatSetContents(Scat, points, NUM_POINTS, SCAT_RESCALE);  
    
    /* Create a rowColumn widget to hold toggle buttons & push buttons. */
    rowColumn = XtVaCreateManagedWidget("rowCol", xmRowColumnWidgetClass, form,
    	    XmNtopAttachment,    XmATTACH_FORM,
    	    XmNleftAttachment,   XmATTACH_FORM,
    	    XmNrightAttachment,  XmATTACH_POSITION,
    	    XmNrightPosition,    20,
    	    NULL);
    
    /* Create square shaped toggle butons. */
    createToggleButton(rowColumn, "logX", "Log X",
    	    xScalingCallback, NULL);
    createToggleButton(rowColumn, "logY", "Log Y",
    	    yScalingCallback, NULL);
    createToggleButton(rowColumn, "darkerPoints", "Darker Points",
    	    darkerCallback, NULL);
    createToggleButton(rowColumn, "doubleBuffer",
    	    "Double Buffer", doubleBufferCB, NULL);
    
    /* Create pushButtons. */
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
    
    /* Display all widgets and give up control to XtMainLoop to
       continuously process events. */
    XtRealizeWidget(toplevel);
    XtAppMainLoop(appContext);
}


static void setRangeCallback(Widget w, XtPointer clientData, XtPointer callData)
{  
    /* Set the range of the data visible in the widget */

    double minXLim, maxXLim, minYLim, maxYLim;
    
    ScatGetVisibleRange(Scat, &minXLim, &minYLim, &maxXLim, &maxYLim);
    setRange(Scat, &minXLim, &minYLim, NULL, &maxXLim, &maxYLim, NULL);
    ScatSetVisibleRange(Scat, minXLim, minYLim, maxXLim, maxYLim);          
}

static void zoomCallback(Widget w, XtPointer data, XtPointer callData)
{
   /* Zoom In, Zoom Out, or Reset View, depending on clientData. */
    
    switch ((int)data) {
        case 1 : ScatZoomIn(Scat); 
                 break;
        case 2 : ScatZoomOut(Scat);
                 break;
        case 3 : ScatResetZoom(Scat);
                 break;
    } 
}

static void helpCallback(Widget w, XtPointer clientData, XtPointer callData)
{ 
    /* Display a help message about using the interactive controls */
    
    displayHelpBitmap(w, "Widget Controls", Help2D_bits, Help2D_width,
    	    Help2D_height);
}

static void xScalingCallback(Widget w, XtPointer clientData, XtPointer callData)
{ 
    /* This routine is activated every time toggle button Log X has
       been pressed. It changes coordinate axis X from linear to Log
       and vice versa. */
    
    XtVaSetValues(Scat, XmNxLogScaling, XmToggleButtonGetState(w), NULL);
}

static void yScalingCallback(Widget w, XtPointer clientData, XtPointer callData)
{
    /* This routine is activated every time toggle button Log Y has
       been pressed. It changes coordinate axis Y from linear to Log
       and vice versa. */
    
    XtVaSetValues(Scat, XmNyLogScaling, XmToggleButtonGetState(w), NULL);
}

static void printCallback(Widget w, XtPointer clientD, XtPointer callD)
{
    /* This routine prints the Scatter plot for the user.
       ScatPrintContents writes a post script file that is passed to
       the utility routine PrintFile, a system independent routine
       queues the user for que information and queues the file 
       for printing. */
       
    static char fileName[] = "scatDemo.ps";
	
    ScatPrintContents(Scat, fileName); 
#ifdef VMS
    PrintFile(w, fileName, fileName, True);
#else
    PrintFile(w, fileName, fileName);
    remove(fileName);
#endif
}

static void darkerCallback(Widget w, XtPointer clientD, XtPointer callD)
{  
    /* Turn on or off point darkening */
    
    XtVaSetValues(Scat, XmNdarkerPoints, XmToggleButtonGetState(w), NULL);
}

static void doubleBufferCB(Widget w, XtPointer clientD, XtPointer callD)
{  
    /* Turn on and off graphics buffering */
    
    XtVaSetValues(Scat, XmNdoubleBuffer, XmToggleButtonGetState(w), NULL);
}
  
static void exitCallback(Widget w, XtPointer clientData, XtPointer callData)
{ 
    exit(0);
}

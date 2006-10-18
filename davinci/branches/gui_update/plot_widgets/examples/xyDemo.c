/********************************************************************
*                                                                   *
* testXY.c - XY Plot Widget Example Program                         *
*                                                                   *
* This program is an example program that demonstrates the XY       *
* widget capabilities                                               *
*                                                                   *
*                                                                   *
* July 15, 1994 by George Dimas and Mark Edel                       *
*                                                                   *
* Updated July 10, 1997 by Joy Kyriakopulos                         *
*                                                                   *
********************************************************************/
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include "../plot_widgets/XY.h"
#include "../plot_widgets/XYDialogs.h"
#include "../plot_widgets/2DHelp.xbm"
#include "../util/printUtils.h"
#include "demoUtils.h"

#define NUM_POINTS  20
#define NUM_CURVES  3

Widget XYPlot;

static void logXCB(Widget w, XtPointer clientData, XtPointer callData);
static void logYCB(Widget w, XtPointer clientData, XtPointer callData);
static void doubleBufferCB(Widget w, XtPointer clientD, XtPointer callD);
static void setRangeCB(Widget w, XtPointer clientData, XtPointer callData);
static void setMarkLineStyleCB(Widget w, XtPointer clientD, XtPointer callData);
static void styleApplyCB(XYCurve *curves, int nCurves, void *arg);
static void styleOKCB(XYCurve *curves, int nCurves, void *arg);
static void showLegendCB(Widget w, XtPointer clientD, XtPointer callD);
static void stringsCB(Widget w, XtPointer clientD, XtPointer callD);
static void errorBarsCB(Widget w, XtPointer clientD, XtPointer callD);
static void createCurves(Boolean init, Boolean showErrorBars);
static void helpCallback(Widget w, XtPointer clientData, XtPointer callData);
static void zoomCB(Widget w, XtPointer data, XtPointer callData);
static void printCB(Widget w, XtPointer clientD, XtPointer callD);
static void exitCB(Widget w, XtPointer clientData, XtPointer callData);

static XYPoint Data[NUM_CURVES][NUM_POINTS];
static XYErrorBar HBars[NUM_CURVES][NUM_POINTS];
static XYErrorBar VBars[NUM_CURVES][NUM_POINTS];
static XYCurve Curves[NUM_CURVES] = {
    	{NULL, XY_SQUARE_MARK, XY_SMALL, XY_2_DOT_DASH,
    	    	0, 1, NUM_POINTS, Data[0], NULL, NULL},
    	{NULL, XY_SQUARE_MARK, XY_LARGE,  XY_FINE_DASH,
    	    	2, 3, NUM_POINTS, Data[1], NULL, NULL},
    	{NULL, XY_X_MARK, XY_SMALL,  XY_MED_FINE_DASH,
    	    	4, 5, NUM_POINTS, Data[2], NULL, NULL}};

main(int argc, char *argv[])
{
    XtAppContext appContext;
    Widget toplevel, form, checkBox, rowColumn, legendToggle, errorToggle;
    XmString s1, s2;
    
    /* Initialize X and Xt */
    toplevel = XtAppInitialize(&appContext, "XYTest", NULL,
	    0, &argc, argv, NULL, NULL, 0);
    
    /* Create a form Widget as the container for a row column,
       check box and XY widget. */
    form = XtCreateManagedWidget("form",
    	xmFormWidgetClass, toplevel, NULL, 0);
    
    /* Create an XY widget. */
    XYPlot = XtVaCreateManagedWidget("XYPlot",
    	    xyWidgetClass, form, 
    	    XmNxAxisLabel, s1=XmStringCreateSimple("X axis label"),
    	    XmNyAxisLabel, s2=XmStringCreateSimple("Y axis label"),
    	    /* XmNmarginPercent, 0, */
    	    XmNtopAttachment, XmATTACH_FORM,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 21,
    	    XmNrightAttachment, XmATTACH_FORM,
    	    XmNbottomAttachment, XmATTACH_FORM,
    	    NULL);
    XmStringFree(s1);
    XmStringFree(s2);

    /* Draw the example curves */
    createCurves(TRUE, TRUE);
    	
    /* Create a row column to hold toggle buttons. */
    checkBox = XtVaCreateManagedWidget("toggleButtons",
    	    xmRowColumnWidgetClass, form,
    	    XmNtopAttachment, XmATTACH_FORM,
    	    XmNleftAttachment, XmATTACH_FORM,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 20,
    	    NULL);
    
    /* Create the toggle buttons */
    createToggleButton(checkBox, "logX", "Log X",logXCB, NULL);
    createToggleButton(checkBox, "logY", "Log Y", logYCB, NULL);
    legendToggle = createToggleButton(checkBox, "showLegend",
    	    "Show Legend", showLegendCB, NULL);
    XtVaSetValues(legendToggle, XmNset, True, 0);
    createToggleButton(checkBox, "showStrings",
    	    "Show Strings", stringsCB, NULL);
    errorToggle = createToggleButton(checkBox, "showErrorBars",
    	    "Show Error Bars", errorBarsCB, NULL);
    XtVaSetValues(errorToggle, XmNset, True, 0);
    createToggleButton(checkBox, "doubleBuffer",
    	    "Double Buffer", doubleBufferCB, NULL);
    
    /* Create a row column to hold the push buttons. */
    rowColumn = XtVaCreateManagedWidget("rowColumn",
    	    xmRowColumnWidgetClass, form,
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNtopWidget, checkBox,
    	    XmNtopOffset, 10,
    	    XmNleftAttachment, XmATTACH_FORM,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 20,
    	    XmNbottomAttachment, XmATTACH_FORM,
    	    NULL);
    
    /* Create the push buttons. */
    createButton(rowColumn, "controlsHelp", "Controls Help...",
	    helpCallback, NULL);
    createButton(rowColumn, "setMarkLineStyle", "Set Mark & Line Style...",
	    setMarkLineStyleCB, NULL);
    createButton(rowColumn, "setRange", "Set Visible Range...",
	    setRangeCB, NULL);
    createButton(rowColumn, "zoomIn", "Zoom In", zoomCB, (XtPointer)1);
    createButton(rowColumn, "zoomOut", "Zoom Out", zoomCB, (XtPointer)2);
    createButton(rowColumn, "resetZoom", "Reset Zoom",
    	    zoomCB, (XtPointer)3);
    createButton(rowColumn, "printPlot", "Print Plot...",
	    printCB, NULL);
    createButton(rowColumn, "exitProgram", "Exit Program",
	    exitCB, NULL);
    
    /* Realize widgets and call XtMainLoop to continuously process events. */
    XtRealizeWidget(toplevel);
    XtAppMainLoop(appContext);
}

static void logXCB(Widget w, XtPointer clientData, XtPointer callData)
{ 
    /* This routine is activated every time toggle button Log X has
       been pressed. It changes the X axis from linear to log scaling
       and vice versa. */
    
    XtVaSetValues(XYPlot, XmNxLogScaling, XmToggleButtonGetState(w), NULL);
}

static void logYCB(Widget w, XtPointer clientData, XtPointer callData)
{
    
    /* This routine is activated every time toggle button Log Y has
       been pressed. It changes the Y axis from linear to log scaling
       and vice versa. */
    
    XtVaSetValues(XYPlot, XmNyLogScaling, XmToggleButtonGetState(w), NULL);
}

static void doubleBufferCB(Widget w, XtPointer clientD, XtPointer callD)
{  
    /* This routine is activated every time toggle button Double Buffer 
       has been pressed. It turns on and off graphics buffering */
    
    XtVaSetValues(XYPlot, XmNdoubleBuffer, XmToggleButtonGetState(w),
    	    NULL);
}

static void setRangeCB(Widget w, XtPointer clientD, XtPointer callD)
{  
    /* This routine is activated every time toggle button Double Buffer 
       has been pressed. It turns on and off graphics buffering */

    double minXLim, maxXLim, minYLim, maxYLim;
    
    XYGetVisibleRange(XYPlot, &minXLim, &minYLim, &maxXLim, &maxYLim); 
    setRange(XYPlot, &minXLim, &minYLim, NULL, &maxXLim, &maxYLim, NULL);
    XYSetVisibleRange(XYPlot, minXLim, minYLim, maxXLim, maxYLim);          
}

static void setMarkLineStyleCB(Widget w, XtPointer clientD, XtPointer callData)
{
    /* This function calls CurvesSetMarkLineStyle, which allows the
       user to mark the critical points (the point where the curve changes 
       direction) with different styles of markers and also change the
       style of the line in each curve. */
       
    XYCreateStylesDialog(XYPlot, Curves, NUM_CURVES, styleOKCB, (void *)1,
    	    styleApplyCB, (void *)2, NULL, NULL);
    /* An easier way to do the above, as long as you don't care about an
       Apply button is:  XYEditStyles(XYPlot, Curves, NUM_CURVES); */
    XYUpdateCurveStyles(XYPlot, Curves, True);
}

static void styleApplyCB(XYCurve *curves, int nCurves, void *arg)
{
    XYUpdateCurveStyles(XYPlot, curves, True);
}

static void styleOKCB(XYCurve *curves, int nCurves, void *arg)
{
    XYUpdateCurveStyles(XYPlot, curves, True);
}

static void showLegendCB(Widget w, XtPointer clientD, XtPointer callD)
{
    /* This shows or hides the legend at the bottom of the plot */
        
    XtVaSetValues(XYPlot, XmNshowLegend, XmToggleButtonGetState(w), NULL);
}

static void stringsCB(Widget w, XtPointer clientD, XtPointer callD)
{
    /* This function switches the widget back an forth between showing
       strings on the plot and not showing them */
       
    char str[25];
    int i;
    XYString *string, strings[NUM_POINTS];
    
    if (!XmToggleButtonGetState(w)) {
        XYSetStrings(XYPlot, NULL, 0);
        return;
    }
    
    for (i=0, string=strings; i<NUM_POINTS; i++, string++) {
    	string->x = Curves[1].points[i].x;
    	string->y = Curves[1].points[i].y-2;
    	string->font = NULL;
    	string->color = BlackPixelOfScreen(XtScreen(XYPlot));
    	string->alignment = XY_CENTER;
    	sprintf(str, "%d", i+1);
    	string->string = XmStringCreateSimple(str);
    }
    XYSetStrings(XYPlot, strings, NUM_POINTS);
    for (i=0, string=strings; i<NUM_POINTS; i++, string++)
        XmStringFree(string->string);
}

static void errorBarsCB(Widget w, XtPointer clientD, XtPointer callD)
{
    /* This function shows or hides error bars on the first curve. */
    int i, showErrorBars = XmToggleButtonGetState(w);
    
    for (i=0; i<NUM_CURVES/2; i++) {
    	Curves[i].vertBars = showErrorBars ? VBars[i] : NULL;
    	Curves[i].horizBars = showErrorBars ? HBars[i] : NULL;
    }
    XYUpdateCurveData(XYPlot, Curves, XY_RESCALE_AT_MAX, True);
}

static void createCurves(Boolean init, Boolean showErrorBars)
{
    Pixel black = BlackPixelOfScreen(XtScreen(XYPlot));
    int i, j;
    float yVal;
    char name[100];

    for (i=0; i<NUM_CURVES; i++) {
    	Curves[i].markerPixel = black;
    	Curves[i].linePixel = black;
    	sprintf(name, "Curve%d", i);
    	Curves[i].name = XmStringCreateSimple(name);
    }
    
    srand(1);
    for (i=0; i<NUM_CURVES; i++) {
    	for (j=0; j<NUM_POINTS; j++) {
    	    Curves[i].points[j].x = j + 2;
    	    yVal = rand() / (RAND_MAX / 100) + 6;
    	    Curves[i].points[j].y = yVal;
    	    HBars[i][j].min = j + 1;
    	    HBars[i][j].max = j + 3;
    	    VBars[i][j].min = yVal - 5;
    	    VBars[i][j].max = yVal + 5;
    	}
    }
    
    for (i=0; i<NUM_CURVES/2; i++) {
    	Curves[i].vertBars = showErrorBars ? VBars[i] : NULL;
    	Curves[i].horizBars = showErrorBars ? HBars[i] : NULL;
    	Curves[i].lineStyle = XY_NO_LINE;
    }
    
    XYSetCurves(XYPlot, Curves, NUM_CURVES,
    	    init ? XY_RESCALE : XY_RESCALE_AT_MAX, True);
}

static void helpCallback(Widget w, XtPointer clientData, XtPointer callData)
{ 
    /* Display a help message about using the interactive controls */
    
    displayHelpBitmap(w, "Widget Controls", Help2D_bits, Help2D_width,
    	    Help2D_height);
}

static void zoomCB(Widget w, XtPointer data, XtPointer callData)
{
    /* This one callback is used for all 3 zoom pushbuttons:
       Zoom In, Zoom Out, and Reset Zoom. */
       
    switch ((int)data) {
        case 1 : XYZoomIn(XYPlot); 
                 break;
        case 2 : XYZoomOut(XYPlot);
                 break;
        case 3 : XYResetZoom(XYPlot); 
                 break;         
    } 
}

static void printCB(Widget w, XtPointer clientD, XtPointer callD)
{
    /* This routine prints a PostScript version of the plot.
       XYPrintContents writes a post script file that is passed to
       the utility routine PrintFile, which presents a dialog and
       queues the file for printing. */
       
    static char fileName[] = "testXY.ps";
	
    XYPrintContents(XYPlot, fileName); 
#ifdef VMS
    PrintFile(w, fileName, fileName, True);
#else
    PrintFile(w, fileName, fileName);
    remove(fileName);
#endif
}    
  
static void exitCB(Widget w, XtPointer clientData, XtPointer callData)
{ 
    exit(0);
}

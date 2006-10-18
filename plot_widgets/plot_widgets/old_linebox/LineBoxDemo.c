/***********************************************************************
*                                                                      *
* histo1DExample.c - Histogram Widget Example Program                  *
*                                                                      *
* This program is an example that demonstrates the histogram (LineBox)     *
* widget capabilities.                                                 *
*                                                                      *
* July 19, 1994, by George Dimas                                       *
*                                                                      *
***********************************************************************/
#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/Separator.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include "../plot_widgets/LineBox.h"
#include "../plot_widgets/2DHelp.xbm"
#include "../util/printUtils.h"
#include "../util/DialogF.h"
#include "demoUtils.h"

#define NUM_BINS 50
#define X_MIN 0.0
#define X_MAX 100.0

Widget HistWidget;
Widget ShowErrorBarsWidget;

static void createHistogramData(Boolean showErrorBars);
static void createAdaptiveData(void);
static void showErrorBarsCB(Widget w, XtPointer clientData, XtPointer callData);
static void logScalingCB(Widget w, XtPointer clientData, XtPointer callData);
static void adaptiveCB(Widget w, XtPointer clientD, XtPointer callD);
static void doubleBufferCB(Widget w, XtPointer clientD, XtPointer callD);
static void helpCallback(Widget w, XtPointer clientData, XtPointer callData);
static void setRangeCB(Widget w, XtPointer clientData, XtPointer callData);
static void barSeparationCB(Widget w, XtPointer clientData, XtPointer callData);
static void zoomCB(Widget w, XtPointer data, XtPointer callData);
static void printCB(Widget w, XtPointer clientD, XtPointer callD);
static void exitCB(Widget w, XtPointer clientData, XtPointer callData);

main(int argc, char *argv[])
{
    XtAppContext appContext;
    Widget toplevel, form, checkBox, rowColumn, separator;
    XmString s1, s2;
    
    /* Initialize X and Xt */
    toplevel = XtAppInitialize(&appContext, "CurvesTest", NULL,
    	0, &argc, argv, NULL, NULL, 0);
    
    /* Create a form Widget as the container for a row column,
       check box and histo1D widget. */
    form = XtCreateManagedWidget("form", xmFormWidgetClass, toplevel, NULL, 0);
    
    /* Create an LineBox widget. */
    HistWidget = XtVaCreateManagedWidget("HistWidget",
    	    lineBoxWidgetClass, form, 
    	    XmNxAxisLabel, s1=XmStringCreateSimple("X axis label"),
    	    XmNyAxisLabel, s2=XmStringCreateSimple("Y axis label"),
    	    XmNtopAttachment, XmATTACH_FORM,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 21,
    	    XmNrightAttachment, XmATTACH_FORM,
    	    XmNbottomAttachment, XmATTACH_FORM,
    	    NULL);
    XmStringFree(s1);
    XmStringFree(s2);
    
    /* Create histogram without error bars */
    createHistogramData(FALSE);  
    	
    /* Create a row column to hold toggle buttons. */
    checkBox = XtVaCreateManagedWidget("toggleButtons",
    	    xmRowColumnWidgetClass, form,
    	    XmNtopAttachment, XmATTACH_FORM,
    	    XmNleftAttachment, XmATTACH_FORM,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 20,
    	    NULL);
    
    /* Create the toggle buttons. */ 
    createToggleButton(checkBox, "logScaling", "Log Scaling",
	    logScalingCB, NULL);
    createToggleButton(checkBox, "adaptiveHistogram", "Adaptive Histogram",
	    adaptiveCB, NULL);
    ShowErrorBarsWidget = createToggleButton(checkBox, "showErrorBars",
    	    "Show Error Bars", showErrorBarsCB, NULL);
    createToggleButton(checkBox, "doubleBuffer",
    	    "Double Buffer", doubleBufferCB, NULL);
    
    /* Create a separator between the check box and row column. */
    separator = XtVaCreateManagedWidget("separator",
            xmSeparatorWidgetClass, form,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, checkBox,
    	    XmNleftAttachment, XmATTACH_FORM,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 20,
            NULL);
    
    /* Create a row column to hold the push buttons. */
    rowColumn = XtVaCreateManagedWidget("rowColumn",
    	    xmRowColumnWidgetClass, form,
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNtopWidget, separator,
    	    XmNtopOffset, 10,
    	    XmNleftAttachment, XmATTACH_FORM,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 20,
    	    XmNbottomAttachment, XmATTACH_FORM,
    	    NULL);
    
    /* Create the push buttons. */ 
    createButton(rowColumn, "controlsHelp", "Controls Help...",
	    helpCallback, NULL);
    createButton(rowColumn, "setRange", "Set Visible Range...",
	    setRangeCB, NULL);
    createButton(rowColumn, "barSeparation", "Bar Separation...",
	    barSeparationCB, NULL);
    createButton(rowColumn, "zoomIn", "Zoom In", zoomCB, (XtPointer)1);
    createButton(rowColumn, "zoomOut", "Zoom Out", zoomCB, (XtPointer)2);
    createButton(rowColumn, "resetZoom", "Reset Zoom",
    	    zoomCB, (XtPointer)3);
    createButton(rowColumn, "printHisto", "Print Histogram...",
	    printCB, NULL);
    createButton(rowColumn, "exitProgram", "Exit Program",
	    exitCB, NULL);
    
    /* Realize widgets and call XtMainLoop to continuously process events. */
    XtRealizeWidget(toplevel);
    XtAppMainLoop(appContext);
}

static void createHistogramData(Boolean showErrorBars)
{
    /* This function creates histogram data for a plain (non-adaptive)
       histogram.  The variable bins holds the points for
       the histogram, the variables uppErr, lowErr hold the
       points for the upper and lower error bars for the 
       histogram. When the showErrorBars is FALSE, uppErr and
       lowErr are NULL, to indicate no error bars.  */
       
    float bins[NUM_BINS], *bin;
    float *upErr, upErrs[NUM_BINS], *lowErr, lowErrs[NUM_BINS];
    int i, x;
    
    /* Create bin data */
    for (i=0, bin=bins; i < NUM_BINS; i++, bin++)
        *bin = (float)(i > NUM_BINS / 2 ? NUM_BINS - i : i);

    /* Create error bar data */
    if (showErrorBars) {
        for (i=0, bin=bins, upErr=upErrs, lowErr=lowErrs; i < NUM_BINS; i++,
        	bin++, upErr++, lowErr++) {
            *upErr = *lowErr = *bin * .1;
        }
    }
    
    /* Display the data in the histogram widget */
    LineBoxSetContents(HistWidget, X_MIN, X_MAX, NUM_BINS, bins, 
	    showErrorBars?upErrs:NULL, showErrorBars?lowErrs:NULL, LineBox_RESCALE);
}

static void createAdaptiveData(void)
{ 
    /* This function creates histogram data for an adaptive
       histogram.  The variable bins holds 
       the points for the adaptive histogram, the variable
       edges hold the points for the edges. At the end of the
       function, LineBoxSetContentsAdaptive is called to display
       the data. */
       
    float bins[NUM_BINS], *bin, edges[NUM_BINS+1], *edge, lastEdge;
    int i;
    
    lastEdge = 0.;
    for (i=0, bin=bins, edge=edges; i<NUM_BINS; i++, bin++, edge++) {
        *bin = (float)i;
        if (i > NUM_BINS/2) 
            *bin = NUM_BINS - *bin;
        *edge = lastEdge + (*bin+1.) * .1;
        lastEdge = *edge;
    }
    *edge = lastEdge + .05;   
    
    LineBoxSetContentsAdaptive(HistWidget, NUM_BINS, bins, edges, LineBox_RESCALE);
}

static void showErrorBarsCB(Widget w, XtPointer clientData, XtPointer callData)
{ 
    /* This function is activated every time toggle button
       Show Error Bars is pressed. It shows or hides error bars
       depending on the state of the toggle button */
    
    createHistogramData(XmToggleButtonGetState(w));
}

static void logScalingCB(Widget w, XtPointer clientData, XtPointer callData)
{ 
    /* This routine is activated every time toggle button Log Scaling 
       has been pressed. It changes coordinate from linear to Log
       and vice versa. */
    
    XtVaSetValues(HistWidget, XmNlogScaling, XmToggleButtonGetState(w), NULL);
}

static void adaptiveCB(Widget w, XtPointer clientD, XtPointer callD)
{
    /* adaptiveCB called when the Adaptive Histogram toggle button
       is pressed. If the toggle button is not pressed, it is a histogram,
       if it is pressed, it is an adaptive histogram. If we are switching
       to an adaptive histogram, LineBoxSetContents must be called with NULL
       data before createAdaptiveData, to get rid of error bars. 
       If we are creating a histogram, than LineBoxSetContentsAdaptive
       must be called with NULL data to get rid of the edge data
       (switching back and forth from histogram to adaptive mode is
       not recommended, it's just done here to demonstrate both modes). */
       
    if (XmToggleButtonGetState(w)) {
	XmToggleButtonSetState(ShowErrorBarsWidget, FALSE, TRUE); 
	XtVaSetValues(ShowErrorBarsWidget, XmNsensitive, FALSE, NULL);
	LineBoxSetContents(HistWidget, X_MIN, X_MAX, NUM_BINS,  
		NULL, NULL, NULL, LineBox_RESCALE);
	createAdaptiveData();
    } else {            
	XtVaSetValues(ShowErrorBarsWidget, XmNsensitive, TRUE, NULL);
	LineBoxSetContentsAdaptive(HistWidget, NUM_BINS, NULL, 
		NULL, LineBox_RESCALE);     
	createHistogramData(FALSE); 
    } 
}

static void doubleBufferCB(Widget w, XtPointer clientD, XtPointer callD)
{  
    /* Turn on and off graphics buffering */
    
    XtVaSetValues(HistWidget, XmNdoubleBuffer, XmToggleButtonGetState(w), NULL);
}

static void helpCallback(Widget w, XtPointer clientData, XtPointer callData)
{ 
    /* Display a help message about using the interactive controls */
    
    displayHelpBitmap(w, "Widget Controls", Help2D_bits, Help2D_width,
    	    Help2D_height);
}

static void setRangeCB(Widget w, XtPointer clientData, XtPointer callData)
{
    /* Set the range of the data visible in the widget */

    double minXLim, maxXLim, minYLim, maxYLim;
    
    LineBoxGetVisibleRange(HistWidget, &minXLim, &minYLim, &maxXLim, &maxYLim); 
    setRange(HistWidget, &minXLim, &minYLim, NULL, &maxXLim, &maxYLim, NULL);
    LineBoxSetVisibleRange(HistWidget, minXLim, minYLim, maxXLim, maxYLim);          
}

static void barSeparationCB(Widget w, XtPointer clientData, XtPointer callData)
{        
    /* Create a prompt dialog where the user can enter the bar
       separation percentage. */
       
    char str[DF_MAX_PROMPT_LENGTH], *ptr;
    int response;
    int factor;
    
    response = DialogF(DF_PROMPT, w, 2, "Bar Separation <Percent>",
    	    str, "OK", "Cancel");
    if (response == 2)
    	return;
    factor = strtol(str, &ptr, 10);
    if (ptr != str)
        XtVaSetValues(HistWidget,XmNbarSeparation, factor, NULL);
}

static void zoomCB(Widget w, XtPointer data, XtPointer callData)
{
    /* Zoom In, Zoom Out, or Reset Zoom. */
       
    switch ((int)data) {
        case 1 : LineBoxZoomIn(HistWidget); 
                 break;
        case 2 : LineBoxZoomOut(HistWidget);
                 break;
        case 3 : LineBoxResetZoom(HistWidget); 
                 break;         
    } 
}

static void printCB(Widget w, XtPointer clientD, XtPointer callD)
{
    /* Generate a postscript file and invoke the print dialog
       to prompt for a print queue and submit the print job */
       
    static char fileName[] = "LineBox.ps";
	
    LineBoxPrintContents(HistWidget, fileName); 
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


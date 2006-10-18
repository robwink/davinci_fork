/**********************************************************************
*                                                                     *
* histo2DExample.c - 2D Histogram Widget Example Program              *
*                                                                     *
* This program demonstrates the capabilities of the 2D                *
* Histogram widget                                                    *
*                                                                     *
* August 1, 1994, by George Dimas and Mark Edel                       *
*                                                                     *
**********************************************************************/
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>
#include <Xm/LabelG.h>
#include <Xm/PushB.h>
#include <Xm/Separator.h>
#include "../plot_widgets/2DHist.h"
#include "../plot_widgets/2DHistHelp.xbm"
#include "../util/printUtils.h"
#include "../util/DialogF.h"
#include "../plot_widgets/adaptHist.h"
#include "demoUtils.h"

#define F_RANDOM() (((double) rand() / ((RAND_MAX + 1.0) / 2.0)) - 1.0)
#define NUM_X_BINS 20
#define NUM_Y_BINS 20
#define NUM_RANDOM_PTS 10000

static Widget HistWidget, ZLogScaling; 
static Widget FiText, PsiText, SensitiveWidgets[6];
static int NLimit=10, Strategy=0;
    
static void zLogScalingCB(Widget w, XtPointer clientData, XtPointer callData);
static void doubleBufferCB(Widget w, XtPointer clientD, XtPointer callD);
static void helpCallback(Widget w, XtPointer clientData, XtPointer callData);
static void setRangeCallback(Widget w, XtPointer clientD, XtPointer callD);
static void zoomCallback(Widget w, XtPointer data, XtPointer callData);
static void zoomViewCallback(Widget w, XtPointer data, XtPointer callData);
static void setViewAnglesCB(Widget w, XtPointer clientD, XtPointer callD);
static void okSetViewCallback(Widget w, Widget x, XtPointer callD);
static void cancelCallback(Widget w, Widget x, XtPointer callData);
static void getViewAnglesCB(Widget w, XtPointer clientD, XtPointer callD);
static void printCallback(Widget w, XtPointer clientD, XtPointer callD);
static void exitCallback(Widget w, XtPointer clientData, XtPointer callData);
static void adaptiveCB(Widget w, XtPointer clientData, XtPointer callData);
static void histoErrorsCB(Widget w, XtPointer clientData, XtPointer callData);
static void strategyCB(Widget w, XtPointer clientData, XtPointer callData);
static void binCountCB(Widget w, XtPointer clientData, XtPointer callData);
static void createHistogram();
static void createHistogramWithErrors();
static void createAdaptiveHistogram();


main(int argc, char *argv[])
{
    XtAppContext appContext;
    Widget toplevel, form, checkBox, separator1, separator2, 
           radioBox, rowColumn, numBins;
    XmString s1;
    
    
    /* Initialize the Intrinsics. */
    toplevel = XtAppInitialize(&appContext, "CurvesTest", NULL,
    	    0, &argc, argv, NULL, NULL, 0);
    
    /* Create a form Widget as the container for everything in the window. */
    form = XtCreateManagedWidget("form", xmFormWidgetClass, toplevel, NULL, 0);
    	
    /* Create a checkbox widget to hold the toggle buttons. */
    checkBox = XmVaCreateSimpleCheckBox(form, "checkBox", 0,
            XmNtopAttachment, XmATTACH_FORM,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_POSITION,
            XmNrightPosition, 20,
            NULL);
    XtManageChild(checkBox);
    
    /* Create the toggle buttons. */ 
    ZLogScaling = createToggleButton(checkBox, "logZ",
    	    "Log Z", zLogScalingCB, NULL);
    SensitiveWidgets[0] = createToggleButton(checkBox, "showErrors",
    	    "Show Errors", histoErrorsCB, NULL);
    createToggleButton(checkBox, "doubleBuffer",
    	    "Double Buffer", doubleBufferCB, NULL);
    SensitiveWidgets[1] = createToggleButton(checkBox, "adaptiveHistogram",
    	    "Adaptive Histogram", adaptiveCB, NULL);
    
    /* Create a 2DHist widget. */
    HistWidget = XtVaCreateManagedWidget("HistWidget",
    	    hist2DWidgetClass, form,
    	    XmNzLogScaling, FALSE,
    	    XmNtopAttachment, XmATTACH_FORM,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 21,
    	    XmNrightAttachment, XmATTACH_FORM,
    	    XmNbottomAttachment, XmATTACH_FORM,
    	    NULL);
    	
    /* Create the histogram data for the widget. */ 
    createHistogram(); 

    /* Create a separator between the check box and radio box. */
    separator1 = XtVaCreateManagedWidget("separator1",
            xmSeparatorWidgetClass, form,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, checkBox,
            XmNtopOffset, 5,
    	    XmNleftAttachment, XmATTACH_FORM,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 20,
            NULL);

    /* Create a radiobox widget to hold the toggle buttons. */
    radioBox = XmVaCreateSimpleRadioBox (form, "radioBox", 0, NULL,
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNtopWidget, separator1,
    	    XmNleftAttachment, XmATTACH_FORM,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 20,
    	    NULL);
    XtManageChild(radioBox);
 
    /* Create the toggle buttons. */ 
    SensitiveWidgets[2] = createToggleButton(radioBox, "splitInHalf",
    	    "Split in Half", strategyCB, (XtPointer)0); 
    XtVaSetValues(SensitiveWidgets[2], XmNset, TRUE, XmNsensitive, FALSE, 0);
    SensitiveWidgets[3] = createToggleButton(radioBox, "centerOfGravity",
    	    "Center of Gravity", strategyCB, (XtPointer)1); 
    XtVaSetValues(SensitiveWidgets[3], XmNset, FALSE, XmNsensitive, FALSE, 0);    
    
    numBins = XtVaCreateManagedWidget("numBins", 
    	    xmPushButtonWidgetClass, form, 
    	    XmNlabelString, s1=XmStringCreateSimple("Set Counts Per Bin..."),
    	    XmNsensitive, FALSE, 
    	    XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, radioBox,
            XmNleftAttachment, XmATTACH_FORM,
            XmNleftOffset, 2,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 20,
            NULL);
    XtAddCallback(numBins, XmNactivateCallback, binCountCB, NULL);
    XmStringFree(s1);
    SensitiveWidgets[4] = numBins;
    
    /* Create a separator between numBins and row column. */
    separator2 = XtVaCreateManagedWidget("separator2",
            xmSeparatorWidgetClass, form,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, numBins,
            XmNtopOffset, 5,
    	    XmNleftAttachment, XmATTACH_FORM,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 20,
            NULL);
   
    /* Create a row column to hold the push buttons. */
    rowColumn = XtVaCreateManagedWidget("rowColumn",
    	    xmRowColumnWidgetClass, form,
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNtopWidget, separator2,
    	    XmNtopOffset, 10,
    	    XmNleftAttachment, XmATTACH_FORM,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 20,
    	    XmNbottomAttachment, XmATTACH_FORM,
    	    NULL);
    
    /* Call the function to create the push buttons. */ 
    createButton(rowColumn, "controlsHelp", "Controls Help...",
	    helpCallback, NULL);
    createButton(rowColumn, "setVisibleRange", "Set Visible Range...",
    	    setRangeCallback, NULL);
    createButton(rowColumn, "zoom", "Zoom...", zoomCallback, NULL);
    createButton(rowColumn, "zoomIn", "Zoom In",
    	    zoomViewCallback, (XtPointer)1);
    createButton(rowColumn, "zoomOut", "Zoom Out",
    	    zoomViewCallback, (XtPointer)2);
    createButton(rowColumn, "resetView", "Reset View",
    	    zoomViewCallback, (XtPointer)3);
    createButton(rowColumn, "setViewAngles", "Set View Angles",
    	    setViewAnglesCB,  NULL);
    createButton(rowColumn, "getViewAngles", "Get View Angles",
    	    getViewAnglesCB,  NULL);
    createButton(rowColumn, "print2DHistogram", "Print 2D Histogram...",
    	    printCallback,  NULL);
    createButton(rowColumn, "exitProgram", "Exit Program",
    	    exitCallback,  NULL);
    
    /* Display all widgets and call XtMainLoop to continuously process events */
    XtRealizeWidget(toplevel);
    XtAppMainLoop(appContext);
}

static void strategyCB(Widget w, XtPointer clientData, XtPointer callData)
{ 
    /* Change the adaptive histogram binning strategy */
       
    Strategy = (int)clientData;
    createAdaptiveHistogram();
}

static void binCountCB(Widget w, XtPointer clientData, XtPointer callData)
{ 
    /* Create a prompt dialog where the user can enter the NLimit, the
       maximum number of counts per bin.  The current setting of NLimit
       will also be displayed.  */ 

    char str[DF_MAX_PROMPT_LENGTH], *ptr;
    int response, value;
       
    response = DialogF(DF_PROMPT, w, 2, "Max Counts (current = %d)", str,
    	    "OK", "Cancel", NLimit);
    if (response == 2)
    	return;
    value = strtol(str, &ptr, 10);
    if (ptr != str)
        NLimit = value;
        
    createAdaptiveHistogram();  
}

static void adaptiveCB(Widget w, XtPointer clientData, XtPointer callData)
{ 
    /* This function is activated everytime Adaptive Histogram
       toggle button is pressed. If the XmToggleButtonGetState
       is TRUE, then an adaptive histogram will be displayed 
       otherwise, a regular histogram will be displayed. 
       Either createAdaptiveHistogram or createHistogram will be
       called depending of the state.  The global widget array
       SensitiveWidgets is used to set/unset sensitive widget
       resources which depend on whether this is an adaptive
       histogram or a normal histogram. */
     
    if( XmToggleButtonGetState(w) == TRUE ) {
        XtVaSetValues(SensitiveWidgets[0], XmNsensitive, FALSE, NULL);
        XtVaSetValues(SensitiveWidgets[2], XmNsensitive, TRUE, NULL);
        XtVaSetValues(SensitiveWidgets[3], XmNsensitive, TRUE, NULL);
        XtVaSetValues(SensitiveWidgets[4], XmNsensitive, TRUE, NULL);
        createAdaptiveHistogram();
    } else {
        XtVaSetValues(SensitiveWidgets[0], XmNsensitive, TRUE, NULL); 
        XtVaSetValues(SensitiveWidgets[2], XmNsensitive, FALSE, NULL);
        XtVaSetValues(SensitiveWidgets[3], XmNsensitive, FALSE, NULL);
        XtVaSetValues(SensitiveWidgets[4], XmNsensitive, FALSE, NULL);
        createHistogram(); 
    }   
}

static void histoErrorsCB(Widget w, XtPointer clientData, XtPointer callData)
{ 
    /* Show or hide error bars */
       
    if (XmToggleButtonGetState(w) == TRUE ) {
        XtVaSetValues(SensitiveWidgets[1], XmNsensitive, FALSE, NULL); 
    	createHistogramWithErrors();
    } else {
        XtVaSetValues(SensitiveWidgets[1], XmNsensitive, TRUE, NULL);
    	createHistogram();
    }   
}

static void createHistogramWithErrors()
{
    /* Create histogram data (with error bars) to display, and call
       hist2DUpdateHistogramData to display the histogram.  Note that
       since this uses hist2DUpdateHistogramData, createHistogram has
       to be called first, to set up the rest of the histogram */ 
       
    float topErrors[NUM_X_BINS * NUM_Y_BINS]; 
    float bottomErrors[NUM_X_BINS * NUM_Y_BINS];
    float data[NUM_X_BINS * NUM_Y_BINS];
    float *dataPoint, *topError, *bottomError, x, y;
    int i, j;

    dataPoint = data;
    topError = topErrors;
    bottomError = bottomErrors;	    
    for (i = 0; i < NUM_X_BINS; i++) {
    	for (j = 0; j < NUM_Y_BINS; j++) {
    	    x = -1.0 + i*2. / NUM_X_BINS;   
    	    y = -1.0 + j*2. / NUM_Y_BINS;
    	    if (fermiLogo(x,y)) {     /* if points within range */
    	        *dataPoint++ = 1.0;    
    	        *topError++ = 0.5;
    	        *bottomError++ = 0.5;
    	    } else {
    	    	*dataPoint++ = 0.0;
    		*topError++ = 0.0;
    		*bottomError++ = 0.0; 
    	    }
    	}
    }

    hist2DUpdateHistogramData(HistWidget, data, topErrors, bottomErrors,
    	    HIST2D_SCALING); 
}

static void createHistogram()
{
    /* Create data, labels, and limits for a the 2DHist widget */
       
    h2DHistSetup hist; 
    float data[NUM_X_BINS * NUM_Y_BINS];
    float *dataPoint, x, y;
    int i, j;

    /* Create example data */
    dataPoint = data;
    for (i = 0; i < NUM_X_BINS; i++) {
    	for (j = 0; j < NUM_Y_BINS; j++) {
    	    x = -1.0 + i*2. / NUM_X_BINS;   
    	    y = -1.0 + j*2. / NUM_Y_BINS;
    	    if (fermiLogo(x,y)) {     /* if points within range */
    	        *dataPoint++ = 1.0;    
    	    } else {
    	    	*dataPoint++ = 0.0;
    	    }
    	}
    }
    
    /* Pass the histogram data to the widget to the widget */
    hist.nXBins = NUM_X_BINS;
    hist.nYBins = NUM_Y_BINS;
    hist.xMin = -1.0;
    hist.xMax = 1.0;
    hist.yMin = -1.0;
    hist.yMax = 1.0;
    hist.xScaleType = H2D_LINEAR;
    hist.xScaleBase = 0.0;
    hist.yScaleType = H2D_LINEAR;
    hist.yScaleBase = 0.0;
    hist.xLabel = "X Axis Label";
    hist.yLabel = "Y Axis Label";
    hist.zLabel = "Z Axis Label";
    hist.bins = data; 
    hist2DSetHistogram(HistWidget, &hist);
    
    /* Since this function is called from various toggle buttons, the
       global widget ZLogScaling will be used to set the correct scale type */ 
    if (XmToggleButtonGetState(ZLogScaling) == TRUE) 
    	XtVaSetValues(HistWidget, XmNzLogScaling, TRUE , NULL); 
}

static void createAdaptiveHistogram()
{
    /* Create and display an adaptive histogram with the widget.  Note that
       it is expected that hist2DSetHistogram was called first to set up
       limits and labels for the widget */ 
       
    static t data[NUM_RANDOM_PTS];
    static int numPts = 0;
    aHistStruct *aHist;
    t *pt;
    int i;
    float x, y;
    
    /* Fill the data array with random x,y coordinate data, filtered
       through the fermilogo subroutine to make a nice picture, do this
       only once and save the array, so the user can continue to work
       with the same data */
    if (numPts == 0) {
	pt = data;
	for (i=0; i<NUM_RANDOM_PTS; i++) { 
            x = F_RANDOM();
    	    y = F_RANDOM(); 
    	    if (fermiLogo(x,y)) {
        	pt->c[0] =  x;
        	pt->c[1] =  y;
        	pt++;
    		numPts++;
    	    }
	}
    }
    
    /* create an adaptive histogram from the random data */
    aHist = Bin2DAdaptHist(data, numPts, NLimit, Strategy);
    hist2DSetAdaptiveHistogramData(HistWidget, aHist, HIST2D_SCALING);
    XtFree((char *)aHist);
}

static void zLogScalingCB(Widget w, XtPointer clientData, XtPointer callData)
{ 
    /* Change the coordinate system on the Z axis from linear to Log
       and vice versa. */
    
    XtVaSetValues(HistWidget, XmNzLogScaling, 
    	    XmToggleButtonGetState(w), NULL); 
}

static void doubleBufferCB(Widget w, XtPointer clientD, XtPointer callD)
{  
    /* Turn on and off graphics buffering */
    
    XtVaSetValues(HistWidget, XmNdoubleBuffer, XmToggleButtonGetState(w),
    	    NULL);
}

static void helpCallback(Widget w, XtPointer clientData, XtPointer callData)
{ 
    /* Display a help message about using the interactive controls */
    
    displayHelpBitmap(w, "Widget Controls", Help2DHist_bits, Help2DHist_width,
    	    Help2DHist_height);
}

static void setRangeCallback(Widget w, XtPointer clientD, XtPointer callD)
{
    /* Create a pop-up dialog, in which a user can enter the maximum and
       minimum view limits for the widget */ 
       
    double xMin, xMax, yMin, yMax, zMin, zMax;
            
    hist2DGetVisiblePart(HistWidget, &xMin, &xMax, &yMin, 
    	    &yMax, &zMin, &zMax);    
    setRange(HistWidget, &xMin, &yMin, &zMin, &xMax, &yMax, &zMax); 
    hist2DSetVisiblePart(HistWidget, xMin, xMax, yMin, yMax, zMin, zMax);  
}

static void zoomCallback(Widget w, XtPointer data, XtPointer callData)
{
    /* Create a prompt dialog where the user can enter the zoom factor. */
       
    char str[DF_MAX_PROMPT_LENGTH], *ptr;
    int response;
    double factor;
    
    response = DialogF(DF_PROMPT, w, 2, "Zoom Factor", str, "OK", "Cancel");
    if (response == 2)
    	return;
    factor = strtod(str, &ptr);
    if (ptr != str)
        hist2DZoom(HistWidget, factor);
}

static void zoomViewCallback(Widget w, XtPointer data, XtPointer callData)
{
    /* Zoom In, Zoom Out, or Reset View depending on callback argument. */
       
    switch ((int)data) {
        case 1 : hist2DZoomIn(HistWidget); 
                 break;
        case 2 : hist2DZoomOut(HistWidget);
                 break;
        case 3 : hist2DResetView(HistWidget);
                 break;
    } 
}

static void setViewAnglesCB(Widget w, XtPointer clientD, XtPointer callD)
{
    /* Create a popup dialog to allow the user to enter the view
       direction, and angle. */
       
    Widget form, labelTitle, labelFi, labelPsi, separator, ok, cancel;
    XmString xmstr;
    
    /* Create a form widget to contain the text & label widgets. */
    form = XmCreateFormDialog(w, "form", NULL, 0);
    XtVaSetValues(form, 
    	    XmNdialogTitle,  xmstr = XmStringCreateSimple("Set View Angles"),
    	    XmNautoUnmanage, FALSE, NULL);
    XmStringFree(xmstr); 
    
    xmstr =  XmStringCreateLtoR(
    	"Fi: specifies the turn of view direction projection\n\
to the XY plane from the X axis\n\
Psi:  specifies the angle between view direction\n\
and the Z axis.", XmSTRING_DEFAULT_CHARSET);	
    labelTitle = XtVaCreateManagedWidget("labelTitle", xmLabelGadgetClass, form,
    	    XmNlabelString,		xmstr,
    	    XmNtopAttachment,		XmATTACH_FORM,
    	    XmNleftAttachment,		XmATTACH_FORM,
    	    NULL);
    XmStringFree(xmstr);     
    
    /* Create a separator. */
    separator = XtVaCreateManagedWidget("separator",
            xmSeparatorWidgetClass, form,
            XmNtopAttachment,		XmATTACH_WIDGET,
            XmNtopWidget,		labelTitle,
    	    XmNtopOffset,		10,
    	    XmNleftAttachment,		XmATTACH_FORM,
    	    XmNrightAttachment,		XmATTACH_FORM,
            NULL);
   
    xmstr = XmStringCreateSimple("Fi");
    labelFi = XtVaCreateManagedWidget("labelFi", xmLabelGadgetClass, form,
    	    XmNlabelString,		xmstr,
    	    XmNtopAttachment,		XmATTACH_WIDGET,
    	    XmNtopWidget,		separator,
    	    XmNtopOffset,		10,
    	    XmNleftAttachment,		XmATTACH_FORM,
    	    XmNleftOffset,		10,
    	    NULL);
    XmStringFree(xmstr); 
    
    FiText = XtVaCreateManagedWidget("textFi", xmTextWidgetClass, form,
    	    XmNtopAttachment,		XmATTACH_WIDGET,
    	    XmNtopWidget,		separator,
    	    XmNtopOffset,		5,
    	    XmNleftAttachment,		XmATTACH_WIDGET,
    	    XmNleftWidget,          	labelFi,
    	    NULL); 
    	
    xmstr = XmStringCreateSimple("Psi");
    labelPsi = XtVaCreateManagedWidget("labelPsi", xmLabelGadgetClass, form,
    	    XmNlabelString,		xmstr,
    	    XmNtopAttachment,		XmATTACH_WIDGET,
    	    XmNtopWidget,		separator,
    	    XmNtopOffset,		10,
    	    XmNleftAttachment,		XmATTACH_WIDGET,
    	    XmNleftWidget,		FiText,
    	    XmNleftOffset,		10,
    	    NULL);
    XmStringFree(xmstr);
    	
    PsiText = XtVaCreateManagedWidget("textPsi", xmTextWidgetClass, form,
    	    XmNtopAttachment,		XmATTACH_WIDGET,
    	    XmNtopWidget,		separator,
    	    XmNtopOffset,		5,
    	    XmNleftAttachment,		XmATTACH_WIDGET,
    	    XmNleftWidget,          	labelPsi,
    	    NULL);
   
    /* Create OK push button widget and callback */
    xmstr = XmStringCreateSimple(" Ok ");
    ok = XtVaCreateManagedWidget("ok", xmPushButtonWidgetClass, form,
    	    XmNlabelString,	 	xmstr,
    	    XmNtopAttachment,		XmATTACH_WIDGET,
    	    XmNtopWidget,		 PsiText,
    	    XmNtopOffset,		20,
    	    XmNleftAttachment,		XmATTACH_FORM,
    	    XmNleftOffset,		20,  
    	    NULL);
    XtAddCallback(ok, XmNactivateCallback, 
	    (XtCallbackProc)okSetViewCallback, (void *)XtParent(form));
    XmStringFree(xmstr);
    
    /* Create Cancel push button widget and callback */ 
    xmstr = XmStringCreateSimple("Cancel");  
    cancel = XtVaCreateManagedWidget("cancel", 
    	    xmPushButtonWidgetClass, form,
    	    XmNlabelString, 		xmstr,
    	    XmNtopAttachment,		XmATTACH_WIDGET,
    	    XmNtopWidget,		PsiText,
    	    XmNtopOffset,		20,
    	    XmNleftAttachment,		XmATTACH_WIDGET,
    	    XmNleftWidget,		ok,
    	    XmNleftOffset,		5, 
    	    NULL);
    XtAddCallback(cancel, XmNactivateCallback, 
    	    (XtCallbackProc)cancelCallback, (void *)XtParent(form));
    XmStringFree(xmstr);
    	
    XtManageChild(form);
}

static void cancelCallback(Widget w, Widget x, XtPointer callData)
{
    XtDestroyWidget(x);
}

static void okSetViewCallback(Widget w, Widget x, XtPointer callD)
{
    /* This function is called after the user has entered the view
       direction and angle from setViewAnglesCB. If the
       users input is error free, the function hist2DSetViewAngles
       is called to change the angles. If the users
       input is blank or in error, the function hist2DSetViewAngles will
       be called with the values returned from hist2DGetViewAngles. */

    double fi, psi, tempDouble;
    char *str, *ptr;
    
    /* Get defaults for blank or error'd fields */
    hist2DGetViewAngles(HistWidget, &fi, &psi);
    
    /* Get text from the 2 widgets and convert string from char to double. */
    str = XmTextGetString(FiText);
    tempDouble = strtod(str, &ptr);
    if (ptr != str)
        fi = tempDouble;
    XtFree(str);
    str = XmTextGetString(PsiText);
    tempDouble = strtod(str, &ptr);
    if (ptr != str)
        psi = tempDouble;
    XtFree(str);
    
    /* Pop down the dialog */
    XtDestroyWidget(x);
    
    /* Set the new view angles */
    hist2DSetViewAngles(HistWidget, fi, psi);
}

static void getViewAnglesCB(Widget w, XtPointer clientD, XtPointer callD)
{
    /* Display the current view angles */
    
    double fi, psi;
    
    hist2DGetViewAngles(HistWidget, &fi, &psi);
    DialogF(DF_INF, w, 1, "Turn of view direction: fi = %g\n\
Angle between view direction: psi = %g", "OK", fi, psi);
}

static void printCallback(Widget w, XtPointer clientD, XtPointer callD)
{
    /* This generates a postscript file and invokes the print dialog
       to prompt for a print queue and submit the print job */
       
    static char fileName[] = "2DHist.ps";
	
    hist2DmakePsImage(HistWidget, fileName); 
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


	
    	        
     
       

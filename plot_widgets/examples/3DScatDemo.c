/*************************************************************************
*                                                                        *
* scat3Demo.c - 3D Scat Plot Widget Example Program                      *
*                                                                        *
* This program demonstrates the capabilities of the Scat3D widget        *
*                                                                        *
* June 6, 1994, by George Dimas and Mark Edel                            *
*                                                                        *
*************************************************************************/
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include "../plot_widgets/3DScat.h"
#include "../plot_widgets/3DHelp.xbm"
#include "../util/printUtils.h"
#include "../util/DialogF.h"
#include "demoUtils.h"

#define NUM_POINTS 20000

static Widget Scat3D;

static void zoomCallback(Widget w, XtPointer clientD, XtPointer callD);
static void exitCallback(Widget w, XtPointer clientD, XtPointer callD);
static void doubleBufferCB(Widget w, XtPointer clientD, XtPointer callD);
static void helpCallback(Widget w, XtPointer clientData, XtPointer callData);
static void xyzScalingCallback(Widget w, XtPointer clientD, XtPointer callD);
static void printCallback(Widget w, XtPointer clientD, XtPointer callD);
static void darkerCallback(Widget w, XtPointer clientD, XtPointer callD);
static void setRangeCallback(Widget w, XtPointer clientD, XtPointer callD);
static void setLimitCallback(Widget w, XtPointer clientD, XtPointer callD);
static void zoomAnyCallback(Widget w, XtPointer clientD, XtPointer callD);


main(int argc, char *argv[])
{
    XtAppContext appContext;
    Widget toplevel, form, rowColumn;
    int numPoints = 0;
    Scat3DPoint *point, *points;
    float x, y, z;
    
    /* Initialize X and the X Toolkit */   
    toplevel = XtAppInitialize(&appContext, "Scat3DTest", NULL,
    	    0, &argc, argv, NULL, NULL, 0);

    /* Create a form widget to hold rowcolumn, and Scat3D */
    form = XtCreateManagedWidget("form", xmFormWidgetClass, toplevel, NULL, 0);

    /* Create a scat widget to display the points. */
    Scat3D = XtVaCreateManagedWidget("Scat3D", scat3DWidgetClass, form,
            XmNdarkerPoints,       FALSE,
            XmNtopAttachment,      XmATTACH_FORM,
            XmNbottomAttachment,      XmATTACH_FORM,
    	    XmNleftAttachment,     XmATTACH_POSITION,
    	    XmNleftPosition,       31,
    	    XmNrightAttachment,    XmATTACH_FORM,
    	    NULL); 
    Scat3DSetAxesNames(Scat3D, "X Axis", "Y Axis", "Z Axis");
    	  
    /* Create the points to put in the plot. */
    points = (Scat3DPoint *)XtMalloc(sizeof(Scat3DPoint) * NUM_POINTS);
    point = points;
    srand(1);
    while (numPoints < NUM_POINTS) {
        x = rand() / (RAND_MAX / 2.) - 1.;
        y = rand() / (RAND_MAX / 2.) - 1.;
        z = rand() / (RAND_MAX / 2.) - 1.;
        if (fermiLogo(x, y) && (z < -.8 || z > .8)) {
            point->x = x + 1.;
            point->y = y + 1.;
            point->z = z + 1.001; /* make sure it's log scaleable */
            point++;
            numPoints++;
        }
    }
    
    /* Display the points. */
    Scat3DSetContents(Scat3D, points, numPoints, SCAT3D_RESCALE);
    XtFree((char *)points);
      
    /* Create a rowColumn widget to hold toggle buttons & push buttons. */
    rowColumn = XtVaCreateManagedWidget("rowColumn", xmRowColumnWidgetClass,
    	    form,
            XmNtopAttachment,    XmATTACH_WIDGET,
            XmNleftAttachment,   XmATTACH_WIDGET,
            XmNrightAttachment,  XmATTACH_POSITION,
            XmNrightPosition,    30,
            NULL);
     
    /* Create 4 square shaped toggle buttons */
    createToggleButton(rowColumn, "logX", "Log X",
    	    xyzScalingCallback, (void *)'x');
    createToggleButton(rowColumn, "logY", "Log Y",
    	    xyzScalingCallback, (void *)'y');
    createToggleButton(rowColumn, "logZ", "Log Z",
    	    xyzScalingCallback, (void *)'z');
    createToggleButton(rowColumn, "darkerPoints", "Darker Points",
    	    darkerCallback, NULL);
    createToggleButton(rowColumn, "doubleBuffer",
    	    "Double Buffer", doubleBufferCB, NULL);
    
    /* Create  8 pushButton widgets and callbacks for rowColumn. */
    createButton(rowColumn, "controlsHelp", "Controls Help...",
	    helpCallback, NULL);
    createButton(rowColumn, "setLimits", "Set Limits...",
    	    setLimitCallback, NULL);
    createButton(rowColumn, "setVisibleRange", "Set Visible Range...",
    	    setRangeCallback, NULL);
    createButton(rowColumn, "zoom", "Zoom...",
    	    zoomAnyCallback, NULL);
    createButton(rowColumn, "zoomIn", "Zoom In",
    	    zoomCallback, (XtPointer)1);
    createButton(rowColumn, "zoomOut", "Zoom Out",
    	    zoomCallback, (XtPointer)2);
    createButton(rowColumn, "resetView", "Reset View",
    	    zoomCallback, (XtPointer)3);
    createButton(rowColumn, "print", "Print",
    	    printCallback, NULL);
    createButton(rowColumn, "exitProgram", "Exit Program",
    	    exitCallback, NULL);
    
    /* Display all widgets and call XtMainLoop to continuously process events */
    XtRealizeWidget(toplevel);
    XtAppMainLoop(appContext);
}

static void setRangeCallback(Widget w, XtPointer clientD, XtPointer callD)
{
    /* Create a pop-up dialog, in which a user can enter the currently
       visible data range for the widget */ 
       
    float xMin, xMax, yMin, yMax, zMin, zMax;
    double dXMin, dXMax, dYMin, dYMax, dZMin, dZMax;
            
    Scat3DGetVisiblePart(Scat3D, &xMin, &xMax, &yMin, &yMax, &zMin, &zMax);    
    dXMin=xMin; dXMax=xMax; dYMin=yMin; dYMax=yMax; dZMin=zMin; dZMax=zMax;
    setRange(Scat3D, &dXMin, &dYMin, &dZMin, &dXMax, &dYMax, &dZMax); 
    xMin=dXMin; xMax=dXMax; yMin=dYMin; yMax=dYMax; zMin=dZMin; zMax=dZMax;
    Scat3DSetVisiblePart(Scat3D, xMin, xMax,yMin, yMax, zMin, zMax);
}

static void setLimitCallback(Widget w, XtPointer clientD, XtPointer callD)
{
    /* Create a pop-up dialog, in which a user can enter the maximum and
       minimum interactive zoom limits for the widget */ 
       
    float xMin, xMax, yMin, yMax, zMin, zMax;
    double dXMin, dXMax, dYMin, dYMax, dZMin, dZMax;
            
    Scat3DGetVisiblePart(Scat3D, &xMin, &xMax, &yMin, &yMax, &zMin, &zMax);    
    dXMin=xMin; dXMax=xMax; dYMin=yMin; dYMax=yMax; dZMin=zMin; dZMax=zMax;
    setRange(Scat3D, &dXMin, &dYMin, &dZMin, &dXMax, &dYMax, &dZMax); 
    xMin=dXMin; xMax=dXMax; yMin=dYMin; yMax=dYMax; zMin=dZMin; zMax=dZMax;
    Scat3DSetLimits(Scat3D, xMin, xMax,yMin, yMax, zMin, zMax);
}

static void zoomAnyCallback(Widget w, XtPointer data, XtPointer callData)
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
        Scat3DZoom(Scat3D, factor);  
}

static void zoomCallback(Widget w, XtPointer data, XtPointer callData)
{
    /* Zoom In, Zoom Out, or Reset View, depending on clientData. */
       
    switch ((int)data) {
        case 1 : Scat3DZoomIn(Scat3D); 
                 break;
        case 2 : Scat3DZoomOut(Scat3D);
                 break;
        case 3 : Scat3DResetView(Scat3D);
                 break;
    } 
}
 
static void xyzScalingCallback(Widget w, XtPointer data, XtPointer callData)
{ 
    /* Cchanges coordinates from linear to Log and vice versa on one
       axis, determined by the "data" parameter. */
    
    int state = XmToggleButtonGetState(w);
    switch ((int)data) {
        case 'x' : XtVaSetValues(Scat3D, XmNxLogScaling, state, NULL); 
                   break;
        case 'y' : XtVaSetValues(Scat3D, XmNyLogScaling, state, NULL);
                   break;
        case 'z' : XtVaSetValues(Scat3D, XmNzLogScaling, state, NULL);
                   break;
    } 
}

static void doubleBufferCB(Widget w, XtPointer clientD, XtPointer callD)
{  
    /* Turn on and off graphics buffering */
    
    XtVaSetValues(Scat3D, XmNdoubleBuffer, XmToggleButtonGetState(w), NULL);
}

static void darkerCallback(Widget w, XtPointer clientD, XtPointer callD)
{  
    /* Turn on or off point darkening */
    
    XtVaSetValues(Scat3D, XmNdarkerPoints, XmToggleButtonGetState(w), NULL);
}

static void helpCallback(Widget w, XtPointer clientData, XtPointer callData)
{ 
    /* Display a help message about using the interactive controls */
    
    displayHelpBitmap(w, "Widget Controls", Help3D_bits, Help3D_width,
    	    Help3D_height);
}

static void printCallback(Widget w, XtPointer clientD, XtPointer callD)
{
    /* Generate a postscript file and invoke the print dialog
       to prompt for a print queue and submit the print job */
       
    static char fileName[] = "Scat3D.ps";
	
    Scat3DPrintContents(Scat3D, fileName); 
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

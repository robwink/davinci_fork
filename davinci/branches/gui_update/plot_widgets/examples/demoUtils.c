/**********************************************************************
*                                                                     *
* demoUtils.c - Common parts for example programs                     *
*                                                                     *
* August 1, 1994 by George Dimas and Paul Lebrun                      *
*                                                                     *
**********************************************************************/
#include <math.h>
#include <Xm/Xm.h>
#include <Xm/Text.h>
#include <Xm/LabelG.h>
#include <Xm/PushB.h>
#include <Xm/Form.h>
#include <Xm/ToggleB.h>
#include "../util/DialogF.h"
#include "../util/misc.h"
#include "demoUtils.h"

typedef struct {
    double minXLim, minYLim, minZLim, maxXLim, maxYLim, maxZLim;
    int doneWithDialog;
    int threeD;
    Widget rangeText[6];
} rangeDialog;

static void rangeOkCB(Widget w, XtPointer clientData, XtPointer callData);
static void rangeCancelCB(Widget w, XtPointer clientData, XtPointer callData);
static void helpDismissCB(Widget w, Pixmap pmToDestroy, caddr_t callData);

void createButton(Widget parent, char *name, char *label,
	XtCallbackProc callbackProc, XtPointer cbArg)
{
    XmString s1;
    Widget button;
    	   	  	
    button = XtVaCreateManagedWidget(name, xmPushButtonWidgetClass, parent,
    	    XmNlabelString, s1=XmStringCreateSimple(label), NULL);
    XtAddCallback(button, XmNactivateCallback, callbackProc, cbArg); 
    XmStringFree(s1);
}

Widget createToggleButton(Widget parent, char *name, char *label,
	XtCallbackProc callbackProc, XtPointer cbArg)
{
    XmString s1;
    Widget button;
    	   	  	
    button = XtVaCreateManagedWidget(name, xmToggleButtonWidgetClass, parent,
    	    XmNlabelString, s1=XmStringCreateSimple(label), NULL);
    XtAddCallback(button, XmNvalueChangedCallback,
    	    (XtCallbackProc)callbackProc, cbArg); 
    XmStringFree(s1);
    return button;
}

/* Simple logical function that draws the Fermi Logo.  Returns 1 (TRUE) or
   0 (FALSE) depending wether we are  on the dark (white) part of the Logo.
   The dimension of the Logo are approximately 1.0 by 1.0, leaving a bit of
   spacing for clarity. */
int fermiLogo(float x, float y)
{
    int result;
    float xa, ya, x1, y1, yMin, yMax, xx, yy, rOut2;
    static float barThickness = 0.2500,  lengthShort = 0.5417,
                posShort = 0.45,  lengthLong = 0.7500,
                posLong = 0.08333,  radiousIn = 0.166667,
                radiousOut = 0.083333,  xCircleOut = 0.416666,
                xCircleIn = 0.250,  topHeight = 0.80;
                
    /* Assume quadrant symmetry. */
    
    xa = fabs(x);
    ya = fabs(y);
    
    if ( xa > lengthLong )
        return 0;
    if ( ya > topHeight)
        return 0;
    if ( xa < lengthShort && (ya > posShort && ya <(posShort + barThickness)))
        return 1;

        
    /* Assume eight fold symmetry. */
    
    if ( xa > ya ) {
        x1 = xa;
        y1 = ya;
     }
    else {
        x1 = ya;
        y1 = xa;
     }
     
    if (y1 < posLong)
        return 0;
    if (x1 > xCircleOut && y1 > (posLong + barThickness))
        return 0;
        
    /* More complicated case, do the circles... */
    
    if (x1 > xCircleOut)
    	yMax = posLong + barThickness;
    else if (x1 < (xCircleOut - radiousOut * 0.7071068))
    	    yMax = x1;
    else {
    	 xx = x1 - xCircleOut;
    	 rOut2 = radiousOut * radiousOut;
    	 yy = radiousOut - sqrt(rOut2 - xx * xx);
    	 yMax = posLong + barThickness + yy;
    }
    	 
    if (x1 > xCircleIn) 
    	yMin = posLong;
    else {
    	xx = xCircleIn - x1;
    	rOut2 = radiousIn * radiousIn;
    	yy = radiousIn - sqrt(rOut2 - xx * xx);
    	yMin = posLong + yy;
    }
    
    if (y1 < yMin || y1 > yMax)
    	return 0;
    	
    return 1;
}

/* Prompt the user for new plot limits.  If minZLim and maxZLim are NULL,
   present a 2D (x,y) dialog, otherwise present a 3D dialog */
void setRange(Widget w, double *minXLim, double *minYLim, double *minZLim,
	double *maxXLim, double *maxYLim, double *maxZLim)
{
    /* This function will create and pop-up a form dialog, in which a user
       can enter the maximum and minimum values for the new range. */ 
       
    Widget labelMinX, labelMinY, labelMinZ, labelMaxX, labelMaxY, labelMaxZ;
    Widget form, ok, cancel;
    XmString xmstr;
    rangeDialog dialog;
    char string[100];
    int threeD = minZLim != NULL;
    
    /* Fill in the data to pass the the callback routines */
    dialog.minXLim = *minXLim;
    dialog.minYLim = *minYLim;
    dialog.maxXLim = *maxXLim;
    dialog.maxYLim = *maxYLim;
    if (threeD) {
    	 dialog.minZLim = *minZLim;
    	 dialog.maxZLim = *maxZLim;
    }
    dialog.threeD = threeD;
    dialog.doneWithDialog = False;
    
    /* Create a form widget to contain the text & label widgets. */
    form = XmCreateFormDialog(w, "form", NULL, 0);
    XtVaSetValues(form, 
    	    XmNdialogTitle, xmstr=XmStringCreateSimple("Set Range"),
    	    XmNautoUnmanage, FALSE, 
    	    NULL);
    XmStringFree(xmstr);

    /* Create label text widgets for range fields */
    sprintf(string, "min X (%g)", *minXLim);
    labelMinX = XtVaCreateManagedWidget("labelMinX", 
    	    xmLabelGadgetClass, form,
    	    XmNlabelString, xmstr=XmStringCreateSimple(string),
    	    XmNtopAttachment, XmATTACH_FORM,
    	    XmNtopOffset, 10,
    	    XmNleftAttachment, XmATTACH_FORM,
    	    XmNleftOffset, 2,
    	    NULL);
    XmStringFree(xmstr); 
    
    dialog.rangeText[0] = XtVaCreateManagedWidget("textMinX", 
    	    xmTextWidgetClass, form,
    	    XmNcolumns, 12,
    	    XmNtopAttachment, XmATTACH_FORM,
    	    XmNtopOffset, 5,
    	    XmNleftAttachment, XmATTACH_WIDGET,
    	    XmNleftWidget, labelMinX,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, threeD ? 33 : 50,
    	    NULL); 
    	
    sprintf(string, "min Y (%g)", *minYLim);
    labelMinY = XtVaCreateManagedWidget("labelMinY", 
    	    xmLabelGadgetClass, form,
    	    XmNlabelString, xmstr=XmStringCreateSimple(string),
    	    XmNtopAttachment, XmATTACH_FORM,
    	    XmNtopOffset, 10,
    	    XmNleftAttachment, XmATTACH_WIDGET,
    	    XmNleftWidget, dialog.rangeText[0],
    	    XmNleftOffset, 10,
    	    NULL);
    XmStringFree(xmstr);
    	
    dialog.rangeText[1] = XtVaCreateManagedWidget("textMinY", 
    	    xmTextWidgetClass, form,
    	    XmNcolumns, 12,
    	    XmNtopAttachment, XmATTACH_FORM,
    	    XmNtopOffset, 5,
    	    XmNleftAttachment, XmATTACH_WIDGET,
    	    XmNleftWidget, labelMinY,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, threeD ? 66 : 99,
    	    NULL);
    
    if (threeD) {
	sprintf(string, "min Z (%g)", *minZLim);
	labelMinZ = XtVaCreateManagedWidget("labelMinZ", 
    		xmLabelGadgetClass, form,
    		XmNlabelString, xmstr=XmStringCreateSimple(string),
    		XmNtopAttachment, XmATTACH_FORM,
    		XmNtopOffset, 10,
    		XmNleftAttachment, XmATTACH_WIDGET,
    		XmNleftWidget, dialog.rangeText[1],
    		XmNleftOffset, 10,
    		NULL);
	XmStringFree(xmstr);
	dialog.rangeText[2] = XtVaCreateManagedWidget("textMinZ", 
    		xmTextWidgetClass, form,
    		XmNcolumns, 12,
    		XmNtopAttachment, XmATTACH_FORM,
    		XmNtopOffset, 5,
    		XmNleftAttachment, XmATTACH_WIDGET,
    		XmNleftWidget, labelMinZ,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, 99,
    		NULL);
    }
    	
    sprintf(string, "max X (%g)", *maxXLim);
    labelMaxX = XtVaCreateManagedWidget("labelMaxX", 
    	    xmLabelGadgetClass, form,
    	    XmNlabelString, xmstr=XmStringCreateSimple(string),
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNtopWidget, labelMinX,
    	    XmNtopOffset, 20,
    	    XmNleftAttachment, XmATTACH_FORM,
    	    XmNleftOffset, 2,
    	    NULL);
    XmStringFree(xmstr);
    dialog.rangeText[3] = XtVaCreateManagedWidget("textMaxX", 
    	    xmTextWidgetClass, form,
    	    XmNcolumns, 12,
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNtopWidget, dialog.rangeText[0],
    	    XmNtopOffset, 5,
    	    XmNleftAttachment, XmATTACH_WIDGET,
    	    XmNleftWidget, labelMaxX,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, threeD ? 33 : 50,
    	    NULL); 
    	
    sprintf(string, "max Y (%g)", *maxYLim);
    labelMaxY = XtVaCreateManagedWidget("labelMaxY", 
            xmLabelGadgetClass, form,
            XmNlabelString, xmstr=XmStringCreateSimple(string),
            XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNtopWidget, labelMinY,
    	    XmNtopOffset, 20,
    	    XmNleftAttachment, XmATTACH_WIDGET,
    	    XmNleftWidget, dialog.rangeText[3],
    	    XmNleftOffset, 10,
    	    NULL);
    XmStringFree(xmstr); 
    dialog.rangeText[4] = XtVaCreateManagedWidget("textMaxY", 
    	    xmTextWidgetClass, form,
    	    XmNcolumns, 12,
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNtopWidget, dialog.rangeText[1],
    	    XmNtopOffset, 5, 
    	    XmNleftAttachment, XmATTACH_WIDGET,
    	    XmNleftWidget, labelMaxY,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, threeD ? 66 : 99,
    	    NULL);	
    
    if (threeD) {	
	sprintf(string, "max Z (%g)", *maxZLim);
	labelMaxZ = XtVaCreateManagedWidget("labelMaxZ", 
        	xmLabelGadgetClass, form,
        	XmNlabelString, xmstr=XmStringCreateSimple(string),
        	XmNtopAttachment, XmATTACH_WIDGET,
    		XmNtopWidget, labelMinY,
    		XmNtopOffset, 20,
    		XmNleftAttachment, XmATTACH_WIDGET,
    		XmNleftWidget, dialog.rangeText[4],
    		XmNleftOffset, 10,
    		NULL);
	XmStringFree(xmstr); 
	dialog.rangeText[5] = XtVaCreateManagedWidget("textMaxZ", 
    		xmTextWidgetClass, form,
    		XmNcolumns, 12,
    		XmNtopAttachment, XmATTACH_WIDGET,
    		XmNtopWidget, dialog.rangeText[1],
    		XmNtopOffset, 5, 
    		XmNleftAttachment, XmATTACH_WIDGET,
    		XmNleftWidget, labelMaxZ,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, 99,
    		NULL);	
    }
    
    /* Create OK push button widget and callback */
    ok = XtVaCreateManagedWidget("ok", 
    	    xmPushButtonWidgetClass, form,
    	    XmNlabelString, xmstr=XmStringCreateSimple(" OK "),
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNtopWidget, dialog.rangeText[3],
    	    XmNtopOffset, 20,
    	    XmNleftAttachment, XmATTACH_FORM,
    	    XmNleftOffset, 20,  
    	    NULL);
    XtAddCallback(ok, XmNactivateCallback, rangeOkCB, (XtPointer)&dialog);
    XmStringFree(xmstr);
    
    /* Create Cancel push button widget and callback */ 
    cancel = XtVaCreateManagedWidget("cancel", 
    	    xmPushButtonWidgetClass, form,
    	    XmNlabelString, xmstr=XmStringCreateSimple("Cancel"),
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNtopWidget, dialog.rangeText[3],
    	    XmNtopOffset, 20,
    	    XmNleftAttachment, XmATTACH_WIDGET,
    	    XmNleftWidget, ok,
    	    XmNleftOffset, 5, 
    	    NULL);
    XtAddCallback(cancel, XmNactivateCallback, 
    	    (XtCallbackProc) rangeCancelCB, (XtPointer)&dialog);
    XmStringFree(xmstr);

    /* Pop up the dialog and process events until one of the callbacks
       signals that we're done, then return the values and destroy the
       dialog */
    XtManageChild(form);
    while (dialog.doneWithDialog == 0)
        XtAppProcessEvent(XtWidgetToApplicationContext(form), XtIMAll);
    *minXLim = dialog.minXLim;
    *minYLim = dialog.minYLim;
    *maxXLim = dialog.maxXLim;
    *maxYLim = dialog.maxYLim;
    if (threeD) {
    	*minZLim = dialog.minZLim;
    	*maxZLim = dialog.maxZLim;
    }
    XtDestroyWidget(XtParent(form));
}

/* This function is called after the user has pressed the OK
   button in the Set Range dialog.  It checks the numbers that were
   entered, and returns the new values if they were ok, or pops up
   a dialog explaining what was wrong if they weren't. */
static void rangeOkCB(Widget w, XtPointer clientData, XtPointer callData)
{
    double minXLim, maxXLim, minYLim, maxYLim, minZLim, maxZLim;
    rangeDialog *dialog = (rangeDialog *)clientData;
    int result;
    
    /* Get text from the 4 widgets and convert string from char to double. */
    result = GetFloatTextWarn(dialog->rangeText[0], &minXLim, "min X", False);
    if (result == TEXT_NOT_NUMBER)
    	return;
    if (result == TEXT_IS_BLANK)
    	minXLim = dialog->minXLim;
    result = GetFloatTextWarn(dialog->rangeText[1], &minYLim, "min Y", False);
    if (result == TEXT_NOT_NUMBER)
    	return;
    if (result == TEXT_IS_BLANK)
    	minYLim = dialog->minYLim;
    if (dialog->threeD) {
	result = GetFloatTextWarn(dialog->rangeText[2], &minZLim, "min Z",
		False);
	if (result == TEXT_NOT_NUMBER)
    	    return;
	if (result == TEXT_IS_BLANK)
    	    minZLim = dialog->minZLim;
    }
    result = GetFloatTextWarn(dialog->rangeText[3], &maxXLim, "max X", False);
    if (result == TEXT_NOT_NUMBER)
    	return;
    if (result == TEXT_IS_BLANK)
    	maxXLim = dialog->maxXLim;
    result = GetFloatTextWarn(dialog->rangeText[4], &maxYLim, "max Y", False);
    if (result == TEXT_NOT_NUMBER)
    	return;
    if (result == TEXT_IS_BLANK)
    	maxYLim = dialog->maxYLim;
    if (dialog->threeD) {
	result = GetFloatTextWarn(dialog->rangeText[5], &maxZLim, "max Z",
		False);
	if (result == TEXT_NOT_NUMBER)
    	    return;
	if (result == TEXT_IS_BLANK)
    	    maxZLim = dialog->maxZLim;
    }

    if (minXLim >= maxXLim) {
    	DialogF(DF_WARN, w, 1, "min X > max X", "OK");
	return;
    }
    if (minYLim >= maxYLim) {
    	DialogF(DF_WARN, w, 1, "min Y > max Y", "OK");
	return;
    }
    if (dialog->threeD && minZLim >= maxZLim) {
    	DialogF(DF_WARN, w, 1, "min Z > max Z", "OK");
	return;
    }
    
    /* return the new values and signal completion */
    dialog->minXLim = minXLim;
    dialog->minYLim = minYLim;
    dialog->minZLim = minZLim;
    dialog->maxXLim = maxXLim;
    dialog->maxYLim = maxYLim;
    dialog->maxZLim = maxZLim;
    dialog->doneWithDialog = True;
}

static void rangeCancelCB(Widget w, XtPointer clientData, XtPointer callData)
{
    ((rangeDialog *)clientData)->doneWithDialog = True;
}

void displayHelpBitmap(Widget parent, char *title, unsigned char *bits,
	int width, int height)
{
    Widget form, button, label;
    XmString st1;
    Pixmap labelPixmap;
    Pixel fg, bg;

    /* create a form dialog to hold everything */
    form = XmCreateFormDialog(parent, "helpForm", NULL, 0);

    /* create a pixmap to display from the bit data, the pixmap must be
       of the same depth as the window it will be drawn to */
    XtVaGetValues(form, XmNforeground, &fg, XmNbackground, &bg, 0);
    labelPixmap = XCreatePixmapFromBitmapData(XtDisplay(parent),
    	    RootWindowOfScreen(XtScreen(parent)), (char *)bits, width, height,
    	    fg, bg, DefaultDepthOfScreen(XtScreen(parent)));

    /* create the widgets in the window: a dismiss button, and a label
       to display the bitmap */
    label = XtVaCreateManagedWidget("bitmap", xmLabelGadgetClass,  form,
    	    XmNlabelType, XmPIXMAP,
    	    XmNlabelPixmap, labelPixmap, 0);
    button = XtVaCreateManagedWidget("dismiss", xmPushButtonWidgetClass, form,
    	    XmNlabelString, st1=XmStringCreateSimple("Dismiss"),
    	    XmNtopAttachment, XmATTACH_WIDGET,
    	    XmNtopWidget, label,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 35,
    	    XmNrightPosition, 65, 0);
    XtAddCallback(button, XmNactivateCallback, (XtCallbackProc)helpDismissCB,
    	    (char *)labelPixmap);
    XmStringFree(st1);
    XtVaSetValues(form, XmNdefaultButton, button, 0);
    
    /* set the title of the window */
    XtVaSetValues(XtParent(form), XmNtitle, title, 0);
    
    /* pop up and return wigit id of the help dialog just created */
    XtManageChild(form);
}

static void dismissCB(Widget w, caddr_t clientData, caddr_t callData)
{
    XtDestroyWidget(XtParent(w));
}

static void helpDismissCB(Widget w, Pixmap pmToDestroy, caddr_t callData)
{
    XFreePixmap(XtDisplay(w), pmToDestroy);
    XtDestroyWidget(XtParent(w));
}

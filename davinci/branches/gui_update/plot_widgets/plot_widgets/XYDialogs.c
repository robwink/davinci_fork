/*******************************************************************************
*									       *
* XYDialogs.c - Dialogs for General Purpose XY Plot Widget		       *
*									       *
* Copyright (c) 1991, 1996 Universities Research Association, Inc.	       *
* All rights reserved.							       *
* 									       *
* This material resulted from work developed under a Government Contract and   *
* is subject to the following license:  The Government retains a paid-up,      *
* nonexclusive, irrevocable worldwide license to reproduce, prepare derivative *
* works, perform publicly and display publicly by or for the Government,       *
* including the right to distribute to other Government contractors.  Neither  *
* the United States nor the United States Department of Energy, nor any of     *
* their employees, makes any warrenty, express or implied, or assumes any      *
* legal liability or responsibility for the accuracy, completeness, or         *
* usefulness of any information, apparatus, product, or process disclosed, or  *
* represents that its use would not infringe privately owned rights.           *
*                                        				       *
* Fermilab Nirvana GUI Library						       *
* May 28, 1992								       *
*									       *
* Written by Mark Edel							       *
*									       *
*******************************************************************************/
#include <stdio.h>
#include <X11/X.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/LabelG.h>
#include <Xm/PushBG.h>
#include <Xm/ToggleBG.h>
#include <Xm/ToggleB.h>
#include <Xm/SeparatoG.h>
#include <Xm/SelectioB.h>
#include "../util/misc.h"
#include "XY.h"
#include "XYDialogs.h"

#define MAX_COLORS 23
static char *ColorName[MAX_COLORS] = {"black", "red", "DodgerBlue3", 
	"magenta2", "blue", "orange", "cyan4", "hot pink", "grey58", 
	"green3", "MediumOrchid2",  "brown", "dark turquoise", "violet", 
	"peru", "MediumPurple", "deep sky blue", "violet red", "forest green", 
        "cornflower blue", "grey42", "salmon", "white"};
static Pixel ColorPixel[MAX_COLORS] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}; /* not in use */

#define N_FILL_STYLES 39
static unsigned HistFillStyles[N_FILL_STYLES] = {XY_NO_FILL, XY_SOLID_FILL,
    	XY_FINE_HORIZ_FILL, XY_COARSE_HORIZ_FILL, XY_FINE_VERT_FILL,
	XY_COARSE_VERT_FILL, XY_FINE_GRID_FILL, XY_COARSE_GRID_FILL,
	XY_FINE_X_FILL, XY_COARSE_X_FILL, XY_FINE_45DEG_FILL,
	XY_MED_45DEG_FILL, XY_COARSE_45DEG_FILL, XY_FINE_30DEG_FILL,
	XY_COARSE_30DEG_FILL, XY_FINE_60DEG_FILL, XY_COARSE_60DEG_FILL,
	XY_R_FINE_45DEG_FILL, XY_R_MED_45DEG_FILL, XY_R_COARSE_45DEG_FILL,
	XY_R_FINE_30DEG_FILL, XY_R_COARSE_30DEG_FILL, XY_R_FINE_60DEG_FILL,
	XY_R_COARSE_60DEG_FILL, XY_L_FINE_HORIZ_FILL, XY_L_COARSE_HORIZ_FILL,
	XY_L_FINE_VERT_FILL, XY_L_COARSE_VERT_FILL, XY_L_FINE_GRID_FILL,
	XY_L_COARSE_GRID_FILL, XY_L_FINE_X_FILL, XY_L_COARSE_X_FILL,
	XY_L_FINE_45DEG_FILL, XY_L_MED_45DEG_FILL, XY_L_COARSE_45DEG_FILL,
	XY_L_FINE_30DEG_FILL, XY_L_COARSE_30DEG_FILL, XY_L_FINE_60DEG_FILL,
	XY_L_COARSE_60DEG_FILL};
	
typedef struct {
    Widget form;
    Widget sizeShell;
    Widget lineBtns[XY_N_HIST_LINE_STYLES];
    Widget markBtns[XY_N_MARK_STYLES];
    Widget sizeBtns[XY_N_MARK_SIZES];
    Widget lineColorBtns[XY_N_COLORS];
    Widget markColorBtns[XY_N_COLORS];
    Pixmap linePixmaps[XY_N_HIST_LINE_STYLES];
    Pixmap markPixmaps[XY_N_MARK_STYLES];
    Pixmap sizePixmaps[XY_N_MARK_SIZES];
    Pixmap lnColorPixmaps[XY_N_COLORS];
    Pixmap mkColorPixmaps[XY_N_COLORS];
    Widget sizeDlogBtns[XY_N_MARK_SIZES];
    Widget *lineOptMenus;
    Widget *markOptMenus;
    Widget *sizeOptMenus;
    Widget *lnColorOptMenus;
    Widget *mkColorOptMenus;
    Pixmap *samplePixmaps;
    Widget *sampleLabels;
    XYCurve *origCurves;
    XYCurve *curves;
    XYMarkLineCallbackProc okCB;
    XYMarkLineCallbackProc applyCB;
    XYMarkLineCallbackProc dismissCB;
    void *okArg;
    void *applyArg;
    void *dismissArg;
    GC gc;
    int nCurves;
    int nColors;
    Pixel color[XY_N_COLORS];
    XColor color_def[XY_N_COLORS];
    Boolean color_used[XY_N_COLORS];
} markDialog;
	
typedef struct {
    Widget form;
    Widget lineBtns[XY_N_HIST_LINE_STYLES];
    Widget fillBtns[N_FILL_STYLES];
    Widget lineColorBtns[XY_N_COLORS];
    Widget fillColorBtns[XY_N_COLORS];
    Pixmap linePixmaps[XY_N_HIST_LINE_STYLES];
    Pixmap fillPixmaps[N_FILL_STYLES];
    Pixmap lineColorPixmaps[XY_N_COLORS];
    Pixmap fillColorPixmaps[XY_N_COLORS];
    Widget *lineOptMenus;
    Widget *fillOptMenus;
    Widget *lineColorOptMenus;
    Widget *fillColorOptMenus;
    Pixmap *samplePixmaps;
    Widget *sampleLabels;
    XYHistogram *origHists;
    XYHistogram *hists;
    XYHistCallbackProc okCB;
    XYHistCallbackProc applyCB;
    XYHistCallbackProc dismissCB;
    void *okArg;
    void *applyArg;
    void *dismissArg;
    GC gc;
    int nHists;
    int nColors;
    Pixel color[XY_N_COLORS];
    XColor color_def[XY_N_COLORS];
    Boolean color_used[XY_N_COLORS];
} histStyleDialog;

enum resultFlags {NO_RESULT, RESULT_OK, RESULT_DISMISS};

static Widget createLineStyleMenu(Widget parent, GC drawGC, Drawable pmWindow,
    	Pixmap pixmaps[XY_N_HIST_LINE_STYLES],
    	Widget btns[XY_N_HIST_LINE_STYLES], XtCallbackProc cbProc, void *cbArg);
static Widget createHistLineStyleMenu(Widget parent, GC drawGC,
    	Drawable pmWindow, Pixmap pixmaps[XY_N_HIST_LINE_STYLES],
    	Widget btns[XY_N_HIST_LINE_STYLES], XtCallbackProc cbProc, void *cbArg);
static Widget createFillStyleMenu(Widget parent, GC drawGC, Drawable pmWindow,
    	Pixmap pixmaps[N_FILL_STYLES], Widget btns[N_FILL_STYLES],
    	XtCallbackProc cbProc, void *cbArg);
static Widget createMarkStyleMenu(Widget parent, GC drawGC, Drawable pmWindow,
    	Pixmap pixmaps[XY_N_MARK_STYLES], Widget btns[XY_N_MARK_STYLES],
    	XtCallbackProc cbProc, void *cbArg);
static Widget createMarkSizeMenu(Widget parent, GC drawGC, Drawable pmWindow,
    	Pixmap pixmaps[XY_N_MARK_SIZES], Widget btns[XY_N_MARK_SIZES],
    	XtCallbackProc cbProc, void *cbArg);
static Widget createMarkColorMenu(Widget parent, GC drawGC, Drawable pmWindow,
    	int nColors, Pixel colors[], Pixmap pixmaps[], Widget btns[],
    	XtCallbackProc cbProc, void *cbArg);
static Widget createLineColorMenu(Widget parent, GC drawGC, Drawable pmWindow,
    	int nColors, Pixel colors[], Pixmap pixmaps[], Widget btns[],
    	XtCallbackProc cbProc, void *cbArg);
static Widget createFillColorMenu(Widget parent, GC drawGC, Drawable pmWindow,
    	int nColors, Pixel colors[], Pixmap pixmaps[], Widget btns[],
    	XtCallbackProc cbProc, void *cbArg);
static void stylesOKCB(XYCurve *curves, int, void *resultFlag);
static void stylesCancelCB(XYCurve *curves, int, void *resultFlag);
static void updateMarkLineDialog(markDialog *dialog);
static void updateHistDialog(histStyleDialog *dialog);
static void markOptMenuCB(Widget w, markDialog *dialog, caddr_t callData);
static void histOptMenuCB(Widget w, histStyleDialog *dialog, caddr_t callData);
static void clearLinesCB(Widget w, markDialog *dialog, caddr_t callData);
static void clearMarksCB(Widget w, markDialog *dialog, caddr_t callData);
static void defaultLinesCB(Widget w, markDialog *dialog, caddr_t callData);
static void defaultMarksCB(Widget w, markDialog *dialog, caddr_t callData);
static void copyColorsCB(Widget w, markDialog *dialog, caddr_t callData);
static void markLineOkCB(Widget w, markDialog *dialog, caddr_t callData);
static void histOkCB(Widget w, histStyleDialog *dialog, caddr_t callData);
static void markLineApplyCB(Widget w, markDialog *dialog, caddr_t callData);
static void histApplyCB(Widget w, histStyleDialog *dialog, caddr_t callData);
static void markLineDismissCB(Widget w, markDialog *dialog, caddr_t callData);
static void histDismissCB(Widget w, histStyleDialog *dialog, caddr_t callData);
static void markLineDestroyCB(Widget w, markDialog *dialog, caddr_t callData);
static void histDestroyCB(Widget w, histStyleDialog *dialog, caddr_t callData);
static void copyCurveStyles(XYCurve *fromCurves, XYCurve *toCurves, int nCurves);
static void copyHistStyles(XYHistogram *fromHists, XYHistogram *toHists,
    	int nHists);
static void updateMarkLineSample(markDialog *dialog, int index);
static void updateHistSample(histStyleDialog *dialog, int index);
static void updateMarkTimerProc(XtPointer clientData, XtIntervalId *id);
static void updateHistTimerProc(XtPointer clientData, XtIntervalId *id);
static void markSizeCB(Widget w, markDialog *dialog, caddr_t callData);
static void createMarkSizeDialog(markDialog *dialog);
static void sizeOKCB(Widget w, markDialog *dialog, caddr_t callData);
static void sizeCancelCB(Widget w, markDialog *dialog, caddr_t callData);
static int findinList(Pixel *list, int numItems, int value);
static void allocNewFreePrevious(Display *display, int screen_num,
    	Pixel *color, int nColors, XColor color_def[XY_N_COLORS],
    	Boolean color_used[XY_N_COLORS], int newColorIndex, Pixel prevPixel);
static int get_colors(Display *display, int screen_num, char *progname, 
	int max_num_colors_ret, Pixel *color_pixel, XColor *exact_def);
static int findFillIndex(unsigned fillStyle);

/*
** XYEditCurveStyles
**
** Prompt user for marker and line styles for a set of curves for display
** by the XY widget.  This is a simpler version of XYCreateCurveStylesDialog,
** which presents the same dialog, except with no Apply button.  The routine
** returns when the user has selected a new set of styles for the curves,
** and alters the array curves to reflect the user's changes.
**
** Parameters
**
**	parent		Parent widget for dialog
**	curves		Array of XYCurve data structures containing style
**			information to alter.  XYEditStyles modifies
**			this data to match the style information selected
**			by the user.
**	nCurves		The number of curves data structures in curves above.
**
** Returns True if styles were changed
*/
int XYEditCurveStyles(Widget parent, XYCurve *curves, int nCurves)
{
    int resultFlag = NO_RESULT;
    Widget dialog;
    
    dialog = XYCreateCurveStylesDialog(parent, curves, nCurves, stylesOKCB,
    	    &resultFlag, NULL, NULL, stylesCancelCB, &resultFlag);
    while (resultFlag == 0)
        XtAppProcessEvent(XtWidgetToApplicationContext(dialog), XtIMAll);
    
    return resultFlag == RESULT_OK;
}

/*
** XYEditHistogramStyles
**
** Prompt user for marker and line styles for a set of curves for display
** by the XY widget.  This is a simpler version of XYCreateCurveStylesDialog,
** which presents the same dialog, except with no Apply button.  The routine
** returns when the user has selected a new set of styles for the curves,
** and alters the array curves to reflect the user's changes.
**
** Parameters
**
**	parent		Parent widget for dialog
**	histograms	Array of XYHistogram data structures containing style
**			information to alter.  XYEditHistogramStyles modifies
**			this data to match the style information selected
**			by the user.
**	nHistograms	The number of histogram data structures in curves above.
**
** Returns True if styles were changed
*/
int XYEditHistogramStyles(Widget parent, XYHistogram *histograms,
    	int nHistograms)
{
    int resultFlag = NO_RESULT;
    Widget dialog;
    
    dialog = XYCreateHistogramStylesDialog(parent, histograms, nHistograms,
    	    (XYHistCallbackProc)stylesOKCB, &resultFlag, NULL, NULL,
    	    (XYHistCallbackProc)stylesCancelCB, &resultFlag);
    while (resultFlag == 0)
        XtAppProcessEvent(XtWidgetToApplicationContext(dialog), XtIMAll);
    
    return resultFlag == RESULT_OK;
}

static void stylesOKCB(XYCurve *curves, int nCurves, void *resultFlag)
{
    *(int *)resultFlag = RESULT_OK;
}
static void stylesCancelCB(XYCurve *curves, int nCurves, void *resultFlag)
{
    *(int *)resultFlag = RESULT_DISMISS;
}

/*
** XYCreateCurveStylesDialog
**
** Display a dialog for setting marker and line styles for a set of curves
** for display by the XY widget.  Returns immediately but leaves up the
** dialog with callbacks attached to the ok, apply, and dismiss buttons.
** When the user presses the OK or Apply buttons, the original set of curves
** structures passed to XYCreateCurveStylesDialog is altered and passed back to
** the apply or ok callback routine.
** 
** The dialog may be popped down by the caller by destroying the widget
** returned by this routine.  Since the ok and dismiss buttons automatically
** handle popping down the dialog, this is usually not necessary.
**
** Parameters
**
**	parent		Parent widget for dialog
**	curves		Array of XYCurves data structures containing style
**			information to alter.  XYCreateCurveStylesDialog
**			modifies this data when the user makes his selections
**			and presses ok or apply, therefore, this must data
**			must be preserved after the call to this routine,
**			until the styles dialog is popped down.
**	nCurves		The number of curves data structures in curves above.
**	okCallback	Routine to call when the user presses OK in the
**			dialog.  Signals that the dialog has been removed
**			and set of curves has been altered with the new
**			style information.
**	okArg		Arbitrary data to pass to the ok callback routine.
**	applyCallback	Routine to call when the user presses Apply in the
**			styles dialog.  Signals that the data in the curves
**			array has been altered, but the dialog is still up.
**			If this is passed as NULL, no apply button is
**			displayed.
**	applyArg	Arbitrary data to pass to the apply callback routine.
**	dismissCallback	Routine to call when the user presses Dismiss in the
**			styles dialog.  Signals that the dialog has been
**			popped down without altering the curve structures.
**	dismissArg	Arbitrary data to pass to the dismiss callback routine.
*/
Widget XYCreateCurveStylesDialog(Widget parent, XYCurve *curves, int nCurves,
	XYMarkLineCallbackProc okCallback, void *okArg,
	XYMarkLineCallbackProc applyCallback, void *applyArg,
	XYMarkLineCallbackProc dismissCallback, void *dismissArg)
{
    Widget form, lineMenu, markMenu, sizeMenu, clearMarks, clearLines, sep;
    Widget mkColorMenu, lnColorMenu, formB;
    Widget defaultMarks, defaultLines, markSize, ok, apply, dismiss;
    Widget lineOptMenu, markOptMenu, sizeOptMenu, sampleLabel, name, name1;
    Widget mkColorOptMenu, lnColorOptMenu, bottomWidget, copyColors;
    XtArgVal bottomAttach;
    int i, n, j;
    Arg args[12];
    markDialog *dialog;
    Display *display = XtDisplay(parent);
    int depth = DefaultDepthOfScreen(XtScreen(parent));
    int screen_num = XScreenNumberOfScreen(XtScreen(parent));
    int hasApply = applyCallback != NULL;
    Pixel parentBGColor;
    GC drawGC;
    Pixmap pm;
    XmString s1; 
    
    /* Create a data structure for maintaining dialog context and passing
       information to the callbacks.  All of this is freed in the destroy
       callback for the dialog */
    dialog = (markDialog *)XtMalloc(sizeof(markDialog));
    dialog->origCurves = curves;
    dialog->curves = (XYCurve *)XtMalloc(sizeof(XYCurve)*nCurves);
    memcpy(dialog->curves, curves, sizeof(XYCurve)*nCurves);
    dialog->nCurves = nCurves;
    dialog->lineOptMenus = (Widget *)XtMalloc(sizeof(Widget)*nCurves);
    dialog->markOptMenus = (Widget *)XtMalloc(sizeof(Widget)*nCurves);
    dialog->sizeOptMenus = (Widget *)XtMalloc(sizeof(Widget)*nCurves);
    dialog->lnColorOptMenus = (Widget *)XtMalloc(sizeof(Widget)*nCurves);
    dialog->mkColorOptMenus = (Widget *)XtMalloc(sizeof(Widget)*nCurves);
    dialog->samplePixmaps = (Pixmap *)XtMalloc(sizeof(Pixmap)*nCurves);
    dialog->sampleLabels = (Widget *)XtMalloc(sizeof(Widget)*nCurves);
    dialog->gc = drawGC = XCreateGC(display, XtWindow(parent), 0, NULL);
    dialog->okCB = okCallback;
    dialog->applyCB = applyCallback;
    dialog->dismissCB = dismissCallback;
    dialog->okArg = okArg;
    dialog->applyArg = applyArg;
    dialog->dismissArg = dismissArg;
    
    /* Create a shell containing a form widget in which to build the dialog */
    n = 0;
    XtSetArg(args[n], XmNtitle, "Mark and Line Styles"); n++;
    XtSetArg(args[n], XmNautoUnmanage, False); n++;
    form = XmCreateFormDialog(parent, "XYMarkLineDialog", args, n);
    XtVaSetValues(form, XmNshadowThickness, 0, 0);
    dialog->form = form;
    XtAddCallback(XtParent(form), XmNdestroyCallback,
    	(XtCallbackProc)markLineDestroyCB, dialog);
    
    /* Check how many of our color choices the screen can handle, and
       allocate colors for displaying the choices in the dialog */
    dialog->nColors = get_colors(display, screen_num, "XYDialogs", 
    	XY_N_COLORS, dialog->color, dialog->color_def);
    for (i=0; i<XY_N_COLORS; i++) 
    	dialog->color_used[i] = FALSE;

    /* Create buttons at the bottom of the dialog (ok, apply, cance/dismiss) */
    ok = XtVaCreateManagedWidget("ok", xmPushButtonGadgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple("OK"),
    	    XmNbottomAttachment, XmATTACH_FORM,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, hasApply ? 4 : 20,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, hasApply ? 30 : 40,
    	    XmNbottomOffset, 5, 0);
    XmStringFree(s1);
    XtAddCallback(ok, XmNactivateCallback, (XtCallbackProc)markLineOkCB, dialog);
    XtVaSetValues(form, XmNdefaultButton, ok, 0);
    if (hasApply) {
	apply = XtVaCreateManagedWidget("apply", xmPushButtonGadgetClass, form,
    		XmNlabelString, s1=XmStringCreateSimple("Apply"),
    		XmNbottomAttachment, XmATTACH_FORM,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, 37,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, 63,
    		XmNbottomOffset, 5, 0);
	XmStringFree(s1);
	XtAddCallback(apply, XmNactivateCallback,
	    	(XtCallbackProc)markLineApplyCB, dialog);
    }
    dismiss = XtVaCreateManagedWidget("dismiss", xmPushButtonGadgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple(
    	    	    hasApply ? "Dismiss" : "Cancel"),
    	    XmNbottomAttachment, XmATTACH_FORM,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, hasApply ? 70 : 60,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, hasApply ? 96 : 80,
    	    XmNbottomOffset, 5, 0);
    XmStringFree(s1);
    XtAddCallback(dismiss, XmNactivateCallback,
    	    (XtCallbackProc)markLineDismissCB, dialog);
    sep = XtVaCreateManagedWidget("sep", xmSeparatorGadgetClass, form,
    	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNbottomWidget, ok,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 0,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 100,
    	    XmNbottomOffset, 9, 0);

    /* Create convenience buttons for modifying groups of options: 
       Clear Lines, Clear Marks, Default LineStyles, Default MarkStyles,
       LineColors -> Marks, Mark Size */
    bottomAttach = XmATTACH_WIDGET;
    bottomWidget = sep;
    clearMarks = XtVaCreateManagedWidget("clearMarks",
    	    xmPushButtonGadgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple("Clear Marks"),
    	    XmNbottomAttachment, bottomAttach,
    	    XmNbottomWidget, bottomWidget,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 1,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 33,
    	    XmNbottomOffset, 6, 0);
    XmStringFree(s1);
    XtAddCallback(clearMarks, XmNactivateCallback,
    	    (XtCallbackProc)clearMarksCB, dialog);
    defaultMarks = XtVaCreateManagedWidget("defaultMarks",
    	    xmPushButtonGadgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple(dialog->nColors > 0
    	    			? "Default MarkStyles" : "Default Marks"),
    	    XmNbottomAttachment, bottomAttach,
    	    XmNbottomWidget, bottomWidget,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 34,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 66,
    	    XmNbottomOffset, 6, 0);
    XmStringFree(s1);
    XtAddCallback(defaultMarks, XmNactivateCallback,
    	    (XtCallbackProc)defaultMarksCB, dialog);
    markSize = XtVaCreateManagedWidget("markSize", xmPushButtonGadgetClass,form,
    	    XmNlabelString, s1=XmStringCreateSimple("Mark Size"),
    	    XmNbottomAttachment, bottomAttach,
    	    XmNbottomWidget, bottomWidget,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 67,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 99,
    	    XmNbottomOffset, 6, 0);
    XmStringFree(s1);
    XtAddCallback(markSize, XmNactivateCallback, (XtCallbackProc)markSizeCB,
    	    dialog);
    clearLines = XtVaCreateManagedWidget("clearLines",
    	    xmPushButtonGadgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple("Clear Lines"),
    	    XmNdefaultButtonShadowThickness, 0,
    	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNbottomWidget, clearMarks,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 1,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 33, 0);
    XmStringFree(s1);
    XtAddCallback(clearLines, XmNactivateCallback, (XtCallbackProc)clearLinesCB,
    	    dialog);
    defaultLines = XtVaCreateManagedWidget("defaultLines",
    	    xmPushButtonGadgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple(dialog->nColors > 0
    	    			? "Default LineStyles" : "Default Lines"),
    	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNbottomWidget, clearMarks,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 34,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 66, 0);
    XmStringFree(s1);
    XtAddCallback(defaultLines, XmNactivateCallback,
    	    (XtCallbackProc)defaultLinesCB, dialog);
    if (dialog->nColors > 0) {
	copyColors = XtVaCreateManagedWidget("copyColors",
    		xmPushButtonGadgetClass, form,
    		XmNlabelString, s1=XmStringCreateSimple("LineColors -> Marks"),
    		XmNbottomAttachment, XmATTACH_WIDGET,
    		XmNbottomWidget, clearMarks,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, 67,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, 99, 0);
	XmStringFree(s1);
	XtAddCallback(copyColors, XmNactivateCallback,
    		(XtCallbackProc)copyColorsCB, dialog);
    }

    /* Create common menus and pixmaps for attachment to the multitude of
       option menus created below */
    lineMenu = createLineStyleMenu(form, drawGC, XtWindow(parent),
    	    dialog->linePixmaps, dialog->lineBtns,
    	    (XtCallbackProc)markOptMenuCB, dialog);
    markMenu = createMarkStyleMenu(form, drawGC, XtWindow(parent),
    	    dialog->markPixmaps, dialog->markBtns,
    	    (XtCallbackProc)markOptMenuCB, dialog);
    sizeMenu = createMarkSizeMenu(form, drawGC, XtWindow(parent),
    	    dialog->sizePixmaps, dialog->sizeBtns,
    	    (XtCallbackProc)markOptMenuCB, dialog);
    if (dialog->nColors > 0) {
    	mkColorMenu = createMarkColorMenu(form, drawGC, XtWindow(parent),
    		dialog->nColors, dialog->color, dialog->mkColorPixmaps,
    		dialog->markColorBtns, (XtCallbackProc)markOptMenuCB, dialog);
    	lnColorMenu = createLineColorMenu(form, drawGC, XtWindow(parent),
    		dialog->nColors, dialog->color, dialog->lnColorPixmaps,
    		dialog->lineColorBtns, (XtCallbackProc)markOptMenuCB, dialog);
    }
    
    /* Create the grid of option menus */
    bottomWidget = clearLines;
    bottomAttach = XmATTACH_WIDGET;
    for (i=nCurves-1; i>=0; i--) {

	n = 0;
	XtSetArg(args[n], XmNspacing, 0); n++;
	XtSetArg(args[n], XmNmarginWidth, 0); n++;
	XtSetArg(args[n], XmNbottomAttachment, bottomAttach); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNbottomWidget, bottomWidget); n++;
	XtSetArg(args[n], XmNsubMenuId, lineMenu); n++;
	XtSetArg(args[n], XmNmenuHistory,
		dialog->lineBtns[curves[i].lineStyle]); n++;
	lineOptMenu = XmCreateOptionMenu(form, "lineOptMenu", args, n);
	XtManageChild(lineOptMenu);
	dialog->lineOptMenus[i] = lineOptMenu;
	XtVaSetValues(XmOptionButtonGadget(lineOptMenu), XmNmarginHeight, 0, 0);
	
	if (dialog->nColors > 0) {
	    n = 0;
	    XtSetArg(args[n], XmNspacing, 0); n++;
	    XtSetArg(args[n], XmNmarginWidth, 0); n++;
	    XtSetArg(args[n], XmNbottomAttachment, bottomAttach); n++;
	    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg(args[n], XmNbottomWidget, bottomWidget); n++;
	    XtSetArg(args[n], XmNleftWidget, lineOptMenu); n++;
	    XtSetArg(args[n], XmNsubMenuId, lnColorMenu); n++;
	    if ((j = findinList(dialog->color, dialog->nColors,
	    	    curves[i].linePixel)) >= 0) {
 	        /* printf("Line Button %d: Found pixel value %d as index %d\n",
	            i, curves[i].linePixel, j);  */
	        XtSetArg(args[n], XmNmenuHistory, dialog->lineColorBtns[j]); 
	        n++;
	    }
	    lnColorOptMenu = XmCreateOptionMenu(form, "lnColorOptMenu", args,n);
	    dialog->lnColorOptMenus[i] = lnColorOptMenu;
	    XtManageChild(lnColorOptMenu);
	    XtVaSetValues(XmOptionButtonGadget(lnColorOptMenu), XmNmarginHeight,
	    	    0, 0);
	}

	n = 0;
	XtSetArg(args[n], XmNspacing, 0); n++;
	XtSetArg(args[n], XmNmarginWidth, 0); n++;
	XtSetArg(args[n], XmNbottomAttachment, bottomAttach); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNbottomWidget, bottomWidget); n++;
	XtSetArg(args[n], XmNleftWidget, dialog->nColors > 0 
						      ? lnColorOptMenu 
    						      : lineOptMenu); n++;
	XtSetArg(args[n], XmNsubMenuId, markMenu); n++;
	XtSetArg(args[n], XmNmenuHistory,
		dialog->markBtns[curves[i].markerStyle]); n++;
	markOptMenu = XmCreateOptionMenu(form, "markOptMenu", args, n);
	dialog->markOptMenus[i] = markOptMenu;
	XtManageChild(markOptMenu);
	XtVaSetValues(XmOptionButtonGadget(markOptMenu), XmNmarginHeight, 0, 0);
	
	n = 0;
	XtSetArg(args[n], XmNspacing, 0); n++;
	XtSetArg(args[n], XmNmarginWidth, 0); n++;
	XtSetArg(args[n], XmNbottomAttachment, bottomAttach); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNbottomWidget, bottomWidget); n++;
	XtSetArg(args[n], XmNleftWidget, markOptMenu); n++;
	XtSetArg(args[n], XmNsubMenuId, sizeMenu); n++;
	XtSetArg(args[n], XmNmenuHistory,
		dialog->sizeBtns[curves[i].markerSize]); n++;
	sizeOptMenu = XmCreateOptionMenu(form, "sizeOptMenu", args, n);
	dialog->sizeOptMenus[i] = sizeOptMenu;
	XtManageChild(sizeOptMenu);
	XtVaSetValues(XmOptionButtonGadget(sizeOptMenu), XmNmarginHeight, 0, 0);

	if (dialog->nColors > 0) {
	    n = 0;
	    XtSetArg(args[n], XmNspacing, 0); n++;
	    XtSetArg(args[n], XmNmarginWidth, 0); n++;
	    XtSetArg(args[n], XmNbottomAttachment, bottomAttach); n++;
	    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg(args[n], XmNbottomWidget, bottomWidget); n++;
	    XtSetArg(args[n], XmNleftWidget, sizeOptMenu); n++;
	    XtSetArg(args[n], XmNsubMenuId, mkColorMenu); n++;
	    if ((j = findinList(dialog->color, dialog->nColors,
	    	    curves[i].markerPixel)) >= 0) {
 	        /*printf("Marker Button %d: Found pixel value %d as index %d\n",
	            i, curves[i].markerPixel, j); */
	        XtSetArg(args[n], XmNmenuHistory, dialog->markColorBtns[j]);
	        n++;
	    }
	    mkColorOptMenu = XmCreateOptionMenu(form, "mkColorOptMenu", args,n);
	    dialog->mkColorOptMenus[i] = mkColorOptMenu;
	    XtManageChild(mkColorOptMenu);
	    XtVaSetValues(XmOptionButtonGadget(mkColorOptMenu), XmNmarginHeight,
	    	    0, 0);
	}
    	
    	bottomAttach = XmATTACH_WIDGET;
    	bottomWidget = lineOptMenu;
    }
    
    /* Create the sample area */
    XtVaGetValues(parent, XmNbackground, &parentBGColor, 0);
    /* printf("Parent background color = %d\n", parentBGColor); */
    formB = XtVaCreateManagedWidget("formB",
    	    	xmFormWidgetClass, form,	
    		XmNbackground, parentBGColor,
    		XmNleftAttachment, XmATTACH_WIDGET,
    		XmNleftWidget, dialog->nColors > 0 ? mkColorOptMenu 
    						      : sizeOptMenu,
    		XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    		XmNbottomWidget, dialog->markOptMenus[nCurves-1],
    		XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    		XmNtopWidget, dialog->markOptMenus[0],  
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, 99,
    		XmNleftOffset, 4,
    		XmNtopOffset, 4, 
    		XmNbottomOffset, 4, 0);
    for (i=0; i<nCurves; i++) {
        pm = XCreatePixmap(display, XtWindow(parent), 52, 12, depth);
        dialog->samplePixmaps[i] = pm;
        sampleLabel = XtVaCreateManagedWidget("sample",
        	xmLabelGadgetClass, formB,
    		XmNlabelType, XmPIXMAP,
    		XmNlabelPixmap, pm,
    		XmNtopAttachment, XmATTACH_POSITION,
    		XmNtopPosition, (i*100)/nCurves,
    		XmNtopOffset, i+5,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, 1, 
    		XmNbottomOffset, 10, 0);
	dialog->sampleLabels[i] = sampleLabel;
        updateMarkLineSample(dialog, i);
	name = XtVaCreateManagedWidget("name", xmLabelGadgetClass, formB,
    		XmNlabelString, curves[i].name,
    		XmNalignment, XmALIGNMENT_BEGINNING,
    		XmNtopAttachment, XmATTACH_POSITION,
    		XmNtopPosition, (i*100)/nCurves,
    		XmNtopOffset, i+5,
    		XmNleftAttachment, XmATTACH_WIDGET,
    		XmNleftWidget, sampleLabel,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, 99, 0);
    }
    
    /* Create the column titles */
    name = XtVaCreateManagedWidget("name", xmLabelGadgetClass, form,
    		XmNlabelString, s1=XmStringCreateSimple("Style"),
    		XmNalignment, XmALIGNMENT_CENTER,
    		XmNbottomAttachment, bottomAttach,
		XmNbottomWidget, bottomWidget,
    		XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET, 
    		XmNleftWidget, lineOptMenu, 
    		XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET, 
    		XmNrightWidget, lineOptMenu, 0);
    XmStringFree(s1);
    if (dialog->nColors > 0) {
	name = XtVaCreateManagedWidget("name", xmLabelGadgetClass, form,
    		    XmNlabelString, s1=XmStringCreateSimple("Color"),
    		    XmNalignment, XmALIGNMENT_CENTER,
    		    XmNbottomAttachment, bottomAttach,
		    XmNbottomWidget, bottomWidget,
    		    XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET, 
    		    XmNleftWidget, lnColorOptMenu, 
    		    XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET, 
    		    XmNrightWidget, lnColorOptMenu, 0);
    	XmStringFree(s1);
    }
    name = XtVaCreateManagedWidget("name", xmLabelGadgetClass, form,
    		XmNlabelString, s1=XmStringCreateSimple("Style"),
    		XmNalignment, XmALIGNMENT_CENTER,
    		XmNbottomAttachment, bottomAttach,
		XmNbottomWidget, bottomWidget,
    		XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET, 
    		XmNleftWidget, markOptMenu, 
    		XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET, 
    		XmNrightWidget, markOptMenu, 0);
    XmStringFree(s1);
    name = XtVaCreateManagedWidget("name", xmLabelGadgetClass, form,
    		XmNlabelString, s1=XmStringCreateSimple("Size"),
    		XmNalignment, XmALIGNMENT_CENTER,
    		XmNbottomAttachment, bottomAttach,
		XmNbottomWidget, bottomWidget,
    		XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET, 
    		XmNleftWidget, sizeOptMenu, 
    		XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET, 
    		XmNrightWidget, sizeOptMenu, 0);
    XmStringFree(s1);
    if (dialog->nColors > 0) {
	name = XtVaCreateManagedWidget("name", xmLabelGadgetClass, form,
    		    XmNlabelString, s1=XmStringCreateSimple("Color"),
    		    XmNalignment, XmALIGNMENT_CENTER,
    		    XmNbottomAttachment, bottomAttach,
		    XmNbottomWidget, bottomWidget,
    		    XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET, 
    		    XmNleftWidget, mkColorOptMenu, 
    		    XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET, 
    		    XmNrightWidget, mkColorOptMenu, 0);
    	XmStringFree(s1);
    }
    name1 = XtVaCreateManagedWidget("name", xmLabelGadgetClass, form,
    		XmNlabelString, s1=XmStringCreateSimple("Mark"),
    		XmNalignment, XmALIGNMENT_CENTER,
    		XmNbottomAttachment, XmATTACH_WIDGET,
		XmNbottomWidget, name,
    		XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET, 
    		XmNleftWidget, markOptMenu, 
    		XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET, 
    		XmNrightWidget, dialog->nColors > 0	? mkColorOptMenu
    							: sizeOptMenu, 0);
    XmStringFree(s1);
    name1 = XtVaCreateManagedWidget("name", xmLabelGadgetClass, form,
    		XmNlabelString, s1=XmStringCreateSimple("Line"),
    		XmNalignment, XmALIGNMENT_CENTER,
    		XmNbottomAttachment, XmATTACH_WIDGET,
		XmNbottomWidget, name,
    		XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET, 
    		XmNleftWidget, lineOptMenu, 
    		XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET, 
    		XmNrightWidget, dialog->nColors > 0	? lnColorOptMenu
    							: lineOptMenu, 0);
    XmStringFree(s1);
    
    /* stop the specialty buttons from showing the default button shadow */
    XtVaSetValues(clearMarks, XmNdefaultButtonShadowThickness, 0, 0);
    XtVaSetValues(clearLines, XmNdefaultButtonShadowThickness, 0, 0);
    XtVaSetValues(defaultMarks, XmNdefaultButtonShadowThickness, 0, 0);
    XtVaSetValues(defaultLines, XmNdefaultButtonShadowThickness, 0, 0);
    XtVaSetValues(markSize, XmNdefaultButtonShadowThickness, 0, 0);
    if (dialog->nColors > 0)
	XtVaSetValues(copyColors, XmNdefaultButtonShadowThickness, 0, 0);
    
    XtManageChild(form);
    return XtParent(form);
}

/*
** XYCreateHistogramStylesDialog
**
** Display a dialog for line styles fill styles for a set of histograms for
** display by the XY widget.  Returns immediately but leaves up the dialog
** with callbacks attached to the ok, apply, and dismiss buttons. When the
** user presses the OK or Apply buttons, the original set of curves
** structures passed to XYCreateHistogramStylesDialog is altered and passed
** back to the apply or ok callback routine.
** 
** The dialog may be popped down by the caller by destroying the widget
** returned by this routine.  Since the ok and dismiss buttons automatically
** handle popping down the dialog, this is usually not necessary.
**
** Parameters
**
**	parent		Parent widget for dialog
**	histograms	Array of XYHistogram data structures containing style
**			information to alter.  XYCreateHistogramStylesDialog
**			modifies this data when the user makes his selections
**			and presses ok or apply, therefore, this must data
**			must be preserved after the call to this routine,
**			until the styles dialog is popped down.
**	nHistograms	The number of data structures in histograms above.
**	okCallback	Routine to call when the user presses OK in the
**			dialog.  Signals that the dialog has been removed
**			and set of histograms has been altered with the new
**			style information.
**	okArg		Arbitrary data to pass to the ok callback routine.
**	applyCallback	Routine to call when the user presses Apply in the
**			styles dialog.  Signals that the data in the
**			histograms array has been altered, but the dialog is
**			still up. If this is passed as NULL, no apply button
**			is displayed.
**	applyArg	Arbitrary data to pass to the apply callback routine.
**	dismissCallback	Routine to call when the user presses Dismiss in the
**			styles dialog.  Signals that the dialog has been
**			popped down without altering the histogram structures.
**	dismissArg	Arbitrary data to pass to the dismiss callback routine.
*/
Widget XYCreateHistogramStylesDialog(Widget parent, XYHistogram *histograms,
    	int nHistograms, XYHistCallbackProc okCallback, void *okArg,
    	XYHistCallbackProc applyCallback, void *applyArg,
    	XYHistCallbackProc dismissCallback, void *dismissArg)
{
    Widget form, lineMenu, fillMenu, sep;
    Widget fillColorMenu, lineColorMenu, formB;
    Widget ok, apply, dismiss;
    Widget lineOptMenu, fillOptMenu, sampleLabel, name, name1;
    Widget fillColorOptMenu, lineColorOptMenu, bottomWidget, copyColors;
    XtArgVal bottomAttach;
    int i, n, j;
    Arg args[12];
    histStyleDialog *dialog;
    Display *display = XtDisplay(parent);
    int depth = DefaultDepthOfScreen(XtScreen(parent));
    int screen_num = XScreenNumberOfScreen(XtScreen(parent));
    int hasApply = applyCallback != NULL;
    Pixel parentBGColor;
    GC drawGC;
    Pixmap pm;
    XmString s1; 
    
    /* Create a data structure for maintaining dialog context and passing
       information to the callbacks.  All of this is freed in the destroy
       callback for the dialog */
    dialog = (histStyleDialog *)XtMalloc(sizeof(histStyleDialog));
    dialog->origHists = histograms;
    dialog->hists = (XYHistogram *)XtMalloc(sizeof(XYHistogram)*nHistograms);
    memcpy(dialog->hists, histograms, sizeof(XYHistogram)*nHistograms);
    dialog->nHists = nHistograms;
    dialog->lineOptMenus = (Widget *)XtMalloc(sizeof(Widget)*nHistograms);
    dialog->fillOptMenus = (Widget *)XtMalloc(sizeof(Widget)*nHistograms);
    dialog->lineColorOptMenus = (Widget *)XtMalloc(sizeof(Widget)*nHistograms);
    dialog->fillColorOptMenus = (Widget *)XtMalloc(sizeof(Widget)*nHistograms);
    dialog->samplePixmaps = (Pixmap *)XtMalloc(sizeof(Pixmap)*nHistograms);
    dialog->sampleLabels = (Widget *)XtMalloc(sizeof(Widget)*nHistograms);
    dialog->gc = drawGC = XCreateGC(display, XtWindow(parent), 0, NULL);
    dialog->okCB = okCallback;
    dialog->applyCB = applyCallback;
    dialog->dismissCB = dismissCallback;
    dialog->okArg = okArg;
    dialog->applyArg = applyArg;
    dialog->dismissArg = dismissArg;
    
    /* Create a shell containing a form widget in which to build the dialog */
    n = 0;
    XtSetArg(args[n], XmNtitle, "Histogram Style"); n++;
    XtSetArg(args[n], XmNautoUnmanage, False); n++;
    form = XmCreateFormDialog(parent, "XYHistStyleDialog", args, n);
    XtVaSetValues(form, XmNshadowThickness, 0, 0);
    dialog->form = form;
    XtAddCallback(XtParent(form), XmNdestroyCallback,
    	(XtCallbackProc)histDestroyCB, dialog);
    
    /* Check how many of our color choices the screen can handle, and
       allocate colors for displaying the choices in the dialog */
    dialog->nColors = get_colors(display, screen_num, "XYDialogs", 
    	XY_N_COLORS, dialog->color, dialog->color_def);
    for (i=0; i<XY_N_COLORS; i++) 
    	dialog->color_used[i] = FALSE;

    /* Create buttons at the bottom of the dialog (ok, apply, cance/dismiss) */
    ok = XtVaCreateManagedWidget("ok", xmPushButtonGadgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple("OK"),
    	    XmNbottomAttachment, XmATTACH_FORM,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, hasApply ? 4 : 20,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, hasApply ? 30 : 40,
    	    XmNbottomOffset, 5, 0);
    XmStringFree(s1);
    XtAddCallback(ok, XmNactivateCallback, (XtCallbackProc)histOkCB, dialog);
    XtVaSetValues(form, XmNdefaultButton, ok, 0);
    if (hasApply) {
	apply = XtVaCreateManagedWidget("apply", xmPushButtonGadgetClass, form,
    		XmNlabelString, s1=XmStringCreateSimple("Apply"),
    		XmNbottomAttachment, XmATTACH_FORM,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, 37,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, 63,
    		XmNbottomOffset, 5, 0);
	XmStringFree(s1);
	XtAddCallback(apply, XmNactivateCallback, (XtCallbackProc)histApplyCB,
		dialog);
    }
    dismiss = XtVaCreateManagedWidget("dismiss", xmPushButtonGadgetClass, form,
    	    XmNlabelString, s1=XmStringCreateSimple(
    	    	    hasApply ? "Dismiss" : "Cancel"),
    	    XmNbottomAttachment, XmATTACH_FORM,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, hasApply ? 70 : 60,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, hasApply ? 96 : 80,
    	    XmNbottomOffset, 5, 0);
    XmStringFree(s1);
    XtAddCallback(dismiss, XmNactivateCallback, (XtCallbackProc)histDismissCB,
    	    dialog);
    sep = XtVaCreateManagedWidget("sep", xmSeparatorGadgetClass, form,
    	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNbottomWidget, ok,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 0,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 100,
    	    XmNbottomOffset, 9, 0);

    /* Create common menus and pixmaps for attachment to the multitude of
       option menus created below */
    lineMenu = createHistLineStyleMenu(form, drawGC, XtWindow(parent),
    	    dialog->linePixmaps, dialog->lineBtns,
    	    (XtCallbackProc)histOptMenuCB, dialog);
    fillMenu = createFillStyleMenu(form, drawGC, XtWindow(parent),
    	    dialog->fillPixmaps, dialog->fillBtns,
    	    (XtCallbackProc)histOptMenuCB, dialog);
    if (dialog->nColors > 0) {
    	fillColorMenu = createFillColorMenu(form, drawGC, XtWindow(parent),
    		dialog->nColors, dialog->color, dialog->fillColorPixmaps,
    		dialog->fillColorBtns, (XtCallbackProc)histOptMenuCB, dialog);
    	lineColorMenu = createLineColorMenu(form, drawGC, XtWindow(parent),
    		dialog->nColors, dialog->color, dialog->lineColorPixmaps,
    		dialog->lineColorBtns, (XtCallbackProc)histOptMenuCB, dialog);
    }
    
    /* Create the grid of option menus */
    bottomWidget = sep;
    bottomAttach = XmATTACH_WIDGET;
    for (i=nHistograms-1; i>=0; i--) {

	n = 0;
	XtSetArg(args[n], XmNspacing, 0); n++;
	XtSetArg(args[n], XmNmarginWidth, 0); n++;
	XtSetArg(args[n], XmNbottomAttachment, bottomAttach); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNbottomWidget, bottomWidget); n++;
	XtSetArg(args[n], XmNsubMenuId, lineMenu); n++;
	XtSetArg(args[n], XmNmenuHistory,
		dialog->lineBtns[histograms[i].lineStyle]); n++;
	lineOptMenu = XmCreateOptionMenu(form, "lineOptMenu", args, n);
	XtManageChild(lineOptMenu);
	dialog->lineOptMenus[i] = lineOptMenu;
	XtVaSetValues(XmOptionButtonGadget(lineOptMenu), XmNmarginHeight, 0, 0);
	
	if (dialog->nColors > 0) {
	    n = 0;
	    XtSetArg(args[n], XmNspacing, 0); n++;
	    XtSetArg(args[n], XmNmarginWidth, 0); n++;
	    XtSetArg(args[n], XmNbottomAttachment, bottomAttach); n++;
	    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg(args[n], XmNbottomWidget, bottomWidget); n++;
	    XtSetArg(args[n], XmNleftWidget, lineOptMenu); n++;
	    XtSetArg(args[n], XmNsubMenuId, lineColorMenu); n++;
	    if ((j = findinList(dialog->color, dialog->nColors,
	    	    histograms[i].linePixel)) >= 0) {
 	        /* printf("Line Button %d: Found pixel value %d as index %d\n",
	            i, histograms[i].linePixel, j);  */
	        XtSetArg(args[n], XmNmenuHistory, dialog->lineColorBtns[j]); 
	        n++;
	    }
	    lineColorOptMenu = XmCreateOptionMenu(form, "lnColorOptMenu",
	    	    args, n);
	    dialog->lineColorOptMenus[i] = lineColorOptMenu;
	    XtManageChild(lineColorOptMenu);
	    XtVaSetValues(XmOptionButtonGadget(lineColorOptMenu),
	    	    XmNmarginHeight, 0, 0);
	}

	n = 0;
	XtSetArg(args[n], XmNspacing, 0); n++;
	XtSetArg(args[n], XmNmarginWidth, 0); n++;
	XtSetArg(args[n], XmNbottomAttachment, bottomAttach); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNbottomWidget, bottomWidget); n++;
	XtSetArg(args[n], XmNleftWidget, dialog->nColors > 0 
						      ? lineColorOptMenu 
    						      : lineOptMenu); n++;
	XtSetArg(args[n], XmNsubMenuId, fillMenu); n++;
	XtSetArg(args[n], XmNmenuHistory,
		dialog->fillBtns[findFillIndex(histograms[i].fillStyle)]); n++;
	fillOptMenu = XmCreateOptionMenu(form, "fillOptMenu", args, n);
	dialog->fillOptMenus[i] = fillOptMenu;
	XtManageChild(fillOptMenu);
	XtVaSetValues(XmOptionButtonGadget(fillOptMenu), XmNmarginHeight, 0, 0);

	if (dialog->nColors > 0) {
	    n = 0;
	    XtSetArg(args[n], XmNspacing, 0); n++;
	    XtSetArg(args[n], XmNmarginWidth, 0); n++;
	    XtSetArg(args[n], XmNbottomAttachment, bottomAttach); n++;
	    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg(args[n], XmNbottomWidget, bottomWidget); n++;
	    XtSetArg(args[n], XmNleftWidget, fillOptMenu); n++;
	    XtSetArg(args[n], XmNsubMenuId, fillColorMenu); n++;
	    if ((j = findinList(dialog->color, dialog->nColors,
	    	    histograms[i].fillPixel)) >= 0) {
	        XtSetArg(args[n], XmNmenuHistory, dialog->fillColorBtns[j]);
	        n++;
	    }
	    fillColorOptMenu = XmCreateOptionMenu(form, "fillColorOptMenu",
	    	    args, n);
	    dialog->fillColorOptMenus[i] = fillColorOptMenu;
	    XtManageChild(fillColorOptMenu);
	    XtVaSetValues(XmOptionButtonGadget(fillColorOptMenu), XmNmarginHeight,
	    	    0, 0);
	}
    	
    	bottomAttach = XmATTACH_WIDGET;
    	bottomWidget = lineOptMenu;
    }
    
    /* Create the sample area */
    XtVaGetValues(parent, XmNbackground, &parentBGColor, 0);
    /* printf("Parent background color = %d\n", parentBGColor); */
    formB = XtVaCreateManagedWidget("formB",
    	    	xmFormWidgetClass, form,	
    		XmNbackground, parentBGColor,
    		XmNleftAttachment, XmATTACH_WIDGET,
    		XmNleftWidget, dialog->nColors > 0 ? fillColorOptMenu 
    						      : fillOptMenu,
    		XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
    		XmNbottomWidget, dialog->fillOptMenus[nHistograms-1],
    		XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
    		XmNtopWidget, dialog->fillOptMenus[0],  
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, 99,
    		XmNleftOffset, 4,
    		XmNtopOffset, 4, 
    		XmNbottomOffset, 4, 0);
    for (i=0; i<nHistograms; i++) {
        pm = XCreatePixmap(display, XtWindow(parent), 52, 12, depth);
        dialog->samplePixmaps[i] = pm;
        sampleLabel = XtVaCreateManagedWidget("sample",
        	xmLabelGadgetClass, formB,
    		XmNlabelType, XmPIXMAP,
    		XmNlabelPixmap, pm,
    		XmNtopAttachment, XmATTACH_POSITION,
    		XmNtopPosition, (i*100)/nHistograms,
    		XmNtopOffset, i+5,
    		XmNleftAttachment, XmATTACH_POSITION,
    		XmNleftPosition, 1, 
    		XmNbottomOffset, 10, 0);
	dialog->sampleLabels[i] = sampleLabel;
        updateHistSample(dialog, i);
	name = XtVaCreateManagedWidget("name", xmLabelGadgetClass, formB,
    		XmNlabelString, histograms[i].name,
    		XmNalignment, XmALIGNMENT_BEGINNING,
    		XmNtopAttachment, XmATTACH_POSITION,
    		XmNtopPosition, (i*100)/nHistograms,
    		XmNtopOffset, i+5,
    		XmNleftAttachment, XmATTACH_WIDGET,
    		XmNleftWidget, sampleLabel,
    		XmNrightAttachment, XmATTACH_POSITION,
    		XmNrightPosition, 99, 0);
    }
    
    /* Create the column titles */
    name = XtVaCreateManagedWidget("name", xmLabelGadgetClass, form,
    		XmNlabelString, s1=XmStringCreateSimple("Style"),
    		XmNalignment, XmALIGNMENT_CENTER,
    		XmNbottomAttachment, bottomAttach,
		XmNbottomWidget, bottomWidget,
    		XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET, 
    		XmNleftWidget, lineOptMenu, 
    		XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET, 
    		XmNrightWidget, lineOptMenu, 0);
    XmStringFree(s1);
    if (dialog->nColors > 0) {
	name = XtVaCreateManagedWidget("name", xmLabelGadgetClass, form,
    		    XmNlabelString, s1=XmStringCreateSimple("Color"),
    		    XmNalignment, XmALIGNMENT_CENTER,
    		    XmNbottomAttachment, bottomAttach,
		    XmNbottomWidget, bottomWidget,
    		    XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET, 
    		    XmNleftWidget, lineColorOptMenu, 
    		    XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET, 
    		    XmNrightWidget, lineColorOptMenu, 0);
    	XmStringFree(s1);
    }
    name = XtVaCreateManagedWidget("name", xmLabelGadgetClass, form,
    		XmNlabelString, s1=XmStringCreateSimple("Style"),
    		XmNalignment, XmALIGNMENT_CENTER,
    		XmNbottomAttachment, bottomAttach,
		XmNbottomWidget, bottomWidget,
    		XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET, 
    		XmNleftWidget, fillOptMenu, 
    		XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET, 
    		XmNrightWidget, fillOptMenu, 0);
    XmStringFree(s1);
    if (dialog->nColors > 0) {
	name = XtVaCreateManagedWidget("name", xmLabelGadgetClass, form,
    		    XmNlabelString, s1=XmStringCreateSimple("Color"),
    		    XmNalignment, XmALIGNMENT_CENTER,
    		    XmNbottomAttachment, bottomAttach,
		    XmNbottomWidget, bottomWidget,
    		    XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET, 
    		    XmNleftWidget, fillColorOptMenu, 
    		    XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET, 
    		    XmNrightWidget, fillColorOptMenu, 0);
    	XmStringFree(s1);
    }
    name1 = XtVaCreateManagedWidget("name", xmLabelGadgetClass, form,
    		XmNlabelString, s1=XmStringCreateSimple("Fill"),
    		XmNalignment, XmALIGNMENT_CENTER,
    		XmNbottomAttachment, XmATTACH_WIDGET,
		XmNbottomWidget, name,
    		XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET, 
    		XmNleftWidget, fillOptMenu, 
    		XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET, 
    		XmNrightWidget, dialog->nColors > 0	? fillColorOptMenu
    							: fillOptMenu, 0);
    XmStringFree(s1);
    name1 = XtVaCreateManagedWidget("name", xmLabelGadgetClass, form,
    		XmNlabelString, s1=XmStringCreateSimple("Line"),
    		XmNalignment, XmALIGNMENT_CENTER,
    		XmNbottomAttachment, XmATTACH_WIDGET,
		XmNbottomWidget, name,
    		XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET, 
    		XmNleftWidget, lineOptMenu, 
    		XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET, 
    		XmNrightWidget, dialog->nColors > 0	? lineColorOptMenu
    							: lineOptMenu, 0);
    XmStringFree(s1);
    
    XtManageChild(form);
    return XtParent(form);
}

static Widget createLineStyleMenu(Widget parent, GC drawGC, Drawable pmWindow,
    	Pixmap pixmaps[XY_N_HIST_LINE_STYLES],
    	Widget btns[XY_N_HIST_LINE_STYLES], XtCallbackProc cbProc, void *cbArg)
{
    int depth = DefaultDepthOfScreen(XtScreen(parent));
    Display *display = XtDisplay(parent);
    Pixel bgColor, fgColor;
    Pixmap pm;
    int i;
    Widget menu;
    
    XtVaGetValues(parent, XmNbackground, &bgColor, 0);
    fgColor = BlackPixelOfScreen(XtScreen(parent));

    menu = XmCreatePulldownMenu(parent, "lineMenu", NULL, 0);
    for (i=0; i<XY_N_LINE_STYLES; i++) {
        pm = XCreatePixmap(display, pmWindow, 50, 12, depth);
        pixmaps[i] = pm;
        XSetForeground(display, drawGC, bgColor);
        XFillRectangle(display, pm, drawGC, 0, 0, 50, 12);
        XYDrawLine(display, pm, drawGC, i, fgColor, 5, 6, 45, 6);
        btns[i] = XtVaCreateManagedWidget("lineStyle",
        	xmPushButtonGadgetClass, menu,
    		XmNlabelType, XmPIXMAP,
    		XmNlabelPixmap, pm, 0);
    }
    XtAddCallback(menu, XmNentryCallback, cbProc, cbArg);
    return menu;
}

static Widget createHistLineStyleMenu(Widget parent, GC drawGC,
    	Drawable pmWindow, Pixmap pixmaps[XY_N_HIST_LINE_STYLES],
    	Widget btns[XY_N_HIST_LINE_STYLES], XtCallbackProc cbProc, void *cbArg)
{
    int depth = DefaultDepthOfScreen(XtScreen(parent));
    Display *display = XtDisplay(parent);
    Pixel bgColor, fgColor;
    Pixmap pm;
    int i;
    Widget menu;
    
    XtVaGetValues(parent, XmNbackground, &bgColor, 0);
    fgColor = BlackPixelOfScreen(XtScreen(parent));

    menu = XmCreatePulldownMenu(parent, "lineMenu", NULL, 0);
    for (i=0; i<XY_N_HIST_LINE_STYLES; i++) {
        pm = XCreatePixmap(display, pmWindow, 50, 12, depth);
        pixmaps[i] = pm;
        XSetForeground(display, drawGC, bgColor);
        XFillRectangle(display, pm, drawGC, 0, 0, 50, 12);
        if (i < XY_N_LINE_STYLES)
            XYDrawLine(display, pm, drawGC, i, fgColor, 5, 6, 45, 6);
        else
            XYDrawHistMarker(display, pm, drawGC, i, fgColor, 25, 6);
        btns[i] = XtVaCreateManagedWidget("lineStyle",
        	xmPushButtonGadgetClass, menu,
    		XmNlabelType, XmPIXMAP,
    		XmNlabelPixmap, pm, 0);
    }
    XtAddCallback(menu, XmNentryCallback, cbProc, cbArg);
    return menu;
}

static Widget createFillStyleMenu(Widget parent, GC drawGC, Drawable pmWindow,
    	Pixmap pixmaps[N_FILL_STYLES], Widget btns[N_FILL_STYLES],
    	XtCallbackProc cbProc, void *cbArg)
{
    int depth = DefaultDepthOfScreen(XtScreen(parent));
    Display *display = XtDisplay(parent);
    Pixel bgColor, fgColor;
    Pixmap pm;
    int i;
    Widget menu;
    
    XtVaGetValues(parent, XmNbackground, &bgColor, 0);
    fgColor = BlackPixelOfScreen(XtScreen(parent));

    menu = XmCreatePulldownMenu(parent, "fillMenu", NULL, 0);
    for (i=0; i<N_FILL_STYLES; i++) {
        pm = XCreatePixmap(display, pmWindow, 50, 12, depth);
        pixmaps[i] = pm;
        XSetForeground(display, drawGC, bgColor);
        XFillRectangle(display, pm, drawGC, 0, 0, 50, 12);
        XYDrawFill(display, pm, drawGC, HistFillStyles[i], fgColor, 0,0, 50,12);
        btns[i] = XtVaCreateManagedWidget("fillStyle",
        	xmPushButtonGadgetClass, menu,
    		XmNlabelType, XmPIXMAP,
    		XmNlabelPixmap, pm, 0);
    }
    XtAddCallback(menu, XmNentryCallback, cbProc, cbArg);
    return menu;
}

static Widget createMarkStyleMenu(Widget parent, GC drawGC, Drawable pmWindow,
    	Pixmap pixmaps[XY_N_MARK_STYLES], Widget btns[XY_N_MARK_STYLES],
    	XtCallbackProc cbProc, void *cbArg)
{
    int depth = DefaultDepthOfScreen(XtScreen(parent));
    Display *display = XtDisplay(parent);
    Pixel bgColor, fgColor;
    Pixmap pm;
    int i;
    Widget menu;
    
    XtVaGetValues(parent, XmNbackground, &bgColor, 0);
    fgColor = BlackPixelOfScreen(XtScreen(parent));

    menu = XmCreatePulldownMenu(parent, "markMenu", NULL, 0);
    for (i=0; i<XY_N_MARK_STYLES; i++) {
        pm = XCreatePixmap(display, pmWindow, 12, 12, depth);
        pixmaps[i] = pm;
        XSetForeground(display, drawGC, bgColor);
        XFillRectangle(display, pm, drawGC, 0, 0, 12, 12);
        XYDrawMarker(display, pm, drawGC, XY_LARGE, i, fgColor, 6, 6);
        btns[i] = XtVaCreateManagedWidget("markStyle",
        	xmPushButtonGadgetClass, menu,
    		XmNlabelType, XmPIXMAP,
    		XmNlabelPixmap, pm, 0);
    }
    XtAddCallback(menu, XmNentryCallback, cbProc, cbArg);
    return menu;
}

static Widget createMarkSizeMenu(Widget parent, GC drawGC, Drawable pmWindow,
    	Pixmap pixmaps[XY_N_MARK_SIZES], Widget btns[XY_N_MARK_SIZES],
    	XtCallbackProc cbProc, void *cbArg)
{
    int depth = DefaultDepthOfScreen(XtScreen(parent));
    Display *display = XtDisplay(parent);
    Pixel bgColor, fgColor;
    Pixmap pm;
    int i;
    Widget menu;
    
    XtVaGetValues(parent, XmNbackground, &bgColor, 0);
    fgColor = BlackPixelOfScreen(XtScreen(parent));
    
    menu = XmCreatePulldownMenu(parent, "sizeMenu", NULL, 0);
    for (i=0; i<XY_N_MARK_SIZES; i++) {
        pm = XCreatePixmap(display, pmWindow, 12, 12, depth);
        pixmaps[i] = pm;
        XSetForeground(display, drawGC, bgColor);
        XFillRectangle(display, pm, drawGC, 0, 0, 12, 12);
        XYDrawMarker(display, pm, drawGC, i, XY_CIRCLE_MARK, fgColor, 6, 6);
        btns[i] = XtVaCreateManagedWidget("markSize",
        	xmPushButtonGadgetClass, menu,
    		XmNlabelType, XmPIXMAP,
    		XmNlabelPixmap, pm, 0);
    }
    XtAddCallback(menu, XmNentryCallback, cbProc, cbArg);
    return menu;
}

static Widget createMarkColorMenu(Widget parent, GC drawGC, Drawable pmWindow,
    	int nColors, Pixel colors[], Pixmap pixmaps[], Widget btns[],
    	XtCallbackProc cbProc, void *cbArg)
{
    int depth = DefaultDepthOfScreen(XtScreen(parent));
    Display *display = XtDisplay(parent);
    Pixel bgColor;
    Pixmap pm;
    int i;
    Widget menu;
    
    XtVaGetValues(parent, XmNbackground, &bgColor, 0);

    menu = XmCreatePulldownMenu(parent, "mkColorMenu", NULL, 0);
    for (i=0; i<nColors; i++) {
        pm = XCreatePixmap(display, pmWindow, 12, 12, depth);
        pixmaps[i] = pm;
        XSetForeground(display, drawGC, bgColor);
        XFillRectangle(display, pm, drawGC, 0, 0, 12, 12);
        XYDrawMarker(display, pm, drawGC, XY_LARGE, XY_SOLID_SQUARE_MARK,
            	colors[i], 6, 6);
        btns[i] = XtVaCreateManagedWidget("markColor",
        	xmPushButtonGadgetClass, menu,
    		XmNlabelType, XmPIXMAP,
    		XmNlabelPixmap, pm, 0);
    }
    XtAddCallback(menu, XmNentryCallback, cbProc, cbArg);
    return menu;
}

static Widget createFillColorMenu(Widget parent, GC drawGC, Drawable pmWindow,
    	int nColors, Pixel colors[], Pixmap pixmaps[], Widget btns[],
    	XtCallbackProc cbProc, void *cbArg)
{
    int depth = DefaultDepthOfScreen(XtScreen(parent));
    Display *display = XtDisplay(parent);
    Pixmap pm;
    int i;
    Widget menu;
    
    menu = XmCreatePulldownMenu(parent, "fillColorMenu", NULL, 0);
    for (i=0; i<nColors; i++) {
        pm = XCreatePixmap(display, pmWindow, 50, 12, depth);
        pixmaps[i] = pm;
        XSetForeground(display, drawGC, colors[i]);
        XFillRectangle(display, pm, drawGC, 0, 0, 50, 12);
        btns[i] = XtVaCreateManagedWidget("fillColor",
        	xmPushButtonGadgetClass, menu,
    		XmNlabelType, XmPIXMAP,
    		XmNlabelPixmap, pm, 0);
    }
    XtAddCallback(menu, XmNentryCallback, cbProc, cbArg);
    return menu;
}

static Widget createLineColorMenu(Widget parent, GC drawGC, Drawable pmWindow,
    	int nColors, Pixel colors[], Pixmap pixmaps[], Widget btns[],
    	XtCallbackProc cbProc, void *cbArg)
{
    int depth = DefaultDepthOfScreen(XtScreen(parent));
    Display *display = XtDisplay(parent);
    Pixel bgColor;
    Pixmap pm;
    int i;
    Widget menu;
    
    XtVaGetValues(parent, XmNbackground, &bgColor, 0);

    menu = XmCreatePulldownMenu(parent, "lnColorMenu", NULL, 0);
    for (i=0; i<nColors; i++) {
        pm = XCreatePixmap(display, pmWindow, 50, 12, depth);
        pixmaps[i] = pm;
        XSetForeground(display, drawGC, bgColor);
        XFillRectangle(display, pm, drawGC, 0, 0, 50, 12);
        XYDrawLine(display, pm, drawGC, XY_THICK_LINE, colors[i], 5,
            	6, 45, 6);
        btns[i] = XtVaCreateManagedWidget("lineColor",
        	xmPushButtonGadgetClass, menu,
    		XmNlabelType, XmPIXMAP,
    		XmNlabelPixmap, pm, 0);
    }
    XtAddCallback(menu, XmNentryCallback, cbProc, cbArg);
    return menu;
}

static void updateMarkLineSample(markDialog *dialog, int index)
{
    XYCurve *curve = &dialog->curves[index];
    Pixmap pm = dialog->samplePixmaps[index];
    Display *display = XtDisplay(dialog->form);
    GC gc = dialog->gc;
    Pixel bgColor;
    
    XtVaGetValues(XtParent(dialog->sampleLabels[index]), XmNbackground, 
    	&bgColor, 0);
    XSetForeground(display, gc, bgColor);
    XFillRectangle(display, pm, gc, 0, 0, 52, 12);
    XYDrawMarker(display, pm, gc, curve->markerSize,
            curve->markerStyle, curve->markerPixel, 46, 6);
    if (curve->lineStyle != XY_NO_LINE) {
	XYDrawMarker(display, pm, gc, curve->markerSize,
        	curve->markerStyle, curve->markerPixel, 6, 6);
        XYDrawLine(display, pm, gc, curve->lineStyle,
            	curve->linePixel, 6, 6, 46, 6);
    }
    /* Kick Motif to re-draw the pixmap area (yep, set values twice!) */
    XtVaSetValues(dialog->sampleLabels[index], XmNlabelPixmap, 
    	    dialog->linePixmaps[0], 0);
    XtVaSetValues(dialog->sampleLabels[index], XmNlabelPixmap, pm, 0);
}

static void updateHistSample(histStyleDialog *dialog, int index)
{
    XYHistogram *hist = &dialog->hists[index];
    Pixmap pm = dialog->samplePixmaps[index];
    Display *display = XtDisplay(dialog->form);
    GC drawGC = dialog->gc;
    Pixel bgColor;
    
    /* Redraw into the pixmap */
    XtVaGetValues(XtParent(dialog->sampleLabels[index]), XmNbackground, 
    	&bgColor, 0);
    XSetForeground(display, drawGC, bgColor);
    XFillRectangle(display, pm, drawGC, 0, 0, 52, 12);
    if (hist->lineStyle < XY_N_LINE_STYLES) {
	XYDrawFill(display, pm, drawGC, hist->fillStyle, hist->fillPixel,
	    	0, 0, 51, 11);
	XYDrawLine(display, pm, drawGC, hist->lineStyle, hist->linePixel,
        	0, 0, 51, 0);
	XYDrawLine(display, pm, drawGC, hist->lineStyle, hist->linePixel,
        	51, 0, 51, 11);
	XYDrawLine(display, pm, drawGC, hist->lineStyle, hist->linePixel,
        	51, 11, 0, 11);
	XYDrawLine(display, pm, drawGC, hist->lineStyle, hist->linePixel,
        	0, 11, 0, 0);
    } else
    	XYDrawHistMarker(display, pm, drawGC, hist->lineStyle, hist->linePixel,
    	    	46, 6);

    /* Kick Motif to re-draw the pixmap area (yep, set values twice!) */
    XtVaSetValues(dialog->sampleLabels[index], XmNlabelPixmap, 
    	    dialog->linePixmaps[0], 0);
    XtVaSetValues(dialog->sampleLabels[index], XmNlabelPixmap, pm, 0);
}

static void updateMarkLineDialog(markDialog *dialog)
{
    int i, j;
    XYCurve *curve;
    Widget *w, selectedW;
    
    for (i=0, curve=dialog->curves; i<dialog->nCurves; i++, curve++) {
        XtVaGetValues(dialog->lineOptMenus[i], XmNmenuHistory, &selectedW, 0);
        for (j=0, w=dialog->lineBtns; j<XY_N_LINE_STYLES; j++, w++) {
            if (*w == selectedW && j != curve->lineStyle) {
        	curve->lineStyle = j;
        	updateMarkLineSample(dialog, i);
        	break;
            }
        }
        XtVaGetValues(dialog->markOptMenus[i], XmNmenuHistory, &selectedW, 0);
        for (j=0, w=dialog->markBtns; j<XY_N_MARK_STYLES; j++, w++) {
            if (*w == selectedW && j != curve->markerStyle) {
        	curve->markerStyle = j;
        	updateMarkLineSample(dialog, i);
        	break;
            }
        }
        XtVaGetValues(dialog->sizeOptMenus[i], XmNmenuHistory, &selectedW, 0);
        for (j=0, w=dialog->sizeBtns; j<XY_N_MARK_SIZES; j++, w++) {
            if (*w == selectedW && j != curve->markerSize) {
        	curve->markerSize = j;
        	updateMarkLineSample(dialog, i);
        	break;
            }
        }
        if (dialog->nColors > 0) {
            XtVaGetValues(dialog->lnColorOptMenus[i], XmNmenuHistory, 
            	&selectedW, 0);
            for (j=0, w=dialog->lineColorBtns; 
            		    j < dialog->nColors && j<XY_N_COLORS; j++, w++) {
        	if (*w == selectedW && dialog->color[j] != curve->linePixel) {
        	    allocNewFreePrevious(XtDisplay(dialog->form),
        	    	    XScreenNumberOfScreen(XtScreen(dialog->form)),
        	    	    dialog->color, dialog->nColors, dialog->color_def,
        	    	    dialog->color_used, j, curve->linePixel);
        	    curve->linePixel = dialog->color[j];
        	    updateMarkLineSample(dialog, i);
        	    break;
        	}
            }
            XtVaGetValues(dialog->mkColorOptMenus[i], XmNmenuHistory, 
            	&selectedW,0);
            for (j=0, w=dialog->markColorBtns; 
            		    j < dialog->nColors && j<XY_N_COLORS; j++, w++) {
        	if (*w == selectedW && dialog->color[j] != curve->markerPixel) {
        	    allocNewFreePrevious(XtDisplay(dialog->form),
        	    	    XScreenNumberOfScreen(XtScreen(dialog->form)),
        	    	    dialog->color, dialog->nColors, dialog->color_def,
        	    	    dialog->color_used, j, curve->markerPixel);
        	    curve->markerPixel = dialog->color[j];
        	    updateMarkLineSample(dialog, i);
        	    break;
        	}
            }
        }
    }
}

static void updateHistDialog(histStyleDialog *dialog)
{
    int i, j;
    XYHistogram *hist;
    Widget *w, selectedW;
    
    for (i=0, hist=dialog->hists; i<dialog->nHists; i++, hist++) {
        XtVaGetValues(dialog->lineOptMenus[i], XmNmenuHistory, &selectedW, 0);
        for (j=0, w=dialog->lineBtns; j<XY_N_HIST_LINE_STYLES; j++, w++) {
            if (*w == selectedW && j != hist->lineStyle) {
        	hist->lineStyle = j;
        	updateHistSample(dialog, i);
        	break;
            }
        }
        XtVaGetValues(dialog->fillOptMenus[i], XmNmenuHistory, &selectedW, 0);
        for (j=0, w=dialog->fillBtns; j<N_FILL_STYLES; j++, w++) {
            if (*w == selectedW && HistFillStyles[j] != hist->fillStyle) {
        	hist->fillStyle = HistFillStyles[j];
        	updateHistSample(dialog, i);
        	break;
            }
        }
        if (dialog->nColors > 0) {
            XtVaGetValues(dialog->lineColorOptMenus[i], XmNmenuHistory, 
            	&selectedW, 0);
            for (j=0, w=dialog->lineColorBtns; 
            		    j<dialog->nColors && j<XY_N_COLORS; j++, w++) {
        	if (*w == selectedW && dialog->color[j] != hist->linePixel) {
        	    allocNewFreePrevious(XtDisplay(dialog->form),
        	    	    XScreenNumberOfScreen(XtScreen(dialog->form)),
        	    	    dialog->color, dialog->nColors, dialog->color_def,
        	    	    dialog->color_used, j, hist->linePixel);
        	    hist->linePixel = dialog->color[j];
        	    updateHistSample(dialog, i);
        	    break;
        	}
            }
            XtVaGetValues(dialog->fillColorOptMenus[i], XmNmenuHistory, 
            	&selectedW,0);
            for (j=0, w=dialog->fillColorBtns; 
            		    j<dialog->nColors && j<XY_N_COLORS; j++, w++) {
        	if (*w == selectedW && dialog->color[j] != hist->fillPixel) {
        	    allocNewFreePrevious(XtDisplay(dialog->form),
        	    	    XScreenNumberOfScreen(XtScreen(dialog->form)),
        	    	    dialog->color, dialog->nColors, dialog->color_def,
        	    	    dialog->color_used, j, hist->fillPixel);
        	    hist->fillPixel = dialog->color[j];
        	    updateHistSample(dialog, i);
        	    break;
        	}
            }
        }
    }
}

static void markOptMenuCB(Widget w, markDialog *dialog, caddr_t callData)
{
    XtAppAddTimeOut(XtWidgetToApplicationContext(dialog->form),
    	    0, updateMarkTimerProc, dialog);
}

static void histOptMenuCB(Widget w, histStyleDialog *dialog, caddr_t callData)
{
    XtAppAddTimeOut(XtWidgetToApplicationContext(dialog->form),
    	    0, updateHistTimerProc, dialog);
}

static void clearLinesCB(Widget w, markDialog *dialog, caddr_t callData)
{
    int i;
    
    for (i=0; i<dialog->nCurves; i++) {
	XtVaSetValues(dialog->lineOptMenus[i], XmNmenuHistory,
		dialog->lineBtns[0], 0);
	updateMarkLineDialog(dialog);
    }
}

static void clearMarksCB(Widget w, markDialog *dialog, caddr_t callData)
{
    int i;
    
    for (i=0; i<dialog->nCurves; i++) {
	XtVaSetValues(dialog->markOptMenus[i], XmNmenuHistory,
		dialog->markBtns[0], 0);
	updateMarkLineDialog(dialog);
    }
}

static void defaultLinesCB(Widget w, markDialog *dialog, caddr_t callData)
{
    int i;
    
    for (i=0; i<dialog->nCurves; i++)
	XtVaSetValues(dialog->lineOptMenus[i], XmNmenuHistory,
		dialog->lineBtns[(i % XY_4_DOT_DASH) + 1], 0);
    updateMarkLineDialog(dialog);
}

static void defaultMarksCB(Widget w, markDialog *dialog, caddr_t callData)
{
    int i;
    
    for (i=0; i<dialog->nCurves; i++)
	XtVaSetValues(dialog->markOptMenus[i], XmNmenuHistory,
		dialog->markBtns[(i % XY_SOLID_CIRCLE_MARK) + 1], 0);
    updateMarkLineDialog(dialog);
}

static void markSizeCB(Widget w, markDialog *dialog, caddr_t callData)
{
    createMarkSizeDialog(dialog);
}

static void copyColorsCB(Widget w, markDialog *dialog, caddr_t callData)
{
    int i, j;
    Widget *wid, selectedW;
    
    for (i=0; i<dialog->nCurves; i++) {
	XtVaGetValues(dialog->lnColorOptMenus[i], XmNmenuHistory, &selectedW,0);
        for (j=0, wid=dialog->lineColorBtns; j<dialog->nColors && j<XY_N_COLORS;
        	    j++, wid++) {
            if (*wid == selectedW) {
        	XtVaSetValues(dialog->mkColorOptMenus[i], XmNmenuHistory,
		    dialog->markColorBtns[j], 0);
        	break;
    	    }
    	}
    }
    updateMarkLineDialog(dialog);
}

static void markLineOkCB(Widget w, markDialog *dialog, caddr_t callData)
{
    copyCurveStyles(dialog->curves, dialog->origCurves, dialog->nCurves);
    if (dialog->okCB != NULL)
    	(*dialog->okCB)(dialog->origCurves, dialog->nCurves, dialog->okArg);
    XtDestroyWidget(XtParent(dialog->form));
}

static void histOkCB(Widget w, histStyleDialog *dialog, caddr_t callData)
{
    copyHistStyles(dialog->hists, dialog->origHists, dialog->nHists);
    if (dialog->okCB != NULL)
    	(*dialog->okCB)(dialog->origHists, dialog->nHists, dialog->okArg);
    XtDestroyWidget(XtParent(dialog->form));
}

static void markLineApplyCB(Widget w, markDialog *dialog, caddr_t callData)
{
    copyCurveStyles(dialog->curves, dialog->origCurves, dialog->nCurves);
    if (dialog->applyCB != NULL)
    	(*dialog->applyCB)(dialog->origCurves, dialog->nCurves,
    		dialog->applyArg);
}

static void histApplyCB(Widget w, histStyleDialog *dialog, caddr_t callData)
{
    copyHistStyles(dialog->hists, dialog->origHists, dialog->nHists);
    if (dialog->applyCB != NULL)
    	(*dialog->applyCB)(dialog->origHists, dialog->nHists,
    		dialog->applyArg);
}

static void markLineDismissCB(Widget w, markDialog *dialog, caddr_t callData)
{
    if (dialog->dismissCB != NULL)
    	(*dialog->dismissCB)(dialog->origCurves, dialog->nCurves,
    		dialog->dismissArg);
    XtDestroyWidget(XtParent(dialog->form));
}

static void histDismissCB(Widget w, histStyleDialog *dialog, caddr_t callData)
{
    if (dialog->dismissCB != NULL)
    	(*dialog->dismissCB)(dialog->origHists, dialog->nHists,
    		dialog->dismissArg);
    XtDestroyWidget(XtParent(dialog->form));
}

static void markLineDestroyCB(Widget w, markDialog *dialog, caddr_t callData)
{
    int i;
    Display *display = XtDisplay(dialog->form);
    
    /* Free all resources used by the dialog */
    for (i=0; i<XY_N_MARK_STYLES; i++)
    	XFreePixmap(display, dialog->markPixmaps[i]);
    for (i=0; i<XY_N_LINE_STYLES; i++)
    	XFreePixmap(display, dialog->linePixmaps[i]);
    for (i=0; i<XY_N_MARK_SIZES; i++)
    	XFreePixmap(display, dialog->sizePixmaps[i]);
    /* free colors allocated that are not in use */
    if (dialog->nColors > 0) {
    	Pixel toFree[XY_N_COLORS];
    	unsigned int planes = 0;
    	int screen_num = XScreenNumberOfScreen(XtScreen(dialog->form));
        int j = 0;
        for (i = 0; i<XY_N_COLORS && i<dialog->nColors; i++)
            if (dialog->color_used[i])
                toFree[j++] = dialog->color[i];
/*  	printf("About to free pixel %d values: ");
 	for (i = 0; i<j; i++)
	    printf("%d  ", toFree[i]);
 */     if (j > 0)
            XFreeColors(display, DefaultColormap(display, screen_num), toFree, 
            	j, planes);
    }
    XtFree((char *)dialog->curves);
    XtFree((char *)dialog->lineOptMenus);
    XtFree((char *)dialog->markOptMenus);
    XtFree((char *)dialog->sizeOptMenus);
    if (dialog->nColors > 0) {
	XtFree((char *)dialog->mkColorOptMenus);
	XtFree((char *)dialog->lnColorOptMenus);
    }
    XtFree((char *)dialog->samplePixmaps);
    XtFree((char *)dialog->sampleLabels);
    XFreeGC(display, dialog->gc);
    XtFree((char *)dialog);
}

static void histDestroyCB(Widget w, histStyleDialog *dialog, caddr_t callData)
{
    int i;
    Display *display = XtDisplay(dialog->form);
    
    /* Free all resources used by the dialog */
    for (i=0; i<N_FILL_STYLES; i++)
    	XFreePixmap(display, dialog->fillPixmaps[i]);
    for (i=0; i<XY_N_HIST_LINE_STYLES; i++)
    	XFreePixmap(display, dialog->linePixmaps[i]);
    /* free colors allocated that are not in use */
    if (dialog->nColors > 0) {
    	Pixel toFree[XY_N_COLORS];
    	unsigned int planes = 0;
    	int screen_num = XScreenNumberOfScreen(XtScreen(dialog->form));
        int j = 0;
        for (i = 0; i<XY_N_COLORS && i<dialog->nColors; i++)
            if (dialog->color_used[i])
                toFree[j++] = dialog->color[i];
/*  	printf("About to free pixel %d values: ");
 	for (i = 0; i<j; i++)
	    printf("%d  ", toFree[i]);
 */     if (j > 0)
            XFreeColors(display, DefaultColormap(display, screen_num), toFree, 
            	j, planes);
    }
    XtFree((char *)dialog->hists);
    XtFree((char *)dialog->lineOptMenus);
    XtFree((char *)dialog->fillOptMenus);
    if (dialog->nColors > 0) {
	XtFree((char *)dialog->fillColorOptMenus);
	XtFree((char *)dialog->lineColorOptMenus);
    }
    XtFree((char *)dialog->samplePixmaps);
    XtFree((char *)dialog->sampleLabels);
    XFreeGC(display, dialog->gc);
    XtFree((char *)dialog);
}

static void copyCurveStyles(XYCurve *fromCurves, XYCurve *toCurves, int nCurves)
{
    int i;
    XYCurve *from, *to;
    
    for(i=0, from=fromCurves, to=toCurves; i<nCurves; i++, from++, to++) {
	to->markerStyle = from->markerStyle;
	to->markerSize = from->markerSize;
	to->lineStyle = from->lineStyle;
	to->markerPixel = from->markerPixel;
	to->linePixel = from->linePixel;
	to->nPoints = from->nPoints;
    }
}

static void copyHistStyles(XYHistogram *fromHists, XYHistogram *toHists,
    	int nHists)
{
    int i;
    XYHistogram *from, *to;
    
    for(i=0, from=fromHists, to=toHists; i<nHists; i++, from++, to++) {
	to->fillStyle = from->fillStyle;
	to->lineStyle = from->lineStyle;
	to->fillPixel = from->fillPixel;
	to->linePixel = from->linePixel;
    }
}

static void updateMarkTimerProc(XtPointer clientData, XtIntervalId *id)
{
    updateMarkLineDialog((markDialog *)clientData);
}

static void updateHistTimerProc(XtPointer clientData, XtIntervalId *id)
{
    updateHistDialog((histStyleDialog *)clientData);
}

static void createMarkSizeDialog(markDialog *dialog)
{
    Widget selBox, checkBox;
    XmString s1;
    int i, n, avgSize, sum = 0;
    Arg args[5];

    /* find the average size of markers for setting default */
    for (i=0; i<dialog->nCurves; i++)
    	sum += dialog->curves[i].markerSize;
    avgSize = sum / dialog->nCurves;
    
    n = 0;
    XtSetArg(args[n], XmNselectionLabelString,
    	    s1=XmStringCreateSimple("Set all markers to size:")); n++;
#if XmVersion >= 1002
    XtSetArg(args[n], XmNchildPlacement, XmPLACE_BELOW_SELECTION); n++;
#endif
    selBox = XmCreatePromptDialog(dialog->form, "markSizeDialog", args, n);
    XmStringFree(s1);
    dialog->sizeShell = XtParent(selBox);
    XtAddCallback(selBox, XmNokCallback, (XtCallbackProc)sizeOKCB, dialog);
    XtAddCallback(selBox, XmNcancelCallback, (XtCallbackProc)sizeCancelCB,
    	    dialog);
    XtUnmanageChild(XmSelectionBoxGetChild(selBox, XmDIALOG_TEXT));
    XtUnmanageChild(XmSelectionBoxGetChild(selBox, XmDIALOG_HELP_BUTTON));
    XtVaSetValues(XtParent(selBox), XmNtitle, "Marker Size", 0);
    AddMotifCloseCallback(XtParent(selBox), (XtCallbackProc)sizeCancelCB,
 	    dialog);

    /* create the size checkbox */
    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    checkBox = XmCreateRadioBox(selBox, "checkBox", args, n);
    XtManageChild(checkBox);
    for (i=0; i<XY_N_MARK_SIZES; i++) {
    	dialog->sizeDlogBtns[i] = XtVaCreateManagedWidget("size",
    		xmToggleButtonGadgetClass, checkBox, XmNlabelType, XmPIXMAP,
    		XmNlabelPixmap, dialog->sizePixmaps[i], XmNset, i==avgSize, 0);
    }
#if XmVersion >= 1002
    XtVaSetValues(checkBox, XmNinitialFocus, dialog->sizeDlogBtns[avgSize], 0);
#endif
    
    XtManageChild(selBox);
}

static void sizeOKCB(Widget w, markDialog *dialog, caddr_t callData)
{
    int i, index;
    
    /* Find out which size was selected */
    for (i=0; i<XY_N_MARK_SIZES; i++)
    	if (XmToggleButtonGetState(dialog->sizeDlogBtns[i]))
    	    index = i;
    
    /* Set all of the option menus to show that size */
    for (i=0; i<dialog->nCurves; i++)
	XtVaSetValues(dialog->sizeOptMenus[i], XmNmenuHistory,
		dialog->sizeBtns[index], 0);
    updateMarkLineDialog(dialog);
    
    /* Get rid of the dialog */
    XtDestroyWidget(dialog->sizeShell);
}

static void sizeCancelCB(Widget w, markDialog *dialog, caddr_t callData)
{
    XtDestroyWidget(dialog->sizeShell);
}

static int findinList(Pixel *list, int numItems, int value)
{
    int i;
    
    for (i = 0; i < numItems && i < XY_N_COLORS; ++i) {
    	if (list[i] == value)
    	    return i;
    }
    return -1;
}

/*
 * allocNewFreePrevious: Free a color previously allocated, and
 *			 re-allocate a new color to be used so can free it later
 */
static void allocNewFreePrevious(Display *display, int screen_num,
    	Pixel *color, int nColors, XColor color_def[XY_N_COLORS],
    	Boolean color_used[XY_N_COLORS], int newColorIndex, Pixel prevPixel)
{
    unsigned int planes = 0;
    int colorFound = findinList(color, nColors, prevPixel);
    
    /* Free color if we previously allocated it */
    if (prevPixel != BlackPixel(display, screen_num) && colorFound != -1) {
/*       printf("About to free pixel value: %d (%d, %d)\n",
            prevPixel, BlackPixel(display, screen_num), colorFound);
 */      XFreeColors(display, DefaultColormap(display, screen_num), &prevPixel, 
            1, planes);
    } /* if not found, was probably default color (black) */
    
    /* Allocate color if not equal to BlackPixel and mark as used */
    if (newColorIndex >= nColors) {
/*     	printf(
    	 "allocNewFreePrevious: newColorIndex >= nColors [%d>=%d]\n",
    	 newColorIndex, nColors);
 */    	return;
    }
    if (color[newColorIndex] != BlackPixel(display, screen_num)) {
        if (!XAllocColor(display, DefaultColormap(display, screen_num), 
   		&color_def[newColorIndex]))
	    fprintf(stderr, "\nCan't allocate color: all colorcells allocated \
and no matching cell found.\n");
    }
    color_used[newColorIndex] = TRUE;
}

/* get_colors - Returns number of read-only colors allocated if X server can 
*               handle color; otherwise returns 0.  Returns 23 or at most 
*               max_num_colors_ret pixel values of the colors allocated in 
*               array color_pixel.  Also returns the exact definitions.
*
*		Some code borrowed from O'Reilly and Associates, Inc.
* 		See ../Copyright for complete rights and liability information.
*/
static int get_colors(Display *display, int screen_num, char *progname, 
	int max_num_colors_ret, Pixel *color_pixel, XColor *exact_def)
{
    int default_depth;
    Visual *default_visual;
    int ncolors = 0;
    Pixel colors[MAX_COLORS];
    int visualClass = 5;
    int i;
    XVisualInfo visual_info;
    static int warningPrinted = 0;
/*
 *     static char *visual_class[] = {
 * 	"StaticGray",
 * 	"GrayScale",
 * 	"StaticColor",
 * 	"PseudoColor",
 * 	"TrueColor",
 * 	"DirectColor"
 *     };
 */ 

    /* Try to allocate colors for PseudoColor, TrueColor, 
     * DirectColor, and StaticColor.  Use black and white
     * for StaticGray and GrayScale */

    default_depth = DefaultDepth(display, screen_num);
    default_visual = DefaultVisual(display, screen_num);
    if (default_depth == 1) {
	/* must be StaticGray, use black and white */
	return(0);
    }

    while (!XMatchVisualInfo(display, screen_num, default_depth, 
	    visualClass--, &visual_info))
	    ;
    ++visualClass;
/*  printf("%s: found a %s class visual at default_depth.\n", progname, 
	    visual_class[visualClass]);
 */
    if (visualClass < 2) {
	/* No color visual available at default_depth.  Some applications
	 * might call XMatchVisualInfo here to try for a GrayScale
	 * visual if they can use gray to advantage, before  giving up
	 * and using black and white.
	 */
	return(0);
    }

    /* Otherwise, got a color visual at default_depth */

    /* The visual we found is not necessarily the default visual, and
     * therefore it is not necessarily the one we used to create our
     * window.  However, we now know for sure that color is supported,
     * so the following code will work (or fail in a controlled way).
     * Let's check just out of curiosity:
     */
/*     if (visual_info.visual != default_visual)
       printf("%s: PseudoColor visual at default depth is not default visual!\n\
	   Continuing anyway...\n", progname);
 */
    for (i = 0; i < MAX_COLORS && ncolors < max_num_colors_ret; i++) {
	/* printf("allocating %s\n", ColorName[i]); */
	if (!XParseColor (display, DefaultColormap(display, screen_num), 
		ColorName[i], &exact_def[ncolors])) {
	    fprintf(stderr, "%s: color name %s not in database\n", 
		progname, ColorName[i]);
	    continue;
	}
/* 	printf("The RGB values from the database are %d, %d, %d\n", 
	    exact_def[ncolors].red, exact_def[ncolors].green, 
	    exact_def[ncolors].blue);
 */   	if (!XAllocColor(display, DefaultColormap(display, screen_num), 
   		&exact_def[ncolors])) {
	    ColorPixel[i] = -1;
	    if (warningPrinted == 0) {
	        fprintf(stderr, 
	            "\nCan't allocate: %s. All colorcells allocated\
 and no matching cell found.\n", ColorName[i]);
	        warningPrinted = -1;
	    }
	    continue;
	}
/* 	printf("The RGB values actually allocated are %d, %d, %d\n", 
	    exact_def[ncolors].red, exact_def[ncolors].green, 
	    exact_def[ncolors].blue);
 */	color_pixel[ncolors] = exact_def[ncolors].pixel;
	ColorPixel[i] = exact_def[ncolors].pixel;
        ncolors++;
    }
    /* Make sure rest of ColorPixel table shows the pixels not used.      */
    /* ColorPixel table used by configFile.c to translate pixels to names */
    for ( ; i < MAX_COLORS; i++)
    	ColorPixel[i] = -1;

    /* printf("%s: allocated %d read-only color cells\n", progname, ncolors); */

    return(ncolors);
}

/*
* GetXYColors - Returns pointers to two arrays for read-only colors  
*               that are allocated by the XY Dialog.  The colorName
*		array contains common color names (e.g. blue).  The
*		colorPixel array contains the corresponding pixel value
*		saved in the curves structure for the XY Widget.  A
*		-1 pixel value denotes that the color is not allocated/used.  
*/
void GetXYColors(char ***colorName , Pixel **colorPixel)
{
    *colorName = ColorName;
    *colorPixel = ColorPixel;
}

/*
** Find the index for the FillStyles and various fill related arrays
** corresponding to fill style "fillStyle"
*/
static int findFillIndex(unsigned fillStyle)
{
    int i;
    
    for (i=0; i<N_FILL_STYLES; i++)
    	if (HistFillStyles[i] == fillStyle)
    	    return i;
    return 0; /* unknown, user-defined, fill pattern */
}

/*
** Obsolete calls for compatibility with v1.0
*/
Widget XYCreateStylesDialog(Widget parent, XYCurve *curves, int nCurves,
	XYCallbackProc okCallback, void *okArg, XYCallbackProc applyCallback,
	void *applyArg, XYCallbackProc dismissCallback, void *dismissArg)
{
    XYCreateCurveStylesDialog(parent, curves, nCurves, okCallback, okArg,
    	    applyCallback, applyArg, dismissCallback, dismissArg);
}
int XYEditStyles(Widget parent, XYCurve *curves, int nCurves)
{
    XYEditCurveStyles(parent, curves, nCurves);
}

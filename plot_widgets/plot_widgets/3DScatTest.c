#include <X11/Intrinsic.h> 
#include <Xm/Xm.h>
#include <Xm/DrawingA.h>
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <Xm/MainW.h>
#include <Xm/Form.h>
#include <Xm/PushB.h>
#include "3DScat.h"

static void saveRotationCB(Widget w, Widget scat, caddr_t call_data);
static void setRotationCB(Widget w, Widget scat, caddr_t call_data);
static void makePs(Widget w, Widget scat, caddr_t call_data);

Widget toplevel, Scat3DW, form, saveRotationBtn, setRotationBtn, printBtn;
double  AlphaR, BetaR, GammaR;
void main(argc, argv)
  unsigned   argc;
  char *argv[];
{
    Scat3DPoint data [60000];
    int i,j,k;
    Scat3DPoint *tmp = data;
    int nPoints = 0;
    XmString s1;
    
/*   for (i = -10; i<= 10; i++)
        for (j = -10; j<= 10; j++)
            for (k = -10; k<= 10; k++)
               if ( i * i + j * j + k * k < 10)
               {
                   tmp->x = i;
                   tmp->y = j;
                   tmp->z = k;
                   tmp++;
                   tmp->x = i + 200;
                   tmp->y = j + 200;
                   tmp->z = k + 200;
                   tmp++;
                   nPoints += 2;
               }    
*/ /*    for (i = -1; i<= 10; i++)
    {
        tmp->x = i * 100;
        tmp->y = i * 100;
        tmp->z = i * 100;
        tmp++;
        nPoints += 1;
    }        
*/
    for (i = 0; i<= 50000; i+=10 )
    {
        tmp->x = i * 100;
        tmp->y = i * 100;
        tmp->z = i * 100;
        tmp++;
        nPoints += 1;
    }        

    toplevel = XtInitialize(argv[0], "Scat3DTest", NULL, 
                            0, &argc, argv);
    /*
     * Create a form, a few buttons, and the widgets.
     */
    form = XtVaCreateManagedWidget("scat3DForm", xmFormWidgetClass,
                                     toplevel, 0);
    saveRotationBtn = XtVaCreateManagedWidget("save", xmPushButtonWidgetClass,
    	    form,
    	    XmNlabelString, s1=XmStringCreateSimple("Save Viewing Rotation"),
    	    XmNtopAttachment, XmATTACH_POSITION,
    	    XmNtopPosition, 25,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 1, 0);
    Scat3DW = XtVaCreateManagedWidget("scat3D", scat3DWidgetClass, 
                        	 form, 
    	    XmNwidth, 450,
    	    XmNheight, 325,
    	    XmNtopAttachment, XmATTACH_POSITION,
    	    XmNtopPosition, 1,
    	    XmNleftAttachment, XmATTACH_WIDGET,
    	    XmNleftWidget, saveRotationBtn,
    	    XmNrightAttachment, XmATTACH_POSITION,
    	    XmNrightPosition, 99,
    	    XmNbottomAttachment, XmATTACH_WIDGET,
    	    XmNbottomWidget, form, 0);
    XtAddCallback(saveRotationBtn, XmNactivateCallback,
            (XtCallbackProc)saveRotationCB, 
    	    (XtPointer) Scat3DW);
    setRotationBtn = XtVaCreateManagedWidget("setR", xmPushButtonWidgetClass,
    	    form,
    	    XmNlabelString, s1=XmStringCreateSimple("Restore Rotation"),
    	    XmNtopAttachment, XmATTACH_POSITION,
    	    XmNtopPosition, 50,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 1, 0);
    XtAddCallback(setRotationBtn, XmNactivateCallback,
            (XtCallbackProc)setRotationCB, 
    	    (XtPointer) Scat3DW);
    printBtn = XtVaCreateManagedWidget("print", xmPushButtonWidgetClass,
    	    form,
    	    XmNlabelString, s1=XmStringCreateSimple("Print"),
    	    XmNtopAttachment, XmATTACH_POSITION,
    	    XmNtopPosition, 75,
    	    XmNleftAttachment, XmATTACH_POSITION,
    	    XmNleftPosition, 1, 0);
                                       
    XtAddCallback(printBtn, XmNactivateCallback,
            (XtCallbackProc)makePs, 
    	    (XtPointer) Scat3DW);
/*                         	 
    Scat3DSetLimits(Scat3DW, 10., 100.,
    			 200., 2000.,
    			 3000., 30000); 
*/
    Scat3DSetAxesNames(Scat3DW, "This is X axis", 
      "This is a name to mark axes Y", "Z Z Z");  
    XtAddCallback(Scat3DW, XmNbtn3Callback, (XtCallbackProc) makePs, NULL); 
    Scat3DSetContents(Scat3DW, data, nPoints, SCAT3D_RESCALE);   			                   	 
    XtRealizeWidget(toplevel);
    XtMainLoop();
}

static void makePs(Widget w, Widget scat, caddr_t call_data) 
{
   Scat3DPrintContents(scat, "image.ps");
}
static void saveRotationCB(Widget w, Widget scat, caddr_t call_data)
{
	printf (" From save button \n");
	Scat3DGetViewEulerAngles ( scat, &AlphaR, &BetaR, &GammaR);
	printf (" Euler Rotations angles : %g %g %g \n", 
	          AlphaR, BetaR, GammaR);
}

static void setRotationCB(Widget w, Widget scat, caddr_t call_data)
{
	printf (" Setting Rotations angles : %g %g %g \n", 
	          AlphaR, BetaR, GammaR);
	Scat3DSetViewEulerAngles ( scat, AlphaR, BetaR, GammaR);
}


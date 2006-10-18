/*******************************************************************************
*
* LineBoxDemo.c - Demonstrating LineBox.c
*
* Copyright 2005
* Mars Space Flight Facility
* Department of Geological Sciences
* Arizona State University
*
* Modified/maintained by Eric engle <eric.engle@asu.edu>
*
*******************************************************************************/

#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/Separator.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include "../examples/demoUtils.h"
#include "../plot_widgets/LineBox.h"

#define BINS 20

static void flip (Widget w, XtPointer clientData, XtPointer callData);

Widget lineBox;

void
moved (LineBoxWidget w)
{
	printf ("moving!  it's MOVING!!\n");
}

float data[BINS][2];

void
changed (LineBoxWidget w)
{
	int i;
	for (i=0; i<BINS; i++)
		printf ("DN %f to %f\n", data[i][0], LBoxConvertDN(lineBox, data[i][0]));
	printf ("\n\n");
}

void
setHist ()
{
	int i;
	for (i=0; i<BINS; i++)
		data[i][0] = data[i][1] = i;
	LBoxSetHistogram (lineBox, sizeof(data)/sizeof(data[0]), data);
	float left = (float)BINS/4.0;
	float right = 3.0*(float)BINS/4.0;
	XtVaSetValues (lineBox,
		LBoxNshowLeftMask, (Bool)True,
		LBoxNshowRightMask, (Bool)True,
		/* This is my short version of the XLib-recommended hack for setting
		   float values.  I know, it sucks. */
		LBoxNleftMask, *(int*)&left,
		LBoxNrightMask, *(int*)&right,
		NULL);
	printf ("%d\n", sizeof(float));
	//LBoxSetMotionCB (lineBox, moved);
	LBoxSetStretchChangeCB (lineBox, changed);
}

void
showResources (Widget w)
{
	int mode;
	Bool left, right;
	float leftMask, rightMask;
	XtVaGetValues(w,
		"lineMode", &mode,
		"leftMask", &leftMask,
		"rightMask", &rightMask,
		"showLeftMask", &left,
		"showRightMask", &right,
		NULL);
	printf ("mode: %d\n", mode);
	printf ("leftMask: %f\n", leftMask);
	printf ("rightMask: %f\n", rightMask);
	printf ("use left: %d\n", left);
	printf ("use right: %d\n", right);
}

int
main(int argc, char *argv[])
{
	XtAppContext appContext;
	Widget toplevel, form, checkBox, rowColumn, separator;

	toplevel = XtAppInitialize(&appContext, "CurvesTest", NULL, 0, &argc, argv, NULL, NULL, 0);
	form = XtCreateManagedWidget("form", xmFormWidgetClass, toplevel, NULL, 0);

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

	/* Create an LineBox widget. */
	lineBox = XtVaCreateManagedWidget("LineBox Widget",
		lineBoxWidgetClass, form, 
		XmNtopAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 21,
		XmNrightAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_FORM,
		NULL);

	createButton(rowColumn, "changeMode", "Mode", flip, NULL);

	setHist ();

	/* Realize widgets and call XtMainLoop to continuously process events. */
	XtRealizeWidget(toplevel);

	showResources (lineBox);

	XtAppMainLoop(appContext);
	return 0;
}

static void
flip (Widget w, XtPointer clientData, XtPointer callData)
{
	int mode;
	XtVaGetValues(lineBox, "lineMode", &mode, NULL);
	printf ("mode was %d, now ", mode);
	mode = (mode+1)%3;
	printf ("%d\n", mode);
	XtVaSetValues(lineBox, "lineMode", mode, NULL);
}



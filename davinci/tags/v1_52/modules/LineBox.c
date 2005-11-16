/*******************************************************************************
*
* LineBox.c
*
* Simple widget to allow the user to build a stretch function.
*
* Copyright 2005
* Mars Space Flight Facility
* Department of Geological Sciences
* Arizona State University
*
* Modified/maintained by Eric engle <eric.engle@asu.edu>
*
* Features:
* User can add, remove, and move points to build a linear transfer function
* Widget tracks mouse position and nearest point position
* Widget can draw adaptive histogram under stretch function line
* Widget can set masks on either side of the DN range, dampening those ranges
* Widget can take set of points to stretch with stretch function
*
*******************************************************************************/

#include <stdio.h>
#include <math.h>
#include <float.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include "LineBoxP.h"

#define BOX_SIZE 11 /* size of the position indicator box */

/* widget prototypes */
static void destroy(Widget w);
static void resize(Widget w);
static void mouseMove(Widget w, XEvent *event, char *args, int n_args);
static void mouseDrag(Widget w, XEvent *event, char *args, int n_args);
static void mouseDown(Widget w, XEvent *event, char *args, int n_args);
static void mouseUp(Widget w, XEvent *event, char *args, int n_args);
static void initialize(LineBoxWidget request, LineBoxWidget new);
static void redisplay(Widget w, XEvent *event, Region region);
static void redisplayContents(LineBoxWidget w);
static Boolean setValues(LineBoxWidget current, LineBoxWidget request,LineBoxWidget new);

/* point routines */
static void addPoint (LineBoxWidget wid, float x, float y);
static void delPoint (LineBoxWidget wid, unsigned int idx);
static int pointNearest (LineBoxWidget w, float normx, float normy);

/* coordinate transformations */
static inline void screenToNorm (LineBoxWidget w, float *x, float *y);
static inline void normToScreen (LineBoxWidget w, float *x, float *y);
static inline void histToNorm (LineBoxWidget w, float *x, float *y);
static inline void normToHist (LineBoxWidget w, float *x, float *y);
static inline void normToDN (LineBoxWidget w, float *x, float *y);
static inline void dnToNorm (LineBoxWidget w, float *x, float *y);

/* miscelaneous functions */
static void set_color (Display *display, GC gc, char *color);
static int enum_lkup (int *list, int val);
static float stretchDN (Widget w, float x);

/*******************************************************************************
*	Primary LineBox Widget Declarations and Functions
*******************************************************************************/

static char defaultTranslations[] = 
    "<Btn1Motion>: MouseDrag()\n\
     <Motion>: MouseMove()\n\
     <Btn1Down>: MouseDown()\n\
     <Btn1Up>: MouseUp()\n";

static XtActionsRec actionsList[] = {
	{"MouseMove", (XtActionProc)mouseMove},
	{"MouseDrag", (XtActionProc)mouseDrag},
	{"MouseDown", (XtActionProc)mouseDown},
	{"MouseUp", (XtActionProc)mouseUp}
};

/*
 * Valid values for:
 * LBoxNlineMode: 0 (add), 1 (move), 2 (delete)
 * LBoxNleftMask: -FLT_MAX - FLT_MAX
 * LBoxNrightMask: -FLT_MAX - FLT_MAX
 * LBoxNshowLeftMask: False, True
 * LBoxNshowRightMask: False, True
 */

/* used to validate mode values */
static int mode_enums[] = { MODE_ADD, MODE_MOV, MODE_DEL, MODE_SHIFT, -1 };

static XtResource resources[] = {
	{LBoxNlineMode, LBoxClineMode, XtRInt, sizeof(int),
		XtOffset(LineBoxWidget, lineBox.mode), XtRImmediate, (XtPointer)MODE_ADD},
	{LBoxNleftMask, LBoxCleftMask, XtRFloat, sizeof(float),
		XtOffset(LineBoxWidget, lineBox.leftMask), XtRFloat, (XtPointer)0},
	{LBoxNrightMask, LBoxCrightMask, XtRFloat, sizeof(float),
		XtOffset(LineBoxWidget, lineBox.rightMask), XtRFloat, (XtPointer)0},
	{LBoxNshowLeftMask, LBoxCshowLeftMask, XtRBool, sizeof(Bool),
		XtOffset(LineBoxWidget, lineBox.showLeftMask), XtRImmediate, (XtPointer)(False)},
	{LBoxNshowRightMask, LBoxCshowRightMask, XtRBool, sizeof(Bool),
		XtOffset(LineBoxWidget, lineBox.showRightMask), XtRImmediate, (XtPointer)(False)},
	{LBoxNhistXMin, LBoxChistXMin, XtRFloat, sizeof(float),
		XtOffset(LineBoxWidget, lineBox.hist_xMin), XtRFloat, (XtPointer)0},
	{LBoxNhistXMax, LBoxChistXMax, XtRFloat, sizeof(float),
		XtOffset(LineBoxWidget, lineBox.hist_xMax), XtRFloat, (XtPointer)0},
	{LBoxNhistYMin, LBoxChistYMin, XtRFloat, sizeof(float),
		XtOffset(LineBoxWidget, lineBox.hist_yMin), XtRFloat, (XtPointer)0},
	{LBoxNhistYMax, LBoxChistYMax, XtRFloat, sizeof(float),
		XtOffset(LineBoxWidget, lineBox.hist_yMax), XtRFloat, (XtPointer)0},
};

LineBoxClassRec lineBoxClassRec = {
	/* CoreClassPart */
	{
		(WidgetClass)&xmPrimitiveClassRec,/* superclass            */
		"LineBox",                        /* class_name            */
		sizeof(LineBoxRec),               /* widget_size           */
		NULL,                             /* class_initialize      */
		NULL,                             /* class_part_initialize */
		FALSE,                            /* class_inited          */
		(XtInitProc)initialize,           /* initialize            */
		NULL,                             /* initialize_hook       */
		XtInheritRealize,                 /* realize               */
		actionsList,                      /* actions               */
		XtNumber(actionsList),            /* num_actions           */
		resources,                        /* resources             */
		XtNumber(resources),              /* num_resources         */
		NULLQUARK,                        /* xrm_class             */
		TRUE,                             /* compress_motion       */
		TRUE,                             /* compress_exposure     */
		TRUE,                             /* compress_enterleave   */
		TRUE,                             /* visible_interest      */
		destroy,                          /* destroy               */
		resize,                           /* resize                */
		redisplay,                        /* expose                */
		(XtSetValuesFunc)setValues,       /* set_values            */
		NULL,                             /* set_values_hook       */
		XtInheritSetValuesAlmost,         /* set_values_almost     */
		NULL,                             /* get_values_hook       */
		NULL,                             /* accept_focus          */
		XtVersion,                        /* version               */
		NULL,                             /* callback private      */
		defaultTranslations,              /* tm_table              */
		NULL,                             /* query_geometry        */
		NULL,                             /* display_accelerator   */
		NULL,                             /* extension             */
	},
	/* Motif primitive class fields */
	{
		(XtWidgetProc)_XtInherit, 		  /* Primitive border_highlight   */
		(XtWidgetProc)_XtInherit,		  /* Primitive border_unhighlight */
		XtInheritTranslations,		      /* translations                 */
		(XtActionProc)mouseDown,		  /* arm_and_activate             */
		NULL,							  /* get resources      		  */
		0,								  /* num get_resources  		  */
		NULL,         					  /* extension                    */
	},
	/* LineBox class part */
	{
		0,                                /* ignored	                  */
	}
};

WidgetClass lineBoxWidgetClass = (WidgetClass) &lineBoxClassRec;

/* Widget initialize method */
static void
initialize (LineBoxWidget request, LineBoxWidget new)
{
	XGCValues values;
	Display *display = XtDisplay(new);

	/* Make sure the window size is not zero. The Core 
	   initialize() method doesn't do this. */
	if (request->core.width == 0)
		new->core.width = 250;
	if (request->core.height == 0)
		new->core.height = 250;

	/* Create graphics contexts for drawing in the widget */
	values.foreground = new->primitive.foreground;
	values.background = new->core.background_pixel;
	new->lineBox.contentsGC = XCreateGC(display, XDefaultRootWindow(display),
		GCForeground|GCBackground, &values);

	/* set default stretch function points */
	new->lineBox.pPoints = NULL;
	LBoxResetPoints ((Widget)new);
	new->lineBox.dragPoint = -1;

	/* set default histogram values */
	new->lineBox.pBins = NULL;
	new->lineBox.hist_xMin = 0;
	new->lineBox.hist_xMax = 1;
	new->lineBox.hist_yMin = 0;
	new->lineBox.hist_yMax = 1;

	/* set default callback values */
	new->lineBox.motionCB = NULL;
	new->lineBox.changeCB = NULL;

	/* Set size dependent items */
	new->lineBox.drawBuffer = 0;
	resize((Widget)new);
}

/* Widget destroy method */
static void
destroy (Widget w)
{
	LineBoxWidget hw = (LineBoxWidget)w;
	XFreeGC(XtDisplay(w), hw->lineBox.contentsGC);
	XFreePixmap(XtDisplay(hw), hw->lineBox.drawBuffer);
	if (hw->lineBox.pPoints != NULL)
		free (hw->lineBox.pPoints);
	if (hw->lineBox.pBins != NULL)
		free (hw->lineBox.pBins);
	if (hw->lineBox.pMappedPoints != NULL)
		free (hw->lineBox.pMappedPoints);
}

/* Widget resize method */
static void
resize (Widget w)
{
	LineBoxWidget hw = (LineBoxWidget)w;

	int borderWidth =
		hw->primitive.shadow_thickness + hw->primitive.highlight_thickness;
	XRectangle clipRect;

	/* resize the drawing buffer, an offscreen pixmap for smoother animation */
	if (hw->lineBox.drawBuffer)
		XFreePixmap(XtDisplay(hw), hw->lineBox.drawBuffer);
	hw->lineBox.drawBuffer = XCreatePixmap(XtDisplay(hw),
		DefaultRootWindow(XtDisplay(hw)), hw->core.width, hw->core.height,
		DefaultDepthOfScreen(XtScreen(hw)));

	/* calculate the area of the widget where contents can be drawn */
	hw->lineBox.xMin = borderWidth;
	hw->lineBox.yMin = borderWidth;
	hw->lineBox.xMax = w->core.width - borderWidth;
	hw->lineBox.yMax = w->core.height - borderWidth;

	/* set plot contents gc to clip drawing at the edges */
	clipRect.x = hw->lineBox.xMin;
	clipRect.y = hw->lineBox.yMin;
	clipRect.width = hw->lineBox.xMax - hw->lineBox.xMin + 1;
	clipRect.height = hw->lineBox.yMax - hw->lineBox.yMin + 1;
	XSetClipRectangles(XtDisplay(w), hw->lineBox.contentsGC, 0, 0, &clipRect,
		1, Unsorted);
}

/* Widget redisplay method */
static void
redisplay (Widget w, XEvent *event, Region region)
{
	LineBoxWidget hw = (LineBoxWidget)w;

	/* Draw the Motif required shadows and highlights */
	if (hw->primitive.shadow_thickness > 0) {
		_XmDrawShadow (XtDisplay(w), XtWindow(w), 
			hw->primitive.bottom_shadow_GC, hw->primitive.top_shadow_GC,
			hw->primitive.shadow_thickness, hw->primitive.highlight_thickness,
			hw->primitive.highlight_thickness,
			hw->core.width  - 2 * hw->primitive.highlight_thickness,
			hw->core.height - 2 * hw->primitive.highlight_thickness);
	}

	if (hw->primitive.highlighted)
		_XmHighlightBorder(w);
	else if (_XmDifferentBackground(w, XtParent((Widget)w)))
		_XmUnhighlightBorder(w);

	/* Now draw the contents of the lineBox widget */
	redisplayContents((LineBoxWidget)w);
}

/* expose action callback */
/* Does not redisplay the motif shadows and highlights. */
static void
redisplayContents (LineBoxWidget w)
{
	Display *display = XtDisplay(w);
	GC gc = w->lineBox.contentsGC;
	Drawable drawBuf;
	int i;
	XSegment *segs;
	float x, y; /* temporary storage for stretch line drawing */
	int near; /* index of nearest point */
	float px, py, qx, qy; /* corners for rectangle drawing */
	int recx, recy, recwidth, recheight;

	/* Save some energy if the widget isn't visible or realized */
	if (!w->core.visible || !XtIsRealized((Widget)w))
		return;

	/* Set destination for drawing commands, offscreen pixmap or window */
	drawBuf = w->lineBox.drawBuffer;

	/* Overwrite drawing area with background pixel color */
	set_color (display, gc, "white");
	XFillRectangle(display, drawBuf, gc, w->lineBox.xMin, w->lineBox.yMin,
		w->lineBox.xMax - w->lineBox.xMin + 1, w->lineBox.yMax - w->lineBox.yMin + 1);

	/* TODO: use overlays to avoid redrawing big histograms all the time */

	/* Draw Histogram */
	set_color (display, gc, "blue");
	for (i=0; i<w->lineBox.nBins; i++) {
		/* upper-left corner */
		px = w->lineBox.pBins[i].x;
		py = w->lineBox.pBins[i].y;
		histToNorm (w, &px, &py);
		normToScreen (w, &px, &py);
		/* lower-right corner */
		if (i < w->lineBox.nBins-1) {
			/* use next bin's starting position as ending for this one */
			qx = w->lineBox.pBins[i+1].x;
		} else if (i > 0) {
			/* continue previous width */
			qx = 2*w->lineBox.pBins[i].x - w->lineBox.pBins[i-1].x;
		} else {
			qx = w->lineBox.hist_xMax;
		}
		histToNorm (w, &qx, NULL);
		qy = 0;
		normToScreen (w, &qx, &qy);
		/* draw rectangle in foreground color */
		recx = (int)px;
		recy = (int)py;
		recheight = (int)qy - (int)py;
		recwidth = (int)qx - (int)px + 1;
		XFillRectangle (display, drawBuf, gc, recx, recy, recwidth, recheight);
	}

	/* Draw Masks */
	set_color (display, gc, "gray");
	if (w->lineBox.showLeftMask == True
	&&  w->lineBox.leftMask >= w->lineBox.hist_xMin) {
		px = 0;
		py = 1;
		normToScreen (w, &px, &py);
		qx = w->lineBox.leftMask;
		histToNorm (w, &qx, NULL);
		qy = 0;
		normToScreen (w, &qx, &qy);
		XFillRectangle (display, drawBuf, gc, px, py, qx-px+1, qy-py+1);
	}
	if (w->lineBox.showRightMask == True
	&&  w->lineBox.rightMask <= w->lineBox.hist_xMax) {
		px = w->lineBox.rightMask;
		histToNorm (w, &px, NULL);
		py = 1;
		normToScreen (w, &px, &py);
		qx = 1;
		qy = 0;
		normToScreen (w, &qx, &qy);
		XFillRectangle (display, drawBuf, gc, px, py, qx-px+1, qy-py+1);
	}

	/* Draw function line */
	set_color (display, gc, "red");
	segs = (XSegment*) XtMalloc ((w->lineBox.nPoints - 1) * sizeof(segs[0]));
	for (i=0; i<w->lineBox.nPoints-1; i++) {
		x = w->lineBox.pPoints[i].x;
		y = w->lineBox.pPoints[i].y;
		normToScreen (w, &x, &y);
		segs[i].x1 = x;
		segs[i].y1 = y;
		x = w->lineBox.pPoints[i+1].x;
		y = w->lineBox.pPoints[i+1].y;
		normToScreen (w, &x, &y);
		segs[i].x2 = x;
		segs[i].y2 = y;
	}
	XDrawSegments (display, drawBuf, gc, segs, w->lineBox.nPoints-1);
	XtFree ((char *)segs);

	/* Draw Position Indicator */
	near = pointNearest (w, w->lineBox.pointer_x, w->lineBox.pointer_y);
	if (near != -1) {
		LBoxPoint_t p = w->lineBox.pPoints[near];
		normToScreen (w, &p.x, &p.y);
		set_color (display, gc, "black");
		segs = (XSegment*) XtMalloc (4 * sizeof(segs[0]));
		segs[0].x1 = p.x - (BOX_SIZE-1)/2;
		segs[0].y1 = p.y - (BOX_SIZE-1)/2;
		segs[0].x2 = p.x - (BOX_SIZE-1)/2;
		segs[0].y2 = p.y + (BOX_SIZE-1)/2;
		segs[1].x1 = p.x - (BOX_SIZE-1)/2;
		segs[1].y1 = p.y + (BOX_SIZE-1)/2;
		segs[1].x2 = p.x + (BOX_SIZE-1)/2;
		segs[1].y2 = p.y + (BOX_SIZE-1)/2;
		segs[2].x1 = p.x + (BOX_SIZE-1)/2;
		segs[2].y1 = p.y + (BOX_SIZE-1)/2;
		segs[2].x2 = p.x + (BOX_SIZE-1)/2;
		segs[2].y2 = p.y - (BOX_SIZE-1)/2;
		segs[3].x1 = p.x + (BOX_SIZE-1)/2;
		segs[3].y1 = p.y - (BOX_SIZE-1)/2;
		segs[3].x2 = p.x - (BOX_SIZE-1)/2;
		segs[3].y2 = p.y - (BOX_SIZE-1)/2;
		XDrawSegments (display, drawBuf, gc, segs, 4);
		XtFree ((char *)segs);
	}

	/* Flash drawing buffer to screen */
	XCopyArea(display, drawBuf, XtWindow(w), gc, 0, 0,
		w->core.width, w->core.height, 0, 0);
}

/* Widget setValues method */
static Boolean
setValues (LineBoxWidget current, LineBoxWidget request, LineBoxWidget new)
{
	Boolean redraw = False;
	Display *display = XtDisplay(new);

	/* If the foreground or background color has changed, change the GCs */
	if (new->core.background_pixel !=current->core.background_pixel) {
		XSetForeground(display, new->lineBox.contentsGC, new->primitive.foreground);
		redraw = TRUE;
	}

	if (new->primitive.foreground != current->primitive.foreground) {
		XSetBackground(display, new->lineBox.contentsGC, new->core.background_pixel);
		redraw = TRUE;
	}

	/* if highlight thickness or shadow thickness changed, resize and redraw */
	if (new->primitive.highlight_thickness != current->primitive.highlight_thickness
	||  new->primitive.shadow_thickness    != current->primitive.shadow_thickness) {
		redraw = TRUE;
	}

	/* if a mask changed, redraw */
	if (new->lineBox.leftMask != current->lineBox.leftMask
	||  new->lineBox.rightMask != current->lineBox.rightMask
	||  new->lineBox.showLeftMask != current->lineBox.showLeftMask
	||  new->lineBox.showRightMask != current->lineBox.showRightMask) {
		redraw = TRUE;
	}
	
	/* if a portion of the histogram view changed, redraw */
	if (new->lineBox.hist_xMin != current->lineBox.hist_xMin
	||  new->lineBox.hist_xMax != current->lineBox.hist_xMax
	||  new->lineBox.hist_yMin != current->lineBox.hist_yMin
	||  new->lineBox.hist_yMax != current->lineBox.hist_yMax) {
		redraw = TRUE;
	}

	/* replace invalid modes with old setting */
	if (new->lineBox.mode != current->lineBox.mode) {
		if (-1 == enum_lkup (mode_enums, new->lineBox.mode)) {
			new->lineBox.mode = current->lineBox.mode;
		}
	}

	return redraw; 
}

/*	button down action callback
	Does something different for each LineBox mode:
	add:	if x position unoccupied, add point & enter drag mode until release
	remove:	remove nearest boxed point
	move:	grab nearest boxed point, enter drag mode until release
*/
static void
mouseDown (Widget w, XEvent *event, char *args, int n_args)
{
	Boolean inBox = False, changed = False;
	int near;
	float one_x;
	float normx, normy, mousex, mousey;

	LineBoxWidget hw = (LineBoxWidget)w;

	/* be nice, give everyone a chance to take focus */
	XmProcessTraversal((Widget)w, XmTRAVERSE_CURRENT);

	mousex = event->xbutton.x;
	mousey = event->xbutton.y;

	/* if point is not in clickable area, get out */
	if (mousex < hw->lineBox.xMin || mousex > hw->lineBox.xMax
	||  mousey < hw->lineBox.yMin || mousey > hw->lineBox.yMax) {
		return;
	}

	normx = mousex;
	normy = mousey;
	screenToNorm (hw, &normx, &normy);

	near = pointNearest (hw, normx, normy);
	if (near != -1) {
		float nearx, neary;
		nearx = hw->lineBox.pPoints[near].x;
		neary = hw->lineBox.pPoints[near].y;
		normToScreen (hw, &nearx, &neary);
		if (fabs(nearx-mousex) <= (BOX_SIZE-1)/2
		&&  fabs(neary-mousey) <= (BOX_SIZE-1)/2) {
			inBox = True;
		}
	}

	/* proceed based on widget mode */
	switch (hw->lineBox.mode) {
	case MODE_ADD:
		/* make sure point is not sharing x value, and is in screen */
		one_x = 1.0;
		screenToNorm (hw, &one_x, NULL);
		if (near == -1
		||  fabs(hw->lineBox.pPoints[near].x - normx) > one_x) {
			/* get screen coordinates, transform them into normalized GC coordinates */
			addPoint (hw, normx, normy);

			/* will drag the near point until mouse is released */
			hw->lineBox.dragPoint = pointNearest (hw, normx, normy);

			changed = True;
		}
		break;
	case MODE_MOV:
		/* drag nearest point if it's within the box */
		if (inBox) {
			hw->lineBox.dragPoint = near;
		}
		break;
	case MODE_DEL:
		/* remove nearest point if it's within the box and not an end point */
		if (inBox && near != 0 && near != hw->lineBox.nPoints-1) {
			delPoint (hw, near);

			changed = True;
		}
		break;
	case MODE_SHIFT:
		/* this is handled entirely in mouseDrag */
		break;
	}

	if (changed) {
		/* call stretch function change callback if it's defined */
		if (hw->lineBox.changeCB != NULL) {
			hw->lineBox.changeCB(w);
		}
	}

	redisplayContents(hw);
}

/* mouse motion action callback - handle normal motion */
static void
mouseMove (Widget w, XEvent *event, char *args, int n_args)
{
	float normx, normy;
	int nearMouse, nearOld;
	LineBoxWidget hw = (LineBoxWidget)w;
	normx = event->xbutton.x;
	normy = event->xbutton.y;
	screenToNorm (hw, &normx, &normy);
	if (normx >= 0.0 && normx <= 1.0 && normy >= 0.0 && normy <= 1.0) {
		nearMouse = pointNearest (hw, normx, normy);
		nearOld = pointNearest (hw, hw->lineBox.pointer_x, hw->lineBox.pointer_y);

		/* update box position */
		if (nearMouse != -1) {
			hw->lineBox.box_x = hw->lineBox.pPoints[nearMouse].x;
			hw->lineBox.box_y = hw->lineBox.pPoints[nearMouse].y;
		}

		/* update pointer position */
		hw->lineBox.pointer_x = normx;
		hw->lineBox.pointer_y = normy;

		/* redraw screen if box moved */
		if (nearMouse != nearOld) {
			redisplayContents (hw);
		}

		/* call mouse motion callback if defined */
		if (hw->lineBox.motionCB != NULL) {
			hw->lineBox.motionCB(w);
		}
	}
}

/* button motion action callback - handle drag motion */
static void
mouseDrag (Widget w, XEvent *event, char *args, int n_args)
{
	int i, drag;
	Boolean changed = False;
	float normx, normy, xMin, xMax, yMin, yMax;
	LineBoxWidget hw = (LineBoxWidget)w;

	/* calculate mouse position */
	normx = event->xbutton.x;
	normy = event->xbutton.y;
	screenToNorm (hw, &normx, &normy);

	drag = hw->lineBox.dragPoint;

	/* if we're dragging a point */
	if (drag != -1) {
		/* calculate boundaries */
		yMin = 0;
		yMax = 1;
		if (drag == 0) {
			/* first point, vertical motion only */
			xMin = 0;
			xMax = 0;
		} else if (drag == hw->lineBox.nPoints-1) {
			/* last point, vertical motion only */
			xMin = 1;
			xMax = 1;
		} else {
			/* a middle point, caged by neighbors */
			xMin = hw->lineBox.pPoints[drag-1].x;
			xMax = hw->lineBox.pPoints[drag+1].x;
			normToScreen (hw, &xMin, NULL);
			normToScreen (hw, &xMax, NULL);
			xMin += 1.0;
			xMax -= 1.0;
			screenToNorm (hw, &xMin, NULL);
			screenToNorm (hw, &xMax, NULL);
		}

		/* constrain dragged point to remain within boundaries */
		if (normx > xMax) normx = xMax;
		if (normx < xMin) normx = xMin;
		if (normy > yMax) normy = yMax;
		if (normy < yMin) normy = yMin;

		/* update the point we're dragging */
		hw->lineBox.pPoints[drag].x = normx;
		hw->lineBox.pPoints[drag].y = normy;

		changed = True;
	}
	/* else if we're in shift mode */
	else if (hw->lineBox.mode == MODE_SHIFT) {
		/* adjust all points by change in normalized mouse x position */
		for (i = 0; i < hw->lineBox.nPoints; i++) {
			hw->lineBox.pPoints[i].x += normx - hw->lineBox.pointer_x;
		}
		changed = True;

		/* make sure we always have a point at normalized (0,0) */
		if (hw->lineBox.pPoints[0].x > 0) {
			if (hw->lineBox.pPoints[0].y == hw->lineBox.pPoints[1].y) {
				/* extend horizontal lines */
				hw->lineBox.pPoints[0].x = 0;
			} else {
				/* add a new point otherwise */
				addPoint (hw, 0, 0);
			}
		}

		/* make sure we always have a point at normalized (1,1) */
		if (hw->lineBox.pPoints[hw->lineBox.nPoints-1].x < 1) {
			if (hw->lineBox.pPoints[hw->lineBox.nPoints-2].y
			==  hw->lineBox.pPoints[hw->lineBox.nPoints-1].y) {
				/* extend horizontal lines */
				hw->lineBox.pPoints[hw->lineBox.nPoints-1].x = 1;
			} else {
				/* add a new point otherwise */
				addPoint (hw, 1, 1);
			}
		}
	}

	/* <motion> events don't fire while dragging, so update pointer_x/y */
	hw->lineBox.pointer_x = normx;
	hw->lineBox.pointer_y = normy;

	if (changed) {
		/* call stretch function change callback if it's defined */
		if (hw->lineBox.changeCB != NULL) {
			hw->lineBox.changeCB(w);
		}
		redisplayContents(hw);
	}
}

/* button up action callback */
static void
mouseUp (Widget w, XEvent *event, char *args, int n_args)
{
	LineBoxWidget hw = (LineBoxWidget)w;
	hw->lineBox.dragPoint = -1;
}

/*******************************************************************************
*	Point routines
*******************************************************************************/

static int
compPoint (void *a, void *b)
{
	LBoxPoint_t *p = a, *q = b;
	if (p->x < q->x)
		return -1;
	else if (p->x > q->x)
		return 1;
	else
		return 0;
}

static void
addPoint (LineBoxWidget wid, float x, float y)
{
	wid->lineBox.pPoints = (LBoxPoint_t *) realloc (wid->lineBox.pPoints,
		sizeof(wid->lineBox.pPoints[0])*(wid->lineBox.nPoints + 1));
	wid->lineBox.pPoints[wid->lineBox.nPoints].x = x;
	wid->lineBox.pPoints[wid->lineBox.nPoints].y = y;
	wid->lineBox.nPoints ++;
	qsort (wid->lineBox.pPoints, wid->lineBox.nPoints,
		sizeof(wid->lineBox.pPoints[0]), compPoint);
}

static void
delPoint (LineBoxWidget wid, unsigned int idx)
{
	if (idx < wid->lineBox.nPoints) {
		if (idx < wid->lineBox.nPoints - 1) {
			memmove (
				wid->lineBox.pPoints + idx,
				wid->lineBox.pPoints + (idx+1),
				(wid->lineBox.nPoints - idx)
					* sizeof(wid->lineBox.pPoints[0]));
		}
		if (wid->lineBox.nPoints > 0) {
			wid->lineBox.nPoints --;
		}
	}
}

#define SQ(a) ((a)*(a))

static int
pointNearest (LineBoxWidget w, float normx, float normy)
{
	int i, closest = -1;
	float best = FLT_MAX, dist;
	for (i=0; i<w->lineBox.nPoints; i++) {
		dist = SQ(w->lineBox.pPoints[i].x - normx)
			 + SQ(w->lineBox.pPoints[i].y - normy);
		if (dist < best) {
			best = dist;
			closest = i;
		}
	}
	return closest;
}

/*******************************************************************************
*	Coordinate Transformations
*******************************************************************************/

/* map a screen point, such as from the mouse position, into normalized space */
static inline void
screenToNorm (LineBoxWidget w, float *x, float *y)
{
	if (x)
		*x = (*x - (float)w->lineBox.xMin)
			/ (float)(w->lineBox.xMax - w->lineBox.xMin);
	if (y)
		*y = 1.0 - (*y - (float)w->lineBox.yMin)
			/ (float)(w->lineBox.yMax - w->lineBox.yMin);
}

/* map a normalized point into current screen cooardinates */
static inline void
normToScreen (LineBoxWidget w, float *x, float *y)
{
	if (x) {
		*x = (*x) * (float)w->lineBox.xMax - (*x - 1.0) * (float)w->lineBox.xMin;
	}
	if (y) {
		*y = (1.0 - *y) * (float)w->lineBox.yMax + (*y) * (float)w->lineBox.yMin;
	}
}

/* maps a histogram point into normalized space */
static inline void
histToNorm (LineBoxWidget w, float *x, float *y)
{
	if (x) {
		if (w->lineBox.hist_xMin != w->lineBox.hist_xMax)
			*x = (*x - w->lineBox.hist_xMin) / (w->lineBox.hist_xMax - w->lineBox.hist_xMin);
		else
			*x = 0.5;
	}
	if (y) {
		if (w->lineBox.hist_yMin != w->lineBox.hist_yMax)
			*y = (*y - w->lineBox.hist_yMin) / (w->lineBox.hist_yMax - w->lineBox.hist_yMin);
		else
			*y = 0.5;
	}
}

/* maps a normalized point into the histogram space */
static inline void
normToHist (LineBoxWidget w, float *x, float *y)
{
	if (x) {
		*x = (*x) * w->lineBox.hist_xMax - (*x - 1) * w->lineBox.hist_xMin;
	}
	if (y) {
		*y = (*y) * w->lineBox.hist_yMax - (*y - 1) * w->lineBox.hist_yMin;
	}
}

/* maps a normalized point into the histogram's DN/DN space */
static inline void
normToDN (LineBoxWidget w, float *x, float *y)
{
	normToHist (w, x, NULL);
	normToHist (w, y, NULL);
}

/* maps DN/DN point into normalized space */
static inline void
dnToNorm (LineBoxWidget w, float *x, float *y)
{
	histToNorm (w, x, NULL);
	histToNorm (w, y, NULL);
}

/*******************************************************************************
*	General Functions
*******************************************************************************/

/* TODO: This seems really retarded... shouldn't we use a colormap, or something?
   Does this have issues in the broader usage of X? Across servers, etc? */
static void
set_color (Display *display, GC gc, char *color)
{
	static struct {
		Pixel pix;
		char *name;
	} color_set[] = {
		{-1, "black"},
		{-1, "white"},
		{-1, "red"},
		{-1, "blue"},
		{-1, "gray"},
		{-1, NULL}
	};
	static int first = 1;

	int i;
	XColor col;
	Colormap map;

	if (first) {
		int result;
		first = 0;
		map = DefaultColormap(display,DefaultScreen(display));
		for (i=0; color_set[i].name != NULL; i++) {
			result = XParseColor (display, map, color_set[i].name, &col); 
			result = XAllocColor (display, map, &col);
			color_set[i].pix = col.pixel;
		}
	}

	for (i=0; color_set[i].name != NULL; i++) {
		if (!strcmp (color, color_set[i].name)) {
			if (color_set[i].pix != -1) {
				XSetForeground (display, gc, color_set[i].pix);
			}
			break;
		}
	}
}

/* General enumeration lookup routine */
static int enum_lkup (int *list, int val)
{
	int i;
	for (i=0; list[i] != -1; i++)
		if (list[i] == val)
			return i;
	return -1;
}

/* returns a single DN stretched by the stretch function line */
static float
stretchDN (Widget w, float x)
{
	float m, b, val, original;
	int i;
	LBoxPoint_t p, q;
	LineBoxWidget hw = (LineBoxWidget)w;

	/* if point is masked, return low value */
	if ((hw->lineBox.showLeftMask && x <= hw->lineBox.leftMask)
	||	(hw->lineBox.showRightMask && x >= hw->lineBox.rightMask)) {
		return hw->lineBox.hist_xMin;
	}

	/* otherwise interpolate new value from points around old one */
	original = x;
	dnToNorm (hw, &x, NULL);
	for (i=0; i<hw->lineBox.nPoints-1; i++) {
		p = hw->lineBox.pPoints[i+0];
		q = hw->lineBox.pPoints[i+1];
		if (x >= p.x && (x < q.x || i==hw->lineBox.nPoints-2)) {
			m = (q.y - p.y) / (q.x - p.x);
			b = p.y - m*p.x;
			val = m*x + b;
			normToDN (hw, &val, NULL);

			/* trim tiny rounding errors that put last point outside view */
			if (val < hw->lineBox.hist_xMin) val = hw->lineBox.hist_xMin;
			if (val > hw->lineBox.hist_xMax) val = hw->lineBox.hist_xMax;

			return val;
		}
	}
	return original;
}

/*******************************************************************************
*	Public Functions
*******************************************************************************/

/* XLib recommended hack for setting float values. */
/* This is really, really uncool */
/* There is another way, perhaps equally nasty:
   char *float_name = "SomeFloatResource"; float float_value = 45.0;
   XtVaSetArgs (widgetID, float_name, *(int*)&float_value, NULL); */
void LBoxSetFloatValue (Widget w, char *name, float val)
{
	Arg arg;
	union {
		int int_val;
		float float_val;
	} u;
	u.float_val = val;
	XtSetArg (arg, name, u.int_val);
	XtSetValues (w, (ArgList)&arg, 1);
}

/* Set histogram data to display behind the stretch line */
void
LBoxSetHistogram (Widget w, int nBins, LBoxPoint_t *pBins)
{
	LineBoxWidget hw = (LineBoxWidget)w;

	/* copy data into widget */
	hw->lineBox.nBins = nBins;
	if (hw->lineBox.pBins != NULL)
		free (hw->lineBox.pBins);
	hw->lineBox.pBins = (LBoxPoint_t *) calloc (nBins, sizeof(hw->lineBox.pBins[0]));
	memcpy (hw->lineBox.pBins, pBins, nBins*sizeof(hw->lineBox.pBins[0]));

	redisplayContents (hw);
}

/* resets the stretch function to the two original points */
void
LBoxResetPoints (Widget w)
{
	LineBoxWidget lw = (LineBoxWidget)w;
	if (lw->lineBox.pPoints != NULL) {
		free (lw->lineBox.pPoints);
		lw->lineBox.pPoints = NULL;
	}
	lw->lineBox.nPoints = 0;

	addPoint (lw, 0,0);
	addPoint (lw, 1,1);

	redisplayContents (lw);

#if 0
	/* stretch function obviously changed, so call change callback */
	if (lw->lineBox.changeCB != NULL) {
		lw->lineBox.changeCB(w);
	}
#endif
}

/* callback to fire when the mouse moves within the drawable area */
void
LBoxSetMotionCB (Widget w, void (*cb)(Widget))
{
	((LineBoxWidget)w)->lineBox.motionCB = cb;
}

/* callback to fire when the stretch function changes */
void
LBoxSetStretchChangeCB (Widget w, void (*cb)(Widget))
{
	((LineBoxWidget)w)->lineBox.changeCB = cb;
}

/* sets the x and y arguments to the current DN/DN position of the pointer */
void
LBoxGetPointerDN (Widget w, float *x, float *y)
{
	*x = ((LineBoxWidget)w)->lineBox.pointer_x;
	*y = ((LineBoxWidget)w)->lineBox.pointer_y;
	normToDN ((LineBoxWidget)w, x, y);
}

/* gets number of points and copy of widget's points, in normalized space */
void
LBoxGetPointsNorm (Widget w, int *pNum, LBoxPoint_t **ppPoints)
{
	int num;
	LBoxPoint_t *copy, *points;

	num = ((LineBoxWidget)w)->lineBox.nPoints;
	points = ((LineBoxWidget)w)->lineBox.pPoints;

	copy = (LBoxPoint_t *) calloc (num, sizeof(copy[0]));
	memcpy (copy, points, num * sizeof(copy[0]));

	*ppPoints = copy;
	*pNum = num;
}

/* set points in normalized space */
void
LBoxSetPointsNorm (Widget w, int nPoints, LBoxPoint_t *pPoints)
{
	int i;
	LineBoxWidget lw = (LineBoxWidget)w;
	LBoxPoint_t *copy;

	/* make sure we have points */
	if (nPoints <= 0 || pPoints == NULL) {
		return;
	}

	/* create and sort a copy of the points */
	copy = (LBoxPoint_t *) calloc (nPoints, sizeof(copy[0]));
	memcpy (copy, pPoints, nPoints * sizeof(copy[0]));
	qsort (copy, nPoints, sizeof(copy[0]), compPoint);

	/* first and last points must be at extreme positions */
	if (copy[0].x != 0.0 || copy[nPoints-1].x != 1.0) {
		free (copy);
		return;
	}

	/* range check the points */
	for (i=0; i<nPoints; i++) {
		if (copy[i].x < 0 || copy[i].x > 1
		||  copy[i].y < 0 || copy[i].y > 1) {
			free (copy);
			return;
		}
	}

	/* replace existing points */
	if (lw->lineBox.pPoints != NULL) {
		free (lw->lineBox.pPoints);
	}
	lw->lineBox.nPoints = nPoints;
	lw->lineBox.pPoints = copy;
}

/* returns a copy of the mapped points */
void
LBoxGetMappedPoints (Widget w, int *pNum, LBoxPoint_t **ppPoints)
{
	int i;
	LineBoxWidget lw = (LineBoxWidget)w;
	*pNum = lw->lineBox.nMappedPoints;
	*ppPoints = (LBoxPoint_t *) calloc (*pNum, sizeof((*ppPoints)[0]));
	for (i=0; i<(*pNum); i++) {
		(*ppPoints)[i].x = lw->lineBox.pMappedPoints[i];
		(*ppPoints)[i].y = stretchDN (w, (*ppPoints)[i].x);
	}
}

/* sets an array of DNs to map with the stretch function */
/* points are mapped ONLY when retrieved */
void
LBoxSetMappedPoints (Widget w, int nPoints, float *pPoints)
{
	LineBoxWidget lw = (LineBoxWidget)w;
	float *copy;

	/* make sure we have points */
	if (nPoints <= 0 || pPoints == NULL) {
		return;
	}

	/* create copy to store internally */
	copy = (float *) calloc (nPoints, sizeof(copy[0]));
	memcpy (copy, pPoints, nPoints * sizeof(copy[0]));

	/* replace existing points */
	if (lw->lineBox.pMappedPoints != NULL) {
		free (lw->lineBox.pMappedPoints);
	}
	lw->lineBox.nMappedPoints = nPoints;
	lw->lineBox.pMappedPoints = copy;
}


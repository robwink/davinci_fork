/*******************************************************************************
* LineBoxP.h - Private header file
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
*******************************************************************************/

#ifndef LineBoxP_H
#define LineBoxP_H

#include "LineBox.h"
#include <Xm/XmP.h>
#if XmVersion >= 2000
#include <X11/CoreP.h>
#include <Xm/PrimitiveP.h>
#endif

/*******************************************************************************
*	Privately-required Definitions
*******************************************************************************/

/* values assignable to _LineBoxPart.mode */
#define MODE_ADD 0
#define MODE_MOV 1
#define MODE_DEL 2

/*******************************************************************************
*	Publically-required Definitions
*******************************************************************************/

typedef struct _LineBoxClassPart{
	int ignore;
} LineBoxClassPart;

typedef struct _LineBoxClassRec{
	CoreClassPart  core_class;
	XmPrimitiveClassPart primitive_class;
	LineBoxClassPart  lineBox_class;
} LineBoxClassRec;

extern LineBoxClassRec lineBoxClassRec;

typedef struct enum_t {
	char *name;
	int val;
} enum_t;

typedef struct _LineBoxPart {
	/* mouse */
	int mode; /* whether mouse click adds, moves, or deletes points */
	float pointer_x; /* position of mouse pointer in normalized coordinates */
	float pointer_y;
	float box_x;     /* position of highlight box in normalized coordinates */
	float box_y;
	int dragPoint; /* index of point being dragged (while button held down) */

	/* screen */
	GC contentsGC;		/* graphics context for plot contents */
	Pixmap drawBuffer;		/* double buffering for non-flashing draws */
	int xMin, yMin, xMax, yMax;	/* boundaries of the drawable area of widget */

	/* stretch line */
	int nPoints; /* number of points, always at least 2 (one for each edge) */
	LBoxPoint_t *pPoints; /* array of x,y coordinates in range 0,1 */

	/* histogram */
	int nBins;         /* data for histogram, x= start position, y= intensity */
	LBoxPoint_t *pBins;
	float histX1, histX2, histY1, histY2; /* boundaries of histogram space */
	Bool showLeftMask, showRightMask; /* indicates if left or right masks have been set */
	float leftMask, rightMask; /* occludes x<=leftMask and x>=rightMask */

	/* callbacks */
	void (*motionCB)(LineBoxWidget); /* called when mouse moves */
	void (*changeCB)(LineBoxWidget); /* called when stretch function changes */
} LineBoxPart;

typedef struct _LineBoxRec {
	CorePart        core;
	XmPrimitivePart primitive;
	LineBoxPart        lineBox;
} LineBoxRec;

#endif

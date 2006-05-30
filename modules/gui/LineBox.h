/*******************************************************************************
* LineBox.h
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

#ifndef  LineBox_H
#define  LineBox_H

/* Resource strings */

#define LBoxNlineMode "lineMode"
#define LBoxClineMode "LBoxLineMode"

#define LBoxNleftMask "leftMask"
#define LBoxCleftMask "LBoxLeftMask"

#define LBoxNrightMask "rightMask"
#define LBoxCrightMask "LBoxRightMask"

#define LBoxNshowLeftMask "showLeftMask"
#define LBoxCshowLeftMask "LBoxShowMask"

#define LBoxNshowRightMask "showRightMask"
#define LBoxCshowRightMask "LBoxShowMask"

#define LBoxNhistXMin "histXMin"
#define LBoxChistXMin "HistXMin"

#define LBoxNhistXMax "histXMax"
#define LBoxChistXMax "HistXMax"

#define LBoxNhistYMin "histYMin"
#define LBoxChistYMin "HistYMin"

#define LBoxNhistYMax "histYMax"
#define LBoxChistYMax "HistYMax"

typedef struct LBoxPoint_t {
	float x;
	float y;
} LBoxPoint_t;

extern WidgetClass lineBoxWidgetClass;

typedef struct _LineBoxClassRec *LineBoxWidgetClass;
typedef struct _LineBoxRec *LineBoxWidget;

/* utility functions */
void  LBoxSetFloatValue (Widget w, char *name, float val);

/* widget data control */
void  LBoxResetPoints (Widget w);
void  LBoxGetPointsNorm (Widget w, int *pNum, LBoxPoint_t **ppPoints);
void  LBoxSetPointsNorm (Widget w, int nPoints, LBoxPoint_t *pPoints);
void  LBoxSetHistogram (Widget w, int nBins, LBoxPoint_t *pBins);
void  LBoxSetMotionCB (Widget w, void (*cb)(Widget));
void  LBoxSetStretchChangeCB (Widget w, void (*cb)(Widget));
void  LBoxGetPointerDN (Widget w, float *x, float *y);
void  LBoxGetMappedPoints (Widget w, int *pNum, LBoxPoint_t **ppPoints);
void  LBoxSetMappedPoints (Widget w, int nPoints, float *pPoints);

#endif

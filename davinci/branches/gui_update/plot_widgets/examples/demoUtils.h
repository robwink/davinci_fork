/**********************************************************************
*                                                                     *
* demoUtils.h - Common parts for example programs                     *
*                                                                     *
* August 1, 1994 by George Dimas                                      *
*                                                                     *
**********************************************************************/

void createButton(Widget parent, char *name, char *label,
	XtCallbackProc callbackProc, XtPointer cbArg);
Widget createToggleButton(Widget parent, char *name, char *label,
	XtCallbackProc callbackProc, XtPointer cbArg);
int fermiLogo(float x, float y);
void setRange(Widget w, double *minXLim, double *minYLim, double *minZLim,
	double *maxXLim, double *maxYLim, double *maxZLim);
void displayHelpBitmap(Widget parent, char *title, unsigned char *bits,
	int width, int height);

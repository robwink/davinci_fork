typedef void (*XYMarkLineCallbackProc)(XYCurve *, int, void *);
typedef void (*XYHistCallbackProc)(XYHistogram *, int, void *);
typedef XYMarkLineCallbackProc XYCallbackProc; /* obsolete */

int XYEditCurveStyles(Widget parent, XYCurve *curves, int nCurves);
int XYEditHistogramStyles(Widget parent, XYHistogram *histograms,
    	int nHistograms);
Widget XYCreateCurveStylesDialog(Widget parent, XYCurve *curves, int nCurves,
	XYMarkLineCallbackProc okCallback, void *okArg,
	XYMarkLineCallbackProc applyCallback, void *applyArg,
	XYMarkLineCallbackProc dismissCallback, void *dismissArg);
Widget XYCreateHistogramStylesDialog(Widget parent, XYHistogram *histograms,
    	int nHistograms, XYHistCallbackProc okCallback, void *okArg,
    	XYHistCallbackProc applyCallback, void *applyArg,
    	XYHistCallbackProc dismissCallback, void *dismissArg);
void GetXYColors(char ***colorName , Pixel **colorPixel);

/* Obsolete routines, for compatability with v1.0 */
int XYEditStyles(Widget parent, XYCurve *curves, int nCurves);
Widget XYCreateStylesDialog(Widget parent, XYCurve *curves, int nCurves,
	XYCallbackProc okCallback, void *okArg, XYCallbackProc applyCallback,
	void *applyArg, XYCallbackProc dismissCallback, void *dismissArg);

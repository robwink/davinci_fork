/* 2DHist.h - include file for Hist2D widget programm interface 
** Preliminary edition 
*/

#ifndef _2DHIST_H
#define _2DHIST_H

#define XmNdoubleBuffer "doubleBuffer"
#define XmCDoubleBuffer "DoubleBuffer"
#define XmNzLogScaling "zLogScaling"
#define XmCZLogScaling "ZLogScaling"
#define XmNbinEdgeLabeling  "binEdgeLabeling"
#define XmCBinEdgeLabeling  "BinEdgeLabeling"
#define XmNshadingOn "shadingOn"
#define XmCShadingOn "ShadingOn"
#define XmNerrorBarsOn "errorBarsOn"
#define XmCErrorBarsOn "ErrorBarsOn"
#define XmNtopPlaneColor "topPlaneColor"
#define XmCTopPlaneColor "TopPlaneColor"
#define XmNleftPlaneColor "leftPlaneColor"
#define XmCLeftPlaneColor "LeftPlaneColor"
#define XmNrightPlaneColor "rightPlaneColor"
#define XmCRightPlaneColor "RightPlaneColor"
#define XmNleftPlanePixmap "leftPlanePixmap"
#define XmCLeftPlanePixmap "LeftPlanePixmap"
#define XmNrightPlanePixmap "rightPlanePixmap"
#define XmCRightPlanePixmap "RightPlanePixmap"
#define XmNclippingColor "clippingColor"
#define XmCClippingColor "ClippingColor"
#define XmNbackPlanesOn "backPlanesOn"
#define XmCBackPlanesOn "BackPlanesOn"
#ifndef XmNfontList
#define XmNfontList "fontList"
#define XmCFontList "FontList"
#endif
#ifndef XmNresizeCallback
#define XmNresizeCallback "resizeCallback"
#define XmCResizeCallback "ResizeCallback"
#endif
#define XmNbtn2Callback "btn2Callback"
#define XmCBtn2Callback "Btn2Callback"
#define XmNbtn3Callback "btn3Callback"
#define XmCBtn3Callback "Btn3Callback"
#define XmNredisplayCallback "redisplayCallback"
#define XmCRedisplayCallback "RedisplayCallback"

typedef enum _h2DScaleType {H2D_LINEAR, H2D_LOG} h2DScaleType;

typedef struct _h2DHistSetup {
    int nXBins;			/* number of bins along x axis of histogram  */
    int nYBins;			/* number of bins along y axis of histogram  */
    double xMin;		/* low edge of first x axis bin		     */
    double xMax;		/* high edge of last x axis bin		     */
    double yMin;		/* low edge of first y axis bin		     */
    double yMax;		/* high edge of last x axis bin		     */
    h2DScaleType xScaleType;	/* how x data is binned: linear, log, etc..  */
    float xScaleBase;		/* base for x axis log scaling, i.e. 10, e   */
    h2DScaleType yScaleType;	/* how y data is binned: linear, log, etc..  */
    float yScaleBase;		/* base for y axis log scaling, i.e. 10, e   */
    char *xLabel;		/* label for histogram x axis		     */
    char *yLabel;		/* label for histogram y axis		     */
    char *zLabel;		/* label for histogram z (vertical) axis     */
    float *bins;		/* the histogram data (a 2 dim. array)	     */
} h2DHistSetup;

/* adaptive histogram node (tree ) */
typedef struct _aHistNode {
    long int nextNodeOffset;
    union {
	float zData;
	float xySplit;
    } data;
} aHistNode;

typedef struct _aHistStruct {
    float xMin, xMax;
    float yMin, yMax;
    aHistNode *aNode;
} aHistStruct;  

extern WidgetClass hist2DWidgetClass;

typedef struct _hist2DClassRec *Hist2DWidgetClass;
typedef struct _hist2DRec *Hist2DWidget;

/* 2D histogram widget program interface */
typedef enum _scalingMode {HIST2D_SCALING, HIST2D_NO_SCALING,
	HIST2D_RESCALE_AT_MAX} scalingMode;
void hist2DSetHistogram (Widget wg, h2DHistSetup *hist);
void hist2DSetAdaptiveHistogramData (Widget wg, aHistStruct *aHist, 
	scalingMode sMode);
void hist2DUpdateHistogramData (Widget wg, float *data, 
  float *topErrors, float *bottomErrors, scalingMode sMode);
void hist2DSetRebinnedData (Widget wg, float *data, float *topErrors,
			      float *bottomErrors, int nXBins, int nYBins,
			      float xMin, float xMax, float yMin, float yMax,
			      scalingMode sMode);
void hist2DClearHistogramData (Widget wg);
void hist2DSetVisiblePart (Widget wg, 
			    double xMin, double xMax,
			    double yMin, double yMax,
			    double zMin, double zMax
			    );
 void hist2DGetVisiblePart (Widget wg, 
			    double *xMin, double *xMax,
			    double *yMin, double *yMax,
			    double *zMin, double *zMax
			    );
void hist2DZoom(Widget wg, double factor);
void hist2DZoomIn(Widget wg);
void hist2DZoomOut(Widget wg);
void hist2DGetViewAngles (Widget wg, double *fi, double *psi);
void hist2DSetViewAngles (Widget wg, double fi, double psi);
void hist2DResetView (Widget wg);

void hist2DmakePsImage(Widget wid, char *fileName);
void hist2DwritePs(Widget wid, FILE *psfile);
#endif

/* 3DScat.h - include file for 3DScat widget programm interface 
** Preliminary edition 
*/

#ifndef _3DSCAT_H
#define _3DSCAT_H

typedef enum {SCAT3D_NO_RESCALE, SCAT3D_RESCALE, 
  SCAT3D_RESCALE_AT_MAX, SCAT3D_GROW_ONLY} Scat3DRescaleModes ;


#define XmNdoubleBuffer "doubleBuffer"
#define XmCDoubleBuffer "DoubleBuffer"
#define XmNdarkerPoints "darkerPoints"
#define XmCDarkerPoints "DarkerPoints"
#define XmNuseBitmap "useBitmap"
#define XmCUseBitmap "UseBitmap"
#define XmNbitmapStrategyAuto "bitmapStrategyAuto"
#define XmCBitmapStrategyAuto "BitmapStrategyAuto"
#define XmNxLogScaling "xLogScaling"
#define XmCXLogScaling "XLogScaling"
#define XmNyLogScaling "yLogScaling"
#define XmCYLogScaling "YLogScaling"
#define XmNzLogScaling "zLogScaling"
#define XmCZLogScaling "ZLogScaling"
#define XmNdataColor "dataColor"
#define XmCDataColor "DataColor"
#define XmNaxesColor "axesColor"
#define XmCAxesColor "AxesColor"
#define XmNlabeledEdgeColor "labeledEdgeColor"
#define XmCLabeledEdgeColor "LabeledEdgeColor"
#define XmNunLabeledEdgeColor "unLabeledEdgeColor"
#define XmCUnLabeledEdgeColor "UnLabeledEdgeColor"
#define XmNfrontEdgeColor "frontEdgeColor"
#define XmCFrontEdgeColor "FrontEdgeColor"
#define XmNbackEdgeColor "backEdgeColor"
#define XmCBackEdgeColor "BackEdgeColor"
#define XmNbackEdgeDashes "backEdgeDashes"
#define XmCBackEdgeDashes "BackEdgeDashes"
#define XmNbackEdgeDashOffset "backEdgeDashOffset"
#define XmCBackEdgeDashOffset "BackEdgeDashOffset"
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

extern WidgetClass scat3DWidgetClass;

typedef struct _scat3DClassRec *Scat3DWidgetClass;
typedef struct _scat3DRec *Scat3DWidget;

typedef struct _scatPoint {
    float x, y, z;
} Scat3DPoint;

void Scat3DSetContents(Widget wg, Scat3DPoint *points, int nPoints,  
  Scat3DRescaleModes rescale);
void Scat3DSetAxesNames(Widget wg, char *xName, char *yName, char *zName);  
void Scat3DSetLimits (Widget wg, 
			    float xMin, float xMax,
			    float yMin, float yMax,
			    float zMin, float zMax
			    );

void Scat3DSetVisiblePart (Widget wg, 
			    float xMin, float xMax,
			    float yMin, float yMax,
			    float zMin, float zMax
			    );
void Scat3DGetVisiblePart (Widget wg, 
			    float *xMin, float *xMax,
			    float *yMin, float *yMax,
			    float *zMin, float *zMax
			    );

void Scat3DResetView(Widget wg);
void Scat3DZoom(Widget wg, float factor);
void Scat3DZoomIn(Widget wg);
void Scat3DZoomOut(Widget wg);
void Scat3DPrintContents(Widget wg, char *psFileName);
void Scat3DWritePS(Widget wg, FILE *psfile);
void Scat3DGetViewEulerAngles (Widget wg, double *alpha,
                                double *beta, double *gamma );
void Scat3DSetViewEulerAngles (Widget wg, double alpha, 
				double beta, double gamma);

#endif

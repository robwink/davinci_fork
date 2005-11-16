/*
 * AMap.h
 *
 * The data structure definitions and forward declarations for the
 * Association Map type of tool.
 *
 * The association map is a pseudo-histogram-like thing that allows
 * for various means of manipulation. This is based on an interface
 * feature from the IVAS demo program.
 *
 * $Header$
 *
 * $Log$
 * Revision 1.1  1999/06/16 03:24:23  gorelick
 * Initial revision
 *
 * Revision 1.1  1999/06/16 01:40:31  gorelick
 * Initial install
 *
 * Revision 0.2  91/09/23  17:58:21  17:58:21  ngorelic (Noel S. Gorelick)
 * *** empty log message ***
 * 
 * Revision 0.1  91/08/17  17:50:23  17:50:23  rray (Randy Ray)
 * *** empty log message ***
 * 
 *
 */

#ifndef _XF_AMAP_H
#define _XF_AMAP_H

typedef struct _AMap* AMap;

/* The values for an AMap's mode */
#define XfAMapNoAction 0
#define XfAMapAdd 1
#define XfAMapDel 2
#define XfAMapMove 3
#define XfAMapSlide 4
#define XfAMapPtPClear 5
#define XfAMapPtPClear2 6

/* Define the structure used for points in the spread */
struct Point
{
  int v;                /* Vertical Value */
  int flag;             /* Whether this is real or interpolated */
};

/* Define the encapusalted (more or less) data structure for an AMap */
struct _AMap
{
  Display* display;             /* Display the AMap is on */
  Window window;                /* Window created for AMap */
  Window parent;                /* window's parent */
  int active;                   /* AMap active/!active */
  int width, height;            /* Window's width and height */
  int x, y;                     /* X & Y positioning */
  int border_width;             /* width (in pixels) of border */
  unsigned long border_color;   /* Color of border */
  char name[256];               /* Name used to identify AMap */
  char* ext;                    /* Externally-visible data */
  int* member;                  /* Pseudo-class identifier */
  struct Point* points;         /* Internal representation of spread */
  int left_edge;                /* Left edge of current window in points */
  int max_left_edge;            /* Maximum value for above */
  unsigned long pen_color;      /* Pen color for lines */
  unsigned long shade_color;    /* Color to use when shading */
  int shade_style;              /* Style for shading */
  Pixmap shade_pattern;         /* Pattern (if used) for shading */
  int shade_left;               /* No. of pixels on left to shade */
  int shade_right;              /* No. of pixels on right to shade */
  int action_mode;              /* Mode to be considered when Action'd */
  Pixmap map;                   /* The Pixmap used to draw the lines in */
  struct VisualInfo* visual;    /* The information for the background */
  struct CallBackList* CallBacks;
  CallBack exposeCallback;
  CallBack updateCallback;
  CallBack readoutCallback;		/* Callback for readout update */
  AMap nextAMap;
};

/* Support routines */
AMap XfCreateAMap();
int XfDestroyAMap();
int XfSetAMap();
int* XfAMapValue();
int XfAddAMapCallback();
int XfDelAMapCallback();
int XfAddAMapVisual();
int XfActivateAMapMode();
int XfDeactivateAMap();
int XfAMapAction();
int XfMoveAMap();
AMap XfGetAMap();
AMap XfEventAMap();
struct Point* XfPhotographAMap();
int XfRestoreAMap();

#define XfActivateAMap(A, M)    XfActivateAMapMode(A, XfAMapNoAction, M)

/* The global list of Assoc. Maps */
extern AMap AMapList;

#endif /* _XF_AMAP_H */

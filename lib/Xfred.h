/*
 * Xfred.h
 *
 * Main header file for the library Xfred.a. Mostly just includes the
 * header files for the various parts, but also defines a union-type
 * thing for generic visual manipulation.
 *
 * $Header$
 *
 * $Log$
 * Revision 1.2  2002/10/09 00:14:10  gorelick
 * Changes for OS/X
 *
 * Revision 1.1.1.1  1999/06/16 03:24:24  gorelick
 * Initial import
 *
 * Revision 1.1  1999/06/16 01:40:52  gorelick
 * Initial install
 *
 * Revision 0.2  91/09/23  17:55:10  17:55:10  ngorelic (Noel S. Gorelick)
 * *** empty log message ***
 * 
 * Revision 0.1  91/07/24  18:04:15  18:04:15  rray (Randy Ray)
 * *** empty log message ***
 * 
 *
 */
#ifndef _XF_XFRED_H
#define _XF_XFRED_H

#define BLACK(d) BlackPixel(d, DefaultScreen(d))
#define WHITE(d) WhitePixel(d, DefaultScreen(d))

#include <stdio.h>
#include <stdlib.h>

#include <limits.h>
#define MINSHORT        SHRT_MIN
#define MININT          INT_MIN
#define MINLONG         LONG_MIN

#define MAXSHORT        SHRT_MAX
#define MAXINT          INT_MAX
#define MAXLONG         LONG_MAX

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "Callback.h"
#include "VisInfo.h"
#include "Button.h"
#include "Slider.h"
#include "Joystick.h"
#include "AMap.h"
#include "List.h"
#include "Composite.h"
#include "xf.h"
#include "3D.h"
#include "hershey.h"
#include "XB.h"

char *xgets();
void toggle_state();
int set_state();

#ifndef MAX
#define MAX(a,b) (a > b ? a : b)
#endif

#ifndef MIN
#define MIN(a,b) (a < b ? a : b)
#endif

#define DiPRINT(i) printf("%s = %d\n",#i, i);
#define DsPRINT(i) printf("%s = %s\n",#i, i);


/**
*** some default globals
**/

extern Display *_xfDisplay;
extern int _xfScreen;
extern int _xfDepth;
extern GC *_xfgc;
extern XFontStruct *_xfFontStruct;
extern int _xf3Dheight;

int initx();

#endif /* _XF_XFRED_H */



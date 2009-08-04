#ifndef _CB_H_
#define _CB_H_

#include "XB_bitmaps.h"

#define CBSIZE 20
#define CB_ON  4

struct tagCB {
	int type;
    Display        *display;
    Window          parent;
    Window          window;
    int             width, height;
    short           hi, lo, fg, bg; /* 3-D colors */
	
    CallBack        function;
   	XButton			nextB;

    int             state;
    char           *text;
    int             align;
    XFontStruct    *font;
    Pixmap          pixmap;
    int             pix_w;
    int             pix_h;
};

#define XfResizeCB(CB, w, h)    XfPosCB(CB, MAXINT, MAXINT, w, h)
#define XfMoveCB(CB, x, y)      XfPosCB(CB, x, y, MAXINT, MAXINT)

/**
*** Function declarations
**/
#if defined(__STDC__) && defined(__LINT__)

	CButton         XfCreateCB(Display *, Window, int, int, int, int,
							   short, short, short, short, char *, int,
							   XFontStruct *, CallBack);
    int             XfPushCB(CButton, XEvent *);
    void            RedrawCB(CButton);
    Pixmap          XfSetCBPixmap(CButton, Pixmap, int, int);
	int 			XfActiveCB(CButton);

#else

	CButton         XfCreateCB();
    int             XfPushCB();
    void            RedrawCB();
    Pixmap          XfSetCBPixmap();
	int 			XfActiveCB();

#endif              /* __STDC__ */

#endif              /* _CB_H_ */

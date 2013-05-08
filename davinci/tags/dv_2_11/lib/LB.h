#ifndef _LB_H_
#define _LB_H_

struct tagLB {
	int             type;
	Display        *display;
	Window          parent;
	Window          window;
	int             width, height;
	short           hi, lo, fg, bg;

	CallBack        function;
	XButton         nextB;

	int             state;
	char           *text;
	int             align;
	XFontStruct    *font;
	Pixmap          pixmap;
	int             pix_w;
	int             pix_h;
};

#define XfResizeLB(LB, w, h)    XfPosLB(LB, MAXINT, MAXINT, w, h)
#define XfMoveLB(LB, x, y)      XfPosLB(LB, x, y, MAXINT, MAXINT)

/**
*** Function declarations
**/
#if defined(__STDC__) && defined(__LINT__)

	LButton         XfCreateLB(Display *, Window, int, int, int, int,
							   short, short, short, short, char *, int,
							   XFontStruct *);
    int             XfPushLB(LButton, XEvent *);
    void            RedrawLB(LButton);
    Pixmap          XfSetLBPixmap(LButton, Pixmap, int, int);

#else

	LButton         XfCreateLB();
    int             XfPushLB();
    void            RedrawLB();
    Pixmap          XfSetLBPixmap();

#endif              /* __STDC__ */

#endif              /* _LB_H_ */

#ifndef _PB_H_
#define _PB_H_

struct tagPB {
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

#define XfResizePB(PB, w, h)    XfPosXB((XButton)PB, MAXINT, MAXINT, w, h)
#define XfMovePB(PB, x, y)      XfPosXB((XButton)PB, x, y, MAXINT, MAXINT)

/**
*** Function declarations
**/
#if defined(__STDC__) && defined(__LINT__)

	PButton         XfCreatePB(Display *, Window, int, int, int, int,
							   short, short, short, short, char *, int,
							   XFontStruct *, CallBack);
    int             XfPushPB(PButton, XEvent *);
    void            RedrawPB(PButton);
    Pixmap          XfSetPBPixmap(PButton, Pixmap, int, int);

#else

	PButton         XfCreatePB();
    int             XfPushPB();
    void            RedrawPB();
    Pixmap          XfSetPBPixmap();

#endif              /* __STDC__ */

#endif              /* _PB_H_ */

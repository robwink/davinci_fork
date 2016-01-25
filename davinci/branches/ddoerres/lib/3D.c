#include "Xfred.h"


void
Draw3DBox(disp, win, gc, x, y, w, h, r, hi, lo, fg, bg, state, options)
Display *disp;
Window win;
GC gc;
int x, y, w, h, r;
short hi, lo, fg, bg;
int state;
int options;
{
    XPoint ul[7],lr[7],*p1,*p2;

    ul[0].x = x;                ul[0].y = y+h;
    ul[1].x = x+r;              ul[1].y = y+h-r;
    ul[2].x = x+w-r;            ul[2].y = y+h-r;
    ul[3].x = x+w-r;            ul[3].y = y+r;
    ul[4].x = x+w;              ul[4].y = y;
    ul[5].x = x+w;              ul[5].y = y+h;
    ul[6].x = x;                ul[6].y = y+h;

    lr[0].x = x;                lr[0].y = y+h;
    lr[1].x = x+r;              lr[1].y = y+h-r;
    lr[2].x = x+r;              lr[2].y = y+r;
    lr[3].x = x+w-r;            lr[3].y = y+r;
    lr[4].x = x+w;              lr[4].y = y;
    lr[5].x = x;                lr[5].y = y;
    lr[6].x = x;                lr[6].y = y+h;

    if (state != -1 && (state & 1) != 0) {
        p1 = ul;
        p2 = lr;
    } else {
        p1 = lr;
        p2 = ul;
    }


    /** 
     ** Background
     **/
    XSetBackground(disp, gc, bg);

	if (options & CLEAR) {
		XSetForeground(disp, gc, bg);
		XFillRectangle(disp, win, gc, x, y, w, h);
	}

	if (r > 1) {
		/**
		 ** lo stippled polygon 
		 **/

		XSetForeground(disp, gc, lo);
		XSetStipple(disp, gc, XfStipple(disp, win, "gray"));
		XSetTSOrigin(disp, gc, x, y+h);
		XSetFillStyle(disp, gc, FillOpaqueStippled);
		XFillPolygon(disp, win, gc, p2, 7, Complex, CoordModeOrigin);

		/**
		 ** hi polygon 
		 **/

		XSetForeground(disp, gc, hi);
		XSetFillStyle(disp, gc, FillSolid);
		XFillPolygon(disp, win, gc, p1, 7, Complex, CoordModeOrigin);

		/**
		 ** edge hilites
		 **/
		if (0 && r > 3) {
			XDrawRectangle(disp, win, gc, x + r, y + r, w-(r*2+1), h-(r*2+1));
			XSetForeground(disp, gc, lo);
			XDrawLine(disp, win, gc, x+w-r, y+r-1, x+w-1, y);
			XDrawLine(disp, win, gc, x, y+h-1, x+r-1, y+h-r);
		}
    } else {
		XSetForeground(disp, gc, bg);
        XDrawRectangle(disp, win, gc, x+1, y+1, w-3, h-3);
    }

    /**
    *** Border
    **/

    XSetForeground(disp, gc, fg);
    XDrawRectangle(disp, win, gc, x, y, w-1, h-1); 
    if (state != -1 && (state & 2) != 0) {
        XDrawRectangle(disp, win, gc, x+1, y+1, w-3, h-3);
    }
}

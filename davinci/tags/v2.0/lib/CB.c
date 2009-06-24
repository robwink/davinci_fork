#include "Xfred.h"
#include "CB.h"

/**
 ** Push buttons
 **
 ** XfCreateCB		- Create a CheckButton
 ** XfPushCB		- Handle user events
 ** RedrawCB		- Redraw CheckButton based on its state
 ** XfSetCBPixmap 	- Set a pixmap in a CheckButton
 ** XfActiveCB		- returns 1 if CheckButton is active
 **
 ** These XB routines that have been #defined for use as CB routines:
 **
 ** XfActivateCB	- Activate and map a CheckButton
 ** XfDeactivateCB	- Deactivate and unmap a CheckButton
 ** XfDestroyCB		- Deactivate, unmap and destroy a CheckButton
 **/

Pixmap cb_check;

CButton
XfCreateCB(d,win,x,y,w,h,hi,lo,fg,bg,text,align,font,func)
	Display        *d;
	Window          win;
	int             x, y, w, h;
	short           hi, lo, fg, bg;
	char           *text;
	int             align;
	XFontStruct    *font;
	CallBack        func;
{
	CButton new;
	new = (CButton) XfCreateXB(d,win,x,y,w,h,hi,lo,fg,bg,func,XF_CB);

	if (text != NULL) {
		new->text = strdup(text);
	} else {
		new->text = NULL;
	}
	new->align = align;
	new->font = font;
	if (font == NULL && text != NULL) font = _xfFontStruct;
	return(new);
}


/**
 ** XfPushCB	- Handle user events
 **/

int
XfPushCB(CB, E)
CButton CB;
XEvent *E;
{
    Display *display;
    XEvent Ev;
	int width,height;

    if (CB == NULL) return(False);

    display = CB->display;

    switch (E->type) {
        case Expose:
            while (XCheckTypedWindowEvent(display, CB->window, Expose, &Ev))
                ;
            RedrawCB(CB);
            break;

        case ButtonPress:
            if (CB->state != -1) {
                CB->state |= 1;
                RedrawCB(CB);
            }
            break;

        case ButtonRelease:
            if (CB->state != -1) {
				/**
				 ** Call user function and toggle check mark
				 **/
				if (CB->state & 1) {
					CB->state ^= CB_ON;
					CB->state &= (~1);
					if (CB->function != NULL) {
						(*(CB->function))(CB,NULL);
					}
                }
				RedrawCB(CB);
            }
            break;

        case EnterNotify:
            if (CB->state != -1) {
                if ((CB->state & 2) && (E->xcrossing.state & Button1Mask)) {
                    CB->state |= 1;
                } else {
                    CB->state |= 2;
                }
				RedrawCB(CB);
            }
            break;

        case LeaveNotify:
            if (CB->state != -1) {
                if ((CB->state & 1) && (E->xcrossing.state & Button1Mask)) {
                    CB->state &= (~1);
                } else {
                    CB->state &= (~2);
                }
                RedrawCB(CB);
            }
            break;

		case ConfigureNotify:
			width = CB->width;
			height = CB->height;
			CB->width = E->xconfigure.width;
			CB->height = E->xconfigure.height;
			if (width != CB->width || height != CB->height) {
				RedrawCB(CB);
			}
			break;
    }
}

/**
 ** RedrawCB	- Redraw CheckButton based on its state
 **/

void
RedrawCB(CB)
CButton CB;
{
    Display *disp = CB->display;
    Window win = CB->window;
    GC gc = DefaultGC(disp, DefaultScreen(disp));
    int width=CB->width;
    int height=CB->height;
    XPoint ul[7],lr[7],*p1,*p2;
    int r = 3, w=0;
	int xlo,xhi,ypos;
	int dir, ascent, descent;
	XCharStruct extents;

	if (CB->text) {
		XTextExtents(CB->font, CB->text, strlen(CB->text), 
					&dir, &ascent, &descent, &extents);
		w = extents.width;
	}
	if (CB->pixmap) {
		w = CB->pix_w;
	}

	if (CB->align < 0) { /* button on right side */
		xlo = width - CBSIZE;
		xhi = xlo - w - 4;
	} else {
		xlo = 0;
		xhi = xlo + CBSIZE + 4;
	}
	ypos = (CB->height-CBSIZE)/2;

	Draw3DBox(disp, win, gc, xlo, ypos, CBSIZE, CBSIZE, Xf3DHeight(),
				CB->hi, CB->lo, CB->fg, CB->bg, CB->state, 0);

	/**
	 ** Paint in checkmark.  Use bg to erase old
	 **/
	if (!cb_check) {
		cb_check = XCreateBitmapFromData(disp,win,cb_check_bits, CBSIZE,CBSIZE);
	}

	XSetStipple(disp, gc, cb_check);
	XSetTSOrigin(disp, gc, xlo,ypos);
	XSetFillStyle(disp, gc, FillStippled);

	if (CB->state != -1 && CB->state & CB_ON) {
		XSetForeground(disp, gc, CB->fg);
		XFillRectangle(disp, win, gc, xlo, ypos, CBSIZE, CBSIZE);
		XSetFillStyle(disp, gc, FillSolid);
	} else {
		XSetForeground(disp, gc, CB->bg);
		XFillRectangle(disp, win, gc, xlo, ypos, CBSIZE, CBSIZE);
		XSetFillStyle(disp, gc, FillSolid);
	}


    XSetForeground(disp, gc, CB->fg);
    XSetBackground(disp, gc, CB->bg);

    if (CB->text != NULL) {
		int y;
        y = (CB->height+ascent)/2;
        XSetFont(disp, gc, CB->font->fid);
        XDrawString(disp, win, gc, xhi, y, CB->text, strlen(CB->text));
    }
    if (CB->pixmap != 0) {
        int y;
        y = (CB->height-CB->pix_h)/2;

        XCopyPlane(disp, CB->pixmap, win, gc, 
                    0, 0, CB->pix_w, CB->pix_h, xhi, y, 1);
    }
	/**
	 ** If inactive, its grayed out
	 **/
	if (CB->state == -1) {
		XSetFillStyle(disp, gc, FillStippled);
		XSetStipple(disp, gc, XfStipple(disp, win, "gray"));
		XSetForeground(disp, gc, CB->bg);
		XFillRectangle(disp, win, gc, 0, 0, CB->width, CB->height);
		XSetFillStyle(disp, gc, FillSolid);
	}
	XFlush(disp);
}

/**
 ** XfSetCBPixmap - Set a pixmap in a CheckButton
 **/

Pixmap
XfSetCBPixmap(CB, pixmap, w, h)
CButton CB;
Pixmap pixmap;
int w,h;
{
    Pixmap ret = CB->pixmap;

    CB->pixmap = pixmap;
    CB->pix_w = w;
    CB->pix_h = h;

    return(ret);
}


int
XfActiveCB(CB)
CButton CB;
{
	return((CB->state & CB_ON)!=0);
}

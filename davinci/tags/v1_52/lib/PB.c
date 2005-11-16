#include "Xfred.h"
#include "PB.h"

/**
 ** Push buttons
 **
 ** XfCreatePB		- Create a PushButton
 ** XfPushPB		- Handle user events
 ** RedrawPB		- Redraw PushButton based on its state
 ** XfSetPBPixmap 	- Set a pixmap in a PushButton
 **
 ** These XB routines that have been #defined for use as PB routines:
 **
 ** XfActivatePB	- Activate and map a PushButton
 ** XfDeactivatePB	- Deactivate and unmap a PushButton
 ** XfDestroyPB		- Deactivate, unmap and destroy a PushButton
 **/


PButton
XfCreatePB(d,win,x,y,w,h,hi,lo,fg,bg,text,align,font,func)
	Display        *d;
	Window          win;
	int             x, y, w, h;
	short           hi, lo, fg, bg;
	char           *text;
	int             align;
	XFontStruct    *font;
	CallBack        func;
{
	PButton new;

	new = (PButton) XfCreateXB(d,win,x,y,w,h,hi,lo,fg,bg,func,XF_PB);

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
 ** XfPushPB	- Handle user events
 **/

int
XfPushPB(PB, E)
PButton PB;
XEvent *E;
{
    Display *display;
    XEvent Ev;
	int width,height;

    if (PB == NULL) return(False);

    display = PB->display;

    switch (E->type) {
        case Expose:
			while (XCheckTypedWindowEvent(display, PB->window, Expose, &Ev))
				;
            RedrawPB(PB);
            break;

        case ButtonPress:
            if (PB->state != -1) {
                PB->state |= 1;
                RedrawPB(PB);
            }
            break;

        case ButtonRelease:
            if (PB->state != -1) {
                if (PB->function != NULL && (PB->state & 1)) {
                    (*(PB->function))(PB,E);
                }
				if (PB->state > 0) {
					PB->state &= (~1);
					RedrawPB(PB);
				}
            }
            break;

        case EnterNotify:
            if (PB->state != -1) {
                if ((PB->state & 2) && (E->xcrossing.state & Button1Mask)) {
                    PB->state |= 1;
                } else {
                    PB->state |= 2;
                }
                RedrawPB(PB);
            }
            break;

        case LeaveNotify:
            if (PB->state != -1) {
                if ((PB->state & 1) && (E->xcrossing.state & Button1Mask)) {
                    PB->state &= (~1);
                } else {
                    PB->state &= (~2);
                }
                RedrawPB(PB);
            }
            break;

		case ConfigureNotify:
			width = PB->width;
			height = PB->height;
			PB->width = E->xconfigure.width;
			PB->height = E->xconfigure.height;
			if (width != PB->width || height != PB->height) {
				XClearWindow(PB->display, PB->window);
				RedrawPB(PB);
			}
			break;
    }
}

/**
 ** RedrawPB	- Redraw PushButton based on its state
 **
 **             - this function uses Draw3DBox.  If a buttons text or pixmap
 **               ever changes, Draw3DBox won't erase the old text/pixmap 
 **               unless the option (CLEAR) is passed as the last argument.
 **				  The safe thing to do is ClearWindow and Expose on change.
 **/

void
RedrawPB(PB)
PButton PB;
{
    Display *display = PB->display;
    Window window = PB->window;
    GC gc = DefaultGC(display, DefaultScreen(display));
    int width = PB->width;
    int height = PB->height;
    int r = 4;

	Draw3DBox(display, window, gc, 0, 0, width, height, Xf3DHeight(),
			  PB->hi, PB->lo, PB->fg, PB->bg, PB->state, 0);

    XSetForeground(display, gc, PB->fg);
    XSetBackground(display, gc, PB->bg);

    if (PB->text != NULL) {
        int dir, ascent, descent;
        int x,y;
        XCharStruct extents;

        XTextExtents(PB->font, PB->text, strlen(PB->text), 
                    &dir, &ascent, &descent, &extents);

        y = (PB->height+ascent)/2;
        switch(PB->align) {
            case 0:     /* center */
                x = (PB->width - extents.width)/2;
                break;
                
            case 1:     /* left */
                x = r+2;
                break;

            case -1:    /* right */
                x = PB->width-(r+2)-extents.width;
                break;
        }
        XSetFont(display, gc, PB->font->fid);
        /**
        *** If inactive, its grayed out
        **/
        if (PB->state == -1) {
			XSetStipple(display, gc, XfStipple(display, window, "gray"));
            XSetFillStyle(display, gc, FillOpaqueStippled);
			XSetBackground(display, gc, PB->bg);
        }
        XDrawString(display, window, gc, x, y, PB->text, strlen(PB->text));
    }
    if (PB->pixmap != 0) {
        int x,y;

        x = (PB->width-PB->pix_w)/2;
        y = (PB->height-PB->pix_h)/2;

        XCopyPlane(display, PB->pixmap, window, gc, 
                    0, 0, PB->pix_w, PB->pix_h, x, y, 1);
        if (PB->state == -1) {
			XSetStipple(display, gc, XfStipple(display, window, "gray"));
            XSetFillStyle(display, gc, FillStippled);
            XSetForeground(display, gc, PB->bg);
            XFillRectangle(display, window, gc, x, y, PB->pix_w, PB->pix_h);
		}
    }
    XSetFillStyle(display, gc, FillSolid);
	XFlush(display);
}

/**
 ** XfSetPBPixmap - Set a pixmap in a PushButton
 **/

Pixmap
XfSetPBPixmap(PB, pixmap, w, h)
PButton PB;
Pixmap pixmap;
int w,h;
{
    Pixmap ret = PB->pixmap;

    PB->pixmap = pixmap;
    PB->pix_w = w;
    PB->pix_h = h;

    return(ret);
}


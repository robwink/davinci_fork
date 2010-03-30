#include "Xfred.h"
#include "LB.h"

/**
 ** Label buttons
 **
 ** XfCreateLB		- Create a LabelButton
 ** XfLabelLB		- Handle user events
 ** RedrawLB		- Redraw LabelButton based on its state
 ** XfSetLBPixmap 	- Set a pixmap in a LabelButton
 **
 ** These XB routines that have been #defined for use as LB routines:
 **
 ** XfActivateLB	- Activate and map a LabelButton
 ** XfDeactivateLB	- Deactivate and unmap a LabelButton
 ** XfDestroyLB		- Deactivate, unmap and destroy a LabelButton
 **/


LButton
XfCreateLB(d,win,x,y,w,h,hi,lo,fg,bg,text,align,font)
	Display        *d;
	Window          win;
	int             x, y, w, h;
	short           hi, lo, fg, bg;
	char           *text;
	int             align;
	XFontStruct    *font;
{
	LButton new;
	int dir, ascent, descent;
    XCharStruct extents;

	if (w == 0 && h == 0) {
		if (text != NULL) {
			XTextExtents(font, text, strlen(text), 
						&dir, &ascent, &descent, &extents);
			w = extents.width;
			h = ascent + descent;
		}
	}
	
	new = (LButton) XfCreateXB(d,win,x,y,w,h,hi,lo,fg,bg,NULL,XF_LB);

	/**
	 ** reset input to just expose and resize
	 **/
	XSelectInput(d, new->window, ExposureMask | StructureNotifyMask);

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
 ** XfPushLB	- Handle user events
 **/

int
XfPushLB(LB, E)
LButton LB;
XEvent *E;
{
    Display *display;
    XEvent Ev;
	int width,height;

    if (LB == NULL) return(False);

    display = LB->display;

    switch (E->type) {
        case Expose:
			while (XCheckTypedWindowEvent(display, LB->window, Expose, &Ev))
				;
            RedrawLB(LB);
            break;

		case ConfigureNotify:
			width = LB->width;
			height = LB->height;
			LB->width = E->xconfigure.width;
			LB->height = E->xconfigure.height;
			if (width != LB->width || height != LB->height) {
				RedrawLB(LB);
			}
			break;
    }
}

/**
 ** RedrawLB	- Redraw LabelButton
 **/

void
RedrawLB(LB)
LButton LB;
{
    Display *display = LB->display;
    Window window = LB->window;
    GC gc = DefaultGC(display, DefaultScreen(display));
    int width = LB->width;
    int height = LB->height;
    int r = 4;

    XSetForeground(display, gc, LB->fg);
    XSetBackground(display, gc, LB->bg);

	XClearWindow(display, LB->window);

    if (LB->text != NULL) {
        int dir, ascent, descent;
        int x,y;
        XCharStruct extents;

        XTextExtents(LB->font, LB->text, strlen(LB->text), 
                    &dir, &ascent, &descent, &extents);

        y = ascent;
        XSetFont(display, gc, LB->font->fid);
        XDrawString(display, window, gc, 0, y, LB->text, strlen(LB->text));
    }
    if (LB->pixmap != 0) {
        int x,y;

        x = (LB->width-LB->pix_w)/2;
        y = (LB->height-LB->pix_h)/2;

        XCopyPlane(display, LB->pixmap, window, gc, 
                    0, 0, LB->pix_w, LB->pix_h, x, y, 1);
    }
	XFlush(display);
}

/**
 ** XfSetLBPixmap - Set a pixmap in a LabelButton
 **/

Pixmap
XfSetLBPixmap(LB, pixmap, w, h)
LButton LB;
Pixmap pixmap;
int w,h;
{
    Pixmap ret = LB->pixmap;

    LB->pixmap = pixmap;
    LB->pix_w = w;
    LB->pix_h = h;

    return(ret);
}


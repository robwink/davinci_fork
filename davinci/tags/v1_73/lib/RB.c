#include "Xfred.h"
#include "RB.h"

/**
 ** Radio buttons
 **
 **	XfCreateRB		 - Create a RadioButton and append it to the supplied list
 ** XfActivateRBList - Activates RadioButton in a list, and maps them all.
 ** XfActiveRB		 - returns index of active RadioButton.  -1 if none active
 ** XfCountRB		 - returns number of items in list
 ** XfWhichRB		 - returns index of RadioButton in list
 ** XfDestroyRB	     - Destroy a RadioButton and remove it from its xor list
 ** XfPushRB 		 - Handle user events
 ** RedrawRB 		 - Redraw the contents of a RadioButton, based on its state
 **
 ** These XB routines that have been #defined for use as RB routines:
 ** 
 ** XfActivateRB	 - Activate and map a RadioButton
 ** XfDeactivateRB	 - Deactivate and unmap a RadioButton
 **
 ** Each member of a list knows its list head. This means that a list can be
 ** represented by any member in the list.
 **/

Pixmap rb_top, rb_bot, rb_frame, rb_hit, rb_dot;

RButton
XfCreateRB(d, win, x, y, w, h, hi, lo, fg, bg, text, align, font, func, RBlist)
Display *d;
Window win;
int x,y,w,h;
short hi,lo,fg,bg;
char *text;
int align;
XFontStruct *font;
CallBack func;
RButton RBlist;
{
	RButton new;
	RButton search;

	new = (RButton) XfCreateXB(d,win,x,y,w,h,hi,lo,fg,bg,func,XF_RB);

	if (text != NULL) {
		new->text = strdup(text);
	} else {
		new->text = NULL;
	}
	new->align = align;
	new->font = font;
	if (font == NULL && text != NULL) font = _xfFontStruct;

	/**
	 ** Put this button in the given RBlist
	 **/ 
	 if (RBlist == NULL) {
		((RButton)new)->headRB = (RButton)new;
	} else {
		search = RBlist;
		while(search->nextRB != NULL) {
			search = search->nextRB;
		}
		search->nextRB = (RButton)new;
		((RButton)new)->headRB = RBlist;
	}
	return((RButton)new);
}


/**
*** XfActivateRBList - Set one RB in a list as active and map them all.
**/


void
XfActivateRBList(RB, count)
RButton RB;
int count;
{
	RButton search, find=NULL;

	/**
	*** Activate all buttons to 0 state or less
	**/
	search = RB;
	while(search != NULL) {
		if (count-- == 0) {
			find = search;
		} else {
			XfActivateRB(search, MIN(search->state, 0));
		}
		search = search->nextRB;
	}
	/**
	*** Set the selected button to active state.
	**/
	if (find) {
		XfActivateRB(find, 4);
	}
}

/**
 ** XfPushRB - Handle user events
 **/


int
XfPushRB(RB, E)
RButton RB;
XEvent *E;
{
    Display *display;
    XEvent Ev;
	int width,height;
	RButton search;
	int state;

    if (RB == NULL) return(False);

    display = RB->display;

    switch (E->type) {
        case Expose:
            while (XCheckTypedWindowEvent(display, RB->window, Expose, &Ev))
                ;
            RedrawRB(RB);
            break;

        case ButtonPress:
            if (RB->state != -1) {
                RB->state |= 1;
                RedrawRB(RB);
            }
            break;

        case ButtonRelease:
            if (RB->state != -1) {
				/**
				 ** If we are still on the button, 
				 ** turn off all other RBs in this list
				 **/
                if (RB->state & 1) {
					search = RB->headRB;
					while (search != NULL) {
						if (search != RB && search->state >= 0) 
							XfActivateRB(search, 0);
						search = search->nextRB;
					}
					/**
					 ** This one gets set to active, and call the user function
					 **/
					RB->state |= 4;	/* user selection */
					RB->state |= 1;	/* to make sure it stays down */
					RedrawRB(RB);
					if (RB->function) {
						(*(RB->function))(RB,NULL);
					}
				} else if (RB->state & 4) {
					RB->state |= 1;	/* to make sure it stays down */
					RedrawRB(RB);
				}
            }
            break;

        case EnterNotify:
            if (RB->state != -1) {
                if ((RB->state & 2) && (E->xcrossing.state & Button1Mask)) {
                    RB->state |= 1;
                } else {
                  	RB->state |= 2;
                }
				RedrawRB(RB);
            }
            break;

        case LeaveNotify:
            if (RB->state != -1) {
                if ((RB->state & 1) && (E->xcrossing.state & Button1Mask)) {
                    RB->state &= (~1);
                } else {
                    RB->state &= (~2);
                }
				RedrawRB(RB);
            }
            break;

		case ConfigureNotify:
			width = RB->width;
			height = RB->height;
			RB->width = E->xconfigure.width;
			RB->height = E->xconfigure.height;
			if (width != RB->width || height != RB->height) {
				RedrawRB(RB);
			}
			break;
    }
}

/**
 ** RedrawRB - Redraw the contents of a RB, based on its state
 **/

void
RedrawRB(RB)
RButton RB;
{
    Display *disp = RB->display;
    Window win= RB->window;
    GC gc = DefaultGC(disp, DefaultScreen(disp));
	int depth = DefaultDepth(disp, DefaultScreen(disp));
    int width = RB->width;
    int height = RB->height;
	int xlo, xhi, ypos;
	int y,w=0;
	short tcolor, bcolor;
	int dir, ascent, descent;
	XCharStruct extents;

	if (!rb_top) {
		/**
		 ** Create pixmaps
		 **/

		rb_frame = XCreateBitmapFromData(disp,win,rb_frame_bits, RBSIZE,RBSIZE);
		rb_top =   XCreateBitmapFromData(disp,win,rb_top_bits,   RBSIZE,RBSIZE);
		rb_bot =   XCreateBitmapFromData(disp,win,rb_bot_bits,   RBSIZE,RBSIZE);
		rb_dot =   XCreateBitmapFromData(disp,win,rb_dot_bits,   RBSIZE,RBSIZE);
		rb_hit =   XCreateBitmapFromData(disp,win,rb_frame1_bits,RBSIZE,RBSIZE);
	}

	if (RB->text) {
		XTextExtents(RB->font, RB->text, strlen(RB->text), 
					&dir, &ascent, &descent, &extents);
		w = extents.width;
	} else if (RB->pixmap) {
		w = RB->pix_w;
	}
	if (RB->align < 0) {
		xlo = width - RBSIZE;
		xhi = xlo - w - 4;
	} else {
		xlo = 0;
		xhi = xlo + RBSIZE + 4;
	}


	ypos = (RB->height-RBSIZE)/2;

	if (RB->state == -1 || !(RB->state & 1)) { 			/* Button UP */
		tcolor = RB->hi;
		bcolor = RB->lo;
	} else {
		tcolor = RB->lo;
		bcolor = RB->hi;
	}

	XSetForeground(disp, gc, RB->bg);
	XFillRectangle(disp, win, gc, xlo, ypos, RBSIZE, RBSIZE);

	XSetFillStyle(disp, gc, FillStippled);
	XSetTSOrigin(disp, gc, xlo, ypos);

	XSetStipple(disp, gc, rb_frame);
	XSetForeground(disp, gc, RB->fg);
	XFillRectangle(disp, win, gc, xlo, ypos, RBSIZE, RBSIZE);

	if (Xf3DHeight() > 1) {
		XSetStipple(disp, gc, rb_top);
		XSetForeground(disp, gc, tcolor);
		XFillRectangle(disp, win, gc, xlo, ypos, RBSIZE, RBSIZE);

		XSetStipple(disp, gc, rb_bot);
		XSetForeground(disp, gc, bcolor);
		XFillRectangle(disp, win, gc, xlo, ypos, RBSIZE, RBSIZE);
	}

	XSetForeground(disp, gc, RB->fg);

	if (RB->state != -1 && (RB->state & 2)) {			/* Mouse hit */
		XSetStipple(disp, gc, rb_hit);
		XFillRectangle(disp, win, gc, xlo, ypos, RBSIZE, RBSIZE);
	}

	if (RB->state != -1 && (RB->state & 4)) {			/* Selected */
		XSetStipple(disp, gc, rb_dot);
		XFillRectangle(disp, win, gc, xlo, ypos, RBSIZE, RBSIZE);
	}

	XSetFillStyle(disp, gc, FillSolid);

/**
 ** Put in the text and stuff
 **/

    if (RB->text != NULL) {
        y = (RB->height+ascent)/2;
        XSetFont(disp, gc, RB->font->fid);
        XDrawString(disp, win, gc, xhi, y, RB->text, strlen(RB->text));
    }
	if (RB->pixmap) {
		y = (RB->height + RB->pix_h)/2;
        XCopyPlane(disp, RB->pixmap, win, gc, 
                    0, 0, RB->pix_w, RB->pix_h, xhi, y, 1);
	}
	if (RB->state == -1) {
		/**
		 ** If inactive, its grayed out
		 **/
		XSetFillStyle(disp, gc, FillStippled);
		XSetStipple(disp, gc, XfStipple(disp, win, "gray"));
		XSetForeground(disp, gc, RB->bg);
		XFillRectangle(disp, win, gc, 0, 0, RB->width, RB->height);
		XSetFillStyle(disp, gc, FillSolid);
	}
	XFlush(disp);
}

/**
 ** XfDestroyRB	- Delete, plus we have to remove element from its xor list
 **/

void
XfDestroyRB(RB)
RButton RB;
{
	RButton search, find;
	
	search = RB->headRB;

	if (search != NULL) {
		/**
		 ** Head element, reset everyone elses head to the next element.
		 **/
		if (search == RB) {
			find = search->nextRB;
			while(search->nextRB != NULL) {
				search->headRB = find;
				search = search->nextRB;
			}
		} else {
			/**
			 ** Just pull element out of list
			 **/
			while(search->nextRB != NULL) {
				if (search->nextRB == RB) {
					search->nextRB = search->nextRB->nextRB;
					break;
				}
				search = search->nextRB;
			}
		}
	}
	XfDestroyXB((XButton)RB);
}

/** 
 ** XfWhichRB		- returns index of RB in list
 **/

int
XfWhichRB(RB)
RButton RB;
{
	RButton search;
	int count=0;

	search = RB->headRB; 
	while(search != NULL && search != RB) {
		count++;
		search = search->nextRB;
	}
	return(count);
}

/** 
 ** XfCountRB		- returns number of items in list
 **/

int
XfCountRB(RB)
RButton RB;
{
	RButton search;
	int count=0;

	search = RB->headRB;
	while(search != NULL) {
		count++;
		search = search->nextRB;
	}
	return(count);
}


/** 
 ** XfActiveRB		- returns index of active RB.  -1 if none active.
 **/

int
XfClickedRB(RB)
RButton RB;
{
	RButton search;
	int count=0;

	search = RB->headRB;
	while(search != NULL) {
		if (search->state & 4)
			return(count);
		count++;
		search = search->nextRB;
	}
	return(-1);
}

int
XfActiveRB(RB)
RButton RB;
{
	RButton search;
	int count=0;

	search = RB->headRB;
	while(search != NULL) {
		if (search->state & 4)
			return(count);
		count++;
		search = search->nextRB;
	}
	return(-1);
}

RButton
XfGetRB(RB, count)
RButton RB;
int count;
{
	RButton search;
	search = RB->headRB;

	while(search != NULL) {
		if (count-- == 0) {
			return(search);
		}
		search = search->nextRB;
	}
	return(NULL);
}

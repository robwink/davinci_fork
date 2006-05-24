#include "Xfred.h"

/**
 ** XButtons - Generic button routines 
 **
 ** XfCreateXB		- Create an XButton
 ** XfActivateXB	- Map a XButton to the screen, and set its state.
 ** XfEventXB		- Returns which XB, if any, the passed event occured in.
 ** XfPushXB		- Handle user events
 ** XfPosXB			- Move and resize an XButton
 ** XfDeactivateXB	- Unmap an XButton
 ** XfDestroyXB		- Destroy an XButton
 ** RedrawXB		- Call the appropirate Redraw routine based on an XB's type
 **/


XButton         XBList;			/* Global list of Buttons */
XButton			*XBModal=NULL;	/* List of modal buttons */
int             XBnModal=0;		/* number of elements in Modal list*/
int             XBsModal=0;		/* size of modal list */


/**
 ** XfCreateXB	- Create an XButton
 **/

XButton
XfCreateXB(disp, win, x, y, w, h, hi, lo, fg, bg, func, type)
	Display        *disp;
	Window          win;
	int             x, y, w, h;
	short           hi, lo, fg, bg;
	CallBack        func;
	int             type;
{
	AButton         new, search;
	new = (AButton) calloc(1,sizeof((*(XButton) NULL)));

	if (disp == NULL) disp = _xfDisplay;
	if (win == 0) win = RootWindow(disp, DefaultScreen(disp));
	if (w <= 0) w = 50;
	if (h <= 0) h = 50;

	new->type = type;
	new->display = disp;
	new->parent = win;
	new->width = w;
	new->height = h;
	new->function = func;

	if (hi == -1) hi = XfColor(disp, "White");
	if (lo == -1) lo = XfColor(disp, "Black");
	if (fg == -1) fg = XfColor(disp, "Black");
	if (bg == -1) bg = XfColor(disp, "gray80");

	/**
	 *** If we can't allocate gray80, default to white
	 **/
	if (bg == 0) bg = XfColor(disp, "White");

	new->hi = hi;
	new->lo = lo;
	new->fg = fg;
	new->bg = bg;


	new->nextB = NULL;

	new->window = XCreateSimpleWindow(disp, win, x, y, w, h, 0, 0, bg);
	XSelectInput(new->display, new->window,
		     ExposureMask | ButtonPressMask | ButtonReleaseMask |
		   EnterWindowMask | LeaveWindowMask | StructureNotifyMask);

	/**
	 ** Add this Button to the front of the ButtonList
	 **/

	new->nextB = XBList;
	XBList = (XButton) new;

	return ((XButton) new);
}
/**
 ** XfActivateXB - Map a XButton to the screen, and set its state.
 **/

int
XfActivateXB(XB, state)
	XButton         XB;
	int             state;
{
	AButton         B = (AButton) XB;
	int             ret = B->state;


	B->state = state;
	XMapRaised(B->display, B->window);

	if (B->state != ret)
		RedrawXB(XB);
	return (ret);
}
/**
 ** Returns which XB, if any, in the modal list, an event occured in.
 **/

XButton
XBModalEvent(E)
	XEvent *E;
{
	int i;

	if (XBnModal == 0 || XBsModal == 0) return(NULL);

	for (i = 0 ; i < XBsModal ; i++) {
		if (XBModal[i] == NULL) continue;
		if (((AButton)XBModal[i])->window == E->xany.window) {
			return(XBModal[i]);
		}
	}
	return(NULL);
}


/**
 ** XfEventXB - returns which XB, if any, the passed event occured in.
 **             Move the found button to the front of the ButtonList for
 **             quicker reference the next time
 **/

XButton
XfEventXB(E)
	XEvent         *E;
{
	AButton         search;
	XButton         find;
	XEvent			Ev;

	if (E->type == GraphicsExpose || E->type == NoExpose)
		return (NULL);

	if (XBList == NULL)
		return;

	/**
	 ** If there are any items in the ModalList, nothing else 
	 ** gets any events (except Expose).
	 **/
	if (XBnModal != 0 && E->type != Expose) {
		return(XBModalEvent(E));
	}

	search = (AButton) XBList;
	if (search->window == E->xany.window) {
		return ((XButton) search);
	} else {
		while (search->nextB != NULL) {
			if (((AButton) (search->nextB))->window == E->xany.window) {
				find = search->nextB;
				search->nextB = ((AButton) (search->nextB))->nextB;
				((AButton) find)->nextB = XBList;
				XBList = find;
				return ((XButton) find);
			}
			search = (AButton) search->nextB;
		}
	} return (NULL);
}


/**
 ** XfPushXB - Handle user events
 **/

int
XfPushXB(XB, E)
	XButton         XB;
	XEvent         *E;
{
	if (XB == NULL) return;
	switch (XB->type) {
	case XF_PB:
		return (XfPushPB((PButton) XB, E));
	 case XF_RB: 
		return (XfPushRB((RButton) XB, E));
	 case XF_CB: 
		return (XfPushCB((CButton) XB, E));
	 case XF_MB: 
		return (XfPushMB((MButton) XB, E));
	 case XF_LB: 
		return (XfPushLB((LButton) XB, E));
	}
}

/**
 ** XfPosXB - Move and resize an XButton
 **/

void
XfPosXB(XB, x, y, w, h)
	XButton         XB;
{
	int             move = 0, resize = 0;
	AButton         B = (AButton) XB;

	if (x != MAXINT && y != MAXINT) {
		move = 1;
	}
	if (w != MAXINT && h != MAXINT) {
		resize = 1;
	}
	if (move && resize) {
		XMoveResizeWindow(B->display, B->window, x, y, w, h);
	} else if (move) {
		XMoveWindow(B->display, B->window, x, y);
	} else if (resize) {
		XResizeWindow(B->display, B->window, w, h);
	}
}


/**
 ** XfDeactivateXB - Unmap an XButton
 **/

void
XfDeactivateXB(XB)
	XButton         XB;
{
	XUnmapWindow(((AButton) XB)->display, ((AButton) XB)->window);
} 

/**
 ** XfDestroyXB - Destroy an XButton
 **/

void
XfDestroyXB(XB)
	XButton         XB;
{
	AButton         search, B;

	if (XBList == NULL || XB == NULL)
		return;


	XfDeactivateXB(XB);
	B = (AButton) XB;

	/**
	 ** Remove from modal list if present.
	 **/
	if (XBnModal != 0) {
		XfDeleteFromModalList(XB);
	}

	XDestroyWindow(B->display, B->window);

	if (B->text != NULL)
		free(B->text);

	/**
	 ** Pull button out of XBList
	 **/

	search = (AButton) XBList;
	if (search == B) {
		XBList = search->nextB;
	} else {
		while (search->nextB != (XButton) B)
			search = (AButton) search->nextB;
		search->nextB = ((AButton) (search->nextB))->nextB;
	} free(XB);
}

/**
 ** RedrawXB - Call the appropirate Redraw routine based on an XB's type
 **/

void
RedrawXB(XB)
	XButton         XB;
{
	switch (XB->type) {
	case XF_PB:
		RedrawPB((PButton) XB);
		break;
	 case XF_RB:
		RedrawRB((RButton)XB);
		break;
	 case XF_CB:
		RedrawCB((CButton)XB);
		break;
	 case XF_MB:
		RedrawMB((MButton)XB);
		break;
	 case XF_LB:
		RedrawLB((LButton)XB);
		break;
	}
}

/**
 ** Add an existing XB to the front of the modal list.
 **/
XfAddToModalList(B)
XButton B;
{
	int i;

	if (XBsModal == 0) {
		XBsModal = 8;
		XBModal = calloc(XBsModal, sizeof(XButton));
	}

	if (XBnModal < XBsModal) {
		for (i = 0 ; i < XBsModal ; i++) {
			if (XBModal[i] == NULL) {
				XBModal[i] = B;
				XBnModal++;
				return;
			}
		}
	}
	XBsModal *= 2;
	XBModal = realloc(XBModal, sizeof(XButton)*XBsModal);
	XBModal[XBnModal] = B;
	XBnModal++;

	return;
}

/**
 ** Delete an XB from the modal list.
 **/
XfDeleteFromModalList(B)
XButton B;
{
	int i;

	if (B == NULL || XBModal == NULL || XBnModal == 0) return;

	for (i = 0 ; i < XBsModal ; i++) {
		if (XBModal[i] == B) {
			XBModal[i] = NULL;
			XBnModal--;
			if (XBnModal == 0) {
				XfClearModalList();
			}
			return;
		}
	}
}

XfClearModalList()
{
	if (XBModal == NULL || XBsModal == 0) return;

	free(XBModal);
	XBModal = NULL;
	XBsModal = XBnModal = 0;
}

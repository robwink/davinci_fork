#include	<X11/Xos.h>
#include	<X11/Xlib.h>
#include	<X11/Xutil.h>

/* 
 * Map a window and wait for it to appear.
 *
 * CAUTION: Changes event mask.
 */

MapAndWait(display,w)
Display *display;
Window w;
{
	XEvent	event;
	XWindowAttributes window_attributes;

	XGetWindowAttributes(display, w, &window_attributes);
	XSelectInput(display, w, ExposureMask);
	XFlush(display);
	XMapWindow(display, w);
	XWindowEvent(display,w,ExposureMask, &event);
	XSelectInput(display, w, window_attributes.your_event_mask);
}

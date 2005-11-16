#include	<stdio.h>
#include	<X11/Xos.h>
#include	<X11/Xlib.h>
#include	<X11/Xutil.h>

Display *_xfDisplay;
int _xfScreen;
int _xfDepth;
GC _xfgc;
XFontStruct _xfFontStruct;

int initx(name,d,s,dpth,gcptr)
char *name;
Display **d;
int *s;
int *dpth;
GC *gcptr;
{
	Display *display;
	int screen;
	int depth;
	GC gc;

	display = XOpenDisplay(name);
	if (display == NULL) {
		fprintf(stderr, "Cannot make connection to the X server %s\n",
			XDisplayName(name));
		return(0);
	}

	screen = DefaultScreen( display );
	depth = DefaultDepth( display, screen );
	gc = XCreateGC( display, RootWindow(display, screen), 
		(unsigned long) 0, NULL);
	if (gc == NULL) {
		fprintf(stderr,"Could not create graphics context.\n");
		return(0);
	}
	if (d != NULL) *d=display;
	if (s != NULL) *s = screen;
	if (dpth != NULL) *dpth = depth;
	if (gcptr != NULL) *gcptr = gc;

	/**
	*** Set default values for these
	**/

	_xfDisplay = display;
	_xfScreen = screen;
	_xfDepth = depth;
	_xfgc = gc;

	XfSetDefaultFont(NULL, NULL);

	return(1);
}

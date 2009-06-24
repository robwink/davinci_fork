#include <varargs.h>
#include "Xfred.h"

/*
 * This routine puts up a verify requestor.
 * It returns n, being the option selected by the user.
 *
 *    ---------------------------------
 *    |                               |
 *    |           prompt1             |
 *    |           prompt2             |
 *    |             ...               |
 *    |           promptn             |
 *    |                               |
 *    |--------  --------     --------|
 *    || Opt1 |  | Opt2 | ... | Optn ||
 *    ||______|  |______|     |______||
 *    |_______________________________|
 *
 *
 */

int done;

void
opt_callback(B,E)
Button B;
XEvent *E;
{
	done = 1+(int)B->ext;
}

ConfirmRequestor(display, parent, gc, font, x, y, width, height, 
				 header, warp, warp_opt,
				 va_alist)
	Display *display;
	Window parent;
	GC gc;
	XFontStruct *font;
	int x,y;
	int width,height;
	int header, warp, warp_opt;
    va_dcl
{
	va_list args;
	Window w;
	Button Main;
	Button *Opts;
	char **opts;
	char **prompt;
	int nopts, nprompt;
	int i;
	int max_width;
	int Bwidth;
	int step;
	int max_len;
	int xpos, ypos;
	XEvent E;


	va_start(args);
	done = 0;


    Main = XfCreateButton(display, parent, x, y, width, height, (header ? 1:3), 
			BLACK(display), "verify", 1);
    XfAddButtonVisual(Main, 0, XfCreateVisual(Main, 0, 3, 0, 0,
        WHITE(display), WHITE(display), XfSolidVisual));

	w = Main->window;

	nprompt = va_arg(args, int);
	prompt = (char *(*))calloc(nprompt,sizeof(char *));

	ypos = (height-30)/2-(nprompt/2*15);

	for (i = 0 ; i < nprompt ; i++) {
		prompt[i] = va_arg(args, char *);
		XfAddButtonVisual(Main, 0, XfCreateVisual(Main, 0, ypos, 0, 0,
			BLACK(display), WHITE(display), XfTextVisual,
			prompt[i], font, 0));
		ypos += 15;
	}

	nopts = va_arg(args, int);
	opts = (char *(*))calloc(nopts,sizeof(char *));
	Opts = (Button *)calloc(nopts,sizeof(Button));

	max_len = 0;
	for (i = 0 ; i < nopts ; i++) {
		opts[i] = va_arg(args, char *);
		max_len = (strlen(opts[i]) > max_len ? strlen(opts[i]) : max_len);
	}

    XSetFont(display, gc, font->fid);
    max_width = font->max_bounds.width;

	Bwidth = max_width*max_len+10;
	step = (width-10)/nopts;

/*
 * Center button at x+step/2;
 */
	ypos = height-30;
	xpos = 5;
	for ( i = 0 ; i < nopts ; i++) {
		Opts[i] = XfCreateButton(display, w, (xpos+step/2-Bwidth/2), ypos,
				  Bwidth, 20, 1, BLACK(display), "opt", 1);
		XfAddButtonVisual(Opts[i], 0, XfCreateVisual(Opts[i], 0, 7, 0, 0,
			BLACK(display), WHITE(display), XfTextVisual, 
				opts[i], font, 0));
		XfAddButtonCallback(Opts[i], 0, opt_callback, NULL);
		XfActivateButton(Opts[i], ExposureMask | ButtonPressMask);
		(Opts[i])->ext = (char *)i;
		xpos += step;
	}
	XSetTransientForHint(display, Main->window, parent);
	if (header == 0) {
		XSetWindowAttributes a;
		a.override_redirect = True;
		XChangeWindowAttributes(display, Main->window, CWOverrideRedirect, &a);
	}
    XfActivateButton(Main, ExposureMask);
	if (warp) {
		XWarpPointer(display, None, Opts[warp_opt-1]->window, 0,0,0,0, Bwidth/2, 10);
	}

/*
    XGrabPointer(display, w, True,
		 (ButtonPressMask | ButtonReleaseMask), GrabModeAsync,
		 GrabModeAsync, w, None, CurrentTime);
 */

    while (done == 0) {
        XNextEvent(display, &E);
		XfButtonPush(XfEventButton(&E), &E);
    }

/*
	XUngrabPointer(display, CurrentTime);
*/
	for (i = 0 ; i < nopts ; i++) {
		XfDestroyButton(Opts[i]);
	}
	XfDestroyButton(Main);
	return(done);
}

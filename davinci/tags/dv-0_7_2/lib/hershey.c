#include <stdio.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <math.h>

#include "hershey.h"
#include "hershey.oc.h"

XfHersheyWidth(c, cset)
char    c;
int     cset;
{
	int     ptr;
	char    *str;
	if (cset == 0) {
		ptr = c;
	} else {
		ptr = oc_trans[cset][c-' '];
	}
	str = oc[ptr].letter;
	return((str[1] - 'R') - (str[0] - 'R'));
}

XfHersheyChar(display, drawable, gc, x, y, xscale, yscale, cost, sint, c, cset)
Display *display;
Drawable drawable;
GC gc;
int     x, y;
float   xscale, yscale;
float   cost, sint;
char    c;
int     cset;
{
	char    *str;
	int     count;
	int     ptr;
	float   xt, yt, x2;
	int     xlast, ylast;
	int     i;
	int     move = 1;

	if (cset == 0) {
		ptr = c;
	} else {
		ptr = oc_trans[cset][c-' '];
	}
	str = oc[ptr].letter;
	count = oc[ptr].count;
	for (i = 1 ; i < count ; i++) {
		if (str[i*2] == ' ') {
				move = 1;
				continue;
		}
		xt = str[(i*2)] - 'R';
		yt = str[(i*2)+1] - 'R';
		x2 = x + (xt*cost -yt*sint)*xscale;
		yt = y + (xt*sint +yt*cost)*yscale;
		xt = x2;
		if (!move) {
			XDrawLine(display, drawable, gc, xlast, ylast, (int)xt, (int)yt);
		}
		move = 0;
		xlast = (int)xt;
		ylast = (int)yt;
	}
}

/**
*** Alignment here signifies the percentage in the positive x direction to
*** align the string.  
*** 1.0 is left justified, 0.0 is centered, -1.0 is right justified.
**/

void
XfHersheyString(display, drawable, gc, x, y, 
				xscale, yscale, angle, str, cset, align)
Display *display;
Drawable drawable;
GC gc;
int     x, y;
float   xscale, yscale;
float   angle;
char    *str;
int     cset;
float		align;
{
	int i;
	float cost,sint;
	float x1,y1,xt;
	int count,c;
	float c1,c2,c3,c4;

	if (strlen(str) == 0) return;

	cost = cos(angle);
	sint = sin(angle);

	x1 = x;
	y1 = y;

	count = 0;
	for (i = 0 ; i < strlen(str) ; i++) {
		count += XfHersheyWidth(str[i], cset);
	}

	/**
	*** Adjust for alignment.k
	**/

	x1 = x1 + (align-1.0)*((float)count*cost*xscale/2.0);
	y1 = y1 + (align-1.0)*((float)count*sint*yscale/2.0);

	/**
	*** Each character needs to be centered in its cell.
	**/

	c3 = 0;
	c4 = 0;
	for (i = 0 ; i < strlen(str) ; i++) {
		c = XfHersheyWidth(str[i], cset);
		c1 = c*xscale*cost;
		c2 = c*yscale*sint;
		x = x1+c3+c1/2.0;
		y = y1+c4+c2/2.0;
		XfHersheyChar(display, drawable, gc, x, y, xscale, yscale, 
			cost, sint, str[i], cset);
		c3 += c1;
		c4 += c2;
	}
}

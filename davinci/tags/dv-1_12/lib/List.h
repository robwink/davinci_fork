#ifndef _LIST_H
#define _LIST_H

typedef struct _list * List;

struct _list {
	char	**items;
	int	nitems;
	Button	view;
	Display *display;
	Window	parent;
	Slider	scroll;
	XFontStruct *font;
	int width;
	int height;
	Pixmap	pixmap;
	XImage	image;
	GC		gc;
	int	font_ascent;
	int	font_descent;
	int	font_height;
	int font_width;
	int scrollwidth;
	int	speed;
	int selected;
	int *member;
	unsigned long hilite;
	CallBack	callback;
	Time	last_time;
	int offset;
} *CreateList();

#endif

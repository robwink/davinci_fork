#include "Xfred.h"
#include "List.h"

int	ListSlider();
void ListCallback();

List
CreateList(display, parent, font, x, y, w, h, sw, fast, hilite, nitems, items)
Display *display;
Window parent;
XFontStruct *font;
int	x, y;   /* upper left corner in parent */
int	w, h;   /* width and height of viewport (including scrollbar)*/
int sw;		/* Slider width */
int	fast;   /* use pixmap or not (fast vs slow) */
unsigned long hilite;	/* color to hilite in */
int	nitems;
char	**items;
{
	List list;
	float	thumb_scale;
	float range;
	int i;

	list = (List)calloc(1,sizeof(struct _list));
	list->display = display;
	list->parent = parent;
	list->font = font;
	list->width = w;
	list->height = h;
	list->speed = fast;
	list->hilite = hilite;
	list->nitems = nitems;
	list->items = items;

	list->selected = -1;
	list->last_time = 0;
	list->callback = NULL;
	list->offset = 0;

	list->scrollwidth = sw;

	list->gc = DefaultGC(display, DefaultScreen(display));

	list->font_ascent = font->ascent*1.2;
	list->font_descent = font->descent*1.2;
	list->font_height = list->font_ascent + list->font_descent +1;
	list->font_width = font->max_bounds.width;

	/* view window */

	list->view = XfCreateButton(display, parent, x, y, w, h, 
		1, BLACK(display), " ", 1);
	list->view->member = (int *)list;
	XfAddButtonCallback(list->view, 0, ListCallback, NULL);

	/* slider bar */

	if (nitems == 0) {
		thumb_scale = 1.0;
	} else {
		thumb_scale = ((float)h/(float)list->font_height)/(float)nitems;
		if (thumb_scale > 1.0) thumb_scale = 1.0;
	}

	thumb_scale *= h;

	range = nitems*list->font_height - h;
	if (range < 0) range = 0;

	list->scroll = XfCreateSlider(display, list->view->window, w - sw - 2, -1, 
	    sw, h, 1, BLACK(display), "ThumbSlider", 
	    XfSliderUpDown, (float)range,0.0, (float)list->font_height, 
		sw, (int)thumb_scale);
	XfAddSliderBarVisual(list->scroll,
	    XfCreateVisual(list->scroll, 0, 0, 0, 0, BLACK(display),
	    WHITE(display), XfStippledVisual, "\252\125", 2, 2));
	XfAddSliderThumbVisual(list->scroll, XfCreateVisual(list->scroll,  
		0, 0, sw, (int)thumb_scale, BLACK(display),
	    WHITE(display), XfSolidVisual));
	XfAddSliderThumbVisual(list->scroll, XfCreateVisual(list->scroll,  
		0, 1, sw, (int)thumb_scale-2, WHITE(display),
	    WHITE(display), XfSolidVisual));
	XfAddSliderCallback(list->scroll, ListSlider, NULL);
	XfActivateSliderValue(list->scroll, 1.0, 
	    (ExposureMask | ButtonPressMask | ButtonMotionMask)) ;
	list->scroll->member = (int *)list;

	if (list->speed) {
		list->pixmap = XCreatePixmap(display, parent, w, 
									 nitems*list->font_height, 8);
		XSetForeground(display, list->gc, WHITE(display));
		XFillRectangle(display, list->pixmap, list->gc, 0,0,
				w, nitems*list->font_height);

		XSetForeground(display, list->gc, BLACK(display));
		for (i = 0 ; i < nitems ; i++) {
			draw(list, i, WHITE(display));
		}
		XfAddButtonVisual(list->view, 0, XfCreateVisual(list->view, 
				0,0,w, nitems*list->font_height, BLACK(display), WHITE(display),
				XfPixmapVisual, 8, list->pixmap));
	} else {
		XfNoAutoExposeButton(list->view);
	}

	return(list);
}
ReCreateList(list, nitems, items)
List list;
int	nitems;
char	**items;
{
	float	thumb_scale;
	float range;
	int i;
	Slider s;
	Display *display;

	list->nitems = nitems;
	list->items = items;

	list->selected = -1;
	list->last_time = 0;
	list->offset = 0;

	display = list->view->display;

	if (nitems == 0) {
		thumb_scale = 1.0;
	} else {
		thumb_scale = ((float)list->height/
						(float)list->font_height)/
						(float)list->nitems;
		if (thumb_scale > 1.0) thumb_scale = 1.0;
	}

	thumb_scale *= list->height;

	range = nitems*list->font_height - list->height;
	if (range < 0) range = 0;

	XfRescaleSlider(list->scroll, (float)range, 0.0, (float)list->font_height);
	XfResizeThumb(list->scroll, list->scrollwidth, (int)thumb_scale);
	list->scroll->thumb->height = thumb_scale;
	list->scroll->thumb->next->height = thumb_scale-2;
	XfSetSliderValue(list->scroll, 1.0);

	if (list->speed) {
		XfFreeVisual(display, list->view->States[0]->Visuals);
		list->view->States[0]->Visuals = NULL;
		list->pixmap = XCreatePixmap(display, list->parent, list->width, 
									 nitems*list->font_height, 8);
		XfAddButtonVisual(list->view, 0, XfCreateVisual(list->view, 
				0,0,list->width, nitems*list->font_height, 
				WHITE(display), WHITE(display),
				XfPixmapVisual, 8, list->pixmap));
	}
	RefreshList(list);
}



ListSlider(S, E)
Slider S;
XEvent *E;
{
	Button B;
	List list;
	int offset;
	list = (List)S->member;
	B = list->view;

/* 
   put slow vs fast stuff here.
   Just repaint all necessary strings, at +offset
*/
	offset = -(int)XfGetSliderValue(S);
	list->offset = offset;
	if (list->speed) {
		B->States[0]->Visuals->y_pos = offset;
		(*(B->updateCallback))(B, NULL);
	} else {
		RefreshList(list);
	}
}


void
ListCallback(B, E)
Button B;
XEvent *E;
{
	/* This is a button press in the view window */
	int	i;
	GC gc;
	List list;

	list = (List)B->member;

	if (E->type == Expose) {
		/* Can only get this if in slow mode */
		RefreshList(list);
	} else if (E->type == ButtonPress) {
		i = (E->xbutton.y - list->offset)/ list->font_height;
		if (i >= list->nitems) 
			return;

		if (i != list->selected) {
			if (list->selected != -1) {
				draw(list, list->selected, WHITE(B->display));
			}
			draw(list, i, list->hilite);
			list->selected = i;
		}
		if (list->callback != NULL)
			(*(list->callback))(list, E);
		list->last_time = E->xbutton.time;
	}
}


AddListCallback(list, proc)
List list;
CallBack proc;
{
	list->callback = proc;
}


RefreshList(list)
List list;
{
	Display *display;
	int i;
	int width;
	int base;


	display = list->view->display;
	width = list->view->width;

	XSetForeground(display, list->gc, WHITE(display));
	if (list->speed) {
		XFillRectangle(display, list->pixmap, list->gc, 0,0,
				list->width, list->nitems*list->font_height);
	} else {
		XFillRectangle(display, list->view->window, list->gc, 0,0,
				width, list->view->height+1);
	}
	for (i = 0 ; i < list->nitems ; i++) {
		if (list->selected == i) {
			draw(list, i, list->hilite);
		} else {
			draw(list, i, WHITE(display));
		}
	}
	if (list->speed) {
		(*(list->view->updateCallback))(list->view, NULL);
	}
}

ListIsDoubleClick(list,E)
List list;
XEvent *E;
{
	return(E->xbutton.time - list->last_time < 500 && list->last_time != 0);
}

draw(list, i, bg)
List list;
int i;
short bg;
{
	int loc;
	Button B;
	B = list->view;

	loc = (i * list->font_height);
	if (list->speed) {
	    XSetForeground(B->display, list->gc, bg);
		XFillRectangle(B->display, list->pixmap, list->gc, 0, 
			loc, list->width, 
			list->font_height);
		XSetForeground(B->display, list->gc, BLACK(B->display));
		XDrawString(B->display, list->pixmap, list->gc, 
			list->font_width / 2, 
			list->font_ascent+ loc,
			list->items[i],
			strlen(list->items[i]));
	} else {
		if ((loc + list->offset) > list->height) return;
		if ((loc + list->offset) < -list->font_height) return;
	    XSetForeground(B->display, list->gc, bg);
		XFillRectangle(B->display, list->view->window, list->gc, 0, 
			(loc + list->offset),
			list->width, list->font_height);
		XSetForeground(B->display, list->gc, BLACK(B->display));
		XDrawString(B->display, list->view->window, list->gc, 
			list->font_width / 2, 
			list->font_ascent + loc + list->offset, 
			list->items[i],
			strlen(list->items[i]));
	}
}

List 
DestroyList(list)
List list;
{
	XfDestroyButton(list->view);
	XfDestroySlider(list->scroll);
	free(list);
	return(NULL);
}
ActivateList(list)
List list;
{
	XfActivateButton(list->view, ExposureMask | ButtonPressMask);
}
DeactivateList(list)
List list;
{
	XfDeactivateButton(list->view);
}

/*
 * Slider.c
 *
 * Coding of the routines for the Slider class of interface controls. Each
 * routine is documented separately.
 *
 * $Header$
 *
 * $Log$
 * Revision 1.1  1999/06/16 03:24:23  gorelick
 * Initial revision
 *
 * Revision 1.1  1999/06/16 01:40:46  gorelick
 * Initial install
 *
 * Revision 0.4  91/10/04  01:28:30  01:28:30  rray (Randy Ray)
 * Altered all usage of XEvent structure so that only pointers are passed
 * to functions, rather than entire structures.
 * 
 * Revision 0.3  91/09/23  17:54:29  17:54:29  ngorelic (Noel S. Gorelick)
 * *** empty log message ***
 * 
 * Revision 0.2  91/08/17  17:51:27  17:51:27  rray (Randy Ray)
 * *** empty log message ***
 * 
 * Revision 0.1  91/07/24  18:03:58  18:03:58  rray (Randy Ray)
 * *** empty log message ***
 * 
 *
 */

#include <math.h>
#include "Xfred.h"
#include "Slider.h"
#include "freelist.h"

/* Actual storage for list allocated here */
Slider SliderList = NULL;

/* Forward declaration of callback default */
void displaySlider();

/*
 * Destroy the passed slider
 */
int     XfDestroySlider(S)
Slider S;
{
        Slider search;
        struct VisualInfo *vis;
        struct VisualInfo *mm;
        struct VisualInfo *nn;
        struct CallBackList *call;

        if ((SliderList == NULL) || (S == NULL))
                return(False);

        XfDeactivateSlider(S);
        if (S == SliderList) {
                vis = S->bar;
                nn = vis;
                while (nn != NULL) {
                        mm = nn->next;
                        if (nn->vtype == XfXImageVisual)
                                XFree((char *)nn->visual.i_vis);
                        if ((nn->vtype == XfPixmapVisual) || 
                            (nn->vtype == XfTiledVisual) || 
                            (nn->vtype == XfStippledVisual) || 
                            (nn->vtype == XfOpaqueStippledVisual))
                                XFreePixmap(S->display, nn->visual.p_vis.map);
                        free(nn);
                        nn = mm;
                }
                vis = S->thumb;
                nn = vis;
                while (nn != NULL) {
                        mm = nn->next;
                        if (nn->vtype == XfXImageVisual)
                                XFree((char *)nn->visual.i_vis);
                        if ((nn->vtype == XfPixmapVisual) || 
                            (nn->vtype == XfTiledVisual) || 
                            (nn->vtype == XfStippledVisual) || 
                            (nn->vtype == XfOpaqueStippledVisual))
                                XFreePixmap(S->display, nn->visual.p_vis.map);
                        free(nn);
                        nn = mm;
                }
                call = S->CallBacks;
                FreeList(struct CallBackList *, call, next);
                SliderList = SliderList->nextSlider;
                free(S);
        } else
         {
                search = SliderList;
                while (search->nextSlider != S)
                        search = search->nextSlider;
                /* Search now points to the parent of S */
                vis = S->bar;
                nn = vis;
                while (nn != NULL) {
                        mm = nn->next;
                        if (nn->vtype == XfXImageVisual)
                                XFree((char *)nn->visual.i_vis);
                        if ((nn->vtype == XfPixmapVisual) || 
                            (nn->vtype == XfTiledVisual) || 
                            (nn->vtype == XfStippledVisual) || 
                            (nn->vtype == XfOpaqueStippledVisual))
                                XFreePixmap(S->display, nn->visual.p_vis.map);
                        free(nn);
                        nn = mm;
                }
                vis = S->thumb;
                nn = vis;
                while (nn != NULL) {
                        mm = nn->next;
                        if (nn->vtype == XfXImageVisual)
                                XFree((char *)nn->visual.i_vis);
                        if ((nn->vtype == XfPixmapVisual) || 
                            (nn->vtype == XfTiledVisual) || 
                            (nn->vtype == XfStippledVisual) || 
                            (nn->vtype == XfOpaqueStippledVisual))
                                XFreePixmap(S->display, nn->visual.p_vis.map);
                        free(nn);
                        nn = mm;
                }
                call = S->CallBacks;
                FreeList(struct CallBackList *, call, next);
                search->nextSlider = S->nextSlider;
                free(S);
        }
        return(True);
}


/*
 * Get the pointer to the named slider.
 */
Slider XfGetSlider(name)
char*name;
{
        Slider search;

        search = SliderList;

        while (search != NULL) {
                if (!strcmp(name, search->name))
                        return(search);
                search = search->nextSlider;
        }

        return(NULL);
}


/*
 * Add the passed VisualInfo structure to the linked list maintained for
 * the slider bar or thumb, based on flag.
 */
int     XfAddSliderVisual(S, vis, flag)
Slider S;
struct VisualInfo *vis;
int     flag;            /* 0 for bar, 1 for thumb */
{
        struct VisualInfo *loop;

        if ((S == NULL) || (vis == NULL))
                return(False);

        vis->next = NULL;
        if (flag)
                loop = S->thumb;
        else
                loop = S->bar;
        if (loop == NULL) {
                if (flag)
                        S->thumb = vis;
                else
                        S->bar = vis;
        } else
         {
                while (loop->next != NULL)
                        loop = loop->next;
                loop->next = vis;
        }
        return(True);
}


/*
 * Move the specified slider to the front of the list, to improve search
 * time. No reason for this to fail.
 */
int     XfMoveSlider(S)
Slider S;
{
        Slider search;

        if ((S == NULL) || (S == SliderList))
                return(True);
        search = SliderList;
        while (search->nextSlider != S) {
                if (search->nextSlider == NULL)
                        return(False);
                search = search->nextSlider;
        }
        /* Now that search->nextSlider == S, start shuffling */
        search->nextSlider = S->nextSlider;
        S->nextSlider = SliderList;
        SliderList = S;
        return(True);
}


/*
 * EventSlider returns which slider, if any, the passed event occured in.
 */
Slider XfEventSlider(E)
XEvent*E;
{
        Slider search;

        search = SliderList;
        while (search != NULL) {
                switch (E->type) {
                case NoExpose:
                case GraphicsExpose:
                        break;
                default:
                        if (search->window == E->xany.window)
                                return(search);
                        break;
                }
                search = search->nextSlider;
        }
        return(NULL);
}


/* 
 * Add a callback to the list for the slider specified. Return
 * an error for any of the many things that could go wrong.
 */
int     XfAddSliderCallback(S, new, old)
Slider S;  /* The slider */
CallBack new;
CallBack old;
{
        struct CallBackList *new_ptr;
        struct CallBackList *loop;
        struct CallBackList *loopn;

        if (S == NULL)
                return(False);

        new_ptr = (struct CallBackList *)calloc(1,sizeof(struct CallBackList ));
        if (new_ptr == NULL)
                return(False);
        new_ptr->proc = new;
        loop = S->CallBacks;

        if (loop == NULL || loop->proc == old) {
                /* Must insert at head of list */
                new_ptr->next = loop;
                S->CallBacks = new_ptr;
        } else
         {
                loopn = loop->next;
                while (loopn != NULL) {
                        if (loopn->proc == old)
                                break;
                        else
                                loop = loopn, loopn = loop->next;
                }
                if (loopn == NULL) {
                        /* Not found. Unless old is NULL (add at end), error */
                        if (old == NULL) {
                                new_ptr->next = NULL;
                                loop->next = new_ptr;
                        } else
                                return(False);
                } else
                 {
                        /* Found it. Insert new callback before */
                        new_ptr->next = loop->next;
                        loop->next = new_ptr;
                }
        }
        return(True);
}


/*
 * Delete the named callback from the list. Error if cb not a member of list.
 */
int     XfDelSliderCallback(S, cb)
Slider S;
CallBack cb;
{
        struct CallBackList *loop;
        struct CallBackList *loopn;

        if ((S == NULL) || (cb == NULL))
                return(False);

        loop = S->CallBacks;
        if (loop == NULL)
                return(False);
        if (loop->proc == cb) {
                S->CallBacks = S->CallBacks->next;
                free(loop);
                return(True);
        }
        loopn = loop->next;
        while (loopn != NULL) {
                if (loopn->proc == cb) {
                        loop = loopn->next;
                        free(loopn);
                        return(True);
                }
                loop = loopn;
                loopn = loop->next;
        }
        return(False);        /* Callback was not found */
}


/*
 * Un-map and select input to zero.
 */
int     XfDeactivateSlider(S)
Slider S;
{
        if (S == NULL)
                return(False);
        S->active = False;
        XUnmapWindow(S->display, S->window);
        return(True);
}


/* 
 * Map the slider's window, set it's event mask, and mark it active.
 */
int     XfActivateSliderValue(S, value, mask)
Slider S;
float   value;
unsigned long   mask;
{
        if (S == NULL)
                return(False);
        S->active = True;
        if (value < 0.0)
                S->value = 0.0;
        else if (value > 1.0)
                S->value = 1.0;
        else
                S->value = value;
        XSelectInput(S->display, S->window, mask);
        XMapRaised(S->display, S->window);
        XFlush(S->display);
        return(True);
}


/*
 * Create a new slider, placing it at the end of the list.
 */
Slider XfCreateSlider(display, parent, x, y, width, height, border_width,
border_color, name, orientation, min, max, delta,
thumb_width, thumb_height)
Display*display;
Window parent;
int     x, y;
int     width, height;
int     border_width;
unsigned long   border_color;
char*name;
int     orientation;
float   min, max;
float   delta;
int     thumb_width, thumb_height;
{
        Slider new;
        Slider search;

        new = (Slider)calloc(1,sizeof(struct _Slider ));
        if (new == NULL)
                return(NULL);

        new->parent = parent;
        new->display = display;
        new->x = x;
        new->y = y;
        new->height = height;
        new->width = width;
        new->border_width = border_width;
        new->border_color = border_color;
        strcpy(new->name, name);
        new->max = max;
        new->min = min;
        new->delta = delta;
        new->value = 0.0;
        new->orientation = orientation;
        new->thumb_width = thumb_width;
        new->thumb_height = (thumb_height <= 0 ? 1 : thumb_height);
        new->bar = (struct VisualInfo *)NULL;
        new->thumb = (struct VisualInfo *)NULL;
        new->CallBacks = (struct CallBackList *)NULL;
        new->window = XCreateSimpleWindow(display, parent, x, y, width, height,
            border_width, border_color, 0);
        new->exposeCallback = defaultSliderCallback;
        new->updateCallback = defaultSliderUpdateCallback;
        new->active = False;
        new->nextSlider = NULL;
        switch (orientation) {
        case XfSliderUpDown:
                /* min_pixel and max_pixel are the Y axis limits */
                new->thumb_min_pixel = 0;
                new->thumb_max_pixel = height - thumb_height;
                new->thumb_cur_pixel = new->thumb_max_pixel;
                break;
        case XfSliderLeftRight:
                /* min_pixel and max_pixel are the X axis limits */
                new->thumb_min_pixel = 0;
                new->thumb_max_pixel = width - thumb_width;
                new->thumb_cur_pixel = 0;
        }

        search = SliderList;
        if (search == NULL)
                SliderList = new;
        else
         {
                while (search->nextSlider != NULL)
                        search = search->nextSlider;
                search->nextSlider = new;
        }
        return(new);
}


/*
 * Evaluate slider S's response to event E. It has already been determined
 * that the event occured in S.
 */
int     XfSliderResponse(S, E)
Slider S;
XEvent*E;
{
        struct CallBackList *execs;
        float   cur_value, tmp_value;
        int     point, x, y;
        XEvent EE;

        if (S == NULL)
                return(False);
        else
                execs = S->CallBacks;

        cur_value = XfGetSliderValue(S);
        tmp_value = cur_value;

        switch (E->type) {
        case Expose:
                while (XCheckTypedWindowEvent(S->display, S->window, Expose, &EE))
                        ;
                (*(S->exposeCallback))(S, E);
                return(True);
                break;
        case ButtonPress:
                while (XCheckMaskEvent(S->display, ButtonPressMask, &EE))
                        ;
                if (E->xbutton.button == Button3) {
                        /* Move slider towards max by delta value */
                        cur_value += S->delta * (S->max > S->min ? 1.0 : -1.0);
                        if (((cur_value > S->max) && (S->max > S->min)) || 
                            ((cur_value < S->max) && (S->max < S->min)))
                                cur_value = S->max;
                } else if (E->xbutton.button == Button2) {
                        /* Move slider towards min by delta value */
                        cur_value -= S->delta * (S->max > S->min ? 1.0 : -1.0);
                        if (((cur_value < S->min) && (S->min < S->max)) || 
                            ((cur_value > S->min) && (S->min > S->max)))
                                cur_value = S->min;
                } else /* Button1 */                {
                        point = ((S->orientation == XfSliderUpDown) ? 
                            (E->xbutton.y - (S->thumb_height / 2)) : 
                            (E->xbutton.x - (S->thumb_width / 2)));
                        point -= S->thumb_min_pixel;
                        if (point < 0)
                                point = 0;
                        if (S->thumb_max_pixel - S->thumb_min_pixel == 0)
                                cur_value = 0;
                        else
                         {
                                cur_value = (float)point / 
                                    (float)(S->thumb_max_pixel - 
                                    S->thumb_min_pixel);
                                if (cur_value < 0.0)
                                        cur_value = 0.0;
                                else if (cur_value > 1.0)
                                        cur_value = 1.0;
                                if (S->orientation == XfSliderUpDown)
                                        cur_value = 1.0 - cur_value;
                        }
                        S->value = cur_value;
                        if (cur_value == 1.0)
                                cur_value = S->max;
						else if (cur_value == 0.0)
                                cur_value = S->min;
                        else
                         {
                                cur_value = XfGetSliderValue(S);
                                cur_value /= S->delta;
                                cur_value = (float)floor((double)cur_value);
                                cur_value *= S->delta;
                        }
                }
                break;
        case MotionNotify:
                if (E->xmotion.state & Button1Mask) {
                        while (XCheckMaskEvent(S->display, ButtonMotionMask, E))
                                ;
                        x = E->xmotion.x - (S->thumb_width / 2);
                        y = E->xmotion.y - (S->thumb_height / 2);
                        point = ((S->orientation == XfSliderUpDown) ? y : x);
                        point -= S->thumb_min_pixel;
                        if (S->thumb_max_pixel - S->thumb_min_pixel == 0) {
                                cur_value = 0;
                        } else
                         {
                                if (S->thumb_max_pixel - S->thumb_min_pixel == 0)
                                        cur_value = 0;
                                else
                                 {
                                        cur_value = (float)point / 
                                            (float)(S->thumb_max_pixel - 
                                            S->thumb_min_pixel);
                                        if (cur_value < 0.0)
                                                cur_value = 0.0;
                                        else if (cur_value > 1.0)
                                                cur_value = 1.0;
                                        if (S->orientation == XfSliderUpDown)
                                                cur_value = 1.0 - cur_value;
                                }
                        }
                        S->value = cur_value;
                        if (cur_value == 1.0)
                                cur_value = S->max;
                        else if (cur_value == 0.0)
                                cur_value = S->min;
                        else
                         {
                                cur_value = XfGetSliderValue(S);
                                cur_value /= S->delta;
                                cur_value = (float)floor((double)cur_value);
                                cur_value *= S->delta;
                        }
                }
                break;
        }
        if (cur_value == tmp_value) 
                return(True);
        if (S->max - S->min == 0)
                cur_value = 0;
        else
                cur_value = (cur_value - S->min) / (S->max - S->min);
        S->value = cur_value;
        (*(S->updateCallback))(S, E);
        while (execs != NULL) {
                if (execs->proc != NULL)
                        (*(execs->proc))(S, E);
                execs = execs->next;
        }
        return(True);
}


/*
 * Default callback to process expose events
 */
void defaultSliderCallback(S, E)
Slider S;
XEvent*E;
{
        struct VisualInfo *vis;       /* For traversing linked lists */
        GC localGC;                   /* Inherited from parent */
        unsigned long   bg;             /* Color for filling bar initially */
        float   value;                  /* Used to extract value from S */

        /* First, update S->thumb_cur_pixel */
        value = S->value;
        if (S->orientation == XfSliderUpDown)
                value = 1.0 - value;
        S->thumb_cur_pixel = (S->thumb_min_pixel + 
            (value * (S->thumb_max_pixel - S->thumb_min_pixel)));
        localGC = DefaultGC(S->display, DefaultScreen(S->display));
        vis = S->bar;
        if (vis != NULL)
                bg = vis->background;
        else
                bg = S->border_color;
        XSetForeground(S->display, localGC, bg);
        XFillRectangle(S->display, S->window, localGC, 0, 0, S->width, S->height);

        displaySlider(S);
}


/*
 * Update callback-- redraw only necessary area
 */
void defaultSliderUpdateCallback(S, E)
Slider S;
XEvent*E;
{
        struct VisualInfo *vis;       /* For traversing linked lists */
        GC localGC;                   /* Inherited from parent */
        unsigned long   bg;             /* Color for filling bar initially */
        int     x_off, y_off;             /* Offsets for thumb displaying */
        float   value;                  /* Used to extract value from S */

        localGC = DefaultGC(S->display, DefaultScreen(S->display));
        vis = S->bar;
        if (vis != NULL)
                bg = vis->background;
        else
                bg = S->border_color;
        XSetForeground(S->display, localGC, bg);
        if (S->orientation == XfSliderUpDown) {
                y_off = S->thumb_cur_pixel;
                x_off = (S->width - S->thumb_width) / 2;
        } else
         {
                x_off = S->thumb_cur_pixel;
                y_off = (S->height - S->thumb_height) / 2;
        }
        XFillRectangle(S->display, S->window, localGC, x_off, y_off,
            S->thumb_width, S->thumb_height);

        value = S->value;
        if (S->orientation == XfSliderUpDown)
                value = 1.0 - value;
        S->thumb_cur_pixel = (S->thumb_min_pixel + 
            (value * (S->thumb_max_pixel - S->thumb_min_pixel)));

        displaySlider(S);
}


/* Since so much code was common... */
void displaySlider(S)
Slider S;
{
        int     x_off, y_off;
        struct VisualInfo *vis;
        int     i;
        GC localGC;         /* Inherited from parent */
        XCharStruct xcs;    /* Character structure for XQueryTextExtents */
        int     dir, asc, des;  /* Used in calls to XQueryTextExtents */

        localGC = DefaultGC(S->display, DefaultScreen(S->display));
        vis = S->bar;
        x_off = 0;
        y_off = 0;

        for (i = 0; i < 2; i++) {
                if (i) {
                        vis = S->thumb;
                        if (S->orientation == XfSliderUpDown) {
                                y_off = S->thumb_cur_pixel;
                                x_off = (S->width - S->thumb_width) / 2;
                        } else
                         {
                                x_off = S->thumb_cur_pixel;
                                y_off = (S->height - S->thumb_height) / 2;
                        }
                }
                while (vis != NULL) {
                        switch (vis->vtype) {
                        case XfXImageVisual:
                                XPutImage(S->display, S->window, localGC,
                                    vis->visual.i_vis, 0,
                                    0, (vis->x_pos + x_off),
                                    (vis->y_pos + y_off),
                                    vis->width, vis->height);
                                break;
                        case XfPixmapVisual:
                                XSetForeground(S->display, localGC, vis->foreground);
                                XSetBackground(S->display, localGC, vis->background);
                                if (vis->visual.p_vis.depth == 1)
                                        XCopyPlane(S->display, vis->visual.p_vis.map,
                                            S->window, localGC, 0, 0,
                                            vis->width,
                                            vis->height,
                                            (vis->x_pos + x_off),
                                            (vis->y_pos + y_off), 1);
                                else
                                        XCopyArea(S->display, vis->visual.p_vis.map,
                                            S->window, localGC, 0, 0, vis->width,
                                            vis->height, (vis->x_pos + x_off),
                                            (vis->y_pos + y_off));
                                break;
                        case XfTextVisual:
                                XSetFont(S->display, localGC,
                                    vis->visual.t_vis.font->fid);
                                XSetForeground(S->display, localGC, vis->foreground);
                                XTextExtents(vis->visual.t_vis.font,
                                    vis->visual.t_vis.text,
                                    strlen(vis->visual.t_vis.text),
                                    &dir, &asc, &des, &xcs);
                                switch (vis->visual.t_vis.align) {
                                case 0: /* Center */
                                        dir = xcs.width / 2;
                                        if (vis->width == 0)
                                                asc = (vis->x_pos + x_off) + S->width / 2;
                                        else
                                                asc = (vis->x_pos + x_off) + vis->width / 2;
                                        asc -= dir;
                                        des = (vis->y_pos + y_off) + xcs.ascent;
                                        break;
                                case 1: /* Left justify */
                                        asc = (vis->x_pos + x_off);
                                        des = (vis->y_pos + y_off) + xcs.ascent;
                                        break;
                                case 2: /* Right justify */
                                        asc = vis->width;
                                        if (asc == 0)
                                                asc = S->width;
                                        asc -= xcs.width;
                                        des = (vis->y_pos + y_off) + xcs.ascent;
                                        break;
                                }
                                XDrawString(S->display, S->window, localGC, asc,
                                    des, vis->visual.t_vis.text,
                                    strlen(vis->visual.t_vis.text));
                                break;
                        case XfOutlineVisual:
                        case XfSolidVisual:
                                XSetForeground(S->display, localGC, vis->foreground);
                                if (vis->vtype == XfSolidVisual)
                                        XFillRectangle(S->display, S->window, localGC,
                                            (vis->x_pos + x_off),
                                            (vis->y_pos + y_off),
                                            vis->width, vis->height);
                                else
                                        XDrawRectangle(S->display, S->window, localGC,
                                            (vis->x_pos + x_off),
                                            (vis->y_pos + y_off),
                                            vis->width, vis->height);
                                break;
                        case XfStippledVisual:
                        case XfOpaqueStippledVisual:
                                XSetBackground(S->display, localGC, vis->background);
                                XSetForeground(S->display, localGC, vis->foreground);
                                XSetStipple(S->display, localGC,
                                    vis->visual.p_vis.map);
                                XSetFillStyle(S->display, localGC,
                                    ((vis->vtype == XfStippledVisual) ? 
                                    FillStippled : FillOpaqueStippled));
                                XFillRectangle(S->display, S->window, localGC,
                                    (vis->x_pos + x_off),
                                    (vis->y_pos + y_off),
                                    vis->width, vis->height);
                                XSetFillStyle(S->display, localGC, FillSolid);
                                break;
                        case XfTiledVisual:
                                XSetTile(S->display, localGC, vis->visual.p_vis.map);
                                XSetFillStyle(S->display, localGC, FillTiled);
                                XFillRectangle(S->display, S->window, localGC,
                                    (vis->x_pos + x_off),
                                    (vis->y_pos + y_off),
                                    vis->width, vis->height);
                                XSetFillStyle(S->display, localGC, FillSolid);
                                break;
                        case XfHersheyVisual:
                                XSetForeground(S->display, localGC, vis->foreground);
                                XfHersheyString(S->display, S->window, localGC, 
                                    (vis->x_pos + x_off),
                                    (vis->y_pos + y_off), 
                                    vis->visual.h_vis.scale,
                                    vis->visual.h_vis.scale, 
                                    vis->visual.h_vis.angle,
                                    vis->visual.h_vis.text, 
                                    vis->visual.h_vis.cset,
									vis->visual.h_vis.align);
                                break;
                        }
                        vis = vis->next;
                }
        }
}


/*
 * Force a slider value and call the callbacks, as well as update
 */
int     XfSetSliderValue(S, v)
Slider S;
float   v;
{
        if (S == NULL)
                return(False);

        if (v < 0.0)
                S->value = 0.0;
        else if (v > 1.0)
                S->value = 1.0;
        else
                S->value = v;

        (*(S->updateCallback))(S, NULL);
        return(True);
}


/*
 * Re-scale the numerical values associated with the slider
 */
int     XfRescaleSlider(S, lo, hi, d)
Slider S;
float   lo, hi, d;
{
        if (S == NULL)
                return(False);

        S->min = lo;
        S->max = hi;
        S->delta = d;
        return(True);
}

void
XfResizeThumb(S, width, height)
Slider S;
int width;
int height;
{
        S->thumb_height = height;
        S->thumb_width = width;

        switch (S->orientation) {
                case XfSliderUpDown:
                        /* min_pixel and max_pixel are the Y axis limits */
                        S->thumb_min_pixel = 0;
                        S->thumb_max_pixel = S->height - S->thumb_height;
                        break;
                case XfSliderLeftRight:
                        /* min_pixel and max_pixel are the X axis limits */
                        S->thumb_min_pixel = 0;
                        S->thumb_max_pixel = S->width - S->thumb_width;
        }
}

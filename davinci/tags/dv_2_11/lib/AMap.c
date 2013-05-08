/*
 * AMap.c
 *
 * The coding of the routines that implement the Association Map tool.
 * Each routine is (poorly) documented immediately prior to it's
 * declaration.
 *
 * $Header$
 *
 * $Log$
 * Revision 1.1  1999/06/16 03:24:23  gorelick
 * Initial revision
 *
 * Revision 1.1  1999/06/16 01:40:30  gorelick
 * Initial install
 *
 * Revision 0.3  91/10/04  01:27:54  01:27:54  rray (Randy Ray)
 * Altered all usage of XEvent structure so that only pointers are passed
 * to functions, rather than entire structures.
 * 
 * Revision 0.2  91/09/23  17:54:24  17:54:24  ngorelic (Noel S. Gorelick)
 * *** empty log message ***
 * 
 * Revision 0.1  91/08/17  17:49:53  17:49:53  rray (Randy Ray)
 * *** empty log message ***
 * 
 *
 */

#include "Xfred.h"
#include "AMap.h"
#include "freelist.h"

/* Defined for actual storage */
AMap AMapList = NULL;

/*
 * Delete the passed AMap
 */
int XfDestroyAMap(A)
    AMap A;
{
    AMap search;
    struct VisualInfo *vis;
    struct VisualInfo *mm;
    struct VisualInfo *nn;
    struct CallBackList *call;
    
    if ((AMapList == NULL) || (A == NULL))
        return(False);
    
    XfDeactivateAMap(A);
    
    if (A == AMapList)
        {
            vis = A->visual;
            call = A->CallBacks;
            
            nn = vis;
            while (nn != NULL)
                {
                    mm = nn->next;
                    if (nn->vtype == XfXImageVisual)
                        XFree((char *)nn->visual.i_vis);
                    if ((nn->vtype == XfPixmapVisual) || 
                        (nn->vtype == XfTiledVisual) || 
                        (nn->vtype == XfStippledVisual) || 
                        (nn->vtype == XfOpaqueStippledVisual))
                        XFreePixmap(A->display, nn->visual.p_vis.map);
                    free(nn);
                    nn = mm;
                }
            FreeList(struct CallBackList *, call, next);
            AMapList = AMapList->nextAMap;
            XFreePixmap(A->display, A->map);
            if ((A->shade_style == FillSolid) || (A->shade_style == 0))
                XFreePixmap(A->display, A->shade_pattern);
            free(A);
        }
    else
        {
            search = AMapList;
            while (search->nextAMap != A)
                search = search->nextAMap;
            vis = A->visual;
            call = A->CallBacks;
            
            nn = vis;
            while (nn != NULL)
                {
                    mm = nn->next;
                    if (nn->vtype == XfXImageVisual)
                        XFree((char *)nn->visual.i_vis);
                    if ((nn->vtype == XfPixmapVisual) || 
                        (nn->vtype == XfTiledVisual) || 
                        (nn->vtype == XfStippledVisual) || 
                        (nn->vtype == XfOpaqueStippledVisual))
                        XFreePixmap(A->display, nn->visual.p_vis.map);
                    free(nn);
                    nn = mm;
                }
            FreeList(struct CallBackList *, call, next);
            search->nextAMap = A->nextAMap;
            XFreePixmap(A->display, A->map);
            if ((A->shade_style == FillSolid) || (A->shade_style == 0))
                XFreePixmap(A->display, A->shade_pattern);
            free(A);
        }
    return(True);
}


/*
 * Get the AMap named
 */
AMap XfGetAMap(name)
    char*name;
{
    AMap search;
    
    search = AMapList;
    
    while (search != NULL)
        {
            if (!strcmp(name, search->name))
                return(search);
            search = search->nextAMap;
        }
    
    return(NULL);
}


/*
 * Add the passed VisualInfo structure tothe linked list maintained for
 * the AMap.
 */
int XfAddAMapVisual(A, vis)
    AMap A;
    struct VisualInfo *vis;
{
    struct VisualInfo *loop;
    
    if ((A == NULL) || (vis == NULL))
        return(False);
    
    vis->next = NULL;
    loop = A->visual;
    if (loop == NULL)
        A->visual = vis;
    else
        {
            while (loop->next != NULL)
                loop = loop->next;
            loop->next = vis;
        }
    return(True);
}


/*
 * Move the specified AMap to the head of the list.
 */
int XfMoveAMap(A)
    AMap A;
{
    AMap search;
    
    if ((A == NULL) || (A == AMapList))
        return(True);
    search = AMapList;
    while (search->nextAMap != A)
        {
            if (search->nextAMap == NULL)
                return(False);
            search = search->nextAMap;
        }
    search->nextAMap = A->nextAMap;
    A->nextAMap = AMapList;
    AMapList = A;
    return(True);
}


/*
 * EventAMap return which AMap E occured in, or NULL
 */
AMap XfEventAMap(E)
    XEvent* E;
{
    AMap search;
    
    search = AMapList;
    while (search != NULL)
        {
            switch (E->type)
                {
                case NoExpose:
                case GraphicsExpose:
                    break;
                default:
                    if (search->window == E->xany.window)
                        return(search);
                    break;
                }
            search = search->nextAMap;
        }
    return(NULL);
}


/*
 * Add a callback to the list for the AMap specified
 */
int XfAddAMapCallback(A, new, old)
    AMap A;
    CallBack new;
    CallBack old;
{
    struct CallBackList *new_ptr;
    struct CallBackList *loop;
    struct CallBackList *loopn;
    
    if (A == NULL)
        return(False);
    
    new_ptr = (struct CallBackList *)calloc(1,sizeof(struct CallBackList ));
    if (new_ptr == NULL)
        return(False);
    new_ptr->proc = new;
    loop = A->CallBacks;
    
    if (loop == NULL || loop->proc == old)
        {
            /* insert at head of list */
            new_ptr->next = loop;
            A->CallBacks = new_ptr;
        }
    else
        {
            loopn = loop->next;
            while (loopn != NULL)
                {
                    if (loopn->proc == old)
                        break;
                    else
                        loop = loopn, loopn = loop->next;
                }
            if (loopn == NULL)
                {
                    if (old == NULL)
                        {
                            new_ptr->next = NULL;
                            loop->next = new_ptr;
                        }
                    else
                        {
                            free(new_ptr);
                            return(False);
                        }
                }
            else
                {
                    new_ptr->next = loop->next;
                    loop->next = new_ptr;
                }
        }
    return(True);
}

int XfAddAMapReadoutCallback(A, new)
    AMap A;
    CallBack new;
{
	A->readoutCallback = new;
}


/*
 * Delete the named callback from the list, if it is there
 */
int XfDelAMapCallback(A, cb)
    AMap A;
    CallBack cb;
{
    struct CallBackList *loop;
    struct CallBackList *loopn;
    
    if ((A == NULL) || (cb == NULL))
        return(False);
    
    loop = A->CallBacks;
    if (loop == NULL)
        return(False);
    if (loop->proc == cb)
        {
            A->CallBacks = A->CallBacks->next;
            free(loop);
            return(True);
        }
    loopn = loop->next;
    while (loopn != NULL)
        {
            if (loopn->proc == cb)
                {
                    loop = loopn->next;
                    free(loopn);
                    return(True);
                }
            loop = loopn;
            loopn = loop->next;
        }
    return(False);
}


/*
 * Un-map and select no input
 */
int XfDeactivateAMap(A)
    AMap A;
{
    if (A == NULL)
        return(False);
    A->active = False;
    XSelectInput(A->display, A->window, 0);
    XUnmapWindow(A->display, A->window);
    return(True);
}


/*
 * Map the AMap's window, etc.
 */
int XfActivateAMapMode(A, mode, mask)
    AMap A;
    int mode;
    unsigned long   mask;
{
    if (A == NULL)
        return(False);
    A->active = True;
    A->left_edge = A->width;
    A->action_mode = mode;
    XSelectInput(A->display, A->window, mask);
    XMapRaised(A->display, A->window);
    XFlush(A->display);
    XfClearAMap(A);
    return(True);
}


/*
 * Create a new AMap
 */
AMap XfCreateAMap(display, parent, x, y, width, height, border_width,
                  border_color, name, pen_color, shade_color, shade_style,
                  shade_pattern)
    Display*display;
    Window parent;
    int x, y;
    int width, height;
    int border_width;
    unsigned long   border_color;
    char*name;
    unsigned long   pen_color, shade_color;
    int shade_style;
    Pixmap shade_pattern;
{
    AMap new;
    AMap search;
    
    if ((height == 0) || (width == 0))
        return(NULL);
    
    /*
     */
    new = (AMap)calloc(1,sizeof(struct _AMap));
    if (new == NULL)
        return(NULL);
    
    new->points = (struct Point *)calloc((width * 3), sizeof(struct Point ));
    if (new->points == NULL)
        {
            free(new);
            return(NULL);
        }
    new->parent = parent;
    new->display = display;
    new->x = x;
    new->y = y;
    new->height = height;
    new->width = width;
    new->border_width = border_width;
    new->border_color = border_color;
    strcpy(new->name, name);
    new->max_left_edge = (width * 2) - 1;
    new->pen_color = pen_color;
    new->shade_color = shade_color;
    new->shade_style = shade_style;
    if (!((shade_style == FillSolid) || (shade_style == 0)))
        new->shade_pattern = shade_pattern;
    new->shade_left = 0;
    new->shade_right = 0;
    new->active = False;
    new->visual = (struct VisualInfo *)NULL;
    new->CallBacks = (struct CallBackList *)NULL;
    new->exposeCallback = defaultAMapCallback;
    new->updateCallback = defaultAMapUpdateCallback;
    new->readoutCallback = NULL;
    new->window = XCreateSimpleWindow(display, parent, x, y, width, height,
                                      border_width, border_color, 0);
    new->map = XCreatePixmap(display, new->window, width, height,
                             DefaultDepth(display, DefaultScreen(display)));
    new->nextAMap = NULL;
    
    search = AMapList;
    if (search == NULL)
        AMapList = new;
    else
        {
            while (search->nextAMap != NULL)
                search = search->nextAMap;
            search->nextAMap = new;
        }
    return(new);
}


/*
 * Set the map A with the passed data, spaced as evenly as possible over the
 * width of the map
 */
int XfSetAMap(A, data, npoints)
    AMap A;
    int data[];
    int npoints;
{
    int i, off, base;
    
    if ((A == NULL) || (npoints > A->width) || (data == NULL))
        return(False);
    
    A->left_edge = A->max_left_edge - A->width;
    
    if (npoints == 1) /* special case */
        {
            for (i = 0; i < (A->width * 3); i++)
                A->points[i].v = data[0], A->points[i].flag = 0;
            A->points[A->left_edge].flag = 1;
            A->points[A->left_edge + A->width - 1].flag = 1;
            A->points[0].flag = 1;
            A->points[(A->width * 3) - 1].flag = 1;
        }
    else
        {
            for (i = 0; i < (A->width * 3); i++)
                A->points[i].flag = 0;
            base = A->left_edge - 1;
            off = A->width / npoints;
            base += off;
            A->points[A->left_edge].v = data[0];
            A->points[A->left_edge].flag = 1;
            for (i = 1; i < (npoints - 1); i++)
                A->points[base].v = data[i], A->points[base].flag = 1,
                base += off;
            A->points[A->left_edge + A->width - 1].v = data[npoints - 1];
            A->points[A->left_edge + A->width - 1].flag = 1;
            A->points[0].v = data[0];
            A->points[0].flag = 1;
            A->points[(A->width * 3) - 1].v = data[npoints - 1];
            A->points[(A->width * 3) - 1].flag = 1;
        }
    
    XfinterpolateAMap(A);
    return(True);
}


/*
 * Get an array of A->width size, containing the data starting from
 * left edge
 */
int*XfAMapValue (A, data)
    AMap A;
    int data[];
{
    int i;
    
    if ((A == NULL) || (data == NULL))
        return(NULL);
    XfinterpolateAMap(A);
    for (i = 0; i < A->width; i++)
        data[i] = A->points[A->left_edge + i].v;
    
    return(data);
}


/*
 * Interpolate data for the un-flagged points
 */
int XfinterpolateAMap(A)
    AMap A;
{
    int front, end, i;
    float   delta;
    
    if (A == NULL)
        return(False);
    front = 0;
    end = 1;
    while (front != ((A->width * 3) - 1))
        {
            while (A->points[end].flag == 0)
                end++;
            delta = (float)(A->points[end].v - A->points[front].v);
            delta /= (float)(end - front);
            for (i = 1; i < (end - front); i++)
                A->points[front + i].v = 
                    (int)((float)A->points[front].v + i * delta);
            front = end;
            end++;
        }
    return(True);
}


/*
 * The default display callback for AMap's
 */
void defaultAMapCallback(A, E)
    AMap A;
    XEvent* E;
{
    struct VisualInfo *vis;
    GC localGC;
    unsigned long   bg;
    XCharStruct xcs;
    int dir, asc, des;
    
    localGC = DefaultGC(A->display, DefaultScreen(A->display));
    vis = A->visual;
    if (vis != NULL)
        bg = vis->foreground;
    else
        bg = A->border_color;
    XSetForeground(A->display, localGC, bg);
    XFillRectangle(A->display, A->map, localGC, 0, 0, A->width, A->height);
    while (vis != NULL)
        {
            switch (vis->vtype)
                {
                case XfXImageVisual:
                    XPutImage(A->display, A->map, localGC, vis->visual.i_vis,
                              0, 0, vis->x_pos, vis->y_pos, vis->width,
                              vis->height);
                    break;
                case XfPixmapVisual:
				    XSetForeground(A->display, localGC, vis->foreground);
				    XSetBackground(A->display, localGC, vis->background);
                    if (vis->visual.p_vis.depth == 1)
                        XCopyPlane(A->display, vis->visual.p_vis.map, A->map,
                                   localGC, 0, 0, vis->width, vis->height,
                                   vis->x_pos,vis->y_pos, 1);
                    else
                        XCopyArea(A->display, vis->visual.p_vis.map, A->map,
                                  localGC, 0, 0, vis->width, vis->height,
                                  vis->x_pos, vis->y_pos);
                    break;
                case XfTextVisual:
                    XSetFont(A->display, localGC, vis->visual.t_vis.font->fid);
                    XSetForeground(A->display, localGC, vis->foreground);
                    XTextExtents(vis->visual.t_vis.font,
                                      vis->visual.t_vis.text,
                                      strlen(vis->visual.t_vis.text), &dir,
                                      &asc, &des, &xcs);
                    switch (vis->visual.t_vis.align)
                        {
                        case 0: /* Center */
                            dir = xcs.width / 2;
                            if (vis->width == 0)
                                asc = vis->x_pos + A->width;
                            else
                                asc = vis->x_pos + vis->width;
                            asc /= 2;
                            asc -= dir;
                            des = vis->y_pos + xcs.ascent;
                            break;
                        case 1: /* Left justify */
                            asc = vis->x_pos;
                            des = vis->y_pos + xcs.ascent;
                            break;
                        case 2: /* Right justify */
                            asc = vis->width;
                            if (asc == 0)
                                asc = A->width;
                            asc -= xcs.width;
                            des = vis->y_pos + xcs.ascent;
                            break;
                        }
                    XDrawString(A->display, A->map, localGC, asc, des, 
                                vis->visual.t_vis.text,
                                strlen(vis->visual.t_vis.text));
                    break;
                case XfOutlineVisual:
                case XfSolidVisual:
                    XSetForeground(A->display, localGC, vis->foreground);
                    if (vis->vtype == XfSolidVisual)
                        XFillRectangle(A->display, A->map, localGC,vis->x_pos, 
                                       vis->y_pos, vis->width, vis->height);
                    else
                        XDrawRectangle(A->display, A->map, localGC,vis->x_pos, 
                                       vis->y_pos, vis->width, vis->height);
                    break;
                case XfStippledVisual:
                case XfOpaqueStippledVisual:
                    XSetBackground(A->display, localGC, vis->background);
                    XSetForeground(A->display, localGC, vis->foreground);
                    XSetStipple(A->display, localGC, vis->visual.p_vis.map);
                    XSetFillStyle(A->display, localGC,
                                  ((vis->vtype == XfStippledVisual) ? 
                                   FillStippled : FillOpaqueStippled));
                    XFillRectangle(A->display, A->map, localGC, vis->x_pos, 
                                   vis->y_pos, vis->width, vis->height);
                    XSetFillStyle(A->display, localGC, FillSolid);
                    break;
                case XfTiledVisual:
                    XSetTile(A->display, localGC, vis->visual.p_vis.map);
                    XSetFillStyle(A->display, localGC, FillTiled);
                    XFillRectangle(A->display, A->map, localGC, vis->x_pos, 
                                   vis->y_pos, vis->width, vis->height);
                    XSetFillStyle(A->display, localGC, FillSolid);
                    break;
                }
            vis = vis->next;
        }
    if (A->shade_left)
        {
            XSetForeground(A->display, localGC, A->shade_color);
            XSetFillStyle(A->display, localGC, A->shade_style);
            if (!((A->shade_style == FillSolid) || (A->shade_style == 0)))
                {
                    if (A->shade_style == FillTiled)
                        XSetTile(A->display, localGC, A->shade_pattern);
                    else
                        XSetStipple(A->display, localGC, A->shade_pattern);
                }
            XFillRectangle(A->display, A->map, localGC, 0, 0, A->shade_left,
                           A->height);
            XSetFillStyle(A->display, localGC, FillSolid);
        }
    if (A->shade_right)
        {
            XSetForeground(A->display, localGC, A->shade_color);
            XSetFillStyle(A->display, localGC, A->shade_style);
            if (!((A->shade_style == FillSolid) || (A->shade_style == 0)))
                {
                    if (A->shade_style == FillTiled)
                        XSetTile(A->display, localGC, A->shade_pattern);
                    else
                        XSetStipple(A->display, localGC, A->shade_pattern);
                }
            XFillRectangle(A->display, A->map, localGC,
                           (A->width - A->shade_right), 0, A->width,
                           A->height);
            XSetFillStyle(A->display, localGC, FillSolid);
        }
    defaultAMapUpdateCallback(A, NULL);
}


void defaultAMapUpdateCallback(A, E)
    AMap A;
    XEvent* E;
{
    int front, end;
    GC localGC;
    
    localGC = DefaultGC(A->display, DefaultScreen(A->display));
    XCopyArea(A->display, A->map, A->window, localGC, 0, 0, A->width,
              A->height, 0, 0);
    XSetForeground(A->display, localGC, A->pen_color);
    
    front = 0;
    end = 1;
    while (front != ((A->width * 3) - 1))
        {
            while (A->points[end].flag == 0)
                end++;
            XDrawLine(A->display, A->window, localGC, (front - A->left_edge),
                      (A->height - A->points[front].v - 1),
                      (end - A->left_edge),
                      (A->height - A->points[end].v - 1));
            front = end;
            end++;
        }
}


/*
 * Respond to an event properly
 */
int XfAMapAction(A, E)
    AMap A;
    XEvent* E;
{
    struct CallBackList *execs;
    int d_left, d_right;
    int x, y;
    XEvent EE;
    static int  lo_x, hi_x;        /* For moving and adding */
    static int  anchor, old_edge;  /* For sliding */
    static int  cur_x;
    static int  button_down;
    
    if (A == NULL)
        return(False);
    else
        execs = A->CallBacks;
    
    switch (E->type)
        {
        case Expose:
            (*(A->exposeCallback))(A, E);
            return(True);
            break;
        case ButtonPress:
            if (A->action_mode == XfAMapNoAction)
                return(False);
            while (XCheckMaskEvent(A->display, ButtonMotionMask, &EE))
                ;
            x = E->xbutton.x;
            y = E->xbutton.y;
            if (y >= A->height)
                y = A->height - 1;
            
            if (A->action_mode == XfAMapAdd)
                {
                    if (A->points[A->left_edge + x].flag == 1)
                        return(True);
                    else
                        cur_x = x;
                }
            else if ((A->action_mode == XfAMapMove) ||
                     (A->action_mode == XfAMapDel))
                {
                    for (d_left = x ; d_left > 0 ; d_left--)
                        {
                            if (A->points[A->left_edge + d_left].flag == 1)
                                break;
                        }
                    for (d_right = x ; d_right < A->width - 1 ; d_right++)
                        {
                            if (A->points[A->left_edge + d_right].flag == 1)
                                break;
                        }
                    if (x - d_left < d_right - x)
                        cur_x = d_left;
                    else
                        cur_x = d_right;
                }
            /*
             * Set end points if necessary 
             */
            if ((A->action_mode == XfAMapAdd) ||
                (A->action_mode == XfAMapMove) || 
                (A->action_mode == XfAMapDel))
                {
                    if ((cur_x != 0) && (cur_x != (A->width - 1)))
                        {
                            for (lo_x = cur_x - 1 ; lo_x > 0 ; lo_x--)
                                if (A->points[A->left_edge + lo_x].flag == 1)
                                    break;
                            if (lo_x == 0)
                                {
                                    XfinterpolateAMap(A);
                                    A->points[A->left_edge].flag = 1;
                                }
                            
                            for (hi_x = cur_x + 1 ; hi_x < A->width-1 ; hi_x++)
                                if (A->points[A->left_edge + hi_x].flag == 1)
                                    break;
                            if (hi_x == A->width - 1)
                                {
                                    XfinterpolateAMap(A);
                                    A->points[A->left_edge + hi_x].flag = 1;
                                }
                        }
                }
            else if (A->action_mode == XfAMapSlide)
                {
                    old_edge = A->left_edge;
                    anchor = x;
                    button_down = True;
                    break;
                }
            
            if (A->action_mode == XfAMapDel || A->action_mode == XfAMapMove)
                {
                    if ((cur_x != 0) && (cur_x != A->width - 1))
                        {
                            A->points[A->left_edge + cur_x].flag = 0;
                            cur_x = x;
                        }
                }
            
            if (A->action_mode != XfAMapDel)
                {
                    A->points[A->left_edge + cur_x].v = A->height - y - 1;
                    A->points[A->left_edge + cur_x].flag = 1;
                }
            (*(A->updateCallback))(A, E);
            button_down = True;
            break;
        case MotionNotify:
            if ((A->action_mode == XfAMapDel) || 
                (A->action_mode == XfAMapNoAction) || 
                (!button_down))
                return(False);
            
            while (XCheckMaskEvent(A->display, ButtonMotionMask, E))
                ;
			
			if (A->readoutCallback != NULL) {
				(*(A->readoutCallback))(A, E);
			}
            
            x = E->xbutton.x;
            y = E->xbutton.y;
            
            if (y < 0)
                y = 0;
            else if (y >= A->height)
                y = A->height - 1;
            
            if (A->action_mode == XfAMapAdd || A->action_mode == XfAMapMove)
                {
                    if (cur_x == 0 || cur_x == A->width - 1)
                        {
                            A->points[A->left_edge + cur_x].v =
                                A->height - y - 1;
                            (*(A->updateCallback))(A, E);
                        }
                    else
                        {
                            A->points[A->left_edge + cur_x].flag = 0;
                            if (x <= lo_x)
                                x = lo_x + 1;
                            else if (x >= hi_x)
                                x = hi_x - 1;
                            
                            A->points[A->left_edge + x].v = A->height - y - 1;
                            A->points[A->left_edge + x].flag = 1;
                            XfinterpolateAMap(A);
                            (*(A->updateCallback))(A, E);
                            cur_x = x;
                        }
                }
            else if (A->action_mode == XfAMapSlide)
                {
                    d_left = old_edge - (x - anchor);
                    if (d_left < 0)
                        d_left = 0;
                    else if (d_left > A->max_left_edge)
                        d_left = A->max_left_edge;
                    A->left_edge = d_left;
                    (*(A->updateCallback))(A, E);
                }
            break;
        case ButtonRelease:
            if ((A->action_mode == XfAMapDel) || 
                (A->action_mode == XfAMapNoAction) || 
                (!button_down))
                return(False);
            
            x = E->xbutton.x;
            y = E->xbutton.y;
            
            if (y < 0)
                y = 0;
            else if (y >= A->height)
                y = A->height - 1;
            
            if (A->action_mode == XfAMapAdd || A->action_mode == XfAMapMove)
                {
                    if (cur_x == 0 || cur_x == A->width - 1)
                        {
                            A->points[A->left_edge + cur_x].v =
                                A->height - y - 1;
                            (*(A->updateCallback))(A, E);
                        }
                    else
                        {
                            A->points[A->left_edge + cur_x].flag = 0;
                            if (x <= lo_x)
                                x = lo_x + 1;
                            else if (x >= hi_x)
                                x = hi_x - 1;
                            
                            A->points[A->left_edge + x].v = A->height - y - 1;
                            A->points[A->left_edge + x].flag = 1;
                            XfinterpolateAMap(A);
                            (*(A->updateCallback))(A, E);
                            cur_x = x;
                            
                            lo_x = -1;
                            hi_x = A->width + 1;
                        }
                }
            else if (A->action_mode == XfAMapSlide)
                {
                    d_left = old_edge - (x - anchor);
                    if (d_left < 0)
                        d_left = 0;
                    else if (d_left > A->max_left_edge)
                        d_left = A->max_left_edge;
                    A->left_edge = d_left;
                    (*(A->updateCallback))(A, E);
                    old_edge = 0;
                    anchor = 0;
                }
            button_down = False;
            break;
        default:
            break;
        }
    
    while (execs != NULL)
        {
            if (execs->proc != NULL)
                (*(execs->proc))(A, E);
            execs = execs->next;
        }
    return(True);
}


/*
 * Set the action mode for the passed AMap
 */
int XfSetAMapAction(A, mode)
    AMap A;
    int mode;
{
    if (A == NULL)
        return(False);
    else
        {
            A->action_mode = mode;
            return(True);
        }
}


/*
 * Set an AMap to it's defaults
 */
int XfClearAMap(A)
    AMap A;
{
    int i;
    
    if (A == NULL)
        return(False);
    for (i = 0; i < (A->width * 3); i++)
        A->points[i].flag = 0;
    A->points[0].flag = 1;
    A->points[0].v = 0;
    A->points[A->width].flag = 1;
    A->points[A->width].v = 0;
    A->points[A->width * 2 - 1].flag = 1;
    A->points[A->width * 2 - 1].v = A->height - 1;
    A->points[A->width * 3 - 1].flag = 1;
    A->points[A->width * 3 - 1].v = A->height - 1;
    A->left_edge = A->width;
    XfinterpolateAMap(A);
    return(True);
}


/*
 * Center the AMap on the data in the window
 */
int XfCenterAMap(A)
    AMap A;
{
    int i;
    struct Point* data;
    
    if (A == NULL)
        return(False);
    
    if (A->left_edge == A->width)
        return(True);
    
    data = (struct Point *)calloc(A->width, sizeof(struct Point));
    if (data == NULL)
        return(False);
    for (i = 0; i < A->width; i++)
        data[i].v = A->points[A->left_edge + i].v,
        data[i].flag = A->points[A->left_edge + i].flag;
    A->left_edge = A->width;
    XfClearAMap(A);
    for (i = 0; i < A->width; i++)
        A->points[A->left_edge + i].v = data[i].v,
        A->points[A->left_edge + i].flag = data[i].flag;
    A->points[A->left_edge].flag = 1;
    A->points[A->left_edge + A->width - 1].flag = 1;
    free(data);
    
    return(True);
}

/*
 * Take a "photograph" of the passed AMap
 */
struct Point* XfPhotographAMap(A)
    AMap A;
{
    int i;
    struct Point* new;
    
    if (A == NULL)
	return(NULL);
    
    new = (struct Point *)calloc(A->width, sizeof(struct Point));
    if (new == NULL)
	return(NULL);

    for (i = 0; i < A->width; i++)
	{
	    new[i].v = A->points[A->left_edge + i].v;
	    new[i].flag = A->points[A->left_edge + i].flag;
	}
	new[0].flag = 1;
	new[A->width-1].flag = 1;
    return(new);
}

/*
 * Restore an AMap from a "photograph."
 */
int XfRestoreAMap(A, data)
    AMap A;
    struct Point* data;
{
    int i;
    
    if ((A == NULL) || (data == NULL))
	return(False);
    
    XfClearAMap(A);
    
    for (i = 0; i < A->width; i++)
	{
	    A->points[A->left_edge + i].v = data[i].v;
	    A->points[A->left_edge + i].flag = data[i].flag;
	}
	(*(A->updateCallback))(A,NULL);
    
    return(True);
}

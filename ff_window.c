#include "parser.h"
#include "window.h"

/*
** Rolling window functions
** 
** 
** Window * create_window(int width, int height, int format);
** 
**  Create a window structure of size width x height.  Format is ignore
**  currently.  Window is always of type FLOAT.  (but pass FLOAT anyway).
** 
** void load_window(Window *w, Var *obj, int x1, int y1, float ignore);
** 
**  Load a window structure with pixels from <obj> centered around 
**  the point <x1,y1>.  Values that fall off the edge of the window 
**  are assigned the <ignore> value.
** 
** void roll_window(Window *w, Var *obj, int x1, int y1, float ignore);
** 
**  Shift the rows in the Window <w> up by one, and fill in the bottom row.
**  This function doesn't do a lot of error checking, so if <x1,y1> isn't
**  exactly equal to <x1,y1-1>, bad things will happen.
** 
** void dump_window(Window *w);
** 
**  Print the contents of a Window 
** 
** void free_window(Window *w);
** 
**  Destroy a Window.
** 
** void load_row(Window *w, Var *obj, int x1, int y1, int row, float ignore);
** 
**  Internal function to load one row of data.
** 
*/


Window *
create_window(int width, int height, int format)
{
    int i;
    int nbytes = NBYTES(format);
    Window *w = calloc(1, sizeof(Window));

    w->width = width;
    w->height = height;
    w->data = calloc(width*height, nbytes);
    w->row = calloc(height, sizeof(void *));
    for (i = 0 ; i < height ; i++) {
        w->row[i] = ((char *)w->data)+ i*width*nbytes;
    }
    return(w);
}

void
load_row(Window *w, Var *obj, int x1, int y1, int row, float ignore)
{
    int i;
    int x = GetX(obj);
    int y = GetY(obj);
    int p,q;

    q = y1+row-w->height/2;
    for (i = 0 ; i < w->width ; i++) {
        p = x1+i-w->width/2;
        if (p < 0 || p >= x || q < 0 || q >= y) {
            ((float *)(w->row[row]))[i] = ignore;
        } else {
            ((float *)(w->row[row]))[i] = extract_float(obj, cpos(p, q, 0, obj));
        }
    }
}

void
dump_window(Window *w)
{
    int i;
    int j;
    for (j = 0 ; j < w->height ; j++) {
        for (i = 0 ; i < w->width ; i++) {
            printf("%f ", ((float *)(w->row[j]))[i]);
        }
        printf("\n");
    }
}

void
load_window(Window *w, Var *obj, int x1, int y1, float ignore) 
{
    /* 
    ** Fill the window array from the current x,y position.
    */
    int row;

    for (row = 0 ; row < w->height ; row++) {
        load_row(w, obj, x1, y1, row, ignore);
    }
}

void
roll_window(Window *w, Var *obj, int x1, int y1, float ignore)
{
    /*
    ** shift the Window pointers to drop a row and add a new one.
    */
    int i;
    void *t = w->row[0];
    for (i = 0 ; i < w->height-1 ; i+=1 ) {
        w->row[i] = w->row[i+1];
    }
    w->row[w->height-1] = t;
    load_row(w, obj, x1, y1, w->height-1, ignore);
}

void
free_window(Window *w)
{
    free(w->row);
    free(w->data);
    free(w);
}

/*
** non-linear window filters
**
** The idea is to extract a 2-D array from the source centered around each 
** point in the source, and pass it to any of several compute functions.
**
** Properly implemented, you can resuse N-1 rows of the previous window
** when you move it, if the compute function doesn't modify its input 
** (or if you don't care if it does)
**
** Did I mention that these kinds of algorithms really suck if there's
** deleted points involved?
**
** Some additional notes:
**     Currently, everything is promoted to floats.
**     Currently, only band 1 is used.
*/

Var *
ff_window(vfuncptr func, Var * arg)
{
    Var *obj = NULL, *rval = NULL;
    float ignore=MINFLOAT;
    float *out;
    int dim[3];
    int format, nbytes;
    float *s1, *s2;
    int a,b,x,y,z,pos, i,j,k;
    int size = 0, width = 0, height = 0;
    char *type = NULL;
    /*
    ** algorithms that could fit into this framework.
    */
    char *types[] = { "median", 
                      "gauss", "guassian", 
                      "box", "lpf", "low",
                      "edge", "sobel",
                      "radial_symmetry",
                      NULL
                    };
    Window *w;

    Alist alist[9];
    alist[0] = make_alist( "object",    ID_VAL,    NULL,   &obj);
    alist[1] = make_alist( "type",    ID_ENUM,   types,   &type);
    alist[2] = make_alist( "x",     INT,    NULL,      &width);
    alist[3] = make_alist( "y",    INT,    NULL,      &height);
    alist[4] = make_alist( "size",    INT,    NULL,      &size);
    alist[5] = make_alist( "ignore",    FLOAT,    NULL,    &ignore);
    alist[6].name = NULL;

    if (parse_args(func, arg, alist) == 0) return(NULL);

    if (obj == NULL) {
        parse_error("%s: No object specified\n", func->name);
        return(NULL);
    }
    if (type == NULL) {
        parse_error("%s: No type specified\n", func->name);
        return(NULL);
    }

    if (size > 0) {
        width = height = size;
    }
    if (width ==0 || height ==0) {
        parse_error("%s: Invalid size specified (%dx%d\n", func->name, width, height);
        return(NULL);
    }

    x = GetX(obj);
    y = GetY(obj);
    z = GetZ(obj);
    w = create_window(width, height, FLOAT);

    if (!strcmp(type, "median")) {
        /*
        ** Median reorders the data (sorts), so you can't roll the window,
        ** you have to reload it each time.
        */
        out = calloc(x*y, sizeof(float));
        rval = newVal(BSQ, x, y, 1, FLOAT, out);
        for (j = 0 ; j < y ; j+=1) {
            for (i = 0 ; i < x ; i+=1) {
                load_window(w, obj, i, j, ignore);
                out[cpos(i, j, 0, rval)] = median_window(w->data, 
                                                    width, height, ignore);
            }
        }
    }

    free_window(w);
    return(rval);
}

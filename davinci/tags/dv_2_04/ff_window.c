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
float median_window(float *s1, int width, int height, float ignore);

static int cmp(const void *a, const void *b) {
  return(*(float *)a - *(float *)b); 
}

float median_window(float *s1, int width, int height, float ignore)
{
    int i;
    int c1 = 0, c2 = 0;

    qsort(s1, width*height, sizeof(float), cmp);

    /* count number of ignore values */
    for (i = 0 ; i < width*height ; i++) {
        if (s1[i] != ignore) c1++;
    }
    /* Count N/2 smalles values */
    for (i = 0 ; i < width*height ; i++) {
        if (s1[i] != ignore) {
            if (c2++ >= c1/2) {
                return(s1[i]);
            }
        }
    }
    return(ignore);
}

Window *
create_window(int width, int height, int format)
{
    int i;
    int nbytes = NBYTES(format);
    Window *w = calloc(1, sizeof(Window));

    w->width = width;
    w->height = height;

	/* changes to handle row[-1]  for histogram window */
	w->handle = calloc(width*(height+1), nbytes); 
    w->rows = calloc(height+1, sizeof(void *));

    w->data = ((char *)w->handle) + width*nbytes;
    w->row = &(w->rows[1]);
    for (i = -1 ; i < height ; i++) {
        w->row[i] = ((char *)w->handle)+ (i+1)*width*nbytes;
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
    void *t = w->row[-1];
    for (i = -1 ; i < w->height-1 ; i+=1 ) {
        w->row[i] = w->row[i+1];
    }
    w->row[w->height-1] = t;
    load_row(w, obj, x1, y1, w->height-1, ignore);
}

void
free_window(Window *w)
{
    free(w->rows);
    free(w->handle);
    free(w);
}


/*
** Rolling window histogram operations
*/


Histogram *
create_histogram()
{
	Histogram *h = calloc(1, sizeof(Histogram));
	h->hist = calloc(65536, sizeof(short));
	h->hist = &(h->hist[32767]);
	return(h);
}

Histogram *
load_histogram(Histogram *h, Window *w, int ignore) 
{
	int i, j;
	int count;
	short *hist;
	int width = w->width;
	int height = w->height;
	int min, max, x;
	float **row = ((float **)w->row);

	if (h == NULL) {
		h = create_histogram();
	} else {
		memset(&(h->hist[-32767]), 0, 65536*2);
	}

	hist = h->hist;
	max = min = ignore;
	count = 0;

	for (j = 0 ; j < height;  j++) {
		for (i = 0 ; i < width; i++) {
			x = row[j][i];
			if (x != ignore) {
				hist[x]++;
				if (count == 0 || x > max) max = x;
				if (count == 0 || x < min) min = x;
				count++;
			}
		}
	}
	h->min = min;
	h->max = max;
	h->count = count;
	h->hist = hist;
	return(h);
}

void
roll_histogram(Histogram *h, Window *w, int ignore)
{
	/* take out the entires in row[-1]; */
	int i, j;
	int x;
	float **row = ((float **)w->row);
	int width = w->width;
	int height = w->height;

	for (i = 0 ; i < width ; i++) {
		if ((x = row[-1][i]) != ignore) {
			h->hist[x]--;
			h->count--;
			if (h->hist[x] == 0 && x <= h->min) {
				/* find new min */
				for (j = x+1 ; j <= h->max ; j++) {
					if (h->hist[j] > 0) {
						h->min = j;
						break;
					}
				}
			}
			if (h->hist[x] == 0 && x >= h->max ) {
				/* find new max */
				for (j = x-1 ; j >= h->min ; j--) {
					if (h->hist[j] > 0) {
						h->max = j;
						break;
					}
				}
			}
		}
	}

	/* add entries from row[height] */
	for (i = 0 ; i < width ; i++) {
		if ((x = row[height-1][i]) != ignore) {
			h->hist[x]++;
			if (h->count == 0 || x < h->min) h->min = x; 
			if (h->count == 0 || x > h->max) h->max = x;
			h->count++;
		}
	}
}

short
histogram_min(Histogram *h, int ignore)
{
	if (h->count == 0) return(ignore);
	return(h->min);
}

short
histogram_max(Histogram *h, int ignore)
{
	if (h->count == 0) return(ignore);
	return(h->max);
}

void free_histogram(Histogram *h) 
{
	free(h->arena);
	free(h);
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
    float ignore=MAXFLOAT;
    float *f_out;
	short *s_out;
    int x,y,z, i,j;
    int size = 0, width = 0, height = 0;
    char *type = NULL;
	Var *mask;
    /*
    ** algorithms that could fit into this framework.
    */
    const char *types[] = { "min", "max", "median", NULL };

/*
                      "gauss", "guassian", 
                      "box", "lpf", "low",
                      "edge", "sobel",
                      "radial_symmetry",
                      "avg","stddev",
                      NULL
                    };
*/
    Window *w;

    Alist alist[9];
    alist[0] = make_alist( "object",  ID_VAL,   NULL,  &obj);
    alist[1] = make_alist( "type",    ID_ENUM,  types, &type);
    alist[2] = make_alist( "x",       INT,      NULL,  &width);
    alist[3] = make_alist( "y",       INT,      NULL,  &height);
    alist[4] = make_alist( "size",    INT,      NULL,  &size);
    alist[5] = make_alist( "ignore",  FLOAT,    NULL,  &ignore);
    alist[6] = make_alist( "mask",    ID_VAL,   NULL,  &mask);
    alist[7].name = NULL;

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
        parse_error("%s: Invalid size specified (%dx%d)\n", func->name, width, height);
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
        f_out = calloc((size_t)x*(size_t)y, sizeof(float));
        rval = newVal(BSQ, x, y, 1, FLOAT, f_out);
        for (j = 0 ; j < y ; j+=1) {
            for (i = 0 ; i < x ; i+=1) {
                load_window(w, obj, i, j, ignore);
                f_out[cpos(i, j, 0, rval)] = median_window(w->data, 
                                                    width, height, ignore);
            }
        }
	} else if (!strcmp(type, "min") || !strcmp(type, "max")) {
		/* histogram operators */
		Histogram *h = NULL;
		short (*hf)(Histogram *, int);

		if (!strcmp(type, "min")) {
			hf = histogram_min;
		} else if (!strcmp(type, "max")) {
			hf = histogram_max;
		}
		s_out = calloc((size_t)x*(size_t)y, sizeof(short));
		rval = newVal(BSQ, x, y, 1, SHORT, s_out);
		for (i = 0 ; i < x ; i+=1) {
			load_window(w, obj, i, 0, ignore);
			h = load_histogram(h, w, ignore);
			for (j = 0 ; j < y ; j+=1) {
				if (j) {
					roll_window(w, obj, i, j, ignore);
					roll_histogram(h, w, ignore);
				}
				s_out[cpos(i, j, 0, rval)] = hf(h, ignore);
			}
		}
		free_histogram(h);
	}

    free_window(w);
    return(rval);
}

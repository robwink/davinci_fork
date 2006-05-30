#include "parser.h"
#include "func.h"
#include "window.h"


float local_maximum(Window *w, float threshold, float ignore);

Var *
ff_local_maximum(vfuncptr func, Var * arg)
{
    Var *obj = NULL, *rval = NULL;
    float ignore=MINFLOAT;
    float *out;
    int dim[3];
    int format, nbytes;
    float *s1, *s2;
    int a,b,x,y,z,pos, i,j,k;
    int size = 3;
    int xdelta=0, ydelta=0;
    int width=0, height=0;
    float *line;
    int all = 0;
	float threshold = MINFLOAT;
    Window *w;

    Alist alist[9];
    alist[0] = make_alist( "object",    ID_VAL, NULL,   &obj);
    alist[1] = make_alist( "size",      INT,    NULL,   &size);
    alist[2] = make_alist( "ignore",    FLOAT,  NULL,    &ignore);
    alist[3] = make_alist( "threshold",    FLOAT,  NULL,    &threshold);
    alist[4].name = NULL;

    if (parse_args(func, arg, alist) == 0) return(NULL);

    if (obj == NULL) {
        parse_error("%s: No object specified\n", func->name);
        return(NULL);
    }

    x = GetX(obj);
    y = GetY(obj);
    z = GetZ(obj);
    w = create_window(size, size, FLOAT);

	out = calloc(x*y, sizeof(float));
	rval = newVal(BSQ, x, y, 1, FLOAT, out);

    for (i = 0 ; i < x ; i+=1) {
        load_window(w, obj, i, 0, ignore);
        for (j = 0 ; j < y ; j+=1) {
            if (j) roll_window(w, obj, i, j, ignore);

            out[cpos(i,j,0,rval)] = local_maximum(w, threshold, ignore);
        }
    }

    free_window(w);
    return(rval);
}

float 
local_maximum(Window *w, float threshold, float ignore) 
{
	float v;
	int i, j;
	int maxi,maxj;
	float max;

	v = ((float *)w->row[w->height/2])[w->width/2];
	maxj = w->height/2;
	maxi = w->width/2;
	if (v == ignore) return(ignore);

	for (i = 0 ; i < w->width ; i+=1) {
		for (j = 0 ; j < w->height ; j+=1) {
			v = ((float *)w->row[j])[i];
			if (v == ignore || v < threshold) continue;

			if (i == 0 && j == 0 || v > max) {
				maxi = i;
				maxj = j;
				max = v;
			}
		}
	}
	if (maxi == w->width/2 && maxj == w->height /2) return(max);
	return(ignore);
}

/*
** Given a 2-D array of data, compute it's median
** this algorithm currently just qsorts the entire NxN array and
** then counts up to the N/2th element, 
** N has to get modified by the number of ignore values in the array
**
** This was the fastest way to get it to work, but it'll be slow.
** Michael's merge sort idea is a better way, but it's tightly coupled 
** with window generation.  It will eventually be: 
**
**** Sort each row, then merge-find (merge-sort without move) across 
**** rows.  You can leave the N-1 rows sorted and just replace 
**** the last one, reducing the problem down to sorting a single N-element 
**** array and then merge sorting across arrays to find the smallest N/2 
**** elements.
**
**** One thing I learned when implementing it the cheezy way, you can't
**** just find the N/2 smallest values if there are ignore values involved.
**** If you're certain <ignore> is the smallest value, you can count them
**** first and then find the (N-nignore)/2 smallest values.
*/

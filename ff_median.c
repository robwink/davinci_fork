#include "parser.h"
#include "func.h"
#include "window.h"


float radial_symmetry(void **data, int width, int height, float ignore,
    int delta_x, int delta_y, int radius, float *out);
float radial_symmetry2(void **data_in, int width, int height, float ignore);

/*
** Extract a 2-D array of values from the input.
** 
** This currently re-extracts the entire array each time, since 
** this process is tightly-coupled with traversal order.
*/

void
load_window_f(float *s1, int size, int a, int b, Var *obj, float ignore)
{
    /* load a convolution window around point a,b of dimention "size" 
    */
    int i,j;
    int x = GetX(obj);
    int y = GetY(obj);
    int p,q;

    for (i = 0 ; i < size ; i++) {
        for (j = 0 ; j < size ; j++) {
            p = a+i-size/2;
            q = b+j-size/2;
            if (p < 0 || p >= x || q < 0 || q >= y) {
                s1[i+j*size] = ignore;
            } else {
                s1[i+j*size] = extract_float(obj, cpos(p, q, 0, obj));
            }
        }
    }
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
static int cmp(float *a, float *b) { return((*a)-(*b)); }

float median_window(float *s1, int width, int height, float ignore)
{
    int sum = 0;
    int i, j;
    int c1 = 0, c2 = 0;
    float r;

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

/*
** This function is being obsolted by ff_window
**/

Var *
ff_median(vfuncptr func, Var * arg)
{
    Var *obj = NULL;
    int size = 3;
    float ignore=MINFLOAT;
    float *out;
    int dim[3];
    int format, nbytes;
    float *s1, *s2;
    int a,b,x,y,pos;

    Alist alist[9];
    alist[0] = make_alist( "object",    ID_VAL,    NULL,    &obj);
    alist[1] = make_alist( "size",      INT,       NULL,    &size);
    alist[2] = make_alist( "ignore",    FLOAT,     NULL,    &ignore);
    alist[3].name = NULL;

    if (parse_args(func, arg, alist) == 0) return(NULL);

    if (obj == NULL) {
        parse_error("%s: No object specified\n", func->name);
        return(NULL);
    }
    if (size <= 1)  {
        parse_error("%s: Invalid filter size: %d\n", func->name, size);
        return(NULL);
    }

    x = GetX(obj);
    y = GetY(obj);
    format = V_FORMAT(obj);
    nbytes = NBYTES(format);

    /* 
     * The idea here is to build a window to pass to a generic 
     * algorithm, cut from one side, and add on the other.
     * We keep two copies, a copy of the original, and something the
     * work routine can chew up.
     */ 

    s1 = calloc(size * size, sizeof(float));        /* stage 1 */
    out = calloc(x*y, sizeof(float));

    for (a = 0 ; a < x ; a++) {
        for (b = 0 ; b < y ; b++) {
            /* 
             * for each point, build the window
             */
            load_window_f(s1, size, a, b, obj, ignore);

            pos = cpos(a,b,0,obj);
            out[pos] = median_window(s1, size, size, ignore);
        }
    }
    return(newVal(BSQ, x, y, 1, FLOAT, out));
}


Var *
ff_radial_symmetry(vfuncptr func, Var * arg)
{
    Var *obj = NULL, *rval = NULL;
    float ignore=MINFLOAT;
    float *out;
    int dim[3];
    int format, nbytes;
    float *s1, *s2;
    int a,b,x,y,z,pos, i,j,k;
    int radius = 10;
    int xdelta=0.0, ydelta=0.0;
    float *line;
    int all = 0;
    Window *w;

    Alist alist[9];
    alist[0] = make_alist( "object",    ID_VAL, NULL,   &obj);
    alist[1] = make_alist( "radius",    INT,    NULL,      &radius);
    alist[2] = make_alist( "xdelta",    INT,    NULL,      &xdelta);
    alist[3] = make_alist( "ydelta",    INT,    NULL,      &ydelta);
    alist[4] = make_alist( "ignore",    FLOAT,  NULL,    &ignore);
    alist[5] = make_alist( "all",       INT,    NULL,    &all);
    alist[6].name = NULL;

    if (parse_args(func, arg, alist) == 0) return(NULL);

    if (obj == NULL) {
        parse_error("%s: No object specified\n", func->name);
        return(NULL);
    }

    if (radius < 0) {
        parse_error("%s: Invalid radius specified\n", func->name);
        return(NULL);
    }

    x = GetX(obj);
    y = GetY(obj);
    z = GetZ(obj);
    w = create_window(radius, radius, FLOAT);

    if (all) {
        out = calloc(x*y*radius, sizeof(float));
        rval = newVal(BSQ, x, y, radius, FLOAT, out);
        line = calloc(radius, sizeof(float));
    } else {
        out = calloc(x*y, sizeof(float));
        rval = newVal(BSQ, x, y, 1, FLOAT, out);
        line = NULL;
    }

    for (i = 0 ; i < x ; i+=1) {
        load_window(w, obj, i, 0, ignore);
        for (j = 0 ; j < y ; j+=1) {
            if (j) roll_window(w, obj, i, j, ignore);

            out[cpos(i,j,0,rval)] = radial_symmetry(w->row, 
                                        radius, radius, ignore, 
                                        xdelta, ydelta, radius, line);
            if (all) {
                for (k = 0 ; k < radius ; k++) {
                    out[cpos(i, j, k, rval)] = line[k];
                }
            }
        }
    }

    free_window(w);
    return(rval);
}

Var *
ff_radial_symmetry2(vfuncptr func, Var * arg)
{
    Var *obj = NULL, *rval = NULL;
    float ignore=MINFLOAT;
    float *out;
    int dim[3];
    int format, nbytes;
    float *s1, *s2;
    int a,b,x,y,z,pos, i,j,k;
    int size = 0;
    int xdelta=0, ydelta=0;
    int width=0, height=0;
    float *line;
    int all = 0;
    Window *w;

    Alist alist[9];
    alist[0] = make_alist( "object",    ID_VAL, NULL,   &obj);
    alist[1] = make_alist( "x",    INT,    NULL,      &width);
    alist[2] = make_alist( "y",    INT,    NULL,      &height);
    alist[3] = make_alist( "size",    INT,    NULL,      &size);
    alist[4] = make_alist( "ignore",    FLOAT,  NULL,    &ignore);
    alist[5].name = NULL;

    if (parse_args(func, arg, alist) == 0) return(NULL);

    if (obj == NULL) {
        parse_error("%s: No object specified\n", func->name);
        return(NULL);
    }

	if (size) {
		width = size;
		height = size;
	}

    if (width <= 0 || height <= 0) {
        parse_error("%s: Invalid size specified (%dx%d)\n", func->name, width, height);
        return(NULL);
    }

    x = GetX(obj);
    y = GetY(obj);
    z = GetZ(obj);
    w = create_window(width, height, FLOAT);

	out = calloc(x*y, sizeof(float));
	rval = newVal(BSQ, x, y, 1, FLOAT, out);

    for (i = 0 ; i < x ; i+=1) {
        load_window(w, obj, i, 0, ignore);
        for (j = 0 ; j < y ; j+=1) {
            if (j) roll_window(w, obj, i, j, ignore);

            out[cpos(i,j,0,rval)] = radial_symmetry2(w->row, 
                                        width, height, ignore);
        }
    }

    free_window(w);
    return(rval);
}

float
radial_symmetry(void **data_in, 
                int width, int height,      /* dimensions of window */
                float ignore,
                int delta_x, int delta_y,   /* direction step size */
                int size,                   /* distance to compute */
                float *out)
{
    float **data = (float**)data_in;
    int xc = width/2;
    int yc = height/2;
    int i,j;
    int odd;
    double x1, y1;

    double ret = ignore;
    double ssxx, ssyy, ssxy;
    double sumx[2]={0,0},sumy[2]={0,0},sumxx[2]={0,0},sumyy[2]={0,0},sumxy[2]={0,0};
    double n[2] = {0,0};

    if (out != NULL) memset(out, size*sizeof(float), 0);

    for (i = 2 ; i <= size ; i++) {
        odd = i%2;
        /*
        ** End up adding a pair of values each time,
        ** but must track odds vs evens separately.
        */
        if (out) out[i-1] = ignore;
        if (odd) {
            y1 = data[yc-delta_y*(i/2)][xc-delta_x*(i/2)];
            x1 = data[yc+delta_y*(i/2)][xc+delta_x*(i/2)];
        } else {
            x1 = data[yc+delta_y*((i-1)/2)][xc+delta_x*((i-1)/2)];
            y1 = data[yc-delta_y*(i/2)][xc-delta_x*(i/2)];
        }
        
        if (x1 == ignore || y1 == ignore) {
            return (ignore);
        }

        sumx[odd] += x1;
        sumy[odd] += y1;
        sumxx[odd] += x1*x1;
        sumyy[odd] += y1*y1;
        sumxy[odd] += x1*y1;
        n[odd] += 1;

        if (out) {
            if (n[odd] > 2) {
                out[i-1] = 0;
                ssxx = sumxx[odd] - sumx[odd]*sumx[odd]/n[odd];
                ssyy = sumyy[odd] - sumy[odd]*sumy[odd]/n[odd];
                ssxy = sumxy[odd] - sumx[odd]*sumy[odd]/n[odd];
                if ((ssxx*ssyy) != 0) {
                    out[i-1] = sqrt(ssxy*ssxy / (ssxx*ssyy));
                }
            }
        }
    }
    if (n[odd] > 2) {
        ssxx = sumxx[odd] - sumx[odd]*sumx[odd]/n[odd];
        ssyy = sumyy[odd] - sumy[odd]*sumy[odd]/n[odd];
        ssxy = sumxy[odd] - sumx[odd]*sumy[odd]/n[odd];
        if ((ssxx*ssyy) != 0) {
            ret = sqrt(ssxy*ssxy / (ssxx*ssyy));
        }
    }
    return(ret);
}

float
radial_symmetry2(void **data_in, 
                int width, int height,      /* dimensions of window */
                float ignore)
{
    float **data = (float**)data_in;
    int i,j;
    int odd;
    double x1, y1;
    int xc = width/2;
    int yc = height/2;

    double ret = ignore;
    double ssxx, ssyy, ssxy;
    double sumx=0,sumy=0,sumxx=0,sumyy=0,sumxy=0;
    double n = 0;

	for (i = 0 ; i < width ; i+=1) {
		for (j = 0 ; j < height ; j+=1) {
			/* skip pixels outside the radius */
			if ((xc-i)*(xc-i) + (yc-j)*(yc-j) > width*height)
				continue;

			/* skip correlating the pixel with itself */
			if (i == (width-1)-i && j == (height-1)-j) {
				continue;
			}
			x1 = data[j][i];
			y1 = data[(height-1)-j][(width-1)-i];
        
			if (x1 == ignore || y1 == ignore) {
				return(ret);
			}

			sumx += x1;
			sumy += y1;
			sumxx += x1*x1;
			sumyy += y1*y1;
			sumxy += x1*y1;
			n += 1;
		}
	}

	if (n > 2) {
		ssxx = sumxx - sumx*sumx/n;
		ssyy = sumyy - sumy*sumy/n;
		ssxy = sumxy - sumx*sumy/n;
		if ((ssxx*ssyy) != 0) {
			ret = sqrt(ssxy*ssxy / (ssxx*ssyy));
		}
	}
	return(ret);
}
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

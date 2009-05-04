#include "parser.h"
#include "func.h"
#include "window.h"


float radial_symmetry(void **data, int width, int height, float ignore,
    int delta_x, int delta_y, int size, float *out);
float radial_symmetry2(void **data_in, int width, int height, float ignore);

/*
** Extract a 2-D array of values from the input.
** 
** This currently re-extracts the entire array each time, since 
** this process is tightly-coupled with traversal order.
*/


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
    int size = 10;
    int xdelta=0.0, ydelta=0.0;
    float *line;
    int all = 0;
	int first = 1;
    Window *w;

    Alist alist[9];
    alist[0] = make_alist( "object",    ID_VAL, NULL,   &obj);
    alist[1] = make_alist( "size",    INT,    NULL,      &size);
    alist[2] = make_alist( "xdelta",    INT,    NULL,      &xdelta);
    alist[3] = make_alist( "ydelta",    INT,    NULL,      &ydelta);
    alist[4] = make_alist( "ignore",    FLOAT,  NULL,    &ignore);
    alist[5] = make_alist( "all",       INT,    NULL,    &all);
    alist[6] = make_alist( "first",    INT,    NULL,      &first);
    alist[7].name = NULL;

    if (parse_args(func, arg, alist) == 0) return(NULL);

    if (obj == NULL) {
        parse_error("%s: No object specified\n", func->name);
        return(NULL);
    }

    if (size < 0) {
        parse_error("%s: Invalid size specified\n", func->name);
        return(NULL);
    }

    x = GetX(obj);
    y = GetY(obj);
    z = GetZ(obj);
    w = create_window(size, size, FLOAT);

    if (all) {
        out = calloc(x*y*(size-first+1), sizeof(float));
        rval = newVal(BSQ, x, y, (size-first+1), FLOAT, out);
        line = calloc(size, sizeof(float));
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
                                        size, size, ignore, 
                                        xdelta, ydelta, size, line);
            if (all) {
                for (k = first ; k <= size ; k++) {
                    out[cpos(i, j, k-first, rval)] = line[k];
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


/*
** This takes a rectangular window and extracts points around the center
*/
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

/* 888888 */
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
    double ssxx = 0 , ssyy = 0, ssxy = 0;
    double sumx=0,sumy=0,sumxx=0,sumyy=0,sumxy=0;
    double n = 0;

	for (j = 0 ; j <= height/2 ; j+=1) {
		for (i = 0 ; i < width ; i+=1) {
			/* skip pixels outside the radius */
			/* this doens't look right */
			/*
			if ((xc-i)*(xc-i) + (yc-j)*(yc-j) > xc*yc)
				continue;
			*/

			/* skip correlating the pixel with itself */
			if (i >= width/2 && j >= height/2) break;

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

/* 
	Try computing all the R values for radius N to M in one pass.
	This is different than radial_symmetry2, which uses a diameter instead
	    of a radii.  This will have some effect on even numbers.
*/

/*
** General algorithm

**		Extract a box as large as the largest radii
**      For each value in that box, compute its distance from the origin
**      Add that value (and its opposite) to bins for radii ceil(d)
**      To get R for radii N, bin[N] = bin[N] +  bin[N-1]
**
**      Odd vs even must be handled separately
**
**      Note: several short-cuts in this code skip output for
**      "null" values.  That means they're zero (from calloc).  
**      Otherwise we'll have to pre-init the whole output array.
**
**      What's necessary to do chirping at this level?
**          Bilinear interpolation?
**
**          If we add a binsize to this algorithm, then can we just
**          offset everyone's distance to simulate moving the origin?
** 
*/


Var *
ff_radial_symmetry3(vfuncptr func, Var * arg)
{
    Var *obj = NULL;
    float ignore=MINFLOAT;
    int width=0, height=0;
	int end=1;
	Var *rval = NULL;
	int x,y,z;
    float *out;
    Window *w;

	int *distance, d;
	int dx, dy;
	double ssxx, ssyy, ssxy;
	int i, j, k, p, q, r;
	int p1,q1, h2,w2;
	float v1, v2;
	int total;
	float *r1, *r2;
	int start = 0, step = 1;

	struct dstore {
		double sumx;
		double sumy;
		double sumxx;
		double sumyy;
		double sumxy;
		int count;
	} *accum, *a1, *a2;

    Alist alist[6];
    alist[0] = make_alist( "object",    ID_VAL, NULL,   &obj);
    alist[1] = make_alist( "size",      INT,    NULL,   &end);
    alist[2] = make_alist( "ignore",    FLOAT,  NULL,   &ignore);
    alist[3] = make_alist( "start",    INT,  NULL,   &start);
    alist[4] = make_alist( "step",    INT,  NULL,   &step);
    alist[5].name = NULL;

    if (parse_args(func, arg, alist) == 0) return(NULL);

    if (obj == NULL) {
        parse_error("%s: No object specified\n", func->name);
        return(NULL);
    }

	if (end <= 0) {
		parse_error("%s: Invalid end value specified\n", func->name);
		return(NULL);
	}

    x = GetX(obj);
    y = GetY(obj);

	width = end*2+1;
	height = end*2+1;
	// width = end;
	// height = end;
    w = create_window(width, height, FLOAT);

	/* cache some frequently used computed values */
	h2 = height/2;
	w2 = width/2;
	total = width*height;

	/* precompute the distance of every point in a given sized window 
	** to the center of the window.
	** 
	** In theory, this only needs to be 1 quadrant of the window, but that's
	** too hard to bookkeep, and doesn't save much.
	*/
	distance = calloc(total, sizeof(int));
	for (i = 0 ; i < width ; i++) {
		dx = i-w2;
		for (j = 0 ; j < height ; j++) {
			dy = j-h2;
			distance[i+j*width] = floor(sqrt(dx*dx+dy*dy));
			/* precheck for values outside the largest circle */
			if (distance[i+j*width] > end) distance[i+j*width] = 0;
		}
	}

	out = calloc(x*y*((end - start) / step +1), sizeof(float));
	rval = newVal(BSQ, x, y, ((end-start)/step +1), FLOAT, out);
	accum = calloc(end+1, sizeof(struct dstore));

	/* run a window over every pixel and do the math */
    for (i = 0 ; i < x ; i+=1) {
        load_window(w, obj, i, 0, ignore);
        for (j = 0 ; j < y ; j+=1) {
            if (j) roll_window(w, obj, i, j, ignore);

			v1 = ((float **)w->row)[h2][w2];

			if (v1 == ignore) {
				continue;
			}

			memset(accum, 0, (end+1)*sizeof(struct dstore));

			/*
			** pick a pair of opposing points, and add them to the
			** accumulator for their given distance.  We'll sum all the
			** accumulators later for a complete result from dist 1..N.
			*/

			for (q = 0 ; q <= h2 ; q++) {
				/* one of the double-derefs moved to here for speed */
				r1 = ((float **)w->row)[q];
				r2 = ((float **)w->row)[(height-1)-q];
				for (p = 0 ; p < width ; p++) {
					if (q == h2 && p == w2) {
						/* We only run the window up to the center point  */
						break;
					}

					v1 = r1[p];
					v2 = r2[(width-1)-p];

					if (v1 == ignore || v2 == ignore) continue;

					if ((d = distance[p+q*width]) != 0) {
						a1 = &(accum[d]);
						a1->sumx  += v1;
						a1->sumy  += v2;
						a1->sumxx += v1*v1;
						a1->sumyy += v2*v2;
						a1->sumxy += v1*v2;
						a1->count++;
					}
				}
			}

			/* now compute correlation from per-radii accumulators */
			for (r = 1 ; r <= end ; r++) {
				/* sum the values in preceeding bins */
				a1 = &accum[r-1];
				a2 = &accum[r];
				if (a1->count) {
					a2->sumx += a1->sumx;
					a2->sumy += a1->sumy;
					a2->sumxx += a1->sumxx;
					a2->sumyy += a1->sumyy;
					a2->sumxy += a1->sumxy;
					a2->count += a1->count;
				}
				// Don't bother if at least 1/2 the box isn't ignore values
				// this is M_PI_4 because we're only counting half the pixels
				// to begin with.
				if ((r - start) % step == 0) {
				if (a2->count > r*r*M_PI_4) {
					double c = a2->count;
					ssxx = a2->sumxx - a2->sumx*a2->sumx/c;
					ssyy = a2->sumyy - a2->sumy*a2->sumy/c;
					ssxy = a2->sumxy - a2->sumx*a2->sumy/c;
					if (ssxx != 0 && ssyy != 0) {
						out[cpos(i,j,((r-start)/step),rval)] = sqrt(ssxy*ssxy / (ssxx*ssyy));
					}
				}
				}
			}
		}
		printf("%d/%d\r", i, x);
		fflush(stdout);
    }

    free_window(w);
	free(distance);
	free(accum);

    return(rval);
}

void draw_cross(Var *obj, int x, int y, float ignore, char *out) ;
void draw_box(Var *obj, int x, int y, float ignore, char *out) ;
void draw_circle(Var *obj, int x, int y, float ignore, char *out) ;

Var *
ff_drawshape(vfuncptr func, Var * arg)
{
    Var *obj = NULL, *ovar = NULL;
    int x,y,z;
    char *out;
    float ignore = MAXFLOAT;
    int i,j,k, x1, x2, y1, y2, v, p1;

    const char *options[] = { "cross", "box", "circle", NULL };
    char *shape = options[0];

    Alist alist[9];
    alist[0] = make_alist( "object",    ID_VAL, NULL,   &obj);
    alist[1] = make_alist( "shape",     ID_ENUM, options,   &shape);
    alist[2] = make_alist( "ignore",    FLOAT, NULL,   &ignore);
    alist[3].name = NULL;

    if (parse_args(func, arg, alist) == 0) return(NULL);

    if (obj == NULL) {
        parse_error("%s: No object specified\n", func->name);
        return(NULL);
    }

    x = GetX(obj);
    y = GetY(obj);
    z = GetZ(obj);
	out = calloc(x*y,sizeof(char));
	ovar = newVal(BSQ, x, y, 1, BYTE, out);

	if (!strcmp(shape, "cross")) {
		draw_cross(obj, x, y, ignore, out);
	} else if (!strcmp(shape, "box")) {
		draw_box(obj, x, y, ignore, out);
	} else if (!strcmp(shape, "circle")) {
		draw_circle(obj, x, y, ignore, out);
	}
	return(ovar);
}

void
draw_cross(Var *obj, int x, int y, float ignore, char *out) 
{
	int i,j,p1,x1,x2,y1,y2 ,k;
	int v;

	for (j = 0 ; j < y ; j++) {
		for (i = 0 ; i < x ; i++) {
			v = extract_int(obj, cpos(i, j, 0, obj));
			if (v != ignore && v > 0) {
				for (k = -v ; k <= v ; k++) {
					if (i+k >= 0 && i+k < x) {
						out[cpos(i+k, j, 0, obj)] = 1;
					}
					if (j+k >= 0 && j+k < y) {
						out[cpos(i, j+k, 0, obj)] = 1;
					}
				}
			}
		}
	}
}

void
draw_box(Var *obj, int x, int y, float ignore, char *out) 
{
	int i,j,k,p1,x1,x2,y1,y2;
	int v;

	for (j = 0 ; j < y ; j++) {
		for (i = 0 ; i < x ; i++) {
			v = extract_int(obj, cpos(i, j, 0, obj));
			if (v != ignore && v > 0) {
				for (k = -v ; k <= v ; k++) {
					if (i+k >= 0 && i+k < x) {
						if (j-v >= 0 && j-v < y) {
							out[cpos(i+k, j-v, 0, obj)] = 1;
						}
						if (j+v >= 0 && j+v < y) {
							out[cpos(i+k, j+v, 0, obj)] = 1;
						}
					}
					if (j+k >= 0 && j+k < y) {
						if (i-v >= 0 && i-v < x) {
							out[cpos(i-v, j+k, 0, obj)] = 1;
						}
						if (i+v >= 0 && i+v < x) {
							out[cpos(i+v, j+k, 0, obj)] = 1;
						}
					}
				}

			}
		}
	}
}

void
draw_circle(Var *obj, int x, int y, float ignore, char *out) 
{
	int i,j,k,p1,x1,x2,y1,y2;
	int r;
	double f, ir;
	double c, s;
	int ic, js;

	for (j = 0 ; j < y ; j++) {
		for (i = 0 ; i < x ; i++) {
			r = extract_int(obj, cpos(i, j, 0, obj));
			if (r != ignore && r > 0) {
				ir = 1.0/r;
				for (f = 0 ; f < M_PI_2 ; f+=ir/2) {
					c = rint(cos(f) * r);
					s = rint(sin(f) * r);
					ic = i+c;
					js = j+s;
					if (ic >= 0 && ic < x && js >= 0 && js < y) {
						out[cpos(ic, js, 0, obj)] = 1;
					}

					ic = i-c;
					js = j-s;
					if (ic >= 0 && ic < x && js >= 0 && js < y) {
						out[cpos(ic, js, 0, obj)] = 1;
					}

					ic = i+c;
					js = j-s;
					if (ic >= 0 && ic < x && js >= 0 && js < y) {
						out[cpos(ic, js, 0, obj)] = 1;
					}

					ic = i-c;
					js = j+s;
					if (ic >= 0 && ic < x && js >= 0 && js < y) {
						out[cpos(ic, js, 0, obj)] = 1;
					}
				}
			}
		}
	}
}

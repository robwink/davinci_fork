#include "parser.h"

/**
 **/
#define ycorner(c, v) ((c) < 4 ? -(v) : ((c) > 6 ? (v) : 0))
#define xcorner(c, v) ((c) == 1 || (c) == 4 || (c) == 7 ? -(v) : \
                      ((c) == 3 || (c) == 6 || (c) == 9 ? (v) : 0))

void jfill_x(Var *, int , int , int , float *, float , int);
void jfill_y(Var *, int , int , int , float *, float , int);
void jfill_merge(float *, float *, float , int);
void jfill_lr(Var *, int , int , int , float *, float , int, int);
void jfill_tb(Var *, int , int , int , float *, float , int);
void jfill_tr(Var *, int , int , int , float *, float , int);
void jfill_tl(Var *, int , int , int , float *, float , int);
void jfill(float *data, int n, float fill, int radius, int wrap);


Var *
ff_jfill(vfuncptr func, Var * arg)
{
  Var *v = NULL, *obj = NULL;
  float val[10], dist[10];
  int ecount, xpos[10], ypos[10], width, height, depth;
  int i, j, k, e, x, y, band, pos;
  float *data = NULL, d;
  int wrap = 0;			/* left to right wrap around */
  int radius = 3;			/* distance to search */
  int neighbors = 5;		/* minimum number of neighbors */
  float fill = 0.0;		/* fill value */
  
  Alist alist[6];
  alist[0] = make_alist( "object",    ID_VAL, NULL, &obj);
  alist[1] = make_alist( "fill",      FLOAT,  NULL, &fill);
  alist[2] = make_alist( "radius",    INT,    NULL, &radius);
  alist[3] = make_alist( "wrap",      INT,    NULL, &wrap);
  alist[4] = make_alist( "neighbors",      INT,    NULL, &neighbors);
  alist[5].name = NULL;
  
  if (parse_args(func, arg, alist) == 0) return(NULL);
  
  if (obj == NULL) {
    parse_error("%s: No object specified\n", func->name);
    return(NULL);
  }
  
  width = GetSamples(V_SIZE(obj), V_ORG(obj));
  height = GetLines(V_SIZE(obj), V_ORG(obj));
  depth = GetBands(V_SIZE(obj), V_ORG(obj));
  data = (float *) calloc(sizeof(float), width * height * depth);
  
  /** 
   ** Put together return value
   **/
  
  v = newVar();
  V_TYPE(v) = ID_VAL;
  V_ORG(v) = V_ORG(obj);
  V_DSIZE(v) = V_DSIZE(obj);
  V_SIZE(v)[0] = V_SIZE(obj)[0];
  V_SIZE(v)[1] = V_SIZE(obj)[1];
  V_SIZE(v)[2] = V_SIZE(obj)[2];
  V_FORMAT(v) = FLOAT;
  V_DATA(v) = data;
  
  for (band = 0; band < depth; band++) {
    for (j = 0; j < height; j++) {
      for (i = 0; i < width; i++) {
	pos = cpos(i, j, band, obj);
	d = extract_double(obj, pos);
	data[pos] = d;
	if (d != fill) {
	  continue;
	}
	ecount = 0;
	for (k = 1; k < radius; k++) {
	  for (e = 1; e <= 9; e++) {
	    if (k == 1)
	      val[e] = fill;
	    if (val[e] == fill) {
	      x = i + xcorner(e, k);
	      y = j + ycorner(e, k);
	      if (x < 0 || x >= width) {
		if (wrap) {
		  x = (x+width) % width;
		} else {
		  continue;
		}
	      }
	      
	      if (y < 0 || y >= height) {
		continue;
	      }
	      
	      val[e] = extract_float(obj, cpos(x, y, band, obj));
	      xpos[e] = x;
	      ypos[e] = y;
	      dist[e] = 1.0 / (k * k);
	      if (val[e] != fill)
		ecount++;
	    }
	  }
	  if (ecount >= neighbors) {
	    d = 0;
	    for (e = 1; e <= 9; e++) {
	      /**
	       ** compute weighted average
	       **/
	      if (e == 5)
		continue;
	      if (val[e] != fill) {
		val[5] += val[e] * dist[e];
		d += dist[e];
	      }
	    }
	    data[pos] = val[5] / d;
	    /**
	     ** stop everything.
	     **/
	    e = 10; 
	    k = radius;
	  }
	}
      }
    }
  }
  return (v);
}

Var *
ff_ifill(vfuncptr func, Var * arg)
{
  Var *v = NULL, *obj = NULL;
  int radius = 3, wrap=0;
  float fill = 0.0;
  float *data1 = NULL, *data2 = NULL, *data3 = NULL;
  int width,height,depth, dsize;
  char *pass = "1234";

  Alist alist[6];
  alist[0] = make_alist( "object",    ID_VAL, NULL, &obj);
  alist[1] = make_alist( "fill",      FLOAT,  NULL, &fill);
  alist[2] = make_alist( "radius",    INT,    NULL, &radius);
  alist[3] = make_alist( "wrap",      INT,    NULL, &wrap);
  alist[4] = make_alist( "pass",      ID_STRING,    NULL, &pass);
  alist[5].name = NULL;
  
  if (parse_args(func, arg, alist) == 0) return(NULL);
  
  if (obj == NULL) {
    parse_error("%s: No object specified\n", func->name);
    return(NULL);
  }

  width = GetSamples(V_SIZE(obj), V_ORG(obj));
  height = GetLines(V_SIZE(obj), V_ORG(obj));
  depth = GetBands(V_SIZE(obj), V_ORG(obj));
  dsize = width*height*depth;
  data1 = (float *) calloc(sizeof(float), dsize);
  data2 = (float *) calloc(sizeof(float), dsize);
  data3 = (float *) calloc(sizeof(float), dsize);

  /** 
   ** Put together return value
   **/

  if (strchr(pass, '3') && strchr(pass, '4')) {
    jfill_tl(obj, width, height, depth, data1, fill, radius);
    jfill_tr(obj, width, height, depth, data2, fill, radius);
    jfill_merge(data1, data2, fill, dsize);
  } else if (strchr(pass, '4')) {
    jfill_tl(obj, width, height, depth, data1, fill, radius);
  } else if (strchr(pass, '3')) {
    jfill_tr(obj, width, height, depth, data1, fill, radius);
  }
  
  if (strchr(pass, '1') && strchr(pass, '2')) {
    jfill_tb(obj, width, height, depth, data2, fill, radius);
    jfill_lr(obj, width, height, depth, data3, fill, radius, wrap);
    jfill_merge(data2, data3, fill, dsize);
  } else if (strchr(pass, '1')) {
    jfill_tb(obj, width, height, depth, data2, fill, radius);
  } else if (strchr(pass, '2')) {
    jfill_lr(obj, width, height, depth, data2, fill, radius, wrap);
  }
  
  if ((strchr(pass, '3') || strchr(pass, '4')) &&
      (strchr(pass, '1') || strchr(pass, '2'))) {
    jfill_merge(data1, data2, fill, dsize);
  } else if (strchr(pass, '1') || strchr(pass, '2')) {
    float *t = data2;
    data2 = data1;
    data1 = t;
  }
  
  v = newVar();
  V_TYPE(v) = ID_VAL;
  V_ORG(v) = V_ORG(obj);
  V_DSIZE(v) = V_DSIZE(obj);
  V_SIZE(v)[0] = V_SIZE(obj)[0];
  V_SIZE(v)[1] = V_SIZE(obj)[1];
  V_SIZE(v)[2] = V_SIZE(obj)[2];
  V_FORMAT(v) = FLOAT;
  V_DATA(v) = data1;
  
  free(data2);
  free(data3);
  return (v);
}

void
jfill_x(Var *obj, int width, int height, int depth, float *data, 
		float fill, int radius)
{
    int i, j, band, pos, x1, x2, y1, y2;
    float m, d;
	int state = 0;

    for (band = 0; band < depth; band++) {
        for (i = 0; i < width; i++) {
			state = 0;
            for (j = 0; j < height; j++) {
                pos = cpos(i, j, band, obj);
                d = extract_float(obj, pos);
                data[pos] = d;
                if (state == 0 && d != fill) {
                        y1 = d;
                        x1 = j;
                        state = 1;
                } else if (state == 1) {
                    if (d == fill) {
                        state = 2;
                    } else {
                        y1 = d;
                        x1 = j;
                    }
                } else if (state == 2 && d != fill) {
                    y2 = d;
                    x2 = j;
					if (x2-x1 > radius) {
						state = 0;
						continue;
					}
                    m = (y2 - y1)/(x2-x1);
                    for (j = x1+1 ; j < x2 ; j++) {
                        pos = cpos(i, j, band, obj);
                        data[pos] = y1+m*(j-x1);
                    }
                    x1 = x2;
                    y1 = y2;
                    state = 1;
                }
            }
        }
    }
}

void
jfill_y(Var *obj, int width, int height, int depth, float *data, 
		float fill, int radius)
{
    int i, j, band, pos, x1, x2, y1, y2;
    float m, d;
	int state = 0;

    for (band = 0; band < depth; band++) {
		for (i = 0; i < height; i++) {
			state = 0;
			for (j = 0; j < width; j++) {
                pos = cpos(j, i, band, obj);
                d = extract_float(obj, pos);
                data[pos] = d;
                if (state == 0 && d != fill) {
                        y1 = d;
                        x1 = j;
                        state = 1;
                } else if (state == 1) {
                    if (d == fill) {
                        state = 2;
                    } else {
                        y1 = d;
                        x1 = j;
                    }
                } else if (state == 2 && d != fill) {
                    y2 = d;
                    x2 = j;
					if (x2-x1 > radius) {
						state = 0;
						continue;
					}
                    m = (y2 - y1)/(x2-x1);
                    for (j = x1+1 ; j < x2 ; j++) {
                        pos = cpos(j, i, band, obj);
                        data[pos] = y1+m*(j-x1);
                    }
                    x1 = x2;
                    y1 = y2;
                    state = 1;
                }
            }
        }
    }
}

void
jfill_lr(Var *obj, int x, int y, int z, float *data, float fill, int radius, int wrap)
{
    int i, j, k, *p;
    float *d;

	/**
	 ** do across
	 **/
	d = (float *)calloc(x, sizeof(float));
	p = (int *)calloc(x, sizeof(int));

    for (k = 0; k < z; k++) {
		for (j = 0; j < y; j++) {
			for (i = 0; i < x; i++) {
				p[i] = cpos(i, j, k, obj);
				d[i] = extract_float(obj, p[i]);
            }
			jfill(d, x, fill, radius, wrap);
			for (i = 0; i < x; i++) {
				data[p[i]] = d[i];
            }
        }
    }
	free(d);
	free(p);
}

void
jfill_tb(Var *obj, int x, int y, int z, float *data, float fill, int radius)
{
    int i, j, k, *p;
    float *d;

	d = (float *)calloc(y, sizeof(float));
	p = (int *)calloc(y, sizeof(int));

    for (k = 0; k < z; k++) {
		for (i = 0; i < x; i++) {
			for (j = 0; j < y; j++) {
				p[j] = cpos(i, j, k, obj);
				d[j] = extract_float(obj, p[j]);
            }
			jfill(d, y, fill, radius, 0);
			for (j = 0; j < y; j++) {
				data[p[j]] = d[j];
            }
        }
    }
	free(d);
	free(p);
}

void
jfill_tr(Var *obj, int x, int y, int z, float *data, float fill, int radius)
{
    int n, i, j, k, l, *p;
    float *d;

	/**
	 ** do top to right
	 **/
	n = max(x,y);
	d = (float *)calloc(n, sizeof(float));
	p = (int *)calloc(n, sizeof(int));

    for (k = 0; k < z; k++) {
		for (i = 0 ; i < x ; i++) {
			for (j = 0 ; j < y ; j++) {
				if (i+j >= x) break;
				p[j] = cpos(i+j, j, k, obj);
				d[j] = extract_float(obj, p[j]);
			}
			jfill(d, j, fill, radius,0);
			for (l = 0 ; l < j ; l++) {
				data[p[l]] = d[l];
			}
		}
		/* we've already done the zero stripe, start at 1 */
		for (j = 1 ; j < y ; j++) {
			for (i = 0 ; i < x ; i++) {
				if (i+j >= y) break;
				p[i] = cpos(i, j+i, k, obj);
				d[i] = extract_float(obj, p[i]);
			}
			jfill(d, i, fill, radius, 0);
			for (l = 0 ; l < i ; l++) {
				data[p[l]] = d[l];
			}
		}
	}
	free(d);
	free(p);
}


void
jfill_tl(Var *obj, int x, int y, int z, float *data, float fill, int radius)
{
    int n, i, j, k, l, *p;
    float *d;

	/**
	 ** do top to right
	 **/
	n = max(x,y);
	d = (float *)calloc(n, sizeof(float));
	p = (int *)calloc(n, sizeof(int));

    for (k = 0; k < z; k++) {
		for (i = 0 ; i < x ; i++) {
			for (j = 0 ; j < y ; j++) {
				if (i-j < 0) break;
				p[j] = cpos(i-j, j, k, obj);
				d[j] = extract_float(obj, p[j]);
			}
			jfill(d, j, fill, radius,0);
			for (l = 0 ; l < j ; l++) {
				data[p[l]] = d[l];
			}
		}
		/* we've already done the zero stripe, start at 1 */

		for (j = 1 ; j < y ; j++) {
			for (i = 0 ; i < x ; i++) {
				if (x-i-1 < 0 || j+i >= y) break;
				p[i] = cpos(x-i-1, j+i, k, obj);
				d[i] = extract_float(obj, p[i]);
			}
			jfill(d, i, fill, radius,0);
			for (l = 0 ; l < i ; l++) {
				data[p[l]] = d[l];
			}
		}
	}
	free(d);
	free(p);
}

void
jfill_merge(float *d1, float *d2, float fill, int dsize)
{
	int i;

	for (i = 0 ; i < dsize ; i++) {
		if (d1[i] == fill)  {
			if (d2[i] != fill) {
				d1[i] = d2[i];
			}
		} else {
			if (d2[i] != fill) {
				d1[i] += d2[i];
				d1[i] /= 2.0;
			}
		}
	}
}

void
jfill(float *data, int n, float fill, int radius, int wrap)
{
	int i, j, x1, x2, state = 0;
	float y1, y2, m, d;
	int N = n;

	if (wrap) N+=radius;

	for (i = 0 ; i < N ; i++) {
		d = data[i % n];
		if (state == 0 && d != fill) {
				y1 = d;
				x1 = i;
				state = 1;
		} else if (state == 1) {
			if (d == fill) {
				state = 2;
			} else {
				y1 = d;
				x1 = i;
			}
		} else if (state == 2 && d != fill) {
			y2 = d;
			x2 = i;
			if (x2-x1 > radius) {
				state = 0;
				continue;
			}
			m = (y2 - y1)/(x2-x1);
			for (j = x1+1 ; j < x2 ; j++) {
				data[j % n] = y1+m*(j-x1);
			}
			x1 = x2;
			y1 = y2;
			state = 1;
		}
	}
}




/**
***
*** This computes the weighted average of all pixels within a neighborhood,
*** where the neighborhood is no larger than the input weighting mask, and
*** the actual (rectangular) limits of which are found by searching for
*** neighbors in the 4 cardinal directions.  If a neighbor isn't found, a
*** distance is determined as the average of the found neighbors.
***
*** This assumes that 0.0 is the ignore value
*** This makes two complete copies of the input data (in float), both
*** with an extra kernel/2 border.
**/

Var *ff_kfill(vfuncptr func, Var * arg)
{
  Var   *pic_v = NULL;                                  /* the picture */
  Var   *wgt_v = NULL;	                                /* the weighting array */
  Var   *itr_v = NULL;	                                /* the number of iterations */
  Var   *out;		                                /* the output picture */
  int    w;                                             /* width of the weight matrix (= height) */
  int    x,y,z;
  float *wgt = NULL;                                    /* storage location of the weight array */
  int    c;                                             /* general array counter */
  int    n;                                             /* number of elements - generally */
  int    i, j, k, mi, mj;                               /* general array counters */
  float  min_wgt;                                       /* minimum value of weight */
  float *pic = NULL, *pic2 = NULL, *tpic = NULL;
  float  pixel;                                         /* working var */
  int    wth;                                           /* (w-1)/2 */
  int    pic_x, pic_y;                                  /* dims of the expanded picture */
  int    px, py, pi;
  int    pl, pr, pu, pd, r, pave;                       /* the nearest neighbor positions */
  float  sum_non_zero_mask;
  int    nn;
  int    niter = 0;
  int    nzeroes = 0;

  
  Alist alist[4];
  alist[0] = make_alist("picture",		ID_VAL,		NULL,	&pic_v);
  alist[1] = make_alist("weight",		ID_VAL,		NULL,	&wgt_v);
  alist[2] = make_alist("iterations", 		ID_VAL,		NULL,	&itr_v);
  alist[3].name = NULL;
  
  if (parse_args(func, arg, alist) == 0) return(NULL);
  
  if (pic_v == NULL && wgt_v == NULL){
    parse_error("kfill() - 4/25/04");
    parse_error("Fills in null points of a 2 dimensional float array by linear interpolation of nearest neighbors.");
    parse_error("Will not handle 3-d arrays!\n");
    parse_error("Syntax:  b = kfill(picture,weight,iterations)");
    parse_error("example: b = kfill(b,wt)");
    parse_error("Set iterations to limit the maximum number of pixels away nearest neighbor must be to be valid.");
    parse_error("The weight array is produced by weight_array() - written by Phil.\n");
    return NULL;
  }
  if (pic_v == NULL) {
    parse_error("ERROR! picture not specified.\n"); return NULL;
  }
  
  if (wgt_v == NULL){
    parse_error("ERROR! weight matrix not specified.\n"); return NULL;
  }
  
  x = GetX(pic_v);		  /* get the x dimension of the image */
  y = GetY(pic_v);		  /* get the y dimension of the image */
  z = GetZ(pic_v);                /* get the z dimension of the image */
  if (z > 1){
    parse_error("Will not handle 3D arrays.\n"); return NULL;
  }

  w = GetX(wgt_v);		  /* get the x dimension of the weighting array */
  if (GetY(wgt_v) != w || GetZ(wgt_v) > 1){
    parse_error("Weight array must be the same dim in X & Y with a Z dim of 1.\n");
    return NULL;
  }

  if ((w%2) == 0 || w < 7){
    parse_error("Width should be an odd number greater than 5.\n");
    return NULL;
  }

  if (itr_v != NULL){
    niter = extract_int(itr_v, cpos(0, 0, 0, itr_v));
  }

  /* extract weight data */
  wgt = (float *)calloc(sizeof(float), w*w);
  if (wgt == NULL){
    parse_error("ERROR! Unable to alloc %d bytes.\n", sizeof(float)*w*w);
    return NULL;
  }

  c = 0;
  for(j = 0; j < w; j++){
    for(i = 0; i < w; i++){
      wgt[c++] = extract_float(wgt_v, cpos(i, j, 0, wgt_v));
    }
  }
  
  wth = (w-1)/2;		  /* the "radius" of the weighting array */
  
  /* extract picture data */
  pic_x = x+2*wth; pic_y = y+2*wth;
  pic = (float *)calloc(sizeof(float), pic_x*pic_y);
  if (pic == NULL){
    parse_error("ERROR! Unable to alloc %d bytes.\n", sizeof(float)*pic_x*pic_y);
    free(wgt); /* cleanup before exit */
    return NULL;
  }

  pic2 = (float *)calloc(sizeof(float), pic_x*pic_y);
  if (pic2 == NULL){
    parse_error("ERROR! Unable to alloc %d bytes.\n", sizeof(float)*pic_x*pic_y);
    free(wgt); /* cleanup before exit */
    free(pic);
    return NULL;
  }

  for(j = 0; j < y; j++){
    for(i = 0; i < x; i++){
      pic[(j+wth)*pic_x+(i+wth)] = extract_float(pic_v, cpos(i, j, 0, pic_v));
    }
  }

  /* find min of the weight array */
  n = w*w;
  min_wgt = wgt[0]; /* assume minimum */
  for(i = 1; i < n; i++){
    if (wgt[i] < min_wgt){ min_wgt = wgt[i]; }
  }

  if (min_wgt == 0.0){ min_wgt = 1.0; } /* sanity */

  /* divide wgt array by the minimum */
  for(i = 0; i < n; i++){ wgt[i] = wgt[i] / min_wgt; }

  /* iterate through data */
  for(k = 1; (k <= niter) || (niter == 0); k++){

    /* copy pic onto pic2 */
    memcpy(pic2, pic, sizeof(float)*pic_x*pic_y);
    nzeroes = 0;

    for(j = 0; j < y; j++){
      for(i = 0; i < x; i++){  
	
	py = j+wth; px = i+wth;
	pi = py * pic_x + px;
	
	if (pic[pi] == 0.0){

	  pl = 0;
	  pr = 0;
	  pu = 0;
	  pd = 0;
	  r = 1;
	  pave = 0;
	  nn = 0;

	  /* loop through the data to find the nearest neighbors in the 4 cardinal directions */
	  while(r <= wth) {
	    pl = (pl == 0 && pic[pi - r] > 0)?r:pl;
	    pr = (pr == 0 && pic[pi + r] > 0)?r:pr;
	    pu = (pu == 0 && pic[pi - r * pic_x] > 0)?r:pu;
	    pd = (pd == 0 && pic[pi + r * pic_x] > 0)?r:pd;

	    /* in the case that we don't have any near neighbors, set values to ave p */
	    if (r == wth) {
              nn = ((pl>0)?1:0) + ((pr>0)?1:0) + ((pu>0)?1:0) + ((pd>0)?1:0);
              if (nn > 0) {
		pave = (pl + pr + pu + pd) / nn;
		pl = (pl == 0)?pave:pl;
		pr = (pr == 0)?pave:pr;
		pu = (pu == 0)?pave:pu;
		pd = (pd == 0)?pave:pd;
	      }
	    }

	    r += 1;
	  }

	  if (nn>1) {
	    nzeroes++;
	    pixel = 0.0;
	    sum_non_zero_mask = 0.0;
	  
	    for(mj = wth - pu; mj <= wth + pd; mj++){
	      for(mi = wth - pl; mi <= wth + pr; mi++){
		pixel += pic[(j+mj)*pic_x+(i+mi)] * wgt[mj*w+mi];
		if (pic[(j+mj)*pic_x+(i+mi)] > 0.0){sum_non_zero_mask += wgt[mj*w+mi];}
	      }
	    }

	    if (sum_non_zero_mask == 0.0){ sum_non_zero_mask = 1.0; } /* sanity */

	    /* divide by weight */
	    /* assign pixel to location */
	    pic2[pi] = pixel / sum_non_zero_mask;
	  }
	}
      }
    }

    /* swap pic & pic2 arrays */
    tpic = pic; pic = pic2; pic2 = tpic;

    if (nzeroes == 0){
      break;
    }
  }

  if (niter == 0){
    parse_error("Done in %d iterations.\n", k);
  }

  /* construct return value */
  pic2 = (float *)realloc(pic2, sizeof(float)*x*y);
  if (pic2 == NULL){
    parse_error("blah");
    free(wgt);
    free(pic);
    return NULL;
  }

  /* copy data out */
  for(j = 0; j < y; j++){
    for(i = 0; i < x; i++){
      pic2[j*x+i] = pic[(j+wth)*pic_x+(i+wth)];
    }
  }
  out = newVal(BSQ, x, y, 1, FLOAT, pic2);

  /* cleanup before exit */
  free(wgt);
  free(pic);

  return out;
}

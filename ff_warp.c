#include "parser.h"
#include "func.h"


/**
*** Cheezy bi-linear interpolation.
*** Need something both faster and better
**/

float
interp_nn(float x1, float y1, Var *obj, float ignore)
{
	int w = GetX(obj);
	int h = GetY(obj);
	float ix = floor(x1);
	float iy = floor(y1);

	if (x1 < 0 || x1 >= w || y1 < 0 || y1 >= h) return(ignore);
	return(extract_float(obj, cpos(ix, iy, 0, obj)));
}


float
interp_bilinear(float x1, float y1, Var *obj, float ignore)
{
	int w = GetX(obj);
	int h = GetY(obj);
	float ix1, iy1, ix2, iy2, px, py;
	float xv;
	float a1, a2, a3, a4;

	if (x1 < 0 || x1 >= w || y1 < 0 || y1 >= h) return(ignore);

	/* pixel centers are assumed to be on steps of 0.5 */
	x1 = max(x1-0.5, 0);
	y1 = max(y1-0.5, 0);

	ix1 = floor(x1);
	iy1 = floor(y1);
	
	ix2 = min(ix1+1, w-1);
	iy2 = min(iy1+1, h-1);

	px = x1-ix1;
	py = y1-iy1;
	
	a1 = extract_float(obj, cpos(ix1,   iy1, 0, obj));
	a2 = extract_float(obj, cpos(ix2,   iy1, 0, obj));
	a3 = extract_float(obj, cpos(ix1,   iy2, 0, obj));
	a4 = extract_float(obj, cpos(ix2,   iy2, 0, obj));

	if (a1 == ignore) {
		if (px < 0.5) return(ignore);
		else a1 = a2;
	}
	if (a2 == ignore) {
		if (px >= 0.5) return(ignore);
		else a2 = a1;
	}
	if (a3 == ignore) {
		if (px < 0.5) return(ignore);
		else a3 = a4;
	}
	if (a4 == ignore) {
		if (px >= 0.5) return(ignore);
		else a4 = a3;
	}
	if (a1 == ignore) {
		if (py < 0.5) return(ignore);
		else {
			a1 = a3;
			a2 = a4;
		}
	}
	if (a3 == ignore) {
		if (py >= 0.5) return(ignore);
		else {
			a3 = a1;
			a4 = a2;
		}
	}

	xv = (1.0-py) * 
			(((1.0-px) * a1) + 
		 	 ((    px) * a2))
	    +(    py) *
			(((1.0-px) * a3) + 
		 	 ((    px) * a4));
	return(xv);
}

void kenel_interpolation() {

	
}

// compute inverse of 3x3 matrix.
float *m_inverse(float *m)
{
    //the inverse is the adjoint divided through the determinant
	float *o = calloc(9,sizeof(float));

	o[0] = m[4]*m[8]-m[5]*m[7];
	o[1] = m[2]*m[7]-m[1]*m[8];
	o[2] = m[1]*m[5]-m[2]*m[4];

	o[3] = m[5]*m[6]-m[3]*m[8];
	o[4] = m[0]*m[8]-m[2]*m[6];
	o[5] = m[2]*m[3]-m[0]*m[5];

	o[6] = m[3]*m[7]-m[4]*m[6];
	o[7] = m[1]*m[6]-m[0]*m[7];
	o[8] = m[0]*m[4]-m[1]*m[3];
		
	return(o);
}

float *vxm(float *v, float *m) 
{
	float out[3];

	out[0] = v[0] * m[0] + v[1] * m[3] + v[2] * m[6];
	out[1] = v[0] * m[1] + v[1] * m[4] + v[2] * m[7];
	out[2] = v[0] * m[2] + v[1] * m[5] + v[2] * m[8];

	v[0] = out[0]/out[2];
	v[1] = out[1]/out[2];
	v[2] = out[2]/out[2];
	return(v);
}

float *new_v(float x, float y)
{
	float *out = calloc(3, sizeof(float));
	out[0] = x;
	out[1] = y;
	out[2] = 1;
	return(out);
}

/*
** This function can take a transformation matrix,
** or it could take some keywords to do the same things:
**
** xtranslate (xpos)
** ytranslate (ypos)
** xscale
** yscale
** xshear
** yshear
** rotate
**
** alternatively, we could just write handler functions to
** generate the M matrix for the user for each of the above operations
**
** This can be run two ways:
**    1) make an output image the same size as the input image and crop
**    2) Make an output image the scaled size of the input image, and 
**       don't crop.
** To do part 2, we need to find the inverse of M, so we can figure out
** where the input corners end up in the output space, and then fit 
** everything to the right limits.
**
** This function will eventually want something more than just my
** cheap bilinear interpolation algorithm too.
*/

Var *
ff_warp(vfuncptr func, Var * arg)
{
	Var *obj = NULL, *xm = NULL, *oval;
	float ignore = MINFLOAT;
	int i, j;
	float *out;
	int x,y,n;
	int grow=0;
	float m[9];
	float *minverse;
	float xmax, xmin, ymax, ymin;
	float v[3];
	int dsize;
	const char *options[] = { "nearest", "bilinear", 0 };
	char *interp = NULL;

	float (*interp_f)(float, float, Var *, float);

	Alist alist[6];

	alist[0] = make_alist( "object",  ID_VAL,    NULL,    &obj);
	alist[1] = make_alist( "matrix",  ID_VAL,    NULL,    &xm);
	alist[2] = make_alist( "ignore",  FLOAT,    NULL,    &ignore);
	alist[3] = make_alist( "grow",    INT,    NULL,    &grow);
	alist[4] = make_alist( "interp",  ID_ENUM,    options,    &interp);
	alist[5].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (obj == NULL) {
		parse_error("%s: No object specified\n", func->name);
		return(NULL);
	}
	if (ignore == MINFLOAT) ignore=-32768;

	x = GetX(obj);
	y = GetY(obj);
	n = V_SIZE(xm)[2];


	for (j = 0 ; j < 3 ; j++) {
		for (i = 0 ; i < 3 ; i++) {
			m[i+j*3] = extract_float(xm, cpos(i, j, 0, xm));
		}
	}

	xmin = ymin = 0;
	xmax = x;
	ymax = y;

	if (grow) {
		/* figure out the size of the output array */
		float *out;
		minverse = m_inverse(m);

		out = vxm(new_v(0,0), minverse);
		xmin = out[0];
		xmax = out[0];
		ymin = out[1];
		ymax = out[1];
		free(out);

		out = vxm(new_v(x,0), minverse);
		xmin = min(xmin, out[0]);
		xmax = max(xmax, out[0]);
		ymin = min(ymin, out[1]);
		ymax = max(ymax, out[1]);
		free(out);

		out = vxm(new_v(0,y), minverse);
		xmin = min(xmin, out[0]);
		xmax = max(xmax, out[0]);
		ymin = min(ymin, out[1]);
		ymax = max(ymax, out[1]);
		free(out);

		out = vxm(new_v(x,y), minverse);
		xmin = min(xmin, out[0]);
		xmax = max(xmax, out[0]);
		ymin = min(ymin, out[1]);
		ymax = max(ymax, out[1]);
		free(out);

		xmax = ceil(xmax);
		xmin = floor(xmin);
		ymax = ceil(ymax);
		ymin = floor(ymin);

		printf("new array corners:\n");
		printf("  %fx%f , %fx%f\n", xmin, ymin, xmax, ymax);
	}

	if (interp == NULL || !strcmp(interp, "nearest")) {
		interp_f = interp_nn;
	} else if (!strcmp(interp, "bilinear")) {
		interp_f = interp_bilinear;
	} else {
		parse_error("Invalid interpolation function\n");
		return(NULL);
	}

	dsize = (xmax-xmin)*(ymax-ymin);
	out = calloc(dsize, sizeof(float));
	oval = newVal(BSQ, xmax-xmin, ymax-ymin, 1, FLOAT, out);

	for (j = ymin ; j < ymax; j++) {
		for (i = xmin ; i < xmax ; i++) {
			v[0] = i + 0.5;
			v[1] = j + 0.5;
			v[2] = 1;
			vxm(v, m);
			out[cpos((int)(i-xmin),(int)(j-ymin),0,oval)] = interp_f(v[0], v[1], obj, ignore);
		}
	}
	return(oval);
}

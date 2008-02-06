#include "parser.h"

int is_deleted(float f)
{
    return (f < -1.22e34 && f > -1.24e34);
}

void cakima (int n, float x[], float y[], float **yd);
Var *linear_interp(Var *v0, Var *v1, Var *v2, float ignore);
Var *cubic_interp(Var *v0, Var *v1, Var *v2, char *type, float ignore);

Var *
ff_interp(vfuncptr func, Var *arg)
{
    Var *s = NULL, *e = NULL;
    float *x = NULL,*y = NULL, *fdata = NULL;
    int i, count = 0;
    Var *v[3] = {NULL,NULL,NULL};
    float x1,y1,x2,y2,w;
    float *m = NULL, *c = NULL; /* slopes and y-intercepts */
    int fromsz, tosz; /* number of elements in from & to arrays */
	float ignore = MINFLOAT;
	char *usage = "usage: %s(y1,x1,x2,[type={'linear'|'cubic'}]";
	char *type = "";
	char *types[] = {"linear", "cubic", NULL};
	Var *out;

    Alist alist[9];
    alist[0] = make_alist( "object",    ID_VAL,    NULL,    &v[0]);
    alist[1] = make_alist( "from",      ID_VAL,    NULL,    &v[1]);
    alist[2] = make_alist( "to",        ID_VAL,    NULL,    &v[2]);
    alist[3] = make_alist( "ignore",    FLOAT,    NULL,    &ignore);
    alist[4] = make_alist( "type",    ID_ENUM,    types,    &type);
    alist[5] = make_alist( "y1",    ID_VAL,    NULL,    &v[0]);
    alist[6] = make_alist( "x1",    ID_VAL,    NULL,    &v[1]);
    alist[7] = make_alist( "x2",    ID_VAL,    NULL,    &v[2]);
    alist[8].name = NULL;
    
    if (parse_args(func, arg, alist) == 0) return(NULL);
    
    if (v[0] == NULL) {
	  parse_error("%s: y1 not specified.", func->name);
	  parse_error(usage, func->name);
      return(NULL);
    }
    if (v[1] == NULL) {
	  parse_error("%s: x1 not specified.", func->name);
	  parse_error(usage, func->name);
      return(NULL);
    }
    if (v[2] == NULL) {
	  parse_error("%s: x2 not specified.", func->name);
	  parse_error(usage, func->name);
	  return(NULL);
    }

    if (V_DSIZE(v[0]) != V_DSIZE(v[1])) {
      parse_error("Object and From values must be same size\n");
    }

	if (type == NULL || strlen(type) == 0 || !strcasecmp(type, "linear")) {
		out = linear_interp(v[0], v[1], v[2], ignore);
	} else if (!strncasecmp(type, "cubic", 5)) {
		out = cubic_interp(v[0], v[1], v[2], type, ignore);
	} else {
		parse_error("%s: Unrecognized type: %s\n", func->name, type);
	}
	return(out);
}
		
Var *linear_interp(Var *v0, Var *v1, Var *v2, float ignore) 
{
    Var *s = NULL, *e = NULL;
    float *x = NULL,*y = NULL, *fdata = NULL;
    int i, count = 0;
    float x1,y1,x2,y2,w;
    float *m = NULL, *c = NULL; /* slopes and y-intercepts */
    int fromsz, tosz; /* number of elements in from & to arrays */
	char *usage = "usage: interp(y1,x1,x2,[type={'linear'|'cubic'}]";

    fromsz = V_DSIZE(v0);
    tosz = V_DSIZE(v2);
    
    x = (float *)calloc(sizeof(FLOAT), fromsz);
    y = (float *)calloc(sizeof(FLOAT), fromsz);

    count = 0;
    for (i = 0 ; i < fromsz ; i++) {
        x[count] = extract_float(v1,i);
        y[count] = extract_float(v0,i);
        if (is_deleted(x[count]) || is_deleted(y[count]) ||
		x[count] == ignore || y[count] == ignore) continue;
		if (count && x[count] <= x[count-1]) {
			parse_error("Error: data is not monotonically increasing x1[%d] = %f", i, x[count]);
			free(fdata);
			free(x);
			free(y);
			return(NULL);
		}
        count++;
    }

    fdata = (float *)calloc(sizeof(FLOAT), tosz);
    m = (float *)calloc(sizeof(FLOAT), fromsz-1);
    c = (float *)calloc(sizeof(FLOAT), fromsz-1);

    /* evaluate & cache slopes & y-intercepts */
    for (i = 1; i < fromsz; i++){
        m[i-1] = (y[i]-y[i-1])/(x[i]-x[i-1]);
        c[i-1] = y[i-1] - m[i-1]*x[i-1];
    }

    for (i = 0 ; i < tosz ; i++) {
        w = extract_float(v2, i); /* output wavelength */
        if (is_deleted(w)) {
            fdata[i] = -1.23e34; 
		} else if (w == ignore) {
			fdata[i] = ignore;
        } else {

            /*
            ** Locate the segment containing the x-value of "w".
            ** Assume that x-values are monotonically increasing.
            */
            int st = 0, ed = fromsz-1, mid;
            
            while((ed-st) > 1){
                mid = (st+ed)/2;
                if (w > x[mid])     { st = mid; }
                else if (w < x[mid]){ ed = mid; }
                else                { st = ed = mid; }
            }
            x2 = x[ed]; y2 = y[ed];
            x1 = x[st]; y1 = y[st];

            if (y2 == y1) {
                fdata[i] = y1;
            } else {
                /* m = (y2-y1)/(x2-x1); */
                /* fdata[i] = m[st]*w + (y1 - m[st]*x1); */
                fdata[i] = m[st]*w + c[st];
            }
        }
    }

    s = newVar();
    V_TYPE(s) = ID_VAL;

    V_DATA(s) = (void *)fdata;
    V_DSIZE(s) = V_DSIZE(v2);
    V_SIZE(s)[0] = V_SIZE(v2)[0];
    V_SIZE(s)[1] = V_SIZE(v2)[1];
    V_SIZE(s)[2] = V_SIZE(v2)[2];
    V_ORG(s) = V_ORG(v2);
    V_FORMAT(s) = FLOAT;

    free(x);
    free(y);
    return(s);
}


Var *
ff_cinterp(vfuncptr func, Var *arg)
{
    Var *v[3] = {NULL,NULL,NULL};

	float **yd, *out, *xp, *yp;
	int npts, nout;
	int i, j;
	float x0, x1, x, h;
	int done;
	float ignore = MINFLOAT;
	char *type = NULL;

    Alist alist[6];
    alist[0] = make_alist( "object",    ID_VAL,    NULL,    &v[0]);
    alist[1] = make_alist( "from",      ID_VAL,    NULL,    &v[1]);
    alist[2] = make_alist( "to",        ID_VAL,    NULL,    &v[2]);
    alist[3] = make_alist( "type",      ID_ENUM,    NULL,   type);
    alist[4] = make_alist( "ignore",    FLOAT,    NULL,    &ignore);
    alist[5].name = NULL;

    if (parse_args(func, arg, alist) == 0) return(NULL);

	if (v[0] == NULL || v[1] == NULL || v[2] == NULL) {
		parse_error("error, usage: cinterp(y1,x1,x2)\n");
		return(NULL);
	}

	if (V_DSIZE(v[0]) != V_DSIZE(v[1])) {
		parse_error("error: x1 and y1 must be the same size\n");
		return(NULL);
	}
	return(cubic_interp(v[0], v[1], v[2], type, ignore));
}

Var *
cubic_interp(Var *v0, Var *v1, Var *v2, char *type, float ignore)
{
	float **yd, *out, *xp, *yp, *arena;
	int npts, nout;
	int i, j;
	float x0, x1, x, h;
	int done;
	int count = 0;
	int error = 0;

	npts = V_DSIZE(v0);
	nout = V_DSIZE(v2);

	/* this is the hard way */
	yd = calloc(npts, sizeof(float *));
	xp = calloc(sizeof(float), npts);
	yp = calloc(sizeof(float), npts);
	arena = calloc(sizeof(float), npts*4);
	out = calloc(nout, sizeof(float));

	for (i = 0 ; i < npts ; i++) {
		xp[count] = extract_float(v1, i);
		yp[count] = extract_float(v0, i);
		yd[count] = arena + 4*count;
		/* Handle deleted points and non-increasing data */
		if (xp[count] == ignore || yp[count] == ignore) {
			continue;
		}
		if (count && xp[count] <= xp[count-1]) {
			parse_error("Error: data is not monotonically increasing x1[%d] = %f", i, xp[count]);
			error= 1;
			break;
		}
		count++;
	}

	/* this is the case if we're not monotonic increasing */
	if (error) {
		free(arena);
		free(yd);
		free(xp);
		free(yp);
		return(NULL);
	}

	npts = count;

	cakima(npts, xp, yp, yd);

	done = i = j = 0;

	while(!done) {
		if (i >= nout) break;
		else if (j >= npts) break;

		x0 = xp[j];
		x1 = xp[j+1];
		x = extract_float(v2, i);
		if (x == ignore) {
			out[i] == ignore;
			i++;
		}

		if (x < x0) i++;
		else if (x > x1) j++;
		else {
			h = x-x0;
			out[i] = yd[j][0]+h*(yd[j][1]+h*(yd[j][2]/2.0+h*yd[j][3]/6.0));
			i++;
		}
	}

	free(arena);
	free(yd);
	free(xp);
	free(yp);
	return(newVal(V_ORG(v2), 
		V_SIZE(v2)[0], 
		V_SIZE(v2)[1], 
		V_SIZE(v2)[2], 
		FLOAT, 
		out));
}

#define ABS(a) fabs(a)
#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a < b ? a : b)

/* Copyright (c) Colorado School of Mines, 2006.*/
/* All rights reserved.                       */

/*********************** self documentation **********************/
/*****************************************************************************
CUBICSPLINE - Functions to compute CUBIC SPLINE interpolation coefficients

cakima		compute cubic spline coefficients via Akima's method
		  (continuous 1st derivatives, only)
cmonot		compute cubic spline coefficients via the Fritsch-Carlson method
		  (continuous 1st derivatives, only)
csplin		compute cubic spline coefficients for interpolation
		  (continuous 1st and 2nd derivatives)

chermite	compute cubic spline coefficients via Hermite Polynomial
		  (continuous 1st derivatives only)
******************************************************************************
Function Prototypes:
void cakima   (int n, float x[], float y[], float yd[][4]);
void cmonot   (int n, float x[], float y[], float yd[][4]);
void csplin   (int n, float x[], float y[], float yd[][4]);
void chermite (int n, float x[], float y[], float yd[][4]);

******************************************************************************
Input:
n		number of samples
x  		array[n] of monotonically increasing or decreasing abscissae
y		array[n] of ordinates

Output:
yd		array[n][4] of cubic interpolation coefficients (see notes)

******************************************************************************
Notes:
The computed cubic spline coefficients are as follows:
yd[i][0] = y(x[i])    (the value of y at x = x[i])
yd[i][1] = y'(x[i])   (the 1st derivative of y at x = x[i])
yd[i][2] = y''(x[i])  (the 2nd derivative of y at x = x[i])
yd[i][3] = y'''(x[i]) (the 3rd derivative of y at x = x[i])

To evaluate y(x) for x between x[i] and x[i+1] and h = x-x[i],
use the computed coefficients as follows:
y(x) = yd[i][0]+h*(yd[i][1]+h*(yd[i][2]/2.0+h*yd[i][3]/6.0))

Akima's method provides continuous 1st derivatives, but 2nd and
3rd derivatives are discontinuous.  Akima's method is not linear,
in that the interpolation of the sum of two functions is not the
same as the sum of the interpolations.

The Fritsch-Carlson method yields continuous 1st derivatives, but 2nd
and 3rd derivatives are discontinuous.  The method will yield a
monotonic interpolant for monotonic data.  1st derivatives are set
to zero wherever first divided differences change sign.

The method used by "csplin" yields continuous 1st and 2nd derivatives.

******************************************************************************
References:
See Akima, H., 1970, A new method for
interpolation and smooth curve fitting based on local procedures,
Journal of the ACM, v. 17, n. 4, p. 589-602.

For more information, see Fritsch, F. N., and Carlson, R. E., 1980,
Monotone piecewise cubic interpolation:  SIAM J. Numer. Anal., v. 17,
n. 2, p. 238-246.
Also, see the book by Kahaner, D., Moler, C., and Nash, S., 1989,
Numerical Methods and Software, Prentice Hall.  This function was
derived from SUBROUTINE PCHEZ contained on the diskette that comes
with the book.

For more general information on spline functions of all types see the book by:
Greville, T.N.E, 1969, Theory and Applications of Spline Functions,
Academic Press.

******************************************************************************
Author:  Dave Hale, Colorado School of Mines c. 1989, 1990, 1991
*****************************************************************************/
/**************** end self doc ********************************/

// #include <cwp.h>

void cakima (int n, float x[], float y[], float **yd)
/*****************************************************************************
Compute cubic interpolation coefficients via Akima's method
******************************************************************************
Input:
n		number of samples
x  		array[n] of monotonically increasing or decreasing abscissae
y		array[n] of ordinates

Output:
yd		array[n][4] of cubic interpolation coefficients (see notes)
******************************************************************************
Notes:
The computed cubic spline coefficients are as follows:
yd[i][0] = y(x[i])    (the value of y at x = x[i])
yd[i][1] = y'(x[i])   (the 1st derivative of y at x = x[i])
yd[i][2] = y''(x[i])  (the 2nd derivative of y at x = x[i])
yd[i][3] = y'''(x[i]) (the 3rd derivative of y at x = x[i])

To evaluate y(x) for x between x[i] and x[i+1] and h = x-x[i],
use the computed coefficients as follows:
y(x) = yd[i][0]+h*(yd[i][1]+h*(yd[i][2]/2.0+h*yd[i][3]/6.0))

Akima's method provides continuous 1st derivatives, but 2nd and
3rd derivatives are discontinuous.  Akima's method is not linear,
in that the interpolation of the sum of two functions is not the
same as the sum of the interpolations.

For more information, see Akima, H., 1970, A new method for
interpolation and smooth curve fitting based on local procedures,
Journal of the ACM, v. 17, n. 4, p. 589-602.
******************************************************************************
Author:  Dave Hale, Colorado School of Mines, 09/30/89
Modified:  Dave Hale, Colorado School of Mines, 02/28/91
	changed to work for n=1.
*****************************************************************************/
{
	int i;
	float sumw,yd1fx,yd1lx,dx,divdf3;

	/* copy ordinates into output array */
	for (i=0; i<n; i++)
		yd[i][0] = y[i];

	/* if n=1, then use constant interpolation */
	if (n==1) {
		yd[0][1] = 0.0;
		yd[0][2] = 0.0;
		yd[0][3] = 0.0;
		return;

	/* else, if n=2, then use linear interpolation */
	} else if (n==2) {
		yd[0][1] = yd[1][1] = (y[1]-y[0])/(x[1]-x[0]);
		yd[0][2] = yd[1][2] = 0.0;
		yd[0][3] = yd[1][3] = 0.0;
		return;
	}

	/* compute 1st divided differences and store in yd[.][2] */
	for (i=1; i<n; i++)
		yd[i][2] = (y[i]-y[i-1])/(x[i]-x[i-1]);

	/* compute weights and store in yd[.][3] */
	for (i=1; i<n-1; i++)
		yd[i][3] = fabs(yd[i+1][2]-yd[i][2]);
	yd[0][3] = yd[1][3];
	yd[n-1][3] = yd[n-2][3];

	/* compute 1st derivative at first x */
	sumw = yd[1][3]+yd[0][3];
	yd1fx = 2.0*yd[1][2]-yd[2][2];
	if (sumw!=0.0)
		yd[0][1] = (yd[1][3]*yd1fx+yd[0][3]*yd[1][2])/sumw;
	else
		yd[0][1] = 0.5*(yd1fx+yd[1][2]);

	/* compute 1st derivatives in interior as weighted 1st differences */
	for (i=1; i<n-1; i++) {
		sumw = yd[i+1][3]+yd[i-1][3];
		if (sumw!=0.0)
			yd[i][1] = (yd[i+1][3]*yd[i][2]+yd[i-1][3]*yd[i+1][2])/sumw;
		else
			yd[i][1] = 0.5*(yd[i][2]+yd[i+1][2]);
	}

	/* compute 1st derivative at last x */
	sumw = yd[n-2][3]+yd[n-1][3];
	yd1lx = 2.0*yd[n-1][2]-yd[n-2][2];
	if (sumw!=0.0)
		yd[n-1][1] = (yd[n-2][3]*yd1lx+yd[n-1][3]*yd[n-1][2])/sumw;
	else
		yd[n-1][1] = 0.5*(yd[n-1][2]+yd1lx);

	/* compute 2nd and 3rd derivatives of cubic polynomials */
	for (i=1; i<n; i++) {
		dx = x[i]-x[i-1];
		divdf3 = yd[i-1][1]+yd[i][1]-2.0*yd[i][2];
		yd[i-1][2] = 2.0*(yd[i][2]-yd[i-1][1]-divdf3)/dx;
		yd[i-1][3] = (divdf3/dx)*(6.0/dx);
	}
	yd[n-1][2] = yd[n-2][2]+(x[n-1]-x[n-2])*yd[n-2][3];
	yd[n-1][3] = yd[n-2][3];
}

void cmonot (int n, float x[], float y[], float yd[][4])
/*****************************************************************************
compute cubic interpolation coefficients via the Fritsch-Carlson method,
which preserves monotonicity
******************************************************************************
Input:
n		number of samples
x  		array[n] of monotonically increasing or decreasing abscissae
y		array[n] of ordinates

Output:
yd		array[n][4] of cubic interpolation coefficients (see notes)
******************************************************************************
Notes:
The computed cubic spline coefficients are as follows:
yd[i][0] = y(x[i])    (the value of y at x = x[i])
yd[i][1] = y'(x[i])   (the 1st derivative of y at x = x[i])
yd[i][2] = y''(x[i])  (the 2nd derivative of y at x = x[i])
yd[i][3] = y'''(x[i]) (the 3rd derivative of y at x = x[i])

To evaluate y(x) for x between x[i] and x[i+1] and h = x-x[i],
use the computed coefficients as follows:
y(x) = yd[i][0]+h*(yd[i][1]+h*(yd[i][2]/2.0+h*yd[i][3]/6.0))

The Fritsch-Carlson method yields continuous 1st derivatives, but 2nd
and 3rd derivatives are discontinuous.  The method will yield a
monotonic interpolant for monotonic data.  1st derivatives are set
to zero wherever first divided differences change sign.

For more information, see Fritsch, F. N., and Carlson, R. E., 1980,
Monotone piecewise cubic interpolation:  SIAM J. Numer. Anal., v. 17,
n. 2, p. 238-246.

Also, see the book by Kahaner, D., Moler, C., and Nash, S., 1989,
Numerical Methods and Software, Prentice Hall.  This function was
derived from SUBROUTINE PCHEZ contained on the diskette that comes
with the book.
******************************************************************************
Author:  Dave Hale, Colorado School of Mines, 09/30/89
Modified:  Dave Hale, Colorado School of Mines, 02/28/91
	changed to work for n=1.
Modified:  Dave Hale, Colorado School of Mines, 08/04/91
	fixed bug in computation of left end derivative
*****************************************************************************/
{
	int i;
	float h1,h2,del1,del2,dmin,dmax,hsum,hsum3,w1,w2,drat1,drat2,divdf3;

	/* copy ordinates into output array */
	for (i=0; i<n; i++)
		yd[i][0] = y[i];

	/* if n=1, then use constant interpolation */
	if (n==1) {
		yd[0][1] = 0.0;
		yd[0][2] = 0.0;
		yd[0][3] = 0.0;
		return;

	/* else, if n=2, then use linear interpolation */
	} else if (n==2) {
		yd[0][1] = yd[1][1] = (y[1]-y[0])/(x[1]-x[0]);
		yd[0][2] = yd[1][2] = 0.0;
		yd[0][3] = yd[1][3] = 0.0;
		return;
	}

	/* set left end derivative via shape-preserving 3-point formula */
	h1 = x[1]-x[0];
	h2 = x[2]-x[1];
	hsum = h1+h2;
	del1 = (y[1]-y[0])/h1;
	del2 = (y[2]-y[1])/h2;
	w1 = (h1+hsum)/hsum;
	w2 = -h1/hsum;
	yd[0][1] = w1*del1+w2*del2;
	if (yd[0][1]*del1<=0.0)
		yd[0][1] = 0.0;
	else if (del1*del2<0.0) {
		dmax = 3.0*del1;
		if (ABS(yd[0][1])>ABS(dmax)) yd[0][1] = dmax;
	}

	/* loop over interior points */
	for (i=1; i<n-1; i++) {

		/* compute intervals and slopes */
		h1 = x[i]-x[i-1];
		h2 = x[i+1]-x[i];
		hsum = h1+h2;
		del1 = (y[i]-y[i-1])/h1;
		del2 = (y[i+1]-y[i])/h2;

		/* if not strictly monotonic, zero derivative */
		if (del1*del2<=0.0) {
			yd[i][1] = 0.0;

		/*
		 * else, if strictly monotonic, use Butland's formula:
		 *      3*(h1+h2)*del1*del2
		 * -------------------------------
		 * ((2*h1+h2)*del1+(h1+2*h2)*del2)
		 * computed as follows to avoid roundoff error
		 */
		} else {
			hsum3 = hsum+hsum+hsum;
			w1 = (hsum+h1)/hsum3;
			w2 = (hsum+h2)/hsum3;
			dmin = MIN(ABS(del1),ABS(del2));
			dmax = MAX(ABS(del1),ABS(del2));
			drat1 = del1/dmax;
			drat2 = del2/dmax;
			yd[i][1] = dmin/(w1*drat1+w2*drat2);
		}
	}

	/* set right end derivative via shape-preserving 3-point formula */
	w1 = -h2/hsum;
	w2 = (h2+hsum)/hsum;
	yd[n-1][1] = w1*del1+w2*del2;
	if (yd[n-1][1]*del2<=0.0)
		yd[n-1][1] = 0.0;
	else if (del1*del2<0.0) {
		dmax = 3.0*del2;
		if (ABS(yd[n-1][1])>ABS(dmax)) yd[n-1][1] = dmax;
	}

	/* compute 2nd and 3rd derivatives of cubic polynomials */
	for (i=0; i<n-1; i++) {
		h2 = x[i+1]-x[i];
		del2 = (y[i+1]-y[i])/h2;
		divdf3 = yd[i][1]+yd[i+1][1]-2.0*del2;
		yd[i][2] = 2.0*(del2-yd[i][1]-divdf3)/h2;
		yd[i][3] = (divdf3/h2)*(6.0/h2);
	}
	yd[n-1][2] = yd[n-2][2]+(x[n-1]-x[n-2])*yd[n-2][3];
	yd[n-1][3] = yd[n-2][3];
}

void csplin (int n, float x[], float y[], float yd[][4])
/*****************************************************************************
compute cubic spline interpolation coefficients for interpolation with
continuous second derivatives
******************************************************************************
Input:
n		number of samples
x  		array[n] of monotonically increasing or decreasing abscissae
y		array[n] of ordinates

Output:
yd		array[n][4] of cubic interpolation coefficients (see notes)
******************************************************************************
Notes:
The computed cubic spline coefficients are as follows:
yd[i][0] = y(x[i])    (the value of y at x = x[i])
yd[i][1] = y'(x[i])   (the 1st derivative of y at x = x[i])
yd[i][2] = y''(x[i])  (the 2nd derivative of y at x = x[i])
yd[i][3] = y'''(x[i]) (the 3rd derivative of y at x = x[i])

To evaluate y(x) for x between x[i] and x[i+1] and h = x-x[i],
use the computed coefficients as follows:
y(x) = yd[i][0]+h*(yd[i][1]+h*(yd[i][2]/2.0+h*yd[i][3]/6.0))
******************************************************************************
Author:  Dave Hale, Colorado School of Mines, 10/03/89
Modified:  Dave Hale, Colorado School of Mines, 02/28/91
	changed to work for n=1.
Modified:  Dave Hale, Colorado School of Mines, 08/04/91
	fixed bug in computation of left end derivative
*****************************************************************************/
{
	int i;
	float h1,h2,del1,del2,dmax,hsum,w1,w2,divdf3,sleft,sright,alpha,t;

	/* if n=1, then use constant interpolation */
	if (n==1) {
		yd[0][0] = y[0];
		yd[0][1] = 0.0;
		yd[0][2] = 0.0;
		yd[0][3] = 0.0;
		return;

	/* else, if n=2, then use linear interpolation */
	} else if (n==2) {
		yd[0][0] = y[0];  yd[1][0] = y[1];
		yd[0][1] = yd[1][1] = (y[1]-y[0])/(x[1]-x[0]);
		yd[0][2] = yd[1][2] = 0.0;
		yd[0][3] = yd[1][3] = 0.0;
		return;
	}

	/* set left end derivative via shape-preserving 3-point formula */
	h1 = x[1]-x[0];
	h2 = x[2]-x[1];
	hsum = h1+h2;
	del1 = (y[1]-y[0])/h1;
	del2 = (y[2]-y[1])/h2;
	w1 = (h1+hsum)/hsum;
	w2 = -h1/hsum;
	sleft = w1*del1+w2*del2;
	if (sleft*del1<=0.0)
		sleft = 0.0;
	else if (del1*del2<0.0) {
		dmax = 3.0*del1;
		if (ABS(sleft)>ABS(dmax)) sleft = dmax;
	}

	/* set right end derivative via shape-preserving 3-point formula */
	h1 = x[n-2]-x[n-3];
	h2 = x[n-1]-x[n-2];
	hsum = h1+h2;
	del1 = (y[n-2]-y[n-3])/h1;
	del2 = (y[n-1]-y[n-2])/h2;
	w1 = -h2/hsum;
	w2 = (h2+hsum)/hsum;
	sright = w1*del1+w2*del2;
	if (sright*del2<=0.0)
		sright = 0.0;
	else if (del1*del2<0.0) {
		dmax = 3.0*del2;
		if (ABS(sright)>ABS(dmax)) sright = dmax;
	}

	/* compute tridiagonal system coefficients and right-hand-side */
	yd[0][0] = 1.0;
	yd[0][2] = 2.0*sleft;
	for (i=1; i<n-1; i++) {
		h1 = x[i]-x[i-1];
		h2 = x[i+1]-x[i];
		del1 = (y[i]-y[i-1])/h1;
		del2 = (y[i+1]-y[i])/h2;
		alpha = h2/(h1+h2);
		yd[i][0] = alpha;
		yd[i][2] = 3.0*(alpha*del1+(1.0-alpha)*del2);
	}
	yd[n-1][0] = 0.0;
	yd[n-1][2] = 2.0*sright;

	/* solve tridiagonal system for slopes */
	t = 2.0;
	yd[0][1] = yd[0][2]/t;
	for (i=1; i<n; i++) {
		yd[i][3] = (1.0-yd[i-1][0])/t;
		t = 2.0-yd[i][0]*yd[i][3];
		yd[i][1] = (yd[i][2]-yd[i][0]*yd[i-1][1])/t;
	}
	for (i=n-2; i>=0; i--)
		yd[i][1] -= yd[i+1][3]*yd[i+1][1];

	/* copy ordinates into output array */
	for (i=0; i<n; i++)
		yd[i][0] = y[i];

	/* compute 2nd and 3rd derivatives of cubic polynomials */
	for (i=0; i<n-1; i++) {
		h2 = x[i+1]-x[i];
		del2 = (y[i+1]-y[i])/h2;
		divdf3 = yd[i][1]+yd[i+1][1]-2.0*del2;
		yd[i][2] = 2.0*(del2-yd[i][1]-divdf3)/h2;
		yd[i][3] = (divdf3/h2)*(6.0/h2);
	}
	yd[n-1][2] = yd[n-2][2]+(x[n-1]-x[n-2])*yd[n-2][3];
	yd[n-1][3] = yd[n-2][3];
}
// #include <cwp.h>

void chermite (int n, float x[], float y[], float yd[][4])
/*****************************************************************************
Compute cubic interpolation coefficients via Hermite polynomial
******************************************************************************
Input:
n		number of samples
x  		array[n] of monotonically increasing or decreasing abscissae
y		array[n] of ordinates

Output:
yd		array[n][4] of cubic interpolation coefficients (see notes)
******************************************************************************
Notes:
The computed cubic spline coefficients are as follows:
yd[i][0] = y(x[i])    (the value of y at x = x[i])
yd[i][1] = y'(x[i])   (the 1st derivative of y at x = x[i])
yd[i][2] = y''(x[i])  (the 2nd derivative of y at x = x[i])
yd[i][3] = y'''(x[i]) (the 3rd derivative of y at x = x[i])

To evaluate y(x) for x between x[i] and x[i+1] and h = x-x[i],
use the computed coefficients as follows:
y(x) = yd[i][0]+h*(yd[i][1]+h*(yd[i][2]/2.0+h*yd[i][3]/6.0))

*****************************************************************************/
{
	int i;
	float dx ,f1,f3;

	/* copy ordinates into output array */
	for (i=0; i<n; i++){
		yd[i][0] = y[i];
        }

	/* if n=1, then use constant interpolation */
	if (n==1) {
		yd[0][1] = 0.0;
		yd[0][2] = 0.0;
		yd[0][3] = 0.0;
		return;

	/* else, if n=2, then use linear interpolation */
	} else if (n==2) {
		yd[0][1] = yd[1][1] = (y[1]-y[0])/(x[1]-x[0]);
		yd[0][2] = yd[1][2] = 0.0;
		yd[0][3] = yd[1][3] = 0.0;
		return;
	}



        /* compute forward & backwards differences at the ends */

        yd[0][1]   = (y[2] - y[0])     / (x[2] - x[0]);
        yd[n-1][1] = (y[n-1] - y[n-2]) / (x[n-1] - x[n-2]);

        /* compute central differences in between */

        for( i=1; i<n-1; i++ ){

           yd[i][1] = 0.5 * (y[i] - y[i-1]) / (x[i] - x[i-1])
	            + 0.5 * (y[i+1] - y[i]) / (x[i+1] - x[i]);
        }

        /* calculate 2nd and 3rd derivatives */

        for( i=0; i<n-1; i++ ){

           dx       = x[i+1] - x[i];
           f1       = (y[i+1] - y[i]) / dx;
           f3       = yd[i][1] + yd[i+1][1] - 2.0 * f1;
           yd[i][2] = 2.0*(f1 - yd[i][1] - f3) / dx;
           yd[i][3] = 6.0*f3 / (dx*dx);

        }

}



Var* ff_interp2d(vfuncptr func, Var *arg)
{
  
  Var    *xdata = NULL;                /* the orignial data */
  Var    *ydata = NULL;                /* the orignial data */ 
  Var    *table = NULL;                /* look up table */
  Var    *out = NULL;                  /* the output struture */
  int     i,j,k;                       /* loop indices */
  float   p1, p2;                      /* percentages */
  int     xx,xy,xz,yx,yy,yz;           /* data size */
  float  *wdata = NULL;                /* working data */
  float   sx=1,dx=1,sy=1,dy=1;         /* start and delta values */
  float   tvx,tvy;                     /* data values */
  int     xi,yi;                       /* new x and y positions */
  float   tv1,tv2;                     /* temporary values */
  
  Alist alist[8];
  alist[0] = make_alist("table",     ID_VAL,    NULL,  &table);
  alist[1] = make_alist("xdata",     ID_VAL,    NULL,  &xdata);
  alist[2] = make_alist("ydata",     ID_VAL,    NULL,  &ydata);
  alist[3] = make_alist("startx",    FLOAT,     NULL,  &sx);
  alist[4] = make_alist("deltax",    FLOAT,     NULL,  &dx);
  alist[5] = make_alist("starty",    FLOAT,     NULL,  &sy);
  alist[6] = make_alist("deltay",    FLOAT,     NULL,  &dy);
  alist[7].name = NULL;
  
  if (parse_args(func, arg, alist) == 0) return(NULL);
  
  if (table == NULL) {
    parse_error("\ninterp2d()- Thu Apr 27 16:20:31 MST 2006");
    parse_error("Bilinear interpolation algorithm");
    parse_error("\nInputs and Outputs:");
    parse_error("table - table of values of a standard delta value for each axis");
    parse_error("xdata - the x data to interpolate");
    parse_error("ydata - the y data to interpolate");
    parse_error("startx - starting x value for the table");
    parse_error("deltax - delta  x value for the table");
    parse_error("starty - starting y value for the table");
    parse_error("deltay - delta y value for the table");
    parse_error("Returns a 1 d, array the size of x and y data\n");
    parse_error("c.edwards");
    return (NULL);
  }
    
  /*size of xdata*/
  xx = GetX(xdata);
  xy = GetY(xdata);
  xz = GetZ(xdata);

  /*size of ydata*/
  yx = GetX(ydata);
  yy = GetY(ydata);
  yz = GetZ(ydata);

  /*error handling, they must be the same size and one band*/
  if(xx!=yx || xy!=yy || xz!=1 || yz != 1) { 
    parse_error("\nThe x and y data must have the same dimensions and only one band\n");
    return NULL;
  }

  /*memory allocation*/
  wdata=(float *)calloc(sizeof(float),xx*xy*1);
  
  for(i=0;i<xx;i+=1) {
    for(j=0;j<xy;j+=1) {
      
      /*extract values from original data*/
      tvx=extract_float(xdata,cpos(i,j,0,xdata));
      tvy=extract_float(ydata,cpos(i,j,0,ydata));
      
      /*apply start and delta to the extracted values*/
      tvx=(tvx-sx)/dx;
      tvy=(tvy-sy)/dy;
      if(tvx<0) tvx=0;
      if(tvy<0) tvy=0;
      if(tvx>xx) tvx=xx-1;
      if(tvy>xy) tvy=xy-1;     
      
      /*calculate percentages */
      p1=(float)(tvx-floor(tvx));
      p2=(float)(tvy-floor(tvy));
      xi=(int)floor(tvx);
      yi=(int)floor(tvy);
      
      /*   apply the bilinear interpolation algorithm                  **
      **   val=(f(1,1)*(1-p1)+f(2,1)*p1)*(1-p2)+(f(1,2)*(1-p1)+f(2,2)*p1)*p2    **
      */

      tv1=(extract_float(table,cpos(xi,yi,0,table))*(1-p1)+extract_float(table,cpos(xi+1,yi,0,table))*(p1))*(1-p2);
      tv2=(extract_float(table,cpos(xi,yi+1,0,table))*(1-p1)+extract_float(table,cpos(xi+1,yi+1,0,table))*(p1))*(p2);
      wdata[xx*j + i]=(float)(tv1+tv2);
    }
  }
  out=newVal(BSQ, xx, xy, 1, FLOAT, wdata);
  return out;
}


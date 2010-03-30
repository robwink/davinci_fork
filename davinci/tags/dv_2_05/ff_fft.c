#include "parser.h"

#include "fft.h"
#include <math.h>

/**
 **/
void cdft(int n, double wr, double wi, double *a);
void rdft(int n, double wr, double wi, double *a);

int mayer_fht(double *fz, int n);
int mayer_ifft(int n, double *real, double *imag);
int mayer_realfft(int n, double *real);
int mayer_fft(int n, double *real, double *imag);
int mayer_realifft(int n, double *real);

Var *
ff_fft(vfuncptr func, Var * arg)
{
	Var *real = NULL, *img = NULL;
	double *data;
	int i, j, n, x, y, z;
	COMPLEX *in, *out;

	Alist alist[4];
	alist[0] = make_alist( "real",    ID_VAL,    NULL,     &real);
	alist[1] = make_alist( "img",    ID_VAL,    NULL,     &img);
	alist[2].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (real == NULL && img == NULL) {
		parse_error("%s: No real or imaginary objects specified\n", func->name);
		return(NULL);
	}
	x = GetSamples(V_SIZE(real), V_ORG(real));
	y = GetLines(V_SIZE(real), V_ORG(real));
	z = GetBands(V_SIZE(real), V_ORG(real));


	if (img == NULL && x == 2) {
		n = y*z;
		in = (COMPLEX *)calloc(n, sizeof(COMPLEX));
		out = (COMPLEX *)calloc(n, sizeof(COMPLEX));
		for (i = 0 ; i < y ; i++) {
			for (j = 0 ; j < z ; j++) {
				in[i].re = extract_double(real, cpos(0, i, j, real));
				in[i].im = extract_double(real, cpos(1, i, j, real));
			}
		}
	} else {
		n = V_DSIZE(real);
		in = (COMPLEX *)calloc(n, sizeof(COMPLEX));
		out = (COMPLEX *)calloc(n, sizeof(COMPLEX));
		for (i = 0 ; i < n ; i++) {
			in[i].re = extract_double(real, i);
			in[i].im = (img == NULL ? 0.0 : extract_double(img, i));
		}
	}

	if (func->fdata == (void *)1) {
		fft(in, n, out);
	} else {
		rft(in, n, out);
	}

	data = (double *)calloc(n*2, sizeof(double));

	for (i = 0 ; i < n ; i++) {
		data[i*2] = out[i].re;
		data[i*2+1] = out[i].im;
	}
	return(newVal(BSQ, 2, n, 1, DOUBLE, data));
}

Var *
ff_realfft(vfuncptr func, Var * arg)
{
	Var *obj = NULL;
	int i, n;
	double *in, *out;

	Alist alist[3];
	alist[0] = make_alist( "obj",    ID_VAL,    NULL,     &obj);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (obj == NULL) {
		parse_error("%s: No object specified\n", func->name);
		return(NULL);
	}

	n = V_DSIZE(obj);
	in = (double *)calloc(n, sizeof(double));
	out = (double *)calloc(n, sizeof(double));

	for (i = 0 ; i < n ; i++) {
		in[i] = extract_double(obj, i);
	}

	if (func->fdata == (void *)1) {
		realfft(in, n, out);
	} else {
		realrft(in, n, out);
	}
	return(newVal(BSQ, 1, n, 1, DOUBLE, out));
}


Var *
ff_realfft2(vfuncptr func, Var * arg)
{
	Var *obj = NULL;
	int i, j, n, x;
	double *in;

	Alist alist[3];
	alist[0] = make_alist( "obj",    ID_VAL,    NULL,     &obj);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (obj == NULL) {
		parse_error("%s: No object specified\n", func->name);
		return(NULL);
	}

	n = V_DSIZE(obj);
	for (x = n ; (x & 1) == 0 ; x >>= 1) 
		;
	if (x != 1) {
		parse_error("dimension not a power of 2. Use version 0.\n");
		return(NULL);
	}

	in = (double *)calloc(n, sizeof(double));

	for (i = 0 ; i < n ; i++) {
		in[i] = extract_double(obj, i);
	}

	if (func->fdata == (void *)1) {
		rdft(n, cos(M_PI/n), sin(M_PI/n), in);
	} else {
		rdft(n, cos(M_PI/n), -sin(M_PI/n), in);
		for (j = 0; j <= n - 1; j++) {
			in[j] *= 2.0 / n;
		}
	}

	return(newVal(BSQ, 1, n, 1, DOUBLE, in));
}



/**  fft(n,real,imag)
**      Does a fourier transform of "n" points of the "real" and
**      "imag" arrays.
**  ifft(n,real,imag)
**      Does an inverse fourier transform of "n" points of the "real"
**      and "imag" arrays.
**  realfft(n,real)
**      Does a real-valued fourier transform of "n" points of the
**      "real" and "imag" arrays.  The real part of the transform ends
**      up in the first half of the array and the imaginary part of the
**      transform ends up in the second half of the array.
**  realifft(n,real)
**      The inverse of the realfft() routine above.
**/


Var *
ff_realfft3(vfuncptr func, Var * arg)
{
  Var *obj = NULL;
  size_t i, n;
  double *in;

  Alist alist[3];
  alist[0] = make_alist( "obj",    ID_VAL,    NULL,     &obj);
  alist[1].name = NULL;

  if (parse_args(func, arg, alist) == 0) return(NULL);

  if (obj == NULL) {
    parse_error("%s: No object specified\n", func->name);
    return(NULL);
  }

  n = V_DSIZE(obj);
  if (n > INT_MAX){
  	parse_error("%s: fft function does not handle objects greater than %ld bytes.\n", func->name, INT_MAX);
  	return NULL;
  }

  in = (double *)calloc(n, sizeof(double));
  if (in == NULL){
  	parse_error("%s: Unable to alloc %ld bytes.\n", func->name, n*sizeof(double));
  	return NULL;
  }

  for (i = 0 ; i < n ; i++) {
    in[i] = extract_double(obj, i);
  }

  if (func->fdata == (void *)1) {
    mayer_realfft(n, in);
  } else {
    mayer_realifft(n, in);
  }

  return(newVal(BSQ, 1, n, 1, DOUBLE, in));
}

/* $Id$
 *
 * Jim Stewart
 * 31 Aug 2004
 *
 * Perform a Gaussian convolution.
 *
 * This function is based on the recursive linear Gaussian filter algorithm
 * designed by Lucas J. van Vliet et. al. and documented at:
 * http://www.ph.tn.tudelft.nl/Courses/FIP/noframes/fip-Smoothin.html.
 *
 * It bears strong resemblance to Luigi Rosa's MATLAB implementation of the
 * same, which can be found at http://utenti.lycos.it/matlab.  Although this
 * work is primarily original, Luigi's implementation was referred to heavily
 * after the fact for confirmation and some of his ideas made their way into
 * this code.
 *
 * This version is also much more memory efficient due to re-use of buffers
 * on a per-line basis during the forward/backward difference equation
 * calculations, although there's a slight time penalty for the extra copy
 * stage to move the buffers to the output object (which could probably be
 * eliminated with some more tweaking at the end of the "columns" pass).
 *
 */

#include "parser.h"

#define DEFAULT_SIGMA 3.0

Var *ff_gconvolve(vfuncptr func, Var * arg)
{
    /* Function args. */

    Var *obj = NULL;
    float sigma = DEFAULT_SIGMA;
    Var *out = NULL;

    /* Misc iterators and stuff. */

    unsigned int dsize, x, y, z;
    int i, j, band, pos;

    float *data;                 /* Output data. */
    float *w, *c;               /* Temp buffers for Gaussian algorithm. */
    unsigned int wc_size;       /* Size of temp buffers. */

    /* These variables correspond to the van Vliet algorithm variables. */

    float q, qq, qqq, b0, b1, b2, b3, B, an, wn;
	float a;

    /* Setup input args. */

    Alist alist[3];

    alist[0] = make_alist("obj", ID_VAL, NULL, &obj);
    alist[1] = make_alist("sigma", FLOAT, NULL, &sigma);
    alist[2].name = NULL;

    /* Parse & validate input args. */

    if (parse_args(func, arg, alist) == 0)
        return NULL;

    if (obj == NULL) {
        parse_error("%s: no obj specified\n", func->name);
        return NULL;
    }

    if (sigma < 0.5) {
        parse_error("%s: sigma must be >= 0.5\n", func->name);
        return NULL;
    }

    dsize = V_DSIZE(obj);
    x = GetX(obj);
    y = GetY(obj);
    z = GetZ(obj);

    data = (float *) malloc(sizeof(float) * dsize);
    if (data == NULL) {
        parse_error("%s: unable to allocate %u bytes for output\n",
                    func->name, dsize);
        return NULL;
    }

    /* Compute gq based on sigma. */

    if (sigma >= 2.5) {
        q = (0.98711 * sigma) - 0.96330;
    } else {
        q = 3.97156 - (4.14554 * sqrt((1 - (0.26891 * sigma))));
    }
    qq = q * q;
    qqq = q * q * q;

    /* Compute filter coefficients based on gq. */

    b0 = 1.57825 + (2.44413 * q) + (1.4281 * qq) + (0.422205 * qqq);
    b1 = ((2.44413 * q) + (2.85619 * qq) + (1.26661 * qqq)) /b0;
    b2 = (-(1.4281 * qq) - (1.26661 * qqq)) /b0;
    b3 = (0.422205 * qqq) /b0;
    B = 1.0 - (b1 + b2 + b3);

    /* Allocate reusable row/column buffers. */

    wc_size = max(x, y);
    w = (float *) malloc(sizeof(float) * wc_size);

    if (w == NULL) {
        parse_error
            ("%s: unable to allocate %u bytes for row/column buffer\n",
             func->name, wc_size);
        free(out);
        return NULL;
    }

    c = (float *) malloc(sizeof(float) * wc_size);

    if (c == NULL) {
        parse_error
            ("%s: unable to allocate %u bytes for row/column buffer\n",
             func->name, wc_size);
        free(out);
        free(w);
        return NULL;
    }

    /* Iterate over each band.. */

    for (band = 0; band < z; band++) {
        for (j = 0; j < y; j++) {				 /* Do rows. */
            for (i = 0; i < x; i++) {

                /* Handle forward difference equation. */
                /* w[n] = B*a[n] + (b1*w[n-1] + b2*w[n-2] + b3*w[n-3])/b0 */
                /* Handle the "borders" by assuming 0 for out-of-range values. */
                pos = cpos(i, j, band, obj);
                a = extract_float(obj, pos);

                w[i] = B * a;
                if (i > 0) w[i] += b1 * w[i - 1];
                if (i > 1) w[i] += b2 * w[i - 2];
                if (i > 2) w[i] += b3 * w[i - 3];
            }

            for (i = x - 1; i >= 0; i--) {
                /* Handle backward difference equation. */
                /* c[n] = B*w[n] + (b1*c[n+1] + b2*c[n+2] + b3*c[n+3])/b0 */
                /* Handle the "borders" by assuming 0 for out-of-range values. */
                c[i] = B * w[i];
                if (i < x - 1) c[i] += b1 * c[i + 1];
                if (i < x - 2) c[i] += b2 * c[i + 2];
                if (i < x - 3) c[i] += b3 * c[i + 3];
			}
			/* Copy c values into out buffer. */
			for (i = 0; i < x; i++) {
				pos = cpos(i, j, band, obj);
				data[pos] = c[i];
			}
		}                   /* row */
		if (y > 1) {
			for (i = 0; i < x; i++) {
				for (j = 0; j < y; j++) {				 /* Do cols. */

					/* Handle forward difference equation. */
					/* w[n] = B*a[n] + (b1*w[n-1] + b2*w[n-2] + b3*w[n-3])/b0 */
					/* Handle the "borders" by assuming 0 for out-of-range values. */
					pos = cpos(i, j, band, obj);
					a = data[pos];

					w[j] = B * a;
					if (j > 0) w[j] += b1 * w[j - 1];
					if (j > 1) w[j] += b2 * w[j - 2];
					if (j > 2) w[j] += b3 * w[j - 3];
				}

				for (j = y - 1; j >= 0; j--) {
					/* Handle backward difference equation. */
					/* c[n] = B*w[n] + (b1*c[n+1] + b2*c[n+2] + b3*c[n+3])/b0 */
					/* Handle the "borders" by assuming 0 for out-of-range values. */
					c[j] = B * w[j];
					if (j < y - 1) c[j] += b1 * c[j + 1];
					if (j < y - 2) c[j] += b2 * c[j + 2];
					if (j < y - 3) c[j] += b3 * c[j + 3];
				}
				/* Copy c values into out buffer. */
				for (j = 0; j < y; j++) {
					pos = cpos(i, j, band, obj);
					data[pos] = c[j];
				}
			}                   /* col */
		}
	}

	/* Done with convolution. */
	/* Free temporary buffers. */

	free(w);
	free(c);
	/* Construct return value. */
	out = newVal(V_ORG(obj), 
					V_SIZE(obj)[0],
					V_SIZE(obj)[1],
					V_SIZE(obj)[2],
					FLOAT, data);
	return (out);
}

/*
 * "ft.c", Pjotr '87.
 */

#include <stdlib.h>
#include	<math.h>
#include	"fft.h"

static unsigned radix (unsigned n);
static void split (COMPLEX *in, unsigned r, unsigned m, COMPLEX *out);
static void join (COMPLEX *in, unsigned m, unsigned n, COMPLEX *out);
int W_init (unsigned n);

/*
 * Forward Fast Fourier Transform on the n samples of complex array in.
 * The result is placed in out.  The number of samples, n, is arbitrary.
 * The W-factors are calculated in advance.
 */
int fft (COMPLEX *in, unsigned n, COMPLEX *out)
{
	unsigned i;

	for (i = 0; i < n; i++)
		c_conj (in [i]);
	
	if (W_init (n) == -1)
		return -1;

	Fourier (in, n, out);

	for (i = 0; i < n; i++) {
		c_conj (out [i]);
		c_realdiv (out [i], n);
	}

	return 0;
}

/*
 * Reverse Fast Fourier Transform on the n complex samples of array in.
 * The result is placed in out.  The number of samples, n, is arbitrary.
 * The W-factors are calculated in advance.
 */
int rft (COMPLEX *in, unsigned n, COMPLEX *out)
{
	if (W_init (n) == -1)
		return -1;

	Fourier (in, n, out);

	return 0;
}

/*
 * Reele forward fast fourier transform van n samples van in naar
 * amplitudes van out.
 * De cosinus komponent van de dc komt in out [0], dan volgen in
 * out [2 * i - 1] en out [2 * i] steeds resp. de cosinus en sinus
 * komponenten van de i-de harmonische.  Bij een even aantal samples
 * bevat out [n - 1] de cosinus komponent van de Nyquist frequentie. 
 * Extraatje: Na afloop is in onaangetast.
 */
int
realfft (double *in, unsigned n, double *out)
{
	COMPLEX *c_in, *c_out;
	unsigned i;

	if (n == 0 ||
	    (c_in = (COMPLEX *) malloc (n * sizeof (COMPLEX))) == 0 ||
	    (c_out = (COMPLEX *) malloc (n * sizeof (COMPLEX))) == 0)
		return 0;
	
	for (i = 0; i < n; i++) {
		c_re (c_in [i]) = in [i];
		c_im (c_in [i]) = 0;
	}

	fft (c_in, n, c_out);

	out [0] = c_re (c_out [0]);		/* cos van dc */
	for (i = 1; i < (n + 1) / 2; i++) {	/* cos/sin i-de harmonische */
		out [2 * i - 1] = c_re (c_out [i]) * 2;
		out [2 * i] = c_im (c_out [i]) * -2;
	}
	if (n % 2 == 0)				/* cos van Nyquist */
		out [n - 1] = c_re (c_out [n / 2]);

	free ((char *) c_in);
	free ((char *) c_out);
        return 0;
}

/*
 * Reele reverse fast fourier transform van amplitudes van in naar
 * n samples van out.
 * De cosinus komponent van de dc staat in in [0], dan volgen in
 * in [2 * i - 1] en in [2 * i] steeds resp. de cosinus en sinus
 * komponenten van de i-de harmonische.  Bij een even aantal samples
 * bevat in [n - 1] de cosinus komponent van de Nyquist frequentie. 
 * Extraatje: Na afloop is in onaangetast.
 */
int
realrft (double *in, unsigned n, double *out)
{
	COMPLEX *c_in, *c_out;
	unsigned i;

	if (n == 0 ||
	    (c_in = (COMPLEX *) malloc (n * sizeof (COMPLEX))) == 0 ||
	    (c_out = (COMPLEX *) malloc (n * sizeof (COMPLEX))) == 0)
		return 0;
	
	c_re (c_in [0]) = in [0];		/* dc */
	c_im (c_in [0]) = 0;
	for (i = 1; i < (n + 1) / 2; i++) {	/* geconj. symm. harmonischen */
		c_re (c_in [i]) = in [2 * i - 1] / 2;
		c_im (c_in [i]) = in [2 * i] / -2;
		c_re (c_in [n - i]) = in [2 * i - 1] / 2;
		c_im (c_in [n - i]) = in [2 * i] / 2;
	}
	if (n % 2 == 0) {			/* Nyquist */
		c_re (c_in [n / 2]) = in [n - 1];
		c_im (c_in [n / 2]) = 0;
	}

	rft (c_in, n, c_out);

	for (i = 0; i < n; i++)
		out [i] = c_re (c_out [i]);

	free ((char *) c_in);
	free ((char *) c_out);

	return(1);
}

/*
 * Recursive (reverse) complex fast Fourier transform on the n
 * complex samples of array in, with the Cooley-Tukey method.
 * The result is placed in out.  The number of samples, n, is arbitrary.
 * The algorithm costs O (n * (r1 + .. + rk)), where k is the number
 * of factors in the prime-decomposition of n (also the maximum
 * depth of the recursion), and ri is the i-th primefactor.
 */
int
Fourier (COMPLEX *in, unsigned n, COMPLEX *out)
{
	unsigned r;

	if ((r = radix (n)) < n)
		split (in, r, n / r, out);
	join (in, n / r, n, out);
        return 0;
}

/*
 * Give smallest possible radix for n samples.
 * Determines (in a rude way) the smallest primefactor of n.
 */
static unsigned radix (unsigned n)
{
	unsigned r;

	if (n < 2)
		return 1;

	for (r = 2; r < n; r++)
		if (n % r == 0)
			break;
	return r;
}

/*
 * Split array in of r * m samples in r parts of each m samples,
 * such that in [i] goes to out [(i % r) * m + (i / r)].
 * Then call for each part of out Fourier, so the r recursively
 * transformed parts will go back to in.
 */
static void split (COMPLEX *in, unsigned r, unsigned m, COMPLEX *out)
{
	register unsigned k, s, i, j;

	for (k = 0, j = 0; k < r; k++)
		for (s = 0, i = k; s < m; s++, i += r, j++)
			out [j] = in [i];

	for (k = 0; k < r; k++, out += m, in += m)
		Fourier (out, m, in);
}

/*
 * Sum the n / m parts of each m samples of in to n samples in out.
 * 		   r - 1
 * Out [j] becomes  sum  in [j % m] * W (j * k).  Here in is the k-th
 * 		   k = 0   k	       n		 k
 * part of in (indices k * m ... (k + 1) * m - 1), and r is the radix.
 * For k = 0, a complex multiplication with W (0) is avoided.
 */
static void join (COMPLEX *in, unsigned m, unsigned n, COMPLEX *out)
{
	register unsigned i, j, jk, s;

	for (s = 0; s < m; s++)
		for (j = s; j < n; j += m) {
			out [j] = in [s];
			for (i = s + m, jk = j; i < n; i += m, jk += j)
				c_add_mul (out [j], in [i], W (n, jk));
		}
}


COMPLEX *W_factors = 0;		/* array of W-factors */
unsigned Nfactors = 0;		/* number of entries in W-factors */

/*
 * W_init puts Wn ^ k (= e ^ (2pi * i * k / n)) in W_factors [k], 0 <= k < n.
 * If n is equal to Nfactors then nothing is done, so the same W_factors
 * array can used for several transforms of the same number of samples.
 * Notice the explicit calculation of sines and cosines, an iterative approach
 * introduces substantial errors.
 */
int W_init (unsigned n)
{
#	define pi	3.1415926535897932384626434
	unsigned k;

	if (n == Nfactors)
		return 0;
	if (Nfactors != 0 && W_factors != 0)
		free ((char *) W_factors);
	if ((Nfactors = n) == 0)
		return 0;
	if ((W_factors = (COMPLEX *) malloc (n * sizeof (COMPLEX))) == 0)
		return -1;

	for (k = 0; k < n; k++) {
		c_re (W_factors [k]) = cos (2 * pi * k / n);
		c_im (W_factors [k]) = sin (2 * pi * k / n);
	}

	return 0;
}

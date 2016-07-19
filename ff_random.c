#include "parser.h"
#include <stdlib.h>

/**
 ** This function generates a cube of noise.
 ** The user specifies the X, Y and Z dimensions of the cube.
 ** The noise is generated using a normal distribution function, which
 ** produces a standard deviation of 0.5
 **/

float g_random();
void g_srandom(int);

Var* ff_random(vfuncptr func, Var* arg)
{
	int x = 1, y = 1, z = 1, seed = INT_MAX;
	float* fdata;
	size_t dsize;
	size_t i;
	char* ptr = NULL;

	const char* options[] = {"normal",  "gaussian", "rand",   "random", "mrand48",
	                         "drand48", "uniform",  "rnoise", NULL};
	Var* seedvar = NULL;
	Alist alist[6];
	alist[0]      = make_alist("x", DV_INT32, NULL, &x);
	alist[1]      = make_alist("y", DV_INT32, NULL, &y);
	alist[2]      = make_alist("z", DV_INT32, NULL, &z);
	alist[3]      = make_alist("seed", ID_VAL, NULL, &seedvar);
	alist[4]      = make_alist("type", ID_ENUM, options, &ptr);
	alist[5].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (func->fdata != NULL) {
		if (ptr == NULL) ptr = (char*)func->fdata;
	}

	if (x <= 0) {
		parse_error("%s(): Invalid value for \"x\"", func->name);
		return (NULL);
	}
	if (y <= 0) {
		parse_error("%s(): Invalid value for \"y\"", func->name);
		return (NULL);
	}
	if (z <= 0) {
		parse_error("%s(): Invalid value for \"z\"", func->name);
		return (NULL);
	}

	if (seedvar != NULL) seed = extract_int(seedvar, 0);

	dsize = (size_t)x * (size_t)y * (size_t)z;
	fdata = (float*)calloc(dsize, sizeof(float));

	if (ptr == NULL || !strcasecmp(ptr, "uniform") || !strcasecmp(ptr, "drand48")) {
		if (seedvar != NULL) srand48(seed);
		for (i = 0; i < dsize; i++) {
			fdata[i] = drand48();
		}
	} else if (!strcasecmp(ptr, "rand")) {
		if (seedvar != NULL) srand(seed);
		for (i = 0; i < dsize; i++) {
			fdata[i] = rand();
		}
	} else if (!strcasecmp(ptr, "random")) {
		if (seedvar != NULL) srandom(seed);
		for (i = 0; i < dsize; i++) {
			fdata[i] = random();
		}
	} else if (!strcasecmp(ptr, "mrand48")) {
		if (seedvar != NULL) srand48(seed);
		for (i = 0; i < dsize; i++) {
			fdata[i] = mrand48();
		}
	} else if (!strcasecmp(ptr, "normal") || !strncasecmp(ptr, "gauss", 5) ||
	           !strcasecmp(ptr, "rnoise")) {
		if (seedvar != NULL) g_srandom(seed);
		for (i = 0; i < dsize; i++) {
			fdata[i] = g_random();
		}
	}
	return (newVal(BSQ, x, y, z, DV_FLOAT, fdata));
}

void g_srandom(int seed)
{
	srand48(seed);
}

float g_random()
{
	static int iset = 0;
	static float gset;
	float r = 3, fac, v1, v2;

	if (iset == 0) {
		iset = 1;
		while (r >= 1) {
			v1 = 2.0 * drand48() - 1.0;
			v2 = 2.0 * drand48() - 1.0;
			r  = v1 * v1 + v2 * v2;
		}
		fac  = sqrt(-2.0 * log(r) / r);
		gset = v1 * fac;
		return (v2 * fac);
	} else {
		iset = 0;
		return (gset);
	}
}

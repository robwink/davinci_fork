#include "parser.h"

/**
 ** gnoise(x=N,y=N,z=N,seed=N)
 **
 ** generate a cube of random noise.  Each plane of the cube is mutually
 ** exclusive to all other planes.  A value of Z=2 produces 2 bands
 ** of 50% noise each.
 **
 ** This function was written for Gregg Swayze on 1/11/95.
 **/

Var *
ff_gnoise(vfuncptr func, Var * arg)
{
	Var *v, *s;
	int x, y, z;
	int seed = 0;
	int i, j, d;

	struct keywords kw[] =
	{
		{"x", NULL},
		{"y", NULL},
		{"z", NULL},
		{"seed", NULL},
		{NULL, NULL}
	};

	if (evaluate_keywords(func, arg, kw)) {
		return (NULL);
	}
	if ((v = get_kw("x", kw)) == NULL) {
		x = 512;
	} else {
		if (V_TYPE(v) != ID_VAL || V_DSIZE(v) != 1 || V_FORMAT(v) != INT) {
			sprintf(error_buf, "Illegal value: %s(...x=...)", func->name);
			parse_error(NULL);
			return (NULL);
		}
		x = V_INT(v);
	}

	if ((v = get_kw("y", kw)) == NULL) {
		y = 512;
	} else {
		if (V_TYPE(v) != ID_VAL || V_DSIZE(v) != 1 || V_FORMAT(v) != INT) {
			sprintf(error_buf, "Illegal value: %s(...y=...)", func->name);
			parse_error(NULL);
			return (NULL);
		}
		y = V_INT(v);
	}

	if ((v = get_kw("z", kw)) == NULL) {
		z = 10;
	} else {
		if (V_TYPE(v) != ID_VAL || V_DSIZE(v) != 1 || V_FORMAT(v) != INT) {
			sprintf(error_buf, "Illegal value: %s(...z=...)", func->name);
			parse_error(NULL);
			return (NULL);
		}
		z = V_INT(v);
	}
	if ((v = get_kw("seed", kw)) == NULL) {
		seed = 0;
	} else {
		if (V_TYPE(v) != ID_VAL || V_DSIZE(v) != 1 || V_FORMAT(v) != INT) {
			sprintf(error_buf, "Illegal value: %s(...seed=...)", func->name);
			parse_error(NULL);
			return (NULL);
		}
		seed = V_INT(v);
	}

	s = newVar();
	V_TYPE(s) = ID_VAL;

	V_DATA(s) = calloc(1, x * y * z);
	V_DSIZE(s) = x * y * z;
	V_SIZE(s)[0] = x;
	V_SIZE(s)[1] = y;
	V_SIZE(s)[2] = z;
	V_ORG(s) = BSQ;
	V_FORMAT(s) = BYTE;

	if (seed == 0)
		seed = time(0) * getpid();
	srand48(seed);

	for (j = 0; j < y; j++) {
		for (i = 0; i < x; i++) {
			d = (int) ((float)z *drand48());
			((unsigned char *) V_DATA(s))[(d * y + j) * x + i] = 255;
		}
	}
	return (s);
}

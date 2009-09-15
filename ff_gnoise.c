#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif /* _WIN32 */
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
	Var *s;
	int x=512, y=512, z=10;
	int seed = 0;
	int i, j, d;

    Alist alist[2];
    alist[0] = make_alist( "x",    INT,    NULL,    &x);
    alist[1] = make_alist( "y",    INT,    NULL,    &y);
    alist[2] = make_alist( "z",    INT,    NULL,    &z);
    alist[3] = make_alist( "seed", INT,    NULL,    &seed);
    alist[1].name = NULL;

    if (parse_args(func, arg, alist) == 0) return(NULL);

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

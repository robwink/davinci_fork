#include "parser.h"

int 
cmp_byte(u_char * a, u_char * b)
{
	if (*a > *b) return(1);
	if (*a < *b) return(-1);
	return(0);
}
int 
cmp_short(short *a, short *b)
{
	if (*a > *b) return(1);
	if (*a < *b) return(-1);
	return(0);
}
int 
cmp_int(int *a, int *b)
{
	if (*a > *b) return(1);
	if (*a < *b) return(-1);
	return(0);
}
int 
cmp_float(float *a, float *b)
{
	if (*a > *b) return(1);
	if (*a < *b) return(-1);
	return(0);
}
int 
cmp_double(double *a, double *b)
{
	if (*a > *b) return(1);
	if (*a < *b) return(-1);
	return(0);
}

Var *
ff_sort(vfuncptr func, Var * arg)
{
	Var *value = NULL, *v;
	int ac;
	Var **av;
	int format, dsize;
	void *data;
	int (*cmp)();

	Alist alist[2];
	alist[0] = make_alist( "object",    ID_VAL,    NULL,     &value, 0);
	alist[1].name = NULL;

	make_args(&ac, &av, func, arg);
	if (parse_args(ac, av, alist)) return(NULL);

	if (value == NULL)  {
		return(NULL);
	}

	format = V_FORMAT(value);
	dsize = V_DSIZE(value);

	data = calloc(dsize, NBYTES(format));
	memcpy(data, V_DATA(value), dsize*NBYTES(format));

	v = newVal(V_ORG(value), 
		V_SIZE(value)[0], V_SIZE(value)[1], V_SIZE(value)[2], 
		format, data);

	switch(format) {
	    case BYTE:		cmp = cmp_byte; break;
	    case SHORT:		cmp = cmp_short; break;
	    case INT:		cmp = cmp_int; break;
	    case FLOAT:		cmp = cmp_float; break;
	    case DOUBLE:	cmp = cmp_double; break;
	}
	qsort(data, V_DSIZE(v), NBYTES(format), cmp);
	return(v);
}

#include "parser.h"

#define XAXIS	1
#define YAXIS	2
#define ZAXIS	4

Var *
ff_avg(vfuncptr func, Var * arg)
{
	Var *obj=NULL, *v;
	char *ptr = NULL;
	int axis = 0, dsize, i, j;
	int in[3], out[3];
	float *fdata, *vx;
	char *options[] =  {
		"x", "y", "z", "xy", "yx", "xz", "zx", "yz", "zy",
		"xyz", "xzy", "yxz", "yzx", "zxy", "zyx", NULL
	};
	int dsize2;
	float f;

	int ac;
	Var **av;
	Alist alist[3];
	alist[0] = make_alist("object",		ID_VAL,		NULL,	&obj);
	alist[1] = make_alist("axis",  		ID_ENUM,	options,	&ptr);
	alist[2].name = NULL;

	make_args(&ac, &av, func, arg);
	if (parse_args(ac, av, alist)) return(NULL);

	if (ptr == NULL) {
		axis = XAXIS | YAXIS | ZAXIS;		/* all of them */
	} else {
		if (strchr(ptr, 'x') || strchr(ptr, 'X')) axis |= XAXIS;
		if (strchr(ptr, 'y') || strchr(ptr, 'Y')) axis |= YAXIS;
		if (strchr(ptr, 'z') || strchr(ptr, 'Z')) axis |= ZAXIS;
	}

	if (obj == NULL) {
		parse_error("%s: No object specified\n", func->name);
		return(NULL);
	}

	dsize = V_DSIZE(obj);
	for (i = 0 ; i < 3 ; i++) {
		in[i] = out[i] = V_SIZE(obj)[i];
	}

	if (axis & XAXIS) out[orders[V_ORG(obj)][0]] = 1;
	if (axis & YAXIS) out[orders[V_ORG(obj)][1]] = 1;
	if (axis & ZAXIS) out[orders[V_ORG(obj)][2]] = 1;


	v = newVal(V_ORG(obj), out[0], out[1], out[2], FLOAT, NULL);
	fdata = V_DATA(v) = calloc(V_DSIZE(v), sizeof(float));

	for (i = 0 ; i < dsize ; i++) {
		fdata[rpos(i, obj, v)] += extract_float(obj, i);
	}
	dsize2 = V_DSIZE(v);
	f = (float)dsize2 / (float)dsize;


	if (!strcmp(func->name, "avg") || !strcmp(func->name, "stddev")) {
		/**
		 ** divide for avg.
		 **/
		for (i = 0 ; i < dsize2 ; i++) {
			fdata[i] *= f;
		}
	}

	if (!strcmp(func->name, "stddev")) {
		vx = calloc(dsize2, sizeof(float));
		for (i = 0 ; i < dsize ; i++) {
			j = rpos(i, obj, v);
			f = extract_float(obj, i) - fdata[j];
			vx[j] += f*f;	
		}
		f = 1 / (((float)dsize / (float)dsize2) - 1);
		for (i = 0 ; i < dsize2 ; i++) {
			vx[i]=sqrt(vx[i]*f);
		}
		free(fdata);
		V_DATA(v) = vx;
	}

	return(v);
}


Var *
ff_min(vfuncptr func, Var * arg)
{
	Var *obj=NULL, *v;
	char *ptr = NULL;
	int axis = 0, dsize, dsize2, i, j;
	int in[3], out[3];
	float *fdata, x;
	void *data;
	int do_min = 0, do_max = 0;
	char *options[] =  {
		"x", "y", "z", "xy", "yx", "xz", "zx", "yz", "zy",
		"xyz", "xzy", "yxz", "yzx", "zxy", "zyx", NULL
	};

	int ac;
	Var **av;
	Alist alist[3];
	alist[0] = make_alist("object",		ID_VAL,		NULL,	&obj);
	alist[1] = make_alist("axis",  		ID_ENUM,	options,	&ptr);
	alist[2].name = NULL;

	make_args(&ac, &av, func, arg);
	if (parse_args(ac, av, alist)) return(NULL);

	if (ptr == NULL) {
		axis = XAXIS | YAXIS | ZAXIS;		/* all of them */
	} else {
		if (strchr(ptr, 'x') || strchr(ptr, 'X')) axis |= XAXIS;
		if (strchr(ptr, 'y') || strchr(ptr, 'Y')) axis |= YAXIS;
		if (strchr(ptr, 'z') || strchr(ptr, 'Z')) axis |= ZAXIS;
	}

	if (obj == NULL) {
		parse_error("%s: No object specified\n", func->name);
		return(NULL);
	}

	if (!strcmp(func->name, "min")) do_min = 1;
	if (!strcmp(func->name, "max")) do_max = 1;

	dsize = V_DSIZE(obj);
	for (i = 0 ; i < 3 ; i++) {
		in[i] = out[i] = V_SIZE(obj)[i];
	}

	if (axis & XAXIS) out[orders[V_ORG(obj)][0]] = 1;
	if (axis & YAXIS) out[orders[V_ORG(obj)][1]] = 1;
	if (axis & ZAXIS) out[orders[V_ORG(obj)][2]] = 1;

	v = newVal(V_ORG(obj), out[0], out[1], out[2], FLOAT, NULL);
	dsize2 = V_DSIZE(v);

	fdata = V_DATA(v) = calloc(dsize2, sizeof(float));
	data = V_DATA(obj);


	/**
	 ** Fill in defaults
	 **/
	for (i = 0 ; i < dsize2 ; i++) {
		fdata[i] = extract_float(obj, rpos(i,v,obj));
	}

	for (i = 0 ; i < dsize ; i++) {
		j = rpos(i, obj, v);
		x = extract_float(obj, i);
		if (do_min && x < fdata[j]) fdata[j] = x;
		if (do_max && x > fdata[j]) fdata[j] = x;
	}

	return(v);
}

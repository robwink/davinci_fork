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

Var *
ff_findmin(vfuncptr func, Var * arg)
{
	Var *obj=NULL, *v;
	int dsize, i;
	float x, val;
	int do_min = 0, do_max = 0;
	int pos;

	int ac;
	Var **av;
	Alist alist[2];
	alist[0] = make_alist("object",		ID_VAL,		NULL,	&obj);
	alist[1].name = NULL;

	make_args(&ac, &av, func, arg);
	if (parse_args(ac, av, alist)) return(NULL);

	if (obj == NULL) {
		parse_error("%s: No object specified\n", func->name);
		return(NULL);
	}

	if (!strcmp(func->name, "minchan")) do_min = 1;
	if (!strcmp(func->name, "maxchan")) do_max = 1;

	dsize = V_DSIZE(obj);

	val = extract_float(obj, 0);
	pos = 0;
	for (i = 1 ; i < dsize ; i++) {
		x = extract_float(obj, i);
		if (do_min && x < val) { val = x ; pos = i; }
		if (do_max && x > val) { val = x ; pos = i; }
	}

	v = newVal(BSQ, 1, 1, 1, INT, NULL);
	V_DATA(v) = calloc(1, sizeof(int));
	V_INT(v) = pos+1;

	return(v);
}


Var *
ff_convolve(vfuncptr func, Var * arg)
{
	Var *obj=NULL, *kernel=NULL, *v;
	int norm=1;
	float *data, val;
	int *wt;

	int dsize, i, j, k;
	int a, b, c;
	int x_pos, y_pos, z_pos;
	int obj_x, obj_y, obj_z;
	int kernel_x_center, kernel_x;
	int kernel_y_center, kernel_y;
	int kernel_z_center, kernel_z;
	int x,y,z;

	int ac;
	Var **av;
	Alist alist[4];
	alist[0] = make_alist("object",		ID_VAL,		NULL,	&obj);
	alist[1] = make_alist("kernel",  	ID_VAL,		NULL,	&kernel);
	alist[2] = make_alist("normalize", 	INT, NULL, &norm);
	alist[3].name = NULL;

	make_args(&ac, &av, func, arg);
	if (parse_args(ac, av, alist)) return(NULL);

	if (obj == NULL) {
		parse_error("%s: No object specified\n", func->name);
		return(NULL);
	}
	if (kernel == NULL) {
		parse_error("%s: No kernel specified\n", func->name);
		return(NULL);
	}

	obj_x = GetSamples(V_SIZE(obj), V_ORG(obj));
	obj_y = GetLines(V_SIZE(obj), V_ORG(obj));
	obj_z = GetBands(V_SIZE(obj), V_ORG(obj));

	kernel_x = GetSamples(V_SIZE(kernel), V_ORG(kernel));
	kernel_y = GetLines(V_SIZE(kernel), V_ORG(kernel));
	kernel_z = GetBands(V_SIZE(kernel), V_ORG(kernel));

	kernel_x_center = kernel_x/2;
	kernel_y_center = kernel_y/2;
	kernel_z_center = kernel_z/2;

	dsize = V_DSIZE(obj);
	if ((data = calloc(dsize, sizeof(float))) == NULL) {
		parse_error("Unable to allocate memory");
		return(NULL);
	}
	if ((wt = calloc(dsize, sizeof(int))) == NULL) {
		parse_error("Unable to allocate memory");
		return(NULL);
	}

	for (i = 0 ; i < dsize ; i++) {
		xpos(i, obj,&x, &y, &z);		/* compute current x,y,z */
		fprintf(stderr, "Convolve: %d %d %d\r", x, y,z);

		for (a = 0 ; a < kernel_x ; a++) {
			x_pos = x + a - kernel_x_center;
			if (x_pos < 0 || x_pos >= obj_x) continue;
			for (b = 0 ; b < kernel_y ; b++) {
				y_pos = y + b - kernel_y_center;
				if (y_pos < 0 || y_pos >= obj_y) continue;
				for (c = 0 ; c < kernel_z ; c++) {
					z_pos = z + c - kernel_z_center;
					if (z_pos < 0 || z_pos >= obj_z) continue;

					j = cpos(x_pos, y_pos, z_pos, obj);
					k = cpos(a, b, c, kernel);
					val = extract_float(kernel,k);
					wt[i]++;
					data[i] += val * extract_float(obj, j);
				}
			}
		}
		if (norm) data[i] /= (float)wt[i];
	}
	return(newVal(V_ORG(obj), 
		V_SIZE(obj)[0],
		V_SIZE(obj)[1],
		V_SIZE(obj)[2],
		FLOAT, 
		data));
}


Var *
ff_convolve2(vfuncptr func, Var * arg)
{
	Var *obj=NULL, *kernel=NULL, *v;
	int norm=1;

	int dsize, i, j, k;
	int a, b, c;
	int x, y, z;
	int kernel_x_center, kernel_x;
	int kernel_y_center, kernel_y;
	int kernel_z_center, kernel_z;
	int *kpos;
	float *data;

	int ac;
	Var **av;
	Alist alist[4];
	alist[0] = make_alist("object",		ID_VAL,		NULL,	&obj);
	alist[1] = make_alist("kernel",  	ID_VAL,		NULL,	&kernel);
	alist[2] = make_alist("normalize", 	INT, NULL, &norm);
	alist[3].name = NULL;

	make_args(&ac, &av, func, arg);
	if (parse_args(ac, av, alist)) return(NULL);

	if (obj == NULL) {
		parse_error("%s: No object specified\n", func->name);
		return(NULL);
	}
	if (kernel == NULL) {
		parse_error("%s: No kernel specified\n", func->name);
		return(NULL);
	}

	kernel_x = GetSamples(V_SIZE(kernel), V_ORG(kernel));
	kernel_y = GetLines(V_SIZE(kernel), V_ORG(kernel));
	kernel_z = GetBands(V_SIZE(kernel), V_ORG(kernel));

	kernel_x_center = kernel_x/2;
	kernel_y_center = kernel_y/2;
	kernel_z_center = kernel_z/2;

	dsize = V_DSIZE(obj);
	data = calloc(dsize, sizeof(float));
	kpos = calloc(V_DSIZE(kernel), sizeof(int));

	/**
	 ** compute initial offsets.
	 **/
	for (i = 0 ; i < V_DSIZE(kernel) ; i++) {
		xpos(i, &x, &y, &z, kernel);
		x -= kernel_x_center;
		y -= kernel_y_center;
		z -= kernel_z_center;
		k = cpos(x, y, z, kernel);
		kpos[i] = rpos(k, kernel, obj);
	}

	/**
	 ** Each neightbor point can be determined by just applying 
	 ** kpos[i] to this point's index
	 **/
	for (i = 0 ; i < dsize ; i++) {
		fprintf(stderr, "Convolve %d\n", i);
		for (j = 0 ; j < V_DSIZE(kernel) ; j++) {
			data[i] += extract_float(kernel,j)*extract_float(obj, i+kpos[j]);
			fprintf(stderr, "    %d\r", j);
		}

	}
	if (VERBOSE) printf("\n");

	return(newVal(V_ORG(obj), 
		V_SIZE(obj)[0],
		V_SIZE(obj)[1],
		V_SIZE(obj)[2],
		FLOAT, 
		data));
}

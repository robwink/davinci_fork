#include "parser.h"

#define alpha(k, N) (k == 0 ? sqrt(1.0/(double)N) : sqrt(2.0/(double)N))

void dct(double *u, double *v, int N)
{
	int n,k;
	double q1, q2;

	q1 = M_PI / (N+N);
	for (k = 0 ; k <= N-1 ; k++) {
		v[k] = 0;
		q2 = q1 * k;
		for (n = 0 ; n <= N-1 ; n++) {
			v[k] += u[n] * cos(q2 * (n+n+1));
		}
		v[k] *= alpha(k, N);
	}
}

void idct(double *v, double *u, int N)
{
	int n,k;
	double q1, q2;

	q1 = M_PI / (N+N);
	for (n = 0 ; n <= N-1 ; n++) {
		u[n] = 0;
		q2 = q1 * (n+n+1);
		for (k = 0 ; k <= N-1 ; k++) {
			u[n] += alpha(k, N) * v[k] * cos(q2 * k);
		}
	}
}

Var *
ff_dct(vfuncptr func, Var * arg)
{
	Var *obj = NULL, *dobj = NULL;
	int x,y,z, i, j, k, l, dsize;
	double *data, *u, *v;
	int dir = -1;
	char *axis = NULL;
	char *options[] =  {
		"x", "y", "z", "xy", "yx", "xz", "zx", "yz", "zy",
		"xyz", "xzy", "yxz", "yzx", "zxy", "zyx", NULL
	};

	int ac;
	Var **av;
	Alist alist[4];
	alist[0] = make_alist( "object",    ID_VAL,    NULL,     &obj);
	alist[1] = make_alist( "axis",    ID_ENUM,    options,   &axis);
	alist[2] = make_alist( "dir",    ID_VAL,    NULL,        &dobj);
	alist[3].name = NULL;

	make_args(&ac, &av, func, arg);
	if (parse_args(ac, av, alist)) return(NULL);

	if (obj == NULL) {
		parse_error("%s: No object specified\n", func->name);
		return(NULL);
	}
	if (dobj == NULL) {
		dir = 1;
	} else {
		dir = extract_int(dobj, 0);
	}

	x = GetSamples(V_SIZE(obj), V_ORG(obj));
	y = GetLines(V_SIZE(obj), V_ORG(obj));
	z = GetBands(V_SIZE(obj), V_ORG(obj));
	dsize = V_DSIZE(obj);

	data = (double *)calloc(dsize, sizeof(double));

	/**
	 ** convert to an array of doubles
	 **/
	for (k = 0 ; k < z ; k++) {
		for (j = 0 ; j < y ; j++) {
			for (i = 0 ; i < x ; i++) {
				l = cpos(i, j, k, obj);
				data[l] = extract_double(obj, l);
			}
		}
	}

	u = (double *)calloc(max(x,max(y,z)), sizeof(double));
	v = (double *)calloc(max(x,max(y,z)), sizeof(double));

	/**
	 ** process rows 
	 **/
	if (x != 1 && (axis == NULL || strpbrk(axis, "xX"))) {
		for (k = 0 ; k < z ; k++) {
			for (j = 0 ; j < y ; j++) {
				for (i = 0 ; i < x ; i++) {
					u[i] = data[cpos(i, j, k, obj)];
				}

				if (dir == -1) idct(u, v, x);
				else dct(u,v,x);

				for (i = 0 ; i < x ; i++) {
					data[cpos(i, j, k, obj)] = v[i];
				}
			}
		}
	}

	/**
	 ** process columns
	 **/
	if (y != 1 && (axis == NULL || strpbrk(axis, "yY"))) {
		for (k = 0 ; k < z ; k++) {
			for (i = 0 ; i < x ; i++) {
				for (j = 0 ; j < y ; j++) {
					u[j] = data[cpos(i, j, k, obj)];
				}

				if (dir == -1) idct(u, v, y);
				else dct(u,v,y);

				for (j = 0 ; j < y ; j++) {
					data[cpos(i, j, k, obj)] = v[j];
				}
			}
		}
	}


	/**
	 ** process bands
	 **/
	if (z != 1 && (axis == NULL || strpbrk(axis, "zZ"))) {
		printf("processing Z\n");
		for (i = 0 ; i < x ; i++) {
			for (j = 0 ; j < y ; j++) {
				for (k = 0 ; k < z ; k++) {
					u[k] = data[cpos(i, j, k, obj)];
				}

				if (dir == -1) idct(u, v, z);
				else dct(u,v,z);

				for (k = 0 ; k < z ; k++) {
					data[cpos(i, j, k, obj)] = v[k];
				}
			}
		}
	}

	return(newVal(V_ORG(obj), 
		V_SIZE(obj)[0], 
		V_SIZE(obj)[1], 
		V_SIZE(obj)[2], 
		DOUBLE, data));
}

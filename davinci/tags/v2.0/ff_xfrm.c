#include "parser.h"
void basis_hadamard(double *data, int N);
void basis_haar(double *data, int N);

Var *
ff_basis(vfuncptr func, Var * arg)
{
    Var *v, *obj = NULL;
    int size = 0;
    int type = 0;
    int x,y,z, n;
    double *basis;
    char *ptr = NULL;

    const char *options[] = { "hadamard", "haar", NULL };
    Alist alist[4];
	alist[0] = make_alist("object",    ID_VAL,    NULL,       &obj);
	alist[1] = make_alist("type",      ID_ENUM,   options,    &ptr);
	alist[2] = make_alist("size",      INT,       NULL,       &size);
	alist[3].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

    if (obj == NULL && size <= 0) {
        parse_error("%s: No object or size specified\n", func->name);
        return(NULL);
    }

    if (ptr == NULL) {
        parse_error("Argument required: %s(%s)", func->name, "type");
        return(NULL);
    } else if (!strcasecmp(ptr, "hadamard")) type = 1;
    else if (!strcasecmp(ptr, "haar")) type = 2;
    else {
        parse_error("Unrecognized value: %s(...%s=...)", func->name, "type");
        return(NULL);
    }

    if (type == 1 || type == 2) {
        /**
        ** These functions require a base image that is a power of 2, square
        **/
        if (obj != NULL) {
            x = GetSamples(V_SIZE(obj), V_ORG(obj));
            y = GetLines(V_SIZE(obj), V_ORG(obj));
            z = GetBands(V_SIZE(obj), V_ORG(obj));

            if (z > 1) {
                parse_error("%s: Ignoring depth > 1", func->name);
            }
            n = max(x,y);
            n = pow(2.0, ceil(log((double)n)/log(2.0)));
        } else if (size > 0) {
            n = size;
            n = pow(2.0, ceil(log((double)n)/log(2.0)));
        } else {
            return(NULL);
        }

        basis = (double *) calloc(sizeof(double), n * n);
        if (type == 1) basis_hadamard(basis, n);
        else if (type == 2) basis_haar(basis, n);
    }

    v = newVal(BSQ, n, n, 1, DOUBLE, basis);
    return(v);
}


Var *
ff_mxm(vfuncptr func, Var * arg)
{
    int i,j,k, x1, y1, z1, x2, y2, z2;
    Var *ob1 = NULL, *ob2 = NULL;
    Var *v;

    Alist	alist[4];
    const char	*formats[] = { "float", "double", NULL };
    char	*format = "double"; /* Default format */

    double	*double_data;
    float	*float_data;

    alist[0] = make_alist("ob1", ID_VAL, NULL, &ob1);
    alist[1] = make_alist("ob2", ID_VAL, NULL, &ob2);
    alist[2] = make_alist("format", ID_ENUM, formats, &format);
    alist[3].name = NULL;

    if (parse_args(func, arg, alist) == 0) return(NULL);

    if (ob1 == NULL || ob2 == NULL) {
        parse_error("%s(), two objects required.", func->name);
        return (NULL);
    }

    x1 = GetSamples(V_SIZE(ob1), V_ORG(ob1));
    y1 = GetLines(V_SIZE(ob1), V_ORG(ob1));
    z1 = GetBands(V_SIZE(ob1), V_ORG(ob1));

    x2 = GetSamples(V_SIZE(ob2), V_ORG(ob2));
    y2 = GetLines(V_SIZE(ob2), V_ORG(ob2));
    z2 = GetBands(V_SIZE(ob2), V_ORG(ob2));

    if (x1 != y2) {
        parse_error("Unable to matrix multiply arrays: %d,%d x %d,%d\n",
                    x1,y1,x2,y2);
        return(NULL);
    }

    if (!strcasecmp(format, "float")) {

      float_data = (float *) calloc(y1*x2, sizeof(float));
      v = newVal(BSQ, x2, y1, 1, FLOAT, float_data);

      for (j = 0 ; j < y1 ; j++) {
        for (i = 0 ; i < x2 ; i++) {
	  float_data[cpos(i,j,0,v)] = 0;
	  for (k = 0 ; k < x1 ; k++) {
	    float_data[cpos(i,j,0,v)] += 
	      extract_float(ob1, cpos(k, j, 0, ob1)) * 
	      extract_float(ob2, cpos(i, k, 0, ob2));
	  }
        }
      }

    }
    else { /* Double */

      double_data = (double *) calloc(y1*x2, sizeof(double));
      v = newVal(BSQ, x2, y1, 1, DOUBLE, double_data);

      for (j = 0 ; j < y1 ; j++) {
        for (i = 0 ; i < x2 ; i++) {
	  double_data[cpos(i,j,0,v)] = 0;
	  for (k = 0 ; k < x1 ; k++) {
	    double_data[cpos(i,j,0,v)] += 
	      extract_double(ob1, cpos(k, j, 0, ob1)) * 
	      extract_double(ob2, cpos(i, k, 0, ob2));
	  }
        }
      }

    }

    return(v);

}

/**
 ** basis_hadamard() - Compute hadamard basis function.
 **     Assumes <data> is NxN in size, and N is a power of 2
 **/
void
basis_hadamard(double *data, int N)
{
    int i, j, k = 1;
    double d;

    data[0] = 1;

    while (k < N) {
        for (i = 0; i < k; i++) {
            for (j = 0; j < k; j++) {
                d = data[i + j * N];
                data[(i + k) + (j) * N] = d;
                data[(i) + (j + k) * N] = d;
                data[(i + k) + (j + k) * N] = -d;
            }
        }
        k = k * 2;
    }
}

/**
 ** basis_haar() - Compute haar basis function.
 **     Assumes <data> is NxN in size, and N is a power of 2
 **/
void
basis_haar(double *data, int N)
{
    int i, j, k, p, q;
    double x, p2, d;

    for (k = 0; k < N; k++) {
        p = 0;                              /* compute p and q from k */
        for (j = k; j > 1; j >>= 1) 
            p++;
        p2 = 1 << p;                        /* this is 2^p */
        q = k - p2 + 1;

        /**
        ** Compute H(x) across a row, where x ranges from 0/N to (N-1)/N 
        **/
        for (i = 0; i < N; i++) {
            x = i / (double) N;
            if (k == 0) {
                d = 1;
            } else if (x >= (q - 1) / p2 && x < (q - 0.5) / p2) {
                d = pow(2.0, (p / 2.0));
            } else if (x >= (q - 0.5) / p2 && x < q / p2) {
                d = -pow(2.0, (p / 2.0));
            } else {
                d = 0;
            }
            data[i + k * N] = d;
        }
    }
}

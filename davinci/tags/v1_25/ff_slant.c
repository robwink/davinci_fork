#include "parser.h"

Var *
ff_slant(vfuncptr func, Var * arg)
{
	Var *obj = NULL;
	Var *ival= NULL;
	Var *out;
	int i,j,k;
	int x,y,z;
	int *leftmost,*rightmost;
	int w, width;
	Var *a;

	float ignore;
	float v;
	float *odata;

	Alist alist[3];
	alist[0] = make_alist("object",		ID_VAL,		NULL,	&obj);
	alist[1] = make_alist("ignore",		ID_VAL,		NULL,	&ival);
	alist[2].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (ival) ignore = extract_float(ival,0);

	x = GetX(obj);
	y = GetY(obj);
	z = GetZ(obj);

	/*
	** output is a column
	*/
	leftmost = calloc(y, sizeof(int));
	rightmost = calloc(y, sizeof(int));

	for (i = 0 ; i < y ; i++) {
		leftmost[i] = x-1;
		rightmost[i] = 0;
	}

	for (j = 0 ; j < y ; j++) {
		for (i = 0 ; i < x ; i++) {
			for (k = 0 ; k < z ; k++) {
				v = extract_float(obj, cpos(i,j,k,obj));
				if (ival && v == ignore) continue;
				if (i < leftmost[j]) {
					leftmost[j] = i;
				}
				if (i > rightmost[j]) {
					rightmost[j] = i;
				}
			}
		}
	}

	/* find maximum width */
	width = 0;
	for (i = 0 ; i < y ; i++) {
		w = rightmost[i] - leftmost[i] +1;
		if (w > width) width = w;
	}

	/* fix leftmost to allow for maximum width */
	for (i = 0 ; i < y ; i++) {
		leftmost[i] = min(leftmost[i], x-width);
	}

	odata = calloc(width*y*z,4);
	out = newVal(BSQ, width, y, z, FLOAT, odata);

	for (j = 0 ; j < y ; j++) {
		for (i = leftmost[j] ; i < leftmost[j]+width ; i++) {
			for (k = 0 ; k < z ; k++) {
				odata[cpos(i-leftmost[j], j, k, out)] = extract_float(obj, cpos(i, j, k, obj));
			}
		}
	}

	a = new_struct(3);
	add_struct(a, "data", out);
	add_struct(a, "leftedge", newVal(BSQ, 1, y, 1, INT, leftmost));
	add_struct(a, "width", newInt(x));

	return(a);
}

Var *
ff_unslant(vfuncptr func, Var * arg)
{
	Var *obj = NULL;
	Var *ival= NULL;
	Var *out;
	int i,j,k;
	int x,y,z;
	int *leftmost,*rightmost;
	int width;
	Var *data, *leftedge, *w;
	float ignore;
	float *odata;
	int p;

	Alist alist[3];
	alist[0] = make_alist("object",	ID_STRUCT,		NULL,	&obj);
	alist[1] = make_alist("ignore",	ID_VAL,		NULL,	&ival);
	alist[2].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (find_struct(obj, "data", &data) == -1 ||
	    find_struct(obj, "leftedge", &leftedge) == -1 ||
	    find_struct(obj, "width", &w) == -1) {
		parse_error("Structure doesn't appear to have members for unslanting.");
		return(NULL);
	}

	x = GetX(data);
	y = GetY(data);
	z = GetZ(data);

	if (ival) ignore = extract_float(ival, 0);
	width = extract_int(w, 0);
	leftmost = V_DATA(leftedge);

	odata = calloc(width*y*z, sizeof(float));
	out = newVal(BSQ, width, y, z, FLOAT, odata);

	for (j = 0 ; j < y ; j++) {
		for (i = 0 ; i < width ; i++) {
			for (k = 0 ; k < z ; k++) {
				p = cpos(i,j,k,out);
				if (i >= leftmost[j] && i < leftmost[j]+x) {
					odata[p] = extract_float(data, cpos(i-leftmost[j],j,k, data));
				} else {
					odata[p] = ignore;
				}
			}
		}
	}
	return(out);
}

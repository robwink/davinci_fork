#include "parser.h"
#include "func.h"


/*
** Varrays 
**
** Varrays are 1 dimensional arrays of Vars.
**           
*/

Var *
vcat(Var *a, Var *b)
{
	int n;
	Var **vdata, *v;

	if (a == NULL) return(b);
	if (b == NULL) return(a);

	if (V_TYPE(a) == ID_VARRAY ) {
		v = V_DUP(a);
		/*
		** Arg1 is an array already.  Just make space in it for 2
		*/
		n = V_DSIZE(v);
		vdata = V_DATA(v);
		vdata = realloc(vdata, (n+1)*sizeof(Var *));
		vdata[n] = V_DUP(b);
		V_DSIZE(v) = n+1;
		V_SIZE(v)[0] = n+1;
		V_DATA(v) = vdata;
	} else {
		v = newVal(BSQ, 2, 1, 1, 0, NULL);
		vdata = calloc(2, sizeof(Var *));
		vdata[0] = V_DUP(a);
		vdata[1] = V_DUP(b);

		V_TYPE(v) = ID_VARRAY;
		V_DATA(v) = vdata;
	}
	return(v);
}

Var *
ff_vcat(vfuncptr func, Var * arg)
{
	Var *a, *b, *v;

	a = arg;
	b = a->next;

	if (a == NULL || b == NULL) {
		parse_error("Not enough args: %s", func->name);
		return (NULL);
	}
	if ((v = eval(a)) == NULL) {
		parse_error("Variable not found: %s", V_NAME(a));
		return (NULL);
	} else {
		a = v;
	}
	if ((v = eval(b)) == NULL) {
		parse_error("Variable not found: %s", V_NAME(b));
		return (NULL);
	} else {
		b = v;
	}

	return(vcat(a,b));
}

Var *
varray_subset(Var *v, Range *r)
{
	Var **in, **out ,*s;
	int size;
	int i, count;


	in = V_DATA(v);
	size = 1 + (r->hi[0] - r->lo[0]) / r->step[0];

	if (size == 1) {
		/*
		** single occurance, just return the Var
		*/
		s = V_DUP(in[r->lo[0]]);
	} else {
		out = calloc(size, sizeof(Var **));
		count = 0;
		for (i = r->lo[0] ; i <= r->hi[0] ; i+= r->step[0])  {
			out[count++] = V_DUP(in[i]);
		}
		s = newVal(BSQ, size, 1, 1, 0, NULL);
		V_TYPE(s) = ID_VARRAY;
		V_DATA(s) = out;
	}
	return(s);
}

Var *
set_varray(Var *v, Range *r, Var *e)
{
	int i;
	int count = 0;
	int size = 1 + (r->hi[0] - r->lo[0]) / r->step[0];

	Var **dst = V_DATA(v);
	Var **src = NULL;

	if (size == 1) {
		i = r->lo[0];
		dst[i] = V_DUP(e);
		mem_claim(dst[i]);
	} else {
		if (V_TYPE(e) == ID_VARRAY) src = V_DATA(e);

		for (i = r->lo[0] ; i <= r->hi[0] ; i += r->step[0]) {
			free_var(dst[i]);
			dst[i] = (src == NULL ? V_DUP(e) : V_DUP(src[count++]));
			mem_claim(dst[i]);
		}
	}
	return(varray_subset(v, r));
}

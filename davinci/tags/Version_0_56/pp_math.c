#include "parser.h"

/**
 ** This file holds the pp_math function and all the support routines and
 ** macro definitions.
 **/

int rpos(int, Var *, Var *);

/**
 ** This is some gross abuse of the preprocessor.
 **
 ** T1 is the types of the arguments to use while doing math.
 ** T2 is the output type.
 ** _E_ and _S_ are the extract and saturate functions.
 **/
#define DO_MATH_LOOP(T1,T2,_E_,_S_) \
{\
	int k; \
	T1 v1, v2, r;\
	T2 *idata = (T2 *)data;\
	for (i = 0 ; i < dsize ; i++) {\
		v1 = _E_(a,rpos(i,val,a));\
		v2 = _E_(b,rpos(i,val,b));\
		switch(op) {\
			case ID_ADD:    idata[i] = _S_(v1+v2); break;\
			case ID_SUB:    idata[i] = _S_(v1-v2); break;\
			case ID_MULT:   idata[i] = _S_(v1*v2); break;\
			case ID_DIV: {\
				if (v2 != 0) {\
					idata[i] = _S_(v1/v2);\
				} else {\
					idata[i] = _S_(0);\
					dzero++;\
				}\
				break;\
			}\
			case ID_MOD: idata[i] = (T2)_S_(fmod((double)v1, (double)v2)); break;\
			case ID_POW: idata[i] = (T2)_S_(pow((double)v1, (double)v2)); break;\
		}\
	}\
}

#define DO_RELOP_LOOP(T1,T2,_E_,_S_) \
{\
	int k; \
	T1 v1, v2, r;\
	T2 *idata = (T2 *)data;\
	for (i = 0 ; i < dsize ; i++) {\
		v1 = _E_(a,rpos(i,val,a));\
		v2 = _E_(b,rpos(i,val,b));\
		switch(op) {\
			case ID_EQ:     idata[i] = (v1 == v2);	break;\
			case ID_NE:     idata[i] = (v1 != v2);	break;\
			case ID_LT:     idata[i] = (v1 < v2);	break;\
			case ID_GT:     idata[i] = (v1 > v2);	break;\
			case ID_LE:     idata[i] = (v1 <= v2);	break;\
			case ID_GE:     idata[i] = (v1 >= v2);	break;\
			case ID_OR:     idata[i] = (v1 || v2);	break;\
			case ID_AND:	idata[i] = (v1 && v2);	break;\
		}\
	}\
}

int
is_relop(int op)
{
    switch (op) {
    case ID_EQ:
    case ID_NE:
    case ID_LT:
    case ID_GT:
    case ID_LE:
    case ID_GE:
    case ID_OR:
    case ID_AND:
        return(1);
    }
    return(0);
}


/**
 ** pp_math()   - perform requested math operation
 **/

Var *
pp_math(Var * a, int op, Var * b)
{
    int in_format, out_format;
    int size[3];
    int dsize = 0;
    int i;
    int order;
    Var *val, *t;
    void *data;
    int count;
    int va, vb;
    int ca = 1, cb = 1;
    int dzero=0;

    if (a == NULL) a = VZERO;	/* define this somewhere */
    if (b == NULL) return(NULL);	/* called with error */

    if ((t = eval(a)) == NULL) {
        sprintf(error_buf, "Variable not found: %s", V_NAME(a));
        parse_error(NULL);
        return (NULL);
    }
    a = t;
    if ((t = eval(b)) == NULL) {
        sprintf(error_buf, "Variable not found: %s", V_NAME(b));
        parse_error(NULL);
        return (NULL);
    }
    b = t;
    /**
    ** Verify that we can actually do math with these two objects.
    **
    ** Okay if:	dimensions are the same or 1.	(size == size)
    **
    **/

    if (V_TYPE(a) == ID_STRING || V_TYPE(b) == ID_STRING || 
		  V_TYPE(a) == ID_TEXT	 || V_TYPE(b) == ID_TEXT) {
        return (pp_math_strings(a, op, b));
    }
    if (V_TYPE(a) != ID_VAL || V_TYPE(b) != ID_VAL) {	/* can this happen? */
        parse_error("math operation illegal on non-values");
        return (NULL);
    }
    count = 0;
    for (i = 0; i < 3; i++) {
        va = V_SIZE(a)[orders[V_ORDER(a)][i]];
        vb = V_SIZE(b)[orders[V_ORDER(b)][i]];
        if (va != 1 && vb != 1 && va != vb) {
            parse_error("math operation illegal, sizes differ");
            return (NULL);
        }
        if (va != vb) {
            ca *= va;
            cb *= vb;
        }
    }
    if (ca != 1 && cb != 1) {
        parse_error("math operation illegal, sizes differ on more than 1 axis");
        return (NULL);
    }
    /**
    ** Figure out return type and size, and allocate space
    **
    ** size(a) == size(b), or one or both are 1.
    ** take order from the larger matrix (less paging that way)
    **/

    in_format = out_format =  max(V_FORMAT(a), V_FORMAT(b));

    if (is_relop(op)) {
        out_format = BYTE;
    }

    dsize = 1;
    for (i = 0; i < 3; i++) {
        size[i] = max(V_SIZE(a)[orders[V_ORDER(a)][i]],
                      V_SIZE(b)[orders[V_ORDER(b)][i]]);
        dsize *= size[i];
    }
    order = (V_DSIZE(a) > V_DSIZE(b) ? V_ORDER(a) : V_ORDER(b));

    if (dsize == 0)
        dsize = 1;	/* impossible? */

    /**
    ** can we reuse one of the input values here?
    **/

    data = calloc(dsize, NBYTES(out_format));

    val = newVar();
    V_TYPE(val) = ID_VAL;
    V_FORMAT(val) = out_format;
    V_DSIZE(val) = dsize;
    V_ORDER(val) = order;
    V_DATA(val) = data;

/** size was extracted as x,y,z.  put it back appropriately **/

    V_SIZE(val)[orders[order][0]] = size[0];
    V_SIZE(val)[orders[order][1]] = size[1];
    V_SIZE(val)[orders[order][2]] = size[2];

    if (is_relop(op)) {
        switch (in_format) {
        case BYTE:
        case SHORT:
        case INT:
            DO_RELOP_LOOP(int, u_char, extract_int, (int));
            break;
        case FLOAT:
            DO_RELOP_LOOP(float, u_char, extract_float, (float));
            break;
        case DOUBLE:
            DO_RELOP_LOOP(double, u_char, extract_double, (double));
            break;
        }
    } else {
        /**
        ** For each output element (0-size), de-compute relative position using
        ** order, and re-compute offset to that element in the other var.
        **/
        switch (in_format) {
        case BYTE:
            DO_MATH_LOOP(int, u_char, extract_int, saturate_byte);
            break;
        case SHORT:
            DO_MATH_LOOP(int, short, extract_int, saturate_short);
            break;
        case INT:
            DO_MATH_LOOP(int, int, extract_int, (int));
            break;
        case FLOAT:
            DO_MATH_LOOP(float, float, extract_float, (float));
            break;
        case DOUBLE:
            DO_MATH_LOOP(double, double, extract_double, (double));
            break;
        }
        if (dzero) {
            parse_error("Division by zero, %d times", dzero);
        }
    }

    return (val);
}



typedef int (*ifptr) (Var *, Var *, int);
ifptr cvtf[3][3] =
{
    {__BSQ2BSQ, __BSQ2BIL, __BSQ2BIP},
    {__BIL2BSQ, __BIL2BIL, __BIL2BIP},
    {__BIP2BSQ, __BIP2BIL, __BIP2BIP}
};

/**
      ** rpos() - reverse calculate a linear offset
      **
      ** This converts from one ordering to another.
      ** If its a constant, just return 0.
      ** if 'from' and 'to' are the same, just return i.
      **/

int
rpos(int i, Var * from, Var * to)
{
    if (V_DSIZE(to) == 1) {
        return (0);
    } else if ((V_ORDER(from) == V_ORDER(to)) &&
               (V_SIZE(from)[0] == V_SIZE(to)[0]) &&
               (V_SIZE(from)[1] == V_SIZE(to)[1]) &&
               (V_SIZE(from)[2] == V_SIZE(to)[2])) {

        return (i);
    } else {
        return (cvtf[V_ORDER(from)][V_ORDER(to)] (from, to, i));
    }
}

/**
 ** return index of x,y,z in v
 **/
int 
cpos(int x, int y, int z, Var *v)
{
    switch(V_ORG(v)) {
    case BSQ: return(x + V_SIZE(v)[0] * (y + z * V_SIZE(v)[1]));
    case BIP: return(z + V_SIZE(v)[0] * (x + y * V_SIZE(v)[1]));
    case BIL: return(x + V_SIZE(v)[0] * (z + y * V_SIZE(v)[1]));
    default:
        printf("cpos: whats this?\n");
    }
    /**
    ** should never get here.
    return(out[0] + V_SIZE(v)[0] * (out[1] + out[2] * V_SIZE(v)[1]));
    **/
    return(0);
}

void
xpos(int i, Var *v, int *x, int *y, int *z)
{
    /**
    ** Given i, where does it fall in V
    **/
    int d[3];

    d[0] = i % (V_SIZE(v)[0]);
    d[1] = (i / V_SIZE(v)[0]) % V_SIZE(v)[1];
    d[2] = i / (V_SIZE(v)[0] * V_SIZE(v)[1]);

    *x = d[orders[V_ORG(v)][0]];
    *y = d[orders[V_ORG(v)][1]];
    *z = d[orders[V_ORG(v)][2]];
}


int 
extract_int(Var * v, int i)
{
    switch (V_FORMAT(v)) {
    case BYTE:
        return ((int) ((u_char *) V_DATA(v))[i]);
    case SHORT:
        return ((int) ((short *) V_DATA(v))[i]);
    case INT:
        return ((int) ((int *) V_DATA(v))[i]);
    case FLOAT:
        return ((int) ((float *) V_DATA(v))[i]);
    case DOUBLE:
        return ((int) ((double *) V_DATA(v))[i]);
    }
    return (0);
}
float 
extract_float(Var * v, int i)
{
    switch (V_FORMAT(v)) {
    case BYTE:
        return ((float) ((u_char *) V_DATA(v))[i]);
    case SHORT:
        return ((float) ((short *) V_DATA(v))[i]);
    case INT:
        return ((float) ((int *) V_DATA(v))[i]);
    case FLOAT:
        return ((float) ((float *) V_DATA(v))[i]);
    case DOUBLE:
        return ((float) ((double *) V_DATA(v))[i]);
    }
    return (0);
}
double 
extract_double(Var * v, int i)
{
    switch (V_FORMAT(v)) {
    case BYTE:
        return (((u_char *) V_DATA(v))[i]);
    case SHORT:
        return (((short *) V_DATA(v))[i]);
    case INT:
        return (((int *) V_DATA(v))[i]);
    case FLOAT:
        return (((float *) V_DATA(v))[i]);
    case DOUBLE:
        return (((double *) V_DATA(v))[i]);
    }
    return (0);
}

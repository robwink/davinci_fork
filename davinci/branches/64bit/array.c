#include "parser.h"

/**
 ** Convert specified position to scalar value, by applying range
 **/

int 
fixup(int i, Range *in, Range *out, int maxsize)
{
	out->lo[i] = in->lo[i];
	out->hi[i] = in->hi[i];
	out->step[i] = in->step[i];

	if (out->lo[i] == 0) out->lo[i] = 1;
	if (out->hi[i] == 0) out->hi[i] = maxsize;
	if (out->step[i] == 0) out->step[i] = 1;

	if (out->lo[i] < 0 || 
		out->hi[i] < 0 || 
		out->step[i] < 0 ||
		out->lo[i] > out->hi[i] || 
		out->hi[i] > maxsize) {
		return(0);
	}
	out->lo[i] = out->lo[i] -1;
	out->hi[i] = out->hi[i] -1;

	return(1);
}


int
fixup_ranges(Var *v, Range *in, Range *out)
{
    int i,j;

	if (V_TYPE(v) == ID_STRUCT) {
		/*
		** Structures only have 1 dimension
		*/
		if (in->dim != 1) {
			parse_error("Too many range values specified.");
			return(0);
		}
		memset(out, 0, sizeof(*out));
		if (fixup(0, in, out, get_struct_count(v)) == 0) return(0);

	} else {
		for (i = 0 ; i < 3 ; i++) {
			j = orders[V_ORG(v)][i];
			if (fixup(i, in, out, V_SIZE(v)[j]) == 0) return(0);
		}
	}
    return(1);
}
/**
 ** These convert from BSQ to something else
 ** Which is why orders[BIP] looks funny.
 **
 ** This is the "location" of axis N.
 **/ 

int orders[3][3] = {
    { 0,1,2 },
    { 0,2,1 },
    { 1,2,0 }
};


Var *
extract_array(Var *v, Range *r)
{
    Range rout;
    Var *out;

    int f_lo[3];
    int f_hi[3];
    int f_step[3];
    size_t i,j,k,count,size=1;
    size_t f1,f2,f3;
    void *data;
    int bytes, opt_bytes, opt_step;

    /**
     ** fix up range values for use and do error detection.
     **/ 

	if (fixup_ranges(v, r, &rout) == 0) {
		parse_error("Illegal range value.");
		return(NULL);
	}
    for (j = 0 ; j < 3 ; j++) {
        i = orders[V_ORG(v)][j];

        f_lo[i] = rout.lo[j];
        f_hi[i] = rout.hi[j];
		f_step[i] = rout.step[j];

        size *= 1 + (f_hi[i] - f_lo[i]) / f_step[i];
    }

    bytes = NBYTES(V_FORMAT(v));
    data = calloc(bytes, size);

    /**
     ** Want to loop on the innermost axis first.
	 **
	 ** This should be optimized for the special case of multiple f[0] in a row
     **/
	opt_bytes = bytes;
	opt_step = f_step[0];
	if (f_step[0] == 1) {
		opt_bytes *= (f_hi[0] - f_lo[0] + 1);
		opt_step = f_hi[0] - f_lo[0] + 1;
	}

    count = 0;
    for (i = f_lo[2] ; i <= f_hi[2] ; i+=f_step[2]) {
        f1 = i*V_SIZE(v)[0]*V_SIZE(v)[1];
        for (j = f_lo[1] ; j <= f_hi[1] ; j+=f_step[1]) {
            f2 = f1 + j*V_SIZE(v)[0];
            for (k = f_lo[0] ; k <= f_hi[0] ; k+=opt_step) {
                f3 = f2+k;
                memcpy(((u_char *)data)+count,
                       ((u_char *)V_DATA(v))+(f3*bytes), opt_bytes);
                count += opt_bytes;
            }
        }
    }
    out = newVar();

    V_DATA(out) = data;
    V_FORMAT(out) = V_FORMAT(v);
    V_DSIZE(out) = size;
    V_ORDER(out) = V_ORG(v);
    for (i = 0 ; i < 3 ; i++) {
        V_SIZE(out)[i] = 1 + (f_hi[i]-f_lo[i])/f_step[i];
    }
    V_TYPE(out) = ID_VAL;
    return(out);
}

/*
Var *
subset(Var *v, 
		int xlo, int xhi, int xskip,
		int ylo, int yhi, int yskip,
		int zlo, int zhi, int zskip)
{
	
}
*/


Var *
ff_translate(vfuncptr func, Var *arg)
{
    Var *object = NULL, *v,*s,*e;
    int from;
    int to;
    int d[3];
    int count;
    char *ptr;
    int flip = 0;
    int nbytes;
    int in_size[3], out_size[3];
    int i,j,k,l,t;
    char *options[] = { "x", "y", "z", NULL };
	char *axis1_str=NULL, *axis2_str = NULL;

	Alist alist[5];
	alist[0] = make_alist( "object",  ID_VAL,    NULL,       &object);
	alist[1] = make_alist( "from",    ID_ENUM,    options,    &axis1_str);
	alist[2] = make_alist( "to",      ID_ENUM,    options,    &axis2_str);
	alist[3] = make_alist( "flip",    INT,       NULL,       &flip);
	alist[4].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (object == NULL) {
        parse_error("%s(), No object specified.", func->name);
        return (NULL);
	}

    if (axis1_str == NULL) {
        parse_error("%s(), No from axis specified.", func->name);
        return (NULL);
    } else {
        if (!strcasecmp(axis1_str, "x")) from = 0;
        else if (!strcasecmp(axis1_str, "y")) from = 1;
        else if (!strcasecmp(axis1_str, "z")) from = 2;
    }

    if (axis2_str == NULL) {
        parse_error("%s(), No to axis specified.", func->name);
        return (NULL);
    } else {
        if (!strcasecmp(axis2_str, "x")) to = 0;
        else if (!strcasecmp(axis2_str, "y")) to = 1;
        else if (!strcasecmp(axis2_str, "z")) to = 2;
    }

    v = object;
    s = newVar();
    V_TYPE(s) = V_TYPE(v);
    V_DSIZE(s) = V_DSIZE(v);
    V_ORG(s) = V_ORG(v);
    V_FORMAT(s) = V_FORMAT(v);
    nbytes = NBYTES(V_FORMAT(v));

    V_DATA(s) = calloc(NBYTES(V_FORMAT(s)), V_DSIZE(s));
    out_size[0] = in_size[0] = V_SIZE(v)[0];
    out_size[1] = in_size[1] = V_SIZE(v)[1];
    out_size[2] = in_size[2] = V_SIZE(v)[2];

    from = orders[V_ORG(v)][from];
    to = orders[V_ORG(v)][to];

    out_size[from] = in_size[to];
    out_size[to] = in_size[from];

    V_SIZE(s)[0] = out_size[0];
    V_SIZE(s)[1] = out_size[1];
    V_SIZE(s)[2] = out_size[2];

    count = 0;
    for (k = 0 ; k < in_size[2] ; k++) {
        for (j = 0 ; j < in_size[1] ; j++) {
            for (i = 0 ; i < in_size[0] ; i++) {
                d[0] = i;
                d[1] = j;
                d[2] = k;

                if (flip) d[from] = in_size[from] - (d[from]+1);

                t = d[from];
                d[from] = d[to];
                d[to] = t;


                l = d[0] + (d[1] + d[2]*out_size[1])*out_size[0];
                memcpy(((unsigned char *)V_DATA(s))+l*nbytes, 
                       ((unsigned char *)V_DATA(v))+count*nbytes, 
                       nbytes);
                count++;
            }
        }
    }

    return(s);

}

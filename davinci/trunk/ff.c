#include "ff.h"
#include "apidef.h"

#ifdef rfunc
#include "rfunc.h"
#endif

#ifdef __MSDOS__
#include <dos.h>

#endif
/**
 ** V_func - find and call named function
 **
 ** This is the function dispatcher.  It locates the function 
 ** handler for a named function and exectues it.
 **/


Var *
V_func(char *name, Var * arg)
{
	vfuncptr f;
	UFUNC *uf, *locate_ufunc(char *);
	APIDEFS *api;

#ifdef rfunc
	ArgsRegister *rf, *locate_rfunc(char *);
#endif

	/**
	 ** Find and call the named function or its handler
	 **/
	for (f = vfunclist; f->name != NULL; f++) {
		if (!strcmp(f->name, name)) {
			return (f->fptr(f, arg));
		}
	}

#ifdef rfunc
	if ((rf = locate_rfunc(name)) != NULL) {
		return(dispatch_rfunc(rf, arg));
	}
#endif

	/**
	 ** No internal function match.  Check ufunc list
	 **/
	if ((uf = locate_ufunc(name)) != NULL) {
		return (dispatch_ufunc(uf, arg));
	}

#ifdef INCLUDE_API
	/**
	 ** Check for func in API list
	 **/
	if((api = api_lookup(name)) != NULL){
		return(dispatch_api(api,arg));
	}
#endif

	/**
	 ** No function found?  Return NULL
	 **/
	parse_error( "Function not found: %s", name);
	return (NULL);
}

/**
 ** verify_single_arg() - check arg list for valid argument.
 **
 ** Verify that the passed argument is:
 **             A named variable of type ID_VAL,
 **                     A unnamed variable of type ID_VAL
 ** and report any errors.
 **/

Var *
verify_single_arg(vfuncptr func, Var * arg)
{
	Var *v;

	if (arg == NULL) {
		return (NULL);
	} else if (arg->next != NULL) {
		parse_error( "Too many arguments to function: %s()", func->name);
		return (NULL);
	}
	if (V_TYPE(arg) == ID_KEYWORD) {
		arg = V_KEYVAL(arg);
	}
	if ((v = eval(arg)) == NULL) {
		parse_error( "Variable not found: %s", V_NAME(arg));
		return (NULL);
	}
	if (V_TYPE(v) != ID_VAL) {
		parse_error( "Invalid argument to function: %s()", func->name);
		return (NULL);
	}
	return (v);
}

Var *
verify_single_string(vfuncptr func, Var * arg)
{
	Var *v;

	if (arg == NULL) {
		return (NULL);
	} else if (arg->next != NULL) {
		parse_error("Too many arguments to function: %s()", func->name);
		return (NULL);
	}
	if ((v = eval(arg)) == NULL) {
		parse_error("Variable not found: %s", V_NAME(arg));
		return (NULL);
	}
	if (V_TYPE(v) != ID_STRING) {
		parse_error("Invalid argument to function: %s()", func->name);
		return (NULL);
	}
	return (v);
}

/**
 ** ff_dfunc() - function caller for intrinsic math (double) functions
 **
 ** This function iterates on all the elements of the passed argument,
 ** calling a named function for each element.  The named functions can
 ** only handle single value arguments (ie: cos(), sin(), etc)
 **
 ** return value should be single precision, unless the passed value
 ** is already double precision.
 **/

Var *
ff_dfunc(vfuncptr func, Var * arg)
{
	Var *v, *s;
	dfunc fptr;
	float *fdata;
	double *ddata;

	void *data;
	int format, dsize;
	int i;

	if ((v = verify_single_arg(func, arg)) == NULL)
		return (NULL);

    /**
     ** Find the named function.
     **/

	fptr = (dfunc) func->fdata;
	if (fptr == NULL) {	/* this should never happen */
		parse_error("Function not found.");
		return (NULL);
	}
    /**
     ** figure out if we should be returning float or double.
     **/
	format = max(FLOAT, V_FORMAT(v));
	dsize = V_DSIZE(v);

    /**
     ** reuse memory if not named.
     **/
	/*
	   if (format == V_FORMAT(v) && V_NAME(v) == NULL) {
	   data = V_DATA(v);
	   } else {
	   data = calloc(dsize, NBYTES(format));
	   }
	 */
	data = calloc(dsize, NBYTES(format));

	switch (format) {
	    case FLOAT:
		    {
			    fdata = (float *) data;
			    for (i = 0; i < dsize; i++) {
				    fdata[i] = (float) fptr(extract_double(v, i));
			    }
			    break;
		    }
	    case DOUBLE:
		    {
			    ddata = (double *) data;
			    for (i = 0; i < dsize; i++) {
				    ddata[i] = (double) fptr(extract_double(v, i));
			    }
			    break;
		    }
	}

    /**
     ** If we reused memory, make sure it doesn't get free'd.
     **/
	if (data == V_DATA(v))
		V_DATA(v) = NULL;

    /**
     ** construct a new var
     **/

	s = new(Var);
	memcpy(s, v, sizeof(Var));
	V_NAME(s) = NULL;
	V_NEXT(s) = NULL;	/* just in case */

	memcpy(V_SYM(s), V_SYM(v), sizeof(Sym));
	V_FORMAT(s) = format;
	V_DATA(s) = data;
	if (V_TITLE(v))
		V_TITLE(s) = strdup(V_TITLE(v));

	return (s);
}


/**
 ** binary operator function, double
 **/

Var *
ff_bop(vfuncptr func, Var * arg)
{
    int format;
    int size[3];
    int dsize = 0;
    int i;
    int order;
    Var *val, *t, *a, *b;
    int count;
    int va, vb;
    int ca = 1, cb = 1;
	double v1, v2;
	double *ddata;
	ddfunc fptr;

	fptr = (ddfunc) func->fdata;

	if (fptr == NULL) {	/* this should never happen */
		parse_error("Function not found.");
		return (NULL);
	}
	if ((a = arg) == NULL || (b = arg->next) == NULL) {
		parse_error("Not enough args: %s", func->name);
		return (NULL);
	}
	if ((t = eval(a)) == NULL) {
		parse_error("Variable not found: %s", V_NAME(a));
		return (NULL);
	}
	a = t;
	if ((t = eval(b)) == NULL) {
		parse_error("Variable not found: %s", V_NAME(b));
		return (NULL);
	}
	b = t;

    /**
    ** Verify that we can actually operate with these two objects.
    **
    ** Okay if:	dimensions are the same or 1.	(size == size)
    **
    **/

	if (V_TYPE(a) != ID_VAL || V_TYPE(b) != ID_VAL) {
		parse_error( "%s: operation illegal on non-values", func->name);
		return (NULL);
	}
	count = 0;
	for (i = 0; i < 3; i++) {
		va = V_SIZE(a)[orders[V_ORDER(a)][i]];
		vb = V_SIZE(b)[orders[V_ORDER(b)][i]];
		if (va != 1 && vb != 1 && va != vb) {

			parse_error("%s: operation illegal, sizes differ", func->name);
			return (NULL);
		}
		if (va != vb) {
			ca *= va;
			cb *= vb;
		}
	}
	if (ca != 1 && cb != 1) {
		parse_error("%s: operation illegal, sizes differ on more than 1 axis",
			func->name);
		return (NULL);
	}
	format = DOUBLE;
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
	ddata = (double *)calloc(dsize, NBYTES(format));

	val = new(Var);
	V_TYPE(val) = ID_VAL;
	V_FORMAT(val) = format;
	V_DSIZE(val) = dsize;
	V_ORDER(val) = order;
	V_DATA(val) = ddata;

/** size was extracted as x,y,z.  put it back appropriately **/

	V_SIZE(val)[orders[order][0]] = size[0];
	V_SIZE(val)[orders[order][1]] = size[1];
	V_SIZE(val)[orders[order][2]] = size[2];


    /**
    ** For each output element (0-size), de-compute relative position using
    ** order, and re-compute offset to that element in the other var.
    **/
	for (i = 0; i < dsize; i++) {
		v1 = extract_double(a, rpos(i, val, a));
		v2 = extract_double(b, rpos(i, val, b));
		ddata[i] = (*fptr)(v1, v2);
	}
	return (val);
}

/**
 ** convert organization
 **/

Var *
ff_org(vfuncptr func, Var * arg)
{
	Var *v, *s, *e, *ob;
	int i, j;
	void *from, *to;
	int org = -1, nbytes, format, dsize;
	char *ptr;

    /**
     ** converting to type stored in vfunc->fdata.
     **/

	struct keywords kw[] =
	{
		{"object", NULL},
		{"order", NULL},
		{"org", NULL},
		{NULL, NULL}
	};

	if (evaluate_keywords(func, arg, kw)) {
		return (NULL);
	}
	if ((v = get_kw("object", kw)) == NULL) {
		parse_error("No object specified: %s()\n", func->name);
		return (NULL);
	}
	if ((e = eval(v)) == NULL || V_TYPE(e) != ID_VAL) {
		parse_error("Illegal argument to org(...object=...)");
		return (NULL);
	}
	ob = e;

	if (func->fdata != NULL) {
		org = (int) (func->fdata) - 10;
	} else {
		if ((v = get_kw("order", kw)) == NULL &&
		    (v = get_kw("org", kw)) == NULL) {
	        /**
             ** no order specified.  Print out the org of the passed object.
             **/
			s = new(Var);
			V_TYPE(s) = ID_STRING;
			V_STRING(s) = strdup(Org2Str(V_ORG(ob)));
			return (s);
		} else {
			/**
			 ** v specifies order.
			 **/
			 char *options[] = { "bsq", "bil", "bip", "xyz", "xzy", "zxy" };

			if ((ptr = enumerated_arg(v, options)) == NULL) {
				parse_error( "%s: Unrecognized value for keyword: %s",
					func->name, "org");
				return (NULL);
			} else {
				if (!strcasecmp(ptr, "bsq")) org = BSQ;
				else if (!strcasecmp(ptr, "xyz")) org = BSQ;
				else if (!strcasecmp(ptr, "bil")) org = BIL;
				else if (!strcasecmp(ptr, "xzy")) org = BIL;
				else if (!strcasecmp(ptr, "bip")) org = BIP;
				else if (!strcasecmp(ptr, "zxy")) org = BIP;
			}
		}
	}

    /**
     ** create output variable
     **/
	dsize = V_DSIZE(ob);
	format = V_FORMAT(ob);

	s = new(Var);
	V_TYPE(s) = ID_VAL;

	memcpy(V_SYM(s), V_SYM(ob), sizeof(Sym));

	V_DATA(s) = calloc(dsize, NBYTES(format));
	V_ORG(s) = org;

	for (i = 0; i < 3; i++) {
		V_SIZE(s)[orders[org][i]] = V_SIZE(ob)[orders[V_ORG(ob)][i]];
	}

	nbytes = NBYTES(format);
	from = V_DATA(ob);
	to = V_DATA(s);
	for (i = 0; i < V_DSIZE(s); i++) {
		j = rpos(i, ob, s);
		memcpy(((char *) to) + (j * nbytes), ((char *) from) + (i * nbytes), nbytes);
	}

	return (s);
}



Var *
ff_conv(vfuncptr func, Var * arg)
{
	int format;
	int dsize;
	void *data;
	Var *v, *s;
	int i;

    /**
     ** converting to type stored in vfunc->fdata.
     **/
	
	if (arg == NULL) return(NULL);

	if (V_TYPE(arg) != ID_VAL) {
		if ((v = verify_single_arg(func, arg)) == NULL)
			return (NULL);
	} else {
		v = arg;
	}

	format = (int) func->fdata;
	dsize = V_DSIZE(v);
	data = calloc(dsize, NBYTES(format));

	switch (format) {
	    case BYTE:
		    {
			    int d;
			    u_char *idata = (u_char *) data;
			    for (i = 0; i < dsize; i++) {
				    d = extract_int(v, i);
				    idata[i] = saturate_byte(d);
			    }
			    break;
		    }
	    case SHORT:
		    {
			    int d;
			    short *idata = (short *) data;
			    for (i = 0; i < dsize; i++) {
				    d = extract_int(v, i);
				    idata[i] = saturate_short(d);
			    }
			    break;
		    }
	    case INT:
		    {
			    int d;
			    int *idata = (int *) data;
			    for (i = 0; i < dsize; i++) {
				    d = extract_int(v, i);
				    idata[i] = saturate_int(d);
			    }
			    break;
		    }
	    case FLOAT:
		    {
			    float *idata = (float *) data;
			    for (i = 0; i < dsize; i++) {
				    idata[i] = extract_float(v, i);
			    }
			    break;
		    }
	    case DOUBLE:
		    {
			    double *idata = (double *) data;
			    for (i = 0; i < dsize; i++) {
				    idata[i] = extract_double(v, i);
			    }
			    break;
		    }
	}
    /**
     ** Make the new var
     **/

	s = new(Var);
	memcpy(s, v, sizeof(Var));
	V_NAME(s) = NULL;;
	V_NEXT(s) = NULL;

	memcpy(V_SYM(s), V_SYM(v), sizeof(Sym));

	V_DATA(s) = data;
	V_FORMAT(s) = format;
	if (V_TITLE(v))
		V_TITLE(s) = strdup(V_TITLE(v));

	return (s);
}

/**
 ** ff_dim()     - return the dimensions of arg
 **/

Var *
ff_dim(vfuncptr func, Var * arg)
{
	Var *v, *s;
	int *iptr;

	if ((v = verify_single_arg(func, arg)) == NULL) {
		return (NULL);
	}
	s = new(Var);
	V_TYPE(s) = ID_VAL;

	iptr = (int *) calloc(3, sizeof(int));

	iptr[0] = GetSamples(V_SIZE(v), V_ORG(v));
	iptr[1] = GetLines(V_SIZE(v), V_ORG(v));
	iptr[2] = GetBands(V_SIZE(v), V_ORG(v));

	V_DATA(s) = iptr;
	V_DSIZE(s) = 3;
	V_FORMAT(s) = INT;
	V_ORDER(s) = BSQ;
	V_SIZE(s)[0] = 3;
	V_SIZE(s)[2] = V_SIZE(s)[1] = 1;

	return (s);
}

/**
 ** ff_format()  - return the format of arg
 **/

Var *
ff_format(vfuncptr func, Var * arg)
{
	Var *v, *s, *object;
	char *ptr;

	struct keywords kw[] =
	{
		{"object", NULL},
		{"format", NULL},
		{"type", NULL},
		{NULL, NULL}
	};

	if (evaluate_keywords(func, arg, kw)) {
		return (NULL);
	}
	if ((v = get_kw("object", kw)) == NULL) {
		parse_error("No object specified to format()");
		return (NULL);
	}
	object = v;

	if ((v = get_kw("format", kw)) == NULL &&
	    (v = get_kw("type", kw)) == NULL) {
	    /**
         ** no format specified.  Print out the format of the passed object.
         **/
		if ((v = eval(object)) != NULL) {
			object = v;
		}
		if (V_TYPE(object) != ID_VAL) {
			parse_error("Incorrect type of object specified to format()");
			return (NULL);
		}
		s = new(Var);
		V_TYPE(s) = ID_STRING;
		V_STRING(s) = strdup(Format2Str(V_FORMAT(object)));
		return (s);
	} else {
		/**
		 ** v specifies format.
		 **/
		 char *options[] = { "byte", "short", "int", "float", "double" };

		if ((ptr = enumerated_arg(v, options)) == NULL) {
			parse_error( "%s: Unrecognized value for keyword: %s",
				func->name, "format");
			return (NULL);
		} else {
			if (!strcasecmp(ptr, "byte")) ptr = "byte";
			else if (!strcasecmp(ptr, "short")) ptr = "short";
			else if (!strcasecmp(ptr, "int")) ptr = "int";
			else if (!strcasecmp(ptr, "float")) ptr = "float";
			else if (!strcasecmp(ptr, "double")) ptr = "double";
		}
		/**
		 ** this is apparently needed to make object an only child.
		 ** Since object hasn't been evaluated, it should be safe
		 **/
		object->next = NULL;
		return (V_func(ptr, object));
	}
}



/**
 ** ff_create() - Create an empty array of specified size
 **
 ** Takes the following named keyword args:
 **
 **             $format={BIL,BIP,BSQ,XYZ,XZY,ZXY}       (BSQ)
 **             $order ={byte,char,short,int,float,double}      (float)
 **
 **
 ** We would like to be able to do: create(dim(a)), to create an array
 ** the same size as 'a'.  This means that args that are arrays need to
 ** be evaluated and multilpe arguments.
 **/



Var *
ff_create(vfuncptr func, Var * arg)
{
	Var *v, *s;
	int size[3];
	int dim = 0;
	int dsize;
	int format;
	int org;
	int i, j;
	char *ptr;

	float start = 0;
	float step = 1.0;

	struct keywords kw[] =
	{
		{"x", NULL},
		{"y", NULL},
		{"z", NULL},
		{"org", NULL},
		{"format", NULL},
		{"start", NULL},
		{"step", NULL},
		{NULL, NULL}
	};

	for (i = 0; i < 3; i++) {
		size[i] = 0;
	}
	/* this removes known and unknown keywords */
	/* unnamed args go in in the order specified */

	if (evaluate_keywords(func, arg, kw)) {
		return (NULL);
	}

    /**
     ** total up the size of kw[0,1,2], and find the size of the array to create
     **/
	for (i = 0; i < 3; i++) {
		v = eval(kw[i].value);
		if (v == NULL)
			continue;
		if (V_TYPE(v) != ID_VAL || V_FORMAT(v) != INT) {
			parse_error("Illegal argument to function: %s()", func->name);
			return (NULL);
		}
		dsize = V_DSIZE(v);
		if ((dim + dsize) > 3) {
			parse_error("Too many arguments to function: %s()", func->name);
			return (NULL);
		}
		for (j = 0; j < dsize; j++) {
			size[dim + j] = ((int *) V_DATA(v))[j];
		}
		dim += dsize;
	}

	dsize = 1;
	for (i = 0; i < 3; i++) {
		if (size[i] == 0)
			size[i] = 1;
		dsize *= size[i];
	}

    /**
     ** Evaluate the remaining args.
     **/
	if ((v = get_kw("org", kw)) == NULL) {
		org = BSQ;
	} else {
		char *org_options[] = { "bsq", "bil", "bip", "xyz", "xzy", "zxy" };
		if ((ptr = enumerated_arg(v, org_options)) == NULL) {
			parse_error("%s: Unrecognized value for keyword: %s=%s",
				func->name, "org", V_NAME(v));
			return (NULL);
		} else {
			if (!strcasecmp(ptr, "bsq")) org = BSQ;
			else if (!strcasecmp(ptr, "xyz")) org = BSQ;
			else if (!strcasecmp(ptr, "bil")) org = BIL;
			else if (!strcasecmp(ptr, "xzy")) org = BIL;
			else if (!strcasecmp(ptr, "bip")) org = BIP;
			else if (!strcasecmp(ptr, "zxy")) org = BIP;
		}
	}

	if ((v = get_kw("format", kw)) == NULL) {
		format = INT;
	} else {
	    char *format_options[] = { "byte", "short", "int", "float", "double" };
		if ((ptr = enumerated_arg(v, format_options)) == NULL) {
			parse_error("%s: Unrecognized value for keyword: %s=%s",
				func->name, "format", V_NAME(v));
			return (NULL);
		} else {
			if (!strcasecmp(V_NAME(v), "byte")) format = BYTE;
			else if (!strcasecmp(V_NAME(v), "char")) format = BYTE;
			else if (!strcasecmp(V_NAME(v), "short")) format = SHORT;
			else if (!strcasecmp(V_NAME(v), "int")) format = INT;
			else if (!strcasecmp(V_NAME(v), "float")) format = FLOAT;
			else if (!strcasecmp(V_NAME(v), "double")) format = DOUBLE;
		}
	}

	KwToFloat("start", kw, &start);
	KwToFloat("step", kw, &step);

	s = new(Var);
	V_TYPE(s) = ID_VAL;

	V_DATA(s) = calloc(NBYTES(format), dsize);
	V_FORMAT(s) = format;
	V_ORDER(s) = org;
	V_DSIZE(s) = dsize;

	for (i = 0; i < 3; i++) {
		V_SIZE(s)[i] = size[i];
	}

	switch (format) {
		case BYTE: {
			u_char *idata = (u_char *) V_DATA(s);
			for (i = 0; i < dsize; i++) {
				idata[i] = (u_char) (start + i * step);
			}
			break;
		}
		case SHORT: {
			short *idata = (short *) V_DATA(s);
			for (i = 0; i < dsize; i++) {
				idata[i] = (short)(start + i * step);
			}
			break;
		}
		case INT: {
			int *idata = (int *) V_DATA(s);
			for (i = 0; i < dsize; i++) {
				idata[i] = (int)(start + i * step);
			}
			break;
		}
		case FLOAT: {
			float *idata = (float *) V_DATA(s);
			for (i = 0; i < dsize; i++) {
				idata[i] = start + i * step;
			}
			break;
		}
		case DOUBLE: {
			double *idata = (double *) V_DATA(s);
			for (i = 0; i < dsize; i++) {
				idata[i] = start + i * step;
			}
			break;
		}
	}

	return (s);
}

/**
 ** For each arg, see if its in the keyword list.  
 **/

int
evaluate_keywords(vfuncptr func, Var * arg, struct keywords *kw)
{
	Var *v;
	struct keywords *k;

	for (v = arg; v != NULL; v = v->next) {
		if (V_TYPE(v) == ID_KEYWORD) {
			for (k = kw; k->name != NULL; k++) {
				if (!strcasecmp(k->name, V_NAME(v))) {
					k->value = V_KEYVAL(v);
					break;
				}
			}
			if (k->name == NULL) {
				parse_error("Unknown keyword to function: %s(... %s= ...)",
					func->name, V_NAME(v));
				return (1);
			}
		} else {
	        /**
             ** not a keyword value.  Stuff it into the first open keyword
             **/
			for (k = kw; k->name != NULL; k++) {
				if (k->value == NULL) {
					k->value = v;
					break;
				}
			}
			if (k->name == NULL) {
				parse_error("Too many arguements to function: %s()", func->name);
				return (1);
			}
		}
	}
	return (0);
}

Var *
get_kw(char *name, struct keywords * kw)
{
	struct keywords *k;

	for (k = kw; k->name != NULL; k++) {
		if (!strcmp(k->name, name)) {
			return (k->value);
			break;
		}
	}
	return (NULL);
}



Var *
ff_nop(vfuncptr func, Var * arg)
{
	return (NULL);
}

Var *
ff_echo(vfuncptr func, Var * arg)
{
    /**
     ** The cheap version.  Should expand this for proper use someday.
     **/
	int t;
	t = VERBOSE;
	VERBOSE = 1;

	pp_print(arg);

	VERBOSE = t;
	return (NULL);
}

/**
 ** Copy an object onto itself N times, in each direction specified.
 **/

Var *
ff_replicate(vfuncptr func, Var * arg)
{
	int x = 0, y = 0, z = 0;
	int org;
	Var *v, *s, *e;

	void *data1, *data2, *d1, *d2;
	int d1ptr, d2ptr, dptr;
	int dsize;
	int len[3];
	int size[3];
	int nbytes;
	int i, j, k, l;

	struct keywords kw[] =
	{
		{"object", NULL},
		{"x", NULL},
		{"y", NULL},
		{"z", NULL},
		{NULL, NULL}
	};

	if (evaluate_keywords(func, arg, kw)) {
		return (NULL);
	}
	if ((v = get_kw("object", kw)) == NULL) {
	/**
         ** No object specified.  Error or use VZERO?
         **/
	} else {
		if ((e = eval(v)) != NULL) {
			v = e;
		}
		if (V_TYPE(v) != ID_VAL) {
			parse_error("%s(): Invalid object specified\n", func->name);
			return (NULL);
		}
	}

	if (KwToInt("x", kw, &x) == -1 ||
	    KwToInt("y", kw, &y) == -1 ||
	    KwToInt("z", kw, &z) == -1) {
	/**
         ** Specified a bad value to one of the keyword.
         ** Error message already reported.  Bail.
         **/
		return (NULL);
	}
	if (x <= 0)
		x = 1;
	if (y <= 0)
		y = 1;
	if (z <= 0)
		z = 1;

	org = V_ORG(v);

	len[orders[org][0]] = x;
	len[orders[org][1]] = y;
	len[orders[org][2]] = z;

	dsize = V_DSIZE(v) * x * y * z;
	data2 = calloc(NBYTES(V_FORMAT(v)), dsize);
	data1 = V_DATA(v);
	nbytes = NBYTES(V_FORMAT(v));
	size[0] = V_SIZE(v)[0];
	size[1] = V_SIZE(v)[1];
	size[2] = V_SIZE(v)[2];

	l = size[0] * nbytes;

	for (k = 0; k < size[2]; k++) {
		d1ptr = k * size[0] * size[1] * nbytes;
		d2ptr = d1ptr * len[0] * len[1];

		d1 = (char *) data1 + d1ptr;
		d2 = (char *) data2 + d2ptr;

		dptr = 0;
		for (i = 0; i < size[1]; i++) {
			for (j = 0; j < len[0]; j++) {
				memcpy((void *) ((char *) d2 + dptr),
				       (void *) ((char *) d1 + i * l), l);
				dptr += l;
			}
		}
		for (i = 1; i < len[1]; i++) {
			memcpy((void *) ((char *) d2 + dptr * i), d2, dptr);
		}
	}

	l = size[2] * size[1] * size[0] * len[1] * len[0] * nbytes;
	for (i = 1; i < len[2]; i++) {
		memcpy((void *) ((char *) data2 + (l * i)), data2, l);
	}

	s = new(Var);
	V_TYPE(s) = ID_VAL;
	V_DATA(s) = data2;
	V_DSIZE(s) = dsize;
	V_SIZE(s)[0] = V_SIZE(v)[0] * len[0];
	V_SIZE(s)[1] = V_SIZE(v)[1] * len[1];
	V_SIZE(s)[2] = V_SIZE(v)[2] * len[2];
	V_FORMAT(s) = V_FORMAT(v);
	V_ORG(s) = V_ORG(v);

	return (s);
}

/**
 ** Get an integer value out of a keyword.
 ** Returns:  1 if found
 **           0 if not specified
 **          -1 if not an int.
 **          >1 if an array of ints, returns first.
 **/

int
KwToInt(char *name, struct keywords *kw, int *val)
{
	Var *v, *e;
	if ((v = get_kw(name, kw)) == NULL)
		return (0);

	if ((e = eval(v)) != NULL)
		v = e;

	if (V_TYPE(v) != ID_VAL || V_FORMAT(v) > INT) {
		parse_error("Bad value.  Expecting an INT for keyword: %s", name);
		return (-1);
	}
	*val = extract_int(v, 0);
	if (V_DSIZE(v) != 1)
		return (V_DSIZE(v));
	return (1);
}
int
KwToFloat(char *name, struct keywords *kw, float *val)
{
	Var *v, *e;
	if ((v = get_kw(name, kw)) == NULL)
		return (0);

	if ((e = eval(v)) != NULL)
		v = e;

	if (V_TYPE(v) != ID_VAL) {
		parse_error("Bad value.  Expecting a FLOAT for keyword: %s", name);
		return (-1);
	}
	*val = extract_float(v, 0);
	if (V_DSIZE(v) != 1)
		return (V_DSIZE(v));
	return (1);
}

/**
 ** Takes two objects, with the same ORG, FORMAT, and two matching axis,
 ** and concatenate them together along specified axis.
 **/

Var *
ff_cat(vfuncptr func, Var * arg)
{
	Var *v;
	Var *ob1, *ob2;
	char *ptr;
	int axis;

	struct keywords kw[] =
	{
		{"ob1", NULL},
		{"ob2", NULL},
		{"axis", NULL},
		{NULL, NULL}
	};

	if (evaluate_keywords(func, arg, kw)) {
		return (NULL);
	}

	if ((v = get_kw("axis", kw)) == NULL) {
		parse_error("%s(), No axis specified.", func->name);
		return (NULL);
	} else {
		char *options[] = { "x", "y", "z" };
		if ((ptr = enumerated_arg(v, options)) == NULL) {
			parse_error("%s(): Unrecognized value for keyword: %s",
				func->name, "axis");
			return (NULL);
		} else {
			if (!strcasecmp(ptr, "x")) axis = 0;
			else if (!strcasecmp(ptr, "y")) axis = 1;
			else if (!strcasecmp(ptr, "z")) axis = 2;
		}
	}

	if ((ob1 = get_kw("ob1", kw)) == NULL) {
		parse_error("No objects specified");
		return (NULL);
	}
	if ((ob2 = get_kw("ob2", kw)) == NULL) {
		parse_error("%s(), second object not specified", func->name);
		return (NULL);
	}
    /**
     ** convert axis from XYZ to ORDER
     **/
	return (do_cat(ob1, ob2, axis));
}

Var *
do_cat(Var * ob1, Var * ob2, int axis)
{
	Var *e, *s;
	int i, j;
	int s1[3], s2[3];
	void *data, *d1, *d2, *out;
	int dsize;
	int nbytes;

	if (ob1 != NULL) axis = orders[V_ORG(ob1)][axis];

	if ((e = eval(ob1)) != NULL) ob1 = e;
	if ((e = eval(ob2)) != NULL) ob2 = e;
	if (ob1 == NULL) return (ob2);
	if (ob2 == NULL) return (ob1);


	if (V_TYPE(ob1) != ID_VAL) {
		parse_error( "cat(), improper object specified");
		return (NULL);
	}
	if (V_TYPE(ob2) != ID_VAL) {
		parse_error( "cat(), improper object specified");
		return (NULL);
	}
	if (V_FORMAT(ob1) != V_FORMAT(ob2)) {
		parse_error( "cat(), Data formats must match.");
		return (NULL);
	}
	if (V_ORG(ob1) != V_ORG(ob2)) {
		parse_error( "cat(), Data organization must match.");
		return (NULL);
	}

	/**
	 ** convert from XYZ into the appropriate ORG
	 **/
	axis = orders[V_ORG(ob1)][axis];

	for (i = 0; i < 3; i++) {
		s1[i] = V_SIZE(ob1)[i];
		s2[i] = V_SIZE(ob2)[i];
		if (i == axis)
			continue;
		if (V_SIZE(ob1)[i] != V_SIZE(ob2)[i]) {
			parse_error("Axis must match");
			return (NULL);
		}
	}

	nbytes = NBYTES(V_FORMAT(ob1));
	d1 = V_DATA(ob1);
	d2 = V_DATA(ob2);
	dsize = V_DSIZE(ob1) + V_DSIZE(ob2);
	data = calloc(nbytes, dsize);
	out = data;

	for (j = 0; j < s1[2]; j++) {
		for (i = 0; i < s1[1]; i++) {
			memcpy(out, d1, s1[0] * nbytes);
			out = (char *) out + s1[0] * nbytes;
			d1 = (char *) d1 + s1[0] * nbytes;
			if (axis == 0) {
				memcpy(out, d2, s2[0] * nbytes);
				out = (char *) out + s2[0] * nbytes;
				d2 = (char *) d2 + s2[0] * nbytes;
			}
		}
		if (axis == 1) {
			memcpy(out, d2, s2[0] * s2[1] * nbytes);
			out = (char *) out + s2[0] * s2[1] * nbytes;
			d2 = (char *) d2 + s2[0] * s2[1] * nbytes;
		}
	}
	if (axis == 2) {
		memcpy(out, d2, s2[0] * s2[1] * s2[2] * nbytes);
	}
	s = new(Var);
	V_TYPE(s) = ID_VAL;
	V_DATA(s) = data;
	V_FORMAT(s) = V_FORMAT(ob1);
	V_ORG(s) = V_ORG(ob1);
	V_DSIZE(s) = dsize;
	V_SIZE(s)[0] = V_SIZE(ob1)[0];
	V_SIZE(s)[1] = V_SIZE(ob1)[1];
	V_SIZE(s)[2] = V_SIZE(ob1)[2];
    /**
     ** replace the axis that changed
     **/
	V_SIZE(s)[axis] = V_SIZE(ob1)[axis] + V_SIZE(ob2)[axis];

	return (s);
}

/**
 ** enumerated_arg() - Make sure arg is one of a list of enumerated values.
 **/

char *
enumerated_arg(Var * v, char **values)
{
	char *ptr;
	Var *e;
	char **p;
	char *result;

	if (V_TYPE(v) == ID_STRING)
		ptr = V_STRING(v);
	else
		ptr = V_NAME(v);

	for (p = values; p && *p; p++) {
		if (ptr && !strcasecmp(ptr, *p)) {
			result = *p;
			return (result);
		}
	}

	if ((e = eval(v)) != NULL) {
		ptr = V_STRING(e);
		for (p = values; p && *p; p++) {
			if (ptr && !strcasecmp(ptr, *p)) {
				result = *p;
				return (result);
			}
		}
	}
	return (NULL);
}

Var *
RequireKeyword(char *name, struct keywords * kw, int type, int format, vfuncptr func)
{
	Var *v, *e;
	char *tstr;

	if (name == NULL)
		return (NULL);

	if ((v = get_kw(name, kw)) == NULL) {
		parse_error("Argument required: %s(%s)", func->name, name);
		return (NULL);
	}
	if ((e = eval(v)) != NULL)
		v = e;
	if (V_TYPE(v) != type ||
	    (V_TYPE(v) == ID_VAL && format >= 0 && V_FORMAT(v) != format)) {
		switch (type) {
		    case ID_VAL:
			    switch (format) {
				case BYTE:
					tstr = "BYTE";
					break;
				case SHORT:
					tstr = "SHORT";
					break;
				case INT:
					tstr = "INT";
					break;
				case FLOAT:
					tstr = "FLOAT";
					break;
				case DOUBLE:
					tstr = "DOUBLE";
					break;
			    }
			    break;
		    case ID_STRING:
			    tstr = "STRING";
			    break;
		}
		parse_error("Expected %s for argument %s(%s)",
			tstr, func->name, name);
		return (NULL);
	}
	return (v);
}

/**
 ** convert bytes to string
 **/

Var *
ff_string(vfuncptr func, Var * arg)
{
	Var *v, *s;

	if ((v = verify_single_arg(func, arg)) == NULL)
		return (NULL);


	s = new(Var);
	V_TYPE(s) = ID_STRING;
	V_STRING(s) = (char *) memcpy(malloc(V_DSIZE(v)), V_DATA(v), V_DSIZE(v));
	return (s);
}

Var *
ff_strlen(vfuncptr func, Var * arg)
{
        char *S1=NULL;
        int ac;
        Var **av;  
        int *Result=(int *)calloc(1,sizeof(int));
        Alist alist[2];
        alist[0] = make_alist("string",         ID_STRING,         NULL,   &S1);
        alist[1].name = NULL;

        make_args(&ac, &av, func, arg);
        if (parse_args(ac, av, alist)) *Result=0;
 
        else if (S1 == NULL){
                *Result=0;
        }

	else {
		*Result=strlen(S1);
	}

	return(newVal(BSQ,1,1,1, INT, Result));
}


Var *
ff_issubstring(vfuncptr func, Var * arg)
{
	char *S1=NULL,*S2=NULL;
        int ac;
        Var **av;
	int *Result=(int *)calloc(1,sizeof(int));
        Alist alist[3];
        alist[0] = make_alist("target",         ID_STRING,         NULL,   &S1);
        alist[1] = make_alist("source",           ID_STRING,        NULL,        &S2);
        alist[2].name = NULL;

        make_args(&ac, &av, func, arg);
        if (parse_args(ac, av, alist)) *Result=0;

	else if (S1 == NULL || S2 == NULL){
		*Result=0;
	}

	else if ((strstr(S1,S2))==NULL){
		*Result=0;
	}
	else 
		*Result=1;

	return(newVal(BSQ,1,1,1, INT, Result));
}



Var *
ff_pow(vfuncptr func, Var * arg)
{
	Var *ob1, *ob2;
	Var *v, *e;
	struct keywords kw[] =
	{
		{"ob1", NULL},
		{"ob2", NULL},
		{NULL, NULL}
	};

	if (evaluate_keywords(func, arg, kw)) {
		return (NULL);
	}
	if ((v = get_kw("ob1", kw)) == NULL) {
		parse_error("%s(), two objects required.", func->name);
		return (NULL);
	} else {
		if ((e = eval(v)) != NULL) {
			v = e;
		}
		if (V_TYPE(v) != ID_VAL) {
			parse_error("%s(), improper object specified", func->name);
			return (NULL);
		}
		ob1 = v;
	}
	if ((v = get_kw("ob2", kw)) == NULL) {
		parse_error("%s(), second object not specified", func->name);
		return (NULL);
	} else {
		if ((e = eval(v)) != NULL) {
			v = e;
		}
		if (V_TYPE(v) != ID_VAL) {
			parse_error("%s(), improper object specified", func->name);
			return (NULL);
		}
		ob2 = v;
	}
	return (pp_math(ob1, ID_POW, ob2));
}

Var *
ff_system(vfuncptr func, Var * arg)
{
	Var *v;
	if ((v = verify_single_string(func, arg)) != NULL) {
		system(V_STRING(v));
	}
	return (NULL);
}

Var *
newVal(int org, int x, int y, int z, int format, void *data)
{
    Var *v = new(Var);
    V_TYPE(v) = ID_VAL;
    V_ORG(v) = org;
    V_DSIZE(v) = x*y*z;
    V_SIZE(v)[0] = x;
    V_SIZE(v)[1] = y;
    V_SIZE(v)[2] = z;
    V_FORMAT(v) = format;
    V_DATA(v) = data;
	return(v);
}
Var *
ff_exit(vfuncptr func, Var * arg)
{
	Var *obj = NULL;
	int ac;
	Var **av;
	Alist alist[2];

	alist[0] = make_alist( "object",    ID_VAL,    NULL,     &obj);
	alist[1].name = NULL;

	make_args(&ac, &av, func, arg);
	if (parse_args(ac, av, alist)) return(NULL);

	if (obj == NULL) {
		exit(0);
	} else {
		exit(extract_int(obj, 0));
	}
}

#include <sys/stat.h>

Var *
ff_fsize(vfuncptr func, Var * arg)
{
	char *filename = NULL;
	int ac;
	Var **av;
	struct stat sbuf;
	int *data;

	Alist alist[2];
	alist[0] = make_alist("filename",    ID_STRING,    NULL,     &filename);
	alist[1].name = NULL;

	make_args(&ac, &av, func, arg);
	if (parse_args(ac, av, alist)) return(NULL);

	if (filename == NULL) {
		fprintf(stderr, "%s: No filename specified\n", (char *)av[0]);
		return(NULL);
	} else {
		data = (int *)calloc(1, sizeof(int));
		*data = -1;
		if ((stat(filename, &sbuf)) == 0) {
			*data = sbuf.st_size;
		}
		return(newVal(BSQ, 1, 1, 1, INT, data));
	}
}


#include "readline/history.h"

Var *
ff_history(vfuncptr func, Var * arg)
{
	Var *value = NULL;
	int ac, i, count = 0;
	Var **av;

	Alist alist[2];
	alist[0] = make_alist( "count",    ID_VAL,    NULL,     &value);
	alist[1].name = NULL;

	make_args(&ac, &av, func, arg);
	if (parse_args(ac, av, alist)) return(NULL);

	if (value == NULL)  {
		count=-1;
	} else if ((i = extract_int(value, 0)) != 0) {
		count = i;
	}

	print_history(count);
	return(NULL);
}

void
print_history(int i)
{
	HIST_ENTRY *h;
	HISTORY_STATE *state;
	int j;

#ifdef HAVE_LIBREADLINE
	state = history_get_history_state();
	if (i == -1) i = state->length;
	for (j = state->length-i ; j < state->length ; j++) {
		h = state->entries[j];
		printf("%6d   %s\n", j+1, h->line);
	}
#endif
}


Var *
ff_hedit(vfuncptr func, Var * arg)
{
	FILE *fp;
	Var *value = NULL;
	int ac, i, j, count = 0;
	Var **av;
	HISTORY_STATE *state;
	char *tmp, *editor, buf[256];

#ifdef HAVE_LIBREADLINE

	Alist alist[2];
	alist[0] = make_alist("number",    ID_VAL,    NULL,     &value);
	alist[1].name = NULL;

	make_args(&ac, &av, func, arg);
	if (parse_args(ac, av, alist)) return(NULL);

	if (value == NULL)  {
		count=10;
	} else if ((i = extract_int(value, 0)) != 0) {
		count = i;
	}

	state = history_get_history_state();
	if (count >= state->length) {
		parse_error("%s: not that many history entires", av[0]);
		return(NULL);
	}
	tmp = tempnam(NULL, NULL);
	if ((fp = fopen(tmp, "w")) == NULL ) {
		parse_error("%s: unable to open temp file: %s", av[0], tmp);
		free(tmp);
		return(NULL);
	}

	for (j = count-1 ; j < state->length-2 ; j++) {
		fprintf(fp, "%s\n", state->entries[j]->line);
	}
	fclose(fp);

    if ((editor = getenv("EDITOR")) == NULL) 
        editor = "/bin/vi";

    sprintf(buf, "%s %s", editor, tmp);
    system(buf);

	fp = fopen(tmp, "r");
	unlink(tmp);
	push_input_stream(fp);

	free(tmp);
#endif
	return(NULL);
}

Var *
ff_resize(vfuncptr func, Var * arg)
{
	int ac, i, j;
	Var **av;
	Var *obj;
	int x = 1,y = 1,z = 1;

	Alist alist[5];
	alist[0] = make_alist("obj",    ID_VAL,     NULL,     &obj);
	alist[1] = make_alist("x",    	INT,    	NULL,     &x);
	alist[2] = make_alist("y",    	INT,    	NULL,     &y);
	alist[3] = make_alist("z",    	INT,    	NULL,     &z);
	alist[4].name = NULL;

	make_args(&ac, &av, func, arg);
	if (parse_args(ac, av, alist)) return(NULL);

	if (obj == NULL)  {
		parse_error("No argument specified: %s(...obj=...)", av[0]);
		return(NULL);
	}

	if (x < 0 || y < 0 || z < 0) {
		parse_error("Illegal dimensions: %dx%dx%d\n", x, y, z);
		return(NULL);
	}
	if (x*y*z != V_DSIZE(obj)) {
		parse_error("Illegal dimensions: %dx%dx%d != %d\n", 
					x, y, z, V_DSIZE(obj));
		return(NULL);
	}

	V_SIZE(obj)[orders[V_ORG(obj)][0]] = x;
	V_SIZE(obj)[orders[V_ORG(obj)][1]] = y;
	V_SIZE(obj)[orders[V_ORG(obj)][2]] = z;

	return(obj);
}

Var *
ff_fork(vfuncptr func, Var * arg)
{
#ifndef __MSDOS__
	if (fork() == 0) {
		sleep(10);
	}
#endif
	return(NULL);
}

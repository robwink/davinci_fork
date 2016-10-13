#include "ff.h"
#include "apidef.h"
#include <errno.h>
#include <math.h>
#include <string.h>

#ifdef HAVE_LIBREADLINE
#include "ff_source.h"
#endif

Var* eval_buffer(char* buf);


int num_internal_funcs = sizeof(vfunclist)/sizeof(struct _vfuncptr);



/**
 ** V_func - find and call named function
 **
 ** This is the function dispatcher.  It locates the function
 ** handler for a named function and exectues it.
 **/

Var* V_func(const char* name, Var* arg)
{
	vfuncptr f;
	UFUNC *uf, *locate_ufunc(const char *);
#ifdef INCLUDE_API
	APIDEFS* api;
#endif

	/*
	** This needs to check ALL the args to determine if there's any
	** that need to be parallelized
	*/
	/*
	  if (parallel_args(arg)) {
	  return(parallel_handler(name, arg));
	  }
	*/

	/*
	** Find and call the named function or its handler
	*/
	for (int i=0; i<num_internal_funcs; ++i) {
		f = &vfunclist[i];
		if (!strcmp(f->name, name)) {
			return (f->fptr(f, arg));
		}
	}

	/*
	** No internal function match.  Check ufunc list
	*/
	if ((uf = locate_ufunc(name)) != NULL) {
		return (dispatch_ufunc(uf, arg));
	}

#ifdef INCLUDE_API
	/*
	** Check for func in API list
	*/
	if ((api = api_lookup(name)) != NULL) {
		return (dispatch_api(api, arg));
	}
#endif

	/*
	** No function found?  Return NULL
	*/
	parse_error("Function not found: %s", name);
	return (NULL);
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

Var* ff_dfunc(vfuncptr func, Var* arg)
{
	Var *v = NULL, *s;
	dfunc fptr;
	float* fdata;
	double* ddata;

	void* data;
	int format;
	size_t dsize;
	size_t i;

	Alist alist[2];
	alist[0]      = make_alist("object", ID_VAL, NULL, &v);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);
	if (v == NULL) {
		parse_error("Not enough arguments to function: %s()", func->name);
		return (NULL);
	}

	// Find the named function.
	fptr = (dfunc)func->fdata;
	if (fptr == NULL) { /* this should never happen */
		parse_error("Function not found.");
		return (NULL);
	}

	// NOTE(rswinkle) I think we should always return double just like C precision
	format = DV_DOUBLE;
	//format = max(DV_FLOAT, V_FORMAT(v));
	dsize  = V_DSIZE(v);

	data = calloc(dsize, NBYTES(format));
	if (data == NULL) {
		parse_error("Unable to alloc %ld bytes.\n", dsize * NBYTES(format));
		return NULL;
	}

	fdata = data;
	ddata = data;
	if (format == DV_FLOAT) {
		for (i = 0; i < dsize; i++) {
			fdata[i] = (float)fptr(extract_double(v, i));
		}
	} else {
		for (i = 0; i < dsize; i++) {
			ddata[i] = fptr(extract_double(v, i));
		}
	}

	// construct a new var
	s = newVar();
	memcpy(s, v, sizeof(Var));
	V_NAME(s) = NULL;

	memcpy(V_SYM(s), V_SYM(v), sizeof(Sym));
	V_FORMAT(s)                = format;
	V_DATA(s)                  = data;
	if (V_TITLE(v)) V_TITLE(s) = strdup(V_TITLE(v));

	return s;
}

/**
 ** Worker function for bop, broken out so we can reuse it for other things.
 **/

Var* ff_binary_op(const char* name,               // Function name, for errors
                  Var* a, Var* b,                 // operands
                  double (*fptr)(double, double), // function pointer
                  int format)                     // output format
{
	int size[3];
	size_t dsize = 0;
	size_t i;
	int order;
	Var* val = NULL;
	int count;
	int va, vb;
	size_t dsizea = 1, dsizeb = 1;
	double v1, v2, v3;

	u_char* cdata;
	short* sdata;
	int* idata;
	float* fdata;
	double* ddata;

	/**
	 ** Verify that we can actually operate with these two objects.
	 **
	 ** Okay if: dimensions are the same or 1.  (size == size)
	 **
	 **/

	count = 0;
	for (i = 0; i < 3; i++) {
		va = V_SIZE(a)[orders[V_ORDER(a)][i]];
		vb = V_SIZE(b)[orders[V_ORDER(b)][i]];
		if (va != 1 && vb != 1 && va != vb) {
			parse_error("%s: operation illegal, sizes differ", name);
			return (NULL);
		}
		if (va != vb) {
			dsizea *= va;
			dsizeb *= vb;
		}
	}
	if (dsizea != 1 && dsizeb != 1) {
		parse_error("%s: operation illegal, sizes differ on more than 1 axis", name);
		return (NULL);
	}
	dsize = 1;
	for (i = 0; i < 3; i++) {
		size[i] = max(V_SIZE(a)[orders[V_ORDER(a)][i]], V_SIZE(b)[orders[V_ORDER(b)][i]]);
		dsize *= size[i];
	}
	order = (V_DSIZE(a) > V_DSIZE(b) ? V_ORDER(a) : V_ORDER(b));

	if (dsize == 0) dsize = 1; /* impossible? */

	/**
	 ** can we reuse one of the input values here?
	 **/

	// Initalize the return object

	val           = newVar();
	V_TYPE(val)   = ID_VAL;
	V_FORMAT(val) = format;
	V_DSIZE(val)  = dsize;
	V_ORDER(val)  = order;

	/** size was extracted as x,y,z.  put it back appropriately **/

	V_SIZE(val)[orders[order][0]] = size[0];
	V_SIZE(val)[orders[order][1]] = size[1];
	V_SIZE(val)[orders[order][2]] = size[2];

	V_DATA(val) = (double*)calloc(dsize, NBYTES(format));
	if (V_DATA(val) == NULL) {
		parse_error("Unable to alloc %ld bytes.\n", dsize * NBYTES(format));
		return NULL;
	}

	cdata = (u_char*)V_DATA(val);
	sdata = (short*)V_DATA(val);
	idata = (int*)V_DATA(val);
	fdata = (float*)V_DATA(val);
	ddata = (double*)V_DATA(val);

	/**
	 ** For each output element (0-size), de-compute relative position using
	 ** order, and re-compute offset to that element in the other var.
	 **/
	for (i = 0; i < dsize; i++) {
		v1 = extract_double(a, rpos(i, val, a));
		v2 = extract_double(b, rpos(i, val, b));
		v3 = (*fptr)(v1, v2);
		switch (format) {
		case DV_UINT8: cdata[i]  = clamp_byte(v3); break;
		case DV_INT16: sdata[i]  = clamp_short(v3); break;
		case DV_INT32: idata[i]  = clamp_int(v3); break;
		case DV_FLOAT: fdata[i]  = clamp_float(v3); break;
		case DV_DOUBLE: ddata[i] = v3; break;
		}
	}
	return (val);
}

/**
 ** binary operator function, double
 **/

Var* ff_bop(vfuncptr func, Var* arg)
{
	Var *a = NULL, *b = NULL;
	ddfunc fptr = (ddfunc)func->fdata;

	Alist alist[3];
	alist[0]      = make_alist("object", ID_VAL, NULL, &a);
	alist[1]      = make_alist("object", ID_VAL, NULL, &b);
	alist[2].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (a == NULL || b == NULL) {
		parse_error("Not enough arguments to function: %s()", func->name);
		return (NULL);
	}

	if (fptr == NULL) { /* this should never happen */
		parse_error("Function not found: %s.", func->name);
		return (NULL);
	}

	return (ff_binary_op(func->name, a, b, fptr, DV_DOUBLE));
}

/**
 ** convert organization
 **/

Var* ff_org(vfuncptr func, Var* arg)
{
	Var *s = NULL, *ob = NULL;
	size_t i, j;
	void *from = NULL, *to = NULL;
	int org = -1, nbytes, format;
	size_t dsize;
	char* org_str      = NULL;
	const char* orgs[] = {"bsq", "bil", "bip", "xyz", "xzy", "zxy", NULL};

	Alist alist[4];
	alist[0]      = make_alist("object", ID_VAL, NULL, &ob);
	alist[1]      = make_alist("org", ID_ENUM, orgs, &org_str);
	alist[2]      = make_alist("order", ID_ENUM, orgs, &org_str);
	alist[3].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (ob == NULL) {
		parse_error("'object' not found");
		return (NULL);
	}

	if (func->fdata != NULL) {
		org = (long)(func->fdata) - 10;
	} else {
		if (org_str == NULL) {
			/**
			 ** no order specified.  Print out the org of the passed object.
			 **/
			s           = newVar();
			V_TYPE(s)   = ID_STRING;
			V_STRING(s) = strdup(Org2Str(V_ORG(ob)));
			return (s);
		} else {
			if (!strcasecmp(org_str, "bsq"))
				org = BSQ;
			else if (!strcasecmp(org_str, "xyz"))
				org = BSQ;
			else if (!strcasecmp(org_str, "bil"))
				org = BIL;
			else if (!strcasecmp(org_str, "xzy"))
				org = BIL;
			else if (!strcasecmp(org_str, "bip"))
				org = BIP;
			else if (!strcasecmp(org_str, "zxy"))
				org = BIP;
		}
	}

	/**
	 ** create output variable
	 **/
	dsize     = V_DSIZE(ob);
	format    = V_FORMAT(ob);
	s         = newVar();
	V_TYPE(s) = ID_VAL;
	memcpy(V_SYM(s), V_SYM(ob), sizeof(Sym));
	V_DATA(s) = calloc(dsize, NBYTES(format));
	if (V_DATA(s) == NULL) {
		parse_error("Unable to allocate %ld bytes: %s\n", dsize * NBYTES(format), strerror(errno));
		return (NULL);
	}
	V_ORG(s) = org;

	for (i = 0; i < 3; i++) {
		V_SIZE(s)[orders[org][i]] = V_SIZE(ob)[orders[V_ORG(ob)][i]];
	}

	nbytes = NBYTES(format);
	from   = V_DATA(ob);
	to     = V_DATA(s);
	for (i = 0; i < V_DSIZE(s); i++) {
		j = rpos(i, ob, s);
		memcpy(((char*)to) + (j * nbytes), ((char*)from) + (i * nbytes), nbytes);
	}
	return (s);
}

Var* ff_conv(vfuncptr func, Var* arg)
{
	int format;
	size_t dsize;
	void* data;
	Var *v = NULL, *s;
	size_t i;

	/**
	 ** converting to type stored in vfunc->fdata.
	 **/

	Alist alist[2];
	alist[0]      = make_alist("object", ID_VAL, NULL, &v);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);
	if (v == NULL) {
		parse_error("Not enough arguments to function: %s()", func->name);
		return (NULL);
	}

	format = (intptr_t)func->fdata;
	dsize  = V_DSIZE(v);
	data   = calloc(dsize, NBYTES(format));
	if (data == NULL) {
		parse_error("Unable to allocate %ld bytes: %s\n", dsize * NBYTES(format), strerror(errno));
		return (NULL);
	}

	u8* u8data = data;
	u16* u16data = data;
	u32* u32data = data;
	u64* u64data = data;

	i8* i8data = data;
	i16* i16data = data;
	i32* i32data = data;
	i64* i64data = data;

	float* fdata = data;
	double* ddata = data;

	u64 tmp_u64;
	i64 tmp_i64;

	for (i=0; i<dsize; ++i) {
		switch (format) {
		case DV_UINT8:
			tmp_u64 = extract_u64(v, i);
			u8data[i] = clamp_u8(tmp_u64);
			break;
		case DV_UINT16:
			tmp_u64 = extract_u64(v, i);
			u16data[i] = clamp_u16(tmp_u64);
			break;
		case DV_UINT32:
			tmp_u64 = extract_u64(v, i);
			u32data[i] = clamp_u32(tmp_u64);
			break;
		case DV_UINT64:
			u64data[i] = extract_u64(v, i);
			break;

		case DV_INT8:
			tmp_i64 = extract_i64(v, i);
			i8data[i] = clamp_i8(tmp_i64);
			break;
		case DV_INT16:
			tmp_i64 = extract_i64(v, i);
			i16data[i] = clamp_i16(tmp_i64);
			break;
		case DV_INT32:
			tmp_i64 = extract_i64(v, i);
			i32data[i] = clamp_i32(tmp_i64);
			break;
		case DV_INT64:
			i64data[i] = extract_i64(v, i);
			break;

		case DV_FLOAT:
			fdata[i] = extract_float(v, i);
			break;
		case DV_DOUBLE:
			ddata[i] = extract_double(v, i);
			break;
		}
	}

	// Make the new var
	s = newVar();
	memcpy(s, v, sizeof(Var));
	V_NAME(s) = NULL;
	;

	memcpy(V_SYM(s), V_SYM(v), sizeof(Sym));

	V_DATA(s)   = data;
	V_FORMAT(s) = format;

	return (s);
}

/**
 ** ff_dim()     - return the dimensions of arg
 **/

Var* ff_dim(vfuncptr func, Var* arg)
{
	Var* v = NULL;
	u64* iptr;

	Alist alist[2];
	alist[0]      = make_alist("object", ID_VAL, NULL, &v);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);
	if (v == NULL) {
		parse_error("Not enough arguments to function: %s()", func->name);
		return (NULL);
	}

	// NOTE(rswinkle) dimensions are actually stored as size_t so unless we add
	// a DV_SIZE_T type and handle it everywhere we just have to use u64 which is the same
	// as size_t on 64 bit but overkill obviously on 32 bit
	iptr = (u64*)calloc(3, sizeof(u64));

	iptr[0] = GetSamples(V_SIZE(v), V_ORG(v));
	iptr[1] = GetLines(V_SIZE(v), V_ORG(v));
	iptr[2] = GetBands(V_SIZE(v), V_ORG(v));

	return newVal(BSQ, 3, 1, 1, DV_UINT64, iptr);
}

/**
 ** ff_format()  - return the format of arg
 **/

Var* ff_format(vfuncptr func, Var* arg)
{
	Var *s, *obj = NULL;
	char* ptr             = NULL;
	char* type            = NULL;
	char* format_str      = NULL;

	Alist alist[4];
	alist[0]      = make_alist("object", ID_UNK, NULL, &obj);

	// TODO(rswinkle) This is stupid and inconsistent with other "format" taking functions.
	// allows doing this
	//
	// format(obj, type=byte, format=int)
	//
	// and uses the last given, ie int in this case
	alist[1]      = make_alist("format", ID_ENUM, FORMAT_STRINGS, &format_str);
	alist[2]      = make_alist("type", ID_ENUM, FORMAT_STRINGS, &format_str);

	alist[3].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (obj == NULL) {
		parse_error("No object specified: %s()", func->name);
		return (NULL);
	}

	if ((s = eval(obj)) != NULL) obj = s;

	if (format_str == NULL) {
		switch (V_TYPE(obj)) {
		case ID_VAL:
			/**
			 ** no format specified.  Print out the format of the passed object.
			 **/
			type = Format2Str(V_FORMAT(obj));
			break;
		case ID_STRING: type = (char*)"STRING"; break;
		case ID_STRUCT: type = (char*)"STRUCT"; break;
		case ID_TEXT: type   = (char*)"TEXT"; break;

		default: type = (char*)"UNDEFINED"; break;
		}
		return (newString(strdup(type)));
	} else {

		if (!strcasecmp(format_str, "byte"))
			ptr = (char*)"byte";
		else if (!strcasecmp(format_str, "short"))
			ptr = (char*)"short";
		else if (!strcasecmp(format_str, "int"))
			ptr = (char*)"int";
		else if (!strcasecmp(format_str, "float"))
			ptr = (char*)"float";
		else if (!strcasecmp(format_str, "double"))
			ptr = (char*)"double";

		return (V_func(ptr, create_args(1, NULL, obj, NULL, NULL)));
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

Var* ff_create(vfuncptr func, Var* arg)
{
	Var* s;
	size_t dsize, count = 0, c;
	size_t i, j, k;
	double v;

	// TODO(rswinkle) combine/with use globals.c ORG2STR array?
	const char* orgs[]    = {"bsq", "bil", "bip", "xyz", "xzy", "zxy", NULL};


	// NOTE(rswinkle) question is do we default to INT64 or not, or base it on the platform
	// and this decision affects at least a couple other functions, ie make_sym.
	// Personally, I think we should officially drop 32-bit support.  The Linux distros have been
	// recommending using amd64 for computers with >2GB of memory for years (who doesn't have > 2GB of
	// memory?) and several are going to stop making i386 releases entirely by 2020
	//
	// Also x,y,z are size_t in other funcs but we don't handle size_t in parse_args.  Fortunately
	// size_t is a u64 on 64 bit.  We could add size_t to parse_args if wanted.
	u64 x = 1, y = 1, z = 1;

	// default format if format_not given
	int format            = DV_INT32;

	int org          = BSQ;
	double start     = 0;
	double step      = 1.0;
	int init         = 1;
	char *format_str = NULL, *org_str = NULL;

	u8* u8data;
	u16* u16data;
	u32* u32data;
	u64* u64data;

	i8* i8data;
	i16* i16data;
	i32* i32data;
	i64* i64data;

	float* fdata;
	double* ddata;

	Alist alist[9];
	alist[0]      = make_alist("x", DV_UINT64, NULL, &x);
	alist[1]      = make_alist("y", DV_UINT64, NULL, &y);
	alist[2]      = make_alist("z", DV_UINT64, NULL, &z);
	alist[3]      = make_alist("org", ID_ENUM, orgs, &org_str);
	alist[4]      = make_alist("format", ID_ENUM, FORMAT_STRINGS, &format_str);
	alist[5]      = make_alist("start", DV_DOUBLE, NULL, &start);
	alist[6]      = make_alist("step", DV_DOUBLE, NULL, &step);
	alist[7]      = make_alist("init", DV_INT32, NULL, &init);
	alist[8].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	dsize = x * y * z;

	if (org_str != NULL) {
		if (!strcasecmp(org_str, "bsq"))
			org = BSQ;
		else if (!strcasecmp(org_str, "xyz"))
			org = BSQ;
		else if (!strcasecmp(org_str, "bil"))
			org = BIL;
		else if (!strcasecmp(org_str, "xzy"))
			org = BIL;
		else if (!strcasecmp(org_str, "bip"))
			org = BIP;
		else if (!strcasecmp(org_str, "zxy"))
			org = BIP;
	}

	// format_str will never be invalid because parse_args would have failed
	// so format will never be set to -1;
	if (format_str) {
		format = dv_str_to_format(format_str);
	}

	// can't be negative cause they're unsigned
	if (!x || !y || !z) {
		parse_error("create(): invalid dimensions: %zux%zux%zu\n", x, y, z);
		return NULL;
	}

	s         = newVar();
	V_TYPE(s) = ID_VAL;

	V_DATA(s) = calloc(dsize, NBYTES(format));
	if (V_DATA(s) == NULL) {
		memory_error(errno, dsize * NBYTES(format));
		return NULL;
	}
	V_FORMAT(s) = format;
	V_ORDER(s)  = org;
	V_DSIZE(s)  = dsize;

	V_SIZE(s)[orders[org][0]] = x;
	V_SIZE(s)[orders[org][1]] = y;
	V_SIZE(s)[orders[org][2]] = z;

	// casts from void* aren't required in C ...
	u8data = (u8*)V_DATA(s);
	u16data = (u16*)V_DATA(s);
	u32data = (u32*)V_DATA(s);
	u64data = (u64*)V_DATA(s);

	i8data = (i8*)V_DATA(s);
	i16data = (i16*)V_DATA(s);
	i32data = (i32*)V_DATA(s);
	i64data = (i64*)V_DATA(s);

	fdata = (float*)V_DATA(s);
	ddata = (double*)V_DATA(s);

	if (init) {
		if (step == 0) {

			//unecessary check
			if (dsize > 0) {
				unsigned char* data = V_DATA(s);
				unsigned int nbytes = NBYTES(format);
				size_t i;

				v = start;
				switch (format) {
				case DV_UINT8: u8data[0]  = clamp_u8(v); break;
				case DV_UINT16: u16data[0]  = clamp_u16(v); break;
				case DV_UINT32: u32data[0]  = clamp_u32(v); break;
				case DV_UINT64: u64data[0]  = clamp_u64(v); break;

				case DV_INT8: i8data[0]  = clamp_i8(v); break;
				case DV_INT16: i16data[0]  = clamp_i16(v); break;
				case DV_INT32: i32data[0]  = clamp_i32(v); break;
				case DV_INT64: i64data[0]  = clamp_i64(v); break;

				case DV_FLOAT: fdata[0]  = clamp_float(v); break;
				case DV_DOUBLE: ddata[0] = v; break;
				}
				for (i = 1; i < dsize; i++) {
					memcpy(data + i * nbytes, data, nbytes);
				}
			}
		} else {
			for (k = 0; k < z; k++) {
				for (j = 0; j < y; j++) {
					for (i = 0; i < x; i++) {
						v = (count++) * step + start;
						c = cpos(i, j, k, s);
						switch (format) {

						case DV_UINT8: u8data[c]  = clamp_u8(v); break;
						case DV_UINT16: u16data[c]  = clamp_u16(v); break;
						case DV_UINT32: u32data[c]  = clamp_u32(v); break;
						case DV_UINT64: u64data[c]  = clamp_u64(v); break;

						case DV_INT8: i8data[c]  = clamp_i8(v); break;
						case DV_INT16: i16data[c]  = clamp_i16(v); break;
						case DV_INT32: i32data[c]  = clamp_i32(v); break;
						case DV_INT64: i64data[c]  = clamp_i64(v); break;

						case DV_FLOAT: fdata[c]  = clamp_float(v); break;
						case DV_DOUBLE: ddata[c] = v; break;
						}
					}
				}
			}
		}
	}
	return (s);
}

Var* ff_nop(vfuncptr func, Var* arg)
{
	return (NULL);
}

Var* ff_echo(vfuncptr func, Var* arg)
{
	int t    = VERBOSE;
	Var* obj = NULL;
	Alist alist[2];
	alist[0]      = make_alist("obj", ID_UNK, NULL, &obj);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	/**
	 ** The cheap version.  Should expand this for proper use someday.
	 **/
	VERBOSE = 1;

	if (obj == NULL) {
		parse_error("echo(): null argument passed");
		return (NULL);
	}

	pp_print(obj);
	VERBOSE = t;
	return (NULL);
}

/**
 ** Copy an object onto itself N times, in each direction specified.
 **/
Var* replicate_text(Var* ob, int x, int y)
{
	Var* s;
	int i, j, k;

	char *str, *out;
	int len;
	int rows;

	if (V_TYPE(ob) == ID_STRING && y == 1) {
		// String, 1 line high, output is a string.
		str = V_STRING(ob);
		len = strlen(str);
		out = calloc(len * x + 1, sizeof(char));
		for (i = 0; i < x; i++) {
			memcpy(&(out[i * len]), str, len + 1);
		}
		return (newString(out));
	} else {
		if (V_TYPE(ob) == ID_STRING) {
			rows = 1;
			str  = V_STRING(ob);
		} else {
			rows = V_TEXT(ob).Row;
			str  = V_TEXT(ob).text[0];
		}

		s           = newText(rows * y, calloc(rows * y, sizeof(char*)));
		char** text = V_TEXT(s).text;

		// Make a set that's full width first.
		for (j = 0; j < rows; j++) {
			if (rows != 1) {
				// Input is string (or possibly, a text of size 1)
				str = V_TEXT(ob).text[j];
			}
			len     = strlen(str);
			text[j] = (char*)calloc(len * x + 1, sizeof(char));
			for (i = 0; i < x; i++) {
				// Len+1 here gets the null terminator
				memcpy(text[j] + i * len, str, len + 1);
			}
		}

		// Replicate each row the appropriate number of times.
		for (j = 0; j < rows; j++) {
			str = text[j];
			len = strlen(str);
			for (k = 1; k < y; k++) {
				text[k * rows + j] = calloc(len + 1, sizeof(char));
				memcpy(text[k * rows + j], str, len + 1);
			}
		}
		return (s);
	}
}

Var* ff_replicate(vfuncptr func, Var* arg)
{
	int x = 1, y = 1, z = 1;
	int org;
	Var *v = NULL, *s;

	void *data1, *data2, *d1, *d2;
	size_t d1ptr, d2ptr, dptr;
	size_t dsize;
	size_t len[3];
	size_t size[3];
	int nbytes;
	size_t i, j, k;
	size_t l;

	Alist alist[5];
	alist[0]      = make_alist("object", ID_UNK, NULL, &v);
	alist[1]      = make_alist("x", DV_INT32, NULL, &x);
	alist[2]      = make_alist("y", DV_INT32, NULL, &y);
	alist[3]      = make_alist("z", DV_INT32, NULL, &z);
	alist[4].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (v == NULL) {
		parse_error("clone: No object specified\n");
		return (NULL);
	}

	if (V_TYPE(v) != ID_VAL && V_TYPE(v) != ID_TEXT && V_TYPE(v) != ID_STRING) {
		parse_error("Invalid replication object");
		return (NULL);
	}

	if (V_TYPE(v) == ID_TEXT || V_TYPE(v) == ID_STRING) {
		return (replicate_text(v, x, y));
	}

	org = V_ORG(v);

	if (x <= 0 || y <= 0 || z <= 0) {
		parse_error("Bad dimension value");
		return (NULL);
	}

	len[orders[org][0]] = x;
	len[orders[org][1]] = y;
	len[orders[org][2]] = z;

	dsize   = V_DSIZE(v) * x * y * z;
	data2   = calloc(NBYTES(V_FORMAT(v)), dsize);
	data1   = V_DATA(v);
	nbytes  = NBYTES(V_FORMAT(v));
	size[0] = V_SIZE(v)[0];
	size[1] = V_SIZE(v)[1];
	size[2] = V_SIZE(v)[2];

	l = size[0] * nbytes;

	for (k = 0; k < size[2]; k++) {
		d1ptr = k * size[0] * size[1] * nbytes;
		d2ptr = d1ptr * len[0] * len[1];

		d1 = (char*)data1 + d1ptr;
		d2 = (char*)data2 + d2ptr;

		dptr = 0;
		for (i = 0; i < size[1]; i++) {
			for (j = 0; j < len[0]; j++) {
				memcpy((void*)((char*)d2 + dptr), (void*)((char*)d1 + i * l), l);
				dptr += l;
			}
		}
		for (i = 1; i < len[1]; i++) {
			memcpy((void*)((char*)d2 + dptr * i), d2, dptr);
		}
	}

	l = size[2] * size[1] * size[0] * len[1] * len[0] * nbytes;
	for (i = 1; i < len[2]; i++) {
		memcpy((void*)((char*)data2 + (l * i)), data2, l);
	}

	s            = newVar();
	V_TYPE(s)    = ID_VAL;
	V_DATA(s)    = data2;
	V_DSIZE(s)   = dsize;
	V_SIZE(s)[0] = V_SIZE(v)[0] * len[0];
	V_SIZE(s)[1] = V_SIZE(v)[1] * len[1];
	V_SIZE(s)[2] = V_SIZE(v)[2] * len[2];
	V_FORMAT(s)  = V_FORMAT(v);
	V_ORG(s)     = V_ORG(v);

	return (s);
}


/**
 ** Takes two objects, with the same ORG, FORMAT, and two matching axis,
 ** and concatenate them together along specified axis.
 **/
Var* ff_cat(vfuncptr func, Var* arg)
{
	int ac;
	Var** av;
	Var *axis_var = NULL, *p, *q, *v;
	char* axis_str;
	int i, j, axis;

	// TODO(rswinkle): Why aren't we using alist + parse_args() here?
	//
	// UPDATE(rswinkle): Ok as far as I can tell the only reason
	// we don't use parse_args() here is because it can't handle variable
	// numbers of arguments...
	//
	// cat() is documented as only taking 2 arrays on both the wiki
	// and dv.gih
	// cat(array1, array2, axis)
	//
	// nothing about arbitrary numbers of arrays
	//
	// Looks like maybe a few places in the library do we use cat() with 
	// more than 2 arrays.  Maybe there's a way to update parse_args to
	// handle this?  future research I guess
	make_args(&ac, &av, func, arg);

	if (ac < 4) {
		parse_error("Not enough arguments to cat()");
		free(av);
		return (NULL);
	}
	/* find axis if specified */
	for (i = 1; i < ac; i++) {
		if (V_TYPE(av[i]) == ID_KEYWORD && V_NAME(av[i]) != NULL &&
		    !strcasecmp(V_NAME(av[i]), "axis")) {

			axis_var = V_KEYVAL(av[i]);
			for (j = i; j < ac - 1; j++) {
				av[j] = av[j + 1];
			}
			ac--;
			break;
		}
	}
	if (axis_var == NULL) {
		axis_var = av[ac - 1];
		ac--;
	}

	if (V_TYPE(axis_var) == ID_STRING) {
		axis_str = V_STRING(axis_var);
	} else if ((v = eval(axis_var)) != NULL && V_TYPE(v) == ID_STRING) {
		axis_str = V_STRING(v);
	} else if (V_NAME(axis_var) != NULL) {
		axis_str = V_NAME(axis_var);
	} else {
		parse_error("cat(): No axis specified");
		free(av);
		return (NULL);
	}
	if (!strcasecmp(axis_str, "x"))
		axis = 0;
	else if (!strcasecmp(axis_str, "y"))
		axis = 1;
	else if (!strcasecmp(axis_str, "z"))
		axis = 2;
	else {
		parse_error("cat(): Invalid axis specified");
		free(av);
		return (NULL);
	}

	p = av[1];
	for (i = 2; i < ac; i++) {
		q = do_cat(p, av[i], axis);
		if (q == NULL) {
			free(av);
			return (NULL);
		}
		p = q;
	}

	free(av);
	return (p);
}

Var* cat_mixed_text(Var* ob1, Var* ob2, int axis)
{
	Var* s = newVar();
	int Row;
	int i;
	char* string;
	char** text;

	V_TYPE(s) = ID_TEXT; /* Text+String=Text; String+Text=Text; either way...*/

	if (V_TYPE(ob1) == ID_TEXT) { /*Left or Top is text*/
		Row    = V_TEXT(ob1).Row;
		string = V_STRING(ob2);
		text   = V_TEXT(ob1).text;
		if (axis == 0) { /*Left, same # of Rows*/
			V_TEXT(s).Row  = Row;
			V_TEXT(s).text = (char**)calloc(sizeof(char*), Row);
			for (i = 0; i < Row; i++) {
				V_TEXT(s).text[i] = (char*)calloc(sizeof(char), strlen(text[i]) + strlen(string));
				strcpy(V_TEXT(s).text[i], text[i]);
				strcat(V_TEXT(s).text[i], string);
			}
		} else { /*Top, w/string as extra row on bottom*/
			Row++;
			V_TEXT(s).Row  = Row;
			V_TEXT(s).text = (char**)calloc(sizeof(char*), Row);
			for (i = 0; i < Row - 1; i++) {
				V_TEXT(s).text[i] = strdup(text[i]);
			}
			V_TEXT(s).text[i] = strdup(string);
		}
	}

	else { /*Right or Bottom is Text*/
		Row    = V_TEXT(ob2).Row;
		string = V_STRING(ob1);
		text   = V_TEXT(ob2).text;

		if (axis == 0) { /*Right, same # of Row*/
			V_TEXT(s).Row  = Row;
			V_TEXT(s).text = (char**)calloc(sizeof(char*), Row);
			for (i = 0; i < Row; i++) {
				V_TEXT(s).text[i] = (char*)calloc(sizeof(char), strlen(text[i]) + strlen(string));
				strcpy(V_TEXT(s).text[i], string);
				strcat(V_TEXT(s).text[i], text[i]);
			}
		}

		else { /*Bottom, w/string as extra row on top*/
			Row++;
			V_TEXT(s).Row     = Row;
			V_TEXT(s).text    = (char**)calloc(sizeof(char*), Row);
			V_TEXT(s).text[0] = strdup(string);
			for (i = 1; i < Row; i++) {
				V_TEXT(s).text[i] = strdup(text[i - 1]);
			}
		}
	}

	return (s);
}

Var* cat_string_text(Var* ob1, Var* ob2, int axis)
{
	Var* s = newVar();

	if (axis == 2) { /*Can't cat Text or Strings in Z*/
		parse_error("Invalid axis specified");
		return (NULL);
	}

	if (V_TYPE(ob1) != V_TYPE(ob2)) { /*mixing strings and text!*/
		return (cat_mixed_text(ob1, ob2, axis));
	}

	if (axis == 1 && V_TYPE(ob1) == ID_STRING) { /*String->Text*/
		V_TYPE(s)         = ID_TEXT;
		V_TEXT(s).Row     = 2;
		V_TEXT(s).text    = (char**)calloc(sizeof(char*), 2);
		V_TEXT(s).text[0] = strdup(V_STRING(ob1));
		V_TEXT(s).text[1] = strdup(V_STRING(ob2));
		return (s);
	}

	else if (axis == 1) {
		int i;
		int counter = 0;
		int Row1, Row2;
		V_TYPE(s)      = ID_TEXT;
		Row1           = V_TEXT(ob1).Row;
		Row2           = V_TEXT(ob2).Row;
		V_TEXT(s).Row  = Row1 + Row2;
		V_TEXT(s).text = (char**)calloc(sizeof(char*), V_TEXT(s).Row);
		for (i = 0; i < Row1; i++) {
			V_TEXT(s).text[counter++] = strdup(V_TEXT(ob1).text[i]);
		}
		for (i = 0; i < Row2; i++) {
			V_TEXT(s).text[counter++] = strdup(V_TEXT(ob2).text[i]);
		}
		return (s);
	}

	else if (V_TYPE(ob1) == ID_STRING) {
		V_TYPE(s)   = ID_STRING;
		V_STRING(s) = (char*)calloc(sizeof(char), strlen(V_STRING(ob1)) + strlen(V_STRING(ob2)) + 1);
		strcpy(V_STRING(s), V_STRING(ob1));
		strcat(V_STRING(s), V_STRING(ob2));

		return (s);
	}

	else {
		int i;
		int Row;
		if (V_TEXT(ob1).Row != V_TEXT(ob2).Row) {
			parse_error("Objects must have equal number of rows");
			return (NULL);
		}

		V_TYPE(s)      = ID_TEXT;
		Row            = V_TEXT(ob1).Row;
		V_TEXT(s).Row  = Row;
		V_TEXT(s).text = (char**)calloc(sizeof(char*), Row);
		for (i = 0; i < Row; i++) {
			V_TEXT(s)
			    .text[i] = (char*)calloc(sizeof(char), strlen(V_TEXT(ob1).text[i]) +
			                                               strlen(V_TEXT(ob2).text[i]) + 1);
			strcpy(V_TEXT(s).text[i], V_TEXT(ob1).text[i]);
			strcat(V_TEXT(s).text[i], V_TEXT(ob2).text[i]);
		}

		return (s);
	}

	return (NULL);
}

Var* do_cat(Var* ob1, Var* ob2, int axis)
{
	Var *e, *s;
	void *data, *d1, *d2, *out;
	size_t dsize;
	int nbytes;
	int ob1_type, ob2_type;

	if ((e = eval(ob1)) != NULL) ob1 = e;
	if ((e = eval(ob2)) != NULL) ob2 = e;
	if (ob1 == NULL) return (ob2);
	if (ob2 == NULL) return (ob1);

	ob1_type = V_TYPE(ob1);
	ob2_type = V_TYPE(ob2);

	if (ob1_type != ID_VAL && ob1_type != ID_STRING && ob1_type != ID_TEXT) {
		parse_error("cat(), improper object specified");
		return (NULL);
	}

	if (ob2_type != ID_VAL && ob2_type != ID_STRING && ob2_type != ID_TEXT) {
		parse_error("cat(), improper object specified");
		return (NULL);
	}

	if ((ob1_type == ID_VAL && ob2_type != ID_VAL) || (ob1_type != ID_VAL && ob2_type == ID_VAL)) {
		parse_error("cat(), Can't mix numbers with non-numbers!");
		return (NULL);
	}

	if (ob1_type == ID_STRING || ob1_type == ID_TEXT) return (cat_string_text(ob1, ob2, axis));

	int out_format = combine_var_formats(ob1, ob2);

	size_t i, j, k;
	for (i = 0; i < 3; i++) {
		if (i == axis) {
			continue;
		}

		// stupid davinci making the size array order change with the org
		if (V_SIZE(ob1)[orders[V_ORG(ob1)][i]] != V_SIZE(ob2)[orders[V_ORG(ob2)][i]]) {

			parse_error("Unspecified axes must match");
			return (NULL);
		}
	}

	nbytes = NBYTES(V_FORMAT(ob1));
	nbytes = NBYTES(out_format);
	d1     = V_DATA(ob1);
	d2     = V_DATA(ob2);
	dsize  = V_DSIZE(ob1) + V_DSIZE(ob2);
	data   = calloc(nbytes, dsize);
	out    = data;
	s            = newVar();
	V_TYPE(s)    = ID_VAL;
	V_DATA(s)    = data;
	V_FORMAT(s)  = out_format;
	V_ORG(s)     = V_ORG(ob1);
	V_DSIZE(s)   = dsize;
	V_SIZE(s)[0] = V_SIZE(ob1)[0];
	V_SIZE(s)[1] = V_SIZE(ob1)[1];
	V_SIZE(s)[2] = V_SIZE(ob1)[2];

	// replace the axis that changed
	//
	// find "location" of specificed axis (see array.c)
	// stupid davinci madness
	int tmp_axis = orders[V_ORG(ob1)][axis];

	V_SIZE(s)[tmp_axis] = V_SIZE(ob1)[tmp_axis] + V_SIZE(ob2)[orders[V_ORG(ob2)][axis]];

	u8* u8data = data;
	u16* u16data = data;
	u32* u32data = data;
	u64* u64data = data;

	i8* i8data = data;
	i16* i16data = data;
	i32* i32data = data;
	i64* i64data = data;

	float* fdata = data;
	double* ddata = data;

	size_t idx, oidx;
	for (i=0; i<GetZ(ob1); ++i) {
		for (j=0; j<GetY(ob1); ++j) {
			for (k=0; k<GetX(ob1); ++k) {
				idx = cpos(k, j, i, ob1);
				oidx = cpos(k, j, i, s);
				
				switch (out_format) {
				case DV_UINT8:  u8data[oidx] = extract_u32(ob1, idx); break;
				case DV_UINT16: u16data[oidx] = extract_u32(ob1, idx); break;
				case DV_UINT32: u32data[oidx] = extract_u32(ob1, idx); break;
				case DV_UINT64: u64data[oidx] = extract_u64(ob1, idx); break;

				case DV_INT8:   u8data[oidx] =  extract_i32(ob1, idx); break;
				case DV_INT16:  u16data[oidx] = extract_i32(ob1, idx); break;
				case DV_INT32:  u32data[oidx] = extract_i32(ob1, idx); break;
				case DV_INT64:  u64data[oidx] = extract_i64(ob1, idx); break;

				case DV_FLOAT:  fdata[oidx] = extract_float(ob1, idx); break;
				case DV_DOUBLE: ddata[oidx] =  extract_double(ob1, idx); break;
				}

			}
			if (axis == 0) {
				for (k=0; k<GetX(ob2); ++k) {
					idx = cpos(k, j, i, ob2);
					oidx = cpos(k+GetX(ob1), j, i, s);
					
					switch (out_format) {
					case DV_UINT8:  u8data[oidx] = extract_u32(ob2, idx); break;
					case DV_UINT16: u16data[oidx] = extract_u32(ob2, idx); break;
					case DV_UINT32: u32data[oidx] = extract_u32(ob2, idx); break;
					case DV_UINT64: u64data[oidx] = extract_u64(ob2, idx); break;

					case DV_INT8:   u8data[oidx] =  extract_i32(ob2, idx); break;
					case DV_INT16:  u16data[oidx] = extract_i32(ob2, idx); break;
					case DV_INT32:  u32data[oidx] = extract_i32(ob2, idx); break;
					case DV_INT64:  u64data[oidx] = extract_i64(ob2, idx); break;

					case DV_FLOAT:  fdata[oidx] = extract_float(ob2, idx); break;
					case DV_DOUBLE: ddata[oidx] =  extract_double(ob2, idx); break;
					}
				}
			}
		}

		if (axis == 1) {
			for (j=0; j<GetY(ob2); ++j) {
				for (k=0; k<GetX(ob1); ++k) {
					idx = cpos(k, j, i, ob2);
					oidx = cpos(k, j+GetY(ob1), i, s);

					switch (out_format) {
					case DV_UINT8:  u8data[oidx] = extract_u32(ob2, idx); break;
					case DV_UINT16: u16data[oidx] = extract_u32(ob2, idx); break;
					case DV_UINT32: u32data[oidx] = extract_u32(ob2, idx); break;
					case DV_UINT64: u64data[oidx] = extract_u64(ob2, idx); break;

					case DV_INT8:   u8data[oidx] =  extract_i32(ob2, idx); break;
					case DV_INT16:  u16data[oidx] = extract_i32(ob2, idx); break;
					case DV_INT32:  u32data[oidx] = extract_i32(ob2, idx); break;
					case DV_INT64:  u64data[oidx] = extract_i64(ob2, idx); break;

					case DV_FLOAT:  fdata[oidx] = extract_float(ob2, idx); break;
					case DV_DOUBLE: ddata[oidx] =  extract_double(ob2, idx); break;
					}
				}
			}
		}
	}
	
	if (axis == 2) {
		for (i=0; i<GetZ(ob2); ++i) {
			for (j=0; j<GetY(ob1); ++j) {
				for (k=0; k<GetX(ob1); ++k) {
					idx = cpos(k, j, i, ob2);
					oidx = cpos(k, j, i+GetZ(ob1), s);

					switch (out_format) {
					case DV_UINT8:  u8data[oidx] = extract_u32(ob2, idx); break;
					case DV_UINT16: u16data[oidx] = extract_u32(ob2, idx); break;
					case DV_UINT32: u32data[oidx] = extract_u32(ob2, idx); break;
					case DV_UINT64: u64data[oidx] = extract_u64(ob2, idx); break;

					case DV_INT8:   u8data[oidx] =  extract_i32(ob2, idx); break;
					case DV_INT16:  u16data[oidx] = extract_i32(ob2, idx); break;
					case DV_INT32:  u32data[oidx] = extract_i32(ob2, idx); break;
					case DV_INT64:  u64data[oidx] = extract_i64(ob2, idx); break;

					case DV_FLOAT:  fdata[oidx] = extract_float(ob2, idx); break;
					case DV_DOUBLE: ddata[oidx] =  extract_double(ob2, idx); break;
					}
				}
			}
		}

	}

	return (s);
}


/**
 ** convert bytes to string
 **/

Var* ff_string(vfuncptr func, Var* arg)
{
	Var *v = NULL, *s;

	Alist alist[2];
	alist[0]      = make_alist("object", ID_VAL, NULL, &v);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (v == NULL) {
		parse_error("Not enough arguments to function: %s()", func->name);
		return (NULL);
	}
	if (V_FORMAT(v) != DV_UINT8) {
		parse_error("%s(), argument must be DV_UINT8 format");
		return (NULL);
	}

	s           = newVar();
	V_TYPE(s)   = ID_STRING;
	V_STRING(s) = (char*)memcpy(calloc(V_DSIZE(v) + 1, 1), V_DATA(v), V_DSIZE(v));
	return (s);
}

Var* ff_issubstring(vfuncptr func, Var* arg)
{
	char *S1 = NULL, *S2 = NULL;
	int* Result = (int*)calloc(1, sizeof(int));

	Alist alist[3];
	alist[0]      = make_alist("target", ID_STRING, NULL, &S1);
	alist[1]      = make_alist("source", ID_STRING, NULL, &S2);
	alist[2].name = NULL;

	if (parse_args(func, arg, alist) == 0) {
		*Result = 0;
	} else if (S1 == NULL || S2 == NULL) {
		*Result = 0;
	} else if ((strstr(S1, S2)) == NULL) {
		*Result = 0;
	} else {
		*Result = 1;
	}

	return (newVal(BSQ, 1, 1, 1, DV_INT32, Result));
}

Var* ff_pow(vfuncptr func, Var* arg)
{
	Var *ob1 = NULL, *ob2 = NULL;

	Alist alist[3];
	alist[0]      = make_alist("ob1", ID_VAL, NULL, &ob1);
	alist[1]      = make_alist("ob2", ID_VAL, NULL, &ob2);
	alist[2].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (ob1 == NULL) {
		parse_error("%s(), two objects required.", func->name);
		return (NULL);
	}
	if (ob2 == NULL) {
		parse_error("%s(), second object not specified", func->name);
		return (NULL);
	}
	return (pp_math(ob1, ID_POW, ob2));
}

Var* ff_system(vfuncptr func, Var* arg)
{
	char* str = NULL;
	int sys_rtnval;
	Alist alist[2];
	alist[0]      = make_alist("command", ID_STRING, NULL, &str);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);
	if (str == NULL) {
		parse_error("Not enough arguments to function: %s()", func->name);
		return (NULL);
	}
	sys_rtnval = system(str);
	return newInt(sys_rtnval);
}

// TODO(rswinkle): make x,y,z size_t?
Var* newVal(int org, int x, int y, int z, int format, void* data)
{
	Var* v = NULL;
	if (!(v      = newVar())) return NULL;
	V_TYPE(v)    = ID_VAL;
	V_ORG(v)     = org;
	V_DSIZE(v)   = ((size_t)x) * ((size_t)y) * ((size_t)z);
	V_SIZE(v)[0] = x;
	V_SIZE(v)[1] = y;
	V_SIZE(v)[2] = z;
	V_FORMAT(v)  = format;
	V_DATA(v)    = data;
	return (v);
}

Var* ff_exit(vfuncptr func, Var* arg)
{
	Var* obj = NULL;
	Alist alist[2];

	alist[0]      = make_alist("object", ID_VAL, NULL, &obj);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	// It's a bad idea for this to call exit() directly, since
	// the cleanup that happens in quit() wont.  Changing to call quit()
	// instead so that temp directories get cleaned up, etc.
	if (obj == NULL) {
		quit(0);
	} else {
		quit(extract_int(obj, 0));
	}
	return (NULL); // never reached
}

#include <sys/stat.h>

Var* ff_fsize(vfuncptr func, Var* arg)
{
	char* filename = NULL;
	struct stat sbuf;
	int* data;

	Alist alist[2];
	alist[0]      = make_alist("filename", ID_STRING, NULL, &filename);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (filename == NULL) {
		fprintf(stderr, "%s: No filename specified\n", func->name);
		return (NULL);
	} else {
		data  = (int*)calloc(1, sizeof(int));
		*data = -1;
		if ((stat(filename, &sbuf)) == 0) {
			*data = sbuf.st_size;
			if ((*data) != sbuf.st_size) {
				parse_error("%s: Integer truncation, size was %ld\n", func->name, sbuf.st_size);
			}
		}
		return (newVal(BSQ, 1, 1, 1, DV_INT32, data));
	}
}

#ifdef HAVE_LIBREADLINE
#include <readline/history.h>
#endif

Var* ff_history(vfuncptr func, Var* arg)
{
	Var* value = NULL;
	int i, count = 0;

	Alist alist[2];
	alist[0]      = make_alist("count", ID_VAL, NULL, &value);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (value == NULL) {
		count = -1;
	} else if ((i = extract_int(value, 0)) != 0) {
		count = i;
	}

	print_history(count);
	return (NULL);
}

void print_history(int i)
{
#ifdef HAVE_LIBREADLINE
	HIST_ENTRY* h;
	HISTORY_STATE* state;
	int j;

	state                    = history_get_history_state();
	if (i == -1) i           = state->length - 1;
	if (i > state->length) i = state->length - 1;
	for (j = state->length - i; j < state->length; j++) {
		h = history_get(j - history_base + 1);
		printf("%6d   %s\n", j, (h == NULL ? "(null)" : h->line));
	}
#endif
}

// This is probably overriden in config.h
#ifndef EDITOR
#define EDITOR "vi"
#endif

Var* ff_hedit(vfuncptr func, Var* arg)
{

#ifdef HAVE_LIBREADLINE

	FILE* fp;
	int j, count = 0;
	HISTORY_STATE* state;
	char *tmp, *editor, buf[256];

	Alist alist[2];
	alist[0]      = make_alist("number", DV_INT32, NULL, &count);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	state = history_get_history_state();
	if (count >= state->length) {
		parse_error("%s: not that many history entires", func->name);
		return (NULL);
	}
	tmp = make_temp_file_path();
	if (tmp == NULL || (fp = fopen(tmp, "w")) == NULL) {
		parse_error("%s: unable to open temp file", func->name);
		if (tmp) free(tmp);
		return (NULL);
	}

	for (j = count - 1; j < state->length - 2; j++) {
		fprintf(fp, "%s\n", history_get(j - history_base)->line);
	}
	fclose(fp);

	if ((editor = getenv("EDITOR")) == NULL) editor = (char*)EDITOR;
	sprintf(buf, "%s %s", editor, tmp);
	system(buf);

	fp = fopen(tmp, "r");
	unlink(tmp);
	push_input_stream(fp, (char*)":history:");

	free(tmp);
#endif
	return (NULL);
}

/*
** This function lets you LIE to the system.
*/
Var* ff_resize(vfuncptr func, Var* arg)
{
	Var* obj = NULL;
	int x = 1, y = 1, z = 1;
	const char* orgs[] = {"bsq", "bil", "bip", "xyz", "xzy", "zxy", NULL};
	char* org_str      = NULL;
	int org            = 0;

	Alist alist[6];
	alist[0]      = make_alist("obj", ID_VAL, NULL, &obj);
	alist[1]      = make_alist("x", DV_INT32, NULL, &x);
	alist[2]      = make_alist("y", DV_INT32, NULL, &y);
	alist[3]      = make_alist("z", DV_INT32, NULL, &z);
	alist[4]      = make_alist("org", ID_ENUM, orgs, &org_str);
	alist[5].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (obj == NULL) {
		parse_error("No argument specified: %s(...obj=...)", func->name);
		return (NULL);
	}

	if (x < 0 || y < 0 || z < 0) {
		parse_error("Illegal dimensions: %dx%dx%d\n", x, y, z);
		return (NULL);
	}
	if (((size_t)x) * ((size_t)y) * ((size_t)z) != V_DSIZE(obj)) {
		parse_error("Illegal dimensions: %dx%dx%d != %d\n", x, y, z, V_DSIZE(obj));
		return (NULL);
	}

	if (org_str != NULL) {
		if (!strcasecmp(org_str, "bsq"))
			org = BSQ;
		else if (!strcasecmp(org_str, "xyz"))
			org = BSQ;
		else if (!strcasecmp(org_str, "bil"))
			org = BIL;
		else if (!strcasecmp(org_str, "xzy"))
			org = BIL;
		else if (!strcasecmp(org_str, "bip"))
			org = BIP;
		else if (!strcasecmp(org_str, "zxy"))
			org    = BIP;
		V_ORG(obj) = org;
	}

	V_SIZE(obj)[orders[V_ORG(obj)][0]] = x;
	V_SIZE(obj)[orders[V_ORG(obj)][1]] = y;
	V_SIZE(obj)[orders[V_ORG(obj)][2]] = z;

	return (obj);
}

Var* ff_fork(vfuncptr func, Var* arg)
{
#if !(defined(__CYGWIN__) || defined(__MINGW32__))
	if (fork() == 0) {
		sleep(10);
	}
#endif
	return (NULL);
}

Var* ff_eval(vfuncptr func, Var* arg)
{
	char* expr = NULL;
	char* buf;

	Alist alist[2];
	alist[0]      = make_alist("expr", ID_STRING, NULL, &expr);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (expr == NULL) {
		return (NULL);
	}

	/*
	** gotta stick a newline terminator on the end
	*/
	buf = calloc(strlen(expr) + 3, 1);
	strcpy(buf, expr);
	strcat(buf, "\n");
	eval_buffer(buf);

	return (pop(scope_tos()));
}

Var* ff_syscall(vfuncptr func, Var* arg)
{
	char* expr = NULL;
	FILE* fp;

	Var* o;
	char** text;
	int Row = 0;
	int Max = 100;
	char* ptr;

	Alist alist[2];
	alist[0]      = make_alist("command", ID_STRING, NULL, &expr);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (expr == NULL) {
		return (NULL);
	}

	if ((fp = popen(expr, "r")) == NULL) return (NULL);

	text = (char**)calloc(Max, sizeof(char*));
	while (dv_getline(&ptr, fp) != EOF) {
		if (Row >= Max) {
			Max += 100;
			if ((text = realloc(text, (Max * sizeof(char*)))) == NULL) {
				parse_error("Couldn't allocate large enough buffer to hold result");
				return (NULL);
			}
		}
		if (ptr[strlen(ptr) - 1] == '\n')
			ptr[strlen(ptr) - 1] = '\0'; /*Strip off \n from end of line*/

		text[Row] = strdup(ptr);
		Row++;
	}

	pclose(fp);
	if (Row == 0) {
		free(text);
		return (NULL);
	}

	o              = newVar();
	V_TYPE(o)      = ID_TEXT;
	V_TEXT(o).Row  = Row;
	V_TEXT(o).text = text;

	return (o);
}

Var* ff_dump(vfuncptr func, Var* arg)
{
	Var* v     = NULL;
	int indent = 0, depth = 0;

	Alist alist[4];
	alist[0]      = make_alist("object", ID_UNK, NULL, &v);
	alist[1]      = make_alist("indent", DV_INT32, NULL, &indent);
	alist[2]      = make_alist("depth", DV_INT32, NULL, &depth);
	alist[3].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (v == NULL) return (NULL);

	dump_var(v, indent, depth);

	return (NULL);
}

/*
** compare a to b to see if they are equivalent
*/
Var* ff_equals(vfuncptr func, Var* arg)
{
	Var *v1 = NULL, *v2 = NULL, *v, *e;
	Alist alist[3];
	char* data;

	alist[0]      = make_alist("obj1", ID_UNK, NULL, &v1);
	alist[1]      = make_alist("obj2", ID_UNK, NULL, &v2);
	alist[2].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if ((e = eval(v1)) != NULL) v1 = e;
	if ((e = eval(v2)) != NULL) v2 = e;

	data    = calloc(1, 1);
	data[0] = compare_vars(v1, v2);
	v       = newVal(BSQ, 1, 1, 1, DV_UINT8, data);
	return (v);
}

int compare_vars(Var* a, Var* b)
{
	size_t i;
	size_t x1, y1, z1;
	size_t x2, y2, z2;
	int rows, format;

	if (a == NULL || b == NULL) return 0;
	if (V_TYPE(a) != V_TYPE(b)) return 0;

	switch (V_TYPE(a)) {
	case ID_STRUCT: return compare_struct(a, b);

	case ID_TEXT:
		if (V_TEXT(a).Row != V_TEXT(b).Row) return (0);
		rows = V_TEXT(a).Row;
		for (i = 0; i < rows; i++) {
			if (strcmp(V_TEXT(a).text[i], V_TEXT(b).text[i])) {
				return 0;
			}
		}
		return 1;

	case ID_STRING:
		if (strcmp(V_STRING(a), V_STRING(b))) {
			return 0;
		}
		return 1;

	case ID_VAL:
		/*
		** pp_math will give us an answer,
		** but will also let us use smaller sets.
		** Verify that these are the same size first
		*/
		x1 = GetSamples(V_SIZE(a), V_ORG(a));
		y1 = GetLines(V_SIZE(a), V_ORG(a));
		z1 = GetBands(V_SIZE(a), V_ORG(a));

		x2 = GetSamples(V_SIZE(b), V_ORG(b));
		y2 = GetLines(V_SIZE(b), V_ORG(b));
		z2 = GetBands(V_SIZE(b), V_ORG(b));

		if (x1 != x2 || y1 != y2 || z1 != z2) {
			return 0;
		}

		//TODO(rswinkle) check C spec integer conversions/promotions
		format = max(V_FORMAT(a), V_FORMAT(b));

		for (i = 0; i < V_DSIZE(a); i++) {
			switch (format) {
			case DV_UINT8:
			case DV_UINT16:
			case DV_UINT32:

			case DV_INT8:
			case DV_INT16:
			case DV_INT32:
			case DV_INT64:
				if (extract_i64(a, i) != extract_i64(b, rpos(i, a, b))) return 0;

			case DV_UINT64:
				if (extract_u64(a, i) != extract_u64(b, rpos(i, a, b))) return 0;

			case DV_FLOAT:
				if (extract_float(a, i) != extract_float(b, rpos(i, a, b))) return 0;
			case DV_DOUBLE:
				if (extract_double(a, i) != extract_double(b, rpos(i, a, b))) return 0;
			}
		}
		return 1;
	}
	return 0;
}

Var* new_u64(u64 i)
{
	Var* v     = newVal(BSQ, 1, 1, 1, DV_UINT64, calloc(1, sizeof(u64)));
	V_UINT64(v) = i;
	return v;
}

Var* new_i64(i64 i)
{
	Var* v     = newVal(BSQ, 1, 1, 1, DV_INT64, calloc(1, sizeof(i64)));
	V_INT64(v) = i;
	return v;
}

// TODO(rswinkle) macro define this based on arch?
// or just change to DV_INT64?
Var* newInt(int i)
{
	Var* v   = newVal(BSQ, 1, 1, 1, DV_INT32, calloc(1, sizeof(i32)));
	V_INT(v) = i;
	return (v);
}

Var* newFloat(float f)
{
	Var* v     = newVal(BSQ, 1, 1, 1, DV_FLOAT, calloc(1, sizeof(float)));
	V_FLOAT(v) = f;
	return (v);
}
Var* newDouble(double d)
{
	Var* v      = newVal(BSQ, 1, 1, 1, DV_DOUBLE, calloc(1, sizeof(double)));
	V_DOUBLE(v) = d;
	return (v);
}

Var* ff_killchild(vfuncptr func, Var* arg)
{
#if !(defined(__CYGWIN__) || defined(__MINGW32__))
	pid_t pid;
	pid = getpgrp();
	pid = -pid;
	kill(pid, SIGUSR1);
#else
	parse_error("Function not supported under DOS/Windows.");
#endif /* __CYGWIN__ */
	return (NULL);
}

// TODO(rswinkle) rename these ... suffix usually implies type but
// the plain functions take double (C99 varieties are cos, cosf, cosl for example)
// I think cos_deg or deg_cos would be clearer
double cosd(double theta)
{
	return (cos(theta * M_PI / 180.0));
}
double sind(double theta)
{
	return (sin(theta * M_PI / 180.0));
}
double tand(double theta)
{
	return (tan(theta * M_PI / 180.0));
}
double acosd(double theta)
{
	return (acos(theta) * 180.0 / M_PI);
}
double asind(double theta)
{
	return (asin(theta) * 180.0 / M_PI);
}
double atand(double theta)
{
	return (atan(theta) * 180.0 / M_PI);
}

Var* ff_exists(vfuncptr func, Var* arg)
{
	Var* v         = NULL;
	char* filename = NULL;
	int i;

	Alist alist[2];
	alist[0]      = make_alist("filename", ID_UNK, NULL, &v);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (v == NULL) {
		parse_error("%s: No filename specified.", func->name);
		return (NULL);
	} else if (V_TYPE(v) == ID_STRING) {
		filename = dv_locate_file(V_STRING(v));
		return (newInt(file_exists(filename)));
	} else if (V_TYPE(v) == ID_TEXT) {
		int n     = V_TEXT(v).Row;
		int* data = calloc(n, sizeof(int));
		for (i = 0; i < n; i++) {
			data[i] = file_exists(V_TEXT(v).text[i]);
		}
		return (newVal(BSQ, 1, n, 1, DV_INT32, data));
	} else {
		parse_error("%s: Argument is not a filename.", func->name);
		return (NULL);
	}
}

Var* ff_unlink(vfuncptr func, Var* arg)
{
	Var* v         = NULL;
	char* filename = NULL;
	int n, i;

	Alist alist[2];
	alist[0]      = make_alist("filename", ID_UNK, NULL, &v);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (v == NULL) {
		parse_error("%s: No filename specified.", func->name);
		return (NULL);
	} else if (V_TYPE(v) == ID_STRING) {
		filename = V_STRING(v);
		return (newInt(unlink(filename) == 0));
	} else if (V_TYPE(v) == ID_TEXT) {
		int* data = calloc(n, sizeof(int));
		n         = V_TEXT(v).Row;
		for (i = 0; i < n; i++) {
			data[i] = (unlink(V_TEXT(v).text[i]) == 0);
		}
		return (newVal(BSQ, 1, n, 1, DV_INT32, data));
	} else {
		parse_error("%s: Argument is not a filename.", func->name);
		return (NULL);
	}
}

Var* ff_putenv(vfuncptr func, Var* arg)
{
	char *name = NULL, *val = NULL;
	char buf[4096];

	Alist alist[3];
	alist[0]      = make_alist("name", ID_STRING, NULL, &name);
	alist[1]      = make_alist("value", ID_STRING, NULL, &val);
	alist[2].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (name == NULL) {
		parse_error("%s: No name specified.", func->name);
		return (NULL);
	}
	if (val == NULL) {
		parse_error("%s: No value specified.", func->name);
		return (NULL);
	}
	sprintf(buf, "%s=%s", name, val);
	putenv(strdup(buf));
	return (NULL);
}

// NOTE(rswinkle) again we don't have a DV_SIZE_T yet and not sure we should
// so we use u64 which is equivalent on 64-bit and overkill on 32-bit
u64 v_length(Var* obj, int* err)
{
	switch (V_TYPE(obj)) {
	case ID_STRUCT: return get_struct_count(obj);
	case ID_TEXT: return V_TEXT(obj).Row;
	case ID_STRING: return strlen(V_STRING(obj));
	case ID_VAL: return V_DSIZE(obj);
	default: *err = 1;
	}
}

Var* ff_length(vfuncptr func, Var* arg)
{
	Var* obj = NULL;
	u64 len;

	Alist alist[2];
	alist[0]      = make_alist("obj", ID_UNK, NULL, &obj);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return NULL;

	if (obj == NULL) {
		parse_error("Expected object");
		return NULL;
	}

	int err = 0;
	len = v_length(obj, &err);
	if (!err) {
		return new_u64(len);
	} else {
		parse_error("%s: unrecognized type", func->name);
		return NULL;
	}
}

int decode_hexstring(char* str, char* buf)
{
	char* vbuf     = buf;
	int char_count = 0;
	int base, shift;

	char *p, *q;
	/*
	** Try to decode value into a hex block
	*/
	if (str[0] == '#') {
		p = str;
		q = strchr(str + 1, '#');
		if (p && q) {
			base = atoi(p + 1);
			switch (base) {
			case 16: shift = 4; break;
			default: return (0);
			}
			p = q + 1;
			/*
			** this is a hacky hardcode for #16#.
			*/
			while (*p) {
				if (*p >= '0' && *p <= '9') {
					*vbuf += *p - '0';
				} else if (*p >= 'a' && *p <= 'f') {
					*vbuf += *p - 'a' + 10;
				} else if (*p >= 'A' && *p <= 'F') {
					*vbuf += *p - 'A' + 10;
				} else {
					return (0);
				}
				char_count++;
				p++;
				if (char_count % 2 == 0) {
					vbuf++;
				} else {
					*vbuf = *vbuf << 4;
				}
			}
		}
		return (char_count / 2);
	}
	return (0);
}

/*
** Return a mask of everywhere that a value occurs, where the
** value can be a hex representation (ala ISIS)
*/

Var* ff_deleted(vfuncptr func, Var* arg)
{
	Var* obj       = NULL;
	Var *str_value = NULL, *v;
	char vbuf[16]  = {0};
	int bytes      = 0;
	int nbytes;
	size_t dsize;
	unsigned char *data, *out;
	size_t i;

	Alist alist[3];
	alist[0]      = make_alist("obj", ID_VAL, NULL, &obj);
	alist[1]      = make_alist("value", ID_UNK, NULL, &str_value);
	alist[2].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (obj == NULL) {
		parse_error("%s: No value specified for keyword: object.", func->name);
		return (NULL);
	}
	if (str_value == NULL) {
		parse_error("%s: No value specified for keyword: value.", func->name);
		return (NULL);
	}

	if (V_TYPE(str_value) == ID_STRING) {
		bytes = decode_hexstring(V_STRING(str_value), vbuf);
	}

	if (bytes == 0) {
		parse_error("Unrecognized value: %s", V_STRING(str_value));
		return (NULL);
	}

	dsize  = V_DSIZE(obj);
	data   = V_DATA(obj);
	nbytes = NBYTES(V_FORMAT(obj));
	if (bytes != nbytes) {
		parse_error("Word sizes differ");
		return (NULL);
	}
	out = calloc(dsize, 1);

	for (i = 0; i < dsize; i++) {
		if (memcmp(data + (i * nbytes), vbuf, bytes) == 0) {
			out[i] = 1;
		}
	}

	v            = newVar();
	V_TYPE(v)    = ID_VAL;
	V_ORG(v)     = V_ORG(obj);
	V_FORMAT(v)  = DV_UINT8;
	V_SIZE(v)[0] = V_SIZE(obj)[0];
	V_SIZE(v)[1] = V_SIZE(obj)[1];
	V_SIZE(v)[2] = V_SIZE(obj)[2];
	V_DSIZE(v)   = V_DSIZE(obj);
	V_DATA(v)    = out;

	return (v);
}

Var* ff_set_deleted(vfuncptr func, Var* arg)
{
	Var *obj = NULL, *mask = NULL;
	Var *str_value = NULL, *v;
	char vbuf[16]  = {0};
	int bytes      = 0;
	int nbytes;
	size_t dsize;
	unsigned char* data;
	size_t i;

	Alist alist[4];
	alist[0]      = make_alist("obj", ID_VAL, NULL, &obj);
	alist[1]      = make_alist("mask", ID_VAL, NULL, &mask);
	alist[2]      = make_alist("value", ID_UNK, NULL, &str_value);
	alist[3].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (obj == NULL) {
		parse_error("%s: No value specified for keyword: object.", func->name);
		return (NULL);
	}
	if (mask == NULL) {
		parse_error("%s: No value specified for keyword: mask.", func->name);
		return (NULL);
	}
	if (str_value == NULL) {
		parse_error("%s: No value specified for keyword: value.", func->name);
		return (NULL);
	}

	if (math_operable(obj, mask) == 0) {
		parse_error("Sizes don't match");
		return (NULL);
	}

	if (V_TYPE(str_value) == ID_STRING) {
		bytes = decode_hexstring(V_STRING(str_value), vbuf);
	}

	if (bytes == 0) {
		parse_error("Unrecognized value: %s", V_STRING(str_value));
		return (NULL);
	}

	dsize  = V_DSIZE(obj);
	nbytes = NBYTES(V_FORMAT(obj));
	if (bytes != nbytes) {
		parse_error("Word sizes differ");
		return (NULL);
	}
	v    = V_DUP(obj);
	data = V_DATA(v);

	for (i = 0; i < dsize; i++) {
		if (extract_int(mask, i)) {
			memcpy(data + (nbytes * i), vbuf, bytes);
		}
	}
	return (v);
}

Var* ff_contains(vfuncptr func, Var* arg)
{
	Var *obj = NULL, *value = NULL;
	size_t dsize;
	size_t i;
	int ret = 0;
	i64 vi;
	u64 vu;
	double vd;

	Alist alist[4];
	alist[0]      = make_alist("obj", ID_VAL, NULL, &obj);
	alist[1]      = make_alist("value", ID_VAL, NULL, &value);
	alist[2].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (obj == NULL) {
		parse_error("%s: No value specified for keyword: object.", func->name);
		return (NULL);
	}
	if (value == NULL) {
		parse_error("%s: No value specified for keyword: value.", func->name);
		return (NULL);
	}
	dsize = V_DSIZE(obj);

	// this is ugly and long ...
	vi = extract_i64(value, 0);
	vu = extract_u64(value, 0);
	switch (V_FORMAT(obj)) {
	case DV_UINT8:
		for (i = 0; i < dsize; i++) {
			if (((u8*)V_DATA(obj))[i] == vi) {
				ret = 1;
				break;
			}
		}
		break;
	case DV_UINT16:
		for (i = 0; i < dsize; i++) {
			if (((u16*)V_DATA(obj))[i] == vi) {
				ret = 1;
				break;
			}
		}
		break;
	case DV_UINT32:
		for (i = 0; i < dsize; i++) {
			if (((u32*)V_DATA(obj))[i] == vi) {
				ret = 1;
				break;
			}
		}
		break;
	case DV_UINT64:
		for (i = 0; i < dsize; i++) {
			if (((u64*)V_DATA(obj))[i] == vu) {
				ret = 1;
				break;
			}
		}
		break;

	case DV_INT8:
		for (i = 0; i < dsize; i++) {
			if (((i8*)V_DATA(obj))[i] == vi) {
				ret = 1;
				break;
			}
		}
		break;
	case DV_INT16:
		for (i = 0; i < dsize; i++) {
			if (((i16*)V_DATA(obj))[i] == vi) {
				ret = 1;
				break;
			}
		}
		break;
	case DV_INT32:
		for (i = 0; i < dsize; i++) {
			if (((i32*)V_DATA(obj))[i] == vi) {
				ret = 1;
				break;
			}
		}
		break;
	case DV_INT64:
		for (i = 0; i < dsize; i++) {
			if (((i64*)V_DATA(obj))[i] == vi) {
				ret = 1;
				break;
			}
		}
		break;

	case DV_FLOAT:
		vd = extract_float(value, 0);
		for (i = 0; i < dsize; i++) {
			if (((float*)V_DATA(obj))[i] == vd) {
				ret = 1;
				break;
			}
		}
		break;
	case DV_DOUBLE:
		vd = extract_double(value, 0);
		for (i = 0; i < dsize; i++) {
			if (((double*)V_DATA(obj))[i] == vd) {
				ret = 1;
				break;
			}
		}
		break;
	}
	return newInt(ret);
}

Var* ff_chdir(vfuncptr func, Var* arg)
{
	char* dir = NULL;
	Alist alist[2];
	alist[0]      = make_alist("dir", ID_STRING, NULL, &dir);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (dir == NULL) {
		parse_error("%s: No directory specified", func->name);
	} else if (file_exists(dir)) {
		chdir(dir);
		return newString(strdup(dir));
	} else {
		parse_error("%s: No such directory %s", func->name, dir);
	}
	return NULL;
}

/*
** A real function to deal with the fact that round() is really a macro
*/
double my_round(double d)
{
	return round(d);
}

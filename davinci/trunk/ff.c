#include "ff.h"
#include "apidef.h"

#ifdef rfunc
#include "rfunc.h"
#endif

#ifdef _WIN32
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
#ifdef INCLUDE_API
	APIDEFS *api;
#endif


    /**
    ** Find and call the named function or its handler
    **/
    for (f = vfunclist; f->name != NULL; f++) {
        if (!strcmp(f->name, name)) {
            return (f->fptr(f, arg));
        }
    }

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

    s = newVar();
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

    val = newVar();
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
    Var *s, *ob = NULL;
    int i, j;
    void *from, *to;
    int org = -1, nbytes, format, dsize;
    char *org_str = NULL;
    char *orgs[] = { "bsq", "bil", "bip", "xyz", "xzy", "zxy", NULL };

    Alist alist[4];
    alist[0] = make_alist( "object", ID_VAL,   NULL,     &ob);
    alist[1] = make_alist( "org",    ID_ENUM,  orgs,     &org_str);
    alist[2] = make_alist( "order",  ID_ENUM,  orgs,     &org_str);
    alist[3].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

    if (func->fdata != NULL) {
        org = (int) (func->fdata) - 10;
    } else {
        if (org_str == NULL) {
            /**
            ** no order specified.  Print out the org of the passed object.
            **/
            s = newVar();
            V_TYPE(s) = ID_STRING;
            V_STRING(s) = strdup(Org2Str(V_ORG(ob)));
            return (s);
        } else { 
            if (!strcasecmp(org_str, "bsq")) org = BSQ;
            else if (!strcasecmp(org_str, "xyz")) org = BSQ;
            else if (!strcasecmp(org_str, "bil")) org = BIL;
            else if (!strcasecmp(org_str, "xzy")) org = BIL;
            else if (!strcasecmp(org_str, "bip")) org = BIP;
            else if (!strcasecmp(org_str, "zxy")) org = BIP;
        }
    }

    /**
    ** create output variable
    **/
    dsize = V_DSIZE(ob);
    format = V_FORMAT(ob);
    s = newVar();
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

    s = newVar();
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
    s = newVar();
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
    Var *s, *object = NULL;
    char *ptr = NULL;
    char *format_str = NULL;
    char *formats[] = { "byte", "short", "int", "float", "double", NULL };

    Alist alist[4];
    alist[0] = make_alist( "object", ID_VAL,   NULL,        &object);
    alist[1] = make_alist( "format", ID_ENUM,  formats,     &format_str);
    alist[2] = make_alist( "type",   ID_ENUM,  formats,     &format_str);
    alist[3].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

    if (format_str == NULL) {
        /**
        ** no format specified.  Print out the format of the passed object.
        **/
        if (V_TYPE(object) != ID_VAL) {
            parse_error("Incorrect type of object specified to format()");
            return (NULL);
        }
        s = newVar();
        V_TYPE(s) = ID_STRING;
        V_STRING(s) = strdup(Format2Str(V_FORMAT(object)));
        return (s);
    } else {
        /**
        ** v specifies format.
        **/

        if (!strcasecmp(format_str, "byte")) ptr = "byte";
        else if (!strcasecmp(format_str, "short")) ptr = "short";
        else if (!strcasecmp(format_str, "int")) ptr = "int";
        else if (!strcasecmp(format_str, "float")) ptr = "float";
        else if (!strcasecmp(format_str, "double")) ptr = "double";

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
    Var *s;
    int dsize;
    int i, j, k, count = 0, c;
    float v;
    char *orgs[] = { "bsq", "bil", "bip", "xyz", "xzy", "zxy", NULL };
    char *formats[] = { "byte", "short", "int", "float", "double", NULL};

    int x = 1, y = 1, z = 1;
    int format = INT;
    int org = BSQ;
    float start = 0;
    float step = 1.0;
    char *format_str = NULL, *org_str = NULL;

    u_char *cdata;
    short *sdata;
    int *idata;
    float *fdata;
    double *ddata;

    Alist alist[8];
    alist[0] = make_alist( "x",      INT,      NULL,     &x);
    alist[1] = make_alist( "y",      INT,      NULL,     &y);
    alist[2] = make_alist( "z",      INT,      NULL,     &z);
    alist[3] = make_alist( "org",    ID_ENUM,  orgs,     &org_str);
    alist[4] = make_alist( "format", ID_ENUM,  formats,  &format_str);
    alist[5] = make_alist( "start",  FLOAT,    NULL,     &start);
    alist[6] = make_alist( "step",   FLOAT,    NULL,     &step);
    alist[7].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

    dsize = x*y*z;

    if (org_str != NULL) {
        if (!strcasecmp(org_str, "bsq")) org = BSQ;
        else if (!strcasecmp(org_str, "xyz")) org = BSQ;
        else if (!strcasecmp(org_str, "bil")) org = BIL;
        else if (!strcasecmp(org_str, "xzy")) org = BIL;
        else if (!strcasecmp(org_str, "bip")) org = BIP;
        else if (!strcasecmp(org_str, "zxy")) org = BIP;
    }

    if (format_str != NULL) {
        if (!strcasecmp(format_str, "byte")) format = BYTE;
        else if (!strcasecmp(format_str, "short")) format = SHORT;
        else if (!strcasecmp(format_str, "int")) format = INT;
        else if (!strcasecmp(format_str, "float")) format = FLOAT;
        else if (!strcasecmp(format_str, "double")) format = DOUBLE;
    }

    s = newVar();
    V_TYPE(s) = ID_VAL;

    V_DATA(s) = calloc(dsize,NBYTES(format));
    V_FORMAT(s) = format;
    V_ORDER(s) = org;
    V_DSIZE(s) = dsize;

    V_SIZE(s)[orders[org][0]] = x;
    V_SIZE(s)[orders[org][1]] = y;
    V_SIZE(s)[orders[org][2]] = z;

    cdata = (u_char *) V_DATA(s);
    sdata = (short *) V_DATA(s);
    idata = (int *) V_DATA(s);
    fdata = (float *) V_DATA(s);
    ddata = (double *) V_DATA(s);

    for (k = 0 ; k < z ; k++) {
        for (j = 0 ; j < y ; j++) {
            for (i = 0 ; i < x ; i++) {
                v = (count++) * step + start;
                c = cpos(i,j,k,s);
                switch (format) {
                case BYTE: 
                    cdata[c] = saturate_byte(v);
                    break;
                case SHORT: 
                    sdata[c] = saturate_short(v);
                    break;
                case INT: 
                    idata[c] = saturate_int(v);
                    break;
                case FLOAT: 
                    fdata[c] = saturate_float(v);
                    break;
                case DOUBLE: 
                    ddata[c] = v;
                    break;
		}
            }
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
replicate_text(Var *ob, int x, int y)
{
    Var *s;
    int i,j,l;
    int Row;

    s=newVar();
    V_TYPE(s)=ID_TEXT;
    V_TEXT(s).Row=V_TEXT(ob).Row*y;
    Row=V_TEXT(ob).Row;
    V_TEXT(s).text=(char **)calloc(V_TEXT(s).Row,sizeof(char *));


    for (j=0;j<y;j++){
        for(l=0;l<Row;l++){
            V_TEXT(s).text[j*Row+l]=(char *)calloc(
                strlen(V_TEXT(ob).text[l])*x,sizeof(char));
            strcpy(V_TEXT(s).text[j*Row+l],V_TEXT(ob).text[l]);
            for (i=1;i<x;i++){
                strcat(V_TEXT(s).text[j*Row+l],V_TEXT(ob).text[l]);
            }
        }
    }

    return(s);
}




Var *
ff_replicate(vfuncptr func, Var * arg)
{
    int x = 1, y = 1, z = 1;
    int org;
    Var *v = NULL, *s;

    void *data1, *data2, *d1, *d2;
    int d1ptr, d2ptr, dptr;
    int dsize;
    int len[3];
    int size[3];
    int nbytes;
    int i, j, k, l;

    Alist alist[5];
    alist[0] = make_alist( "object", ID_UNK,   NULL, &v);
    alist[1] = make_alist( "x",   INT, NULL,  &x);
    alist[2] = make_alist( "y",   INT, NULL,  &y);
    alist[3] = make_alist( "z",   INT, NULL,  &z);
    alist[4].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

    if (v == NULL) {
        parse_error("clone: No object specified\n");
        return(NULL);
    }

    if (V_TYPE(v)!=ID_VAL && V_TYPE(v)!=ID_TEXT){
        parse_error("Invalid replication object");
        return(NULL);
    }

    if (V_TYPE(v)==ID_TEXT)
        return(replicate_text(v,x,y));

    org = V_ORG(v);

	if (x <= 0 || y <= 0 || z <= 0) {
		parse_error("Bad dimension value");
		return(NULL);
	}

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

    s = newVar();
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

#if 0
Var *
ff_cat(vfuncptr func, Var * arg)
{
    Var *v;
    Var *ob1 = NULL, *ob2 = NULL;
    char *axis_str = NULL;
    int axis;
    char *options[] = { "x", "y", "z", NULL };

    Alist alist[4];
    alist[0] = make_alist( "ob1", ID_UNK,   NULL,     &ob1);
    alist[1] = make_alist( "ob2", ID_UNK,   NULL,     &ob2);
    alist[2] = make_alist( "axis",   ID_ENUM,  options,     &axis_str);
    alist[3].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

    if (axis_str == NULL) {
        parse_error("%s(), No axis specified.", func->name);
        return (NULL);
    } else {
        if (!strcasecmp(axis_str, "x")) axis = 0;
        else if (!strcasecmp(axis_str, "y")) axis = 1;
        else if (!strcasecmp(axis_str, "z")) axis = 2;
    }

    if (ob1 == NULL) {
        parse_error("No objects specified");
        return (NULL);
    }
    if (ob2 == NULL) {
        parse_error("%s(), second object not specified", func->name);
        return (NULL);
    }
    /**
    ** convert axis from XYZ to ORDER
    **/
    return (do_cat(ob1, ob2, axis));
}
#endif


Var *
ff_cat(vfuncptr func, Var * arg)
{
	int ac;
	Var **av;
	Var *axis_var = NULL, *p, *q, *v;
	char *axis_str;
	int i, j, axis;

	make_args(&ac, &av, func, arg);

	if (ac < 4) {
		parse_error("Not enough arguments to cat()");
		return(NULL);
	}
	/* find axis if specified */
	for (i = 1 ; i < ac ; i++) {
		if (V_TYPE(av[i]) == ID_KEYWORD && !strcasecmp(V_NAME(av[i]), "axis")) {
			axis_var = V_KEYVAL(av[i]);
			for (j = i ; j < ac-1 ; j++) {
				av[j] = av[j+1];
			}
			ac--;
			break;
		}
	}
	if (axis_var == NULL) {
		axis_var = av[ac-1];
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
		return(NULL);
	}
	if (!strcasecmp(axis_str, "x")) axis = 0;
	else if (!strcasecmp(axis_str, "y")) axis = 1;
	else if (!strcasecmp(axis_str, "z")) axis = 2;
	else {
		parse_error("cat(): Invalid axis specified");
		return(NULL);
	}

	p = av[1];
	for (i = 2 ; i < ac ; i++) {
		q = do_cat(p,av[i],axis);
		if (q == NULL) {
			return(NULL);
		}
		p = q;
	}
	free(av);
	return(p);
}

Var *
cat_mixed_text(Var * ob1, Var * ob2, int axis)
{
    Var *s=newVar();
    int Row;
    int i;
    char *string;
    char **text;

    V_TYPE(s)=ID_TEXT; /* Text+String=Text; String+Text=Text; either way...*/

    if (V_TYPE(ob1)==ID_TEXT) { /*Left or Top is text*/
        Row=V_TEXT(ob1).Row;
        string=V_STRING(ob2);
        text=V_TEXT(ob1).text;
        if (axis==0) {/*Left, same # of Rows*/
            V_TEXT(s).Row=Row;
            V_TEXT(s).text=(char **)calloc(sizeof(char *),Row);
            for (i=0;i<Row;i++){
                V_TEXT(s).text[i]=(char *)calloc(sizeof(char),
                                                 strlen(text[i])+strlen(string));
                strcpy(V_TEXT(s).text[i],text[i]);
                strcat(V_TEXT(s).text[i],string);
            }
        }
        else { /*Top, w/string as extra row on bottom*/
            Row++;
            V_TEXT(s).Row=Row;
            V_TEXT(s).text=(char **)calloc(sizeof(char *),Row);
            for (i=0;i<Row-1;i++){
                V_TEXT(s).text[i]=strdup(text[i]);
            }
            V_TEXT(s).text[i]=strdup(string);
        }
    }

    else {/*Right or Bottom is Text*/
        Row=V_TEXT(ob2).Row;
        string=V_STRING(ob1);
        text=V_TEXT(ob2).text;

        if (axis==0){ /*Right, same # of Row*/
            V_TEXT(s).Row=Row;
            V_TEXT(s).text=(char **)calloc(sizeof(char *),Row);
            for (i=0;i<Row;i++){
                V_TEXT(s).text[i]=(char *)calloc(sizeof(char),
                                                 strlen(text[i])+strlen(string));
                strcpy(V_TEXT(s).text[i],string);
                strcat(V_TEXT(s).text[i],text[i]);
            }
        }

        else {/*Bottom, w/string as extra row on top*/
            Row++;
            V_TEXT(s).Row=Row;
            V_TEXT(s).text=(char **)calloc(sizeof(char *),Row);
            V_TEXT(s).text[0]=strdup(string);
            for(i=1;i<Row;i++){
                V_TEXT(s).text[i]=strdup(text[i-1]);
            }
        }
    }

    return(s);
}

Var *
cat_string_text(Var * ob1, Var * ob2, int axis)
{ 
    Var *s=newVar();

    if (axis==2) { /*Can't cat Text or Strings in Z*/
        parse_error("Invalid axis specified");
        return(NULL);
    }

    if (V_TYPE(ob1) != V_TYPE(ob2)){ /*mixing strings and text!*/
        return(cat_mixed_text(ob1,ob2,axis));
    }

    if (axis==1 && V_TYPE(ob1)==ID_STRING){ /*String->Text*/
        V_TYPE(s)=ID_TEXT;
        V_TEXT(s).Row=2;
        V_TEXT(s).text=(char **)calloc(sizeof(char *),2);
        V_TEXT(s).text[0]=strdup(V_STRING(ob1));
        V_TEXT(s).text[1]=strdup(V_STRING(ob2));
        return(s);
    }

    else if (axis==1) {
        int i;
        int counter=0;
        int Row1,Row2;
        V_TYPE(s)=ID_TEXT;
        Row1=V_TEXT(ob1).Row;
        Row2=V_TEXT(ob2).Row;
        V_TEXT(s).Row=Row1+Row2;
        V_TEXT(s).text=(char **)calloc(sizeof(char *),V_TEXT(s).Row);
        for (i=0;i<Row1;i++){
            V_TEXT(s).text[counter++]=strdup(V_TEXT(ob1).text[i]);
        }
        for (i=0;i<Row2;i++){
            V_TEXT(s).text[counter++]=strdup(V_TEXT(ob2).text[i]);
        }
        return(s);
    }

    else if (V_TYPE(ob1)==ID_STRING){
        V_TYPE(s)=ID_STRING;
        V_STRING(s)=(char *)calloc(sizeof(char),strlen(V_STRING(ob1))+
                                   strlen(V_STRING(ob2))+1);
        strcpy(V_STRING(s),V_STRING(ob1));
        strcat(V_STRING(s),V_STRING(ob2));
	
        return(s);
    }

    else {
        int i;
        int Row;
        if (V_TEXT(ob1).Row != V_TEXT(ob2).Row){
            parse_error("Objects must have equal number of rows");
            return(NULL);
        }
		
        V_TYPE(s)=ID_TEXT;
        Row=V_TEXT(ob1).Row;
        V_TEXT(s).Row=Row;
        V_TEXT(s).text=(char **)calloc(sizeof(char *),Row);
        for (i=0;i<Row;i++){	
            V_TEXT(s).text[i]=(char *)calloc(sizeof(char),strlen(V_TEXT(ob1).text[i])+
                                             strlen(V_TEXT(ob2).text[i])+1);
            strcpy(V_TEXT(s).text[i],V_TEXT(ob1).text[i]);
            strcat(V_TEXT(s).text[i],V_TEXT(ob2).text[i]);
        }

        return(s);
    }

    return(NULL);
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
    int ob1_type,ob2_type;


    if ((e = eval(ob1)) != NULL) ob1 = e;
    if ((e = eval(ob2)) != NULL) ob2 = e;
    if (ob1 == NULL) return (ob2);
    if (ob2 == NULL) return (ob1);

    ob1_type=V_TYPE(ob1);
    ob2_type=V_TYPE(ob2);


    if (ob1_type != ID_VAL && ob1_type != ID_STRING && ob1_type != ID_TEXT) {
        parse_error( "cat(), improper object specified");
        return (NULL);
    }

    if (ob2_type != ID_VAL && ob2_type != ID_STRING && ob2_type != ID_TEXT) {
        parse_error( "cat(), improper object specified");
        return (NULL);
    }

    if ((ob1_type == ID_VAL && ob2_type!=ID_VAL) || 
        (ob1_type!=ID_VAL && ob2_type==ID_VAL)){
        parse_error( "cat(), Can't mix numbers with non-numbers!");
        return(NULL);
    }

    if (ob1_type == ID_STRING || ob1_type==ID_TEXT)
        return(cat_string_text(ob1,ob2,axis));

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
    s = newVar();
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


    s = newVar();
    V_TYPE(s) = ID_STRING;
    V_STRING(s) = (char *) memcpy(malloc(V_DSIZE(v)), V_DATA(v), V_DSIZE(v));
    return (s);
}

Var *
ff_strlen(vfuncptr func, Var * arg)
{
    Var *S1=NULL;
    int *Result=(int *)calloc(1,sizeof(int));
    Alist alist[2];
    alist[0] = make_alist("string",         ID_UNK,         NULL,   &S1);
    alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) {
		*Result=0;
	} else if (S1 == NULL){
        *Result=0;
    } else if (V_TYPE(S1)==ID_TEXT){
        *Result=V_TEXT(S1).Row;
    } else if (V_TYPE(S1)==ID_STRING){
        *Result=strlen(V_STRING(S1));
    } else {
        parse_error("Invalid type");
        return(NULL);
    }

    return(newVal(BSQ,1,1,1, INT, Result));
}


Var *
ff_issubstring(vfuncptr func, Var * arg)
{
    char *S1=NULL,*S2=NULL;
    int *Result=(int *)calloc(1,sizeof(int));
    Alist alist[3];
    alist[0] = make_alist("target", ID_STRING, NULL, &S1);
    alist[1] = make_alist("source", ID_STRING, NULL, &S2);
    alist[2].name = NULL;

	if (parse_args(func, arg, alist) == 0) {
		*Result=0;
	} else if (S1 == NULL || S2 == NULL){
        *Result=0;
    } else if ((strstr(S1,S2))==NULL){
        *Result=0;
    } else {
        *Result=1;
	}

    return(newVal(BSQ,1,1,1, INT, Result));
}



Var *
ff_pow(vfuncptr func, Var * arg)
{
    Var *ob1 = NULL, *ob2 = NULL;
    Alist alist[3];
    alist[0] = make_alist( "ob1", ID_VAL,   NULL,     &ob1);
    alist[1] = make_alist( "ob2", ID_VAL,   NULL,     &ob2);
    alist[2].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

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
    Var *v = newVar();
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
    Alist alist[2];

    alist[0] = make_alist( "object",    ID_VAL,    NULL,     &obj);
    alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

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
    struct stat sbuf;
    int *data;

    Alist alist[2];
    alist[0] = make_alist("filename",    ID_STRING,    NULL,     &filename);
    alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

    if (filename == NULL) {
        fprintf(stderr, "%s: No filename specified\n", func->name);
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


#include <readline/history.h>

Var *
ff_history(vfuncptr func, Var * arg)
{
    Var *value = NULL;
    int i, count = 0;

    Alist alist[2];
    alist[0] = make_alist( "count",    ID_VAL,    NULL,     &value);
    alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

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
#ifdef HAVE_LIBREADLINE
    HIST_ENTRY *h;
    HISTORY_STATE *state;
    int j;

    state = history_get_history_state();
    if (i == -1) i = state->length;
	if (i > state->length) i = state->length;
    for (j = state->length-i ; j < state->length ; j++) {
        h = state->entries[j];
        printf("%6d   %s\n", j+1, h->line);
    }
#endif
}


Var *
ff_hedit(vfuncptr func, Var * arg)
{

#ifdef HAVE_LIBREADLINE

    FILE *fp;
    Var *value = NULL;
    int i, j, count = 0;
    HISTORY_STATE *state;
    char *tmp, *editor, buf[256];

    Alist alist[2];
    alist[0] = make_alist("number",    ID_VAL,    NULL,     &value);
    alist[1].name = NULL;

    if (parse_args(func, arg, alist) == 0) return(NULL);

    if (value == NULL)  {
        count=10;
    } else if ((i = extract_int(value, 0)) != 0) {
        count = i;
    }

    state = history_get_history_state();
    if (count >= state->length) {
        parse_error("%s: not that many history entires", func->name);
        return(NULL);
    }
    tmp = tempnam(NULL, NULL);
    if ((fp = fopen(tmp, "w")) == NULL ) {
        parse_error("%s: unable to open temp file: %s", func->name, tmp);
        free(tmp);
        return(NULL);
    }

    for (j = count-1 ; j < state->length-2 ; j++) {
        fprintf(fp, "%s\n", state->entries[j]->line);
    }
    fclose(fp);

    if ((editor = getenv("EDITOR")) == NULL) 
#ifdef _WIN32
		editor = "notepad";
#else /* UNIX */
        editor = "/bin/vi";
#endif /* _WIN32 */

    sprintf(buf, "%s %s", editor, tmp);
    system(buf);

    fp = fopen(tmp, "r");
    unlink(tmp);
    push_input_stream(fp);

    free(tmp);
#endif
    return(NULL);
}

/*
** This function lets you LIE to the system.
*/
Var *
ff_resize(vfuncptr func, Var * arg)
{
    Var *obj;
    int x = 1,y = 1,z = 1;
    char *orgs[] = { "bsq", "bil", "bip", "xyz", "xzy", "zxy", NULL };
    char *org_str = NULL;
    int org = 0;

    Alist alist[6];
    alist[0] = make_alist("obj",    ID_VAL,     NULL,     &obj);
    alist[1] = make_alist("x",    	INT,    	NULL,     &x);
    alist[2] = make_alist("y",    	INT,    	NULL,     &y);
    alist[3] = make_alist("z",    	INT,    	NULL,     &z);
    alist[4] = make_alist("org",    ID_ENUM,    orgs,     &org_str);
    alist[5].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

    if (obj == NULL)  {
        parse_error("No argument specified: %s(...obj=...)", func->name);
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


    if (org_str != NULL) {
        if (!strcasecmp(org_str, "bsq")) org = BSQ;
        else if (!strcasecmp(org_str, "xyz")) org = BSQ;
        else if (!strcasecmp(org_str, "bil")) org = BIL;
        else if (!strcasecmp(org_str, "xzy")) org = BIL;
        else if (!strcasecmp(org_str, "bip")) org = BIP;
        else if (!strcasecmp(org_str, "zxy")) org = BIP;
        V_ORG(obj) = org;
    }

    V_SIZE(obj)[orders[V_ORG(obj)][0]] = x;
    V_SIZE(obj)[orders[V_ORG(obj)][1]] = y;
    V_SIZE(obj)[orders[V_ORG(obj)][2]] = z;

    return(obj);
}

Var *
ff_fork(vfuncptr func, Var * arg)
{
#ifndef _WIN32
    if (fork() == 0) {
        sleep(10);
    }
#endif
    return(NULL);
}

Var *
ff_eval(vfuncptr func, Var * arg)
{
    char *expr;
    char *buf;
	
    Alist alist[2];
    alist[0] = make_alist("expr",    ID_STRING,     NULL,     &expr);
    alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

    if (expr == NULL)  {
        return(NULL);
    }
	
    /*
    ** gotta stick a newline terminator on the end
    */
    buf = calloc(strlen(expr)+3, 1);
    strcpy(buf, expr);
    strcat(buf, "\n");
    eval_buffer(buf);

    return(pop(scope_tos()));
}

Var *
ff_syscall(vfuncptr func, Var * arg)
{
    char *expr;
    FILE *fp;

    Var *o;
    char **text;
    int Row=0;
    int Max=100;
    char *ptr;

    Alist alist[2];
    alist[0] = make_alist("command",    ID_STRING,     NULL,     &expr);
    alist[1].name = NULL;
 
	if (parse_args(func, arg, alist) == 0) return(NULL);
 
    if (expr == NULL)  {
        return(NULL);
    }

    if ((fp=popen(expr,"r"))==NULL)
        return(NULL);

    text=(char **)calloc(Max,sizeof(char *));	
    while(getline(&ptr, fp) != EOF) {
        if (Row >=Max){
            Max+=100;
            if((text=realloc(text,(Max*sizeof(char *))))==NULL){
                parse_error("Couldn't allocate large enough buffer to hold result");
                return(NULL);
            }
        }
        if (ptr[strlen(ptr)-1]=='\n')
            ptr[strlen(ptr)-1]='\0'; /*Strip off \n from end of line*/

        text[Row]=strdup(ptr);
        Row++;
    }

    pclose(fp);
	if (Row == 0) {
		return(NULL);
	}

    o=newVar();
    V_TYPE(o)=ID_TEXT;
    V_TEXT(o).Row=Row;
    V_TEXT(o).text=text;

    return(o);
}
		
Var *
ff_dump(vfuncptr func, Var * arg)
{
    Var *v;
    int depth;

    Alist alist[3];
    alist[0] = make_alist("object",    ID_VAL,     NULL,     &v);
    alist[1] = make_alist("depth",    INT,     NULL,     &depth);
    alist[2].name = NULL;
 
	if (parse_args(func, arg, alist) == 0) return(NULL);
 
    if (v == NULL) return(NULL);

    dump_var(v, 0, 0);

    return(NULL);
}


/*
** compare a to b to see if they are equivalent
*/
Var *
ff_equals(vfuncptr func, Var * arg)
{
    Var *v1 = NULL, *v2 = NULL, *v, *e;
    Alist alist[3];
    char *data;

    alist[0] = make_alist("obj1",    ID_UNK,     NULL,     &v1);
    alist[1] = make_alist("obj2",    ID_UNK,     NULL,     &v2);
    alist[2].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

    if ((e = eval(v1)) != NULL) v1 = e;
    if ((e = eval(v2)) != NULL) v2 = e;

    data = calloc(1,1);
    data[0] = compare_vars(v1,v2);
    v = newVal(BSQ, 1, 1, 1, BYTE, data);
    return(v);
}

int
compare_vars(Var *a, Var *b)
{
    int i;
    int x1, y1, z1;
    int x2, y2, z2;
    int rows, format;

    if (a == NULL || b == NULL) return(0);
    if (V_TYPE(a) != V_TYPE(b)) return(0);

    switch (V_TYPE(a)) {
    case ID_STRUCT:
        return(compare_struct(a, b));
		
    case ID_TEXT:
        if (V_TEXT(a).Row != V_TEXT(b).Row) return(0);
        rows = V_TEXT(a).Row;
        for (i = 0 ; i < rows ; i++) {
            if (strcmp(V_TEXT(a).text[i], V_TEXT(b).text[i])) {
                return(0);
            }
        }
        return(1);

    case ID_STRING:
        if (strcmp(V_STRING(a), V_STRING(b))) {
            return(0);
        }
        return(1);

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
            return(0);
        }

        format = max(V_FORMAT(a), V_FORMAT(b));

        for (i = 0 ; i < V_DSIZE(a) ; i++) {
            switch(format) {
            case BYTE:
            case SHORT:
            case INT:
                if (extract_int(a,i) != extract_int(b,rpos(i,a,b)))
                    return(0);
                break;
            case FLOAT:
                if (extract_float(a,i) != extract_float(b,rpos(i,a, b)))
                    return(0);
                break;
            case DOUBLE:
                if (extract_double(a,i) != extract_double(b,rpos(i,a, b)))
                    return(0);
                break;
            }
        }
        return(1);
    }
    return(0);
}


Var *
newInt(int i)
{
    Var *v = newVal(BSQ, 1, 1,1, INT, calloc(1, sizeof(int)));	
    V_INT(v) = i;
    return(v);
}
Var *
newFloat(float f)
{
	Var *v = newVal(BSQ, 1, 1,1, FLOAT, calloc(1, sizeof(float)));	
	V_FLOAT(v) = f;
	return(v);
}

Var *
ff_killchild(vfuncptr func, Var *arg)
{
#ifndef _WIN32
	pid_t pid;
	pid=getpgrp();
	pid=-pid;
	kill(pid,SIGUSR1);
#else
	parse_error("Function not supported under DOS/Windows.");
#endif /* _WIN32 */
	return(NULL);
}


double cosd(double theta) { return(cos(theta*M_PI/180.0)); }
double sind(double theta) { return(sin(theta*M_PI/180.0)); }
double tand(double theta) { return(tan(theta*M_PI/180.0)); }
double acosd(double theta) { return(acos(theta)*180.0/M_PI); }
double asind(double theta) { return(asin(theta)*180.0/M_PI); }
double atand(double theta) { return(atan(theta)*180.0/M_PI); }

Var *
ff_exists(vfuncptr func, Var * arg)
{
  Var *v = NULL;
  char *filename = NULL;
  int n, i;

    Alist alist[2];
    alist[0] = make_alist("filename",    ID_UNK,     NULL,     &v);
    alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (v == NULL) {
        parse_error( "%s: No filename specified.", func->name);
		return(NULL);
	} else if (V_TYPE(v) == ID_STRING) {
		filename = V_STRING(v);
		return(newInt(access(filename, F_OK) == 0));
	} else if (V_TYPE(v) == ID_TEXT) {
		int *data = calloc(n, sizeof(int));
		n = V_TEXT(v).Row;
		for (i = 0 ; i < n ; i++) {
			data[i] = (access(V_TEXT(v).text[i], F_OK) == 0);
		}
		return(newVal(BSQ, 1, n, 1, INT, data));
	} else {
        parse_error( "%s: Argument is not a filename.", func->name);
		return(NULL);
	}
}

Var *
ff_putenv(vfuncptr func, Var * arg)
{
    char *name = NULL, *val=NULL;
    char buf[4096];

    Alist alist[3];
    alist[0] = make_alist("name",    ID_STRING,     NULL,     &name);
    alist[1] = make_alist("value",    ID_STRING,     NULL,     &val);
    alist[2].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (name == NULL) {
        parse_error( "%s: No name specified.", func->name);
		return(NULL);
	}
	if (val == NULL) {
        parse_error( "%s: No value specified.", func->name);
		return(NULL);
	}
	sprintf(buf, "%s=%s", name, val);
	putenv(buf);
	return(NULL);
}

Var *
ff_length(vfuncptr func, Var * arg)
{
    Var *obj;

    Alist alist[2];
    alist[0] = make_alist("obj",    ID_UNK,     NULL,     &obj);
    alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	switch (V_TYPE(obj))  {
		case ID_STRUCT:
			return(newInt(get_struct_count(obj)));
		case ID_TEXT:
			return(newInt(V_TEXT(obj).Row));
		case ID_STRING:
			return(newInt(strlen(V_STRING(obj))));
		case ID_VAL:
			return(newInt(V_DSIZE(obj)));
		default:
			parse_error("%s: unrecognized type", func->name);
			return(NULL);
	}
}

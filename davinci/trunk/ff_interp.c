#include "parser.h"

int is_deleted(float f)
{
    return (f < -1.22e34 && f > -1.24e34);
}

Var *
ff_interp(vfuncptr func, Var *arg)
{
    Var *s, *e;
    float *x,*y, *fdata;
    int i,j, count = 0;
    Var *v[3];
    float x1,y1,x2,y2,w;
    float m;


    struct keywords kw[] = {
        { "object", NULL },
        { "from", NULL },
        { "to", NULL },
        { NULL, NULL }
    };

    if (evaluate_keywords(func, arg, kw)) {
        return(NULL);
    }

    if ((v[0] = get_kw("object", kw)) == NULL) {
	parse_error("Object= not specified");
	return(NULL);
    }
    if ((v[1] = get_kw("from", kw)) == NULL) {
	parse_error("From= not specified");
	return(NULL);
    }
    if ((v[2] = get_kw("to", kw)) == NULL) {
	parse_error("To= not specified");
	return(NULL);
    }

    for (i = 0 ; i< 3 ; i++) {
	if ((e = eval(v[i])) != NULL) v[i] = e;
	if (V_TYPE(v[i]) != ID_VAL) {
	    parse_error("Objects must be Values\n");
	    return(NULL);
	}
    }

    if (V_DSIZE(v[0]) != V_DSIZE(v[1])) {
	parse_error("Object and From values must be same size\n");
    }

    fdata = (float *)calloc(sizeof(FLOAT), V_DSIZE(v[2]));
    x = (float *)calloc(sizeof(FLOAT), V_DSIZE(v[0]));
    y = (float *)calloc(sizeof(FLOAT), V_DSIZE(v[0]));

    count = 0;
    for (i = 0 ; i < V_DSIZE(v[0]) ; i++) {
	x[count] = extract_float(v[1],i);
	y[count] = extract_float(v[0],i);
	if (is_deleted(x[count]) || is_deleted(y[count])) continue;
	count++;
    }

    for (i = 0 ; i < V_DSIZE(v[2]) ; i++) {
	w = extract_float(v[2], i); /* output wavelength */
	if (is_deleted(w)) {
	    fdata[i] = -1.23e34; 
	} else {
	    for (j = 0 ;j < count ; j++) {
		if (x[j] > w) break;
	    }
	    x2 = x[j];
	    y2 = y[j];
	    x1 = x[j-1];
	    y1 = y[j-1];

	    if (y2 == y1) {
		fdata[i] = y1;
	    } else {
		m = (y2-y1)/(x2-x1);
		fdata[i] = m*w + (y1 - m*x1);
	    }
	}
    }

    s = newVar();
    V_TYPE(s) = ID_VAL;

    V_DATA(s) = (void *)fdata;
    V_DSIZE(s) = V_DSIZE(v[2]);
    V_SIZE(s)[0] = V_SIZE(v[2])[0];
    V_SIZE(s)[1] = V_SIZE(v[2])[1];
    V_SIZE(s)[2] = V_SIZE(v[2])[2];
    V_ORG(s) = V_ORG(v[2]);
    V_FORMAT(s) = FLOAT;


    free(x);
    free(y);
    return(s);
}

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
    int i, count = 0;
    Var *v[3];
    float x1,y1,x2,y2,w;
    float *m, *c; /* slopes and y-intercepts */
    int fromsz, tosz; /* number of elements in from & to arrays */

	Alist alist[4];
	alist[0] = make_alist( "object",    ID_VAL,    NULL,    &v[0]);
	alist[1] = make_alist( "from",      ID_VAL,    NULL,    &v[1]);
	alist[2] = make_alist( "to",        ID_VAL,    NULL,    &v[2]);
	alist[3].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

    if (v[0] == NULL) {
        parse_error("Object= not specified");
        return(NULL);
    }
    if (v[1] == NULL) {
        parse_error("From= not specified");
        return(NULL);
    }
    if (v[2] == NULL) {
        parse_error("To= not specified");
        return(NULL);
    }

    if (V_DSIZE(v[0]) != V_DSIZE(v[1])) {
        parse_error("Object and From values must be same size\n");
    }

    fromsz = V_DSIZE(v[0]);
    tosz = V_DSIZE(v[2]);
    
    fdata = (float *)calloc(sizeof(FLOAT), tosz);
    x = (float *)calloc(sizeof(FLOAT), fromsz);
    y = (float *)calloc(sizeof(FLOAT), fromsz);
    m = (float *)calloc(sizeof(FLOAT), fromsz-1);
    c = (float *)calloc(sizeof(FLOAT), fromsz-1);

    count = 0;
    for (i = 0 ; i < fromsz ; i++) {
        x[count] = extract_float(v[1],i);
        y[count] = extract_float(v[0],i);
        if (is_deleted(x[count]) || is_deleted(y[count])) continue;
        count++;
    }

    /* evaluate & cache slopes & y-intercepts */
    for (i = 1; i < fromsz; i++){
        m[i-1] = (y[i]-y[i-1])/(x[i]-x[i-1]);
        c[i-1] = y[i-1] - m[i-1]*x[i-1];
    }

    for (i = 0 ; i < tosz ; i++) {
        w = extract_float(v[2], i); /* output wavelength */
        if (is_deleted(w)) {
            fdata[i] = -1.23e34; 
        } else {

            /*
            ** Locate the segment containing the x-value of "w".
            ** Assume that x-values are monotonically increasing.
            */
            int st = 0, ed = fromsz-1, mid;
            
            while((ed-st) > 1){
                mid = (st+ed)/2;
                if (w > x[mid])     { st = mid; }
                else if (w < x[mid]){ ed = mid; }
                else                { st = ed = mid; }
            }
            x2 = x[ed]; y2 = y[ed];
            x1 = x[st]; y1 = y[st];

            if (y2 == y1) {
                fdata[i] = y1;
            } else {
                /* m = (y2-y1)/(x2-x1); */
                /* fdata[i] = m[st]*w + (y1 - m[st]*x1); */
                fdata[i] = m[st]*w + c[st];
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

#include "parser.h"

/**
 ** Take three specified images, and combine them in a 3-plane BIP cube,
 ** for output as a 24-bit RGB.
 **
 ** At some point this could be enhanced to do quantization
 **/

Var *
ff_rgb(vfuncptr func, Var *arg)
{
    int count, i;
    Var *v[3] = { NULL, NULL, NULL };
	Var *s;
    int x,y;
    u_char *data;

	Alist alist[4];
	alist[0] = make_alist( "red",    ID_VAL,    NULL,    &v[0]);
	alist[1] = make_alist( "green",  ID_VAL,    NULL,    &v[1]);
	alist[2] = make_alist( "blue",   ID_VAL,    NULL,    &v[2]);
	alist[3].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

    for (i = 0 ; i < 3 ; i++) {
		if (v[i] == NULL) {
			parse_error("rgb() requires three images.  Image %d is NULL", i+1);
			return(NULL);
		}
        if (V_TYPE(v[i]) != ID_VAL || V_FORMAT(v[i]) != BYTE) {
            parse_error("%s: Image planes must be BYTE format",func->name);
            return(NULL);
        }
        if (V_ORG(v[i]) != V_ORG(v[0]))  {
            parse_error( 
                    "%s: Image planes must have the same org",func->name);
            return(NULL);
        }
        if (V_SIZE(v[i])[0] != V_SIZE(v[0])[0]) {
            parse_error( 
                    "%s: Image planes must be the same size",func->name);
            return(NULL);
        }
        if (GetBands(V_SIZE(v[i]), V_ORG(v[i])) != 1) {
            parse_error( 
                    "%s: Image planes must have depth=1",func->name);
            return(NULL);
        }
    }

    x = GetSamples(V_SIZE(v[0]), V_ORG(v[0]));
    y = GetLines(V_SIZE(v[0]), V_ORG(v[0]));

    data = (unsigned char *)calloc(3, x*y);
    count = 0;
    for (i = 0 ; i < x*y ; i++) {
        data[count++] = ((u_char *)V_DATA(v[0]))[i];
        data[count++] = ((u_char *)V_DATA(v[1]))[i];
        data[count++] = ((u_char *)V_DATA(v[2]))[i];
    }

    s = newVar();
    V_TYPE(s) = ID_VAL;
    V_DATA(s) = data;
    V_ORG(s) = BIP;
    V_SIZE(s)[0] = 3;
    V_SIZE(s)[1] = x;
    V_SIZE(s)[2] = y;
    V_DSIZE(s) = x*y*3;
    V_FORMAT(s) = BYTE;

    return(s);
}

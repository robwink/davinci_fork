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
    Var *v[3], *s;
    int x,y;
    u_char *data;

    struct keywords kw[] = {
        { "red", NULL },
        { "green", NULL },
        { "blue", NULL },
        { NULL, NULL }
    };

    if (evaluate_keywords(func, arg, kw)) {
        return(NULL);
    }

    v[0] = get_kw("red", kw);
    v[1] = get_kw("green", kw);
    v[2] = get_kw("blue", kw);

    for (i = 0 ; i < 3 ; i++) {
        if ((s = eval(v[i])) != NULL) {
            v[i] = s;
        }
        if (V_TYPE(v[i]) != ID_VAL || V_FORMAT(v[i]) != BYTE) {
            sprintf(error_buf, 
                    "%s: Image planes must be BYTE format\n",func->name);
            parse_error(NULL);
            return(NULL);
        }
        if (V_ORG(v[i]) != V_ORG(v[0]))  {
            sprintf(error_buf, 
                    "%s: Image planes must have the same org\n",func->name);
            parse_error(NULL);
            return(NULL);
        }
        if (V_SIZE(v[i])[0] != V_SIZE(v[0])[0]) {
            sprintf(error_buf, 
                    "%s: Image planes must be the same size\n",func->name);
            parse_error(NULL);
            return(NULL);
        }
        if (GetBands(V_SIZE(v[i]), V_ORG(v[i])) != 1) {
            sprintf(error_buf, 
                    "%s: Image planes must have depth=1\n",func->name);
            parse_error(NULL);
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

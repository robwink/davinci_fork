#include "parser.h"

/**
 ** cluster()
 **
 ** A windowing cluster algorithm.
 ** Arguments:
 **
 ** count(object=data, threshold=4)
 **/

/**
 ** scan a given image, using a specified window size, to generate a
 ** cluster overlay
 **
 ** Args:
 **             object = VAR
 **             radius = INT
 **             threshold = FLOAT
 ** Returns:
 **     Cluster overlay image
 **
 ** Notes:
 **             This only works with integer images.
 **/

Var *
ff_cluster(vfuncptr func, Var * arg)
{
    int radius;
    int threshold;
    Var *v, *e, *obj;
    int i, j, a, b, x, y, z;
    u_char *data;

    struct keywords kw[] =
    {
        {"object", NULL},
        {"radius", NULL},
        {"threshold", NULL},
        {NULL, NULL}
    };
    if (evaluate_keywords(func, arg, kw)) {
        return (NULL);
    }
    if ((v = get_kw("object", kw)) == NULL) {
        parse_error("No target object specified");
        return (NULL);
    }
    if ((e = eval(v)) != NULL) {
        v = e;
    }
    if (V_TYPE(v) != ID_VAL || GetBands(V_SIZE(v), V_ORG(v)) != 1) {
        sprintf(error_buf, "%s: Object must be an image of depth 1", func->name);
        parse_error(error_buf);
        return (NULL);
    }
    obj = v;

    if ((v = get_kw("radius", kw)) == NULL) {
        radius = 1;
    } else {
        if ((e = eval(v)) != NULL) {
            v = e;
        }
        if (V_TYPE(v) != ID_VAL || V_DSIZE(v) != 1 || V_FORMAT(v) != INT) {
            sprintf(error_buf, "%s(): radius must be an integer", func->name);
            parse_error(error_buf);
        }
        radius = extract_int(v, 0);
    }


    if ((v = get_kw("threshold", kw)) == NULL) {
        threshold = 1;
    } else {
        if ((e = eval(v)) != NULL) {
            v = e;
        }
        if (V_TYPE(v) != ID_VAL || V_DSIZE(v) != 1 || V_FORMAT(v) != INT) {
            sprintf(error_buf, "%s(): illegal value for threshold", func->name);
            parse_error(error_buf);
        }
        threshold = extract_int(v, 0);
    }

    x = GetSamples(V_SIZE(obj), V_ORG(obj));
    y = GetLines(V_SIZE(obj), V_ORG(obj));
    data = (unsigned char *)calloc(1, V_DSIZE(obj));

    for (j = 0; j < y; j++) {
        for (i = 0; i < x; i++) {
            z = j * x + i;
            if (extract_int(obj, z) >= threshold) {
                for (a = j - radius; a <= j + radius; a++) {
                    for (b = i - radius; b <= i + radius; b++) {
                        if (a < 0 || a >= y || b < 0 || b >= x)
                            continue;
                        data[z] += (extract_int(obj, a * x + b) >= threshold);
                    }
                }
            }
        }
    }

    /** 
     ** Put together return value
     **/

    v = newVar();
    V_TYPE(v) = ID_VAL;
    V_DATA(v) = data;
    V_ORG(v) = V_ORG(obj);
    V_DSIZE(v) = V_DSIZE(obj);
    V_SIZE(v)[0] = V_SIZE(obj)[0];
    V_SIZE(v)[1] = V_SIZE(obj)[1];
    V_SIZE(v)[2] = V_SIZE(obj)[2];
    V_FORMAT(v) = BYTE;

    return (v);
}


Var *
ff_ccount(vfuncptr func, Var * arg)
{

    Var *v, *e, *obj;
    int threshold, ignore, i, dsize;
    int top = 0, bottom = 0, value = 0;

    struct keywords kw[] =
    {
        {"object", NULL},
        {"threshold", NULL},
        {"ignore", NULL},
        {NULL, NULL}
    };

    if (evaluate_keywords(func, arg, kw)) {
        return (NULL);
    }
    if ((v = get_kw("object", kw)) == NULL) {
        parse_error("No target object specified");
        return (NULL);
    }
    if ((e = eval(v)) != NULL) {
        v = e;
    }
    if (V_TYPE(v) != ID_VAL || GetBands(V_SIZE(v), V_ORG(v)) != 1) {
        sprintf(error_buf, "%s: Object must be an image of depth 1", func->name);
        parse_error(error_buf);
        return (NULL);
    }
    obj = v;

    if ((v = get_kw("threshold", kw)) == NULL) {
        threshold = 2;
    } else {
        if ((e = eval(v)) != NULL) {
            v = e;
        }
        if (V_TYPE(v) != ID_VAL || V_DSIZE(v) != 1 || V_FORMAT(v) != INT) {
            sprintf(error_buf, "%s(): threshold must be an integer", func->name);
            parse_error(error_buf);
        }
        threshold = extract_int(v, 0);
    }

    if ((v = get_kw("ignore", kw)) == NULL) {
        ignore = 1;
    } else {
        if ((e = eval(v)) != NULL) {
            v = e;
        }
        if (V_TYPE(v) != ID_VAL || V_DSIZE(v) != 1 || V_FORMAT(v) != INT) {
            sprintf(error_buf, "%s(): ignore must be an integer", func->name);
            parse_error(error_buf);
        }
        ignore = extract_int(v, 0);
    }


    dsize = V_DSIZE(obj);

    for (i = 0; i < dsize; i++) {
        value = extract_int(obj, i);
        if (value > ignore)
            bottom++;
        if (value >= threshold)
            top++;
    }

    v = newVar();
    V_TYPE(v) = ID_VAL;
    V_DATA(v) = calloc(3, sizeof(float));
    V_DSIZE(v) = V_SIZE(v)[0] = 3;
    V_SIZE(v)[1] = V_SIZE(v)[2] = 1;
    V_ORG(v) = BSQ;
    V_FORMAT(v) = FLOAT;

    if (bottom == 0) {
        ((float *) V_DATA(v))[0] = (float) 0.0;
    } else {
        ((float *) V_DATA(v))[0] = (float) top / (float) bottom;
    }
    ((float *) V_DATA(v))[1] = top;
    ((float *) V_DATA(v))[2] = bottom;

    return (v);
}

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
    int radius=1;
    int threshold = 1;
    Var *obj = NULL;
    int i, j, a, b, x, y;
	size_t z;
    u_char *data;

	Alist alist[4];
	alist[0] = make_alist( "object",    ID_VAL,    NULL,     &obj);
	alist[1] = make_alist( "radius",    INT,    NULL,     &radius);
	alist[2] = make_alist( "threshold", INT,    NULL,     &threshold);
	alist[3].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (obj == NULL) {
        parse_error("%s: No object specified.", func->name);
        return (NULL);
	}

	i = GetBands(V_SIZE(obj), V_ORG(obj));
	if (i != 1) {
        parse_error("%s: Object must be an image of depth 1", func->name);
        return (NULL);
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
	return(newVal(V_ORG(obj), x, y, 1, BYTE, data));
}


Var *
ff_ccount(vfuncptr func, Var * arg)
{
    Var *v, *obj = NULL;
    int threshold = 2, ignore = 1;
	size_t i, dsize;
    int top = 0, bottom = 0, value = 0;

	Alist alist[4];
	alist[0] = make_alist( "object",    ID_VAL,    NULL,     &obj);
	alist[1] = make_alist( "threshold", INT,    NULL,    &threshold);
	alist[2] = make_alist( "ignore",    INT,    NULL,     &ignore);
	alist[3].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (obj == NULL) {
        parse_error("%s: No object specified.", func->name);
        return (NULL);
	}

	i = GetBands(V_SIZE(obj), V_ORG(obj));
	if (i != 1) {
        parse_error("%s: Object must be an image of depth 1", func->name);
        return (NULL);
    }

    dsize = V_DSIZE(obj);

    for (i = 0; i < dsize; i++) {
        value = extract_int(obj, i);
        if (value > ignore)
            bottom++;
        if (value >= threshold)
            top++;
    }

	v = newVal(BSQ, 3, 1, 1, FLOAT, calloc(3, sizeof(float)));

    if (bottom == 0) {
        ((float *) V_DATA(v))[0] = (float) 0.0;
    } else {
        ((float *) V_DATA(v))[0] = (float) top / (float) bottom;
    }
    ((float *) V_DATA(v))[1] = top;
    ((float *) V_DATA(v))[2] = bottom;

    return (v);
}

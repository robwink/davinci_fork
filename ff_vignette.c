#include "parser.h"

/**
 ** vignette() - compute vignetteing function
 **/

float *
vignette(int width, int height, float r0, float offset, float i0)
{
    int cx, cy;
    float r;
    float pi = acos(-1.0);
    float *a = (float *)calloc(width * height, sizeof(float));
    int i,j;

    cx = width/2;
    cy = height/2;

    for (j = 0 ; j < height ; j++) {
        for (i = 0 ; i < width ; i++) {
            r = sqrt((i-cx)*(i-cx) + (j-cy)*(j-cy));
            a[j*width+i] = i0 * cos((r/r0) * (pi/2)) + offset;
        }
    }
    return(a);
}

/**
 ** ff_vignette() - entry point for vignette function
 **
 ** usage: vignette(width=INT, 
 **                 height=INT, 
 **                 radius=FLOAT, 
 **                 offset=FLOAT, 
 **                 intensity=FLOAT)
 **/

Var *
ff_vignette(vfuncptr func, Var *arg)
{
    int width, height;
    float radius, offset, intensity;
    float *data;
    Var *s;

    /**
     ** List the keywords this function can accept.  Order is important.
     **/
    struct keywords kw[] = {
        { "width", NULL },
        { "height", NULL },
        { "radius", NULL },
        { "offset", NULL },
        { "intensity", NULL },
        { NULL, NULL }                      /* end with NULL */
    };

    /**
     ** Parse up the arguments.
     **/
    if (evaluate_keywords(func, arg, kw)) {
        return(NULL);
    }

    /**
     ** Extract the value for each keyword.  
     ** Bail (with an error message) if any important ones weren't given
     **/
    if (KwToInt("width", kw, &width) <= 0) {
        parse_error("Invalid value for \"width\"");
        return(NULL);
    }
    if (KwToInt("height", kw, &height) <= 0) {
        parse_error("Invalid value for \"height\"");
        return(NULL);
    }
    if (KwToFloat("radius", kw, &radius) <= 0) {
        parse_error("Invalid value for \"radius\"");
        return(NULL);
    }
    if (KwToFloat("offset", kw, &offset) <= 0) {
        parse_error("Invalid value for \"offset\"");
        return(NULL);
    }
    if (KwToFloat("intensity", kw, &intensity) <= 0) {
        parse_error("Invalid value for \"intensity\"");
        return(NULL);
    }

    /**
     ** Create the output object.  All of these are required.
     **/
    s = newVar();
    V_TYPE(s) = ID_VAL;             /* typical */
    V_DSIZE(s) = width*height;      /* number of elements */
    V_FORMAT(s) = FLOAT;            /* data format */
    V_ORG(s) = BSQ;                 /* data organization */
    V_SIZE(s)[0] = width;           /* size of first axis  (BSQ[0] = x-axis) */
    V_SIZE(s)[1] = height;          /* size of second axis (BSQ[1] = y-axis) */
    V_SIZE(s)[2] = 1;               /* size of third axis  (BSQ[2] = z-axis) */

    /**
     ** Actually do the computation, and return the new object 
     **/
    V_DATA(s) = vignette(width, height, radius, offset, intensity);
    return(s);
}

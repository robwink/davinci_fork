#include "parser.h"
/**
 ** This function computes some standard statistics of data:
 **						minimum value
 **						maximum value
 **                     mean,
 **                     average deviation
 **                     standard deviation
 **                     variance
 **                     skewness
 **             and kurtosis
 **
 ** values are returned in a 6x1x1 array of floats, in the above order
 **/

Var *
ff_moment(vfuncptr func, Var *arg)
{
    Var *v, *out;
    void *data;
    float *fdata;
    int dsize;
    int i;
    double ave,adev,sdev,var,skew,curt,s,p;
    double maxval, minval, d;

    if ((v = verify_single_arg(func, arg)) == NULL) return(NULL);

    dsize = V_DSIZE(v);

    if (dsize < 2) {
        parse_error("Unable to compute moments for data with less than 2 elements.");
        return(NULL);
    }

    s = 0;
    maxval = minval = extract_double(v,0);
    for (i = 0 ; i < dsize ; i++) {
        d = extract_double(v,i);
        s = s + d;
        if (d > maxval) maxval = d;
        if (d < minval) minval = d;
    }

    ave = s/dsize;
    adev = 0;
    var =0;
    skew =0;
    curt=0;
    for (i = 0 ; i < dsize ; i++) {
        s = extract_double(v,i)-ave;
        adev=adev+fabs(s);
        p=s*s;
        var=var+p;
        p=p*s;
        skew=skew+p;
        p=p*s;
        curt=curt+p;
    }
    adev=adev/dsize;
    var=var/(dsize-1);
    sdev=sqrt(var);
    if (var != 0) {
        skew=skew/(dsize*sdev*sdev*sdev);
        curt=curt/(dsize*var*var)-3;
    } else {
        skew = curt = 0;
    }

    fdata = (float *)calloc(sizeof(float), 8);
    fdata[0] = (float) minval;
    fdata[1] = (float) maxval;
    fdata[2] = (float) ave;
    fdata[3] = (float) adev;
    fdata[4] = (float) sdev;
    fdata[5] = (float) var;
    fdata[6] = (float) skew;
    fdata[7] = (float) curt;

    out = newVar();
    V_TYPE(out) = ID_VAL;

    V_DATA(out) = fdata;
    V_DSIZE(out) = 8;
    V_SIZE(out)[0] = 8;
    V_SIZE(out)[1] = 1;
    V_SIZE(out)[2] = 1;
    V_ORG(out) = BSQ;
    V_FORMAT(out) = FLOAT;

    return(out);
}

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
    Var *v=NULL, *out;
    float *fdata;
    int dsize;
    int i;
    double ave,adev,sdev,var,skew,curt,s,p;
    double maxval, minval, d;

	Alist alist[2];
	alist[0] = make_alist( "object",    ID_VAL,    NULL,    &v);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);
	if (v == NULL) {
		parse_error("Not enough arguments to function: %s()", func->name);
		return(NULL);
	}

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

/**
 ** This function computes some standard statistics of data:
 **						minimum value
 **						maximum value
 **                     mean,
 **                     average deviation
 **                     standard deviation
 **                     variance
 **                     skewness
 **                     kurtosis
 **                     sum
 **                 and count
 **
 ** Results are returned in a structure
 ** The user may designate a value for deleted points using the ignore value
 **/

Var *
ff_moments(vfuncptr func, Var *arg)
{
    Var *v = NULL, *out;
    int dsize;
    int i;
    double ave,adev,sdev,var,skew,curt,s,p;
    double maxval, minval, d, sum;
	int count = 0;
	Var *ignore = NULL;
	double ign_val;

	Alist alist[3];
	alist[0] = make_alist( "object",    ID_VAL,    NULL,    &v);
	alist[1] = make_alist( "ignore",    ID_VAL,    NULL,    &ignore);
	alist[2].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (v == NULL) {
		parse_error("%s: No object specified\n", func->name);
		return(NULL);
	}

	if (ignore != NULL) {
		ign_val = extract_double(ignore, 0);
	}

    dsize = V_DSIZE(v);

    s = 0;
    for (i = 0 ; i < dsize ; i++) {
        d = extract_double(v,i);
		if (ignore && d == ign_val) continue;
        s = s + d;
        if (count == 0 || d > maxval) maxval = d;
        if (count == 0 || d < minval) minval = d;
		count++;
    }

	if (count < 2) {
        parse_error("moment: Not enough values (<2).  Can't compute moments.");
		return(NULL);
	}

	sum = s;
    ave = s/count;
    adev = 0;
    var =0;
    skew =0;
    curt=0;
    for (i = 0 ; i < dsize ; i++) {
        d = extract_double(v,i);
		if (ignore && d == ign_val) continue;
        s = d-ave;
        adev=adev+fabs(s);
        p=s*s;
        var=var+p;
        p=p*s;
        skew=skew+p;
        p=p*s;
        curt=curt+p;
    }
    adev=adev/count;
    var=var/(count-1);
    sdev=sqrt(var);
    if (var != 0) {
        skew=skew/(count*sdev*sdev*sdev);
        curt=curt/(count*var*var)-3;
    } else {
        skew = curt = 0;
    }

	out = new_struct(9);
	add_struct(out, "min", newDouble(minval));
	add_struct(out, "max", newDouble(maxval));
	add_struct(out, "avg", newDouble(ave));
	add_struct(out, "avgdev", newDouble(adev));
	add_struct(out, "stddev", newDouble(sdev));
	add_struct(out, "variance", newDouble(var));
	add_struct(out, "skewness", newDouble(skew));
	add_struct(out, "kurtosis", newDouble(curt));
	add_struct(out, "sum", newDouble(sum));
	add_struct(out, "count", newDouble((double)count));

	return(out);
}

#include "parser.h"

/**
 ** hstretch()
 ** stretch image based on histogram modification
 **/

/*
"linear", "low", "high"
"clip", "shift"
"contour", "interval"
"alarm", "value"
"table", "table"
"itable", "table"
"pstretch", "ampl", "freq", "phi", "dc"
"smooth", 
"gauss", "gmean", "gsigma"
"log", "low", "high", "curve"
"ellipse", 
"power",
"peak", "range", "factor", "percent"
"mean", "range", "factor", "percent"
"astretch", "percent", "hpercent", "lpercent"
"post", "low", "high"
*/
static float *cdf_gauss(int dnmin, int dnmax, float gsigma, float amean);
static float *cdf_smooth(int dnmin, int dnmax);
static float *cdf_ellipse(int dnmin, int dnmax);
int rnd(float f);
float *hmod(int nhist, float *hist, int dnmin, int dnmax, float *cdf, int npts);


Var *
ff_hstretch(vfuncptr func, Var * arg)
{

    Var *obj=NULL, *histogram=NULL;
    const char *types[] = { "linear", "gauss","smooth","ellipse" };
    int dnmin=0, dnmax = 255;
    const char *type = "gauss";
    float gmean=127, gsigma = 3.0;

    int npts=0;
    int hsize;
    float * cdf;
    float * out;
    float *x, *y;
    int i;
    Var *o, *s;
    Var *new = NULL;
    Var *vx, *vy, *vcdf;
    int lhist = 0;
    int debug = 0;

    float minval = -MAXFLOAT;
    float maxval = MAXFLOAT;

    Alist alist[11];
    alist[0] = make_alist( "object",    ID_VAL,    NULL,    &obj);
    alist[1] = make_alist( "type",      ID_STRING, NULL,   &type);
    alist[2] = make_alist( "histogram", ID_VAL,    NULL,    &histogram);
    alist[3] = make_alist( "minval",    FLOAT,   NULL,    &minval);
    alist[4] = make_alist( "maxval",    FLOAT,   NULL,    &maxval);

/* these args a specific to gaussian */
    alist[5] = make_alist( "gmean",     FLOAT,     NULL,    &gmean);
    alist[6] = make_alist( "gsigma",    FLOAT,     NULL,    &gsigma);
    alist[7] = make_alist( "dnmin",     INT,   NULL,    &dnmin);
    alist[8] = make_alist( "dnmax",     INT,   NULL,    &dnmax);
    alist[9] = make_alist( "debug",     INT,   NULL,    &debug);
    alist[10].name = NULL;

    if (parse_args(func, arg, alist) == 0) return(NULL);

    if (obj == NULL && histogram == NULL) {
        parse_error("%s: No object specified\n", func->name);
        return(NULL);
    }
    if (!strncmp(type, "gauss", 5)) {
        cdf = cdf_gauss(dnmin, dnmax, gsigma, gmean);
    } else if (!strcmp(type, "smooth")) {
        cdf = cdf_smooth(dnmin, dnmax);
    } else if (!strcmp(type, "ellipse")) {
        cdf = cdf_ellipse(dnmin, dnmax);
    } else {
        printf("%s: unknown stretch type: %s", func->name, type);
        return(NULL);
    }

    if (histogram == NULL) {
        Var * args = create_args(3, 
                                NULL,    obj, 
                                "steps", newInt(65536),
                                "compress", newInt(1),
                                NULL,NULL);
        if (minval != -MAXFLOAT) {
            args = append_arg(args, "start", newFloat(minval));
        }
        histogram = V_func("histogram", args);
        lhist = 1;
    }
    /* split the histogram into x and y
     * We'll use the x later as part of the lookup 
     */

    hsize = V_DSIZE(histogram) / 2;
    x = (float *)calloc(hsize, sizeof(float));
    y = (float *)calloc(hsize, sizeof(float));
    for (i = 0 ; i < hsize ; i++) {
        x[i] = extract_float(histogram, cpos(0,i,0, histogram));
        y[i] = extract_float(histogram, cpos(1,i,0, histogram));
        npts += y[i];
    }

    if (minval > -MAXFLOAT || maxval < MAXFLOAT) {
        for (i = 0 ; i < hsize ; i++) {
            if (x[i] <= minval) {
                npts -= y[i];
                y[i] = 0;
            }
            if (x[i] >= maxval) {
                npts -= y[i];
                y[i] = 0;
            }
        }
    }

    vx = newVal(BSQ, 1, hsize, 1, FLOAT, x),
    vy = newVal(BSQ, 1, hsize, 1, FLOAT, y),
    vcdf = newVal(BSQ, dnmax-dnmin+1, 1, 1, FLOAT, cdf);
    out = hmod(hsize, y, dnmin, dnmax, cdf, npts);
    o = newVal(BSQ, 1, hsize, 1, FLOAT, out);

    if (obj != NULL) {
        Var *args = create_args(3,
                                NULL, o,
                                NULL, vx,
                                NULL, obj,
                                NULL, NULL);
        new = V_func("interp", args);
        if (dnmin >= 0 && dnmax <= 255) {
            new = V_func("byte", create_args(1, NULL, new, NULL, NULL));
        }
    }

    if (debug) {
        s = new_struct(0);
        add_struct(s, "x", vx);
        add_struct(s, "y", vy);
        add_struct(s, "o", o);
        add_struct(s, "cdf", vcdf),
        add_struct(s, "out", new);
        return(s);
    }

    if (new == NULL) {
        Var *args = create_args(3,
                                NULL, vx,
                                NULL, o,
                                "axis", newString(strdup("x")),
                                NULL, NULL);
        return(V_func("cat", args));
    } else {
        return(new);
    }
}

static float *
cdf_gauss(int dnmin, int dnmax, float gsigma, float amean)
{
    float sigma, a, b, ss, sum;
    float *out;
    int npts, i;

    sigma = (dnmax - dnmin + 1) / (2 * gsigma);
    a = 1.0 / (sqrt(2.0 * 3.14159) * sigma);
    b = -1.0 / (2.0 * sigma*sigma);
    npts = dnmax - dnmin + 1;
    sum = 0.0;

    out = calloc(sizeof(float), npts);

    for (i = dnmin; i <= dnmax; i+=1) {
        ss = (dnmin + i - 1)-amean;
        sum = sum + a * exp(b * ss * ss);
        out[i-dnmin] = sum;
    }
    return(out);
}

static float *
cdf_smooth(int dnmin, int dnmax)
{
    float *out;
    int npts, i;
	float anorm;

    npts = dnmax - dnmin + 1;
    out = calloc(sizeof(float), npts);

    for (i = 0; i < npts; i+=1) {
		out[i] = (float)i/npts;
    }
    return(out);
}

static float *
cdf_ellipse(int dnmin, int dnmax)
{
    float *out;
    int npts, i;
    float b,p,bsq,sum,bsqxsq;
    float x,y, anorm;
	int noffset;

    npts = dnmax - dnmin + 1;
    out = calloc(sizeof(float), npts);

    b = (dnmax-dnmin+1)/2.0;
    p = 0.5;
    bsq = b*b;
    noffset = rnd(b);
    sum = 0;
   
    for (i = dnmin; i <= dnmax; i+=1) {
    	x = i - noffset;
		bsqxsq = bsq-x*x;
		if (bsqxsq < 0) bsqxsq= 0;
		sum = sum + p*sqrt(bsqxsq);
    }
    anorm = npts / sum;

    for (i = dnmin; i <= dnmax; i+=1) {
    	x = i - noffset;
		bsqxsq = bsq-x*x;
		if (bsqxsq < 0) bsqxsq = 0;
		sum += anorm*p*sqrt(bsqxsq);
		out[i] = rnd(sum);
    }
    return(out);
}

/*
** This might be the wrong way to do this, but it works with interp()
*/

float *
hmod(int nhist, float *hist, int dnmin, int dnmax, float *cdf, int npts)
{
    float *out;
    int i, j;
    int cumsum;
    int nlev = dnmax-dnmin+1;

    out = calloc(nhist, sizeof(float));

/*
    hist has thousands of bins
    cdf has 256 levels
    skip through hist accumulating values until they exceed the cdf,
    and assigning dn values to them all the while
*/
    cumsum = 0;
    j = 0;
    for (i = 0 ; i < nhist ; i++) {
        if (cumsum + hist[i] < cdf[j] * npts) {
            out[i] = j+dnmin;
            cumsum += hist[i];
        } else {
            while (cumsum + hist[i] >= cdf[j] * npts && j < nlev) {
                j++;
            }
            out[i] = j+dnmin;
            cumsum += hist[i];
        }
        if (j == nlev) {
            /* Everything else gets the maxval */
            for ( ; i < nhist ; i++) {
                out[i] = j+dnmin;
            }
        }
    }
    return(out);
}

#if 0
/* given a histogram and a CDF, create a lookup table */

hmod(Var * histogram, dnmin, dnmax, float *cdf, int npts)
{

    nlev = dnmax - dnmin + 1;
    cumsum = 0;
    idn = dnmin;

    // form cdf by integrating to form best approx. using
    // discrete pdf

    for (i = 1; i <= nlev; i += 1) {
        if ((cumsum+hist[idn]) > (cdf[i]*npts)) {
            continue;
        }
        // continue summation for current dn level
        cumsum = cumsum + hist[idn];
        lut[idn++] = dnmin + i - 1;
        if (i >= nlev) break;
        if (idn <= dnmax) {
            i--;
            continue;
        }
        break;
    }

    //  summation complete
    if (idn <= dnmax) {
        for (i = idn; i <= dnmax; i += 1) {
            lut[i] = dnmax;
        }
    }
}
#endif

int rnd(float f) 
{
	if (f > 0) {
		return(f+0.5);
	} else {
		return(f-0.5);
	}
}



Var *
ff_sstretch2(vfuncptr func, Var * arg)
{

  /* takes data and stretches it similar to sstretch() davinci function*/
  /* but does it band by band */

  typedef unsigned char byte;
  
  Var       *data=NULL;         /* input */
  Var       *out=NULL;          /* output */
  byte      *w_data2=NULL;      /* working data */
  float      ignore=-32768;     /* ignore value*/
  size_t     x,y,z;             /* indices */
  double     sum = 0;           /* sum of elements in data */
  double     sumsq = 0;         /* sum of the square of elements in data */
  double     stdv = 0;          /* standard deviation */
  size_t     cnt = 0;           /* total number of non-null points */
  size_t     i,j,k;
  float      tv,max=-32768;
  float      v=40; 
  
  Alist alist[4];
  alist[0] = make_alist("data", 	  ID_VAL,	NULL,	&data);
  alist[1] = make_alist("ignore", 	  FLOAT,	NULL,	&ignore);
  alist[2] = make_alist("variance",       FLOAT,        NULL,   &v);
  alist[3].name = NULL;
  
  if (parse_args(func, arg, alist) == 0) return(NULL);
  
  if (data == NULL) {
    parse_error("\nsstretch() - 03/26/05\n");
    parse_error("Used to sstretch data band by band");
    parse_error("Similar to the davinci function sstretch()\n");
    parse_error("$1=data to be stretched");
    parse_error("ignore=value to ignore (Default=-32768)");
    parse_error("variance=variance of the stretch (Default=40)\n");
    parse_error("c.edwards\n");
    return NULL;
  }

  /* x, y and z dimensions of the data */
  x = GetX(data);
  y = GetY(data);
  z = GetZ(data);
  max = ignore;

  /* create out array */
  w_data2 = (byte *)calloc(sizeof(byte),x*y*z);

  if (w_data2 == NULL){
  	parse_error("sstretch: Unable to allocate %ld bytes.\n", sizeof(byte)*x*y*z);
    return NULL;
  }
  
  /* stretch each band separately */
  for(k=0;k<z;k++) {
    sum = 0;
    sumsq = 0;
    stdv = 0;
    cnt = 0;
    tv = 0;
    
    for(j=0; j<y; j++) {
        for(i=0; i<x; i++) {
            if ((tv = extract_float(data, cpos(i,j,k, data))) != ignore){
                sum += tv;
                sumsq += ((double)tv)*((double)tv);
                cnt ++;
            }
        }
    }

    stdv = sqrt((sumsq - (sum*sum/cnt))/(cnt-1));
    sum /= cnt;

    /*convert to bip */
    for(j=0; j<y; j++) {
        for(i=0; i<x; i++) {
            if ((tv = extract_float(data, cpos(i,j,k, data))) != ignore){
                tv = (float)((tv - sum)*(v/stdv)+127);
            }
            if (tv < 0) tv = 0;
            if (tv > 255) tv = 255;

            w_data2[j*x*z + i*z + k]=(byte)tv;
        }
    }
  }
  
  
  /* clean up and return data */
  out = newVal(BIP,z, x, y, BYTE, w_data2);	
  return(out);
}

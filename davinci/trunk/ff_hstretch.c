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


Var *
ff_hstretch(vfuncptr func, Var * arg)
{

	Var *obj=NULL, *histogram=NULL;
	char *types[] = { "linear", "gauss" };
	float gmean, gsigma;
	char *type;
	int x, y, z, dsize;

	Alist alist[9];
	alist[0] = make_alist( "object",    ID_VAL,    NULL,    &obj);
	alist[1] = make_alist( "type",      ID_ENUM,      types,   &type);
	alist[2] = make_alist( "gmean",     FLOAT,     NULL,    &gmean);
	alist[3] = make_alist( "gsigma",    FLOAT,     NULL,    &gsigma);
	alist[4] = make_alist( "histogram", ID_VAL,    NULL,    &histogram);
	alist[5].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (obj == NULL) {
		parse_error("%s: No object specified\n", func->name);
		return(NULL);
	}

	x = GetSamples(V_SIZE(obj), V_ORG(obj));
	y = GetLines(V_SIZE(obj), V_ORG(obj));
	z = GetBands(V_SIZE(obj), V_ORG(obj));
	dsize = V_DSIZE(obj);

	if (histogram == NULL) {
		Var * args = create_args(3, 
								NULL,    obj, 
								"steps", newInt(65536),
								"compress", newInt(1),
								NULL,NULL);
		histogram = V_func("histogram", args);
	}
}

/* generate a CDF */
static float *gauss(dnmin, dnmax, gsigma, amean)
{
	float sigma, a, b, ss, sum;
	float *out;
	int nent, i;

	sigma = (dnmax - dnmin + 1) / (2 * gsigma);
	a = 1.0 / (sqrt(2.0 * 3.14159) * sigma);
	b = -1.0 / (2.0 * sigma*sigma);
	nent = dnmax - dnmin + 1;
	sum = 0.0;

	out = calloc(sizeof(float), nent);

	for (i = dnmin; i <= nent; i+=1) {
		ss = (dnmin + i - 1)-amean;
		sum = sum + a * exp(b * ss * ss);
		out[i] = sum;
	}
	return(out);
}

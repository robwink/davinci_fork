#include "parser.h"
#include "fit.h"

/**
** ff_fit() - non-linear least-squares fitting function
**
** usage:      ff_fit(object=VAR, type=TYPE, start=VAR, iterations=VAR)
**
**     object  - Sample data to fit
 **     type    - Type of fit to perform.  Options are:
 **                     gauss   - Gaussian
 **                     gaussc  - Gaussian + constant
 **                     gaussl  - Gaussian + linear
 **                     linear  - linear [default]
 **     start   - Initial values 
 **               If no inital values are given, a best guess is performed.
 **               iterations - Number of iterations to perform [1]
 **
 **  The output of the ff_fit() function is suitable for use as start value
 **  for another iteration of ff_fit()
 **
 **  If an array is passed with only one dimension, an abscissa will be
 **  provided, starting at 1.  If an array containing an dimension of 2xN
 **  is given, the subset 1xN is assumed to be the abscissa, and 2xN, the
 **  data values.
 **/

Var * lin_fit(Var *x, Var *y,int Row,int plot, double ignore);
void    first_guess(double *, char *, int);
int     fit(int,int);
ifptr   getfcnptr(char *, int *, int *, int *, char *);
int     mrqfit(double **, struct data_order, int, int, int, double *,
               int, int, double **, double *, ifptr,int);
int     alpha_beta_chisq(double **, struct data_order,int , int , double *,
                         int ,int , double **, double *, double *, ifptr);
void    gd(Var *, Var *, char *, double);
int     dfit(Var *, Var *, Var *, char *, int, double **, int *, int,int, double );


Var * ff_fit(vfuncptr func, Var *arg)
{
    Var *v = NULL;
    Var *y = NULL;
    Var *x = NULL;
    Var *ip = NULL;
    Var *s;
    double *op;
    int nparam;
    char *ftype = "linear";
    int iter = 1;
    int plot = 0 ;
	int verbose =0;
	Var *ignore_val = NULL;
	double ignore=MINFLOAT;

    char *fits[] = { "gauss", "gaussc", "gaussl", "ngauss", "lorenz",
                     "2lorenz", "linear", "quad", "cube", "poly", 
					 "nexp", "xyquad", "xygauss", "sincos", NULL };

	Alist alist[9];
	alist[0] = make_alist( "y",       ID_VAL,    NULL,    &y);
	alist[1] = make_alist( "type",    ID_ENUM,   fits,    &ftype);
	alist[2] = make_alist( "start",   ID_VAL,    NULL,    &ip);
	alist[3] = make_alist( "steps",   INT,       NULL,    &iter);
	alist[4] = make_alist( "x",       ID_VAL,    NULL,    &x);
	alist[5] = make_alist( "plot",    INT,       NULL,    &plot);
	alist[6] = make_alist( "verbose", INT,       NULL,    &verbose);
	alist[7] = make_alist( "ignore",  DOUBLE,    NULL,    &ignore);
	alist[8].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

    if (y == NULL) {
        parse_error("%s: No data specified (y=VAR)", func->name);
        return(NULL);
    }
	/*
	** we could provide a default X here if we wanted
	**
	** Ok, I did.
    ** if (x == NULL) {
    **     parse_error("%s: No x data specified (x=VAR)", func->name);
    **     return(NULL);
    ** }
	*/ 

    if (x && (V_DSIZE(x) != V_DSIZE(y))) {
        sprintf(error_buf, "X axis for data [%d points] not same size as Y axis [%d points]", 
                V_DSIZE(x), V_DSIZE(y));
        parse_error(NULL);
        return(NULL);
    }

    if (V_DSIZE(y) < 3) {
        parse_error("Can not fit less than 3 points\n");
        return(NULL);
    }

    if (!(strcmp(ftype,"linear"))) {
        s=lin_fit(x,y,V_DSIZE(x),plot, ignore);
    } else if (dfit(x, y, ip, ftype, iter, &op, &nparam, plot,verbose,ignore)) {
        s = NULL;
    } else {
        s = newVar();
        V_TYPE(s) = ID_VAL;

        V_DATA(s) = op;
        V_DSIZE(s) = nparam+2;
        V_FORMAT(s) = DOUBLE;
        V_ORDER(s) = BSQ;
        V_SIZE(s)[0] = nparam+2;
        V_SIZE(s)[2] = V_SIZE(s)[1] = 1;
    }

    return(s);
}

Var *
lin_fit(Var *x, Var *y,int Row, int plot, double ignore)
{
	double **data;
	char *tmp;
   char buf[256];
   char buf2[256];
	FILE *fp;

	int i, count;

	double **A,**Cov,*B,*r;
	double x1, y1;

	A=dmatrix(2,2);
	/* data=dmatrix(Row,2); */
	Cov=dmatrix(2,2);
	B=dvector(2);
	r=dvector(2);

	Cov[0][0]=A[0][0]=0.0;
	Cov[0][1]=A[0][1]=0.0;
	r[0]=B[0]=Cov[1][0]=A[1][0]=0.0;
	r[1]=B[1]=Cov[1][1]=A[1][1]=0.0;

	count = 0;
	for (i=0;i<Row;i++){
        x1 = ((x==NULL) ? i+1 : extract_double(x, i)); /* a */
		y1 = extract_double(y,i); /*b*/

		if (x1 == ignore || y1 == ignore) continue;

		count++;

		A[0][1]+=x1;
		A[1][0]+=x1;
		A[1][1]+=(x1*x1);

		B[0]+=y1;
		B[1]+=(x1*y1);
	}
	A[0][0]=(float)count;

	solve_for_da(A,Cov,B,r,2);

	free_dmatrix(A,2,2);
	free_dmatrix(Cov,2,2);
	free(B);

	if (plot) {
		tmp = make_temp_file_path();
		if (tmp == NULL || (fp = fopen(tmp, "w")) == NULL ) {
			parse_error("unable to open temp file");
			if (tmp) free(tmp);
			return(NULL);
		}

		for (i = 0 ; i < Row ; i++) {
			x1 = ((x==NULL) ? i+1 : extract_double(x, i)); /* a */
			y1 = extract_double(y,i); /*b*/
			if (x1 == ignore || y1 == ignore) continue;
			fprintf(fp, "%g %g\n", x1, y1);
		}

		fclose(fp);

		for (i = 0 ; i < 2 ; i++) {
			sprintf(buf, "a%d=%g\n", i, r[i]);
			send_to_plot(buf);
		}

/*		strcpy(buf2, comment); */
/*		p = strchr(buf2, '='); */
		send_to_plot("set noparametric\n");
		sprintf(buf, "plot \"%s\" using 1:2 with points, %s with lines\n", tmp, "a0+a1*x");
		free(tmp);
		send_to_plot(buf);
	}

	/* free_dmatrix(data,Row,2); */

	return(newVal(BSQ,2,1,1,DOUBLE,r));
}	



static double  *a = NULL;       /* array of parameters for fit */
static double **covar = NULL;   /* covariant matrix for fit */

static char     comment[160];
static int      num_indep = 1;  /* number of independent variables */
static struct   data_order order;
static int      ndata = 0;      /* number of data points */
static int      ma;             /* number of parameters */
static int      mfit;           /* number of parameters being varied */

static double   chisq;          /* squared error for fit */
static double   alamda;         /* how much we change the fitting parameters */
static double   **data;           /* matrix which holds data */
static int      i;              /* indices for loops */
static int      datacols = 6;   /* number of columns in the data matrix */
static int      linflag = 0;    /* is function linear */
static int      failed;         /* did function fail? */
static int      jmax;
ifptr           func = NULL;


int
dfit(Var *x, Var *y, Var *ip, char *fname, 
     int iter, double **op, int *nparam, int plot,int verbose, double ignore)
{

    FILE *fp;
    char buf[256];
    char buf2[256], *p;
    char *tmp;
    int status;
    int i;

    gd(x,y,fname,ignore);              /* load data, and function */

    *op = a = dvector(ma+2);
    *nparam = ma;

    for (i = 0 ; i < ma ; i++) {
        if (ip && i < V_DSIZE(ip)) a[i] = extract_double(ip, i);
        else a[i] = 0.0;
    }
    first_guess(a, fname,verbose);

    if (ip && V_DSIZE(ip) == ma+2) alamda = extract_double(ip, ma+1);
    else
        /* start alamda small */
        alamda = 1e-3;

    status = fit(iter,verbose);         /* fit using the given number of iterations */

    /* put return values in place */

    a[ma] = chisq;
    a[ma+1] = alamda;

    if (plot) {
		tmp = make_temp_file_path();
		if (tmp == NULL || (fp = fopen(tmp, "w")) == NULL ) {
			parse_error("unable to open temp file");
			if (tmp) free(tmp);
			return(NULL);
		}
        for (i = 0 ; i < ndata ; i++) {
            fprintf(fp, "%g %g\n", data[0][i], data[1][i]);
        }
        fclose(fp);
	for (i = 0 ; i < ma ; i++) {
		sprintf(buf, "a%d=%g\n", i, a[i]);
		send_to_plot(buf);
	}
	strcpy(buf2, comment);
	p = strchr(buf2, '=');
	send_to_plot("set noparametric\n");
	sprintf(buf, "plot \"%s\" using 1:2 with points, %s with lines\n",
		tmp, p+1);
        free(tmp);
				 
        send_to_plot(buf);
    }

    free(data);

    if (verbose) printf("%s\n", comment);
    return(failed);
}

void
first_guess(double *a, char *fname,int verbose)
{
    int i;
    int maxi=0, mini=0, left, right;
    float avg=0.0;

    double *x, *y;

    x = data[0];
    y = data[1];


    /**
     ** find minimum and maximum.
     **/
        
    for (i = 0 ; i < ndata ; i++) {
        if (i == 0 || y[maxi] < y[i]) {
            maxi = i;
        }
        if (i == 0 || y[mini] > y[i]) {
            mini = i;
        }
        avg += y[i]/ndata;
    }
    if (avg == 0.0) avg = 1.0;

    if (!strncmp(fname, "gauss", 5)) {
        /* guess at left and right extents for width  */

        left = max(maxi-1, 0);
        right = min(maxi+1, ndata);
        while(left > 0 && y[left] <= y[maxi]) left--;
        while(right < (ndata-1) && y[right] <= y[maxi]) right++;

        if (a[0] == 0.0) a[0] = x[maxi]; /* center */

        if (a[1] == 0.0) {
            a[1] = x[right] - x[left]; /* width */
            if (a[1] == 0.0) {
                a[1] = (x[ndata-1]-x[0])/2;
            }
        }
        if (a[2] == 0.0) a[2] = y[maxi]-y[mini]; /* multiplier */

        if (!strcmp(fname, "gaussc") || !strcmp(fname, "gaussl")) {
            if (avg == 0.0) avg = 1.0;
            if (a[3] == 0.0) a[3] = avg; /* constant additive term */
        }
        if (!strcmp(fname, "guassl"))  { /* linear additive term */
            if (a[4] == 0.0) {
                a[4] = (x[maxi] > x[mini] ? 1.0 : -1.0);
            }
        }
    } else {
        if (a[0] == 0.0) a[0] = avg;
        if (a[1] == 0.0) a[1] = (x[maxi] > x[mini] ? 1.0 : -1.0);
    }

	if (verbose)
		fprintf(stderr, "guess: %f %f %f\n", a[0], a[1], a[2]);
}

void
gd(Var *x, Var *y, char *fname, double ignore)
{
    data = dmatrix(datacols, V_DSIZE(y));
    jmax = 2;
	ndata = 0;
    for (i = 0 ; i < V_DSIZE(y) ; i++) {
		/* this supplies a fake x axis if one isn't present. */
		/* also handles an ignore value */
        data[0][i] = ((x==NULL) ? i+1 : extract_double(x, i));
        data[1][i] = extract_double(y, i);
		if (data[0][i] == ignore || data[1][i] == ignore) continue;

        data[jmax][i] = 1;      /* weighting column of ones */
		ndata++;
    }

    order.nsig = jmax + 0;      /* data column to be all ones: no weighting */
    order.ssig = jmax + 2;      /* data column used for statistical weighting */
    order.yfit = jmax + 1;      /* assign a data column for yfit */

    if (jmax == 2 && num_indep == 1) order.sig = 2;
    if (jmax == 3 && num_indep == 2) order.sig = 3;

    if (!(func = getfcnptr(fname, &num_indep, &linflag, &ma, comment))) {
        printf("Function %s not found, try lf to list functions\n", fname);
        return;
    }

    order.x = ivector(num_indep);
    order.xsig = ivector(num_indep);

    for (i = 0; i < num_indep; i++) { /* assign default values */
        order.x[i] = i;
        order.xsig[i] = -1;
    }
    order.y = num_indep;        /* dependent variable column */

    covar = dmatrix(ma, ma);
    mfit = ma;
}

int
fit(int itmax,int verbose) 
{
    /* nonzero returns 1 if all sigma's are non-zero */
    if (nonzero(data[order.sig], ndata)) {
        failed = mrqfit(data, order, num_indep, ndata, itmax, a, ma,
                        mfit, covar, &chisq, func,verbose);
    } else {
        failed = 1;
        printf("all sigmas must be non-zero, check weighting and order\n");
    }
    if (failed != 0) printf("fit failed\n");

    /* free allocated arrays (except data, which is free'd later) */

    if (covar != NULL) free_dmatrix(covar, ma, ma);
    free(order.x);
    free(order.xsig);

    return(failed);
}




/* calculates alpha, beta, and chisq */
/* see Numerical Recipes in C for definition of these */
/* the Levenberg Marquardt algorithm requires us to slove */
/* the equation alpha*da = beta (where alpha is a matrix */
/* and da and beta are vectors) for da */
/* this function does this by gauss-jordan */
/* elimination */

/* declarations for data declared externally */
void            myerror(char *);

int 
mrqfit(double **data,
       struct data_order order,
       int num_indep,
       int ndata,
       int itmax,
       double *a,
       int ma,
       int mfit,
       double **covar,
       double *chisq,
       ifptr func,
		 int verbose)
{
    int             i, j, k;    /* indices for loops */
    double        **alpha;
    double         *beta;
    double         *atry;       /* parameters to try and see if we reduce chisqr */
    double        **alpha_try;
    double          ochisq;     /* best chisqr so far */
    double         *da;

    /* allocate space for arrays */
    atry = dvector(ma);
    alpha_try = dmatrix(ma, ma);
    beta = dvector(ma);
    da = dvector(ma);
    alpha = dmatrix(ma, ma);


    /* calculate alpha, beta, and chisq for current value of parameters */
    if (alpha_beta_chisq(data, order, num_indep, ndata, a, ma, mfit,
                         alpha, beta, chisq, func)) {
        free_dmatrix(alpha, ma, ma);
        free(atry);
        free_dmatrix(alpha_try, ma, ma);
        free(beta);
        free(da);
        return 0;
    }
    i = 0;

    /* loop specified number of iterations */
    while (i <= itmax) {
		if (verbose) fprintf(stderr, "iteration %d\n", i);
        ochisq = *chisq;
        for (j = 0; j < mfit; j++) {
            for (k = 0; k < mfit; k++) {
                alpha_try[j][k] = alpha[j][k];
            }
            alpha_try[j][j] = alpha[j][j] + alamda;
        }
        solve_for_da(alpha_try, covar, beta, da, mfit);

        for (j = 0; j < ma; j++) atry[j] = a[j];
        for (j = 0; j < mfit; j++) atry[j] += da[j];

        if (alpha_beta_chisq(data, order, num_indep, ndata, atry, ma,
                             mfit, alpha, beta, chisq, func)) {
            free_dmatrix(alpha, ma, ma);
            free(atry);
            free_dmatrix(alpha_try, ma, ma);
            free(beta);
            free(da);
            return 0;
        }
        if (*chisq >= ochisq) {
            alamda *= 10;
            if (alamda > 1e15)
                alamda /= 3e14;
            *chisq = ochisq;
        } else {
            alamda *= 0.1;
            if (alamda < 1e-15)
                alamda *= 3e14;
            for (j = 0; j < mfit; j++)
                a[j] = atry[j];
        }

        if (verbose) {
            for (j = 0; j < ma; j++)
                printf("a%d= %g\t", j, a[j]);
            printf("\nchisqr = %g", *chisq);
            printf("\nalamda = %g", alamda);
            printf("\n");
        } else
			if (verbose) 
				printf("iteration: %d chisqr: %g\n", i, *chisq);
        i++;

    }
    free_dmatrix(alpha, ma, ma);
    free(atry);
    free_dmatrix(alpha_try, ma, ma);
    free(beta);
    free(da);

    return 0;
}


int 
alpha_beta_chisq(double **data, struct data_order order,
                 int num_indep, int ndata, double *a, int ma,
                 int mfit, double **alpha, double *beta, double *chisq,
                 ifptr funcs)
{
    int             i, j, k;
    double         *x, *y, *sig, *xsig;
    double          ymod, *dyda, sig2i, wt, dy;
    int            *fita, *dydx_flag;
    double         *dydx;

    dyda = dvector(ma);
    fita = ivector(ma);
    x = dvector(num_indep);
    dydx_flag = ivector(num_indep);
    dydx = dvector(num_indep);

    y = data[order.y];
    sig = data[order.sig];

    for (i = 0; i < num_indep; i++) {
        if (order.xsig[i] >= 0) {
            xsig = data[order.xsig[i]];
            dydx_flag[i] = 1;
        } else
            dydx_flag[i] = 0;
    }

    for (i = 0; i < ma; i++) {
        fita[i] = 0;
        for (j = 0; j < mfit; j++)
            if (j == i)
                fita[i] = 1;
    }

    *chisq = 0;
    for (j = 0; j < mfit; j++) {
        beta[j] = 0;
        for (k = 0; k < mfit; k++)
            alpha[j][k] = 0;
    }

    for (i = 0; i < ndata; i++) {
        for (j = 0; j < num_indep; j++) {
            x[j] = data[order.x[j]][i];
        }

        if ((*funcs) (x, a, &ymod, dyda, ma, 1, fita, dydx_flag, dydx, y[i]))  {
            free(dyda);
            free(fita);
            free(x);
            free(dydx_flag);
            free(dydx);
            return 1;
        }
        data[order.yfit][i] = ymod;
        sig2i = (sig[i] * sig[i]);
        for (j = 0; j < num_indep; j++)
            if (order.xsig[j] >= 0)
                sig2i += (dydx[j]) * (dydx[j]) * xsig[i] * xsig[i];
        dy = y[i] - ymod;
        for (j = 0; j < mfit; j++) {
            wt = dyda[j] / sig2i;
            for (k = 0; k <= j; k++)
                alpha[j][k] += wt * dyda[k];
            beta[j] += dy * wt;
        }
        *chisq += dy * dy / sig2i;
    }

    for (j = 1; j < mfit; j++)
        for (k = 0; k <= j - 1; k++)
            alpha[k][j] = alpha[j][k];

    free(dyda);
    free(fita);
    free(x);
    free(dydx_flag);
    free(dydx);
    return 0;
}
#define SWAP(a,b) {double temp=(a);(a)=(b);(b)=temp;}

/* gaussj inverts the matrix a and performs */
/* the same operations on the matrix b */
/* This function is very similar to the one in */
/* Numerical Recipes of the same name. */

void 
gaussj(double **a, int n, double **b, int m)
{
    int            *indxc, *indxr, *ipiv;
    int             i, icol, irow, j, k, l, ll;
    double          big, dum, pivinv;

    indxc = ivector(n);
    indxr = ivector(n);
    ipiv = ivector(n);
    for (j = 0; j < n; j++)
        ipiv[j] = 0;
    for (i = 0; i < n; i++) {
        big = 0.0;
        for (j = 0; j < n; j++) {
            if (ipiv[j] != 1) {
                for (k = 0; k < n; k++) {
                    if (ipiv[k] == 0) {
                        if (fabs(a[j][k]) >= big) {
                            big = fabs(a[j][k]);
                            irow = j;
                            icol = k;
                        }
                    } else if (ipiv[k] > 1) {
                        myerror("GAUSSJ: Singular Matrix-1");
                    }
                }
            }
        }
        ++(ipiv[icol]);
        if (irow != icol) {
            for (l = 0; l < n; l++)
                SWAP(a[irow][l], a[icol][l]);
            for (l = 0; l < m; l++)
                SWAP(b[irow][l], b[icol][l]);
        }
        indxr[i] = irow;
        indxc[i] = icol;
        if (a[icol][icol] == 0.0) {
            myerror("GAUSSJ: Singular Matrix-2");
        }
        pivinv = 1.0 / a[icol][icol];
        a[icol][icol] = 1.0;
        for (l = 0; l < n; l++) a[icol][l] *= pivinv;
        for (l = 0; l < m; l++) b[icol][l] *= pivinv;
        for (ll = 0; ll < n; ll++)
            if (ll != icol) {
                dum = a[ll][icol];
                a[ll][icol] = 0.0;
                for (l = 0; l < n; l++) a[ll][l] -= a[icol][l] * dum;
                for (l = 0; l < m; l++) b[ll][l] -= b[icol][l] * dum;
            }
    }
    for (l = n - 1; l >= 0; l--) {
        if (indxr[l] != indxc[l])
            for (k = 0; k < n; k++) SWAP(a[k][indxr[l]], a[k][indxc[l]]);
    }
    free(ipiv);
    free(indxr);
    free(indxc);
}

#undef SWAP

/* This program solves the equation alpha*da = beta for da. */
/* alpha is a matrix and da and beta are vectors */
/* covar is used for temporary space */

void 
solve_for_da(double **alpha, double **covar, double *beta, double *da, int mfit)
{
    int             i, j;
    double        **mbeta;

    mbeta = dmatrix(mfit, 1);

    for (i = 0; i < mfit; i++) {
        mbeta[i][0] = beta[i];
        for (j = 0; j < mfit; j++)
            covar[i][j] = alpha[i][j];
    }
    gaussj(covar, mfit, mbeta, 1);
    for (i = 0; i < mfit; i++)
        da[i] = mbeta[i][0];
    free_dmatrix(mbeta, mfit, 1);
}


/* This file contains a bunch of function for allocating */
/* vectors and matrices.  */


static double        **
dmatrix(int nr, int nc)
{
    int             i;
    double        **m;
    char            s[80];

    m = (double **) malloc((unsigned) (nr) * sizeof(double *));
    if (!m)
        myerror("allocation failure 1 in dmatrix()");

    for (i = 0; i < nr; i++) {
        m[i] = (double *) malloc((unsigned) (nc) * sizeof(double));
        sprintf(s, "allocation failure 2 in dmatrix() on column %d", i);
        if (!m[i])
            myerror(s);
    }
    return m;
}

int           **
imatrix(int nr, int nc)
{
    int             i, **m;

    m = (int **) malloc((unsigned) (nr) * sizeof(int *));
    if (!m)
        myerror("allocation failure 1 in imatrix()");

    for (i = 0; i < nr; i++) {
        m[i] = (int *) malloc((unsigned) (nc) * sizeof(int));
        if (!m[i])
            myerror("allocation failure 2 in imatrix()");
    }
    return m;
}

void 
free_dmatrix(double **m, int nr, int nc)
{
    int             i;

    for (i = nr - 1; i >= 0; i--)
        free((char *) (m[i]));
    free((char *) (m));
}

void 
free_imatrix(int **m, int nr, int nc)
{
    int             i;

    for (i = nr - 1; i >= 0; i--)
        free((char *) m[i]);
    free((char *) (m));
}

void 
myerror(char s[80])
{
    printf("%s\n", s);
}

static double         *
dvector(int n)
{
    return (double *) malloc((unsigned) (n) * sizeof(double));
}

static int            *
ivector(int n)
{
    return (int *) malloc((unsigned) (n) * sizeof(int));
}


int 
nonzero(double *array, int ndata)
{
    int             i;
    for (i = 0; i < ndata; i++)
        if (array[i] < 1e-60)
            return 0;
    return 1;
}


#define NUM_FCNS 21             /* CHANGE if you define more than 20 functions */

extern int      debug;
/* a few function declarations */
int             fgauss(double *x, double *a, double *y, double *dyda, int na, int dyda_flag, int *fita, int *dydx_flag, double *dydx, double ydat);
int             fgaussc(double *x, double *a, double *y, double *dyda, int na, int dyda_flag, int *fita, int *dydx_flag, double *dydx, double ydat);
int             fgaussl(double *x, double *a, double *y, double *dyda, int na, int dyda_flag, int *fita, int *dydx_flag, double *dydx, double ydat);
int             fgaussn(double *x, double *a, double *y, double *dyda, int na, int dyda_flag, int *fita, int *dydx_flag, double *dydx, double ydat);
int             florenz(double *x, double *a, double *y, double *dyda, int na, int dyda_flag, int *fita, int *dydx_flag, double *dydx, double ydat);
int             florenz2(double *x, double *a, double *y, double *dyda, int na, int dyda_flag, int *fita, int *dydx_flag, double *dydx, double ydat);
int             fline(double *x, double *a, double *y, double *dyda, int na, int dyda_flag, int *fita, int *dydx_flag, double *dydx, double ydat);
int             fquad();
int             fpoly(double *x, double *a, double *y, double *dyda, int na, int dyda_flag, int *fita, int *dydx_flag, double *dydx, double ydat);
int             fexpn(double *x, double *a, double *y, double *dyda, int na, int dyda_flag, int *fita, int *dydx_flag, double *dydx, double ydat);
int             fxyquad(double *x, double *a, double *y, double *dyda, int na, int dyda_flag, int *fita, int *dydx_flag, double *dydx, double ydat);
int             fxygauss(double *x, double *a, double *y, double *dyda, int na, int dyda_flag, int *fita, int *dydx_flag, double *dydx, double ydat);
int             fconic();
int             fsincos(double *x, double *a, double *y, double *dyda, int na, int dyda_flag, int *fita, int *dydx_flag, double *dydx, double ydat);

/* The structure fcn holds the information about each function */
/* Specifically, it is here that the name used by the command line, */
/* the name of the function in c, the number of parameters, */
/* and a comment are defined.  If gnuplot is used for plotting, */
/* then the comment must be a valid definition of the function */
/* on the gnuplot command line. */
/* to add a function, you must add a line to the fcn initialization */
/* For a function with a variable number of parameters */
/* you must initialize na to be -1.  You have to choose the number */
/* of parameters when you choose the function at the fit> command */
/* prompt. */
/* If a comment begins with "FILE", plotting is done through a */
/* tempory file.  For functions with variable number of parameters, */
/* you must begin your comment with "FILE" */
/* Functions of two independent variables which are not plotted */
/* through a file should be expressed in terms of u and v instead */
/* of x and y.  This is because gnuplot wants it that way. */

/* The elements of the struct routine are as follows: */
/* char name[20]    the name of the function at the fit> command line */
/* int num_indep   the number of independent variables */
/* int linflag   Is the function linear in the parameters? */
/* linflag = 0 for a non-linear function.  linflag = 1 for */
/* a function linear in the parameters */
/* int (*func)()   the name of the function as known to the */
/* compiler */
/* int na        the number of parameters, -1 for variable number */
/* char comment[160] a comment about the function */

struct routine {
    char            name[20];
    int             num_indep;
    int             linflag;
    ifptr           func;
    int             na;
    char            comment[160];
}               fcn[NUM_FCNS] = {
    { "gauss", 1, 0, &fgauss, 3, "f(x) = a2*exp(-((x-a0)/a1)**2)" },
    { "gaussc", 1, 0, &fgaussc, 4, "f(x) = a2*exp(-((x-a0)/a1)**2) + a3" },
    { "gaussl", 1, 0, &fgaussl, 5, "f(x) = a2*exp(-((x-a0)/a1)**2) + a3 + a4*x" },
    { "ngauss", 1, 0, &fgaussn, -1, "FILE f(x) = sum( a[i+2]*exp(-((x-ai)/a[i+1])**2) ) + a[n-1]" },
    { "lorenz", 1, 0, &florenz, 3, "f(x) = a1*a2/(4*(x-a0)**2 + a1)" },
    { "2lorenz", 1, 0, &florenz2, 7, "f(x) = a1*a2/(4*(x-a0)**2 + a1) + a4*a5/(4*(x-a3)**2 + a4) + a6" },
    { "linear", 1, 1, &fpoly, 2, "f(x) = a0 + a1*x" },
    { "quad", 1, 0, &fpoly, 3, "f(x) = a0 + a1*x + a2*x*x" },
    { "cube", 1, 0, &fpoly, 4, "f(x) = a0 + a1*x + a2*x*x + a3*x*x*x" },
    { "poly", 1, 1, &fpoly, -1, "FILE f(x) = sum( ai * pow(x,i) )" },
    { "nexp", 1, 0, &fexpn, -1, "FILE f(x) = sum( a[i+2]*exp((x-a[i])/a[i+1]) ) + a[n-1]" },
    { "xyquad", 2, 1, &fxyquad, 6, "f(u,v) = a0 + a1*u + a2*v + a3*u*u + a4*v*v + a5*u*v" },
    { "xygauss", 2, 0, &fxygauss, 6, "f(u,v) = a4*exp(-((u-a0)/a2)**2 - ((v-a1)/a3)**2 ) + a5" },
    { "sincos", 1, 0, &fsincos, -1, "FILE a0 + a[i]*sin(a[i+1]*x) + a[i+2]*cos(a[i+3]*x)" },
};

/* The function getfcnptr() looks through the struct fcn to find a */
/* pointer corresponding to the function specified at the fit> */
/* prompt with the fn command */

ifptr
getfcnptr(char *name, int *num_indep, int *linflag, int *na, char *comment)
{
    int             i = 0;
    while (fcn[i].na != 0) {
        if (strcmp(name, fcn[i].name) == 0) {
            *na = fcn[i].na;
            *linflag = fcn[i].linflag;
            *num_indep = fcn[i].num_indep;
            strcpy(comment, fcn[i].comment);
            return fcn[i].func;
        }
        i++;
    }

    /* If function name not found return pointer to NULL */
    *na = 0;
    return ((ifptr) NULL);
}

/* listfcns() lists the functions available */
int 
listfcns(void)
{
    int             i = 0;
    char buf[256];
    while (fcn[i].na != 0) {
        printf("%s %s\n", fcn[i].name, fcn[i].comment);
        i++;
        if (i == 20) {
            i = 0;
            // gets(buf);
	    // The warning was driving me nuts. It's also an invite
	    // to segfaults and possible hacks
	    fgets(buf, 256, stdin);
        }
    }
    return 0;
}

/* the definition of a function must be of the form: */
/* int function(double *x, double *a, double *y, double *dyda, */
/* int na, int dyda_flag, int *fita, int dydx_flag, */
/* double *dydx, double ydat); */

/*
 * The x[i]'s are the independent variables passed to the function.
 *  *y should equal the value of the function at x.
 * The a[i]'s are the parameters.
 * The dyda[i]'s should be set equal to the first derivative
 * of the fitting function with respect to each parameter.
 *
 * dyda_flag, fita[], and dydx_flag[] are flags which are
 * passed to the function and used for optimization
 * purposes only.  These need not be used at all,
 * but may result in a significant performance boost.
 * if calculating the derivative is slow and it is not 
 * needed by the calling function,
 * dydx[i] should be set equal to the derivative of the fitting 
 * function with respect to x[i].  It is used only if 
 * the fitting algorithm is told to consider errors in the x[i] 
 * ydat is the data value that you are fitting to. 
 * It might be useful in multivalued functions 
 * Using it might destroy the statistical relevance of your fit. 
 */

/* The linear fitting routine makes different use of user defined */
/* fitting functions.  the dyda[i]'s and dydx[i]'s are not needed. */
/* Linear fitting functions which are set up properly for non-linear fitting */
/* work fine with the linear fitting routine.  Using dydx_flag, */
/* dyda_flag, and fita[i] will speed up linear fitting. */


/* f(u,v) = a4*exp(-((u-a0)/a2)**2 - ((v-a1)/a3)**2 ) + a5 */
/* a two dimensional gaussian */
int 
fxygauss(double *x, double *a, double *y, double *dyda, int na, int dyda_flag, int *fita, int *dydx_flag, double *dydx, double ydat)
{
    double          fac0, ex0, arg0, fac1, ex1, arg1;
    arg0 = (x[0] - a[0]) / a[2];
    ex0 = exp(-arg0 * arg0);
    fac0 = a[4] * ex0 * 2.0 * arg0;
    arg1 = (x[1] - a[1]) / a[3];
    ex1 = exp(-arg1 * arg1);
    fac1 = a[4] * ex1 * 2.0 * arg1;

    *y = a[4] * ex0 * ex1 + a[5];

    if (dyda_flag) {
        if (fita[0])
            dyda[0] = ex1 * fac0 / a[2];
        if (fita[1])
            dyda[1] = ex0 * fac1 / a[3];
        if (fita[2])
            dyda[2] = ex1 * fac0 * arg0 / a[2];
        if (fita[3])
            dyda[3] = ex0 * fac1 * arg1 / a[3];
        if (fita[4])
            dyda[4] = ex0 * ex1;
        if (fita[5])
            dyda[5] = 1;
    }
    if (dydx_flag[0])
        dydx[0] = -ex1 * fac0 / a[2];
    if (dydx_flag[1])
        dydx[1] = -ex0 * fac1 / a[3];
    return 0;
}


/* a sum of sines and cosines */
int 
fsincos(double *x, double *a, double *y, double *dyda, int na, int dyda_flag, int *fita, int *dydx_flag, double *dydx, double ydat)
{

    int             i;
    double          temp;

    *y = a[0];
    dydx[0] = 0;

    dyda[0] = 1;
    for (i = 1; i < na - 3; i += 4) {
        dyda[i] = sin(a[i + 1] * x[0]);
        *y += a[i] * dyda[i];
        if (dydx_flag[0] || fita[i + 1])
            temp = a[i] * cos(a[i + 1] * x[0]);
        dydx[0] += a[i + 1] * temp;
        dyda[i + 1] = x[0] * temp;
    }

    for (i = 3; i < na - 1; i += 4) {
        dyda[i] = cos(a[i + 1] * x[0]);
        *y += a[i] * dyda[i];
        if (dydx_flag[0] || fita[i + 1])
            temp = -a[i] * sin(a[i + 1] * x[0]);
        dydx[0] += a[i + 1] * temp;
        dyda[i + 1] = x[0] * temp;
    }

    return 0;
}



/* a quadradic in x and y */
int 
fxyquad(double *x, double *a, double *y, double *dyda, int na, int dyda_flag, int *fita, int *dydx_flag, double *dydx, double ydat)
{
    double          x0, x1, x02, x12;

    x0 = x[0];
    x1 = x[1];
    x02 = x0 * x0;
    x12 = x1 * x1;

    *y = a[0] + a[1] * x0 + a[2] * x1 + a[3] * x02 + a[4] * x12 + a[5] * x1 * x0;
    if (dyda_flag) {
        if (fita[0])
            dyda[0] = 1;
        if (fita[1])
            dyda[1] = x0;
        if (fita[2])
            dyda[2] = x1;
        if (fita[3])
            dyda[3] = x02;
        if (fita[4])
            dyda[4] = x12;
        if (fita[5])
            dyda[5] = x1 * x0;
    }
    if (dydx_flag[0])
        dydx[0] = a[1] + 2 * x0 * a[3] + x1 * a[5];
    if (dydx_flag[1])
        dydx[1] = a[2] + 2 * x1 * a[4] + x0 * a[5];
    return 0;
}

/* a single lorenzian */
int 
florenz(double *x, double *a, double *y, double *dyda, int na, int dyda_flag, int *fita, int *dydx_flag, double *dydx, double ydat)
{
    double          denom, del, del2;

    del = x[0] - a[0];
    del2 = del * del;
    denom = 4.0 * del2 + a[1];
    *y = a[2] * a[1] / denom;
    if (dyda_flag) {
        if (fita[0])
            dyda[0] = 8.0 * del * (*y) / denom;
        if (fita[1])
            dyda[1] = (*y) / a[1] - (*y) / denom;
        if (fita[2])
            dyda[2] = (*y) / a[2];
    }
    if (dydx_flag[0])
        dydx[0] = -8.0 * del * (*y) / denom;
    return 0;
}

/* sum of two lorenzians */
int 
florenz2(double *x, double *a, double *y, double *dyda, int na, int dyda_flag, int *fita, int *dydx_flag, double *dydx, double ydat)
{
    double          denom, del, del2, y1, y2;

    del = x[0] - a[0];
    del2 = del * del;
    denom = 4.0 * del2 + a[1];
    y1 = a[2] * a[1] / denom;
    if (dyda_flag) {
        if (fita[0])
            dyda[0] = 8.0 * del * y1 / denom;
        if (fita[1])
            dyda[1] = y1 / a[1] - y1 / denom;
        if (fita[1])
            dyda[2] = y1 / a[2];
    }
    if (dydx_flag[0])
        dydx[0] = -8.0 * del * y1 / denom;

    del = x[0] - a[3];
    del2 = del * del;
    denom = 4.0 * del2 + a[4];
    y2 = a[4] * a[5] / denom;
    if (dyda_flag) {
        if (fita[3])
            dyda[3] = 8.0 * del * y2 / denom;
        if (fita[4])
            dyda[4] = y2 / a[4] - y2 / denom;
        if (fita[5])
            dyda[5] = y2 / a[5];
        if (fita[6])
            dyda[6] = 1;
    }
    if (dydx_flag[0])
        dydx[0] += -8.0 * del * y2 / denom;
    *y = y1 + y2 + a[6];
    return 0;
}

/* a gaussian */
int 
fgauss(double *x, double *a, double *y, double *dyda, int na, int dyda_flag, int *fita, int *dydx_flag, double *dydx, double ydat)
{
    double          fac, ex, arg;
    arg = (x[0] - a[0]) / a[1];
    ex = exp(-arg * arg);
    fac = a[2] * ex * 2.0 * arg;
    *y = a[2] * ex;

    if (dyda_flag) {
        if (fita[0])
            dyda[0] = fac / a[1];
        if (fita[1])
            dyda[1] = fac * arg / a[1];
        if (fita[2])
            dyda[2] = ex;
    }
    if (dydx_flag[0])
        dydx[0] = -fac / a[1];
    return 0;
}

/* a gaussian plus a constant */
int 
fgaussc(double *x, double *a, double *y, double *dyda, int na, int dyda_flag, int *fita, int *dydx_flag, double *dydx, double ydat)
{
    double          fac, ex, arg;
    arg = (x[0] - a[0]) / a[1];
    ex = exp(-arg * arg);
    fac = a[2] * ex * 2.0 * arg;
    *y = a[2] * ex + a[3];
    if (dyda_flag) {
        if (fita[0])
            dyda[0] = fac / a[1];
        if (fita[1])
            dyda[1] = fac * arg / a[1];
        if (fita[2])
            dyda[2] = ex;
        if (fita[3])
            dyda[3] = 1;
    }
    if (dydx_flag[0])
        dydx[0] = -fac / a[1];
    return 0;
}

/* a gaussian plus a linear term */
int 
fgaussl(double *x, double *a, double *y, double *dyda, int na, int dyda_flag, int *fita, int *dydx_flag, double *dydx, double ydat)
{
    double          fac, ex, arg;
    arg = (x[0] - a[0]) / a[1];
    ex = exp(-arg * arg);
    fac = a[2] * ex * 2.0 * arg;
    *y = a[2] * ex + a[3] + a[4]*x[0];
    if (dyda_flag) {
        if (fita[0])
            dyda[0] = fac / a[1];
        if (fita[1])
            dyda[1] = fac * arg / a[1];
        if (fita[2])
            dyda[2] = ex;
        if (fita[3])
            dyda[3] = 1;
        if (fita[4])
            dyda[4] = x[0];
    }
    if (dydx_flag[0])
        dydx[0] = -fac / a[1];
    return 0;
}

/* sum of gaussians plus a constant */
int 
fgaussn(double *x, double *a, double *y, double *dyda, int na, int dyda_flag, int *fita, int *dydx_flag, double *dydx, double ydat)
{
    double          fac, ex, arg;
    int             i;
    dydx[0] = 0;
    *y = 0;
    for (i = 0; i < na - 1; i += 3) {
        arg = (x[0] - a[i]) / a[i + 1];
        ex = exp(-arg * arg);
        fac = a[i + 2] * ex * 2.0 * arg;
        *y += a[i + 2] * ex;
        if (dyda_flag) {
            if (fita[i])
                dyda[i] = fac / a[i + 1];
            if (fita[i + 1])
                dyda[i + 1] = fac * arg / a[i + 1];
            if (fita[i + 2])
                dyda[i + 2] = ex;
        }
        if (dydx_flag[0])
            dydx[0] += -fac / a[i + 1];
    }
    dyda[na - 1] = 1;
    *y += a[na - 1];
    return 0;
}


int 
fline(double *x, double *a, double *y, double *dyda, int na, int dyda_flag, int *fita, int *dydx_flag, double *dydx, double ydat)
{
    dyda[0] = 1;
    dyda[1] = x[0];
    *y = a[0] + a[1] * x[0];
    dydx[0] = a[1];
    return 0;
}


int 
fpoly(double *x, double *a, double *y, double *dyda, int na, int dyda_flag, int *fita, int *dydx_flag, double *dydx, double ydat)
{
    int             i;
    double          xn, ynum;
    xn = 1;
    ynum = 0;
    dydx[0] = 0;
    for (i = 0; i < na; i++) {
        ynum += xn * a[i];
        if (dydx_flag[0] && i > 0)
            dydx[0] += a[i - 1] * xn;
        if (fita[i])
            dyda[i] = xn;
        xn *= x[0];
    }
    *y = ynum;
    return 0;
}

/* sum of exponentials plus a constant */
int 
fexpn(double *x, double *a, double *y, double *dyda, int na, int dyda_flag, int *fita, int *dydx_flag, double *dydx, double ydat)
{
    double          fac, ex, arg;
    int             i;
    double          s;

    *y = 0;
    dydx[0] = 0;
    for (i = 0; i < na - 1; i += 3) {
        if (x[0] < a[i])
            s = 0;
        else
            s = 1;
        arg = (x[0] - a[i]) / a[i + 1];
        ex = exp(-arg);
        fac = a[i + 2] * ex;
        *y += s * fac;
        if (dyda_flag) {
            if (fita[i])
                dyda[i] = s * fac / a[i + 1];
            if (fita[i + 1])
                dyda[i + 1] = s * fac * arg / a[i + 1];
            if (fita[i + 2])
                dyda[i + 2] = s * ex;
        }
        if (dydx_flag[0])
            dydx[0] += -s * fac / a[i + 1];
    }
    dyda[na - 1] = 1;
    *y += a[na - 1];
    return 0;
}

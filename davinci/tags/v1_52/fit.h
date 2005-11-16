/* This structure tells us how the data is represented internally in
     the data matrix.  If order.x = 0, then the 0th column is the x's.
*/
/* 	int *x;        independent variables */
 /*	int y;         dependent variable */
 /*	int yfit;      value of function with current parameters */
 /*	int sig;       sigma: an error estimate for y, used in fitting */
 /*	int nsig;     no sigma, column of data full of 1's */
 /*	int ssig;     sigma for statistical weighting */
 /*	int isig;      sigma for instrumental weighting */
 /*	int osig;     sigma for other weighting */

struct data_order{
	int *x;
	int y;
	int yfit;
	int sig;
	int nsig;
	int ssig;
	int isig;
	int osig;
	int *xsig;
};

/*** a few function declarations ***/

static double **dmatrix(int,int);				/* allocates a 2-D array of doubles */
static double *dvector(int);           		/* allocates a 1-D array of doubles */
static int *ivector(int);              		/* allocates a 1-D array of ints */
void free_dmatrix(double **, int,int);  /* frees a 2-D array of doubles */
int listfcns(void);             		/* lists the functions available */

typedef int     (*ifptr)
    (double *, double *, double *, double *,int , int , int *, int *,
     double *, double );

/* pointer to the fitting function */
extern ifptr func;

/* returns pointer to the fitting function */
ifptr getfuncptr(char *function_name, int *num_indep,
			int linflag, int *num_parameters, char *comment);

/* reads the data from a file */
int get_data(double **data, struct data_order *order, char *filename,
				int maxrows, int maxcols);

/* does the nonlinear fit */
int mrqfit(double **data,struct data_order order,
				int num_indep, int ndata, int itmax, double *a,
				int ma, int mfit, double **covar,
				double *chisq, int (*func)(),int);

void solve_for_da(double **, double **, double *, double *, int);
int alpha_beta_chisq(double **data, struct data_order order,
				int num_indep, int ndata, double *a, int ma, 
				int mfit, double **alpha, double *beta,
				double *chisq, int (*funcs)());

void gaussj(double **, int n, double **, int m);

/* nonzero returns 1 if all elements of an array are non-zero */
int nonzero(double *, int ndata);

/*
	C A U T I O N !

	The following code uses both zero-indexed and one-indexed
	arrays.
*/

/*********************************************************************/
/* Principal Components Analysis or the Karhunen-Loeve expansion is a
   classical method for dimensionality reduction or exploratory data
   analysis.  One reference among many is: F. Murtagh and A. Heck,
   Multivariate Data Analysis, Kluwer Academic, Dordrecht, 1987 
   (hardbound, paperback and accompanying diskette).

   This program is public-domain.  If of importance, please reference 
   the author.  Please also send comments of any kind to the author:

   F. Murtagh
   Schlossgartenweg 1          or        35 St. Helen's Road
   D-8045 Ismaning                       Booterstown, Co. Dublin
   W. Germany                            Ireland

   Phone:        + 49 89 32006298 (work)
   + 49 89 965307 (home)
   Telex:        528 282 22 eo d
   Fax:          + 49 89 3202362
   Earn/Bitnet:  fionn@dgaeso51,  fim@dgaipp1s,  murtagh@stsci
   Span:         esomc1::fionn
   Internet:     murtagh@scivax.stsci.edu


   A Fortran version of this program is also available.     

   F. Murtagh, Munich, 6 June 1989                                   */
/*********************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "parser.h"

static const char PCS[]="pcs1";
static const char PCA[]="pca1";
#define I_AM_PCS(func)  (strcmp((func)->name, PCS) == 0)
#define I_AM_PCA(func)  (strcmp((func)->name, PCA) == 0)

#define SIGN(a, b) ( (b) < 0 ? -fabs(a) : fabs(a) )

float **dstretch(
	float	**data,	/* data[n][m] */
	int	n,
	int	m,
	float	*evals,	/* eigen-values[m] */
	float	**evecs,	/* eigen-vectors[m][m] */
	float	*dscale	/* scaling vector [m] */
);

float **eval_dstmat(
	float	**gain,	/* gain factors matrix [nxn] */
	float	**evec,	/* eigen vectors matrix [nxn]*/
	int	n,
	int	biggest_processing
);

void corcol();
void covcol();
void scpcol();
void erhand();
float *vector();
float **matrix();
void free_vector();
void free_matrix();
void scpcol();
void tred2();
void tqli();
void stddev(float **data, int n, int m, float *stddev);

static float **mxm(
	float **m1, int r1, int c1,
	float **m2, int r2, int c2,
	float	**result
);

static float **xpose(float **m, int r, int c, float **result);

/*
** Multiply matrix m1 with matrix m2 i.e. m1 x m2.
**
** "m1", "m2", and "result" are r1xc1, r2xc2, and r1xc2 matrices
** pre-allocated using matrix().
*/ 
static float **
mxm(
	float **m1, int r1, int c1,
	float **m2, int r2, int c2,
	float	**result
)
{
	int		i, j, k;
	double	d;

	if (c1 != r2){
		fprintf(stderr, "mxm: Invalid dimenstions for matrix multiply.\n");
		return NULL;
	}

	for (i=1; i<=r1; i++){
		for(j=1; j<=c2; j++){
			d = 0;
			for(k=1; k<=c1; k++){
				d += m1[i][k] * m2[k][j];
			}
			result[i][j] = d;
		}
	}

	return result;
}

/*
** Returns transpose of the matrix "m".
**
** "m" and "result" are rxc and cxr (sized) matrices pre-allocated
** using matrix()
*/
static float **
xpose(
	float **m, int r, int c,
	float **result
)
{
	int	i, j;

	for(j=1; j<=r; j++){
		for(i=1; i<=c; i++){
			result[i][j] = m[j][i];
		}
	}

	return result;
}

/*
** pca() - Principal Component Analysis
*/
float *
pca(
	float		**data,	/* n x m matrix of input data */
	int		n,			/* n-rows (or n-vectors, each of length m) */
	int		m,			/* m-columns (or m-variables) */
	char		opt,		/* processing option:
									'v' - use covariance matrix
									'r' - use correlation matrix
									's' - use SSCP matrix
							*/
	float		*evals,	/* Eigen Values 1 x m vector */
	float		**symmat	/* Eigen Vectors m x m matrix */
)
{
	float		*interm;

	/* Look at analysis option; branch in accordance with this. */
	switch (opt) {
	case 'R':/* Output correlation matrix - symmat[m][m]. */
	case 'r':
		corcol(data, n, m, symmat);
		break;

	case 'V':/* Output variance-covariance matrix - symmat[m][m]. */
	case 'v':
		covcol(data, n, m, symmat);
		break;

	case 'S':/* Output SSCP matrix - symmat[m][m]. */
	case 's':
		scpcol(data, n, m, symmat);
		break;

	default:
		fprintf(stderr, "pca(): Invalid processing option %c\n", opt);
		free_matrix(symmat, m, m);
		free_vector(evals, m);
		return NULL;
	}

	/* Allocate storage for dummy and new vectors. */
	interm = vector(m);	/* Storage alloc. for 'intermediate' vector */

	/* Calculate Eigen values and vectors */
	tred2(symmat, m, evals, interm);	/* Triangular decomposition */
	tqli(evals, interm, m, symmat);		/* Reduction of sym. trid. matrix */
	/* evals now contains the eigenvalues,
		columns of symmat now contain the associated eigenvectors. */

	free_vector(interm, m);

	return evals;
}

/*
**
*/
float **
eval_stretch_matrix(
	float		*evals,		/* m Eigen Values */
	float		**evecs,		/* m x m Eigen Vectors */
	int		m,
	float		*scale,		/* m Scaling Values or
									NULL (scaling = 1.0) */
	float		**dstmat		/* output m x m stretch matrix */
)
{
	float		**gainfac, **iresult;
	float		**evecst;
	float		x;
	int		i, j;

	/* allocate space for gain-factor matrix */
	gainfac = matrix(m, m);

	for(j=1; j<=m; j++){
		for(i=1; i<=m; i++){
			gainfac[j][i] = 0.0;
		}
	}

	/* evaluate the gain-factor (scaling) matrix */
	x = (m<2)? evals[1]: evals[2];

	for(i=1; i<=m; i++){
		gainfac[i][i] = sqrt(fabs(x/evals[i]))*((scale == NULL)? 1.0: scale[i]);
	}

	/*
	**     dstmat = evec x gain x evec'
	**
	** where evec' is the transpose of evec matrix
	** and gain is the scaling matrix
	**
	** use:
	**     data x dstmat
	**
	** to generate the stretched data outside this routine
	*/

	iresult = matrix(m, m);
	mxm(evecs, m, m, gainfac, m, m, iresult);
	free_matrix(gainfac, m, m);

	evecst = matrix(m, m);
	xpose(evecs, m, m, evecst);

	/* allocate space for stretch matrix */
	mxm(iresult, m, m, evecst, m, m, dstmat);

	free_matrix(iresult, m, m);
	free_matrix(evecst, m, m);

	return dstmat;
}


/*
**	data[][] is replaced with the stretched data
*/
float **
pcs(
	float		**data,
	int		n,
	int		m,
	char		opt,
	float		*scale		/* scaling vector (m scaling values) or
									NULL for scaling of 1.0 (i.e. no scaling) */
)
{
	float		*evals, **evecs, *v;
	float		**dstmat;
	int		i, j, k;

	/* Allocate storage for Eigen Vectors */
	evecs = matrix(m, m);

	/* Allocate storage for vector of eigenvalues */
	evals = vector(m);

	if (pca(data, n, m, opt, evals, evecs) == NULL){
		free_matrix(evecs, m, m);
		free_vector(evals, m);
		return NULL;
	}

	/* allocate space for m x m decorrelation stretch matrix */
	dstmat = matrix(m, m);

	eval_stretch_matrix(evals, evecs, m, scale, dstmat);

	/* free unused space */
	free_vector(evals, m);
	free_matrix(evecs, m, m);

	/*
	** apply stretch matrix to the input data:
	**
	**          data x dstmat
	**
	** I am not using mxm() here so that I can do the multiplication
	** in place
	*/

	/* allocate space for a row of data matrix */
	v = vector(m);

	for(j=1; j<=n; j++){

		/* evaluate a single row of data matrix */
		for(i=1; i<=m; i++){
			v[i] = 0;
			for(k=1; k<=m; k++){
				v[i] += data[j][k] * dstmat[k][i];
			}
		}

		/* replace the original row with the computed data */
		for(i=1; i<=m; i++){ data[j][i] = v[i]; }
	}

	free_vector(v, m);

	return data;
}

/***************************************************************************/
/** davinci function pcs(obj [, opt = v, axis = z, scale = 1])            **/
/***************************************************************************/
Var *
ff_pcs(
	vfuncptr	func,
	Var		*args
)
{
	int		ac;
	Var		**av;
	Var		*obj = NULL, *axis_arg = NULL;
	Var		*scale_arg = NULL, *opt_arg = NULL;
	Var		*v_return;
	char		*axis_enums[] = { /* values that axis can take */
		"x",  "X",
		"y",  "Y",
		"z",  "Z",
		NULL
	};
	char		*opt_enums[] = { /* values taken by "opt"; see pca() */
		"v",	"V", /* use covariance matrix -  default */
		"r",	"R", /* use correlation matrix */
		"s",	"S", /* use sum of squares matrix */
		NULL
	};
	char		opt = 'v'; /* default processing option (value of "opt" arg) */

	float		**data, *scale = NULL;
	float		*fdata = NULL;
	int		dim[3], indices[3];
	int		ref[3] = {0, 1, 2};
	int		n, m, offset;
	int		i, j, k, index;

	Alist		alist[5];
	alist[0] = make_alist( "obj",    ID_VAL,    NULL,        &obj);
	alist[1] = make_alist( "opt",    ID_ENUM,   opt_enums,   &opt_arg);
	alist[2] = make_alist( "axis",   ID_ENUM,   axis_enums,  &axis_arg);
	alist[3] = make_alist( "scale",  ID_VAL,    NULL,        &scale_arg);
	alist[4].name = NULL;

	make_args(&ac, &av, func, args);
	if (parse_args(ac, av, alist)) return(NULL);

	if (obj == NULL) {
		parse_error("%s(): Argument \"%s\" not specified\n",
			func->name, alist[0].name);
		return(NULL);
	}

	switch(V_FORMAT(obj)){
	case BYTE:
	case SHORT:
	case INT:
	case FLOAT:
	case DOUBLE:
		/* Only the above data types are supported */
		break;
	
	default:
		parse_error("%s(): \"%s\" is of invalid format.\n",
			func->name, alist[0].name);
		return(NULL);
	}

	/* Get dimensions of input data */
	dim[0] = GetSamples(V_SIZE(obj), V_ORG(obj));	/* x */
	dim[1] = GetLines(V_SIZE(obj), V_ORG(obj));		/* y */
	dim[2] = GetBands(V_SIZE(obj), V_ORG(obj));		/* z */

	/*
	** If axis is not specified, use the default, i.e. Z
	*/
	offset = (axis_arg != NULL)? toupper(*(char *)axis_arg)-'X': 'Z'-'X';

	/*
	** ref[0] contains the index of the "axis" passed in by the user
	** ref[1-2] contains the indices of the other two axes
	*/
	for(i = 0; i < 3; i++){ ref[i] = (i+offset)%3; }

	/*
	** number of columns (or variables) is along the "axis"
	** specified by the user
	*/
	m = dim[ref[0]];

	/*
	** number of rows is the rest of the two (out of three)
	** dimensions
	*/
	n = dim[ref[1]] * dim[ref[2]];

	/* 
	** If scale[] is specified, do some sanity checks on its dimensions
	** and retrieve the scaling data
	*/
	if (scale_arg != NULL){
		int sm, sn, sw;

		/* verify scaling data's data-type */

		switch(V_FORMAT(scale_arg)){
		case BYTE:
		case SHORT:
		case INT:
		case FLOAT:
		case DOUBLE:
			/* Only the above data types are supported */
			break;
		
		default:
			parse_error("%s(): \"%s\" is of invalid format.\n",
				func->name, alist[3].name);
			return(NULL);
		}

		sm = GetSamples(V_SIZE(scale_arg), V_ORG(scale_arg));
		sn = GetLines(V_SIZE(scale_arg), V_ORG(scale_arg));
		sw = GetBands(V_SIZE(scale_arg), V_ORG(scale_arg));

		if (sm != m || sn != 1 || sw != 1){
			parse_error("%s(): \"%s\" must be [%d, 1, 1].\n",
				func->name, alist[3].name, m);
			return NULL;
		}

		/* allocate space for scaling data */
		scale = vector(m);

		/* retrieve scaling data */
		for(i=0; i<m; i++){
			index = cpos(i, 0, 0, scale_arg);
			scale[i+1] = extract_float(scale_arg, index);
		}
	}

	/*
	** retrieve the processing option
	*/
	if (opt_arg != NULL){
		opt = *(char *)opt_arg;
	}

	/* allocate data space for n-rows, and m-columns */
	data = matrix(n, m);

	/* extract data */
	for(j=0; j<dim[ref[1]]; j++){
		for(i=0; i<dim[ref[2]]; i++){

			for(k=0; k<dim[ref[0]]; k++){ /* "axis" or "m" dimension */

				indices[ref[1]] = j;
				indices[ref[2]] = i;
				indices[ref[0]] = k;

				/*
				** here indices[0], indices[1], indices[2] are 
				** x, y, & z indices respectively
				*/
				index = cpos(indices[0], indices[1], indices[2], obj);

				/* data[][] has 1-based indices */
				data[j*dim[ref[2]]+i+1][k+1] = extract_float(obj, index);
			}
		}
	}

	/* perform the Principal Component Stretch */
	if (pcs(data, n, m, (strcmp(func->name, "pcsx")? tolower(opt): toupper(opt)), scale) == NULL){
		free_matrix(data, n, m);
		if(scale) { free_vector(scale, m); }
		return NULL;
	}

	if(scale){ free_vector(scale, m); }

	/* collect, package, and return results */
	fdata = (float *)calloc(dim[0]*dim[1]*dim[2], sizeof(float));

	/* store data */
	for(j=0; j<dim[ref[1]]; j++){
		for(i=0; i<dim[ref[2]]; i++){

			for(k=0; k<dim[ref[0]]; k++){ /* "axis" or "m" dimension */

				indices[ref[1]] = j;
				indices[ref[2]] = i;
				indices[ref[0]] = k;

				/*
				** here indices[0], indices[1], indices[2] are 
				** x, y, & z indices respectively
				*/
				index = cpos(indices[0], indices[1], indices[2], obj);

				/* data[][] has 1-based indices */
				fdata[index] = data[j*dim[ref[2]]+i+1][k+1];
			}
		}
	}

	/* cleanup temporary matrix */
	free_matrix(data, n, m);

	v_return = newVal(V_ORG(obj), dim[0], dim[1], dim[2], FLOAT, fdata);
	return v_return;
}


/***************************************************************************/
/** davinci function corr(obj [, axis = z])                               **/
/** davinci function covar(obj [, axis = z])                              **/
/** davinci function scp(obj [, axis = z])                                **/
/***************************************************************************/
Var *
ff_corr_covar_and_scp(
	vfuncptr	func,
	Var		*args
)
{
	int		ac;
	Var		**av;
	Var		*obj = NULL, *axis_arg = NULL;
	Var		*v_return;
	char		*axis_enums[] = { /* values that axis can take */
		"x",  "X",
		"y",  "Y",
		"z",  "Z",
		NULL
	};

	float		**data, **symmat;
	float		*fdata = NULL;
	int		dim[3], indices[3];
	int		ref[3] = {0, 1, 2};
	int		n, m, offset;
	int		i, j, k, index;

	Alist		alist[3];
	alist[0] = make_alist( "obj",    ID_VAL,    NULL,        &obj);
	alist[1] = make_alist( "axis",   ID_ENUM,   axis_enums,  &axis_arg);
	alist[2].name = NULL;

	make_args(&ac, &av, func, args);
	if (parse_args(ac, av, alist)) return(NULL);

	if (obj == NULL) {
		parse_error("%s(): Argument \"%s\" not specified\n",
			func->name, alist[0].name);
		return(NULL);
	}

	switch(V_FORMAT(obj)){
	case BYTE:
	case SHORT:
	case INT:
	case FLOAT:
	case DOUBLE:
		/* Only the above data types are supported */
		break;
	
	default:
		parse_error("%s(): \"%s\" is of invalid format.\n",
			func->name, alist[0].name);
		return(NULL);
	}

	/*
	** If axis is not specified, use the default, i.e. Z
	*/
	offset = (axis_arg != NULL)? toupper(*(char *)axis_arg)-'X': 'Z'-'X';

	/*
	** ref[0] contains the index of the "axis" passed in by the user
	** ref[1-2] contains the indices of the other two axes
	*/
	for(i = 0; i < 3; i++){ ref[i] = (i+offset)%3; }

	dim[0] = GetSamples(V_SIZE(obj), V_ORG(obj));	/* x */
	dim[1] = GetLines(V_SIZE(obj), V_ORG(obj));		/* y */
	dim[2] = GetBands(V_SIZE(obj), V_ORG(obj));		/* z */

	/*
	** number of columns (or variables) is along the "axis"
	** specified by the user
	*/
	m = dim[ref[0]];

	/*
	** number of rows is the rest of the two (out of three)
	** dimensions
	*/
	n = dim[ref[1]] * dim[ref[2]];

	/* allocate data space for n-rows, and m-columns */
	data = matrix(n, m);

	/* extract data */
	for(j=0; j<dim[ref[1]]; j++){
		for(i=0; i<dim[ref[2]]; i++){

			for(k=0; k<dim[ref[0]]; k++){ /* "axis" or "m" dimension */

				indices[ref[1]] = j;
				indices[ref[2]] = i;
				indices[ref[0]] = k;

				/*
				** here indices[0], indices[1], indices[2] are 
				** x, y, & z indices respectively
				*/
				index = cpos(indices[0], indices[1], indices[2], obj);

				/* data[][] has 1-based indices */
				data[j*dim[ref[2]]+i+1][k+1] = extract_float(obj, index);
			}
		}
	}

	/* allocate space for resultant matrix */
	symmat = matrix(m, m);

	/* apply processing function */

	if (strcmp(func->name, "covar") == 0){
		/* evaluate covariance */
		covcol(data, n, m, symmat);
	}
	else if (strcmp(func->name, "corr") == 0){
		/* evaluate correlation */
		corcol(data, n, m, symmat);
	}
	else if (strcmp(func->name, "scp") == 0){
		/* evaluate sum of squares and cross products matrix */
		scpcol(data, n, m, symmat);
	}
	else {
		/* should be fairly unreachable - an uncommon occurrance */
		parse_error("%s() <-- NOT IMPLEMENTED.\n", func->name);
		return NULL;
	}

	/* cleanup original data */
	free_matrix(data, n, m);

	/* collect, package, and return results */
	fdata = (float *)calloc(m*m, sizeof(float));

	for(j=0; j<m; j++){
		for(i=0; i<m; i++){
			fdata[j*m+i] = symmat[j+1][i+1];
		}
	}

	/* cleanup temporary matrix */
	free_matrix(symmat, m, m);

	v_return = newVal(BSQ, m, m, 1, FLOAT, fdata);
	return v_return;
}

void
stddev(
	float	**data,
	int	n,
	int	m,
	float	*stddev
)
{
	float eps = 0.005;
	float *mean, *vector();
	int	i, j;

/* Allocate storage for mean vector */
	mean = vector(m);

/* Determine mean of column vectors of input data matrix */

	for (j = 1; j <= m; j++) {
		mean[j] = 0.0;
		for (i = 1; i <= n; i++) {
			mean[j] += data[i][j];
		}
		mean[j] /= (float) n;
	}

/* Determine standard deviations of column vectors of data matrix. */

	for (j = 1; j <= m; j++) {
		stddev[j] = 0.0;
		for (i = 1; i <= n; i++) {
			stddev[j] += ((data[i][j] - mean[j]) *
				      (data[i][j] - mean[j]));
		}
		stddev[j] /= (float) n;
		stddev[j] = sqrt(stddev[j]);
		/* The following in an inelegant but usual way to handle
		   near-zero std. dev. values, which below would cause a zero-
		   divide. */
		if (stddev[j] <= eps)
			stddev[j] = 1.0;
	}

	free_vector(mean, m);
}


/**  Correlation matrix: creation  ***********************************/

/* Create m * m correlation matrix from given n * m data matrix. */

void 
corcol(data, n, m, symmat)
     float **data, **symmat;
     int n, m;
{
	float eps = 0.005, sqn;
	float x, *mean, *stddev, *vector();
	int i, j, j1, j2;

/* Allocate storage for mean and std. dev. vectors */
	mean = vector(m);
	stddev = vector(m);

/* Determine mean of column vectors of input data matrix */

	for (j = 1; j <= m; j++) {
		mean[j] = 0.0;
		for (i = 1; i <= n; i++) {
			mean[j] += data[i][j];
		}
		mean[j] /= (float) n;
	}

/* Determine standard deviations of column vectors of data matrix. */

	for (j = 1; j <= m; j++) {
		stddev[j] = 0.0;
		for (i = 1; i <= n; i++) {
			stddev[j] += ((data[i][j] - mean[j]) *
				      (data[i][j] - mean[j]));
		}
		stddev[j] /= (float) n;
		stddev[j] = sqrt(stddev[j]);
		/* The following in an inelegant but usual way to handle
		   near-zero std. dev. values, which below would cause a zero-
		   divide. */
		if (stddev[j] <= eps)
			stddev[j] = 1.0;
	}

#if 0
/* Center and reduce the column vectors. */
	for (i = 1; i <= n; i++) {
		for (j = 1; j <= m; j++) {
			data[i][j] -= mean[j];
			x = sqrt((float) n);
			x *= stddev[j];
			data[i][j] /= x;
		}
	}

/* Calculate the m * m correlation matrix. */
	for (j1 = 1; j1 <= m - 1; j1++) {
		symmat[j1][j1] = 1.0;
		for (j2 = j1 + 1; j2 <= m; j2++) {
			symmat[j1][j2] = 0.0;
			for (i = 1; i <= n; i++) {
				symmat[j1][j2] += (data[i][j1] * data[i][j2]);
			}
			symmat[j2][j1] = symmat[j1][j2];
		}
	}
	symmat[m][m] = 1.0;
#else
/* Saadat - Make this routine non-destructive for data[][] */

/* Calculate the m * m correlation matrix. */
	sqn = sqrt((float)n);

	for (j1 = 1; j1 <= m - 1; j1++) {
		symmat[j1][j1] = 1.0;
		for (j2 = j1 + 1; j2 <= m; j2++) {
			symmat[j1][j2] = 0.0;
			for (i = 1; i <= n; i++) {
				symmat[j1][j2] += 
					((data[i][j1] - mean[j1]) / (stddev[j1] * sqn)) *
					((data[i][j2] - mean[j2]) / (stddev[j2] * sqn));
			}
			symmat[j2][j1] = symmat[j1][j2];
		}
	}
	symmat[m][m] = 1.0;

#endif

	return;

}

/**  Variance-covariance matrix: creation  *****************************/

void 
covcol(data, n, m, symmat)
     float **data, **symmat;
     int n, m;
/* Create m * m covariance matrix from given n * m data matrix. */
{
	float *mean, *vector();
	int i, j, j1, j2;

/* Allocate storage for mean vector */

	mean = vector(m);

/* Determine mean of column vectors of input data matrix */

	for (j = 1; j <= m; j++) {
		mean[j] = 0.0;
		for (i = 1; i <= n; i++) {
			mean[j] += data[i][j];
		}
		mean[j] /= (float) n;
	}

#if 0
/* Center the column vectors. */
	for (i = 1; i <= n; i++) {
		for (j = 1; j <= m; j++) {
			data[i][j] -= mean[j];
		}
	}

/* Calculate the m * m covariance matrix. */
	for (j1 = 1; j1 <= m; j1++) {
		for (j2 = j1; j2 <= m; j2++) {
			symmat[j1][j2] = 0.0;
			for (i = 1; i <= n; i++) {
				symmat[j1][j2] += data[i][j1] * data[i][j2];
			}
			symmat[j2][j1] = (symmat[j1][j2] /= (n-1));
		}
	}
#else
/* Saadat - Make this routine non-destructive for data[][] */

/* Calculate the m * m covariance matrix. */
	for (j1 = 1; j1 <= m; j1++) {
		for (j2 = j1; j2 <= m; j2++) {
			symmat[j1][j2] = 0.0;
			for (i = 1; i <= n; i++) {
				symmat[j1][j2] += 
					(data[i][j1] - mean[j1]) *
					(data[i][j2] - mean[j2]);
			}
			symmat[j2][j1] = (symmat[j1][j2] /= ((float)(n-1)));
		}
	}
#endif

	return;

}

/**  Sums-of-squares-and-cross-products matrix: creation  **************/

void 
scpcol(data, n, m, symmat)
     float **data, **symmat;
     int n, m;
/* Create m * m sums-of-cross-products matrix from n * m data matrix. */
{
	int i, j1, j2;

/* Calculate the m * m sums-of-squares-and-cross-products matrix. */

	for (j1 = 1; j1 <= m; j1++) {
		for (j2 = j1; j2 <= m; j2++) {
			symmat[j1][j2] = 0.0;
			for (i = 1; i <= n; i++) {
				symmat[j1][j2] += data[i][j1] * data[i][j2];
			}
			symmat[j2][j1] = symmat[j1][j2];
		}
	}

	return;

}

/**  Error handler  **************************************************/

void 
erhand(err_msg)
     char err_msg[];
/* Error handler */
{
	fprintf(stderr, "Run-time error:\n");
	fprintf(stderr, "%s\n", err_msg);
	fprintf(stderr, "Exiting to system.\n");
	exit(1);
}

/**  Allocation of vector storage  ***********************************/

float *
vector(n)
     int n;
/* Allocates a float vector with range [1..n]. */
{

	float *v;

	v = (float *) malloc((unsigned) n * sizeof(float));
	if (!v)
		erhand("Allocation failure in vector().");
	return v - 1;

}

/**  Allocation of float matrix storage  *****************************/

float **
matrix(n, m)
     int n, m;
/* Allocate a float matrix with range [1..n][1..m]. */
{
	int i;
	float **mat;

	/* Allocate pointers to rows. */
	mat = (float **) malloc((unsigned) (n) * sizeof(float *));
	if (!mat)
		erhand("Allocation failure 1 in matrix().");
	mat -= 1;

	/* Allocate rows and set pointers to them. */
	for (i = 1; i <= n; i++) {
		mat[i] = (float *) malloc((unsigned) (m) * sizeof(float));
		if (!mat[i])
			erhand("Allocation failure 2 in matrix().");
		mat[i] -= 1;
	}

	/* Return pointer to array of pointers to rows. */
	return mat;

}

/**  Deallocate vector storage  *********************************/

void 
free_vector(v, n)
     float *v;
     int n;
/* Free a float vector allocated by vector(). */
{
	free((char *) (v + 1));
}

/**  Deallocate float matrix storage  ***************************/

void 
free_matrix(mat, n, m)
     float **mat;
     int n, m;
/* Free a float matrix allocated by matrix(). */
{
	int i;

	for (i = n; i >= 1; i--) {
		free((char *) (mat[i] + 1));
	}
	free((char *) (mat + 1));
}

/**  Reduce a real, symmetric matrix to a symmetric, tridiag. matrix. */

void 
tred2(a, n, d, e)
     float **a, *d, *e;
/* float **a, d[], e[]; */
     int n;
/* Householder reduction of matrix a to tridiagonal form.
   Algorithm: Martin et al., Num. Math. 11, 181-195, 1968.
   Ref: Smith et al., Matrix Eigensystem Routines -- EISPACK Guide
   Springer-Verlag, 1976, pp. 489-494.
   W H Press et al., Numerical Recipes in C, Cambridge U P,
   1988, pp. 373-374.  */
{
	int l, k, j, i;
	float scale, hh, h, g, f;

	for (i = n; i >= 2; i--) {
		l = i - 1;
		h = scale = 0.0;
		if (l > 1) {
			for (k = 1; k <= l; k++)
				scale += fabs(a[i][k]);
			if (scale == 0.0)
				e[i] = a[i][l];
			else {
				for (k = 1; k <= l; k++) {
					a[i][k] /= scale;
					h += a[i][k] * a[i][k];
				}
				f = a[i][l];
				g = f > 0 ? -sqrt(h) : sqrt(h);
				e[i] = scale * g;
				h -= f * g;
				a[i][l] = f - g;
				f = 0.0;
				for (j = 1; j <= l; j++) {
					a[j][i] = a[i][j] / h;
					g = 0.0;
					for (k = 1; k <= j; k++)
						g += a[j][k] * a[i][k];
					for (k = j + 1; k <= l; k++)
						g += a[k][j] * a[i][k];
					e[j] = g / h;
					f += e[j] * a[i][j];
				}
				hh = f / (h + h);
				for (j = 1; j <= l; j++) {
					f = a[i][j];
					e[j] = g = e[j] - hh * f;
					for (k = 1; k <= j; k++)
						a[j][k] -= (f * e[k] + g * a[i][k]);
				}
			}
		} else
			e[i] = a[i][l];
		d[i] = h;
	}
	d[1] = 0.0;
	e[1] = 0.0;
	for (i = 1; i <= n; i++) {
		l = i - 1;
		if (d[i]) {
			for (j = 1; j <= l; j++) {
				g = 0.0;
				for (k = 1; k <= l; k++)
					g += a[i][k] * a[k][j];
				for (k = 1; k <= l; k++)
					a[k][j] -= g * a[k][i];
			}
		}
		d[i] = a[i][i];
		a[i][i] = 1.0;
		for (j = 1; j <= l; j++)
			a[j][i] = a[i][j] = 0.0;
	}
}

/**  Tridiagonal QL algorithm -- Implicit  **********************/

void 
tqli(d, e, n, z)
     float d[], e[], **z;
     int n;
{
	int m, l, iter, i, k;
	float s, r, p, g, f, dd, c, b;
	void erhand();

	for (i = 2; i <= n; i++)
		e[i - 1] = e[i];
	e[n] = 0.0;
	for (l = 1; l <= n; l++) {
		iter = 0;
		do {
			for (m = l; m <= n - 1; m++) {
				dd = fabs(d[m]) + fabs(d[m + 1]);
				if (fabs(e[m]) + dd == dd)
					break;
			}
			if (m != l) {
				if (iter++ == 30)
					erhand("No convergence in TLQI.");
				g = (d[l + 1] - d[l]) / (2.0 * e[l]);
				r = sqrt((g * g) + 1.0);
				g = d[m] - d[l] + e[l] / (g + SIGN(r, g));
				s = c = 1.0;
				p = 0.0;
				for (i = m - 1; i >= l; i--) {
					f = s * e[i];
					b = c * e[i];
					if (fabs(f) >= fabs(g)) {
						c = g / f;
						r = sqrt((c * c) + 1.0);
						e[i + 1] = f * r;
						c *= (s = 1.0 / r);
					} else {
						s = f / g;
						r = sqrt((s * s) + 1.0);
						e[i + 1] = g * r;
						s *= (c = 1.0 / r);
					}
					g = d[i + 1] - p;
					r = (d[i] - g) * s + 2.0 * c * b;
					p = s * r;
					d[i + 1] = g + p;
					g = c * r - b;
					for (k = 1; k <= n; k++) {
						f = z[k][i + 1];
						z[k][i + 1] = s * z[k][i] + c * f;
						z[k][i] = c * z[k][i] - s * f;
					}
				}
				d[l] = d[l] - p;
				e[l] = g;
				e[m] = 0.0;
			}
		} while (m != l);
	}
}

/***************************************************************************/
/** davinci function eigen(obj)                                           **/
/** calculates Eigen Values & Vectors of Real Symmetric Matrices          **/
/***************************************************************************/
Var *
ff_eigen(
	vfuncptr	func,
	Var		*args
)
{
	int		ac;
	Var		**av;
	Var		*obj = NULL;
	Var		*v_return;
	float		*interm, *evals, **symmat;
	float		*fdata = NULL;
	int		n, m, w;
	int		i, j, index;

	Alist		alist[2];
	alist[0] = make_alist( "obj",    ID_VAL,    NULL,        &obj);
	alist[1].name = NULL;

	make_args(&ac, &av, func, args);
	if (parse_args(ac, av, alist)) return(NULL);

	if (obj == NULL) {
		parse_error("%s(): Argument \"%s\" not specified\n",
			func->name, alist[0].name);
		return(NULL);
	}

	switch(V_FORMAT(obj)){
	case BYTE:
	case SHORT:
	case INT:
	case FLOAT:
	case DOUBLE:
		/* Only the above data types are supported */
		break;
	
	default:
		parse_error("%s(): \"%s\" is of invalid format.\n",
			func->name, alist[0].name);
		return(NULL);
	}

	m = GetSamples(V_SIZE(obj), V_ORG(obj));	/* x */
	n = GetLines(V_SIZE(obj), V_ORG(obj));		/* y */
	w = GetBands(V_SIZE(obj), V_ORG(obj));		/* z */

	if (m != n && w != 1){
		parse_error("%s(): Argument \"%s\" must be a real symmetric matrix.\n",
			func->name, alist[0].name);
		return NULL;
	}

	/* allocate data space for n-rows, and m-columns */
	symmat = matrix(m, m);

	/* extract data */
	for(j=0; j<n; j++){
		for(i=0; i<m; i++){
			index = cpos(i, j, 0, obj);
			symmat[j+1][i+1] = extract_float(obj, index);
		}
	}

	/* Allocate space for Eigen Values */
	evals = vector(m);

	/* Allocate storage for dummy and new vectors. */
	interm = vector(m);	/* Storage alloc. for 'intermediate' vector */

	/* Calculate Eigen values and vectors */
	tred2(symmat, m, evals, interm);	/* Triangular decomposition */
	tqli(evals, interm, m, symmat);		/* Reduction of sym. trid. matrix */
	/* evals now contains the eigenvalues,
		columns of symmat now contain the associated eigenvectors. */

	/* cleanup temporary variables */
	free_vector(interm, m);

	/*
	** collect, package, and return results
	*/
	fdata = (float *)calloc(m*(m+1), sizeof(float));

	/* store eigen vectors starting from the second column of output data */
	for(j=0; j<m; j++){
		for(i=0; i<m; i++){
			fdata[j*(m+1)+(i+1)] = symmat[j+1][i+1];
		}
	}

	/* save eigen values in the first column of output data */
	for(j=0; j<m; j++){
		fdata[j*(m+1)] = evals[j+1];
	}

	/* cleanup temporary matrix */
	free_matrix(symmat, m, m);
	free_vector(evals, m);

	v_return = newVal(BSQ, m+1, m, 1, FLOAT, fdata);
	return v_return;
}

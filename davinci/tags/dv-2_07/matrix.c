#include "parser.h"

#define TINY 1.0e-20

void matrix_LUinvert(int n, double *A, double *B);

Var *
ff_identity(vfuncptr func, Var * arg)
{
	int size = 0, i;
	float *data;

	Alist alist[2];
	alist[0] = make_alist( "size",    INT,    NULL,     &size);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (size <= 0) {
		parse_error("%s: No size specified\n", func->name);
		return(NULL);
	}

	data = calloc(size * size, sizeof(float));
	for (i = 0 ; i < size ; i++) {
		data[i + i*size] = 1.0;
	}
	return(newVal(BSQ, size, size, 1, FLOAT, data));
}
Var *
ff_minvert(vfuncptr func, Var * arg)
{
	Var *obj = NULL;
	int x,y,z, i, dsize;
	double *a, *b;

	Alist alist[2];
	alist[0] = make_alist( "object",    ID_VAL,    NULL,     &obj);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (obj == NULL) {
		parse_error("%s: No object specified\n", func->name);
		return(NULL);
	}

	x = GetSamples(V_SIZE(obj), V_ORG(obj));
	y = GetLines(V_SIZE(obj), V_ORG(obj));
	z = GetBands(V_SIZE(obj), V_ORG(obj));
	dsize = V_DSIZE(obj);

	if (x != y && z != 1)  {
		parse_error("%s: not a square matrix.\n", func->name);
		return(NULL);
	}

	if (V_TYPE(obj) == DOUBLE) {
		a = (double *)V_DATA(obj);
	} else {
		a = (double *)calloc(dsize, sizeof(double));
		for (i = 0 ; i < dsize ; i++) {
			a[i] = extract_double(obj, i);
		}
	}
	b = (double *)calloc(dsize, sizeof(double));

	matrix_LUinvert(x, a, b);

	if (V_TYPE(obj) != DOUBLE) {
		free(a);
	}

	return(newVal(V_ORG(obj), 
		V_SIZE(obj)[0], 
		V_SIZE(obj)[1], 
		V_SIZE(obj)[2], 
		DOUBLE, b));

}



static double *dmatrix(int n, int m) 
{ 
	return((double *)calloc(n * m, sizeof(double)));
}
double *dvector(int n)
{
	return(dmatrix(n, 1));
}

int *ivector(int n)
{
	return((int *)calloc(n, sizeof(int)));
}

void mfree(double * a, int z)
{
	free(a);
}

void ludcmp(int n, double *a, int *indx, double *d);
void lubksb(int n, double *a, int *indx, double *b);
void matrix_copy(int rows, int cols, double *A, double *B);

/* invert a matrix using LU decomposition */

void matrix_LUinvert(int n, double *A, double *B)
{
    int ii, jj;
    double *col;
    int *indx;
    double d;
    double *tmpA;

    tmpA = dmatrix(n,n);

    col=dvector(n);
    indx=ivector(n);

    matrix_copy(n,n,A,tmpA);
    ludcmp(n, tmpA, indx, &d);

    for(jj=0; jj<n; jj++) {
        for(ii=0; ii<n; ii++)
            col[ii] = 0.0;
        col[jj] = 1.0;
        lubksb(n, tmpA, indx, col);
        for(ii=0; ii<n; ii++)
            B[ii*n+jj] = col[ii];
    }
    mfree(tmpA,n);
    free(col);
    free(indx);
}

/* decompose a matrix using LU decomposition */
/* from Num. Rec. p 43 */

void ludcmp(int n, double *a, int *indx, double *d)
{
    int i, imax, j, k;
    double big, dum, sum, temp;
    double *vv;

    if(!(vv = dvector(n))) 
        return;

    *d = 1.0;
    for (i=0; i<n; i++) {
        big = 0.0;
        for (j=0; j<n; j++) {
            if ((temp = fabs(a[i*n+j])) > big)
                big = temp;
        }
        if (big == 0.0) {
            fprintf(stderr, "Singular matrix\n");
            return;
        }
        vv[i] = 1.0 / big;
    }

    for (j=0; j<n; j++) {
        for (i=0; i<j; i++) {
            sum = a[i*n+j];
            for (k=0; k<i; k++) sum -= a[i*n+k]*a[k*n+j];
            a[i*n+j] = sum;
        }
        big = 0.0;
        for (i=j; i<n; i++) {
            sum = a[i*n+j];
            for (k=0; k<j; k++) {
                sum -= a[i*n+k]*a[k*n+j];
			}
            a[i*n+j] = sum;
            if ( (dum=vv[i]*fabs(sum)) >= big) {
                big = dum;
                imax = i;
            }
        }
        if (j != imax) {
            for (k=0; k<n; k++) {
                dum = a[imax*n+k];
                a[imax*n+k] = a[j*n+k];
                a[j*n+k] = dum;
            }
            *d = -(*d);
            vv[imax] = vv[j];
        }
        indx[j] = imax;
        if (a[j*n+j] == 0)
	    a[j*n+j] = TINY;
        if (j!=(n-1)) {
            dum = 1.0/a[j*n+j];
            for (i=j+1;i<n;i++) a[i*n+j] *= dum;
        }
    }
    free(vv);
}


/* forward and backward substitution from Num Rec. p 44 */
void lubksb(int n, double *a, int *indx, double *b)
{
    int ii, ip, jj;
    int iii=0, kkk=0;
    double sum;

    for(ii=0; ii<n; ii++) {
        ip = indx[ii];
        sum = b[ip];
        b[ip] = b[ii];
        if(kkk)
            for(jj=iii; jj<=ii-1; jj++)
                sum -= a[ii*n+jj]*b[jj];
        else if (sum!=0.0) {
            iii = ii;
            kkk = iii+1;
        }
        b[ii] = sum;
    }
    for(ii=n-1; ii>=0; ii--) {
        sum = b[ii];
        for(jj=ii+1; jj<n; jj++)
            sum -= a[ii*n+jj]*b[jj];
        b[ii] = sum/a[ii*n+ii];
    }
}

/* copy a matrix into another */
void matrix_copy(int rows, int cols, double *A, double *B)
{
    int ii, jj;
 
    for(ii=0; ii<rows; ii++)
        for(jj=0; jj<cols; jj++)
            B[ii*cols+jj] = A[ii*cols+jj];
}

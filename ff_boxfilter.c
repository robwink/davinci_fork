#include "parser.h"

void init_sums(Var *data, int w, int h, Var **rcount, Var **rmean, double ignore);

Var *
ff_boxfilter(vfuncptr func, Var * arg)
{
	Var *v= NULL;
	Var *rcount, *rmean;
	Var *a;
	int w=0;
	int h=0;
	double ignore=MINFLOAT;

    Alist alist[5];
    alist[0] = make_alist("obj",    ID_VAL,     NULL,     &v);
    alist[1] = make_alist("width",  INT,     NULL,     &w);
    alist[2] = make_alist("height", INT,     NULL,     &h);
    alist[3] = make_alist("ignore", DOUBLE,     NULL,     &ignore);
    alist[4].name = NULL;
	
	if (parse_args(func, arg, alist) == 0) return(NULL);
	if (v == NULL) {
		parse_error("%s(): No object specified\n", func->name);
		return(NULL);
	}
	init_sums(v, w, h, &rcount, &rmean, ignore);

	a = new_struct(0);
	add_struct(a, "rcount", rcount);
	add_struct(a, "rmean", rmean);
	return(a);
}

/*
** compute_windowed_mean() - computes the mean and count of the pixels
** within a wxh window, using a running-sum table.  
** Returns the mean and count for each pixel. 
*/
void
init_sums(Var *data, int w, int h, Var **rcount, Var **rmean, double ignore) 
{
	int x, y, z, u, v;
	int i, j, k;
	int p1, p2, p3, p4;
	int east, south;

	double *s;
	int *n;
	float *mean;
	double value;
	float sum;
	int count;

	x = GetX(data);
	y = GetY(data);
	z = GetZ(data);

	s = calloc(x*y*z,sizeof(double));		/* running sum */
	n = calloc(x*y*z,sizeof(int));			/* running count */
	mean = calloc(x*y*z,sizeof(float));

	/*
	** compute the running sum and count of V
	**
	** s(i,j) = f(i,j) +s(i-1,j)+s(i,j-1)-s(i-1,j-1)
	*/

	for (k = 0 ; k < z ; k++) {
		for (j = 0 ; j < y ; j++) {
			for (i = 0 ; i < x ; i++) {
				p1 = cpos(i,   j,   k, data);

				value = extract_double(data, p1);
				if (value != ignore) {
					s[p1] = value;
					n[p1] = 1;
				}
				if (i) {
					p2 = cpos(i-1, j,   k, data);
					s[p1] += s[p2];
					n[p1] += n[p2];
				}
				if (j) {
					p3 = cpos(i,   j-1, k, data);
					s[p1] += s[p3];
					n[p1] += n[p3];
				}
				if (i && j) {
					p4 = cpos(i-1, j-1, k, data);
					s[p1] -= s[p4];
					n[p1] -= n[p4];
				}
			}
		}
	}

    /* Formula to compute windowed sum from running sum is:
    **    s(u,v) = s(u+M-1,v+N-1) - s(u-1,v+N-1) - s(u+M-1,v-1) + s(u-1,v-1)
    ** This is:      sum(pt) = s(se) - s(sw) - s(ne) + s(nw).  
	** Given a 4x3 mask, it's this:
    ** 
    **              nw -- -- -- ne
    **              -- pt -- -- --
    **              -- -- -- -- --
    **              sw -- -- -- se
	**/

	for (k = 0 ; k < z ; k++) {
		for (j = 0 ; j < y ; j++) {
			for (i = 0 ; i < x ; i++) {
				east = min(i+w-1, x-1);
				south = min(j+h-1, y-1);

				p1 = cpos(east, south, k, data);
				count = n[p1];
				sum   = s[p1];

				if (i) {
					p2 = cpos(i-1, south, k, data);
					count -= n[p2];
					sum   -= s[p2];
				}
				if (j) {
					p3 = cpos(east, j-1, k, data);
					count -= n[p3];
					sum   -= s[p3];
				}
				if (i && j) {
					p4 = cpos(i-1,j-1,k,data);
					count += n[p4];
					sum   += s[p4];
				}

/*
				sum = s[cpos(min(i+w-1, x-1), min(j+h-1, y-1), k, data)];
				sum -= (i ? s[cpos(i-1, min(j+h-1, y-1), k, data)] : 0.0);
				sum -= (j ? s[cpos(min(i+w-1, x-1), j-1, k, data)] : 0.0);
				sum += (i == 0 || j == 0 ? 0.0 : s[cpos(i-1,j-1,k,data)]);
*/
				if (count) {
					mean[cpos(i,j,k,data)] = sum/count;
				} else {
					mean[cpos(i,j,k,data)] = ignore;
				}
			}
		}
	}

	*rcount = newVal(V_ORG(data), 
					V_SIZE(data)[0], V_SIZE(data)[1], V_SIZE(data)[2], 
					INT, n);
	*rmean = newVal(V_ORG(data), 
					V_SIZE(data)[0], V_SIZE(data)[1], V_SIZE(data)[2], 
					FLOAT, mean);
	free(s);
}

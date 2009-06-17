#include "parser.h"


/*
** compute_windowed_mean() - computes the mean and count of the pixels
** within a wxh window, using a running-sum table.  
** Returns the mean and count for each pixel. 
*/
void
init_sums(Var *data, int w, int h, int d, Var **rn, Var **rs, Var **rcount, Var **rmean, Var **rsigma, double ignore) 
{
	int x, y, z, u, v;
	int i, j, k;
	size_t p1, p2, p3, p4, p5, p6, p7, p8;
    size_t nelements;
	int east, south, north, west, front, back;

	double *s, *s2;
	int *n;
	float *mean;
	float *sigma;
	int *c;

	double value;
	double sum, sum2;
	int count;

	x = GetX(data);
	y = GetY(data);
	z = GetZ(data);
    nelements = V_DSIZE(data);

	s = calloc(nelements,sizeof(double));		/* running sum */
	s2 = calloc(nelements,sizeof(double));		/* running sum */
	n = calloc(nelements,sizeof(int));			/* running count */
	mean = calloc(nelements,sizeof(float));
	sigma = calloc(nelements,sizeof(float));
	c = calloc(nelements,sizeof(int));

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
					s2[p1] = value*value;
					n[p1] = 1;
				}
				if (i) {
					p2 = cpos(i-1, j,   k, data);
					s[p1] += s[p2];
					s2[p1] += s2[p2];
					n[p1] += n[p2];
				}
				if (j) {
					p3 = cpos(i,   j-1, k, data);
					s[p1] += s[p3];
					s2[p1] += s2[p3];
					n[p1] += n[p3];
				}
				if (i && j) {
					p4 = cpos(i-1, j-1, k, data);
					s[p1] -= s[p4];
					s2[p1] -= s2[p4];
					n[p1] -= n[p4];
				}
				if (k) {
					p5 = cpos(i,j,k-1, data);
					s[p1] += s[p5];
					s2[p1] += s2[p5];
					n[p1] += n[p5];
					if (i) {
						p6 = cpos(i-1, j,   k-1, data);
						s[p1] -= s[p6];
						s2[p1] -= s2[p6];
						n[p1] -= n[p6];
					}
					if (j) {
						p7 = cpos(i,   j-1, k-1, data);
						s[p1] -= s[p7];
						s2[p1] -= s2[p7];
						n[p1] -= n[p7];
					}
					if (i && j) {
						p8 = cpos(i-1, j-1, k-1, data);
						s[p1] += s[p8];
						s2[p1] += s2[p8];
						n[p1] += n[p8];
					}
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
				east = min(i+(w/2), x-1);
				south = min(j+(h/2), y-1);
				front = min(k+(d/2), z-1);

				west = i-(w/2)-1;
				north = j-(h/2)-1;
				back = k-(d/2)-1;

				p1 = cpos(east, south, front, data);
				count = n[p1];
				sum   = s[p1];
				sum2  = s2[p1];

				if (west >= 0) {
					p2 = cpos(west, south, front, data);
					count -= n[p2];
					sum   -= s[p2];
					sum2   -= s2[p2];
				}

				if (north >= 0) {
					p3 = cpos(east, north, front, data);
					count -= n[p3];
					sum   -= s[p3];
					sum2   -= s2[p3];
				}

				if (north >= 0 && west >= 0) {
					p4 = cpos(west,north,front,data);
					count += n[p4];
					sum   += s[p4];
					sum2   += s2[p4];
				}

				if (back >= 0) {
					p5 = cpos(east, south, back, data);
					count -= n[p5];
					sum   -= s[p5];
					sum2   -= s2[p5];

					if (west >= 0) {
						p6 = cpos(west, south, back, data);
						count += n[p6];
						sum   += s[p6];
						sum2   += s2[p6];
					}

					if (north >= 0) {
						p7 = cpos(east, north, back, data);
						count += n[p7];
						sum   += s[p7];
						sum2   += s2[p7];
					}

					if (north >= 0 && west >= 0) {
						p8 = cpos(west,north,back,data);
						count -= n[p8];
						sum   -= s[p8];
						sum2   -= s2[p8];
					}
				}

/*
				sum = s[cpos(min(i+w-1, x-1), min(j+h-1, y-1), k, data)];
				sum -= (i ? s[cpos(i-1, min(j+h-1, y-1), k, data)] : 0.0);
				sum -= (j ? s[cpos(min(i+w-1, x-1), j-1, k, data)] : 0.0);
				sum += (i == 0 || j == 0 ? 0.0 : s[cpos(i-1,j-1,k,data)]);
*/
				if (count) {
					p1 = cpos(i,j,k, data);
					mean[p1] = sum/count;
					if (count > 1) {
						sigma[p1] = sqrt((sum2-(sum*sum/count))/(count-1));
					} else {
						sigma[p1] = ignore;
					}
					c[p1] = count;
				} else {
					mean[cpos(i,j,k,data)] = ignore;
					sigma[cpos(i,j,k,data)] = ignore;
					c[p1] = ignore;
				}
			}
		}
	}

	free(s2);

	*rn = newVal(V_ORG(data), 
					V_SIZE(data)[0], V_SIZE(data)[1], V_SIZE(data)[2], 
					INT, n);
	*rs = newVal(V_ORG(data), 
					V_SIZE(data)[0], V_SIZE(data)[1], V_SIZE(data)[2], 
					DOUBLE, s);
	*rcount = newVal(V_ORG(data), 
					V_SIZE(data)[0], V_SIZE(data)[1], V_SIZE(data)[2], 
					INT, c);
	*rmean = newVal(V_ORG(data), 
					V_SIZE(data)[0], V_SIZE(data)[1], V_SIZE(data)[2], 
					FLOAT, mean);
	*rsigma = newVal(V_ORG(data), 
					V_SIZE(data)[0], V_SIZE(data)[1], V_SIZE(data)[2], 
					FLOAT, sigma);
}

Var *
ff_boxfilter(vfuncptr func, Var * arg)
{
	Var *v= NULL;
	Var *rcount, *rmean, *rs, *rn, *rsigma;
	Var *a;
	int x=0;
	int y=0;
	int z=0;
	int size=0;
	double ignore=MINFLOAT;
	int verbose=0;

    Alist alist[8];
    alist[0] = make_alist("obj",    ID_VAL,     NULL,     &v);
    alist[1] = make_alist("x",      INT,     NULL,     &x);
    alist[2] = make_alist("y",      INT,     NULL,     &y);
    alist[3] = make_alist("z",      INT,     NULL,     &z);
    alist[4] = make_alist("size",   INT,     NULL,     &size);
    alist[5] = make_alist("ignore", DOUBLE,  NULL,     &ignore);
    alist[6] = make_alist("verbose", INT,    NULL,     &verbose);
    alist[7].name = NULL;
	
	if (parse_args(func, arg, alist) == 0) return(NULL);
	if (v == NULL) {
		parse_error("%s(): No object specified\n", func->name);
		return(NULL);
	}
	if (x && y && size) {
		parse_error("%s(): Specify either size or (x, y, z), not both", 
			func->name);
		return(NULL);

	}
	if (x == 0) x = 1;
	if (y == 0) y = 1;
	if (z == 0) z = 1;
	if (size != 0) x=y=size;

	if (x == 0 || y == 0) {
		parse_error("%s(): No x or y specified", func->name);
		return(NULL);
	}

	init_sums(v, x, y, z, &rn, &rs, &rcount, &rmean, &rsigma, ignore);

	if (verbose) {
		a = new_struct(0);
		add_struct(a, "count", rcount);
		add_struct(a, "mean", rmean);
		add_struct(a, "sigma", rsigma);
		add_struct(a, "n", rn);
		add_struct(a, "s", rs);
		return(a);
	} else {
		mem_claim(rcount); free_var(rcount);
		mem_claim(rn);     free_var(rn);
		mem_claim(rs);     free_var(rs);
		return(rmean);
	}
}

/*
filter(type=STRING, size=INT, x=INT, y=INT, z=INT, ignore=, gsigma=FLOAT)

	"box"		- box filter, uniform XxY convolution
	"lpf"		- same as box filter
	"gaussian"	- gaussian filter
		gsigma		- width of gaussian
	"median"   	- median filter
	"sobel"		- sobel edge detection filter
	"unsharp"	- unsharp filter
*/

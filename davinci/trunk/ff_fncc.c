#include "parser.h"

double * TwoD_Convolve(int trow,int tcol,int frow,int fcol, double *tdata, double *fdata, double *ignore);

void build_t_constants(double *t, int t_row, int t_col, double *t_avg, double *t_prime, double *t2_prime, double *ignore);

double * fncc(double *r, int g_row,int g_col,int t_row,int t_col,int f_row,
					int f_col,double *f, double t_var, double t_avg,double *ignore);

double f_sum(double *f, int f_row, int f_col, int x, int y, int t_cen_row, 
				 int t_cen_col,int row_even_mod,int col_even_mod,double *sots,
				 int *ignore_count,double *ingnore);

void find_max_point(int col, int row, double *cc, int *p);

								
								 


Var *
ff_fncc(vfuncptr func, Var * arg)
{
	Var *obj=NULL;
	Var *template=NULL;
	int ac;
	Var **av;
	double *r;
	double *cc;

	char *results=NULL;
	char *result_enums[] = {"new_image","max_point","max","corner_point","corner","ncc", NULL};	

	int x,y;
	int f_row,f_col;
	int t_row,t_col;
	double *f=NULL;
	double *t=NULL;
	double *t_prime;

	double t_avg,t_var;
	double *ignore = NULL;
	double ig=MINFLOAT;
	double ref = MINFLOAT;

	int *p;

	Alist alist[5];
	alist[0] = make_alist("template",     ID_VAL,     NULL, &template);
	alist[1] = make_alist("object",     ID_VAL,     NULL, &obj);
	alist[2] = make_alist("output",     ID_ENUM,     result_enums, &results);
	alist[3] = make_alist("ignore",     DOUBLE,     NULL, &ig);
	alist[4].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (obj == NULL) {
		parse_error("%s: No object specified\n", func->name);
		return(NULL);
	}

	if (template == NULL) {
		parse_error("%s: No template (kernel) specified\n", func->name);
		return(NULL);
	}


	if (V_FORMAT(obj) != DOUBLE) {
		parse_error("%s: Object must contain DOUBLE values\n", func->name);
		return(NULL);
	}

	if (V_FORMAT(template) != DOUBLE) {
		parse_error("%s: Template must contain DOUBLE values\n", func->name);
		return(NULL);
	}

	if (GetZ(obj) > 1 || GetZ(template) > 1) {
		parse_error("%s: Currently only works on single Band objects\n", func->name);
		return(NULL);
	}

	f_row = GetY(obj);
	f_col = GetX(obj);
	f = (double *)V_DATA(obj);

	t_row = GetY(template);
	t_col = GetX(template);
	t = (double *)V_DATA(template);

	x = t_col+f_col-1;
	y = t_row+f_row-1;

	if (ig != ref) 
		ignore = &ig;

	t_prime = (double *)malloc(t_col * t_row * sizeof(double));
	parse_error("Building t constants");
	build_t_constants(t,t_row,t_col,&t_avg,t_prime,&t_var,ignore);
	parse_error("Varience of t: %g\n",t_var);
	parse_error("Avg of t: %g\n",t_avg);

	parse_error("Building convolution");
	r = TwoD_Convolve(t_row,t_col,f_row,f_col,t,f,ignore); 

	parse_error("Building cross-correlation");
	cc=fncc(r,y,x,t_row,t_col,f_row,f_col,f,t_var,t_avg,ignore);

	free(t_prime);

	free(r);

	if (results == NULL)
		return newVal(BSQ,x,y,1,DOUBLE,(void *)cc);

	else if (!strcmp(results,"ncc"))
		return newVal(BSQ,x,y,1,DOUBLE,(void *)cc);

	else if (!strcmp(results,"max_point") ||
				!strcmp(results,"max")			) {

		p = (int *) malloc(2 * sizeof(int));

		find_max_point(x,y,cc,p);

		parse_error("Max Point:    [%d, %d]\n",p[0]+1,p[1]+1);
		parse_error("Corner Point: [%d, %d]\n",(p[0]-t_col+2),(p[1]-t_row+2));
		return newVal(BSQ,x,y,1,DOUBLE,(void *)cc);

	}
/*
	else if (!strcmp(results,"corner_point") ||
				!strcmp(results,"corner")			) {

		p = (int *) malloc(2 * sizeof(int));

		find_max_point(x,y,cc,p);

		free(cc);
		p[0]=p[0]-t_col+2;	
		p[1]=p[1]-t_row+2;
		return newVal(BSQ,2,1,1,INT,(void *)p);
	}
*/
	else
		return(NULL);
}

void 
find_max_point(int col, int row, double *cc, int *p)
{
	int i,j;
	int x=0,y=0;
	double max=cc[0];
	
	for(i=0;i<row;i++){
		for(j=0;j<col;j++){
			if (cc[i*col+j] > max) {
				max = cc[i*col+j];
				x = j; y = i;
			}
		}
	}
	p[0]=x;p[1]=y;
}

double 
f_sum(double *f, int f_row, int f_col, int x, int y, int t_cen_row,
		int t_cen_col,int row_even_mod,int col_even_mod,double *sots,
		int *ignore_count,double *ignore)
{
	int i,j;
	double f_sum=0.0;
	double sum_of_the_squares=0.0;
	int ig_flag=0;
	double ig=MINFLOAT;
	double ic = 0; //ignore count;


	if (ignore != NULL) {
		ig_flag=1;
		ig = *ignore;
	}

	for (i = y-t_cen_row+row_even_mod; i <= (y+t_cen_row); i ++){
		for (j = x-t_cen_col+col_even_mod; j <= (x+t_cen_col); j++){

			if (i < 0 || i >= f_row || j < 0 || j >= f_col)
				
				;

			else 
				if (ig_flag  && f[i * f_col + j] == ig)
					ic++;

				else {
					f_sum += f[i * f_col + j];
					sum_of_the_squares +=(f[i * f_col + j] * f[i * f_col + j]);
				}
		}
	}
	*sots = sum_of_the_squares;
	*ignore_count=ic;
	return (f_sum);
}


double *
fncc(double *r, int g_row,int g_col,int t_row,int t_col,int f_row,
		int f_col,double *f, double t_var, double t_avg,double *ignore)
{


	int x,y;
	int i,j;
	int t_cen_row;
	int t_cen_col;
	int row_even_mod = 1;
	int col_even_mod = 1;
	double total = f_col * f_row;
	int f_ic = 0; //igore count for the f-window under t
	double tol = 1e-8;

	double fsum;
	double favg;
	double sots; //Sum of the Squares of f (under t)

	double *g = (double *)malloc(g_col * g_row * sizeof(double));

	int ten_p = ((double)g_row * 0.1);
	if (ten_p < 1) ten_p = 1;

	if(g == NULL) {
		parse_error("We don't have enough memory to make the final matrix!");
		return(NULL);
	}

	t_cen_row = (t_row/2);
 	t_cen_col = (t_col/2);

	if (t_row % 2 ) /* Odd row count */
		row_even_mod = 0;
	if (t_col % 2)  /* Odd col count */
		col_even_mod = 0;

	for (i=0;i<g_row;i++){
		y = i - t_cen_row;

		if (!((i+1) % ten_p))
			parse_error(".");

		for(j=0;j<g_col;j++){
			x = j - t_cen_col;
			fsum = f_sum(f,f_row,f_col,x,y,t_cen_row,t_cen_col,row_even_mod,col_even_mod,&sots,&f_ic,ignore);
			favg = (fsum*fsum)/(t_col*t_row-f_ic);


			{ /* For debugging purposes, I've broken up the expression to make sure we don't have dreck! */
				double p3;

				p3 = (sots - favg)/(t_col*t_row-1-f_ic);

				if ( p3 < 0. ) {
					parse_error("Error at cell: %d,%d we have:",j,i);
					g[i*g_col+j] = 0.;
				}

				else if ( p3 < tol && p3 > -tol) //as good as dead
					g[i*g_col+j] = MINFLOAT;

				else {

					g[i*g_col+j] = (r[i*g_col+j] - (fsum*t_avg))/(t_col*t_row-1-f_ic);
					g[i*g_col+j] /= sqrt( p3 * t_var);
				}
			}
		}
	}

	return(g);
}


void 
build_t_constants(double *t, int t_row, int t_col, double *t_avg, 
						double *t_prime, double *t_var, double *ignore)
{
/* NOTES:
** *t: 			Incomming template data set (pointer to array of row x col data)
** t_row:		Number of rows in t
** t_col:		Number of cols in t
** *t_avg:		Pointer, set value to the average of t
** *t_prime:	New data set.  Represents t-t_avg (pointer to array)
** *t_var:		Pointer, set value to sum of each value in t, minus t_avg, squared (ie the variance of t)
** *ignore:		Pointer to value to ignore (NA) or NULL if none supplied
*/

	int i,j;
	int ig_flag = 0; /*Ignore flag; 0 = not set, 1 = set */
	int ig_count = 0;
	double ig=MINFLOAT;
	double t1 = 0.0; /*Temporary var for t_avg */
	double t2 = 0.0; /*Temporary var for t2_prime */

	if (ignore != NULL) {
		ig_flag = 1;
		ig = *ignore;
	}

	for (i=0;i<t_row;i++){
		for(j=0;j<t_col;j++){

			if (ig_flag && t[i * t_col + j] == ig)  
				ig_count++;
			else
				t1 += t[i * t_col + j];
		}
	}

	t1 /= (double)(t_col * t_row - ig_count);
	*t_avg = t1;

	for (i=0;i<t_row;i++){
		for(j=0;j<t_col;j++){

			if (ig_flag && t[i * t_col + j] == ig)
					t_prime[i * t_col + j] = ig;

			else
			{
				t_prime[i * t_col + j] = t[i * t_col + j] - t1;
				t2 += t_prime[i * t_col + j] * t_prime[i * t_col + j];
			}

		}
	}

	*t_var = (t2/(double)(t_row*t_col-1-ig_count));
}

	
double *
TwoD_Convolve(int trow,int tcol,int frow,int fcol, double *t, double *f, double *ignore)
{
/*
	trow = template row size
	tcol = template col size
	frow = source row size
	fcol = source col size
	
	t = template data block (ptr)
	f = source data block (ptr)

*/

/* Set up matries sizes */

	int t_row = trow , t_col = tcol; 
	int f_row = frow , f_col = fcol; 

	int g_row,g_col; /* these will be calcualted */

	int k,l,x,y,i,j,ti,tj;

	int row_even_mod = 1;
	int col_even_mod = 1;

	int ig_flag = 0;
	double ig = MINFLOAT;

/* 
** While this integer division seems poor, it actually acomplishes
** the needed results.  An odd sized dimension of a zero-indexed array
** will center at the appropriate index (eg size 3 div 2 gives 1 --> [0,1,2]).
** It is the even sized dimensions which are screwy, but we want the same answer.
** That is, 2 div 2 also gives 1 --> [0, 1].  This is why we have a even_mod flag.
** The behavior of the template and the resulting matrix vary depending on which side
** you are on.  The Left and Top sides look and act as if the template is odd in dimension,
** while the Right and Bottom sides are off by 1.  This is where the even_mod flag comes in.
*/

	int t_cen_col = (t_col/2); /* Where is the collumn center */  
	int t_cen_row = (t_row/2); /* Where is the row center */


	double *g;
/*	double favg; */

	int ten_p;

/* DEBUG PURPOSES ONLY 
	int ff_1;
	int ff_2;
*/
	

	/* build info for output matrix: g */

	g_row = f_row + t_row - 1;
	g_col = f_col + t_col - 1;

	ten_p = ((double)g_row * 0.1);
	if (ten_p < 1) ten_p = 1;

	/* Malloc result array */
	
	g = (double *)malloc(g_col * g_row * sizeof(double));

	if (t_row % 2 ) /* Odd row count */
		row_even_mod = 0;
	if (t_col % 2)  /* Odd col count */
		col_even_mod = 0;



	if (ignore != NULL) {
		ig_flag=1;
		ig = *ignore;
	}



/*
** t is our template or kernel matrix.  It can be thought of as the pattern we seek,
** f is our source matrix.  We are looking for occurances of t inside f
** g is the solution matrix which is the result of convolving t and f

** k & l are our index into g
** x & y are our reference to f (but they will obtain non-valid values)
**       They are used to map between g, f and t
** i & j are our index into f
** ti & tj are our index into t
*/

	for (y = -t_cen_row;y<(f_row+t_cen_row-row_even_mod);y++){
		l = y + t_cen_row;
		if (!( (l+1) % ten_p))
			parse_error("+");
		for(x = -t_cen_col;x<(f_col+t_cen_col-col_even_mod);x++){
			k = x + t_cen_col;

			g[l*g_col+k] = 0.0; /* Initialize current array element */


/**********************************
** DEBUG ONLY **
			printf("g(%02d,%02d) =",k,l); 
			ff_1 = 1;
			ff_2 = 0;
***********************************/

			ti = 0;

			for (i = y-t_cen_row+row_even_mod; i <= (y+t_cen_row); i ++){
				tj = 0;
				for (j = x-t_cen_col+col_even_mod; j <= (x+t_cen_col); j++){


/********************************************************************
** DEBUG ONLY **
	
					if (ff_1) {
						printf("\t");
						ff_1=0;
					}
					else
						if (ff_2) {
							printf("\t\t");
							ff_2=0;
						}
						else
							printf(" ");

					if (i == (y+t_cen_row) & j == (x+t_cen_col))	
						printf("[t(%02d,%02d)*f(%02d,%02d)]\n",tj,ti,j,i);
					else
						printf("[t(%02d,%02d)*f(%02d,%02d)] +",tj,ti,j,i);
*********************************************************************/


					if (i < 0 || i >= f_row ||  /* skip invalid locations in f */
						 j < 0 || j >= f_col)     /* We can also add an INGORE value for f here as well */
						;
					else
						if (ig_flag && (t[ti * t_col + tj] == ig || f[i * f_col + j] ==ig))
							;
						else
							g[l*g_col+k] += t[ti * t_col + tj] * f[i * f_col + j] ;

					tj++;
				} /* for j */

/************************** 
** DEBUG ONLY **
				ff_2 = 1;
				printf("\n");
**************************/

				ti++;

			} /* for i */

/***********************
** DEBUG ONLY **
			printf("\n");
************************/


		} /* for x */
	} /* for y */

	return (g);
}


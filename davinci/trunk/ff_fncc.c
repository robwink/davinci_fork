#include "parser.h"

double * TwoD_Convolve(int trow,int tcol,int frow,int fcol, double *tdata, double *fdata, double *ignore);

void build_t_constants(double *t, int t_row, int t_col, double *t_avg, double *t_prime, double *t2_prime, double *ignore);

double * fncc(double *r, int g_row,int g_col,int t_row,int t_col,int f_row,
					int f_col,double *f, double t_var, double t_avg,double *ignore,
					double *rf, double *rf2);

double f_sum(double *f, int f_row, int f_col, int x, int y, int t_cen_row, 
				 int t_cen_col,int row_even_mod,int col_even_mod,double *sots,
				 int *ignore_count,double *ingnore);

void find_max_point(int col, int row, double *cc, int *p);

int build_running_sums(int trow,int tcol,int frow,int fcol,int grow,int gcol,double *f, double *rf, double *rf2);
								

Var *
ff_fncc(vfuncptr func, Var * arg)
{
	Var *obj=NULL;
	Var *template=NULL;
	Var *result;
	int ac;
	Var **av;
	double *r;
	double *cc;

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

	double *running_f = NULL;
	double *running_f2 = NULL;

	Var *rf=NULL;
	Var *rf2=NULL;

	int *p;
	int *p1;

	Alist alist[6];
	alist[0] = make_alist("template",     ID_VAL,     NULL, &template);
	alist[1] = make_alist("object",     ID_VAL,     NULL, &obj);
	alist[2] = make_alist("rf",     ID_VAL,     NULL, &rf);
	alist[3] = make_alist("r2",     ID_VAL,     NULL, &rf2);
	alist[4] = make_alist("ignore",     DOUBLE,     NULL, &ig);
	alist[5].name = NULL;

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
	
	if ((rf == NULL && rf2 != NULL) || (rf!=NULL && rf2==NULL)){
		parse_error("Hey, you can't sent just ONE running sum table...you HAVE to have both");
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

	if (rf == NULL) { /* by the query above, we know BOTH are NULL */
		running_f = (double *)malloc(x * y * sizeof(double));
		running_f2 = (double *)malloc(x * y * sizeof(double));

		if (running_f == NULL || running_f2 == NULL) {
			parse_error("Memory Error: Not enough memory to support this call!");
			return(NULL);
		}

		if (build_running_sums(t_row,t_col,f_row,f_col,y,x,f,running_f,running_f2))
			/* If we're here, there wasn't enough memory to do this! */
			return(NULL);
	}
	else {
		if (GetX(rf) != x || GetX(rf2) != x ||
			 GetY(rf) != y || GetY(rf2) != y  ){
			
			parse_error("The supplied tables are the wrong size for this template and object");
			return(NULL);
		}
		running_f = (double *)V_DATA(rf);
		running_f2 = (double *)V_DATA(rf2);
	}

	t_prime = (double *)malloc(t_col * t_row * sizeof(double));
	parse_error("Building t constants");
	build_t_constants(t,t_row,t_col,&t_avg,t_prime,&t_var,ignore);
	parse_error("Varience of t: %g\n",t_var);
	parse_error("Avg of t: %g\n",t_avg);

	parse_error("Building convolution");
	r = TwoD_Convolve(t_row,t_col,f_row,f_col,t,f,ignore); 

	parse_error("Building cross-correlation");
	cc=fncc(r,y,x,t_row,t_col,f_row,f_col,f,t_var,t_avg,ignore,running_f,running_f2);

	free(t_prime);

	free(r);

	p = (int *) malloc(2 * sizeof(int));
	p1 = (int *) malloc(2 * sizeof(int));
	find_max_point(x,y,cc,p);
	p[0]++;p[1]++;
	p1[0]=p[0]-t_col+2;
	p1[1]=p[1]-t_row+2;

	result = new_struct(5);
	add_struct(result,"cross_correlation",newVal(BSQ,x,y,1,DOUBLE,cc));
	add_struct(result,"max_point",newVal(BSQ,2,1,1,INT,p));
	add_struct(result,"corner_point",newVal(BSQ,2,1,1,INT,p1));
	add_struct(result,"running_sum",newVal(BSQ,x,y,1,DOUBLE,running_f));
	add_struct(result,"running_sum_squared",newVal(BSQ,x,y,1,DOUBLE,running_f2));


	return(result);
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
		int f_col,double *f, double t_var, double t_avg,double *ignore, 
		double *running_f, double *running_f2)
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

/*			fsum = f_sum(f,f_row,f_col,x,y,t_cen_row,t_cen_col,row_even_mod,col_even_mod,&sots,&f_ic,ignore); */

			fsum = running_f[i*g_col+j];
			sots = running_f2[i*g_col+j];

			favg = (fsum*fsum)/(t_col*t_row-f_ic);


/*
			if ( fabs(fsum-running_f[i*g_col+j]) > tol) 
				parse_error("@ %d,%d (row,col): fsum = %f while running_f = %f",i,j,fsum,running_f);
			if (fabs(sots-running_f2[i*g_col+j]) > tol)
				parse_error("@ %d,%d (row,col): sots = %f while running_f2 = %f",i,j,sots,running_f2);
*/



			{ /* For debugging purposes, I've broken up the expression to make sure we don't have dreck! */
				double p3;

				p3 = (sots - favg)/(t_col*t_row-1-f_ic);

				if ( p3 < 0. ) {
					parse_error("Error at cell: %d,%d we have: %g",j,i,p3);
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

int
build_running_sums(int trow,int tcol,int frow,int fcol,int grow,
						 int gcol,double *f, double *running_f, double *running_f2)
{
	double *s=NULL;
	double *ss=NULL;

	int x, y;
	int u, v;

	s = (double *)malloc(gcol * grow * sizeof(double));
	ss = (double *)malloc(gcol * grow * sizeof(double));

	if (s == NULL || ss == NULL ) {
			parse_error("Could not allocate enough memory to perform this task...aborting");
			if (s != NULL)
				free(s);
			if(ss != NULL)
				free(ss);
			return 1;
	}

/* 
** First we build a set a cumulative tables
*/

	parse_error("Building first round tables");

	for (v=0;v<grow;v++){
		for(u=0;u<gcol;u++){
			/* SUM of f */
			s[v*gcol+u] = (((u >= fcol)||(v >= frow)) ? 0.0 : f[v * fcol + u]);
			s[v*gcol+u]+= ((u == 0) ? 0.0 : s[v*gcol+(u-1)]);
			s[v*gcol+u]+= ((v == 0) ? 0.0 : s[(v-1)*gcol+u]);
			s[v*gcol+u]-= (((u == 0) || (v == 0)) ? 0.0 : s[(v-1)*gcol+(u-1)]);

			/* SUM of f^2 */
			ss[v*gcol+u] = (((u >= fcol)||(v >= frow)) ? 0.0 : (f[v * fcol + u] * f[v * fcol + u]));
			ss[v*gcol+u]+= ((u == 0) ? 0.0 : ss[v*gcol+(u-1)]);
			ss[v*gcol+u]+= ((v == 0) ? 0.0 : ss[(v-1)*gcol+u]);
			ss[v*gcol+u]-= (((u == 0) || (v == 0)) ? 0.0 : ss[(v-1)*gcol+(u-1)]);
		}
	}
			 
/* 
** Now we build the final sum tables
*/
	parse_error("Building second round tables");

	for(y=-(trow-1);y<grow-(trow-1);y++){
		v=y+(trow-1);
		for(x=-(tcol-1);x<gcol-(tcol-1);x++){
			u=x+(tcol-1);

			/* Final f SUM */
			running_f[v*gcol+u] = s[v*gcol+u];
			running_f[v*gcol+u]-= ((x < 1) ? 0.0 : s[v*gcol+(x-1)]);
			running_f[v*gcol+u]-= ((y < 1) ? 0.0 : s[(y-1)*gcol+u]);
			running_f[v*gcol+u]+= (((x < 1) || (y < 1)) ? 0.0 : s[(y-1)*gcol+(x-1)]);

			/* Final f^2 SUM */
			running_f2[v*gcol+u] = ss[v*gcol+u];
			running_f2[v*gcol+u]-= ((x < 1) ? 0.0 : ss[v*gcol+(x-1)]);
			running_f2[v*gcol+u]-= ((y < 1) ? 0.0 : ss[(y-1)*gcol+u]);
			running_f2[v*gcol+u]+= (((x < 1) || (y < 1)) ? 0.0 : ss[(y-1)*gcol+(x-1)]);
		}
	}

			
	parse_error("Done!");

	free(s);
	free(ss);

	return 0;
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


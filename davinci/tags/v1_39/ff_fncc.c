#include "parser.h"
#include "fft.h"
#include <errno.h>

extern int mayer_realfft(int n, double *out);
extern int mayer_realifft(int n, double *out);
extern int mayer_fft(int n, double *out, double *im);
extern int mayer_ifft(int n, double *out, double *im);

double *flip_t(int trow,int tcol,double *t);

double * TwoD_Convolve(int trow,int tcol,int frow,int fcol, double *tdata, double *fdata, double *ignore);
double * FFT_2D_Convolve(int trow,int tcol,int frow,int fcol, double *tdata, double *fdata);
double * pad_template(int trow,int tcol,int frow,int fcol, double *tdata);

void build_t_constants(double *t, int t_row, int t_col, double *t_avg, double *t_prime, double *t2_prime, double *ignore);

double * fncc(double *r, int g_row,int g_col,int t_row,int t_col,int f_row,
					int f_col,double *f, double t_var, double t_avg,double *ignore,
					double *rf, double *rf2, int rec);

double f_sum(double *f, int f_row, int f_col, int x, int y, int t_cen_row, 
				 int t_cen_col,int row_even_mod,int col_even_mod,double *sots,
				 int *ignore_count,double *ingnore);

void find_max_point(int col, int row, double *cc, int *p, double *val);

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
	int fft=0;

	int x,y;
	int f_row,f_col;
	int t_row,t_col;
	int rec = 0;

	double *f=NULL;
	double *t=NULL;
	double *tmp_t=NULL;
	double *t_prime;

	double t_avg,t_var;
	double *ignore = NULL;
	double ig=MINFLOAT;
	double ref = MINFLOAT;

	double *running_f = NULL;
	double *running_f2 = NULL;
	
	double *max_p=(double *)malloc(sizeof(double));

	Var *rf=NULL;
	Var *rf2=NULL;

	int *p;
	int *p1;

	Alist alist[8];
	alist[0] = make_alist("template",     ID_VAL,     NULL, &template);
	alist[1] = make_alist("object",     ID_VAL,     NULL, &obj);
	alist[2] = make_alist("rf",     ID_VAL,     NULL, &rf);
	alist[3] = make_alist("r2",     ID_VAL,     NULL, &rf2);
	alist[4] = make_alist("fft",     INT,     NULL, &fft);
	alist[5] = make_alist("ignore",     DOUBLE,     NULL, &ig);
	alist[6] = make_alist("rectify",     INT,     NULL, &rec);
	alist[7].name = NULL;

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
		/*DEBUG - set to odd value to find non-address elements */
		memset((void *)running_f,0xff,(x * y * sizeof(double))); 

		running_f2 = (double *)malloc(x * y * sizeof(double));
		memset((void *)running_f2,0xff,(x * y * sizeof(double))); 

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
	build_t_constants(t,t_row,t_col,&t_avg,t_prime,&t_var,ignore);
	free(t_prime);

	parse_error("Building convolution");

//	r = TwoD_Convolve(t_row,t_col,f_row,f_col,t,f,ignore); 
	r = (double *)calloc(x * y , sizeof(double));

	parse_error("Building cross-correlation");
	cc=fncc(r,y,x,t_row,t_col,f_row,f_col,f,t_var,t_avg,ignore,running_f,running_f2,rec);

	p = (int *) malloc(2 * sizeof(int));
	p1 = (int *) malloc(2 * sizeof(int));
	max_p[0]=MINFLOAT;
	find_max_point(x,y,cc,p,max_p);
	p[0]++;p[1]++;
	p1[0]=p[0]-t_col+1;
	p1[1]=p[1]-t_row+1;

	result = new_struct(3);
//	add_struct(result,"convolution",newVal(BSQ,x,y,1,DOUBLE,r));
//	add_struct(result,"cross_correlation",newVal(BSQ,x,y,1,DOUBLE,cc));
	free(cc);
	free(r);

/* Max point is 0-indexed */
	add_struct(result,"max_point",newVal(BSQ,2,1,1,INT,p));
/* Corner point is 0-indexed */
	add_struct(result,"corner_point",newVal(BSQ,2,1,1,INT,p1));

/* 
** Our corner point is based on a peak value in the corelation matrix.
** That value represents the strength of the match (1.0 would be a perfect match)
*/
	add_struct(result,"weight",newVal(BSQ,1,1,1,DOUBLE,max_p));


//	add_struct(result,"running_sum",newVal(BSQ,x,y,1,DOUBLE,running_f));
//	add_struct(result,"running_sum_squared",newVal(BSQ,x,y,1,DOUBLE,running_f2));

//	free(running_f);
//	free(running_f2);


	return(result);
}

double * 
FFT_2D_Convolve(int trow,int tcol,int frow,int fcol, double *t, double *f)
{
	double *pad_t;
	double *pad_f;

	double *f_vector_col;
	double *f_vector_row;
	double *t_vector_col;
	double *t_vector_row;
	double *t_fft;
	double *f_fft; 
	double *f_vector_col_im;
	double *f_vector_row_im;
	double *t_vector_col_im;
	double *t_vector_row_im;
	double *t_fft_im;
	double *f_fft_im; 

	double *r;

	int i,j;
	int col_pad;
	int row_pad;
	int row_byte_diff;
	int col_byte_diff;
	int row_byte;
	int col_byte;

	int gcol = fcol+tcol-1;
	int grow = frow+trow-1;
	
	int col_chunk = gcol * sizeof(double);

	double tol = 1e-9;

	pad_f=pad_template(frow,fcol,grow,gcol,f);

	pad_t=pad_template(trow,tcol,grow,gcol,t);

	row_pad = (int)pow(2.,ceil(log((double)grow)/log(2.)));
	col_pad = (int)pow(2.,ceil(log((double)gcol)/log(2.)));


	row_byte_diff = (row_pad-grow)*sizeof(double);
	col_byte_diff = (col_pad-gcol)*sizeof(double);
	row_byte=row_pad*sizeof(double);
	col_byte=col_pad*sizeof(double);


	f_vector_col = (double *)malloc(row_pad*sizeof(double)); //Collumn (has grow # of entries)
	f_vector_row = (double *)malloc(col_pad*sizeof(double)); //Row (has gcol # of entries)
	t_vector_col = (double *)malloc(row_pad*sizeof(double));
	t_vector_row = (double *)malloc(col_pad*sizeof(double));
	f_fft = (double *)malloc(col_pad*row_pad*sizeof(double));
	t_fft = (double *)malloc(col_pad*row_pad*sizeof(double));
//	r =  (double *)malloc(grow*gcol*sizeof(double));
	r =  (double *)malloc(col_pad*row_pad*sizeof(double));

	f_vector_col_im = (double *)malloc(row_pad*sizeof(double)); //Collumn (has grow # of entries)
	f_vector_row_im = (double *)malloc(col_pad*sizeof(double)); //Row (has gcol # of entries)
	t_vector_col_im = (double *)malloc(row_pad*sizeof(double));
	t_vector_row_im = (double *)malloc(col_pad*sizeof(double));
	f_fft_im = (double *)malloc(col_pad*row_pad*sizeof(double));
	t_fft_im = (double *)malloc(col_pad*row_pad*sizeof(double));

/* First lets translate every col, row by row (this is easy) */

	for(i=0;i<grow;i++){
		memset(f_vector_row,0x0,col_byte);
		memset(t_vector_row,0x0,col_byte);

		memset(f_vector_row_im,0x0,col_byte);
		memset(t_vector_row_im,0x0,col_byte);

		memcpy(f_vector_row,pad_f+(i*gcol),col_chunk);
		memcpy(t_vector_row,pad_t+(i*gcol),col_chunk);

//		mayer_realfft(col_pad,f_vector_row);
//		mayer_realfft(col_pad,t_vector_row);

		mayer_fft(col_pad,f_vector_row,f_vector_row_im);
		mayer_fft(col_pad,t_vector_row,t_vector_row_im);

		memcpy(f_fft+i*col_pad,f_vector_row,col_byte);
		memcpy(t_fft+i*col_pad,t_vector_row,col_byte);

		memcpy(f_fft_im+i*col_pad,f_vector_row_im,col_byte);
		memcpy(t_fft_im+i*col_pad,t_vector_row_im,col_byte);
	}

	printf("+");

/* 
** Now we process the rows, col by col.
** At the same time, after a collumn has been processed
** we do the multiplication and store the result back in f_fft
** 
*/

	for(j=0;j<col_pad;j++){
		memset(f_vector_col,0x0,row_byte);
		memset(t_vector_col,0x0,row_byte);
		memset(f_vector_col_im,0x0,row_byte);
		memset(t_vector_col_im,0x0,row_byte);
		for(i=0;i<grow;i++){
			f_vector_col[i]=f_fft[i*col_pad+j];
			t_vector_col[i]=t_fft[i*col_pad+j];
			f_vector_col_im[i]=f_fft_im[i*col_pad+j];
			t_vector_col_im[i]=t_fft_im[i*col_pad+j];
		}
//		mayer_realfft(row_pad,f_vector_col);
//		mayer_realfft(row_pad,t_vector_col);
		mayer_fft(row_pad,f_vector_col,f_vector_col_im);
		mayer_fft(row_pad,t_vector_col,t_vector_col_im);
		

		for(i=0;i<row_pad;i++) {
//			f_fft[i*col_pad+j] = f_vector_col[i];// * t_vector_col[i];// + f_vector_col_im[i] * t_vector_col_im[i]*-1.;

			f_fft[i*col_pad+j] = f_vector_col[i] * t_vector_col[i] + f_vector_col_im[i] * t_vector_col_im[i]*-1.;
			f_fft_im[i*col_pad+j] = f_vector_col[i] * t_vector_col_im[i] + f_vector_col_im[i]*t_vector_col[i] ;
		}

		
	}
	printf("+");
	

/*
** Now we have the maxtrix f_fft which is f*t in freq. space.
** Now we need to run an inverse fft on this matrix to get
** our convolution back!
*/

/*
** First we process colums, row by row
*/

	for (i=0;i<row_pad;i++){
		memcpy(f_vector_row,f_fft+(i*col_pad),col_byte);
		memcpy(f_vector_row_im,f_fft_im+(i*col_pad),col_byte);
//		mayer_realifft(col_pad,f_vector_row);
		mayer_ifft(col_pad,f_vector_row,f_vector_row_im);
		memcpy(f_fft+(i*col_pad),f_vector_row,col_byte);
		memcpy(f_fft_im+(i*col_pad),f_vector_row_im,col_byte);
	}	
	printf("+");

/*
** Now we process the rows, column by collumn
*/

	for(j=0;j<col_pad;j++){
		for(i=0;i<row_pad;i++) {
			f_vector_col[i] = f_fft[i*col_pad+j];
			f_vector_col_im[i] = f_fft_im[i*col_pad+j];
		}

//		mayer_realifft(row_pad,f_vector_col);
		mayer_ifft(row_pad,f_vector_col,f_vector_col_im);

		for(i=0;i<row_pad;i++) {
//			r[i*gcol+j]=f_vector_col[i];///(double)(col_pad*row_pad);	
//			if (r[i*gcol+j] < tol && r[i*gcol+j] > -tol)
//				r[i*gcol+j]=0.;
			r[i*col_pad+j]=f_vector_col[i]/(double)(col_pad*row_pad);	
			if (r[i*col_pad+j] < tol && r[i*col_pad+j] > -tol)
				r[i*col_pad+j]=0.;
		}
	}
	printf("+\n");

	return(r);
}

double * 
pad_template(int trow,int tcol,int frow,int fcol, double *t)
{
	double *pad_t;
	int i,j;

	pad_t = (double *)malloc(frow*fcol*sizeof(double));
	memset((void *) pad_t,0x0,(frow*fcol*sizeof(double)));

	for(i=0;i<trow;i++){
		for(j=0;j<tcol;j++){
			pad_t[i*fcol+j]=t[i*tcol+j];
		}
	}

	return(pad_t);
}

void 
find_max_point(int col, int row, double *cc, int *p, double *val)
{
	int i,j;
	int x=0,y=0;
	double max=cc[0];
	
	*val=max;
	for(i=0;i<row;i++){
		for(j=0;j<col;j++){
			if (cc[i*col+j] > max) {
				max = cc[i*col+j];
				*val=max;
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
		double *running_f, double *running_f2, int rec)
{


	int x,y;
	int i,j;
	int t_cen_row;
	int t_cen_col;
	int row_even_mod = 1;
	int col_even_mod = 1;
	double total = f_col * f_row;
	int f_ic = 0; //igore count for the f-window under t
	double tol = 1e-18;

	int 		SOTS_error_count = 0;
	double 	SOTS_error_abs_max = 0.;
	double 	SOTS_error_RMS = 0.;

	int 		FSUM_error_count = 0;
	double 	FSUM_error_abs_max = 0.;
	double 	FSUM_error_RMS = 0.;

	double fsum;
	double favg;
	double sots; //Sum of the Squares of f (under t)

	double *g = (double *)malloc(g_col * g_row * sizeof(double));
	
	double *f_error = (double *)malloc(g_col * g_row * sizeof(double));
	double *f2_error = (double *)malloc(g_col * g_row * sizeof(double));


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

//			fsum = running_f[i*g_col+j];
//			sots = running_f2[i*g_col+j];

			favg = (fsum*fsum)/(t_col*t_row-f_ic);

//			if ( fabs(fsum-running_f[i*g_col+j]) > tol)  {
				FSUM_error_count++;
				FSUM_error_RMS+=(fsum-running_f[i*g_col+j])*(fsum-running_f[i*g_col+j]);
				if (fabs(fsum-running_f[i*g_col+j]) > FSUM_error_abs_max ) FSUM_error_abs_max=fabs(fsum-running_f[i*g_col+j]);

//				fprintf(stderr,"%d,%d (row,col): FSUM = %16.12lf while running_f = %16.12lf (delta=%16.12lf)\n", i,j,fsum,running_f[i*g_col+j],fabs(fsum-running_f[i*g_col+j]));


				f_error[i*g_col+j] = fabs(fsum-running_f[i*g_col+j]);
//			}



//			if (fabs(sots-running_f2[i*g_col+j]) > tol) {
				SOTS_error_count++;
				SOTS_error_RMS+=(sots-running_f2[i*g_col+j])*(sots-running_f2[i*g_col+j]);
				if (fabs(sots-running_f2[i*g_col+j]) > SOTS_error_abs_max ) SOTS_error_abs_max=fabs(sots-running_f2[i*g_col+j]);
//				fprintf(stderr,"%d,%d (row,col): SOTS = %16.12lf while running_f2 = %16.12lf (delta=%16.12lf)\n", i,j,sots,running_f2[i*g_col+j],fabs(sots-running_f2[i*g_col+j]));
				f2_error[i*g_col+j] = fabs(sots-running_f2[i*g_col+j]);
//			}



			{ /* For debugging purposes, I've broken up the expression to make sure we don't have dreck! */
				double p3;

				p3 = (sots - favg)/(t_col*t_row-1-f_ic);

				if ( p3 < 0. ) {
//					parse_error("Error at cell: %d,%d we have: %g",j,i,p3);
					g[i*g_col+j] = MINFLOAT;
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

	if (FSUM_error_count) {
		fprintf(stderr,"FSUM: %d errors encountered out of %d calculations (--> %f%%)\n",
					FSUM_error_count,(g_col*g_row),((double)FSUM_error_count/(double)(g_col*g_row))*100.);
		fprintf(stderr,"\t\tRMS: %14.12f\n",sqrt(FSUM_error_RMS/(double)FSUM_error_count));
		fprintf(stderr,"\t\tABS MAX: %14.12f\n",FSUM_error_abs_max);
	}

	if (SOTS_error_count) {
		fprintf(stderr,"SOTS: %d errors encountered out of %d calculations (--> %f%%)\n",
					SOTS_error_count,(g_col*g_row),((double)SOTS_error_count/(double)(g_col*g_row))*100.);
		fprintf(stderr,"\tRMS: %14.12f\n",sqrt(SOTS_error_RMS/(double)SOTS_error_count));
		fprintf(stderr,"\tABS MAX: %14.12f\n",SOTS_error_abs_max);
	}

	{
		FILE *out;

		fprintf(stderr,"Writing out %d Col X %d Row Raw Double File: f_error\n",g_col,g_row);

		out = fopen("f_error","wb");
		fwrite(f_error,(g_col*g_row),sizeof(double),out);
		fclose(out);
		
		fprintf(stderr,"Writing out %d Col X %d Row Raw Double File: f2_error\n",g_col,g_row);

		out = fopen("f2_error","wb");
		fwrite(f2_error,(g_col*g_row),sizeof(double),out);
		fclose(out);
		

		free(f_error);
		free(f2_error);
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

	int t_cen_col;
	int t_cen_row;

	s = (double *)malloc(gcol * grow * sizeof(double));
	memset((void *)s,0x0,(gcol * grow * sizeof(double)));
	ss = (double *)malloc(gcol * grow * sizeof(double));
	memset((void *)ss,0x0,(gcol * grow * sizeof(double)));

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
	t_cen_row = (trow/2);
 	t_cen_col = (tcol/2);

/*
	for (v=0;v<grow;v++){
		for(u=0;u<gcol;u++){
*/

	for (v=0;v<grow;v++){
		for(u=0;u<gcol;u++){

			/* SUM of f */
			s[v*gcol+u] = (((u >= fcol)||(v >= frow)) ? (double)0.0 : f[v * fcol + u]);
			s[v*gcol+u]+= ((u == 0) ? (double)0.0 : s[v*gcol+(u-1)]);
			s[v*gcol+u]+= ((v == 0) ? (double)0.0 : s[(v-1)*gcol+u]);
			s[v*gcol+u]-= (((u == 0) || (v == 0)) ? (double)0.0 : s[(v-1)*gcol+(u-1)]);

			/* SUM of f^2 */
			ss[v*gcol+u] = (((u >= fcol)||(v >= frow)) ? (double)0.0 : (f[v * fcol + u] * f[v * fcol + u]));
			ss[v*gcol+u]+= ((u == 0) ? (double)0.0 : ss[v*gcol+(u-1)]);
			ss[v*gcol+u]+= ((v == 0) ? (double)0.0 : ss[(v-1)*gcol+u]);
			ss[v*gcol+u]-= (((u == 0) || (v == 0)) ? (double)0.0 : ss[(v-1)*gcol+(u-1)]);



/*
			s[v*gcol+u]+= ((u == 0) ? 0.0 : s[v*gcol+(u-1)]);
			s[v*gcol+u]+= ((v == 0) ? 0.0 : s[(v-1)*gcol+u]);
			s[v*gcol+u]-= (((u == 0) || (v == 0)) ? 0.0 : s[(v-1)*gcol+(u-1)]);

			ss[v*gcol+u]+= ((u == 0) ? 0.0 : ss[v*gcol+(u-1)]);
			ss[v*gcol+u]+= ((v == 0) ? 0.0 : ss[(v-1)*gcol+u]);
			ss[v*gcol+u]-= (((u == 0) || (v == 0)) ? 0.0 : ss[(v-1)*gcol+(u-1)]);
*/
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
			running_f[v*gcol+u]-= ((x < 1) ? (double)0.0 : s[v*gcol+(x-1)]);
			running_f[v*gcol+u]-= ((y < 1) ? (double)0.0 : s[(y-1)*gcol+u]);
			running_f[v*gcol+u]+= (((x < 1) || (y < 1)) ? (double)0.0 : s[(y-1)*gcol+(x-1)]);

			/* Final f^2 SUM */
			running_f2[v*gcol+u] = ss[v*gcol+u];
			running_f2[v*gcol+u]-= ((x < 1) ? (double)0.0 : ss[v*gcol+(x-1)]);
			running_f2[v*gcol+u]-= ((y < 1) ? (double)0.0 : ss[(y-1)*gcol+u]);
			running_f2[v*gcol+u]+= (((x < 1) || (y < 1)) ? (double)0.0 : ss[(y-1)*gcol+(x-1)]);
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

double *
flip_t(int trow,int tcol,double *t)
{
	int i,j;

	double *tt = (double *)malloc(trow*tcol*sizeof(double));

	for(i=0;i<trow;i++){
		for(j=0;j<tcol;j++){
			tt[(trow-i-1)*tcol+(tcol-j-1)]=t[i*tcol+j];
		}
	}

	return(tt);
}



#define reverse4(x) { unsigned char *p=(char *)&(x), t; t=p[0]; p[0]=p[3]; p[3]=t; t=p[1]; p[1]=p[2]; p[2]=t; }

Var *
ff_fncc_write_mat(vfuncptr func, Var * arg)
{
    FILE  *fp      = NULL;
	Var   *obj     = NULL;
	char  *fname   = NULL;
	long   complex = 0;    /* assume strided complex planes in the z-direction */
	int    force   = 0;    /* write the file even when it already exists */
	long   x, y, z;
	long   hdr_type;
	char  *obj_name = NULL;
	long   namelen;
	int    i, j, k;

	long   ii, ll;
	short  ss;
	char   bb;
	double dd;
	float  ff;

	Alist alist[4];
	alist[0] = make_alist( "obj",       ID_UNK,     NULL,     &obj);
	alist[1] = make_alist( "filename",  ID_STRING,  NULL,     &fname);
	alist[2] = make_alist( "force",     INT,        NULL,     &force);
	alist[3].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(newInt(0));

	if (obj == NULL) {
        parse_error("%s: missing obj.", func->name); return (newInt(0));
	}

	if (fname == NULL){
		parse_error("%s: missing filename.\n", func->name); return(newInt(0));
	}


	x = GetX(obj); y = GetY(obj); z = GetZ(obj);
	if (!(z == 2 || z == 1)){
		parse_error("%s: data size must be XxYx1 for real and XxYx2 for complex.\n", func->name);
		return newInt(0);
	}

	complex = (z == 2);

#ifdef WORDS_BIGENDIAN
	hdr_type = 1000;
#else
	hdr_type = 0000;
#endif /* WORDS_BIGENDIAN */

	switch(V_FORMAT(obj)){
	case BYTE:   hdr_type += 50; break;
	case SHORT:  hdr_type += 40; break;
	case INT:    hdr_type += 20; break;
	case FLOAT:  hdr_type += 10; break;
	case DOUBLE: hdr_type += 00; break;
	default:
		parse_error("%s: Unsupported data type %s in obj.\n",
			func->name, Format2Str(V_FORMAT(obj)));
		return newInt(0);
		break;
	}

	if (access(fname, F_OK) == 0 && !force){
		parse_error("%s: File %s already exists. Use \"force\".\n", func->name, fname);
		return newInt(0);
	}

	if ((fp = fopen(fname, "wb")) == NULL){
		parse_error("%s: Unable to write file %s. Reason: %s.\n",
			func->name, fname, strerror(errno));
		return newInt(0);
	}


	/* construct mat-file header */

	obj_name = (obj->name == NULL)? "obj": obj->name;
	namelen = strlen(obj_name) + 1;

	for(i = 0; i < 5; i++){
		switch(i){
		case 0: ll = hdr_type; break;
		case 1: ll = y; break;
		case 2: ll = x; break;
		case 3: ll = complex; break;
		case 4: ll = namelen; break;
		}
#ifndef WORDS_BIGENDIAN
		/* reverse4(ll); */
#endif /* WORDS_BIGENDIAN */
		fwrite(&ll, sizeof(long), 1, fp);
	}
	fwrite(obj_name, sizeof(char), namelen, fp);

	for(k = 0; k < z; k++){
		for(i = 0; i < x; i++){
			for(j = 0; j < y; j++){
				switch(V_FORMAT(obj)){
				case BYTE:
					bb = extract_int(obj, cpos(i,j,k,obj));
					fwrite(&bb, sizeof(bb), 1, fp);
					break;
				case SHORT:
					ss = extract_int(obj, cpos(i,j,k,obj));
					fwrite(&ss, sizeof(ss), 1, fp);
					break;
				case INT:
					ii = extract_int(obj, cpos(i,j,k,obj));
					fwrite(&ii, sizeof(ii), 1, fp);
					break;
				case FLOAT:
					ff = extract_double(obj, cpos(i,j,k,obj));
					fwrite(&ff, sizeof(ff), 1, fp);
					break;
				case DOUBLE:
					dd = extract_double(obj, cpos(i,j,k,obj));
					fwrite(&dd, sizeof(dd), 1, fp);
					break;
				default:
					parse_error("%s: INTERNAL ERROR: %s:%d reached. Giving up!\n",
						func->name, __FILE__, __LINE__);
					fclose(fp);
					return newInt(0);
				}
			}
		}
	}

	fclose(fp);
    return newInt(1);
}

static double *fft_convolve_real(double *a, int arows, int acols, double *b, int brows, int bcols, int *rows, int *cols);
static COMPLEX *complex_multiply(COMPLEX *c1, COMPLEX *c2, int rows, int cols);
static COMPLEX *fft2d(COMPLEX *input, int rows, int cols);
static COMPLEX *ifft2d(COMPLEX *input, int rows, int cols);

Var *
ff_fncc_fft2d(vfuncptr func, Var *arg)
{
	Var *obj = NULL;
    double *output = NULL;
    int cols, rows, depth;
    int i, j, k;
    double v;
    COMPLEX *input = NULL, *result = NULL;

	Alist alist[2];
	alist[0] = make_alist("obj",     ID_VAL,     NULL, &obj);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (obj == NULL) {
		parse_error("%s: obj not specified\n", func->name);
		return(NULL);
	}

	cols = GetX(obj); rows = GetY(obj); depth = GetZ(obj);
    if (depth > 2){
            parse_error("%s: Depth can either be 1 (real data) or 2 (complex data)\n", func->name);
            return NULL;
    }

    input = (COMPLEX *)calloc(sizeof(COMPLEX), rows*cols);
    if (input == NULL){
            parse_error("%s: calloc(%d,%d) failed\n", sizeof(COMPLEX), rows*cols, func->name);
            return NULL;
    }

    for(k=0; k<depth; k++){
            for(j=0; j<rows; j++){
                    for(i=0; i<cols; i++){
                            v = extract_double(obj, cpos(i,j,k,obj));
                            if (k == 0){ input[j*cols+i].re = v; }
                            else       { input[j*cols+i].im = v; }
                    }
            }
    }

    result = fft2d(input, rows, cols);
    free(input);

    output = (double *)calloc(sizeof(double), rows*cols*2);
    for(j=0; j<rows; j++){
            for(i=0; i<cols; i++){
                    output[0*(rows*cols)+j*cols+i] = result[j*cols+i].re;
                    output[1*(rows*cols)+j*cols+i] = result[j*cols+i].im;
            }
    }
    free(result);

    return(newVal(BSQ, cols, rows, 2, DOUBLE, output));
}

Var *
ff_fncc_ifft2d(vfuncptr func, Var *arg)
{
	Var *obj = NULL;
    double *output = NULL;
    int cols, rows, depth;
    int i, j, k;
    double v;
    COMPLEX *input = NULL, *result = NULL;

	Alist alist[2];
	alist[0] = make_alist("obj",     ID_VAL,     NULL, &obj);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (obj == NULL) {
		parse_error("%s: obj not specified\n", func->name);
		return(NULL);
	}

	cols = GetX(obj); rows = GetY(obj); depth = GetZ(obj);
    if (depth > 2){
            parse_error("%s: Depth can either be 1 (real data) or 2 (complex data)\n", func->name);
            return NULL;
    }

    input = (COMPLEX *)calloc(sizeof(COMPLEX), rows*cols);
    if (input == NULL){
            parse_error("%s: calloc(%d,%d) failed\n", sizeof(COMPLEX), rows*cols, func->name);
            return NULL;
    }

    for(k=0; k<depth; k++){
            for(j=0; j<rows; j++){
                    for(i=0; i<cols; i++){
                            v = extract_double(obj, cpos(i,j,k,obj));
                            if (k == 0){ input[j*cols+i].re = v; }
                            else       { input[j*cols+i].im = v; }
                    }
            }
    }

    result = ifft2d(input, rows, cols);
    free(input);

    output = (double *)calloc(sizeof(double), rows*cols*2);
    for(j=0; j<rows; j++){
            for(i=0; i<cols; i++){
                    output[0*(rows*cols)+j*cols+i] = result[j*cols+i].re;
                    output[1*(rows*cols)+j*cols+i] = result[j*cols+i].im;
            }
    }
    free(result);

    return(newVal(BSQ, cols, rows, 2, DOUBLE, output));
}

Var *
ff_fncc_cmplx_mul(vfuncptr func, Var *arg)
{
	Var *obj1 = NULL, *obj2 = NULL, *obj = NULL;
    COMPLEX *i1 = NULL, *i2 = NULL, *input = NULL;
    COMPLEX *result = NULL;
    double *output = NULL;
    int cols, rows, depth;
    int i, j, k, o;
    double v;

	Alist alist[3];
	alist[0] = make_alist("obj1",     ID_VAL,     NULL, &obj1);
	alist[1] = make_alist("obj2",     ID_VAL,     NULL, &obj2);
	alist[2].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (obj1 == NULL || obj2 == NULL) {
		parse_error("%s: Either one of obj1 or obj2 was not specified\n", func->name);
		return(NULL);
	}

	cols = GetX(obj1);
	rows = GetY(obj1);
    depth = GetZ(obj1);

    if (depth != 2){
            parse_error("%s: Depth cannot be other than 2 (real plane + imag plane)\n", func->name);
            return NULL;
    }

    if (cols != GetX(obj2) || rows != GetY(obj2) || depth != GetZ(obj2)){
            parse_error("%s: obj1 and obj2 must have the same dimension\n", func->name);
            return NULL;
    }

    i1 = (COMPLEX *)calloc(sizeof(COMPLEX), rows*cols);
    i2 = (COMPLEX *)calloc(sizeof(COMPLEX), rows*cols);
    if (i1 == NULL || i2 == NULL){
            parse_error("%s: calloc(%d,%d)*2 failed\n", sizeof(COMPLEX), rows*cols, func->name);
            free(i1); free(i2);
            return NULL;
    }

    for(o=0; o<2; o++){
            if (o==0){ obj = obj1; input = i1; }
            else { obj = obj2; input = i2; }
            
            for(k=0; k<depth; k++){
                    for(j=0; j<rows; j++){
                            for(i=0; i<cols; i++){
                                    v = extract_double(obj, cpos(i,j,k,obj));
                                    if (k==0){ input[j*cols+i].re = v; }
                                    else     { input[j*cols+i].im = v; }
                            }
                    }
            }
    }
    result = complex_multiply(i1, i2, rows, cols);
    free(i1); free(i2);

    output = (double *)calloc(sizeof(double), rows*cols*2);
    if (output == NULL){
            parse_error("%s: calloc(%d,%d)*2 failed\n", sizeof(double), rows*cols, func->name);
            free(result);
            return NULL;
    }
    for(k=0; k<2; k++){
            for(j=0; j<rows; j++){
                    for(i=0; i<cols; i++){
                            output[k*rows*cols+j*cols+i] = k==0? result[j*cols+i].re: result[j*cols+i].im;
                    }
            }
    }
    free(result);

    return(newVal(BSQ, cols, rows, depth, DOUBLE, output));
}


Var *
ff_fncc_fft_conv_real(vfuncptr func, Var *arg)
{
    Var *obj[2] = { NULL, NULL };
    double *input[2] = { NULL, NULL };
    double *result = NULL;
    int cols[2], rows[2], depth[2];
    int outrows, outcols;
    int i, j, o;
    double v;

	Alist alist[3];
	alist[0] = make_alist("obj1",     ID_VAL,     NULL, &obj[0]);
	alist[1] = make_alist("obj2",     ID_VAL,     NULL, &obj[1]);
	alist[2].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (obj[0] == NULL || obj[1] == NULL) {
		parse_error("%s: Either one of obj1 or obj2 was not specified\n", func->name);
		return(NULL);
	}

    for(o=0; o<2; o++){
            cols[o] = GetX(obj[o]); rows[o] = GetY(obj[o]); depth[o] = GetZ(obj[o]);
    }
    
    if (depth[0] != 1 && depth[1] != 1){
            parse_error("%s: Depth of objs cannot be other than 1\n", func->name);
            return NULL;
    }

    for(o=0; o<2; o++){
            input[o] = (double *)calloc(sizeof(double), rows[o]*cols[o]);
            if (input[o] == NULL){
                    parse_error("%s: calloc(%d,%d) failed\n",
                                sizeof(double), rows[o]*cols[o], func->name);
                    free(input[0]); free(input[1]);
                    return NULL;
            }
    }

    for(o=0; o<2; o++){
            for(j=0; j<rows[o]; j++){
                    for(i=0; i<cols[o]; i++){
                            v = extract_double(obj[o], cpos(i,j,0,obj[o]));
                            input[o][j*cols[o]+i] = v;
                    }
            }
    }
    result = fft_convolve_real(input[0], rows[0], cols[0], input[1], rows[1], cols[1], &outrows, &outcols);
    free(input[0]); free(input[1]);
	if (result == NULL){
		parse_error("%g: Mem allocation failure in fft_convolve_real()\n", func->name);
		return NULL;
	}

    return(newVal(BSQ, outcols, outrows, 1, DOUBLE, result));
}


static double *
fft_convolve_real(
        double *a, int arows, int acols,
        double *b, int brows, int bcols,
        int *rows, int *cols)
{
        COMPLEX *data, *afft, *bfft, *result;
        double *output;
        int i,j;

        *rows = arows+brows-1;
        *cols = acols+bcols-1;

        data = (COMPLEX *)calloc(sizeof(COMPLEX), (*rows)*(*cols));
		if (data == NULL){ return NULL; }
        for(j=0; j<arows; j++){
                for(i=0; i<acols; i++){
                        data[j*(*cols)+i].re = a[j*acols+i];
                        data[j*(*cols)+i].im = 0.0;
                }
        }
        afft = fft2d(data, *rows, *cols);

        memset(data, 0, sizeof(COMPLEX)*(*rows)*(*cols));
        for(j=0; j<brows; j++){
                for(i=0; i<bcols; i++){
                        data[j*(*cols)+i].re = b[j*bcols+i];
                        data[j*(*cols)+i].im = 0.0;
                }
        }
        bfft = fft2d(data, *rows, *cols);
        
        free(data);
        data = complex_multiply(afft, bfft, *rows, *cols);
        free(afft); free(bfft);
		if (data == NULL){ return NULL; }

        result = ifft2d(data, *rows, *cols); free(data);
		if (result == NULL){ return NULL; }

        /* pull the real part out - we only care about the real part */
        output = (double *)calloc(sizeof(double), (*rows)*(*cols));
		if (output == NULL){ free(result); return NULL; }
        for(j=0; j<*rows; j++){
                for(i=0; i<*cols; i++){
                        output[j*(*cols)+i] = result[j*(*cols)+i].re;
                }
        }
        free(result);

        return output;
}

static COMPLEX *
complex_multiply(COMPLEX *c1, COMPLEX *c2, int rows, int cols)
{
        int i, j;
        double a, b, c, d;
        COMPLEX *result = (COMPLEX *)calloc(sizeof(COMPLEX), rows*cols);

		if (result == NULL){ return NULL; }
        
        for(j=0; j<rows; j++){
                for(i=0; i<cols; i++){
                        a = c1[j*cols+i].re; b = c1[j*cols+i].im;
                        c = c2[j*cols+i].re; d = c2[j*cols+i].im;
                        
                        result[j*cols+i].re = a*c - b*d;
                        result[j*cols+i].im = a*d + b*c;
                }
        }
        return result;
}

static COMPLEX *
fft2d(COMPLEX *input, int rows, int cols)
{
        COMPLEX *result = (COMPLEX *)calloc(sizeof(COMPLEX), rows*cols);
        COMPLEX *col = (COMPLEX *)calloc(sizeof(COMPLEX), rows);
        COMPLEX *col_result = (COMPLEX *)calloc(sizeof(COMPLEX), rows);
        int i, j;

		if (result == NULL || col == NULL || col_result == NULL){
			free(result); free(col); free(col_result); return NULL;
		}

        for(j=0; j<rows; j++){
                fft(&input[j*cols], cols, &result[j*cols]);
        }
        for(i=0; i<cols; i++){
                for(j=0; j<rows; j++){ col[j] = result[j*cols+i]; }
                fft(col, rows, col_result);
                for(j=0; j<rows; j++){
					result[j*cols+i] = col_result[j];
					result[j*cols+i].re *= (rows*cols);
					result[j*cols+i].im *= (rows*cols);
				}
        }

        free(col); free(col_result);

        return result;
}

static COMPLEX *
ifft2d(COMPLEX *input, int rows, int cols)
{
        COMPLEX *result = (COMPLEX *)calloc(sizeof(COMPLEX), rows*cols);
        COMPLEX *col = (COMPLEX *)calloc(sizeof(COMPLEX), rows);
        COMPLEX *col_result = (COMPLEX *)calloc(sizeof(COMPLEX), rows);
        int i, j;

		if (result == NULL || col == NULL || col_result == NULL){
			free(result); free(col); free(col_result); return NULL;
		}

        for(j=0; j<rows; j++){
                rft(&input[j*cols], cols, &result[j*cols]);
        }
        for(i=0; i<cols; i++){
                for(j=0; j<rows; j++){ col[j] = result[j*cols+i]; }
                rft(col, rows, col_result);
                for(j=0; j<rows; j++){
					result[j*cols+i] = col_result[j];
					result[j*cols+i].re /= (rows*cols);
					result[j*cols+i].im /= (rows*cols);
				}
        }

        free(col); free(col_result);

        return result;
}

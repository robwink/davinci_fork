#include "parser.h"
#include "dvio.h"

static float *do_resample(float *y, float *xold, float *xnew, int npts, int new_npts);

Var* ff_resample(vfuncptr func, Var * arg)
{

	//Set up Vars for input/ouput
	Var    *oldx=NULL;                 //old x
	Var    *oldy=NULL;                 //old y
	Var    *newx=NULL;                 //new x
	Var    *out=NULL;                  //output array
	Var    *v_label=NULL;              //label var
	Var    **av;                       //argument values
	Var    *st = NULL;                 //structure
	Alist  alist[4];                   //argument list
	int    ac;                            //argument count

	//arrays and indices
	char   *label=NULL;                //label char
	float  *xold=NULL;                 //old x array
	float  *yold=NULL;                 //old y array
	float  *xnew=NULL;                 //new x array
	float  *ynew=NULL;                 //new x array
	float  *ynew_tmp=NULL;
	float  *yold_tmp=NULL;
	int    i,j,k,npts,new_npts;       //indices and sizes
	int    xi,yi;	                   //dimensions
	int    l_type=0;                   //label type (sample_name or label)
	int    type=0;
	
	make_args(&ac, &av, func, arg);	
	ac=ac-1;

	if (ac == 2) {
		alist[0] = make_alist("foo", ID_STRUCT, NULL, &st);
		alist[1] = make_alist("newx", ID_VAL, NULL, &newx);
		alist[2].name = NULL;
		if (parse_args(func, arg, alist) == 0) return(NULL);
		
		find_struct(st,"data",&oldy);
		find_struct(st,"label",&v_label);
		find_struct(st,"xaxis",&oldx);
		
		if(oldy==NULL) {
			parse_error("Structure does not contain element .data"); 
			return(NULL);
		}
		if (!(v_label != NULL && (V_TYPE(v_label) == ID_STRING || V_TYPE(v_label) == ID_TEXT))){
			find_struct(st,"sample_name",&v_label);
			l_type=1;
			if (!(v_label != NULL && (V_TYPE(v_label) == ID_STRING|| V_TYPE(v_label) == ID_TEXT))){
				parse_error("Structure does not contain element .label or .sample_name"); 
				return(NULL);
			}
		}
		if(oldx==NULL) {
			parse_error("Structure does not contain element .xaxis"); 
			return(NULL);
		}
		if (V_TYPE(v_label)==ID_STRING) label = strdup(V_STRING(v_label));
  }
  else if (ac == 3) {
		alist[0] = make_alist("oldy",    ID_VAL,      NULL,  &oldy);
		alist[1] = make_alist("oldx",    ID_VAL,      NULL,  &oldx); 
		alist[2] = make_alist("newx",    ID_VAL,      NULL,  &newx); 
		alist[3].name = NULL;
		
		if (parse_args(func, arg, alist) == 0) return(NULL);
		
		/*set label equal to */
		label = strdup("(null)");
	}
	else {
		printf ("\n resample() - 12/1/2007\n");
		printf (" Resample a spectrum to a given scale using cubic spline interpolation \n");
		printf (" $1 = spectrum to be resampled \n");
		printf (" $2 = old scale \n");
		printf (" $3 = new scale \n");
		printf ("\nOR\n\n");
		printf (" $1 = standard spectral structure with .data, .xaxis, .label \n");
		printf (" $2 = new scale \n");
		printf (" \n");
		printf (" c.edwards\n\n");
		return(NULL);
	}  	

	/* get dimensions of arrays and allocate memory*/
	xi=GetX(oldy);
	yi=GetY(oldy);
	npts=GetZ(oldx);
	new_npts=GetZ(newx);

	yold = (float *) calloc(sizeof(float),npts*xi*yi);
	ynew = (float *) calloc(sizeof(float),new_npts*xi*yi);
	xold = (float *) calloc(sizeof(float),npts);
	xnew = (float *) calloc(sizeof(float),new_npts);
	yold_tmp = (float *) calloc(sizeof(float),npts);
	ynew_tmp = (float *) calloc(sizeof(float),new_npts);

	parse_error("1");
	//check to see which axis things are on
	if(yi > 1 && xi > 1) {
		parse_error("Multiple spectra must be included in one axis");
		return(NULL);
	}
	if(yi>1) type=1;
	if(xi>1) type=2;
	parse_error("2");

	//extract the data from the old x and y at the same time
	for(k=0;k<npts;k++) {
		xold[k]=extract_float(oldx,cpos(0,0,k,oldx));
		for(i=0;i<xi;i++) {
			for(j=0;j<yi;j++) {
				yold[k*xi*yi + i*yi + j] = extract_float(oldy,cpos(i,j,k,oldy));
			}
		}
	}			

	parse_error("3");
	//extract the newx data
	for(i=0;i<new_npts;i++) {
		xnew[i]=extract_float(newx,cpos(0,0,i,newx));
	}
	parse_error("4");

	//if the data is in y make copies of each array section and then resample and replace in to proper output array
	if(type==1) {
		for(i=1;i<=yi;i+=1) {
			for(k=0;k<npts;k+=1) {
				yold_tmp[k]=yold[npts*i+k];
			}
			ynew_tmp=do_resample(yold_tmp,xold,xnew,npts,new_npts);
			for(k=0;k<new_npts;k+=1) {
				ynew[new_npts*i+k]=ynew_tmp[k];
			}
		}
	}
	parse_error("5");

	//if the data is in x make copies of each array section and then resample and replace in to proper output array
	if(type==2) {
		for(i=1;i<=xi;i+=1) {
			for(k=0;k<npts;k+=1) {
				yold_tmp[k]=yold[npts*i+k];
			}
			ynew_tmp=do_resample(yold_tmp,xold,xnew,npts,new_npts);
			for(k=0;k<new_npts;k+=1) {
				ynew[new_npts*i+k]=ynew_tmp[k];
			}
		}
	}
	parse_error("5");

	//free stuff (check to make sure this is all)
	//	free(yold_tmp);
	//	free(ynew_tmp);
	//	free(xold);
	return(newVal(BSQ, xi, yi, new_npts, FLOAT, ynew));
	/* create the output structure */
	out=new_struct(3);
	add_struct(out,"data", newVal(BSQ, xi, yi, new_npts, FLOAT, ynew));
	if(V_TYPE(v_label)==ID_TEXT) {
		if(l_type==0) {	
			add_struct(out,"label",v_label);
		}
		if(l_type==1) {
			add_struct(out,"sample_name",v_label);
		}
	} else {
		if(l_type==0) {	
			add_struct(out,"label", newString(label));
		}
		if(l_type==1) {
			add_struct(out,"sample_name", newString(label));
		}
	}
	add_struct(out,"xaxis", newVal(BSQ, 1, 1, new_npts, FLOAT, xnew));
	return(out);
}





static float *do_resample(float *y, float *xold, float *xnew, int npts, int new_npts) {

 	float  *ynew=NULL;                 //new y array
	float  *y2d=NULL;                  //second derivative array
	float  *u=NULL;                    //u array
	int     start_count,stop_count;    //stop and start locations               
	float   sig,p,h,a,b;               //cubic spline variables
	int     samp_hi,samp_lo,samp_new;  //hi,lo,new samples
	float   min=0,max=0;               //min and max values
	int     i;

	y2d = (float *) calloc(sizeof(float),npts);
	u = (float *) calloc(sizeof(float),npts-1);
	ynew = (float *) calloc(sizeof(float),new_npts);
  
	/*set boundary spline conditions; Second derivative is 0. */
	min=xold[0];
	max=xold[npts-1];
	for(i=0;i<npts;i++) {
		if(xold[i] < min) { min=xold[i]; }
		if(xold[i] > max) { max=xold[i]; }
		y2d[i]=y[i]-y[i];
		if(i<npts-1) {
			u[i]=y2d[i];
		}
	}  
	
	/*Set start and end points in case many values are zeroed.*/
	start_count=1;
	while (y[start_count] == 0. && start_count <= npts) {
		start_count=start_count+1;
	}
	stop_count=npts;
	while (y[stop_count] == 0. && stop_count >=1) {
		stop_count=stop_count-1;
	}
  
	/* Do the decomposition loop of the tridiagonal algorithm */
	for (i=(start_count); i<=(stop_count-1); i+=1) {
		sig=(xold[i]-xold[(i-1)])/(xold[(i+1)]-xold[(i-1)]);
		p=sig*y2d[(i-1)]+2.;
		y2d[i]=(sig-1.)/p;
		if(xold[(i+1)]-xold[i] !=0 ) { u[i]=(y[(i+1)]-y[i])/(xold[(i+1)]-xold[i]);}
		if(xold[i]-xold[(i-1)] !=0 ) { u[i]= u[i] - (y[i]-y[(i-1)])/(xold[i]-xold[(i-1)]); }
		if(xold[(i+1)]-xold[(i-1)] !=0) { u[i]=(6.*u[i]/(xold[(i+1)]-xold[(i-1)]) - sig*u[(i-1)])/p;}
	}
	
	for (i=(stop_count-1); i>=start_count; i-=1) {
		y2d[i]=y2d[i]*y2d[(i+1)] + u[i];
	}
  	
	/* Now that we have the second derivative //
	// we can evaluate our y with respect to the new xaxis // 
	// get the size of the new xaxis and alloate the memory */
	for (i=0; i<new_npts; i+=1) {
		samp_hi=npts-1;
		samp_lo=0;
		while (samp_hi-samp_lo > 1) {
			samp_new=(samp_hi+samp_lo)/2;
			if (xold[samp_new] > xnew[i]) {
				samp_hi=samp_new;
			}
			if (xold[samp_new] <= xnew[i]) {
				samp_lo=samp_new;
			}
		}
		h=xold[samp_hi]-xold[samp_lo];
		a=(xold[samp_hi]-xnew[i])/h;
		b=(xnew[i]-xold[samp_lo])/h;
		if(xnew[i]<=max && xnew[i]>=min) {
			ynew[i]=a*y[samp_lo]+b*y[samp_hi]+((a*a*a-a)*y2d[samp_lo]+(b*b*b-b)*y2d[samp_hi])*(h*h)/6.;
		} 
	}
	if(xnew[0]==xold[0]) {
		ynew[0]=y[0];
	}
	
	free(y2d);
	free(u);
	return(ynew);
 }

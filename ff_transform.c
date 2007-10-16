#include "parser.h"

/** ff_image_resize - Resizes the dimensions of the image
 * using bilinear algorithm
 * betim@asu.edu
 **/
Var *
ff_image_resize(vfuncptr func, Var * arg){
	Var *data = NULL;	

	Var *out = NULL;
	float x_factor = 0,y_factor=0, factor = 0;
	int xx,yy,zz;
	int new_xx = 0, new_yy = 0, new_zz, new_size;

	int new_x, new_y, new_z;
	int interpl_x,interpl_y;
	int calc_x, calc_y;
	int pos;
	int preserve_ratio = 0;

	int p1x,p2x, p1y,p2y;

	double d_c11,d_c12,d_c21,d_c22, d_new_c1, d_new_c2, d_new_c;	
	int i_c11,i_c12,i_c21,i_c22, i_new_c1, i_new_c2, i_new_c;	
	int i,j,k;

	Alist alist[8];

	alist[0] = make_alist( "data", ID_VAL, NULL, &data);
	alist[1] = make_alist( "factor", FLOAT, NULL, &factor);
	alist[2] = make_alist( "xfactor", FLOAT, NULL, &x_factor);
	alist[3] = make_alist( "yfactor", FLOAT, NULL, &y_factor);
	alist[4] = make_alist( "width",  INT, NULL, &new_xx);
	alist[5] = make_alist( "height", INT, NULL, &new_yy);
	alist[6] = make_alist( "preserve_ratio", INT, NULL, &preserve_ratio);
	alist[7].name = NULL;
    
	if (parse_args(func, arg, alist) == 0) return(NULL);

	if(data == NULL){
		parse_error("No input specified: %s(...data=...)", func->name);
		return NULL;
	}
	if(new_xx != 0 && (x_factor != 0 || factor != 0)){
		parse_error("Argument conflict: width and factor conflict. Please select only one at a time\n");
		return NULL;
	}
	if(new_yy != 0 && (y_factor != 0 || factor != 0)){
		parse_error("Argument conflict: height and yfactor conflict. Please select only one at a time\n");
		return NULL;
	}
	if(factor != 0 && (x_factor !=0 || y_factor  != 0)){
		parse_error("Argument conflict: (factor) and (x or y factor). Please select only one at a time\n");
		return NULL;
	}
	
	/* x, y and z dimensions of the data */
	xx = GetX(data);
	yy = GetY(data);
	zz = GetZ(data);

	//If common factor for both x and y
	if(factor != 0){
	   x_factor = factor;
	   y_factor = factor;
	}else {
		// calculate x-factor
		if(new_xx != 0){
		   x_factor = new_xx/(float)xx;
		}else if(x_factor == 0){
			x_factor = 1; //default	
		}
		// calculate y-factor
		if(new_yy != 0){
		   y_factor = new_yy/(float)yy;
		}else if(y_factor == 0){
			y_factor = 1; //default	
		}
	}

	/* x, y and z dimensions of the new data */
	new_xx = (int)(xx*x_factor);
	new_yy = (int)(yy*y_factor);
	new_zz = zz;
	new_size = new_xx * new_yy * new_zz;

	//If ratio preservation is required, try to preserve it or die
	if(preserve_ratio == 1){
		if(x_factor !=1 && y_factor != 1){
		   if(new_xx/(float)new_yy != xx/(float)yy){
				parse_error("Argument conflict: Ratio could not be preserved\n");
				return NULL;
			}
	      }else{
			if(x_factor == 1){
				x_factor = y_factor;
			}else if(y_factor == 1){
				y_factor = x_factor;
			}
			/* x, y and z dimensions of the new data */
			new_xx = (int)(xx*x_factor);
			new_yy = (int)(yy*y_factor);
			new_zz = zz;
			new_size = new_xx * new_yy * new_zz;
		}		
	}

	/* create out array */
	out = newVar();
	V_TYPE(out) = ID_VAL;
	V_DSIZE(out) = new_size;
	V_ORG(out) = V_ORG(data);
	V_FORMAT(out) = V_FORMAT(data);
	V_SIZE(out)[orders[V_ORG(out)][0]] = new_xx;
	V_SIZE(out)[orders[V_ORG(out)][1]] = new_yy;
	V_SIZE(out)[orders[V_ORG(out)][2]] = new_zz;
	V_DATA(out) = calloc(new_size, NBYTES(V_FORMAT(data)));

	/** Bi-Linear interpolation **/
	/* z-axis */
	for(k=0; k < new_zz; k++){		
		/* y-axis */
		for(j=0; j < new_yy; j++){
			new_y = j/y_factor;
			interpl_y = new_y ^ 0; 		//bit xor
			calc_y = new_y - interpl_y;			
			/* x-axis */
			for(i=0; i< new_xx; i++){
				new_x = i/x_factor;	
				interpl_x = new_x ^ 0;	//bit xor
				calc_x = new_x - interpl_x;

				p1x = min(interpl_x + 1, xx-1);
				p2x = min(interpl_x + 2, xx-1);
				p1y = min(interpl_y + 1, yy-1);
				p2y = min(interpl_y + 2, yy-1);
				
				if(calc_x != 0 ||calc_y != 0){
				   parse_error("%d %d\n", calc_x, calc_y);	
				}
			     	/*  get the 4 pixels around the interpolated one */
				// Integer
				if(V_FORMAT(data) == BYTE || V_FORMAT(data) == SHORT || V_FORMAT(data) == INT){				   
					i_c11 = extract_int(data, cpos(p1x, p1y, k, data));
					i_c21 = extract_int(data, cpos(p2x, p1y, k, data));
					i_c12 = extract_int(data, cpos(p1x, p2y, k, data));
					i_c22 = extract_int(data, cpos(p2x, p2y, k, data));
		
					i_new_c1 = i_c11 * (1 - calc_y) + i_c12 * calc_y;
					i_new_c2 = i_c21 * (1 - calc_y) + i_c22 * calc_y;				
					i_new_c = i_new_c1 * (1 - calc_x) + i_new_c2 * calc_x;
				//Floats
				}else{
					d_c11 = extract_float(data, cpos(p1x, p1y, k, data));
					d_c21 = extract_float(data, cpos(p2x, p1y, k, data));
					d_c12 = extract_float(data, cpos(p1x, p2y, k, data));
					d_c22 = extract_float(data, cpos(p2x, p2y, k, data));
	
					d_new_c1 = d_c11 * (1 - calc_y) + d_c12 * calc_y;
					d_new_c2 = d_c21 * (1 - calc_y) + d_c22 * calc_y;				
					d_new_c = d_new_c1 * (1 - calc_x) + d_new_c2 * calc_x;
				}

				/*  Set this pixel into the new image */
				pos = cpos(i,j,k, out);
				switch(V_FORMAT(data)){
					case BYTE:	((u_char *)V_DATA(out))[pos] = i_new_c;	break;
					case SHORT:	((short *)V_DATA(out))[pos] = i_new_c;	break;
					case INT:	((int *)V_DATA(out))[pos] = i_new_c; 	break;
					case FLOAT:	((float *)V_DATA(out))[pos] = d_new_c;	break;
					case DOUBLE:	((double *)V_DATA(out))[pos]= d_new_c;	break;
				}
			}
		}
	}
	return(out);
}

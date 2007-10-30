#include "parser.h"

double get_image_bilinear_interp(Var *,double, double, double);
double  get_image_bicubic_interp(Var *,  double, double, double);

double get_image_box_average(Var *, double, double, double, double, double);
/** ff_image_resize - Resizes the dimensions of the image
 * using bilinear algorithm
 * betim@asu.edu
 **/
Var *
ff_image_resize(vfuncptr func, Var * arg){
	Var *data = NULL;	
	Var *out = NULL;

	double x_factor = 0,y_factor=0, factor = 0;

	//dimensions of the old image
	int xx,yy,zz; 
	//dimensions of the new image
	int new_xx = 0, new_yy = 0, new_zz, new_size;
	
	int pos;
	int lockratio = 0;
	int suspected_touched_ratio_x = 0;
	int suspected_touched_ratio_y = 0;

	double x,y,x1,y1,z; //interpolation indeces for the old image
	int i,j,k; 	//indeces of the new image

	int i_new_c;
	double d_new_c;

       char *usage = "usage: %s(data [, factor] [, xfactor] [, yfactor] [,width] [,height] [,lockratio={'0'|'1'}] [,type={'bilinear'|'bicubic'}]";
       char *type = NULL;
	int itype = 0;
       char *types[] = {"bilinear", "bicubic", NULL};


	Alist alist[9];
	alist[0] = make_alist( "data", ID_VAL, NULL, &data);
	alist[1] = make_alist( "factor", DOUBLE, NULL, &factor);
	alist[2] = make_alist( "xfactor", DOUBLE, NULL, &x_factor);
	alist[3] = make_alist( "yfactor", DOUBLE, NULL, &y_factor);
	alist[4] = make_alist( "width",  INT, NULL, &new_xx);
	alist[5] = make_alist( "height", INT, NULL, &new_yy);
	alist[6] = make_alist( "lockratio", INT, NULL, &lockratio);
	alist[7] = make_alist( "type",    ID_ENUM,    types,    &type);
	alist[8].name = NULL;
    
	if (parse_args(func, arg, alist) == 0) return(NULL);

	if(data == NULL){
		parse_error("No input specified: %s(...data=...)", func->name);
		parse_error(usage, func->name);
		return NULL;
	}
	if(new_xx != 0 && (x_factor != 0 || factor != 0)){
		parse_error("Argument conflict: width and factor conflict. Please select only one at a time\n");
		parse_error(usage, func->name);
		return NULL;
	}
	if(new_yy != 0 && (y_factor != 0 || factor != 0)){
		parse_error("Argument conflict: height and yfactor conflict. Please select only one at a time\n");
		parse_error(usage, func->name);
		return NULL;
	}
	if(factor != 0 && (x_factor !=0 || y_factor  != 0)){
		parse_error("Argument conflict: (factor) and (x or y factor). Please select only one at a time\n");
		parse_error(usage, func->name);
		return NULL;
	}

	if (type == NULL || strlen(type) == 0 || !strcasecmp(type, "bilinear")) {
		itype=0;
	} else if (!strncasecmp(type, "bicubic", 7)) {
		itype =1;
	} else {
		parse_error("%s: Unrecognized type: %s\n", func->name, type);
		parse_error(usage, func->name);
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
			x_factor = new_xx/(double)xx;
			suspected_touched_ratio_x = 1;
		}else if(x_factor == 0){
			x_factor = 1; //default
		}else{
		 	suspected_touched_ratio_x = 1;
		}
		// calculate y-factor
		if(new_yy != 0){
			y_factor = new_yy/(double)yy;
		 	suspected_touched_ratio_y = 1;
		}else if(y_factor == 0){
			y_factor = 1; //default	
		}else{
		 	suspected_touched_ratio_y = 1;
		}
	}

	/* x, y and z dimensions of the new data */
	new_xx = (int) my_round( (xx*x_factor));
	new_yy = (int) my_round(yy*y_factor);
	new_zz = zz;
	new_size = new_xx * new_yy * new_zz;

	//If ratio preservation is required, try to preserve it or die
	if(lockratio == 1){
	 	if(suspected_touched_ratio_x == 1 && suspected_touched_ratio_y == 1){
		      if(new_xx/(double)new_yy != xx/(double)yy){
				parse_error("Argument conflict: Ratio could not be preserved\n");
				parse_error(usage, func->name);
				return NULL;
		      }
		}
		if(x_factor == 1){
			x_factor = y_factor;
		}else if(y_factor == 1){
			y_factor = x_factor;
		}
		/* x, y and z dimensions of the new data */
		new_xx = (int)my_round(xx*x_factor);
		new_yy = (int)my_round(yy*y_factor);
		new_zz = zz;
		new_size = new_xx * new_yy * new_zz;
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
	if(V_DATA(out) == NULL){
		parse_error("Error: Could not allocate enough memory\n");
		return NULL;
	}



	/* Loop over the bands and do appropriate scaling  */

	/* z-axis */
	for(k=0; k < new_zz; k++){		
		z = k;
		/* y-axis */
		for(j=0; j < new_yy; j++){
			y = j/y_factor;
			/* x-axis */
			for(i=0; i< new_xx ; i++){
				x = i/x_factor;
				//scale down
				if(x_factor < 1 && y_factor < 1){						
					//Average in 0.5 pixel offset
					y1= (j+0.5)/y_factor;
					x1= (i+0.5)/x_factor;
					// Integers
					if(V_FORMAT(data) == BYTE || V_FORMAT(data) == SHORT || V_FORMAT(data) == INT){					   
					   i_new_c = (int)my_round(get_image_box_average(data, x,  y, x1, y1, z));
					//Doubles
					}else{
						d_new_c = get_image_box_average(data, x,  y, x1, y1, z);
					}
				//scale up
				}else{
					// Integers
					if(V_FORMAT(data) == BYTE || V_FORMAT(data) == SHORT || V_FORMAT(data) == INT){
						//bicubic
						if(itype == 1){
						   i_new_c = (int)my_round(get_image_bicubic_interp(data, x,  y, z));
						//bilinear
						}else{
						   i_new_c = (int)my_round(get_image_bilinear_interp(data, x,  y, z));	
						}
					}else{
						//bicubic
						if(itype == 1){
							d_new_c = get_image_bicubic_interp(data, x,  y, z);
						//bilinear
						}else{
							d_new_c = get_image_bilinear_interp(data, x,  y, z);
						}
					}
				}

				/*  Set this pixel into the new image */
				pos = cpos(i,j,k, out);


				switch(V_FORMAT(data)){
					case BYTE:	((u_char *)V_DATA(out))[pos] = saturate_byte(i_new_c);	break;
					case SHORT:	((short *)V_DATA(out))[pos]	 = saturate_short(i_new_c);	break;
					case INT:	((int *)V_DATA(out))[pos]	 = saturate_int(i_new_c); 	break;
					case FLOAT:	((float *)V_DATA(out))[pos]	 = saturate_float(d_new_c);	break;
					case DOUBLE:	((double *)V_DATA(out))[pos] = saturate_double(d_new_c);break;
				}
			}
		}
	}
	return(out);
}



/** Bi-Linear interpolation **/
/*  get the 4 pixels around the interpolated one */
double  get_image_bilinear_interp(Var *obj,  double x, double y, double z){
	double fraction_x, fraction_y;
	int xx = GetX(obj);
	int yy = GetY(obj);

	int y_floor = (int)floor(y);
	int y_ceil = y_floor + 1;
	
	int x_floor = (int)floor(x);
	int x_ceil = x_floor + 1;

	double d_c1,d_c2,d_c3,d_c4, d_new_cc1, d_new_cc2, d_new_c;  
	int i_c1,i_c2,i_c3,i_c4, i_new_cc1, i_new_cc2, i_new_c;	


	if(y_ceil >= yy){
		y_ceil = yy-1;
	}
	fraction_y = (y - y_floor);
		

	if(x_ceil >= xx){
		x_ceil = xx-1;
	}
	fraction_x = (x - x_floor);


     	/*  get the 4 pixels around the interpolated one */
	// Integers
	if(V_FORMAT(obj) == BYTE || V_FORMAT(obj) == SHORT || V_FORMAT(obj) == INT){				   
		i_c1 = extract_int(obj, cpos(x_floor, y_floor, z, obj));
		i_c2 = extract_int(obj, cpos(x_ceil,  y_floor, z, obj));
		i_c3 = extract_int(obj, cpos(x_floor, y_ceil, z, obj));
		i_c4 = extract_int(obj, cpos(x_ceil,  y_ceil, z, obj));
		//bilinear expressions	
		i_new_cc1 = i_c1 * (1 - fraction_y) + i_c2 * fraction_y;
		i_new_cc2 = i_c3 * (1 - fraction_y) + i_c4 * fraction_y;
		i_new_c = i_new_cc1 * (1 - fraction_x) + i_new_cc2 * fraction_x;
	
		return (double)i_new_c;
	//Floats
	}else{
		d_c1 = extract_double(obj, cpos(x_floor, y_floor, z, obj));
		d_c2 = extract_double(obj, cpos(x_ceil,  y_floor, z, obj));
		d_c3 = extract_double(obj, cpos(x_floor, y_ceil, z, obj));
		d_c4 = extract_double(obj, cpos(x_ceil,  y_ceil, z, obj));		
		//bilinear expressions	
    		d_new_cc1 = d_c1 * (1 - fraction_y) + d_c2 * fraction_y;
		d_new_cc2 = d_c3 * (1 - fraction_y) + d_c4 * fraction_y;
		d_new_c = d_new_cc1 * (1 - fraction_x) + d_new_cc2 * fraction_x;
		return (double)d_new_c;
	}
}


/* Used by get_image_bicubic_interp */
double get_my_cubic(double offset, double v0, double v1, double v2, double v3){
	// offset is the offset of the sampled value between v1 and v2
return   (((( -7 * v0 + 21 * v1 - 21 * v2 + 7 * v3 ) * offset +
	     ( 15 * v0 - 36 * v1 + 27 * v2 - 6 * v3 ) ) * offset +
	    ( -9 * v0 + 9 * v2 ) ) * offset + (v0 + 16 * v1 + v2) ) / 18.0;
}

/* Used by get_image_bicubic_interp */
double get_my_cubic_row(Var *obj, int x, int y, int z, double offset){
	double c0,c1,c2,c3;
	int xx = GetX(obj);
	int yy = GetY(obj);
	int x0 = x;
	int x1 = x+1;
	int x2 = x+2;
	int x3 = x+3;

	if(x0 >= xx){
	   x0 = xx-1;
	}else if(x0 < 0){
	   x0 = 0;
	}
	if(x1 >= xx){
	   x1 = xx-1;
	}else if(x1 < 0){
	   x1 = 0;
	}
	if(x2 >= xx){
	   x2 = xx-1;
	}else if(x2 < 0){
	   x2 = 0;
	}

	if(x3 >= xx){
	   x3 = xx-1;
	}else if(x3 < 0){
	   x3 = 0;
	}


	if(y >= yy){
	   y = yy-1;
	}else if(y <0){
	   y = 0;
	}


	// Integers
	if(V_FORMAT(obj) == BYTE || V_FORMAT(obj) == SHORT || V_FORMAT(obj) == INT){				   
		
		c0 = extract_int(obj, cpos(x0, y, z, obj));
		c1=  extract_int(obj, cpos(x1, y, z, obj)); 
		c2 = extract_int(obj, cpos(x2, y, z, obj));
		c3 = extract_int(obj, cpos(x3, y, z, obj));
	}else{
		c0 = extract_double(obj, cpos(x0, y, z, obj));
		c1 = extract_double(obj, cpos(x1, y, z, obj)); 
		c2 = extract_double(obj, cpos(x2, y, z, obj));
		c3 = extract_double(obj, cpos(x3, y, z, obj));
	}
	return get_my_cubic(offset,c0,c1,c2,c3);
}



/*
 * Bi-cubic interpolation 
 * based on http://pippin.gimp.org/image_processing/chap_resampling.html  
 */
double  get_image_bicubic_interp(Var *obj,  double x, double y, double z){
	double fraction_x, fraction_y;

	int yi = (int)floor(y);
	int xi = (int)floor(x);
	int zi = (int)z;

	double c,c0,c1,c2,c3;

	fraction_y = (y - yi);	      
	fraction_x = (x - xi);

	c0 = get_my_cubic_row(obj, xi-1, yi-1, z, fraction_x);
	c1 = get_my_cubic_row(obj, xi-1, yi,   z, fraction_x);
	c2 = get_my_cubic_row(obj, xi-1, yi+1, z, fraction_x);
	c3 = get_my_cubic_row(obj, xi-1, yi+2, z, fraction_x);
	
	
	c = get_my_cubic(fraction_y, c0,c1,c2,c3);
	
	return c;
}





/*
 * Takes an average value of the given area, and returns the average color value 
 * based on http://pippin.gimp.org/image_processing/chap_resampling.html  
 */
double get_image_box_average(Var *obj, double x0, double y0, double x1, double y1, double z){
	double area = 0;
	double sum = 0;

	int x,y,x_, y_,z_;
	int y0_floor;
	int y1_ceil;
	int x0_floor;
	int x1_ceil;
	double size;
	int i_c;
	double d_c;
	int xx = GetX(obj);
	int yy = GetY(obj);

	y0_floor = floor(y0);
	y1_ceil =  ceil(y1);	
	x0_floor = floor(x0);
	x1_ceil =  ceil(x1);
	
	z_ = z;

	for (y = y0_floor; y <= y1_ceil; y++)
	{
		size = 1.0;
		if( y < y0){
		   size = size * (1.0 - (y0 - y));
		}	
		if(y > y1){
			size = size * (1.0- (y - y1));
		}
		//In case it's out of bounds
		y_ = y;
		if(y_ >= yy){
		   y_ = yy-1;
		}
	
		for (x=x0_floor; x<= x1_ceil; x++){
			x_ = x;

			//In case it's out of bounds
			if(x_ >= xx){
			   x_ = xx-1;
			}

			// Integers
			if(V_FORMAT(obj) == BYTE || V_FORMAT(obj) == SHORT || V_FORMAT(obj) == INT){				   
				i_c = extract_int(obj, cpos(x_, y_, z_, obj));
				if( x < x0){
					size = size * (1.0 - (x0-x));
				}
				if ( x > x1){
					 size = size * (1.0 - (x-x1));
				}
				sum  += i_c*size;
				area += size;
			//Floats
			}else{
				d_c = extract_double(obj, cpos(x_,y_, z_, obj));
				if( x < x0){
					size = size * (1.0 - (x0-x));
				}
				if ( x > x1){
					 size = size * (1.0 - (x-x1));
				}
				sum  += d_c*size;
				area += size;
		      	}
		}
	}  
	return sum/area;
}


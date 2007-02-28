#include "parser.h"
#include "math.h"
#include "ff_modules.h"

static float *sstretch_sobel(float *data, float ignore, int x, int y, int z); 
static float *smooth(float *obj, float *kernel, int ox, int oy, int oz, int kx, int ky, int kz, int norm, float ignore);
static float *unslantc(float *data, int *leftedge, int width, int x, int y, int z, float ignore);
static float *unshearc(float *w_pic, float angle, int x, int y, int z, float nullv);
static float round_dp(float input, float decimal);

static Var *cse_contour(vfuncptr func, Var *arg);
static Var *cse_maxpos(vfuncptr func, Var *arg);
static Var *cse_minpos(vfuncptr func, Var *arg);
static Var *cse_cleandcs(vfuncptr func, Var *arg);
static Var *cse_rmnoise(vfuncptr func, Var *arg);
static Var *cse_find_shift(vfuncptr func, Var *arg);
static Var *cse_shift(vfuncptr func, Var *arg);
static Var *cse_unshift(vfuncptr func, Var *arg);
static Var *cse_tes_shift(vfuncptr func, Var *arg);
static Var *cse_sobel(vfuncptr func, Var *arg);
static Var *cse_circle(vfuncptr func, Var *arg);
static Var *cse_reconst(vfuncptr func, Var *arg);
static Var *cse_sstretch2(vfuncptr func, Var *arg);
static Var *cse_unscale(vfuncptr func, Var *arg);
static Var *cse_ramp(vfuncptr func, Var * arg);
static Var *cse_interp2d(vfuncptr func, Var * arg);
static Var *cse_columnator(vfuncptr func, Var * arg);

static dvModuleFuncDesc exported_list[] = {
  { "contour", (void *) cse_contour },
  { "maxpos", (void *) cse_maxpos },
  { "minpos", (void *) cse_minpos },
  { "cleandcs", (void *) cse_cleandcs },
  { "rmnoise", (void *) cse_rmnoise },
  { "find_shift", (void *) cse_find_shift },
  { "shift", (void *) cse_shift},
  { "unshift", (void *) cse_unshift},
  { "tes_shift", (void *) cse_tes_shift},
  { "sobel", (void *) cse_sobel},
  { "circle", (void *) cse_circle},
  { "reconst", (void *) cse_reconst},
  { "sstretch", (void *) cse_sstretch2},
  { "unscale", (void *) cse_unscale},
  { "ramp", (void *) cse_ramp},
  { "interp2d", (void *) cse_interp2d},
  { "columnator", (void *) cse_columnator}
};

static dvModuleInitStuff is = {
  exported_list, 17,
  NULL, 0
};

dv_module_init(
    const char *name,
    dvModuleInitStuff *init_stuff
    )
{
    *init_stuff = is;
    
    parse_error("Loaded module cse.");

    return 1; /* return initialization success */

}

void
dv_module_fini(const char *name)
{
  parse_error("Unloaded module cse.");
}



/*static programs*/

static float round_dp(float input, float decimal) {
  int      i = 0;
  double   val = 0.0;
  float    output = 0;
  float    mult = 0.0; 
  float    multiplier = 1.0;
  int      topval = 0;
  float    remainder = 0.0;

  // determine the multiplier value
  if (decimal > 1.0) {
    for (mult = 1.0; decimal >= 10.0; mult/=10){
      decimal/=10;
      multiplier/=10;
    }
  } else if (decimal < 1.0) {
    for (mult = 1.0; decimal < 1.0; mult*=10) {
      decimal*=10;
      multiplier*=10;
    }
  }

  val = (double)input;
  val*=multiplier;
  topval = (int)val;
  remainder = val - topval;
  if(remainder > 0.5) topval = topval + 1;
  if(remainder < -0.5) topval = topval - 1;
  output = (float)topval/multiplier;
  
  return(output);
}



static float *sstretch_sobel(float *data, float ignore, int x, int y, int z)
{

  /* takes data and stretches it similar to sstretch() davinci function */
  /* but keeps data in float space */

  double     sum = 0;           /* sum of elements in data */
  double     sumsq = 0;         /* sum of the square of elements in data */
  double     stdv = 0;          /* standard deviation */
  int        cnt = 0;           /* total number of non-null points */
  float     *out = NULL;        /* stretched data to be returned */
  int        i,j,k;
  int        tv;

  /* create out array */
  out = (float *)calloc(sizeof(float), x*y*z);

  /* stretch each band separately */
  for(k=0;k<z;k++) {
    sum = 0;
    sumsq = 0;
    stdv = 0;
    cnt = 0;
    tv = 0;

    /* calculate the sum and the sum of squares */
    for(j=0; j<y; j++) {
      for(i=0; i<x; i++) {
	if(data[k*y*x + j*x + i] != ignore){
	  sum += (double)data[k*y*x + j*x + i];
	  sumsq += (double)(data[k*y*x + j*x + i]*data[k*y*x + j*x + i]);
	  cnt += 1;
	}
      }
    }

    /* calculate standard deviation */
    stdv = sqrt((sumsq - (sum*sum/cnt))/(cnt-1));
    sum /= (double)cnt;
    
    /* fill in stretched values */
    for(j=0; j<y; j++) {
      for(i=0; i<x; i++) {
	if(data[k*y*x + j*x + i] != ignore){
	  out[k*y*x + j*x + i] = (data[k*y*x + j*x + i] - sum)*(40/stdv)+127;
	}
      }
    }
  }

  return(out);
}



static float *smooth(float *obj, float *kernel, int ox, int oy, int oz, int kx, int ky, int kz, int norm, float ignore)
{

  float   *data;                /* the smoothed array */
  float   *wt;                  /* array of number of elements in sum */
  int      a, b, c;             /* x, y, z position of kernel element */
  int      x_pos, y_pos, z_pos; /* x, y, z position of object element */
  int      kx_center;           /* x radius and total x dimension of kernel */
  int      ky_center;           /* y radius and total y dimension of kernel */
  int      kz_center;           /* z radius and total z dimension of kernel */
  int      x, y, z;             /* x, y, z pos of object convolved element */
  int      i, j, k;             /* memory locations */
  float    kval, oval;          /* value of kernel and object element */
  int      objsize;             /* total 1-d size of object */

  kx_center = kx/2;
  ky_center = ky/2;
  kz_center = kz/2;
  
  objsize = ox * oy * oz;

  data = (float *)calloc(sizeof(float), objsize);
  wt = (float *)calloc(sizeof(float), objsize);

  if (data == NULL || wt == NULL) {
    parse_error("Unable to allocate memory");
    return NULL ;
  }
 
  for (i = 0 ; i < objsize ; i++) {
    if(obj[i] == ignore) { data[i] = ignore; continue; }
    z = i/(ox*oy);                 
    y = (i - z*ox*oy)/ox ;
    x =  i - z*ox*oy - y*ox;
    for (a = 0 ; a < kx ; a++) {
      x_pos = x + a - kx_center;   
      if (x_pos < 0 || x_pos >= ox) continue;
      for (b = 0 ; b < ky ; b++) {    
	y_pos = y + b - ky_center; 
	if (y_pos < 0 || y_pos >= oy) continue;
	for (c = 0 ; c < kz ; c++) {
	  z_pos = z + c - kz_center;
	  if (z_pos < 0 || z_pos >= oz) continue;
	  j = z_pos*ox*oy + y_pos*ox + x_pos;
	  k = c*kx*ky + b*kx + a;
	  kval = kernel[k];
	  oval = obj[j];
	  if (oval != ignore && kval != ignore) {
	    wt[i] += kval;                   
	    data[i] += (kval * oval);
	  }
	} 
      }
    }
    if (norm != 0 && wt[i] != 0) {
      data[i] /= (float)wt[i];
    }
  } 
  free(wt);
  return(data);
}



static float *unslantc(float *data, int *leftedge, int width, int x, int y, int z, float ignore)
{
  /* set up variables */
  int i,j,k,p;
  float *odata=NULL;
  
  /* allocate memory for the new data */
  odata = calloc(width*y*z, sizeof(float));
  
  /* extract the data and shift it */
  for (j=0;j<y;j++) {
    for (i=0;i<width;i++) {
      for (k=0;k<z;k++) {
 	if (i >= leftedge[j] && i < leftedge[j]+x) {
	  odata[width*y*k + width*j +i] = data[x*y*k + x*j + (i-leftedge[j])];
	} else {
	  odata[width*y*k + width*j +i] = ignore;
	}
      }
    }
  } 
  
  free(data);
  /* return the data */
  return(odata);
}



static float *unshearc(float *w_pic, float angle, int x, int y, int z, float nullv)
{

  float   *pic=NULL;
  float    shift;                   /* the shift/pixel */
  int      lshift;                  /* the largest shift (int) */
  int      i, j, k;                 /* loop indices */
  int      nx, ni, nj;              /* memory locations */
  Var     *out=NULL;

  /* calculating the number of rows to add to the picture to accomodate the shear */
  shift = tan((M_PI / 180.0) * angle);
  lshift = (int)((x*fabs(shift)-0.5)+1)*(shift/fabs(shift));
  
   /*create the new picture array */
  pic = (float *)calloc(sizeof(float), x*z*(y-abs(lshift)));
  
  if (pic == NULL) {
    parse_error("ERROR! Unable to allocate %d bytes\n", sizeof(float)*x*z*(y+abs(lshift)));
    return NULL;
  }
  
  /* fill in the null value into all pic elements */
  if(nullv != 0) {
    for(k=0; k<z; k++) {
      for(j=0; j<y-abs(lshift); j++) {
	for(i=0; i<x; i++) {
	  ni = k*(y-abs(lshift))*x + j*x + i;
	  pic[ni] = nullv;
	}
      }
    }
  }
  
  /* extract the picture directly into a sheared, but trimmed, array */
  for(k=0; k<z; k++) {
    for(j=0; j<y-abs(lshift); j++) {
      for(i=0; i<x; i++) {
	
	/* the index of the new array */
	nx = k*(y-abs(lshift))*x + j*x + i;
	
	/* the shifted y pixel */
	/* if the angle is greater than 0 */
	if(lshift>0) {
	  nj = j + (int)(fabs(shift)*i+0.5);
	}
	
	/* if the angle is less than 0 */
	if(lshift<0) {
	  nj = j + abs(lshift) - (int)(fabs(shift)*i+0.5);
	}
	pic[nx] = w_pic[x*y*k + nj*x + i];
      }
    }
  }
  free(w_pic);
  return(pic);
}






/*davinci cse_funtions*/

Var*
cse_contour(vfuncptr func, Var *arg)
{
  
  Var    *data = NULL;                      /* the orignial data */
  Var    *out = NULL;                       /* the output structure */
  float   bin = 10;                         /* contour interval */
  float  *w_data;                           /* contoured image */
  int     i, j, k;                          /* loop indices */
  int     x=0, y=0, z=0;                    /* size of the data */ 
  float   tv=0,tv2=0;                       /* temporary value */
  int     startx=0, starty=0;               /* start x and y vals */
  int     endx=0, endy=0;                   /* end x and y vals */
  int     i1=0,j1=0;                        /* small loop indices */
  int     fill=0;                           /* pixel fill check */
  float   zero=0;                           /* zero point */

  Alist alist[4];
  alist[0] = make_alist("data",           ID_VAL,    NULL,  &data);
  alist[1] = make_alist("interval",       FLOAT,     NULL,  &bin);
  alist[2] = make_alist("zero",           FLOAT,     NULL,  &zero);
  alist[3].name = NULL;
  
  if (parse_args(func, arg, alist) == 0) return(NULL);
  if (data == NULL) { 
    parse_error("\ncontour() - Thu Aug 10 14:14:13 MST 2006");
    parse_error("Create contour map");
    parse_error("\nInputs and Outputs:");
    parse_error("data - input data array");
    parse_error("interval - contour interval (Default = 10)");
    parse_error("zero - zero level (or \"sea level\") of data (Default = 0)");
    parse_error("\nreturns an array of contours of the same dimensions as the");
    parse_error("input with contours of a multiplicative of the interval value\n") ;
    parse_error("c.edwards\n");
    return (NULL);
  }
  
  /* x, y and z dimensions of the data */
  x = GetX(data);
  y = GetY(data);
  z = GetZ(data);
  
  /* allocate memory for the picture */
  w_data = (float *)calloc(sizeof(float), x*y*z);
  
  /* loop through data and extract points and fill in contours*/
  for(i=0; i<x; i++) {
    for(j=0; j<y; j++) {
      for(k=0; k<z; k++) {

	/* extract temporary binned value */
	tv=(round_dp((extract_float(data, cpos(i,j,k,data))-zero)/bin,1))*bin;

	/* set up small loop start and end points */
	startx=i-1;
	endx=i+1;
	starty=j-1;
	endy=j+1;

	/* edge protection */
	if(startx<0) startx=0;
	if(endx>=x) endx=x-1;
	if(starty<0) starty=0;
	if(endy>=y) endy=y-1;
	
	/* small loop indices and fill flag */
	i1=startx;
	j1=starty;
	fill=0;
	
	/* dooo the looop */
	while(fill==0 && i1<=endx) {
	  while(fill==0 && j1<=endy) {
	    /* make sure not to check comparison value */
	    if(i1!=i && j1!=j) {
	      /*perform the check and fill in data when it is triggered */
	      if(tv<(round_dp((extract_float(data, cpos(i1,j1,k,data))-zero)/bin,1))*bin) {
		w_data[x*y*k + x*j + i]=tv;
		fill=1;
	      }
	    }
	    j1+=1;
	  }
	  j1=starty;
	  i1+=1;
	}
      }
    }
  }
  /* return the contoured data */
  out=newVal(BSQ,x,y,z,FLOAT,w_data);
  return out;
}


Var*
cse_maxpos(vfuncptr func, Var *arg)
{
 
  Var    *data = NULL;                      /* the orignial data */
  Var    *out = NULL;                       /* the output structure */
  int    *pos = NULL;                       /* the output position */
  float   ignore = -32768;                  /* null value */
  float  *val = NULL;                       /* the output values */
  int     i, j, k, l;                       /* loop indices */
  int     x=0, y=0, z=0;                    /* size of the data */
  float   ni1=0, ni2=-32798, ni3=-852;      /* temp values and positions */
  int     opt=0;                            /* return array option */
  int     iter=1;                           /* number of iterations to include */
   
  Alist alist[5];
  alist[0] = make_alist("data",      ID_VAL,    NULL,  &data);
  alist[1] = make_alist("ret",       INT,       NULL,  &opt);
  alist[2] = make_alist("iter",      INT,       NULL,  &iter);
  alist[3] = make_alist("ignore",    FLOAT,     NULL,  &ignore);
  alist[4].name = NULL;
  
  if (parse_args(func, arg, alist) == 0) return(NULL);
  
  if (data == NULL) {
    parse_error("\nUsed to find the position of the max point in an array\n");
    parse_error("$1 = the data");
    parse_error("Prints the value and location\n");
    parse_error("ret=1 returns the value and location in a structure\n");
    parse_error("iter= # of points to find in the data");
    parse_error("if iter >1 it will automatically return the points\n");
    parse_error("ignore = the value to ignore\n");
    parse_error("c.edwards 5/19/04\n");
    return NULL;
  }

  if (iter > 1 ) {
    opt=1;
  }

  /* create array for postion */
  pos = (int *)calloc(sizeof(int), 3*iter);
  val = (float *)calloc(sizeof(float), iter);
  
  /* x, y and z dimensions of the data */
  x = GetX(data);
  y = GetY(data);
  z = GetZ(data);

  /* find the maximum point and its position */  
  for(l=0; l<iter; l++) {
    for(k=0; k<z; k++) {
      for(j=0; j<y; j++) {
	for(i=0; i<x; i++) {
	  
	  /* ni1 = current value */
	  /* ni2 = curent max value */
	  /* ni3 = old max val */
	  ni1 = extract_float(data, cpos(i,j,k,data));
	  if(ni1>ni2 && (ni3 == -852 || (ni1<ni3 && ni3 != -852)) && ni1 != ignore) {
	    ni2=ni1;
	    pos[3*l+2]=k+1;
	    pos[3*l+1]=j+1;
	    pos[3*l+0]=i+1;
	  }
	}
      }
    }
    ni3=ni2;
    val[l]=ni2;
    ni2=-32798;
  }

  if(iter==1) {
    /* return the findings */
    printf("\nLocation: %i,%i,%i\n",pos[0],pos[1],pos[2]);
    printf("%.13f\n\n",ni3);
    
    if(opt==0) return NULL;
  }

  if(opt!=0) {
    out = new_struct(2);
    add_struct(out,"pos",newVal(BSQ, 3, iter, 1, INT, pos));
    add_struct(out,"val",newVal(BSQ, 1, iter, 1, FLOAT, val));
    return out;
  }
}



Var*
cse_minpos(vfuncptr func, Var *arg)
{

  Var    *data = NULL;                 /* the orignial data */
  Var    *out = NULL;                  /* the output struture */
  int    *pos = NULL;                  /* the output postion */
  float   ignore = -32768;             /* null value */
  float  *val = NULL;                  /* the output values */
  int     i, j, k, l;                  /* loop indices */
  int     x=0, y=0, z=0;               /* size of the data */
  float   ni1=0, ni2=32798, ni3=852;   /* temp values and positions */
  int     opt=0;                       /* return array option */
  int     iter=1;                      /* number of iterations to include */

  Alist alist[5];
  alist[0] = make_alist("data",      ID_VAL,    NULL,  &data);
  alist[1] = make_alist("ret",       INT,       NULL,  &opt);
  alist[2] = make_alist("iter",      INT,       NULL,  &iter);
  alist[3] = make_alist("ignore",    FLOAT,     NULL,  &ignore);
  alist[4].name = NULL;
  
  if (parse_args(func, arg, alist) == 0) return(NULL);
 
  if (data == NULL) {
    parse_error("\nUsed to find the position of the min point in an array\n");
    parse_error("$1 = the data");
    parse_error("Prints the value and location\n");
    parse_error("ret = 1 returns the value and location in a structure\n");
    parse_error("iter = # of points to find in the data");
    parse_error("if iter >1 it will automatically return the points\n");
    parse_error("ignore = the value to ignore\n");
    parse_error("c.edwards 5/19/04\n");
    return NULL;
  }

  if (iter > 1 ) {
    opt=1;
  }
  
  /* create array for position and value*/
  pos = (int *)calloc(sizeof(int), 3*iter);
  val = (float *)calloc(sizeof(float), iter);
  
  /* x, y and z dimensions of the data */
  x = GetX(data);
  y = GetY(data);
  z = GetZ(data);

  /* find the minimum point and its position */  
  for(l=0; l<iter; l++) {
    for(k=0; k<z; k++) {
      for(j=0; j<y; j++) {
	for(i=0; i<x; i++) {
	  
	  /* ni1 = current value */
	  /* ni2 = curent max value */
	  /* ni3 = old max val */
	  ni1 = extract_float(data, cpos(i,j,k,data));
	  if(ni1<ni2 && (ni3 == 852 || (ni1>ni3 && ni3 != 852)) && ni1 != ignore) {
	    ni2=ni1;
	    pos[3*l+2]=k+1;
	    pos[3*l+1]=j+1;
	    pos[3*l+0]=i+1;
	  }
	}
      }
    }
    ni3=ni2;
    val[l]=ni2;
    ni2=32798;
  }

  /* return the findings */
  if(iter==1) {    
    printf("\nLocation: %i,%i,%i\n",pos[0],pos[1],pos[2]);
    if(ni2<-32768) {
      printf("%.9e\n\n",ni3);
    }
    if(ni2>=-32768) {
      printf("%.9f\n\n",ni3);
    }
    if(opt==0) return NULL;
  }

  if(opt!=0) {
    out = new_struct(2);
    add_struct(out,"pos",newVal(BSQ, 3, iter, 1, INT, pos));
    add_struct(out,"val",newVal(BSQ, 1, iter, 1, FLOAT, val));
    return out;
  }
}



Var *
cse_cleandcs(vfuncptr func, Var * arg)
{

  typedef unsigned char byte;
  
  Var    *pic = NULL;            /* the orignial dcs pic */
  Var    *out = NULL;            /* the output pic */
  byte   *w_pic;                 /* modified image */
  int     i, j, k;               /* loop indices */
  int     x=0, y=0, z=0;         /* size of the picture */
  byte    tv=0;                  /* temp pixel value */
  int     opt=2;                 /* option for possible clean */
  
  Alist alist[3];
  alist[0] = make_alist("pic",      ID_VAL,     NULL,  &pic);
  alist[1] = make_alist("opt",      INT,        NULL,  &opt);
  alist[2].name = NULL;
  
  if (parse_args(func, arg, alist) == 0) return(NULL);
  if ( opt < 1 || opt > 2) opt=2;

  if (pic == NULL) {
    parse_error("\nUsed to remove extraneous color from a dcs image\n");
    parse_error("$1 = the dcs picture to clean");
    parse_error("opt = 1 more clean but less data is saved");
    parse_error("opt = 2 less clean but more data is saved(Default)\n");
    parse_error("c.edwards 6/14/04\n");
    return NULL;
  }

  /* x, y and z dimensions of the data */
  x = GetX(pic);
  y = GetY(pic);
  z = GetZ(pic);
  
  /* allocate memory for the picture */
  w_pic = (byte *)calloc(sizeof(byte), x*y*z);
  
  if(w_pic == NULL) return NULL;

  /* loop through data and extract new points with null value of 0 ignored */
  for(j=0; j<y; j++) {
    for(i=0; i<x; i++) {
      for(k=0; k<z; k++) {
	tv = (byte)extract_int(pic, cpos(i,j,k, pic));
    	w_pic[j*x*z + i*z + k] = tv;
	if (k==z-1) {
	  if((w_pic[j*x*z + i*z + k] == 0) + (w_pic[j*x*z + i*z + (k-1)] == 0) + (w_pic[j*x*z + i*z + (k-2)] == 0) >= opt ) {
	    w_pic[j*x*z + i*z + k]=0;
	    w_pic[j*x*z + i*z + (k-1)]=0;
	    w_pic[j*x*z + i*z + (k-2)]=0;
	  } 
	}
      }
    }
  }
  
  /* return the modified data */
  out = newVal(BIP, z, x, y, BYTE, w_pic);
  return out;
}



Var*
cse_rmnoise(vfuncptr func, Var *arg)
{
  typedef unsigned char byte;

  byte   *bc = NULL;                /* band count for orignal stuff */        
  Var    *data = NULL;              /* the original data */
  Var    *out = NULL;               /* the output structure */
  float   nullval=-32768;           /* null value */  
  float  *w_pic, *w_pic2;           /* working data */
  float   tv=0;                     /* temporary value */
  float   *total;                   /* the sum of the data (luminosity) */
  int     i, j, k;                  /* loop indices */
  int     x=0, y=0, z=0;            /* size of the data */
  float  *kern;                     /* kernel for the convolve */
  int     b10=10;                   /* band option */
  float  *band_10;                  /* band 10 to smooth */
  int     filt=7;                   /* the filter size */
  int     kernsize;                 /* number of kern elements */ 
  int     vb=1;                     /* verbosity for updataes */

  Alist alist[7]; 
  alist[0] = make_alist("data",      ID_VAL,    NULL,  &data);
  alist[1] = make_alist("null",      FLOAT,     NULL,  &nullval);
  alist[2] = make_alist("b10",       INT,       NULL,  &b10);
  alist[3] = make_alist("filt",      INT,       NULL,  &filt);
  alist[4] = make_alist("verbose",   INT,       NULL,  &vb);
  alist[5] = make_alist("ignore",    FLOAT,     NULL,  &nullval);
  alist[6].name = NULL;

  if (parse_args(func, arg, alist) == 0) return(NULL);
 
  if (data == NULL) {
    parse_error("\nUsed to remove white noise from data");
    parse_error("More bands are better\n");
    parse_error("$1 = the data to remove the noise from");
    parse_error("ignore = null value for the image (Default is -32768)");
    parse_error("b10 = the atmospheric band (Default is 10)");
    parse_error("If b10=0, then there is no band 10 information");
    parse_error("filt = the filter size to use (Default is 7)\n");
    parse_error("NOTE: This WILL CHANGE the data and");
    parse_error("should only be used for \"pretty pictures\"");
    parse_error("Band 10 will be smoothed by convolution\n");
    parse_error("Last Modified by c.edwards 8/10/04");
    parse_error("Added filt and band 10 smooth\n");
    parse_error("There is yet another option to clean images");
    parse_error("You must source \"/u/cedwards/christopher.dvrc\"");
    parse_error("The function is rmnoise_pca()");
    parse_error("It has more limitations but retains more color information");
    parse_error("Type \"rmnoise_pca()\" for help\n");

    return NULL;
  }

  /* set the band number for c */
  if(b10 != 0 ) {
    b10-=1;
    if(b10<0 || b10>9) b10=9;
  }

  /* x, y, and z dimensions of the data  */
  x = GetX(data);
  y = GetY(data);
  z = GetZ(data);
  kernsize=filt*filt;
  
  /* more error handling */
  if(z < 2) {
    printf("\nSorry you need more bands to remove noise\n\n");
    return NULL;
  }
  if(vb==1) printf("\nMORE UPDATES! CHECK THE HELP!\n");

  /* allocate memory for the data */
  bc = (byte *)calloc(sizeof(byte), x*y);
  w_pic = (float *)calloc(sizeof(float), x*y*z);
  w_pic2 = (float *)calloc(sizeof(float), x*y*z);
  total = (float *)calloc(sizeof(float), x*y);
  kern = (float *)calloc(sizeof(float), kernsize);
  band_10 = (float *)calloc(sizeof(float),x*y);  

  if(w_pic == NULL) return NULL;

  /* if there is a band 10 */
  if(b10 != 0 ) {
    /* extract values and make luminosity map */
    for(k=0; k<z; k++) {
      for(j=0; j<y; j++) {
	for(i=0; i<x; i++) {
	  tv = extract_float(data, cpos(i,j,k, data));
	  if(k!=b10) {
	    total[x*j + i]+=tv;
	    if(tv!=nullval) bc[x*j + i]+=1;
	  }
	  if(k==(x-1)) band_10[x*j + i]=tv;
	  w_pic[x*y*k + x*j + i]=tv;
	}
      }
    }
    
    /* divide by the luminosity map and check for null values */
    for(k=0; k<z; k++) {
      if(k!=b10) {
	for(j=0; j<y; j++) {
	  for(i=0; i<x; i++) {
	    if(bc[x*j + i] == b10) w_pic2[x*y*k + x*j + i]=w_pic[x*y*k + x*j + i]/total[x*j + i];  
	  }
	}
      }
    }
    
    /* convolve over the image */
    for(i=0;i<kernsize;i++) {
      kern[i]=1.0;
    }
    w_pic2=smooth(w_pic2,kern,x,y,z,filt,filt,1,1,0);
    band_10=smooth(band_10,kern,x,y,1,filt,filt,1,1,0);
    
  /* convert back to normal space and fill other pixels */
    for(k=0; k<z; k++) {
      for(j=0; j<y; j++) {
	for(i=0; i<x; i++) {
	  if(k!=b10) {
	    if(bc[x*j + i] == b10) w_pic2[x*y*k + x*j + i]*=total[x*j + i];
	    if(bc[x*j + i] < b10) w_pic2[x*y*k + x*j + i]=w_pic[x*y*k + x*j + i];
	  }
	  if(k==b10) w_pic2[x*y*k + x*j + i]=band_10[x*j + i];
	}
      }
    }
  }
  
  /*if there is not a band 10 */
  if(b10 == 0 ) {
    /* extract values and make luminosity map */
    for(k=0; k<z; k++) {
      for(j=0; j<y; j++) {
	for(i=0; i<x; i++) {
	  tv = extract_float(data, cpos(i,j,k, data));
	  total[x*j + i]+=tv;
	  w_pic[x*y*k + x*j + i]=tv;
	}
      }
    }
    for(k=0; k<z; k++) {
      for(j=0; j<y; j++) {
	for(i=0; i<x; i++) {
	  w_pic2[x*y*k + x*j + i]=w_pic[x*y*k + x*j + i]/total[x*j + i];  
	}
      }
    }
  
    /* convolve over the image */
    for(i=0;i<kernsize;i++) {
      kern[i]=1.0;
    }
    w_pic2=smooth(w_pic2,kern,x,y,z,filt,filt,1,1,0);
    
    /* convert back to normal space and fill other pixels */
    for(k=0; k<z; k++) {
      for(j=0; j<y; j++) {
	for(i=0; i<x; i++) {
	  w_pic2[x*y*k + x*j + i]*=total[x*j + i];
	  if(w_pic[x*y*k + x*j + i]==nullval) w_pic2[x*y*k + x*j + i]=nullval;
	}
      }
    }
  }

  /*clean up and return the image */
  free(w_pic);
  free(total);
  free(kern);
  free(band_10);
  free(bc);

  out=newVal(BSQ,x,y,z,FLOAT,w_pic2);
  return out;
}



Var *
cse_find_shift(vfuncptr func, Var * arg)
{

  Var     *data=NULL;               /* misslanted data */
  Var     *out=NULL;                /* output data */
  int      i, j;                    /* loop indices */
  int      x=0, y=0, z=0;           /* size of the picture */
  float    tvright=0;               /* temporary pixel value */
  float    tvleft=0;                /* temporary pixel value */
  float    nullval=-32768;          /* null value */

  Alist alist[4];
  alist[0] = make_alist("data",      ID_VAL,     NULL,  &data);
  alist[1] = make_alist("null",      FLOAT,      NULL,  &nullval);
  alist[2] = make_alist("ignore",    FLOAT,      NULL,  &nullval);
  alist[3].name = NULL;

  if (parse_args(func, arg, alist) == 0) return(NULL);

  if(data == NULL) {
    parse_error("\nUsed to find the side of the mosaic that contains data");
    parse_error("Which is then used to shift/unshift data for deplaiding \n");
    parse_error("$1 =  the data (should be part of a davinci mosaic)\n");
    parse_error("ignore = the null value of the image (default -32768)");
    parse_error("Companion functions are shift/unshift\n");
    parse_error("c.edwards 6/4/04\n");
    return NULL;
  }

  /* x and y dimensions of the data */
  x = GetX(data);
  y = GetY(data);
  
  /* if the right side has data values */
  for(j=0;j<y;j++){
    tvright = extract_float(data,cpos((x-1),j,0,data));
    if(tvright > nullval) {
      out = newInt(1);
      return out;
    }
  }
  /* if the left side has data values */
  for(j=0;j<y;j++) {
    tvleft = extract_float(data,cpos(0,j,0,data));
    if(tvleft > nullval) {
      out = newInt(2);
      return out;
    }
  }
  if(tvleft == nullval && tvright == nullval) {
    out = newInt(0);
    return out;
  }
}



Var *
cse_shift(vfuncptr func, Var * arg)
{

  Var     *data=NULL;               /* misslanted data */
  Var     *out=NULL;                /* output data */
  float   *w_data=NULL;             /* working data */
  int      i, j, k;                 /* loop indices */
  int      x=0, y=0, z=0;           /* size of the picture */
  float    tv=0;                    /* temporary pixel value */
  float    nullval=-32768;          /* null value */
  int     *newshift=NULL;           /* new left edge */ 
  int      opt;                     /* the way to shift pixels */

  Alist alist[5];
  alist[0] = make_alist("data",      ID_VAL,     NULL,  &data);
  alist[1] = make_alist("null",      FLOAT,      NULL,  &nullval);
  alist[2] = make_alist("side",      INT,        NULL,  &opt);
  alist[3] = make_alist("ignore",    FLOAT,      NULL,  &nullval);
  alist[4].name = NULL;

  if (parse_args(func, arg, alist) == 0) return(NULL);

  if(data == NULL) {
    parse_error("\nUsed to shift pixels for mosaics for deplaiding\n");
    parse_error("$1 = the data to shift (must be rectified \n");
    parse_error("ignore = the null value of the image (default -32768)");
    parse_error("side = 1 data is on right and shift will be left");
    parse_error("side = 2 data is on left and shift will be right");
    parse_error("side = 0 no shift (returns NULL)");
    parse_error("Companion function is unshift\n");
    parse_error("c.edwards 6/14/04\n");
    return NULL;
  }

  /* error handling */
  if(opt > 2 || opt < 0 ) {
    parse_error("Enter either 1 for right and 2 for left");
    return NULL;
  }
  if(opt==0) return NULL;

  /* x, y and z dimensions of the data and allocate memory */
  x = GetX(data);
  y = GetY(data);
  z = GetZ(data);
  w_data=(float *)calloc(sizeof(float),x*y*z);  
  newshift=(int *)calloc(sizeof(int),y);

  /* shift pixels to the left / data is on the right */
  if(opt==1) {
    /* determine the shift for each row */
    for(j=0;j<y;j++) {
      for(i=0;i<x;i++) {
	tv = extract_float(data,cpos(i,j,0,data));
	if(tv != nullval) {
	  newshift[j]=i;
	  i=x;
	}
	if(i==(x-1)) newshift[j]=x-1;
      }
    }
    /*extract and shift the pixels */
    for(k=0;k<z;k++) {
      for(j=0;j<y;j++) {
	for(i=0;i<x;i++) {
	  if(newshift[j] == (x-1)) {
	    w_data[x*y*k + x*j + i]=nullval;
	  }
	  if(newshift[j] == 0) {
	    w_data[x*y*k + x*j + i]=extract_float(data,cpos(i,j,k,data));
	  }
	  if(newshift[j] != 0 && newshift[j] != (x-1)) {
	    if((newshift[j]+i) < x) {
	      w_data[x*y*k + x*j + i]=extract_float(data,cpos(i+newshift[j],j,k,data));
	    }
	    if((newshift[j]+i) >= x) {
	      w_data[x*y*k + x*j + i]=nullval;
	    }
	  }
	}  
      }
    }
  }
  
  /* shift pixels to the right / data is on the left */
  if(opt==2) {
    /* determine the shift for each row */
    for(j=0;j<y;j++) {
      for(i=x-1;i>=0;i--) {
	tv = extract_float(data,cpos(i,j,0,data));
	if(tv != nullval) {
	  newshift[j]=i;
	  i=0;
	}
	if(i == 0 && tv == nullval) newshift[j]=0;
      }
    }
    /*extract and shift the pixels */
    for(k=0;k<z;k++) {
      for(j=0;j<y;j++) {
	for(i=0;i<x;i++) {
	  tv=extract_float(data,cpos(i,j,k,data));
	  if(newshift[j] == 0){
	    w_data[x*y*k + x*j + i]=nullval;
	  }
	  if(newshift[j] == (x-1)) {
	    w_data[x*y*k + x*j + i ]=extract_float(data,cpos(i,j,k,data));
	  }
	  if(newshift[j] != 0 && newshift[j] != (x-1)){
	    if(i >= x-newshift[j]-1) {
	      w_data[x*y*k + x*j + i]=extract_float(data,cpos(i-x+newshift[j]+1,j,k,data));
	    }
	    if(i < x-newshift[j]-1) {
	      w_data[x*y*k + x*j + i]=nullval;
	    }
	  }
	}
      }
    }
  }
  
  /* return and clean up */
  out=new_struct(2);
  add_struct(out,"data",newVal(BSQ,x,y,z,FLOAT,w_data));
  add_struct(out,"shift_edge",newVal(BSQ,1,y,1,INT,newshift));
  return out;
}



Var *
cse_unshift(vfuncptr func, Var * arg)
{
  
  Var     *rect=NULL;               /* structure that contains left_shift */
  Var     *data=NULL;               /* misslanted data */
  Var     *out=NULL;                /* output data */
  Var     *newshiftt=NULL;          /* new left pointer */
  float   *w_data;                  /* working data */
  int      i, j, k;                 /* loop indices */
  int      x=0, y=0, z=0;           /* size of the picture */
  float    tv=0;                    /* temp pixel value */
  float    nullval=-32768;          /* null value */ 
  int     *newshift=NULL;           /* new edge */ 
  int      opt;                     /* the way to unshift pixels */
 
  Alist alist[5];
  alist[0] = make_alist("rect",      ID_STRUCT,  NULL,  &rect);
  alist[1] = make_alist("null",      FLOAT,      NULL,  &nullval);
  alist[2] = make_alist("side",      INT,        NULL,  &opt);
  alist[3] = make_alist("ignore",    FLOAT,      NULL,  &nullval);
  alist[4].name = NULL;
  
  if (parse_args(func, arg, alist) == 0) return(NULL);
 
  if(rect == NULL) {
    parse_error("\nUsed to unshift pixels for mosaics for deplaiding\n");
    parse_error("$1 = the data to unshift (must be shift"); 
    parse_error("structure w/ \"shift_edge\" and \"data\")\n");
    parse_error("ignore = the null value of the image (default -32768)");
    parse_error("side = 1 data is on right and unshift will be right");
    parse_error("side = 2 data is on left and unshift will be left");
    parse_error("side = 0 no shift (returns null)");
    parse_error("Companion function is shift\n");
    parse_error("c.edwards 6/16/04\n");
    return NULL;
  }
 
  /* get structures */
  find_struct(rect,"data",&data);
  find_struct(rect,"shift_edge",&newshiftt);
 
  /* error handling */
  if(newshiftt == NULL || data == NULL) {
    parse_error("Structure must contain the elements:");
    parse_error("\"data\" and \"shift_edge\"");
    return NULL;
  }
  if(opt > 2 || opt < 0 ) {
    parse_error("Enter either 1 for right and 2 for left");
    return NULL;
  }
  if(opt==0) return NULL;
 
  /* x, y and z dimensions of the data and allocate memory */
  x = GetX(data);
  y = GetY(data);
  z = GetZ(data);
  w_data=calloc(sizeof(float),x*y*z);  
  newshift=calloc(sizeof(int),y);
 
  /* extract the old shift and turn it to negative shift*/
  for(j=0;j<y;j++) {
    newshift[j]=-(extract_int(newshiftt,cpos(0,j,0,newshiftt)));
  }
 
  /* extract and shift the pixels */
  /* shift pixels to the left / data is on the right */
  if(opt==1) {
    for(k=0;k<z;k++) {
      for(j=0;j<y;j++) {
	for(i=0;i<x;i++) {
	  if(newshift[j] == -(x-1)){
	    w_data[x*y*k + x*j + i]=nullval;
	  }
	  if(newshift[j] == 0) {
	    w_data[x*y*k + x*j + i ]=extract_float(data,cpos(i,j,k,data));
	  }
	  if(newshift[j] != 0 && newshift[j] != -(x-1)) {
	    if(i >= -newshift[j]) {
	      w_data[x*y*k + x*j + i]=extract_float(data,cpos(i+newshift[j],j,k,data));
	    }
	    if(i < -newshift[j]) {
	      w_data[x*y*k + x*j + i]=nullval;
	    }
	  }
	}
      }
    }
  }
 
  /* shift pixels to the right / data is on the left */
  if(opt==2) {
    for(k=0;k<z;k++) {
      for(j=0;j<y;j++) {
	for(i=0;i<x;i++) {
	  if(newshift[j] == 0){
	    w_data[x*y*k + x*j + i]=nullval;
	  }
	  if(newshift[j] == -(x-1)) {
	    w_data[x*y*k + x*j + i ]=extract_float(data,cpos(i,j,k,data));
	  }
	  if(newshift[j] != 0 && newshift[j] != -(x-1)) {
	    if(i < -newshift[j]+1) {
	      w_data[x*y*k + x*j + i]=extract_float(data,cpos(i+(newshift[j]+x)-1,j,k,data));
	    }
	    if(i >= -newshift[j]+1) {
	      w_data[x*y*k + x*j + i]=nullval;
	    }
	  }
	}
      }
    }
  }
 
  /* return and clean up */
  free(newshift);
  out=newVal(BSQ,x,y,z,FLOAT,w_data);
  return out;
}



Var *
cse_tes_shift(vfuncptr func, Var * arg)
{

  typedef unsigned char byte;

  Var    *data=NULL;         /* tes map to be shifted */
  Var    *out=NULL;          /* output data */
  Var    *shifts=NULL;       /* shift info */
  float  *w_data=NULL;       /* working data */
  int     w_shifts;          /* working shifts */  
  int     x,y,i,j;           /* dims and indices */
  int     tv=-32768;         /* tmpval */
  int     maxshift=-32768;   /* maxshift */ 

  Alist alist[3];
  alist[0] = make_alist("data",      ID_VAL,      NULL,  &data);
  alist[1] = make_alist("shifts",    ID_VAL,      NULL,  &shifts);
  alist[2].name = NULL;

  if (parse_args(func, arg, alist) == 0) return(NULL);

  if(data == NULL || shifts == NULL ){
    parse_error("\nUsed to shift tes maps for vertical lines and deplaid\n");
    parse_error("$1=the data to shift");
    parse_error("$2=the shift data from tes_ock_interp() or ");
    parse_error("from some default shifts\n");
    parse_error("To unshift use negative shifts\n");
    parse_error("c.edwards 9/24/04\n");
    return NULL;
  }
  
  /*get dims */
  x=GetX(data);
  y=GetY(data);
  if(GetZ(data)!=1) {
    parse_error("please use single band data\n");
    return NULL;
  }
  
  /* get max shift */
  for(j=0;j<y;j++) {
    tv=extract_int(shifts,cpos(0,j,0,shifts));
    if(tv>maxshift) maxshift=tv;
  }
  /*memory allocation */
  w_data=(float *)calloc(sizeof(float),(x+maxshift)*y);
  
  /* get shifts, extract and shift the data */
  for(j=0;j<y;j++) {
    w_shifts=extract_int(shifts,cpos(0,j,0,shifts));
    for(i=0;i<x;i++) {
      w_data[(x+maxshift)*j +(maxshift-w_shifts+i)]=(float)extract_float(data,cpos(i,j,0,data));
    }
  }

  /* return the data */
  out=newVal(BSQ,x+maxshift,y,1,FLOAT,w_data);
  return out;
}



Var *
cse_sobel(vfuncptr func, Var * arg)
{

  Var    *pic=NULL;          /* input data*/
  Var    *out=NULL;          /* output pic */
  int     x,y,z;             /* image size */
  int     i,j,k;             /* loop indeces */
  float  *xkern,*ykern;      /* sobel kernals */
  float  *w_pic=NULL;        /* working picture */
  float  *dir=NULL;          /* direction vectors */
  float  *sobelx,*sobely;    /* sobel convolved images */
  double  tv,tx,ty;          /* tmp value */
  float   null=-32768;       /* ignore values */

  Alist alist[3];
  alist[0] = make_alist("pic",      ID_VAL,     NULL,  &pic);
  alist[1] = make_alist("ignore",   FLOAT,      NULL,  &null);
  alist[2].name = NULL;

  if (parse_args(func, arg, alist) == 0) return(NULL);
 
  if(pic==NULL) {
    parse_error("Runs a Sobel edge detecor on an image\n");
    parse_error("This will sstretch the data and return the");
    parse_error("magnitude and direction as mag and dir respectively\n");
    parse_error("$1 = data, either sstretched or not");
    parse_error("ignore = ignore value\n");
    parse_error("c.edwards 10/1/04\n");
    return NULL;
  }

  /* x, y and z dimensions of the data */
  x = GetX(pic);
  y = GetY(pic);
  z = GetZ(pic);

  /* make room for the data!!*/
  w_pic=(float *)calloc(sizeof(float),x*y*z);
  dir=(float *)calloc(sizeof(float),x*y*z);
  sobely=(float *)calloc(sizeof(float),x*y*z);
  sobelx=(float *)calloc(sizeof(float),x*y*z);
  xkern=(float *)calloc(sizeof(float),9);
  ykern=(float *)calloc(sizeof(float),9);
  
  /*make the sobel kernel */
  xkern[0]=-1;
  xkern[1]=0;
  xkern[2]=1;
  xkern[3]=-2;
  xkern[4]=0;
  xkern[5]=2;
  xkern[6]=-1;
  xkern[7]=0;
  xkern[8]=1;
  ykern[0]=1;
  ykern[1]=2;
  ykern[2]=1;
  ykern[3]=0;
  ykern[4]=0;
  ykern[5]=0;
  ykern[6]=-1;
  ykern[7]=-2;
  ykern[8]=-1;

  /*extract the data */
  for(k=0;k<z;k++) {
    for(j=0;j<y;j++) {
      for(i=0;i<x;i++) {
	tv = extract_float(pic, cpos(i,j,k,pic));
	w_pic[k*x*y + x*j + i]=tv;
      }
    }
  }
  
  /* sstretch the data */
  w_pic=sstretch_sobel(w_pic,null,x,y,z);
  
  /* run sobel kernal over the data */
  sobelx=smooth(w_pic,xkern,x,y,z,3,3,1,1,0);
  sobely=smooth(w_pic,ykern,x,y,z,3,3,1,1,0);
  
  /* determine the magnitude and direction vectors */
  for(k=0;k<z;k++) {
    for(j=0;j<y;j++) {
      for(i=0;i<x;i++) {
	tx=sobelx[k*x*y + x*j + i];
	ty=sobely[k*x*y + x*j + i];
	w_pic[k*x*y + x*j + i]=(float)sqrt(tx*tx+ty*ty);
	dir[k*x*y + x*j + i]=(float)(-atan2(ty,tx)*(360/2*3.14159265));
      }
    }
  }
 
  /*clean up and return data */
  free(sobelx);
  free(sobely);
  free(xkern);
  free(ykern);
  
  out=new_struct(2);
  add_struct(out,"mag",newVal(BSQ,x,y,z,FLOAT,w_pic));
  add_struct(out,"dir",newVal(BSQ,x,y,z,FLOAT,dir));	     
  return out;
}



Var *
cse_circle(vfuncptr func, Var * arg)
{
  Var    *out=NULL;          /* out data */
  float  *pic=NULL;          /* working data */
  int     x=0,y=0;           /* array dimensions */
  int     xi=0,yi=0;         /* array points */
  int     cx=0,cy=0,j,i;     /* center points and loop indices */
  float   t=0;               /* theta for parametric equation */
  float   r=0;               /* radius */
  float   whigh,wlow;        /* the values for the weighting option */
  int     count;             /* on pixel count */
  int     weight=0;          /* weight option */

  Alist alist[3];
  alist[0] = make_alist("r",      FLOAT,        NULL,  &r);
  alist[1] = make_alist("weight", INT,          NULL,  &weight);
  alist[2].name = NULL;
  
  if (parse_args(func, arg, alist) == 0) return(NULL);
  
  if (r == 0){
    parse_error("Circle drawing algorithm\n");
    parse_error("$1 = raduis of the circle");
    parse_error("float is supported but diameter will be integer");
    parse_error("weight = enter 2 to weight the array to sum to 0");
    parse_error("weight = 1 or -1 to set half circle equal to 10 or -10\n");
    parse_error("c.edwards 10/8/04\n");
    return NULL;
  }  
  
  /*set x,y and the center */
  /*checking to see if diameter is odd */
  
  x=y=(int)(r*2);
  if(x%2==0) {
    cx=(int)r;
    cy=(int)r;
  }
  if(x%2==1) {
    cx=(int)r+1;
    cy=(int)r+1;
  }
  
  /* allocate memory*/
  pic=(float *)calloc(sizeof(float),(int)x*y);
  
  /* loop through using a parametric circle drawing techinque */
  for(t=0.0;t<2*M_PI;t+=1/r) {
    xi=(int)r*cos(t)+cx;
    yi=(int)r*sin(t)+cy;
    pic[x*yi + xi]=1;
  }  

  /* weighted array option */
  /* get the count of on pixels */
  if(weight==2) {
    count=0;
    for(j=0;j<y;j++) {
      for(i=0;i<x;i++) {
	if(pic[x*j + i]==1) count++;
      }
    }
    /*set the weighting amounts */
    whigh=(float)1./count;
    wlow=-(float)1./(x*y-count);
    
    /* loop through and set the values */
    for(j=0;j<y;j++) {
      for(i=0;i<x;i++) {
	if(pic[x*j + i]!=1) pic[x*j + i]=wlow;
	if(pic[x*j + i]==1) pic[x*j + i]=whigh;
      }
    }
  }
  /* crater kernel weight */
  if(weight==1) {
    for(j=0;j<y;j++) {
      for(i=0;i<r;i++) {
	if(pic[x*j + i]==1 ) pic[x*j + i]=10;
      }
      for(i=r;i<x;i++) {
	if(pic[x*j + i]==1) pic[x*j + i]=-10;
      }
    }
  }
  if(weight==-1) {
    for(j=0;j<y;j++) {
      for(i=0;i<r;i++) {
	if(pic[x*j + i]==1) pic[x*j + i]=-10;
      }
      for(i=r;i<x;i++) {
	if(pic[x*j + i]==1) pic[x*j + i]=10;
      }
    }
  }
  
  /* return the circle array */
  out=newVal(BSQ,x,y,1,FLOAT,pic);
  return out;
}



Var*
cse_reconst(vfuncptr func, Var * arg)
{
  
  Var    *obj=NULL;                 /* input rectify structre */
  Var    *out=NULL;                 /* output data */
  Var    *vdata=NULL;               /* data  var */
  Var    *vleftedge=NULL;           /* leftedge var */
  Var    *w=NULL,*a=NULL;           /* width and angle var */ 
  int    *leftedge=NULL;            /* leftedge array */
  float   angle=0;                  /* angle value */
  int     width=0;                  /* width value */
  int     x,y,z;                    /* rectified image size */
  int     i,j,k;                    /* loop indices */
  float  *new_data=NULL;            /* changing data size */
  float   null=-32768;              /* null value */
  int     lshift;                   /* vert shift max shift */
  float   shift;                    /* tan of the shift */
  
  
  Alist alist[4]; 
  alist[0] = make_alist("rect",      ID_STRUCT,  NULL,  &obj);
  alist[1] = make_alist("null",      FLOAT,      NULL,  &null);
  alist[2] = make_alist("ignore",    FLOAT,      NULL,  &null); 
  alist[3].name = NULL;
  
  if (parse_args(func, arg, alist) == 0) return(NULL);
  
  if(obj==NULL ) {
    parse_error("\nTakes rectified cubes and returns");
    parse_error("them to projected geometry\n");
    parse_error("$1 = rectify structure");
    parse_error("ignore = null value of the data (default -32768)\n");
    parse_error("c.edwards 10/9/04\n");
    return NULL;
  }
  
  /* get stuff from structre */
  find_struct(obj,"data",&vdata);
  find_struct(obj,"leftedge",&vleftedge);
  find_struct(obj,"angle",&a);
  find_struct(obj,"width",&w);
  width = extract_int(w, 0);
  angle = -extract_float(a,0);
  
  if(angle == 0) {
    parse_error("Why did you bother to shear the data by a 0 angle?\n"); return NULL;
  }
  if(angle > 80 || angle < -80) {
    parse_error("Try to stay between -80 and 80, love.  Our motto is \"No Crashy Crashy.\""); return NULL;
  }
  
  /* get x,y,z */
  x=GetX(vdata);
  y=GetY(vdata);
  z=GetZ(vdata);
  new_data=(float *)calloc(sizeof(float),x*y*z);
  leftedge=(int *)calloc(sizeof(int),y);
  
  /* extract the data  and leftedge*/
  for(j=0;j<y;j++) {
    leftedge[j]=extract_int(vleftedge,cpos(0,j,0,vleftedge));
  }
  for (k=0;k<z;k++) {
    for (j=0;j<y;j++) {
      for (i=0;i<x;i++) {
	new_data[x*y*k + x*j + i]=extract_float(vdata,cpos(i,j,k,vdata));
      }
    }
  }

  /* run unslantc */
  new_data=unslantc(new_data,leftedge,width,x,y,z,null);
  
  /* calculating the number of rows to add to the picture to accomodate the shear */
  shift = tan((M_PI / 180.0) * angle);
  lshift = (int)((width*fabs(shift)-0.5)+1)*(shift/fabs(shift));

  /* error handling */
  if(lshift == 0) {
    parse_error("The shear is too small to accomplish anything. Try again bozo.\n"); return NULL;
  }
  if(y-abs(lshift)<=0) {
    parse_error("The trimmed array has a length of %d, you nincompoop. Stop trying to crash me.\n", y-abs(lshift));
    return NULL;
  }
  
  /* run unshearc */
  new_data=unshearc(new_data,angle,width,y,z,null);
  free(leftedge);

  /* return the data */  
  out = newVal(BSQ, width, y-abs(lshift), z, FLOAT, new_data);
  return(out);
}



Var * 
cse_sstretch2(vfuncptr func, Var * arg)
{

  /* takes data and stretches it similar to sstretch() davinci function*/
  /* but does it band by band */

  typedef unsigned char byte;
  
  Var       *data=NULL;               /* input */
  Var       *out=NULL;                /* output */
  float     *w_data=NULL;             /* working data2 */
  byte      *w_data2=NULL;            /* working data */
  float     *sample=NULL;             /* sample data */
  Var       *sam_data=NULL;           /* sample input */
  float      ignore=-32768;           /* ignore value*/
  int        x,y,z,samx,samy,samz;    /* indices */
  double     sum = 0;                 /* sum of elements in data */
  double     sumsq = 0;               /* sum of the square of elements in data */
  double     stdv = 0;                /* standard deviation */
  int        cnt = 0;                 /* total number of non-null points */
  int        i,j,k;                   /* loop indices */
  float      tv;                      /* tmp value */
  float      v=40;                    /* variance */
  
  Alist alist[5];
  alist[0] = make_alist("data", 	  ID_VAL,	NULL,	&data);
  alist[1] = make_alist("ignore", 	  FLOAT,	NULL,	&ignore);
  alist[2] = make_alist("variance",       FLOAT,        NULL,   &v);
  alist[3] = make_alist("sample",         ID_VAL,       NULL,   &sam_data);
  alist[4].name = NULL;
  
  if (parse_args(func, arg, alist) == 0) return(NULL);
  
  if (data == NULL) {
    parse_error("\nUsed to sstretch data band by band");
    parse_error("Similar to the davinci function sstretch()\n");
    parse_error("$1=data to be stretched");
    parse_error("ignore=value to ignore (Default -32768)");
    parse_error("variance=variance of the stretch (Default=40)");
    parse_error("sample=sample data to stretch, must have sampe # of bands\n");
    parse_error("c.edwards 6/15/05\n");	
    out = newVal(BSQ, 1, 1, 1, BYTE, (float *)calloc(sizeof(float), 1));
    return(out);
  }

  /* x, y and z dimensions of the data */
  x = GetX(data);
  y = GetY(data);
  z = GetZ(data);
  if(sam_data != NULL) {
    samx = GetX(sam_data);
    samy = GetY(sam_data);
    samz = GetZ(sam_data);
  }
 
  if(z != samz && sam_data != NULL) {
    parse_error("\nThe sample data and the data must have the same # of bands\n");
    return(NULL);
  }

  /* create out array */
  w_data = (float *)calloc(sizeof(float), x*y*z);
  w_data2 = (byte *)calloc(sizeof(byte),x*y*z);
  
  /* stretch each band separately */
  for(k=0;k<z;k++) {
    sum = 0;
    sumsq = 0;
    stdv = 0;
    cnt = 0;
    tv = 0;
    
    /*calculate the sum and sum squares if non sampled */
    if(sam_data == NULL) {
      for(j=0; j<y; j++) {
	for(i=0; i<x; i++) {
	  tv=extract_float(data, cpos(i,j,k, data));
	  w_data[k*y*x + j*x + i]=tv;
	  if( tv != ignore){
	    sum += (double)tv;
	    sumsq += (double)(tv*tv);
	    cnt += 1;
	  }
	}
      }
    }
    
    /* calculate the sum and sum of squares if sampled image */
    if(sam_data != NULL) {
      for(j=0; j<samy; j++) {
	for(i=0; i<samx; i++) {
	  tv=extract_float(sam_data, cpos(i,j,k, sam_data));
	  if( tv != ignore){
	    sum += (double)tv;
	    sumsq += (double)(tv*tv);
	    cnt += 1;
	  }
	}
      }

      /* extract data values if want sampled image */
      for(j=0; j<y; j++) {
	for(i=0; i<x; i++) {
	  w_data[k*y*x + j*x + i]=extract_float(data, cpos(i,j,k, data));
	}
      }
    }

    /* calculate standard deviation */
    stdv = sqrt((sumsq - (sum*sum/cnt))/(cnt-1));
    sum /= (double)cnt;
    
    /* fill in stretched values */
    for(j=0; j<y; j++) {
      for(i=0; i<x; i++) {
	if(w_data[k*y*x + j*x + i] != ignore) w_data[k*y*x + j*x + i] = (float)((w_data[k*y*x + j*x + i] - sum)*(v/stdv)+127);
	if(w_data[k*y*x + j*x + i] < 0) w_data[k*y*x + j*x + i] = 0; 
	if(w_data[k*y*x + j*x + i] > 255) w_data[k*y*x + j*x + i] = 255; 
      }
    }
  }
  
  /*convert to bip */
  for(j=0; j<y; j++) {
    for(i=0; i<x; i++) {
      for(k=0; k<z; k++) {
	w_data2[j*x*z + i*z + k]=(byte)w_data[k*y*x + j*x + i];
      }
    }
  }
  
  /* clean up and return data */
  free(w_data);
  out = newVal(BIP,z, x, y, BYTE, w_data2);	
  return(out);
}



Var *
cse_unscale(vfuncptr func, Var * arg)
{
  
  Var      *pds = NULL;                  /* the original pds structure */
  Var      *out = NULL;                  /* the output scaled data */
  Var      *w_pds=NULL;                  /* data from struct */
  float    *w_pic=NULL;                  /* working data */
  Var      *struc=NULL;                  /* second struct */
  Var      *bin=NULL;                    /* band_bin struct */
  Var      *baset, *multt;               /* base/multiplier temp*/
  double   *mult, *base;                 /* base/multiplier */
  Var      *c_null=NULL;                 /* core_null var */
  float     core_null=-32768;            /* core null value */
  int       i, j, k;                     /* loop indices */
  int       x=0, y=0, z=0, x1=0;         /* size of the picture */
  float     tv1=0;
  float     tv2=0;                       /* temp pixel value */
  
 
  Alist alist[2];
  alist[0] = make_alist("obj",        ID_STRUCT,     NULL,   &pds);
  alist[1].name = NULL;

  if (parse_args(func, arg, alist) == 0) return(NULL);

  if(pds==NULL) {
    parse_error("\nUsed to unscale a pds from short to float data\n");
    parse_error("$1 = the pds structure\n");
    parse_error("core_null values will be set to -32768");
    parse_error("c.edwards 7/6/05\n");
    return NULL;
  }

  /* get structures */
  find_struct(pds,"spectral_qube",&struc);
  find_struct(struc,"data",&w_pds);
  find_struct(struc,"core_null",&c_null);
  core_null=extract_float(c_null, 0);
  find_struct(struc,"band_bin",&bin);
  find_struct(bin,"band_bin_multiplier",&multt);
  find_struct(bin,"band_bin_base",&baset);
  
  
  /* set up base and multiplier arrays */
  x1 = GetX(multt);
  
  base = (double *)calloc(sizeof(double), x1);
  mult = (double *)calloc(sizeof(double), x1);
 
  for(i=0; i<x1; i++) {
    mult[i]=extract_double(multt,cpos(i,0,0,multt));
    base[i]=extract_double(baset,cpos(i,0,0,multt));
  }

  /* x, y and z dimensions of the data */
  x = GetX(w_pds);
  y = GetY(w_pds);
  z = GetZ(w_pds);

  /* allocate memory for the picture */
  w_pic = (float *)calloc(sizeof(float), x*y*z);  
  
  /* loop through and unscale data */
  for(k=0; k<z; k++) {
    for(j=0; j<y; j++) {
      for(i=0; i<x; i++) {
  	tv1 = extract_float(w_pds, cpos(i,j,k,w_pds));
  	tv2 = (float)((tv1*mult[k])+base[k]);
  	if (tv1 == core_null ) {
  	  tv2 = -32768.0;
  	}
  	w_pic[x*y*k + x*j + i]=tv2;
      }
    }
  }
  
  printf("core_null values set to -32768.0\n");
  
  /* clean up and return data */
  free(base);
  free(mult);
  
  out=newVal(BSQ,x,y,z,FLOAT,w_pic);
  return out;
}



Var *
cse_ramp(vfuncptr func, Var * arg)
{
  /* made more efficient and fixed several bugs Oct 14, 2005                         **
  ** added ability to speed up ramp calculation by setting a maximum # of iterations **
  ** Fri Oct 14 16:36:44 MST 2005                                                    */

  Var    *pic_1 = NULL;		 /* picture one                                   */
  Var    *pic_2 = NULL;		 /* picture two                                   */
  Var    *out = NULL;		 /* the output picture                            */
  float  *ramp = NULL; 	         /* storage location of output ramp               */
  int    *ol1 = NULL;	         /* the overlap in picture 1                      */
  int    *ol2 = NULL;	         /* the overlap in picture 2                      */
  float   nullv = 0.0;	         /* the ignore value                              */
  float   tmpval = 0.0;          /* a temporary extracted value from the pictures */
  int     r1 = -1, r2 = -1;      /* beginning and ending rows of the overlap      */
  int     c1 = -1, c2 = -1;      /* beginning and ending columns of the overlap   */
  int     i, j, k;	         /* loop indices                                  */
  int     x, y;		         /* dimensions of the input pictures              */
  int     t1;  			 /* memory location                               */
  int     ct = 1;		 /* counter                                       */
  int     n=-1, num = 1;	 /* fill-in number                                */
  int     up, down, left, right; /* some neighborhood indices                     */
  int     pare = 100000;         /* maximum # of pixels away from edge to search  */
  float   sum=0;  

  Alist alist[5];
  alist[0] = make_alist("pic1",   ID_VAL, NULL, &pic_1);
  alist[1] = make_alist("pic2",   ID_VAL, NULL, &pic_2);
  alist[2] = make_alist("stop",   INT,    NULL, &pare);
  alist[3] = make_alist("ignore", FLOAT,  NULL, &nullv);
  alist[4].name = NULL;

  if (parse_args(func, arg, alist) == 0)
    return (NULL);
  
  /* if no pictures got passed to the function */
  if (pic_1 == NULL || pic_2 == NULL) {
    parse_error("ramp() - Fri Oct 14 14:45:09 MST 2005");
    parse_error("Calculates a 0 - 1 float ramp between two overlapping pictures.");
    parse_error("You need to pass me two overlapping pictures contained in arrays\nof identical size, and an ignore value.\n");
    parse_error("Syntax:  b = thm.ramp(pic1, pic2, stop, ignore)");
    parse_error("example: b = thm.ramp(a1, a2, stop=100, ignore=-32768");
    parse_error("pic1 - may be any 2-d array - float, int, short, etc.");
    parse_error("pic2 - may be any 2-d array - float, int, short, etc.");
    parse_error("stop - maximum number of pixels away from edge to search. Default 100000");
    parse_error("ignore - the non-data pixel values. Default is 0.\n");
    parse_error("NOTES:\n");
    parse_error("You need to multiply the ramp*pic2 and (1-ramp)*pic1.");
    parse_error("The ramp is only found for OVERLAPPING regions.");
    parse_error("Non-overlapping regions from pic1 and pic2 need to be added to make a full blend.");
    parse_error("You can speed up ramp calculation by setting \'stop\'. 100 works well.");
    return NULL;
  }
  
  /* x and y dimensions of the original pictures */
  x = GetX(pic_1);
  y = GetY(pic_1);
  
  if (x != GetX(pic_2) || y != GetY(pic_2)) {
    parse_error("The two pictures need to be of the exact same dimensions.\n");
    return NULL;
  }
  
  /* create the two overlap arrays */
  ol1 = (int *) malloc(sizeof(int)*x*y);
  ol2 = (int *) malloc(sizeof(int)*x*y);
  
  /* lines and columns bounding data */
  r1 = y-1;
  r2 = 0;
  c1 = x-1;
  c2 = 0;
  
  /* extract profiles of images */
  for (j = 0; j < y ; j++) {
    for (i = 0; i < x ; i++) {
      t1 = j * x + i;
      ol1[t1] = -1;
      ol2[t1] = -1;
      
      tmpval = extract_float(pic_1, cpos(i, j, 0, pic_1));
      if(tmpval != nullv) ol1[t1] = 0;
      
      tmpval = extract_float(pic_2, cpos(i, j, 0, pic_2));
      if(tmpval != nullv) ol2[t1] = 0;
  
      if(ol1[t1] == 0) sum+=ol2[t1];
      /* find left and right limits of overlapping area */
      if (ol1[t1] != -1 && ol2[t1] != -1) {
	if (j < r1) { r1 = j; }
	if (j > r2) { r2 = j; }
	if (i < c1) { c1 = i; }
	if (i > c2) { c2 = i; }
      }
    }
  }
  
  /* ol1 and ol2 were being left zero (sum==0) causing the ramp to fail  **
  ** setting the outteredges of both ol1 and ol2 to -1 fixed the problem.*/ 
  if(sum==0) {
    for(i = 0; i < x; i++){
      ol1[x*y-(i+1)]=-1;
      ol1[i]=-1;
    }
    for(j = 1; j < y; j++){
      ol1[x*j]=-1;
      ol1[x*j-1]=-1;
    }
  }
  
  /* loop through the overlap arrays filling in the appropriate value.   **
  ** On the first iteration we search for any pixels with a neighbor of  **
  ** -1 and we set that pixel to 1.  On all other iterations we look for **
  ** neighbors with values num-1. If "stop" number is specified, we stop **
  ** searching for distances and set all values to value of "pare".      */
  k=1;
  while (ct > 0) {
    ct = 0;
    
    for (j = r1; j <= r2; j++) {
      for (i = c1; i <= c2; i++) {
	t1 = j * x + i;
	
	/* neighbor pixel positions */
	up = (j-1) * x + i;
	down = (j+1) * x + i;
	left = t1 - 1;
	right = t1 + 1;
	
	/* safety against falling off the edge */
	if(j==0) up = t1;
	if(j==y-1) down = t1;
	if(i==0) left = t1;
	if(i==x-1) right = t1;
	
	if (ol1[t1] == 0
	    && ((ol1[left] == n) || (ol1[right] == n)
		|| (ol1[up] == n) || (ol1[down] == n))) {
	  ol1[t1] = num;
	  ct += 1;
	}
	/* If the sum of the overlap == 0 then we don't need to do this part */
	if(sum!=0) {
	  if (ol2[t1] == 0
	      && ((ol2[left] == n) || (ol2[right] == n)
		  || (ol2[up] == n) || (ol2[down] == n))) {
	    ol2[t1] = num;
	    ct += 1;
	  }
	}
      }
    }
    num += 1;
    n = num-1;
    k+=1;

    /* we've searched enough. Set all remaining values to 'pare' */
    if(k==pare) {
      for (j = r1; j <= r2; j++) {
	for (i = c1; i <= c2; i++) {
	  t1 = j * x + i;
	  if (ol1[t1] == 0) ol1[t1] = pare;
	  if(sum!=0) {
	    if (ol2[t1] == 0) ol2[t1] = pare;
	  }
	}
      }
      ct=0;
    }
  }
  
  /* loop through last time and create ramp using the special cases*/
  ramp = (float *) calloc(sizeof(float), x*y);
  for (j = 0; j < y; j++) {
    for (i = 0; i < x; i++) {
      t1 = j*x + i;
      if (ol1[t1] != -1 && ol2[t1] != -1) {
	if(sum!=0)  ramp[t1] = (float)(ol2[t1]) / (float)(ol2[t1] + ol1[t1]);
	if(sum==0)  ramp[t1] = (float)(1 -((float)(ol1[t1]) / (float)(2*k)));
      }
    }
  }
  
  free(ol1);
  free(ol2);
  
  out = newVal(BSQ, x, y, 1, FLOAT, ramp);
  return out;
}



Var*
cse_interp2d(vfuncptr func, Var *arg)
{
  
  Var    *xdata = NULL;                /* the orignial data */
  Var    *ydata = NULL;                /* the orignial data */ 
  Var    *table = NULL;                /* look up table */
  Var    *out = NULL;                  /* the output struture */
  int     i,j,k;                       /* loop indices */
  float   p1, p2;                      /* percentages */
  int     xx,xy,xz,yx,yy,yz;           /* data size */
  float  *wdata = NULL;                /* working data */
  float   sx=1,dx=1,sy=1,dy=1;         /* start and delta values */
  float   tvx,tvy;                     /* data values */
  int     xi,yi;                       /* new x and y positions */
  float   tv1,tv2;                     /* temporary values */
  
  Alist alist[8];
  alist[0] = make_alist("table",     ID_VAL,    NULL,  &table);
  alist[1] = make_alist("xdata",     ID_VAL,    NULL,  &xdata);
  alist[2] = make_alist("ydata",     ID_VAL,    NULL,  &ydata);
  alist[3] = make_alist("startx",    FLOAT,     NULL,  &sx);
  alist[4] = make_alist("deltax",    FLOAT,     NULL,  &dx);
  alist[5] = make_alist("starty",    FLOAT,     NULL,  &sy);
  alist[6] = make_alist("deltay",    FLOAT,     NULL,  &dy);
  alist[7].name = NULL;
  
  if (parse_args(func, arg, alist) == 0) return(NULL);
  
  if (table == NULL) {
    parse_error("\ninterp2d()- Thu Apr 27 16:20:31 MST 2006");
    parse_error("Bilinear interpolation algorithm");
    parse_error("\nInputs and Outputs:");
    parse_error("table - table of values of a standard delta value for each axis");
    parse_error("xdata - the x data to interpolate");
    parse_error("ydata - the y data to interpolate");
    parse_error("startx - starting x value for the table");
    parse_error("deltax - delta  x value for the table");
    parse_error("starty - starting y value for the table");
    parse_error("deltay - delta y value for the table");
    parse_error("Returns a 1 d, array the size of x and y data\n");
    parse_error("c.edwards");
    return (NULL);
  }
    
  /*size of xdata*/
  xx = GetX(xdata);
  xy = GetY(xdata);
  xz = GetZ(xdata);

  /*size of ydata*/
  yx = GetX(ydata);
  yy = GetY(ydata);
  yz = GetZ(ydata);

  /*error handling, they must be the same size and one band*/
  if(xx!=yx || xy!=yy || xz!=1 || yz != 1) { 
    parse_error("\nThe x and y data must have the same dimensions and only one band\n");
    return NULL;
  }

  /*memory allocation*/
  wdata=(float *)calloc(sizeof(float),xx*xy*1);
  
  for(i=0;i<xx;i+=1) {
    for(j=0;j<xy;j+=1) {
      
      /*extract values from original data*/
      tvx=extract_float(xdata,cpos(i,j,0,xdata));
      tvy=extract_float(ydata,cpos(i,j,0,ydata));
      
      /*apply start and delta to the extracted values*/
      tvx=(tvx-sx)/dx;
      tvy=(tvy-sy)/dy;
      if(tvx<0) tvx=0;
      if(tvy<0) tvy=0;
      if(tvx>xx) tvx=xx-1;
      if(tvy>xy) tvy=xy-1;     
      
      /*calculate percentages */
      p1=(float)(tvx-floor(tvx));
      p2=(float)(tvy-floor(tvy));
      xi=(int)floor(tvx);
      yi=(int)floor(tvy);
      
      /*   apply the bilinear interpolation algorithm                  **
      **   val=(f(1,1)*(1-p1)+f(2,1)*p1)*(1-p2)+(f(1,2)*(1-p1)+f(2,2)*p1)*p2    **
      */

      tv1=(extract_float(table,cpos(xi,yi,0,table))*(1-p1)+extract_float(table,cpos(xi+1,yi,0,table))*(p1))*(1-p2);
      tv2=(extract_float(table,cpos(xi,yi+1,0,table))*(1-p1)+extract_float(table,cpos(xi+1,yi+1,0,table))*(p1))*(p2);
      wdata[xx*j + i]=(float)(tv1+tv2);
    }
  }
  out=newVal(BSQ, xx, xy, 1, FLOAT, wdata);
  return out;
}



Var*
cse_columnator(vfuncptr func, Var * arg)
{
  
  Var    *data=NULL;                 /* input rectify structre */
  Var    *out=NULL;                 /* output data */
  int     x,y,z;                    /* rectified image size */
  int     i,j,k;                    /* loop indices */
  float  *wdata=NULL;            /* changing data size */
  float   null=-32768;              /* null value */
  int     count=0;                  /* count*/

  Alist alist[3]; 
  alist[0] = make_alist("data",      ID_VAL,  NULL,  &data);
  alist[1] = make_alist("ignore",    FLOAT,      NULL,  &null); 
  alist[2].name = NULL;
  
  if (parse_args(func, arg, alist) == 0) return(NULL);
  
  if(data==NULL ) {
    return NULL;
  }
  
  /* get x,y,z */
  x=GetX(data);
  y=GetY(data);
  
  for (j=0;j<y;j++) {
    for (i=0;i<x;i++) {
      if(extract_float(data,cpos(i,j,0,data))!=null) {
	count+=1;
      }
    }
  }
  
  /*memory allocation*/
  wdata=(float *)calloc(sizeof(float),3*count);

  count=0;
  /*fill in the loop*/
  for (j=0;j<y;j++) {
    for (i=0;i<x;i++) {
      if(extract_float(data,cpos(i,j,0,data))!=null) {
	wdata[3*count + 1]=(float)i;
	wdata[3*count + 2]=(float)j;
	wdata[3*count + 3]=extract_float(data,cpos(i,j,0,data));
	 count+=1;
      }
    }
  }
  
  /* return the data */  
  out = newVal(BSQ, 3, count, 1, FLOAT, wdata);
  return(out);
}

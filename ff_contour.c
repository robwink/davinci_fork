#include "parser.h"

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



Var* ff_contour(vfuncptr func, Var *arg)
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



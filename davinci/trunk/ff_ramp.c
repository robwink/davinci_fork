#include "parser.h"

Var *ff_ramp(vfuncptr func, Var * arg)
{
  /* made more efficient and fixed several bugs Oct 14, 2005                         **
  ** added ability to speed up ramp calculation by setting a maximum # of iterations **
  ** Fri Oct 14 16:36:44 MST 2005                                                    **
  ** added completely overlapping picture functionality                              **
  ** Fri Mar  3 13:31:47 MST 2006                                                                                */

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
    parse_error("ramp() - Fri Mar  3 13:31:47 MST 2006");
    parse_error("Calculates a 0 - 1 float ramp between two overlapping pictures.");
    parse_error("You need to pass me two overlapping pictures contained in arrays\nof identical size, and an ignore value.\n");
    parse_error("Syntax:  b = ramp(pic1, pic2, stop, ignore)");
    parse_error("example: b = ramp(a1, a2, stop=100, ignore=-32768");
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

    /* we've searched enough. Set all remaining values to 'pare' (stop in spanish) */
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

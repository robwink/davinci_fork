#include "parser.h"
#include "ff_modules.h"

static Var *ff_y_shear(vfuncptr, Var *);
static Var *ff_kfill(vfuncptr, Var *);

static dvModuleFuncDesc exported_list[] = {
  { "y_shear", (void *) ff_y_shear },
  { "kfill", (void *) ff_kfill }
};

static dvModuleInitStuff is = {
  exported_list, 2,
  NULL, 0
};

dv_module_init(
    const char *name,
    dvModuleInitStuff *init_stuff
    )
{
    *init_stuff = is;
    
    parse_error("Loaded module kjn.");

    return 1; /* return initialization success */

}

void
dv_module_fini(const char *name)
{
  parse_error("Unloaded module kjn.");
}

Var *
ff_y_shear(vfuncptr func, Var * arg)
{
  Var     *pic_v = NULL;            /* the picture */
  Var     *out;                     /* the output picture */
  float   *pic = NULL;              /* the new sheared picture */
  float    angle = 0;               /* the shear angle */
  float    shift;                   /* the shift/pixel */
  int	   trim = 0;		    /* whether to trim the data or not */
  int      lshift;                  /* the largest shift (int) */
  int      x, y, z;                 /* dimensions of the original picture */
  int      i, j, k;                 /* loop indices */
  int      nx, ni, nj;              /* memory locations */

  Alist alist[4];
  alist[0] = make_alist("picture",		ID_VAL,		NULL,	&pic_v);
  alist[1] = make_alist("angle",		FLOAT,		NULL,	&angle);
  alist[2] = make_alist("trim",		        INT,		NULL,    &trim);
  alist[3].name = NULL;

  if (parse_args(func, arg, alist) == 0) return(NULL);

  /* if no picture got passed to the function */
  if (pic_v == NULL){
    parse_error("Yain't got no picture, FOOL!\n\n");
    parse_error("SYNTAX EXAMPLE:  b=y_shear(picture=a,angle=8,trim=1)\n");
    parse_error("or more simply:  b=y_shear(a,8,1)\n\n");
    parse_error("y_shear() takes an input array and shears the y position of each pixel according to a shear angle in degrees.\n");
    parse_error("The shear angle may be between -80 and 80 degrees\n");
    parse_error("If you want to unshear the array an equal but opposite angle and want it to have the same dimensions as the original, then use the \"trim=1\" option\n");
    return NULL;
  }

  /* x, y, and z dimensions of the original picture */
  x = GetX(pic_v);
  y = GetY(pic_v);
  z = GetZ(pic_v);

  if (angle == 0) {
    parse_error("Why did you bother to shear the data by a 0 angle?\n"); return NULL;
  }

  if (angle > 80 || angle < -80) {
    parse_error("Try to stay between -80 and 80, love.  Our motto is \"No Crashy Crashy.\""); return NULL;
  }

  /* calculating the number of rows to add to the picture to accomodate the shear */
  shift = tan((M_PI / 180.0) * angle);
  lshift = (int)((x*fabs(shift)-0.5)+1)*(shift/fabs(shift));

  if (lshift == 0) {
    parse_error("The shear is too small to accomplish anything. Try again bozo.\n"); return NULL;
  }

  /* for trim == 0 */
  if(trim == 0) {

    /* create the new picture array */
    pic = (float *)calloc(sizeof(float), x*z*(y+abs(lshift)));

    if (pic == NULL) {
      parse_error("ERROR! Unable to allocate %d bytes\n", sizeof(float)*x*z*(y+abs(lshift)));
      return NULL;
    }

    /* extract the picture directly into a sheared picture */
    for(k=0; k<z; k++) {
      for(j=0; j<y; j++) {
        for(i=0; i<x; i++) {

          /* the shifted y pixel */
	    /* if the angle is greater than 0 */
 	    if(lshift>0) {
	      ni = k*(y+abs(lshift))*x + ((int)(j+abs(lshift)-(int)(shift*i+0.5)))*x + i;
	    }

	    /* if the angle is less than 0 */
	    if(lshift<0) {
	      ni = k*(y+abs(lshift))*x + (j+(int)(fabs(shift)*i+0.5))*x + i;
	    }

	    pic[ni] = extract_float(pic_v, cpos(i, j, k, pic_v));
        }
      }
    }
    /* final output */
    out = newVal(BSQ, x, y+abs(lshift), z, FLOAT, pic);
  }

  /* for trim != 0 */
  if(trim != 0) {

    if(y-abs(lshift)<=0) {
      parse_error("The trimmed array has a length of %d, you nincompoop. Stop trying to crash me.\n", y-abs(lshift));
      return NULL;
    }

    /*create the new picture array */
    pic = (float *)calloc(sizeof(float), x*z*(y-abs(lshift)));

    if (pic == NULL) {
      parse_error("ERROR! Unable to allocate %d bytes\n", sizeof(float)*x*z*(y+abs(lshift)));
      return NULL;
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

          pic[nx] = extract_float(pic_v, cpos(i, nj, k, pic_v));
        }
      }
    }
    /* final output */
    out = newVal(BSQ, x, y-abs(lshift), z, FLOAT, pic);
  } 
  return out;
}


Var *
ff_kfill(vfuncptr func, Var * arg)
{
  Var *pic_v = NULL;	 /* the picture */
  Var *wgt_v = NULL;	 /* the weighting array */
  Var *itr_v = NULL;	 /* the number of iterations */
  Var *out;		 /* the output picture */
  int    w; /* width of the weight matrix (= height) */
  int    x,y,z;
  float *wgt = NULL; /* storage location of the weight array */
  int    c; /* general array counter */
  int    n; /* number of elements - generally */
  int    i, j, k, mi, mj; /* general array counters */
  float  min_wgt; /* minimum value of weight */
  float *pic = NULL, *pic2 = NULL, *tpic = NULL;
  float  pixel; /* working var */
  int    wth; /* (w-1)/2 */
  int    pic_x, pic_y; /* dims of the expanded picture */
  int    px, py, pi;
  int    pl, pr, pu, pd, r, pave;    /* the nearest neighbor positions */
  float  sum_non_zero_mask;
  int    nn;
  int    niter = 0;
  int    nzeroes = 0;

  
  Alist alist[4];
  alist[0] = make_alist("picture",		ID_VAL,		NULL,	&pic_v);
  alist[1] = make_alist("weight",		ID_VAL,		NULL,	&wgt_v);
  alist[2] = make_alist("iterations", 		ID_VAL,		NULL,	&itr_v);
  alist[3].name = NULL;
  
  if (parse_args(func, arg, alist) == 0) return(NULL);
  
  if (pic_v == NULL){
    parse_error("ERROR! picture not specified.\n"); return NULL;
  }
  
  if (wgt_v == NULL){
    parse_error("ERROR! weight matrix not specified.\n"); return NULL;
  }
  
  x = GetX(pic_v);		  /* get the x dimension of the image */
  y = GetY(pic_v);		  /* get the y dimension of the image */
  z = GetZ(pic_v);                /* get the z dimension of the image */
  if (z > 1){
    parse_error("Will not handle 3D arrays.\n"); return NULL;
  }

  w = GetX(wgt_v);		  /* get the x dimension of the weighting array */
  if (GetY(wgt_v) != w || GetZ(wgt_v) > 1){
    parse_error("Weight array must be the same dim in X & Y with a Z dim of 1.\n");
    return NULL;
  }

  if ((w%2) == 0 || w < 7){
    parse_error("Width should be an odd number greater than 5.\n");
    return NULL;
  }

  if (itr_v != NULL){
    niter = extract_int(itr_v, cpos(0, 0, 0, itr_v));
  }

  /* extract weight data */
  wgt = (float *)calloc(sizeof(float), w*w);
  if (wgt == NULL){
    parse_error("ERROR! Unable to alloc %d bytes.\n", sizeof(float)*w*w);
    return NULL;
  }

  c = 0;
  for(j = 0; j < w; j++){
    for(i = 0; i < w; i++){
      wgt[c++] = extract_float(wgt_v, cpos(i, j, 0, wgt_v));
    }
  }
  
  wth = (w-1)/2;		  /* the "radius" of the weighting array */
  
  /* extract picture data */
  pic_x = x+2*wth; pic_y = y+2*wth;
  pic = (float *)calloc(sizeof(float), pic_x*pic_y);
  if (pic == NULL){
    parse_error("ERROR! Unable to alloc %d bytes.\n", sizeof(float)*pic_x*pic_y);
    free(wgt); /* cleanup before exit */
    return NULL;
  }

  pic2 = (float *)calloc(sizeof(float), pic_x*pic_y);
  if (pic2 == NULL){
    parse_error("ERROR! Unable to alloc %d bytes.\n", sizeof(float)*pic_x*pic_y);
    free(wgt); /* cleanup before exit */
    free(pic);
    return NULL;
  }

  for(j = 0; j < y; j++){
    for(i = 0; i < x; i++){
      pic[(j+wth)*pic_x+(i+wth)] = extract_float(pic_v, cpos(i, j, 0, pic_v));
    }
  }

  /* find min of the weight array */
  n = w*w;
  min_wgt = wgt[0]; /* assume minimum */
  for(i = 1; i < n; i++){
    if (wgt[i] < min_wgt){ min_wgt = wgt[i]; }
  }

  if (min_wgt == 0.0){ min_wgt = 1.0; } /* sanity */

  /* divide wgt array by the minimum */
  for(i = 0; i < n; i++){ wgt[i] = wgt[i] / min_wgt; }

  /* iterate through data */
  for(k = 1; (k <= niter) || (niter == 0); k++){

    /* copy pic onto pic2 */
    memcpy(pic2, pic, sizeof(float)*pic_x*pic_y);
    nzeroes = 0;

    for(j = 0; j < y; j++){
      for(i = 0; i < x; i++){  
	
	py = j+wth; px = i+wth;
	pi = py * pic_x + px;
	
	if (pic[pi] == 0.0){

	  pl = 0;
	  pr = 0;
	  pu = 0;
	  pd = 0;
	  r = 1;
	  pave = 0;
	  nn = 0;

	  /* loop through the data to find the nearest neighbors in the 4 cardinal directions */
	  while(r <= wth) {
	    pl = (pl == 0 && pic[pi - r] > 0)?r:pl;
	    pr = (pr == 0 && pic[pi + r] > 0)?r:pr;
	    pu = (pu == 0 && pic[pi - r * pic_x] > 0)?r:pu;
	    pd = (pd == 0 && pic[pi + r * pic_x] > 0)?r:pd;

	    /* in the case that we don't have any near neighbors, set values to ave p */
	    if (r == wth) {
              nn = ((pl>0)?1:0) + ((pr>0)?1:0) + ((pu>0)?1:0) + ((pd>0)?1:0);
              if (nn > 0) {
		pave = (pl + pr + pu + pd) / nn;
		pl = (pl == 0)?pave:pl;
		pr = (pr == 0)?pave:pr;
		pu = (pu == 0)?pave:pu;
		pd = (pd == 0)?pave:pd;
	      }
	    }

	    r += 1;
	  }

	  if (nn>1) {
	    nzeroes++;
	    pixel = 0.0;
	    sum_non_zero_mask = 0.0;
	  
	    for(mj = wth - pu; mj <= wth + pd; mj++){
	      for(mi = wth - pl; mi <= wth + pr; mi++){
		pixel += pic[(j+mj)*pic_x+(i+mi)] * wgt[mj*w+mi];
		if (pic[(j+mj)*pic_x+(i+mi)] > 0.0){sum_non_zero_mask += wgt[mj*w+mi];}
	      }
	    }

	    if (sum_non_zero_mask == 0.0){ sum_non_zero_mask = 1.0; } /* sanity */

	    /* divide by weight */
	    /* assign pixel to location */
	    pic2[pi] = pixel / sum_non_zero_mask;
	  }
	}
      }
    }

    /* swap pic & pic2 arrays */
    tpic = pic; pic = pic2; pic2 = tpic;

    if (nzeroes == 0){
      break;
    }
    
  }

  if (niter == 0){
    parse_error("Done in %d iterations.\n", k);
  }

  /* construct return value */
  pic2 = (float *)realloc(pic2, sizeof(float)*x*y);
  if (pic2 == NULL){
    parse_error("blah");
    free(wgt);
    free(pic);
    return NULL;
  }

  /* copy data out */
  for(j = 0; j < y; j++){
    for(i = 0; i < x; i++){
      pic2[j*x+i] = pic[(j+wth)*pic_x+(i+wth)];
    }
  }
  out = newVal(BSQ, x, y, 1, FLOAT, pic2);

  /* cleanup before exit */
  free(wgt);
  free(pic);

  return out;
}

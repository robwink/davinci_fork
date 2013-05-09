#include "parser.h"

float *sawtooth(int x, int y, int z)
{
  float *kernel;                           /* the end kernel array */
  int i, j, k;                             /* loop indices */
  float xp, yp, zp;                        /* value of orthogonal parts */
  float xm, ym, zm;                        /* multiplier value in x, y, z */
  float total;                             /* sum value of filter */

  kernel = (float *)calloc(sizeof(float), x*y*z);
  xm = (x>1)?4.0/((float)x):1.0;
  ym = (y>1)?4.0/((float)y):1.0;
  zm = (z>1)?4.0/((float)z):1.0;

  for(k=0; k<z; k++) {
    zp = (z>1)?(((z+1)/2)-abs((z/2)-k))*zm:1.0;
    for(j=0; j<y; j++) {
      yp = (y>1)?(((y+1)/2)-abs((y/2)-j))*ym:1.0;
      for(i=0; i<x; i++) {
	xp = (x>1)?(((x+1)/2)-abs((x/2)-i))*xm:1.0;
	kernel[k*x*y + j*x + i] = xp*yp*zp;
	total += xp*yp*zp;
      }
    }
  }

  total = (float)x*(float)y*(float)z/total;

  for(k=0; k<z; k++) {
    for(j=0; j<y; j++) {
      for(i=0; i<x; i++) {
	kernel[k*x*y + j*x + i] = kernel[k*x*y + j*x + i]*total;
      }
    }
  }

  return(kernel);
}


Var *ff_sawtooth(vfuncptr func, Var * arg)
{

  Var *out;                   /* output array */
  float *tooth;               /* the output of sawtooth routine */
  int x = 0, y = 0, z = 0;

  Alist alist[4];
  alist[0] = make_alist("x",       INT,         NULL,	&x);
  alist[1] = make_alist("y",       INT,         NULL,	&y);
  alist[2] = make_alist("z",       INT,         NULL,   &z);
  alist[3].name = NULL;
  
  if (parse_args(func, arg, alist) == 0) return(NULL);

  if (x == 0 || y == 0 || z == 0) {
    parse_error("sawtooth() - 4/25/04");
    parse_error("Creates a sawtooth filter kernel of size x by y by z\n");
    parse_error("Syntax: a = sawtooth(1,300,1)\n");
    return NULL;
  }

  tooth = sawtooth(x, y, z);
  out = newVal(BSQ, x, y, z, FLOAT, tooth);
  return out;
}


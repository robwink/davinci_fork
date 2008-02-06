#include "parser.h"
#if 0
vector convolve_BRUTE(const vector& vec1, const vector& vec2)
{
        const vector* v1;
        const vector* v2;
        vector v(vec1.size + vec2.size - 1, 0.0);

        if(vec1.size <= vec2.size)
        {
                v1 = &vec1;
                v2 = &vec2;
        }
        else
        {
                v1 = &vec2;
                v2 = &vec1;
        }

        long int i, j;
        /* part 1 */
        for(i = 0; i <= (v1->size - 1); ++i)
                for(j = 0; j <= i; ++j)
                        v.data[i] += v1->data[j] * v2->data[i - j];
        /* part 2 */
        if(v2->size > v1->size)
                for(i = v1->size; i <= (v2->size - 1); ++i)
                        for(j = 0; j <= (v1->size - 1); ++j)
                                v.data[i] += v1->data[j] * v2->data[i - j];
        /*  part 3 */
        for(i = 1; i <= (v1->size - 1); ++i)
                for(j = i; j <= (v1->size - 1); ++j)
                        v.data[(v2->size - 1) + i] +=
                        v1->data[j] * v2->data[(v2->size - 1) + i - j];

        return v;
}

#endif

Var *
ff_self_convolve(vfuncptr func, Var * arg)
{
	Var *v1 = NULL, *v2 = NULL;
	int m, n, i, j, d1, d2;
	float *out;

	int ac;
	Var **av;
	Alist alist[2];
	alist[0] = make_alist( "obj1",    ID_VAL,    NULL,     &v1);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (v1 == NULL) {
		parse_error("%s: No obj1 specified\n", func->name);
		return(NULL);
	}

	d1 = V_DSIZE(v1);

	n = d1 *2-1;
	out = (float *)calloc(sizeof(float), n);

	for (m = 0 ; m < d1 ; m++) {
		for (n = 0 ; n < d1 ; n++) {
			d2 = d1/2 -1 -n +m;
			if (d2 < 0 || d2 >= n) continue;
			out[m] += extract_float(v1, n) * extract_float(v1, d2);
		}
	}

	return(newVal(V_ORG(v1), V_SIZE(v1)[0], V_SIZE(v1)[1], V_SIZE(v1)[2], FLOAT, out));
}




static Var *do_my_convolve(Var *obj, Var *kernel, int norm, float ignore, int kernreduce)
{

  float *data;                             /* the smoothed array */
  int a, b, c;                             /* x, y, z position of kernel element */
  int x_pos, y_pos, z_pos;                 /* x, y, z position of object element */
  int obj_x, obj_y, obj_z;                 /* total x, y, z dimensions of object */
  int kx_center, krn_x;                    /* x radius and total x dimension of kernel */
  int ky_center, krn_y;                    /* y radius and total y dimension of kernel */
  int kz_center, krn_z;                    /* z radius and total z dimension of kernel */
  int x, y, z;                             /* x, y, z position of object convolved element */
  int i, j, k;                             /* memory locations */
  float kval, oval;                        /* value of kernel and object element */
  float *wt;                               /* array of number of elements in sum */
  int objsize;                             /* total 1-d size of object */
  int a_opp, b_opp, c_opp;                 /* x, y, z position of anti-symmetric kernel element */
  int x_opp, y_opp, z_opp;                 /* x, y, z position of anti-symmetric kernel element */

  obj_x = GetX(obj);
  obj_y = GetY(obj);
  obj_z = GetZ(obj);
  krn_x = GetX(kernel);
  krn_y = GetY(kernel);
  krn_z = GetZ(kernel);
  kx_center = krn_x/2;
  ky_center = krn_y/2;
  kz_center = krn_z/2;
  
  objsize = obj_x * obj_y * obj_z;

  data = (float *)calloc(sizeof(float), objsize);
  wt = (float *)calloc(sizeof(float), objsize);

  if (data == NULL || wt == NULL) {
    parse_error("Unable to allocate memory");
    return NULL ;
  }

  if(kernreduce==0) {
 
    for (i = 0 ; i < objsize ; i++) {                           /* loop through every element in object */
      if(extract_float(obj, i) == ignore) { data[i] = ignore; continue; }
      xpos(i, obj, &x, &y, &z);                                 /* compute current x,y,z position in object */
      for (a = 0 ; a < krn_x ; a++) {                           /* current x position of kernel */
	x_pos = x + a - kx_center;                              /* where the current operation is being done in x */
	if (x_pos < 0 || x_pos >= obj_x) continue;
	for (b = 0 ; b < krn_y ; b++) {                         /* current y position of kernel */    
	  y_pos = y + b - ky_center;                            /* where the current operation is being done in y */
	  if (y_pos < 0 || y_pos >= obj_y) continue;
	  for (c = 0 ; c < krn_z ; c++) {                       /* current z position of kernel */
	    z_pos = z + c - kz_center;                          /* where the current operation is being done in z */
	    if (z_pos < 0 || z_pos >= obj_z) continue;
	    j = cpos(x_pos, y_pos, z_pos, obj);                 /* position in 1-D object at (x_pos,y_pos,z_pos) */
	    k = cpos(a, b, c, kernel);                          /* position in 1-D kernel at (a,b,c) */
	    kval = extract_float(kernel,k);                     /* value of kernel at (k) */
	    oval = extract_float(obj, j);                       /* value of object at (j) */
	    if (oval != ignore && kval != ignore) {
	      wt[i] += kval;                                    /* sum of values in used pixels of kernel*/
	      data[i] += kval * oval;
	    }
	  } 
	}
      }

      if (norm != 0 && wt[i] != 0) {
	data[i] /= wt[i];
      }
    } 
  }

  if(kernreduce==1) {

    for (i = 0 ; i < objsize ; i++) {                           /* loop through every element in object */
      if(extract_float(obj, i) == ignore) { data[i] = ignore; continue; }
      xpos(i, obj, &x, &y, &z);                                 /* compute current x,y,z position in object */
      for (a = 0 ; a < krn_x ; a++) {                           /* current x position of kernel */
	a_opp = krn_x - a - 1;                                  /* anti-symmetric x position of kernel */
	x_pos = x + a - kx_center;                              /* where the current operation is being done in x */
	x_opp = x + a_opp - kx_center;                          /* anti-symmetric x position of object */
	if (x_pos < 0 || x_pos >= obj_x || x_opp < 0 || x_opp >= obj_x) continue;
	for (b = 0 ; b < krn_y ; b++) {                         /* current y position of kernel */    
	  b_opp = krn_y - b - 1;                                /* anti-symmetric y position of kernel */
	  y_pos = y + b - ky_center;                            /* where the current operation is being done in y */
	  y_opp = y + b_opp - ky_center;                        /* anti-symmetric y position in object */
	  if (y_pos < 0 || y_pos >= obj_y || y_opp < 0 || y_opp >= obj_y) continue;
	  for (c = 0 ; c < krn_z ; c++) {                       /* current z position of kernel */
	    c_opp = krn_z - c - 1;                              /* anti-symmetric z position of kernel */
	    z_pos = z + c - kz_center;                          /* where the current operation is being done in z */
	    z_opp = z + c_opp - kz_center;                      /* anti-symmetric z position in object */
	    if (z_pos < 0 || z_pos >= obj_z || z_opp < 0 || z_opp >= obj_z) continue;
	    j = cpos(x_pos, y_pos, z_pos, obj);                 /* position in 1-D object at (x_pos,y_pos,z_pos) */
	    k = cpos(a, b, c, kernel);                          /* position in 1-D kernel at (a,b,c) */
	    kval = extract_float(kernel,k);                     /* value of kernel at (k) */
	    oval = extract_float(obj, j);                       /* value of object at (j) */
	    if (oval != ignore && kval != ignore) {
	      wt[i] += kval;                                    /* sum of used pixels in kernel */
	      data[i] += kval * oval;
	    }
	  } 
	}
      }

      if (norm != 0 && wt[i] != 0) {
	data[i] /= wt[i];
      }
    } 
  }

  free(wt);
  return(newVal(V_ORG(obj), V_SIZE(obj)[0], V_SIZE(obj)[1], V_SIZE(obj)[2], FLOAT, data));
}


Var *ff_convolve2(vfuncptr func, Var * arg)
{
  Var *obj = NULL;                         /* the object to be smoothed */
  Var *kernel = NULL;                      /* the kernel with which to smooth */
  int norm = 1;                            /* initializing normalization to 1 */
  float ignore = 0;                        /* initializing ignore to 0 */
  int kernreduce = 0;                      /* option to use kernel reduction near obj boundaries */

  Alist alist[6];
  alist[0] = make_alist("object",       ID_VAL,         NULL,	      &obj);
  alist[1] = make_alist("kernel",       ID_VAL,         NULL,      &kernel);
  alist[2] = make_alist("normalize",    INT,            NULL,        &norm);
  alist[3] = make_alist("ignore",       FLOAT,          NULL,      &ignore);
  alist[4] = make_alist("kernreduce",   INT,            NULL,  &kernreduce);
  alist[5].name = NULL;
  
  if (parse_args(func, arg, alist) == 0) return(NULL);
  
  if (obj == NULL && kernel == NULL) {
    parse_error("convolve2() - 4/25/04");
    parse_error("Convolve function that will NOT fill in nullvalues of smoothed array with interpolated points.");
    parse_error("Instead it will fill null pixels in with the null value.");
    parse_error("Also provides for kernels that contain float values.\n");
    parse_error("Syntax:  b = convolve2(object, kernel, normalize, ignore, kernreduce)");
    parse_error("example: b = convolve2(a,clone(1.0,10,10,1,),ignore=0,0)");
    parse_error("object - may be any 1,2 or 3-D array.");
    parse_error("kernel - the convolve kernel, may also be any 1,2 or 3-d array.");
    parse_error("normalize - whether or not to normalize the output to the weight of the kernel. Default is 1 (yes).");
    parse_error("ignore - the null value of pixels to be ignored in calculation. Default is 0.");
    parse_error("kernreduce - whether to reduce the kernel on both sides as calculation approaches object boundary. Default is 0 (no).\n");
    return NULL;
  }
  if (obj == NULL) {
    parse_error("No object specified\n");
    return NULL;
  }
  if (kernel == NULL) {
    parse_error("No kernel specified\n");
    return NULL;
  }

  return(do_my_convolve(obj, kernel, norm, ignore, kernreduce));
}


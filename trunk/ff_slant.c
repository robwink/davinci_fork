#include "parser.h"

Var *
ff_slant(vfuncptr func, Var * arg)
{
  Var *obj = NULL;
  Var *ival= NULL;
  Var *out = NULL;
  int i,j,k;
  int x,y,z;
  int *leftmost = NULL,*rightmost = NULL;
  int w, width;
  Var *a = NULL;
  float ignore;
  float v;
  float *odata = NULL;

  Alist alist[3];
  alist[0] = make_alist("object",		ID_VAL,		NULL,	&obj);
  alist[1] = make_alist("ignore",		ID_VAL,		NULL,	&ival);
  alist[2].name = NULL;

  if (parse_args(func, arg, alist) == 0) return(NULL);

  if (obj == NULL) {
    parse_error("expected object");
    return(NULL);
  }

  if (ival) ignore = extract_float(ival,0);

  x = GetX(obj);
  y = GetY(obj);
  z = GetZ(obj);

  /*
  ** output is a column
  */
  leftmost = calloc(y, sizeof(int));
  rightmost = calloc(y, sizeof(int));
  
  for (i = 0 ; i < y ; i++) {
    leftmost[i] = x-1;
    rightmost[i] = 0;
  }

  for (j = 0 ; j < y ; j++) {
    for (i = 0 ; i < x ; i++) {
      for (k = 0 ; k < z ; k++) {
	v = extract_float(obj, cpos(i,j,k,obj));
	if (ival && v == ignore) continue;
	if (i < leftmost[j]) {
	  leftmost[j] = i;
	}
	if (i > rightmost[j]) {
	  rightmost[j] = i;
	}
      }
    }
  }

  /* find maximum width */
  width = 0;
  for (i = 0 ; i < y ; i++) {
    w = rightmost[i] - leftmost[i] +1;
    if (w > width) width = w;
  }

  /* fix leftmost to allow for maximum width */
  for (i = 0 ; i < y ; i++) {
    leftmost[i] = min(leftmost[i], x-width);
  }

  odata = calloc(width*y*z,4);
  out = newVal(BSQ, width, y, z, FLOAT, odata);

  for (j = 0 ; j < y ; j++) {
    for (i = leftmost[j] ; i < leftmost[j]+width ; i++) {
      for (k = 0 ; k < z ; k++) {
	odata[cpos(i-leftmost[j], j, k, out)] = extract_float(obj, cpos(i, j, k, obj));
      }
    }
  }

  a = new_struct(3);
  add_struct(a, "data", out);
  add_struct(a, "leftedge", newVal(BSQ, 1, y, 1, INT, leftmost));
  add_struct(a, "width", newInt(x));

  return(a);
}

Var *
ff_unslant(vfuncptr func, Var * arg)
{
	Var *obj = NULL;
	Var *ival= NULL;
	Var *out;
	int i,j,k;
	int x,y,z;
	int *leftmost;
	int width;
	Var *data, *leftedge, *w;
	float ignore;
	float *odata;
	int p;

	Alist alist[3];
	alist[0] = make_alist("object",	ID_STRUCT,		NULL,	&obj);
	alist[1] = make_alist("ignore",	ID_VAL,		NULL,	&ival);
	alist[2].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (find_struct(obj, "data", &data) == -1 ||
	    find_struct(obj, "leftedge", &leftedge) == -1 ||
	    find_struct(obj, "width", &w) == -1) {
		parse_error("Structure doesn't appear to have members for unslanting.");
		return(NULL);
	}

	x = GetX(data);
	y = GetY(data);
	z = GetZ(data);
	parse_error("input y dim in unslant: %i", y);
	if (ival) ignore = extract_float(ival, 0);
	width = extract_int(w, 0);
	leftmost = V_DATA(leftedge);

	odata = calloc(width*y*z, sizeof(float));
	out = newVal(BSQ, width, y, z, FLOAT, odata);

	for (j = 0 ; j < y ; j++) {
	  parse_error("leftmost[%i]: %i", j, leftmost[j]);
		for (i = 0 ; i < width ; i++) {
			for (k = 0 ; k < z ; k++) {
				p = cpos(i,j,k,out);
				if (i >= leftmost[j] && i < leftmost[j]+x) {
					odata[p] = extract_float(data, cpos(i-leftmost[j],j,k, data));
				} else {
					odata[p] = ignore;
				}
			}
		}
	}
	return(out);
}
Var * ff_unslant_shear(vfuncptr func, Var * arg) {
  

  Var * object = NULL, * dv_nullvalue = NULL, *dv_trim = NULL;

  Var * data = NULL, * angle = NULL, * width = NULL, *leftedge = NULL;

  Var * out = NULL;

  float angle_f;


  int trim = 0, errexit = 0;
  int this_line = 0;
  int b,w,x,y,z,i,j,k,l, lsize, elements, fin_pos, yprime;
  float nullvalue_f = -32768.0, s;

  float *odata;
  int * leftmost;
  Alist alist[4];

  alist[0] = make_alist("object", ID_STRUCT, NULL, &object);
  alist[1] = make_alist("ignore", ID_VAL, NULL, &dv_nullvalue);
  alist[2] = make_alist("trim", ID_VAL, NULL, &dv_trim);
  alist[3].name = NULL;
  
  if (parse_args(func, arg, alist) == 0) return NULL;

  if (dv_nullvalue) nullvalue_f = extract_float(dv_nullvalue, 0);
  if (dv_trim) trim = extract_int(dv_trim, 0);
  
  if (find_struct(object, "data", &data) == -1) {
    parse_error("Object structure requires 'data' member.");
    errexit = 1;
  }
  if (find_struct(object, "angle", &angle) == -1) {
    parse_error("Object structure requires 'angle' member.");
    errexit = 1;
  }
  if (find_struct(object, "width", &width) == -1) {
    parse_error("Object structure requires 'width' member.");
    errexit = 1;
  }
  if (find_struct(object, "leftedge", &leftedge) == -1) {
    parse_error("Object structure requires 'leftedge' member.");
    errexit = 1;
  }

  if (errexit) return NULL;
  
  if (dv_nullvalue) nullvalue_f = extract_float(dv_nullvalue, 0);
  if (dv_trim) trim = extract_int(dv_trim, 0);
  
  x = GetX(data); y=GetY(data); z=GetZ(data);
  parse_error("input y dim in unslant_shear: %i", y);
  leftmost = V_DATA(leftedge);

  w = extract_int(width, 0);
  /* We invert the angle for our purposes */
  angle_f = -extract_float(angle, 0);
  if (fabs(angle_f) > 80.0) {
    parse_error("Object angle member must be between -80 and 80.");
    errexit = 1;
  }

  if (errexit) return NULL;

  if (angle_f == 0.0) {
    parse_error("Shearing by 0 degrees has no effect.  Use unslant() instead.");
    return NULL;
  }
  
  s = tan((M_PI/180.0) * angle_f);
  l = (int)((x * fabs(s) -0.5) + 1)*(s/fabs(s));
  lsize = (int)((w * fabs(s) -0.5) + 1)*(s/fabs(s));
  b = sizeof(float);
  
  elements = w * (y + abs(lsize)) * z;

  parse_error("elements computed: %i  s: %f  l: %i lsize: %i array_size: (%i,%i,%i)", 
	      elements,s ,l,lsize, w,(y-abs(lsize)),z);
  if (elements <= 0) {
    parse_error("Elements computed to %i.  Cannot allocate.", elements);
    return NULL;
  }
  /* Use malloc to save time, since we're going to initialize the area ourselves. */
  odata = (float *)malloc(elements * b);
  out = newVal(BSQ, w, y-abs(lsize), z, FLOAT, odata);

  /* Initialize odata with null value*/
  for (i=0; i<elements; odata[i++] = nullvalue_f);
  
  
  for (k=0; k<z; k++) {
    for (j=0; j<y; j++) {
      //parse_error("leftmost[%i]: %i", j, leftmost[j]);
      for(i=0; i<w; i++) {
	/* compute yprime */
	if (l>0) {
	  //yprime = (int)(abs(l)-(int)(s*i+0.5));
	  yprime = (int)(fabs(s)*i+0.5);
	}
	else {
	  //yprime = (int)(fabs(s) * i + 0.5);
	  yprime = - (int)(fabs(s) * i + 0.5) - lsize;
	}
	fin_pos = cpos(i,j-yprime,k,out); /* final position */
	if (fin_pos > elements) {
	  this_line++;
	}
	else {
	  if (i >= leftmost[j] && i<leftmost[j]+x) {
	    odata[fin_pos] = extract_float(data, cpos(i-leftmost[j],j,k,data));
	  }
	  else {
	    //odata[fin_pos] = extract_float(data, cpos(i-leftmost[j],j,k,data));
	    odata[fin_pos] = nullvalue_f;
	  }
	}
      }
    }
  }
  if (this_line) {
    parse_error("Image boundary limits exceeded %i time(s)", this_line);
  }
  return(out);
}

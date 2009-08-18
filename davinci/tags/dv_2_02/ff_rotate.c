#include "parser.h"
#include <math.h>

static int rot_round(float numf);
static Var *rotation(Var *obj, float angle, float ign);

Var *ff_rotation(vfuncptr func, Var * arg)
{

  Var   *obj = NULL;
  Var   *out = NULL;
  float  ign = 0.0;
  float  angle = 0.0;

  Alist alist[4];
  alist[0] = make_alist( "object",      ID_VAL,   NULL,     &obj);
  alist[1] = make_alist( "angle",  	FLOAT,    NULL,     &angle);
  alist[2] = make_alist( "ignore",      FLOAT,    NULL,     &ign);
  alist[3].name = NULL;

  if (parse_args(func, arg, alist) == 0) return NULL;
  
  if (obj == NULL) {
    parse_error("rotate() - Mon Feb 13 14:35:25 MST 2006");
    parse_error("Rotates an array a specified angle in degrees");
    parse_error("Uses 3 shear Paeth algorithm.\n");
    parse_error("Syntax:  b = rotate(obj,angle,ignore)");
    parse_error("example: b = rotate(a,5.0)");
    parse_error("example: b = rotate(a,-64.3,ignore=-32768)");
    parse_error("obj     - any 3-d BSQ array.");
    parse_error("angle   - desired angle of rotation in degrees.");
    parse_error("ignore  - the value in non-data pixels. Default is 0.");
    return NULL;
  }

  out=rotation(obj, angle, ign);
  return(out);
}



union DATA {
    char   b;
    short  s;
    int    i;
    float  f;
    double d;
};



Var *rotation(Var *obj, float angle, float ign)
{

  Var   *out = NULL, *s = NULL;
  int    x=0, y=0, z=0;              /* input dimensions                      */
  int    rx=0, ry=0;
  int    i, j, k;                    /* array indices                         */
  int   *roffset = NULL;
  char  *robj = NULL;
  float  pi = 3.141592653589;
  float  ang = 0.0;                  /* the rotation angle in radians         */
  int   *xshear = NULL;              /* first and third shears in three shear */
  int   *yshear = NULL;              /* second shear in three shear operation */
  int    xdim_shr1 = 0;              /* x dimension after first x shear       */
  int    ydim_shr1 = 0;              /* y dimension after first y shear       */
  int    xdim_shr2 = 0;              /* x dimension after second x shear      */
  int    ti = 0;                     /* temporary integer value               */
  float  tf = 0.0;                   /* temporary float value                 */
  int    minx1=0, minx2=0, miny=0;
  int    maxx1=0, maxx2=0, maxy=0;
  int    top,bot,lef,rig;
  int    x_1=0, y_1=0, x_2=0;
  int    flpflg = 0;
  int    nbytes = 0;
  union DATA ign_resized; /* properly resized value of ignore */

  switch(V_FORMAT(obj)){
  case BYTE:
		ign_resized.b = ign;
		break;
  case SHORT:
		ign_resized.s = ign;
		break;
  case INT:
		ign_resized.i = ign;
		break;
  case FLOAT:
		ign_resized.f = ign;
		break;
  case DOUBLE:
		ign_resized.d = ign;
		break;
  default:
		break;
  }
  
  x = GetX(obj);
  y = GetY(obj);
  z = GetZ(obj);
	
  /* set the angle between -pi/2.0 and pi/2.0 */
  ang = angle - ((int)angle/360)*360;
  if(ang < 0) ang = ang + 360;  // ang is now between 0 and 360
  if(ang > 180.0) ang -= 360;   // ang is now between -180 and 180
  ang = ang*2*pi/360.0;         // ang is now betweeen -pi and pi
  if(ang > pi/2.0) {
    ang = ang - pi;
    flpflg = 1;                 // this means I've assumed the image to be flipped 180 degrees
  }
  if(ang < -pi/2.0) {
    ang = ang + pi;
    flpflg = 1;                 // this means I've assumed the image to be flipped 180 degrees
  }
	
  /* x-dimension of array after the first x-shear */
  tf = (float)y * tan(ang/2.0);
  ti = rot_round(tf);
  xdim_shr1 = x + abs(ti);
  if(ti < minx1) minx1 = ti;
  if(ti > maxx1) maxx1 = ti;
	
  /* y-dimension of array after the first y-shear */
  tf = (float)xdim_shr1 * -1 * sin(ang);
  ti = rot_round(tf);
  ydim_shr1 = y + abs(ti);
  if(ti < miny) miny = ti;
  if(ti > maxy) maxy = ti;
	
  /* x-dimension of array after the second x-shear */
  tf = (float)ydim_shr1 * tan(ang/2.0);
  ti = rot_round(tf);
  xdim_shr2 = xdim_shr1 + abs(ti);
  if(ti < minx2) minx2 = ti;
  if(ti > maxx2) maxx2 = ti;
	
  /* create shear lookup arrays */
  yshear = (int *)malloc(sizeof(int)*xdim_shr2);
  xshear = (int *)malloc(sizeof(int)*ydim_shr1);
	
  /* fill in shearing lookup arrays */
  for(i=0;i<xdim_shr2;i++) { yshear[i] = rot_round((float)i * sin(ang)); }
  for(j=0;j<ydim_shr1;j++) { xshear[j] = rot_round((float)j * -1 * tan(ang/2.0)); }
	
  /* set the default edges of rotated array */
  top = ydim_shr1;
  bot = ydim_shr1*-1;
  lef = xdim_shr2;
  rig = xdim_shr2*-1;
	
  /* search for edges of data in rotated array */
  for(j=0;j<y;j++) {
    for(i=0;i<x;i++) {
      x_1 = i - xshear[j] - minx1;
      y_1 = j - yshear[x_1] - miny;
      x_2 = x_1 - xshear[y_1] - minx2;
      
      for(k=0;k<z;k++) {
				if(flpflg == 1) {
					tf = extract_float(obj, cpos((x-1)-i,(y-1)-j,k,obj));
				} else {
					tf = extract_float(obj, cpos(i,j,k,obj));
				}
				
				if (tf != ign) {
					if (y_1 < top) top = y_1;
					if (y_1 > bot) bot = y_1;
					if (x_2 < lef) lef = x_2;
					if (x_2 > rig) rig = x_2;
				}
      }
    }
  }
	
  /* create roffset */
  roffset = (int *)malloc(sizeof(int)*2);
  roffset[0] = lef;
  roffset[1] = top;
	
  /* create robj */
  rx = rig-lef+1;
  ry = bot-top+1;
	
  /* create new output var */
  s = newVar();
  V_TYPE(s) = V_TYPE(obj);
  V_ORG(s) = V_ORG(obj);
  V_FORMAT(s) = V_FORMAT(obj);
  V_SIZE(s)[orders[V_ORG(s)][0]] = rx;
  V_SIZE(s)[orders[V_ORG(s)][1]] = ry;
  V_SIZE(s)[orders[V_ORG(s)][2]] = z;
  V_DSIZE(s) = rx*ry*z;
  V_DATA(s) = calloc(nbytes = NBYTES(V_FORMAT(s)), V_DSIZE(s));
  robj = (char *)V_DATA(s);
	
  /* fill in every element of rotated array with null value */
  for(k=0;k<rx*ry*z;k++) { memcpy(robj+k*nbytes, &ign_resized, nbytes); }
	
  /* every element in robj must be filled in */
  for(j=0;j<y;j++) {
    for(i=0;i<x;i++) {
      x_1 = i - xshear[j] - minx1;
      y_1 = j - yshear[x_1] - miny;
      x_2 = x_1 - xshear[y_1] - minx2;
      
      /* if not outside data fill in with value */
      if(x_2 >= lef && x_2 <= rig && y_1 >= top && y_1 <= bot) {
				for(k=0;k<z;k++) {
					int xx, yy, zz;
					
					xx = flpflg? (x-1)-i: i;
					yy = flpflg? (y-1)-j: j;
					zz = k;
					tf = extract_float(obj, cpos(xx,yy,zz,obj));
					
					if(tf != ign) {
						memcpy(((char *)V_DATA(s))+cpos(x_2-lef,y_1-top,k,s)*nbytes,
									 ((char *)V_DATA(obj))+cpos(xx,yy,zz,obj)*nbytes,
									 nbytes); 
					}
				}
      }
    }
  }
	
  /* clean up */
  free(yshear);
  free(xshear);
	
  /* final output */
  //free(roffset);
  //return(s);
	
	
  out = new_struct(0);
  add_struct(out, "data", s);
  add_struct(out, "angle", newFloat(angle));
  add_struct(out, "offset", newVal(BSQ, 2, 1, 1, INT, roffset));
  return out;
}



static int rot_round(float numf) {
  float    m;
  int      numi;

  /* calculate the rotated y coordinate */
  numi = (int)numf;
  m = numf - (float)numi;
  if (m >= 0.5) { numi += 1; }
  if (m <= -0.5) { numi -= 1; }

  return(numi);
}

#include "parser.h"

/*
**   . a . 
**   b p c 
**   . d . 
*/

#define deg(theta) (theta)*M_PI/180.0

/*
** azimuth -  Azimuth of the source, measured from up, clockwise, in deg
** elevation - Elevation of source, measured from plane, in deg.  Vertical = 90
** pixsize - size of a pixel relative to a DN of height. 
*/

void sphrec(float lon, float lat, float *x, float *y, float *z);

/*
 * shade.c
 * Mark Mann
 * Wed May  4 1994
 */

Var *
ff_shade(vfuncptr func, Var * arg) 
{
	int i, j;
	float *odata;
	double		gx, gy, gz, sx, sy, sz;
	int x, y;
	double left, up, val;
	
	Var *obj, *out;
	double sun_elevation = 45;
	double sun_azimuth = 45;
	double cosI, cosE;
	double scale = 1.0;
	
	Alist alist[5];
	alist[0] = make_alist( "object",    ID_VAL,    NULL, &obj);
	alist[1] = make_alist( "angle",     DOUBLE,    NULL,    &sun_elevation);
	alist[2] = make_alist( "azimuth",     DOUBLE,    NULL,  &sun_azimuth);
	alist[3] = make_alist( "scale",     DOUBLE,    NULL,  &scale);
	alist[4].name = NULL;
	
	if (parse_args(func, arg, alist) == 0) return(NULL);
	
	if (obj == NULL) {
		parse_error("shade()");
    parse_error("\nUsed to shade DEM data");
    parse_error("Takes an input DEM and sun orientation and illuminates the DEM");
    parse_error("object = data to be shaded");
    parse_error("angle = sun elevation angle in degrees, measured from up");
    parse_error("azimuth = sun azimuth in degrees with vertical at 90");
    parse_error("scale = the size of a pixel relative to a DN of height");
    return NULL;
	}
	
	if(GetZ(obj) > 1 ) {
		parse_error("Single band data only\n");
		return NULL;
	}
	x = GetX(obj);
	y = GetY(obj);
	
	/*
	 * Get ground to sun vector
	 *   (-1,0,0)  Sun setting in west
	 *   ( 0,0,1)  Sun at noon
	 *   ( 1,0,0)  Sun rising in east
	 */
	sx = cos(sun_azimuth * M_PI/180.0);
	sy = sin(sun_azimuth * M_PI/180.0);
	sz = sin((M_PI/2) - (sun_elevation * M_PI/180.0));
	
	printf("Sun vector: %.4f %.4f %.4f\n", sx, sy, sz);
	
	odata = calloc(x*(size_t)y, sizeof(float));
	
	for (j = 1 ; j < y ; j+=1) {
		for (i = 1 ; i < x ; i+=1) {
			left = extract_double(obj, cpos(i-1, j, 0, obj));
			up = extract_double(obj, cpos(i, j-1, 0, obj));
			val = extract_double(obj, cpos(i, j, 0, obj));
			
			gx = (val - left) / scale;
			gy = (val - up) / scale;
			if(sqrt(gx*gx +gy*gy)==0) {
				gz=1;
			} else{
				gz = 1.0 / sqrt(gx*gx + gy*gy);
			}
			
			/* n = H(gx, gy, gz) */
			/* E = H(0,0,1) . n */
			/* I = sun . n */
			/* R = 1/(1+cos(E)/cos(I)) */
			
			/*  
					v1 = H(-1, 0, gx/scale)
					v2 = H(0, -1, gy/scale)
					n = (gx, -gy, gz)
			*/
			
			cosE = gz;
			cosI = sx*gx + sy*gy + sz*gz;
			
			odata[cpos(i,j, 0, obj)] = 1.0/ (1.0+cosE/cosI);
		}
	}
	out = newVal(BSQ, x, y, 1, FLOAT, odata);
	return(out);
}

Var *
ff_shade2(vfuncptr func, Var * arg) 
{
    int		l, s;
    unsigned char     *odata;
    double	sun_angle = -45.0;
    double	dx = 1.0;
    double		gx, gy, sx, sy;
    double		drop;
    double		shadow_height;
    int x, y;
	size_t pos;
	double v;
	int no_shade = 0;
	float theta, ctheta, color;
	float csun, ssun;

    Var *obj, *out;
	
    Alist alist[5];
    alist[0] = make_alist( "object",    ID_VAL,    NULL, &obj);
    alist[1] = make_alist( "pixdist",   DOUBLE,     NULL,    &dx);
    alist[2] = make_alist( "angle",     DOUBLE,    NULL,    &sun_angle);
    alist[3] = make_alist( "noshadow",  INT,       NULL,    &no_shade);
    alist[4].name = NULL;

    if (parse_args(func, arg, alist) == 0) return(NULL);

    if (obj == NULL) {
        parse_error("No object specified.\n");
        return(NULL);
    }

    x = GetX(obj);
    y = GetY(obj);

    /*
     * Get ground to sun vector
     *   (-1,0)  Sun setting in west
     *   ( 0,1)  Sun at noon
     *   ( 1,0)  Sun rising in east
     */
    sx = cos((90.0 - sun_angle) * M_PI / 180.0);
    sy = sin((90.0 - sun_angle) * M_PI / 180.0);

    printf("Sun vector  x:%12.8f  y:%12.8f   pixdist:%f\n", sx, sy, dx);
	csun = cos(-sun_angle);
	ssun = sin(-sun_angle);

    if (sx == 0.0) {
        drop = 50000.0;
    } else {
        drop = fabs(sy/sx) * dx;
    }

    printf("Sun vector is %f\n", atan2(sy, sx) * 180 / M_PI);

    odata = calloc(x*(size_t)y, 1);
    out = newVal(BSQ, x, y, 1, BYTE, odata);

    gx = dx;

    for (l=0 ; l<y ; l++) {
        if (sun_angle < 0) {
            shadow_height = extract_double(obj, 0) - 40000.0;
            for (s=1; s< x-1; s++) {
                /*
                 * Compute cos(theta) where theta is angle between
                 * ground to sun vector and the normal to the ground
                 * slope
                 *
                 * cos(theta) = (G dot S)/(|G| * |S|)
                 *
                 *   Sun vector is (sx, sy)
                 *     Magnitude of sun vector is 1
                 *   Ground slope vector is (gx, gy)
                 *   Normal to ground slope vector is (-gy, gx)
                 *
                 * cos(theta) = ((-gy,gx)dot(sx,sy))/(|(-gy,gx)|)
                 *            = (-gy*sx + gx*sy)/sqrt(gy*gy + gx*gx)
                 *
                 * Must normalize so that max is 255 min is 0
                 *
                 * Output value = 255 * cos(theta)
                 */
				pos = cpos(s, l, 0, obj);
			    v = extract_double(obj,pos);
                if (no_shade == 0  && v < shadow_height) {
                    odata[s] = 0;
                    shadow_height -= drop;
                } else {
                    gy = (int)v - (int) extract_double(obj, pos-1);

					ctheta = (-gy*sx + gx*sy) / sqrt(gy*gy + gx*gx);
					theta = acos(ctheta);
                    color = 127*(ctheta * csun - sin(theta) * ssun)+127;
					if (color < 0) color = 0;
					if (color > 255) color = 255;
                    odata[s] = color;
                    shadow_height = (int)v - drop;
                }
            }
            odata[0] = odata[1];
        } else {
            shadow_height = extract_double(obj, cpos(x-1, l, 1, obj)) - 40000.0;
            for (s=x-2; s>=0; s--) {
				pos = cpos(s, l, 0, obj);
			    v = extract_double(obj,pos);
                if (no_shade == 0 && v < shadow_height) {
                    odata[s] = 0;
                    shadow_height -= drop;
                } else {
                    gy = (int)extract_double(obj, cpos(l, s+1, 0, obj)) - (int)v;
					ctheta = (-gy*sx + gx*sy) / sqrt(gy*gy + gx*gx);
					theta = acos(ctheta);
                    color = 127*(ctheta * csun - sin(theta) * ssun)+127;
					if (color < 0) color = 0;
					if (color > 255) color = 255;
					odata[s] = color;

                    shadow_height = (int)v - drop;
                }
            }
            odata[x-1] = odata[x-2];
        }
        odata += x;
    }
	return(out);
}

/*
** Convert az, el into vector
*/
void sphrec(float lon, float lat, float *x, float *y, float *z)
{
	*x = cos(lat)*cos(lon);
	*y = cos(lat)*sin(lon);
	*z = sin(lat);
}

int GetDX(Var *obj) 
{
  switch (V_ORG(obj)) {
    case BSQ: return(1);
    case BIL: return(1);
    case BIP: return(V_SIZE(obj)[0]);
  }
  return 0;
}
int GetDY(Var *obj)
{
  switch (V_ORG(obj)) {
    case BSQ: return(V_SIZE(obj)[0]);
    case BIL: return(V_SIZE(obj)[0] * V_SIZE(obj)[1]);
    case BIP: return(V_SIZE(obj)[0] * V_SIZE(obj)[1]);
  }
  return 0;
}
int GetDZ(Var *obj)
{
  switch (V_ORG(obj)) {
    case BSQ: return(V_SIZE(obj)[0] * V_SIZE(obj)[1]);
    case BIL: return(V_SIZE(obj)[0]);
    case BIP: return(1);
  }
  return 0;
}

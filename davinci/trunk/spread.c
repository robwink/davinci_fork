#include "parser.h"
#include "func.h"

Var * 
ff_distance_map(vfuncptr func, Var * arg)
{
	int x, y, z;
	int dx,dy;
	Var *o1 = NULL;
	Var *o2 = NULL;
	Var *a;
	Var *ignore = NULL;
	float dir = 0.0;
	float rise, run;
	int *dist, *diff;

	int i,j,k, v,q;
	int pos, skip=0, skipv, radius=100;

	/* find bounding box of each piece */
	Alist alist[7];
	alist[0] = make_alist( "o1",    ID_VAL,    NULL,    &o1);
	alist[1] = make_alist( "o2",    ID_VAL,    NULL,    &o2);
	alist[2] = make_alist( "dir",   FLOAT,     NULL,    &dir);
	alist[3] = make_alist( "ignore",ID_VAL,    NULL,    &ignore);
	alist[4] = make_alist( "skip",  INT,       NULL,    &skip);
	alist[5] = make_alist( "radius",  INT,       NULL,    &radius);
	alist[6].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (o1 == NULL) {
		parse_error("%s: No object specified\n", func->name);
		return(NULL);
	}
	if (o2 == NULL) {
		parse_error("%s: No object specified\n", func->name);
		return(NULL);
	}

	x = GetSamples(V_SIZE(o1), V_ORG(o1));
	y = GetLines(V_SIZE(o1), V_ORG(o1));
	z = GetBands(V_SIZE(o1), V_ORG(o1));

	if (x != GetSamples(V_SIZE(o2), V_ORG(o2)) || 
		y != GetLines(V_SIZE(o2), V_ORG(o2)) ||
		z != GetBands(V_SIZE(o2), V_ORG(o2))) {

		parse_error("Objects must be same size");
		return(NULL);
	}

	/* for each pixel in src image, find first pixel in dest image, 
	   along directional vector 
	 */
	run = cos(dir);
	rise = -sin(dir);
	dist = (int *)calloc(x*y,sizeof(int));
	diff = (int *)calloc(x*y,sizeof(int));

	 for (j = 1 ; j < y ; j++) {
		 for (i = 1 ; i < x ; i++) {
		 	pos = cpos(i,j,0,o2);
		 	dist[pos] = -1;
			q = extract_int(o1,pos);
		 	if (q >= -32752) {
				skipv = skip;
				for (k = 0 ; ; k++) {
					dx = i+run*k;
					dy = j+rise*k;
					// if (dx == i && dy == j) continue; 	/* same as start */
					if (dx < 0 || dx > x-1) break;		/* fell off image */
					if (dy < 0 || dy > y-1) break;		/* fell off image */
					v = extract_int(o2,cpos(dx,dy,0,o2));
					if (v >= -32752) {
						if (skipv == 0) {
							dist[pos] = k;
							diff[pos] = (v-extract_int(o1,cpos(dx,dy,0,o1))) *
							            ((float)(radius-k)/radius);
							if (diff[pos] < 0) diff[pos] = 0;
							break;
						} else {
							skipv--;
						}
					}
				}
			}
		 }
	 }
	 a = new_struct(2);
	 add_struct(a, "dist", newVal(BSQ,x,y,1,INT,dist));
	 add_struct(a, "diff", newVal(BSQ,x,y,1,INT,diff));
	 return(a);
}

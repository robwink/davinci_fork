#include "parser.h"
#include "func.h"

Var * make_mask(int size, float dir);

Var * 
ff_distance_map(vfuncptr func, Var * arg)
{
	int x, y, z;
	int dx,dy;
	Var *o1 = NULL;
	Var *o2 = NULL;
	Var *a;
	Var *ignore = NULL;
	Var *dval, *mask;
	float dir = 0.0;
	float rise, run;
	int *dist, *diff;
	int smooth=0;
	int xmin,xmax,ymin,ymax;

	int i,j,k, v,q;
	int pos, skip=0, skipv, radius=100;

	Alist alist[8];
	alist[0] = make_alist( "o1",    ID_VAL,    NULL,    &o1);
	alist[1] = make_alist( "o2",    ID_VAL,    NULL,    &o2);
	alist[2] = make_alist( "dir",   FLOAT,     NULL,    &dir);
	alist[3] = make_alist( "ignore",ID_VAL,    NULL,    &ignore);
	alist[4] = make_alist( "skip",  INT,       NULL,    &skip);
	alist[5] = make_alist( "radius",  INT,       NULL,    &radius);
	alist[6] = make_alist( "smooth",  INT,       NULL,    &smooth);
	alist[7].name = NULL;

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

    xmin = x;
    ymin = y;
    xmax = 0;
    ymax = 0;
    for (j = 1 ; j < y ; j++) {
        for (i = 1 ; i < x ; i++) {
            pos = cpos(i,j,0,o2);
            dist[pos] = -1;
            q = extract_int(o1,pos);
            if (q >= -32752) {
                if (i < xmin) xmin = i;
                if (i > xmax) xmax = i;
                if (j < ymin) ymin = j;
                if (j > ymax) ymax = j;
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
	 dval = newVal(BSQ,x,y,1,INT,diff);
	 a = new_struct(3);
	 add_struct(a, "dist", newVal(BSQ,x,y,1,INT,dist));
	 add_struct(a, "diff", dval);

	 /*
	 ** Smooth the results as necessary
	 */
	 if (smooth) {
	 	Range r;
		Var *data, *s, *t;

	 	mask = make_mask(smooth, dir);
		r.step[0] = r.step[1]= r.step[2] = 1;
		r.lo[0]= xmin;
		r.hi[0]= xmax;
		r.lo[1]= ymin;
		r.hi[1]= ymax;
		r.lo[2]= r.hi[2]= 0;
		data = extract_array(dval, &r);
		s = do_convolve(data, mask, 1);
		t = V_DUP(dval);
        array_replace(t, s,  &r);
		add_struct(a, "mask", mask);
		add_struct(a, "data", data);
		add_struct(a, "s", s);
		add_struct(a, "t", t);
	 }

	 return(a);
}

Var *
make_mask(int size, float dir)
{
	Var *mask;
	int *mdata;
	int x1, x2, y1, y2;
	int ct;
	int flag;
	float rise, run;
	int i;

	if (size%2 == 0) size++;
	mdata = calloc(size*size, sizeof(int));
	mask = newVal(BSQ,size,size,1,INT,mdata);
	ct = size/2;

	/* create a mask that's 90 degrees rotated from the downtrack direction */

	run = cos(dir+M_PI_2);
	rise = -sin(dir+M_PI_2);

	flag = 0;
	for (i = 0 ; ; i++) {
		x1 = i*run + ct + 0.5;
		y1 = i*rise + ct + 0.5;
		if (x1 < 0 || x1 >= size || y1 < 0 || y1 >= size) {
			flag |= 1;
		} else {
			mdata[x1+y1*size] = 1;
		}

		x2 = (-i*run) + ct + 0.5;
		y2 = (-i*rise) + ct + 0.5;
		if (x2 < 0 || x2 >= size || y2 < 0 || y2 >= size) {
			flag |= 2;
		} else {
			mdata[x2+y2*size] = 1;
		}
		if (flag == 3) break;
	}
	return(mask);
}

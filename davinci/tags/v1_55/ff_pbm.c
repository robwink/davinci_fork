#include "parser.h"

#define MakeSize(a, x, y, z) { a[0] = x; a[1] = y; a[2] = z; }

Var * cut(Var *img, int x, int y, int z, int w, int h, int d);


/* cut out a portion of an image */
Var *
cut(Var *img, int x, int y, int z, int w, int h, int d)
{
    int size[3], start[3];
    int dim[3];
    char *axis[3] = { "x", "y", "z" };
    int i;
    Range r;
    
    MakeSize(dim,
             GetX(img),
             GetY(img),
             GetZ(img));
    MakeSize(start, x, y, z);
    MakeSize(size, w, h, d);

    for (i = 0 ; i < 3 ; i++) {
        /*
        ** Check limits
        */
        if (start[i] < -dim[i]) {
            parse_error("Start %s is too negative (%d).  Image's %s is %d",
                        axis[i], start[i], axis[i], dim[i]);
            return(NULL);
        }
        if (start[i] > dim[i]) {
            parse_error("Start %s is too large (%d).  Image's %s is %d", 
                        axis[i], start[i], axis[i], dim[i]);
            return(NULL);
        }
        /*
        ** Make negative numbers relative to the right hand side
        */
        
		if (start[i] == 0) start[i] = 1;
        if (start[i] < 0) start[i] += dim[i]+1;

        /*
        ** Check overall size
        */
        if (size[i] == 0) size[i] = dim[i] - start[i] + 1;

        if (size[i] < 0) {
			if (size[i] <= -dim[i]) {
				parse_error("%s size is too small (%d).  Image's %s is %d",
                        axis[i], size[i], axis[i], dim[i]);
				return(NULL);
			}
			size[i] = (dim[i] - start[i] + 1) + size[i];
		}

        if (start[i] + size[i]-1 > dim[i]) {
            parse_error("%s size is too large (%d).  Image's %s is %d", 
                        axis[i], 
						start[i] + size[i] -1,
						axis[i], dim[i]);
			return(NULL);
        }
	}
	for (i = 0 ; i < 3 ; i++) {
		r.lo[i] = start[i];
		r.hi[i] = start[i]+size[i]-1;
		r.step[i] = 0;
	}

	return(extract_array(img, &r));
}


Var *
ff_cut( vfuncptr func, Var *arg)
{
	Var *obj = NULL;
	int x=0, y=0, z=0, w=0, h=0, d=0;
	
	int ac;
	Var **av;
	Alist alist[8];
	alist[0] = make_alist( "object",   ID_VAL, NULL,     &obj);
	alist[1] = make_alist( "x",        INT,    NULL,     &x);
	alist[2] = make_alist( "y",        INT,    NULL,     &y);
	alist[3] = make_alist( "z",        INT,    NULL,     &z);
	alist[4] = make_alist( "width",    INT,    NULL,     &w);
	alist[5] = make_alist( "height",   INT,    NULL,     &h);
	alist[6] = make_alist( "depth",    INT,    NULL,     &d);
	alist[7].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (obj == NULL) return(NULL);

	return(cut(obj, x, y, z, w, h, d));
}

/* crop off portions of an image */
Var *
crop(Var *img)
{
	int top = 0, bottom = 0, left = 0, right = 0;
	/*
	** Examine the whole row/column to determine if it needs cropping
	*/

	Var *ul = cut(img, 0, 0, 0, 1, 1, 0);
	Var *ur = cut(img, -1, 0, 0, 1, 1, 0);
	Var *ll = cut(img, 0, -1, 0, 1, 1, 0);
	Var *lr = cut(img, -1, -1, 0, 1, 1, 0);

	if (pp_compare(ul, ur)) {
		top = 1;
		while(pp_compare(ul, cut(img, 0, top, 0, 0, 1, 0))) {
			top++;
		}
	}

	if (pp_compare(ul, ll)) {
		left = 1;
		while(pp_compare(ul, cut(img, left, 0, 0, 1, 0, 0))) {
			left++;
		}
	}

	if (pp_compare(ur, lr)) {
		right = -1;
		while(pp_compare(ur, cut(img, right, 0, 0, 1, 0, 0))) {
			right--;
		}
		right++;
	}

	if (pp_compare(lr, ll)) {
		bottom = -1;
		while(pp_compare(lr, cut(img, 0, bottom, 0, 0, 1, 0))) {
			bottom--;
		}
		bottom++;
	}

	return(cut(img, left, top, 0, right, bottom, 0));
}


Var *
ff_crop( vfuncptr func, Var *arg)
{
	Var *obj = NULL;

	int ac;
	Var **av;
	Alist alist[8];
	alist[0] = make_alist( "object",   ID_VAL, NULL,     &obj);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (obj == NULL) return(NULL);

	return(crop(obj));
}


/* scale an image */
Var *
scale(Var *img,
      int xscale,
      int yscale)
{
	return(NULL);
}


/* add padding to an image */
Var *
pad(Var *img,
    Var *color,
    int left,
    int right,
    int top,
    int bottom,
	int front,
	int back
    )
{
	
	return(NULL);
}

/* paste one image into another */
Var *
paste(Var *src, Var *dst, int x, int y, int z)
{
	
	return(NULL);
}

color()		/* colorize a grayscale image */
{

}


text()		/* create images of text */
{

}

cat()		/* concatenate two images, with pad */
{

}

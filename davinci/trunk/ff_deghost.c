#include "parser.h"

char *
createThemisGhostImage(int ghostDown,
					   int ghostRight,
					   const char *msg,
					   const char *stamp_id,
					   int imageW,
					   int imageH,
					   int bytesPerPixel,
					   void *imageDataBuff,
					   void *ghostDataBuff,
					   int reverse);
Var * make_band(Var *in, int band);


Var *
ff_deghost(vfuncptr func, Var * arg)
{

	Var *obj, *down, *right, *bands, *v, *out;
	char *id;
	int z, y, x, nbytes, i;
	void *out_data;
	char *msg;
	char prompt[256];
	int reverse = 0;

	Alist alist[7];
	alist[0] = make_alist("object",		ID_VAL,		NULL,	&obj);
	alist[1] = make_alist("bands",  	ID_VAL,	    NULL,	&bands);
	alist[2] = make_alist("down",  		ID_VAL,	    NULL,	&down);
	alist[3] = make_alist("right",  	ID_VAL,	    NULL,	&right);
	alist[4] = make_alist("id",  	    ID_STRING,	NULL,	&id);
	alist[5] = make_alist("reverse",  	INT,	    NULL,	&reverse);
	alist[6].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (obj == NULL) {
		parse_error("%s: No object specified\n", func->name);
		return(NULL);
	}
	if (down == NULL) {
		parse_error("%s: No downward offsets specified\n", func->name);
		return(NULL);
	}
	if (bands == NULL) {
		parse_error("%s: Bands not specified\n", func->name);
		return(NULL);
	}

	x = GetX(obj);
	y = GetY(obj);
	z = GetZ(obj);
	nbytes = NBYTES(V_FORMAT(obj));
	
	if (z != V_DSIZE(down)) {
		parse_error("%s: downward offsets don't match input image size\n", func->name);
		return(NULL);
	}
	if (z != V_DSIZE(bands)) {
		parse_error("%s: band numbers don't match input image size\n", func->name);
		return(NULL);
	}


	out = newVal(BSQ, x, y, z, V_FORMAT(obj), calloc(x*y*z,nbytes));

	for (i = 0 ; i < z ; i++) {
		v = make_band(obj, i);
		out_data = V_DATA(out)+cpos(0,0,i,out)*nbytes;
		if (extract_int(down,i) == 0 && (right == NULL || extract_int(right,i) == 0)) {
			memcpy(out_data, V_DATA(v), x*y*nbytes);
		} else  {
			sprintf(prompt, "Band %d: ", i+1);
			msg = 
				createThemisGhostImage(extract_int(down, i),		/* down */
								(right ? extract_int(right,i) : 0), /* right */
								prompt, 						/* msg */
								id, 						/* imageID */
								x, 							/* width */
								y, 							/* height */
								nbytes, 					/* nbytes */
								V_DATA(v), 			/* imageDataBuff */
								out_data,           /* ghostDataBuff */
								reverse); 			/* reverse direction */
			if (msg != NULL) {
				parse_error(msg);
				return(NULL);
			}
		}
	}
	return(out);
}

Var *
make_band(Var *in, int band)
{
	int x, y, z;
	int i, j ,k;
	int nbytes;
	Var *out;
	int p1, p2;

	x = GetX(in);
	y = GetY(in);
	z = GetZ(in);
	nbytes = NBYTES(V_FORMAT(in));

	out = newVal(BSQ, x, y, 1, V_FORMAT(in), calloc(x*y, nbytes));

	if (V_ORG(in) == BSQ) {
		p1 = cpos(0,0,band,in);
		memcpy(V_DATA(out), V_DATA(in)+p1*nbytes, nbytes*x*y);
	} else {
		for (i = 0 ; i< x ; i++) {
			for (j = 0 ; j< y ; j++) {
				p1 = cpos(x,y,band,in);
				p2 = cpos(x,y,0,out);
				memcpy(V_DATA(out)+p2*nbytes, V_DATA(in)+p1*nbytes, nbytes);
			}
		}
	}
	return(out);
}

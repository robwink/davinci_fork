#include "parser.h"
#include "func.h"

/**
 ** histogram()
 **
 ** rgb2hsi()
 ** hsi2rgb()
 ** threshold()
 ** saturate()
 ** scale()
 ** stretch()
 ** color_index()
 **/

/**
 ** compute histogram of an image
 **/

typedef struct { float r, g, b; } RGB;
typedef struct { float h, s, v; } HSV;
RGB HSVToRGB(HSV hsv);
HSV RGBToHSV(RGB rgb);

Var *
ff_histogram(vfuncptr func, Var * arg)
{
	Var *obj = NULL, *compress = NULL, *normalize = NULL;
	int type = 0;
	int x,y,z, n, i, j, dsize, low, high, v;
	int *data;
	char *ptr = NULL;

	int ac;
	Var **av;
	Alist alist[4];
	alist[0] = make_alist( "object",    ID_VAL,    NULL,     &obj);
	alist[1] = make_alist( "compress",  ID_VAL,    NULL,     &compress);
	alist[2] = make_alist( "normalize", ID_VAL,    NULL,     &normalize);
	alist[3].name = NULL;

	make_args(&ac, &av, func, arg);
	if (parse_args(ac, av, alist)) return(NULL);

	if (obj == NULL) {
		parse_error("%s: No object specified\n", func->name);
		return(NULL);
	}

	x = GetSamples(V_SIZE(obj), V_ORG(obj));
	y = GetLines(V_SIZE(obj), V_ORG(obj));
	z = GetBands(V_SIZE(obj), V_ORG(obj));
	dsize = V_DSIZE(obj);

	switch(V_FORMAT(obj)) {
		case BYTE: 
			low = 0; 
			high = 255; 
			break;
		case SHORT: 
			low = -32768;
			high = 32767;
			break;
		case INT:
			parse_error("%s(): %s not supported.\n", func->name, "INT");
			return(NULL);
		case FLOAT:
			parse_error("%s(): %s not supported.\n", func->name, "FLOAT");
			return(NULL);
		case DOUBLE:
			parse_error("%s(): %s not supported.\n", func->name, "DOUBLE");
			return(NULL);
	}

	data = (int *)calloc((high-low+1) * 2, sizeof(int));
	for (i = 0 ; i < high-low+1 ; i++) {
		data[i*2] = i+low;
	}

	for (i = 0 ; i < dsize ; i++) {
		v = extract_int(obj, i);
		data[(v-low)*2+1]++;
	}

	j = high-low+1;
	if (compress != NULL) {
		for (i = j = 0 ; i < high-low+1 ; i++) {
			if (data[i*2+1] != 0) {
				data[j*2] = data[i*2];
				data[j*2+1] = data[i*2+1];
				j++;
			}
		}
	}
	if (normalize) {
		float *fdata = (float *)calloc(2*j, sizeof(float));
		for (i = 0 ; i < j ; i++) {
			fdata[i*2] = data[i*2];
			fdata[i*2+1] = (float)data[i*2+1]/(float)dsize;
		}
		free(data);
		return(newVal(BSQ, 2, j, 1, FLOAT, fdata));
	} else {
		return(newVal(BSQ, 2, j, 1, INT, data));
	}
}

Var *
ff_rgb2hsv(vfuncptr func, Var * arg)
{
	Var *obj = NULL, *maxval = NULL;
	float *data;
	char *ptr = NULL;
	double mval;
	int x,y,z,i,j,k1,k2,k3;

	RGB a;
	HSV b;

	int ac;
	Var **av;
	Alist alist[2];
	alist[0] = make_alist( "object",    ID_VAL,    NULL,     &obj);
	alist[1] = make_alist( "maxval",  ID_VAL,    NULL,     &maxval);

	make_args(&ac, &av, func, arg);
	if (parse_args(ac, av, alist)) return(NULL);

	if (obj == NULL) {
		parse_error("%s: No object specified\n", func->name);
		return(NULL);
	}

	if (maxval == NULL) {
		switch (V_FORMAT(obj)) {
			case BYTE:		mval = (1 << 8)-1; break;
			case SHORT:		mval = MAXSHORT; break;
			case INT:		mval = MAXINT; break;
			case FLOAT:		mval = 1.0; break;
			case DOUBLE:	mval = 1.0; break;
		}
	} else {
		mval = extract_double(maxval, 0);
	}

	x = GetSamples(V_SIZE(obj), V_ORG(obj));
	y = GetLines(V_SIZE(obj), V_ORG(obj));
	z = GetBands(V_SIZE(obj), V_ORG(obj));

	if (z != 3) {
		parse_error("%s: Input must have 3 bands.\n", av[0]);
		return(NULL);
	}

	data = (float *)calloc(4, 3*x*y);
	for (i = 0 ; i < y ; i++) {
		for (j = 0 ; j < x ; j++) {
			k1 = cpos(j,i,0, obj);
			k2 = cpos(j,i,1, obj);
			k3 = cpos(j,i,2, obj);

			a.r = extract_double(obj, k1) / mval;
			a.g = extract_double(obj, k2) / mval;
			a.b = extract_double(obj, k3) / mval;

			b = RGBToHSV(a);

			data[k1] = b.h;
			data[k2] = b.s;
			data[k3] = b.v;
		}
	}
	return(newVal(V_ORG(obj), 
		V_SIZE(obj)[0], 
		V_SIZE(obj)[1], 
		V_SIZE(obj)[2], 
		FLOAT, data));
}

Var *
ff_hsv2rgb(vfuncptr func, Var * arg)
{
	Var *obj = NULL, *maxval = NULL;
	float *data;
	char *ptr = NULL;
	double mval;
	int x,y,z,i,j,k1,k2,k3;

	HSV a;
	RGB b;

	int ac;
	Var **av;
	Alist alist[2];
	alist[0] = make_alist( "object",    ID_VAL,    NULL,     &obj);
	alist[1] = make_alist( "maxval",  ID_VAL,    NULL,     &maxval);

	make_args(&ac, &av, func, arg);
	if (parse_args(ac, av, alist)) return(NULL);

	if (obj == NULL) {
		parse_error("%s: No object specified\n", func->name);
		return(NULL);
	}
	if (maxval == NULL) {
		mval = 1.0;
	} else {
		mval = extract_double(maxval, 0);
	}

	x = GetSamples(V_SIZE(obj), V_ORG(obj));
	y = GetLines(V_SIZE(obj), V_ORG(obj));
	z = GetBands(V_SIZE(obj), V_ORG(obj));

	if (z != 3) {
		parse_error("%s: Input must have 3 bands.\n", av[0]);
		return(NULL);
	}

	data = (float *)calloc(4, 3*x*y);
	for (i = 0 ; i < y ; i++) {
		for (j = 0 ; j < x ; j++) {
			k1 = cpos(j,i,0, obj);
			k2 = cpos(j,i,1, obj);
			k3 = cpos(j,i,2, obj);

			a.h = extract_double(obj, k1);
			a.s = extract_double(obj, k2);
			a.v = extract_double(obj, k3);

			b = HSVToRGB(a);

			data[k1] = b.r*mval;
			data[k2] = b.g*mval;
			data[k3] = b.b*mval;
		}
	}
	return(newVal(V_ORG(obj), 
		V_SIZE(obj)[0], 
		V_SIZE(obj)[1], 
		V_SIZE(obj)[2], 
		FLOAT, data));
}


HSV
RGBToHSV(RGB rgb)
{
    HSV hsv;
    float   mn, mx;
    float   rc, gc, bc;
    
    mx = max(max(rgb.r, rgb.g), rgb.b);
    mn = min(min(rgb.r, rgb.g), rgb.b);
    hsv.v = mx;
    if (mx == 0.0)
        hsv.s = 0.0;
    else
        hsv.s = (mx - mn) / mx;
    if (hsv.s == 0.0)
        hsv.h = 0.0;
    else {
        rc = (mx - rgb.r) / (mx - mn);
        gc = (mx - rgb.g) / (mx - mn);
        bc = (mx - rgb.b) / (mx - mn);
        if (rgb.r == mx)
            hsv.h = bc - gc;
        else if (rgb.g == mx)
            hsv.h = 2.0 + rc - bc;
        else if (rgb.b = mx)
            hsv.h = 4.0 + gc - rc;
 
        if (hsv.h < 0.0)
            hsv.h += 6.0;
        hsv.h = hsv.h / 6.0;
    }
    return hsv;
}

RGB
HSVToRGB(HSV hsv)
{
    RGB rgb;
    float   p, q, t, f;
    int i;
    
    if (hsv.s == 0.0)
		rgb.r = rgb.b = rgb.g = hsv.v;
    else {
        if (hsv.s > 1.0) hsv.s = 1.0;
        if (hsv.s < 0.0) hsv.s = 0.0;
        if (hsv.v > 1.0) hsv.v = 1.0;
        if (hsv.v < 0.0) hsv.v = 0.0;
        while (hsv.h >= 1.0)
            hsv.h -= 1.0;

        hsv.h = 6.0 * hsv.h;
        i = (int) hsv.h;
        f = hsv.h - (float) i;
        p = hsv.v * (1.0 - hsv.s);
        q = hsv.v * (1.0 - (hsv.s * f));
        t = hsv.v * (1.0 - (hsv.s * (1.0 - f)));
 
        switch(i) {
			case 0: rgb.r = hsv.v; rgb.g = t; rgb.b = p; break;
			case 1: rgb.r = q; rgb.g = hsv.v; rgb.b = p; break;
			case 2: rgb.r = p; rgb.g = hsv.v; rgb.b = t; break;
			case 3: rgb.r = p; rgb.g = q; rgb.b = hsv.v; break;
			case 4: rgb.r = t; rgb.g = p; rgb.b = hsv.v; break;
			case 5: rgb.r = hsv.v; rgb.g = p; rgb.b = q; break;
		}
	}
	return rgb;
}

/**
 ** Compute entropy of an image
 **/

Var *
ff_entropy(vfuncptr func, Var * arg)
{
	Var *obj = NULL;
	int i, dsize, count;
	void *a, *b, *data;
	int format, nbytes;
	float p, ent = 0;
	int (*cmp)(const void *, const void *);

	int ac;
	Var **av;
	Alist alist[2];
	alist[0] = make_alist( "object",    ID_VAL,    NULL,     &obj);
	alist[1].name = NULL;

	make_args(&ac, &av, func, arg);
	if (parse_args(ac, av, alist)) return(NULL);

	if (obj == NULL) {
		parse_error("%s: No object specified\n", func->name);
		return(NULL);
	}

	dsize = V_DSIZE(obj);
	format = V_FORMAT(obj);
	nbytes = NBYTES(V_FORMAT(obj));
	data = memdup(V_DATA(obj), dsize * NBYTES(V_FORMAT(obj)));

	switch(format) {
		case BYTE:          cmp = cmp_byte; break;
		case SHORT:         cmp = cmp_short; break;
		case INT:           cmp = cmp_int; break;
		case FLOAT:         cmp = cmp_float; break;
		case DOUBLE:        cmp = cmp_double; break;
	}
	qsort(data, V_DSIZE(obj), NBYTES(format), cmp);

	a = data;
	count = 0;
	for (i = 0 ; i < dsize ; i++) {
		b = a + nbytes;
		if (!cmp(a, b) && (i+1) < dsize) {
			count++;
		} else {
			p  = (float)(count+1)/(float)dsize;
			ent += p * log(p) / M_LN2;
			count = 0;
		}
		a = b;
	} 
	ent = -ent;
	free(data);
	return(newVal(BSQ, 1, 1, 1, FLOAT, memdup(&ent, sizeof(FLOAT))));
}

#include "parser.h"

Var *
ff_coreg(vfuncptr func, Var * arg)
{
  typedef unsigned char byte;

  Var      *pic1_in = NULL;                            /* first picture to be coregistered */
  Var      *pic2_in = NULL;                            /* second picture to be coregistered */
  Var      *out = NULL;
  int     ignore = 0;
  float    *solution = NULL;                           /* map of solution space */
  int       verbose = 0;                               /* flag to dump solution space at end */
  int       search = 10;                               /* search radius */
  int       s_dia = 21;                                /* search diameter */
  int       x, y;                                      /* size of images */
  int       a = 0, b = 0;                              /* position of lowval*/
  int       i, j, m, n;                                /* loop indices */
  float     lowval = 2e11;                             /* lowest value found by search */
  float     curval = 0;                                /* current value */
  int       p1, p2, v1, v2;
  int      *pos;                                       /* final position returned */
  int      *wt;
  int random = 1000;
  int ok = 0;
  int count = 0;
  int total = 0;
  int sum = 0;

  Alist alist[8];
  alist[0] = make_alist("pic1",     ID_VAL,	NULL,	&pic1_in);
  alist[1] = make_alist("pic2",     ID_VAL,     NULL,   &pic2_in);
  alist[2] = make_alist("search",   INT,        NULL,   &search);
  alist[3] = make_alist("ignore",   INT,        NULL,   &ignore);
  alist[4] = make_alist("verbose",    INT,        NULL,   &verbose);
  alist[5] = make_alist("random",   INT,        NULL,   &random);
  alist[6].name = NULL;
  
  if (parse_args(func, arg, alist) == 0) return(NULL);
  
  /* if two pics did not get passed to the function */
  if (pic1_in == NULL || pic2_in == NULL) {
  	parse_error("%s: no objects specified.\n", func->name);
    return NULL;
  }
  
  x = GetX(pic1_in);
  y = GetY(pic1_in);
  if (x != GetX(pic2_in) || y != GetY(pic2_in)) {
  	parse_error("%s: images are not same size.\n", func->name);
	return(NULL);
  }
  
  
  if(search < 0) {
    parse_error("Invalid value: %s(...search=%d)\n",func->name);
	return(NULL);
  }
 
	s_dia = search*2 + 1;

	solution = (float *)calloc(sizeof(float),(search*2+1)*(search*2+1));
	wt = (int *)calloc(sizeof(int),(search*2+1)*(search*2+1));

	if (random) {
		/* 
		** Random search
		*/
		while (count < random) {
			/* stop when we've tried too many times */
			if (++total > V_DSIZE(pic1_in)) break;
			i = lrand48() % x;
			j = lrand48() % y;
			p1 = cpos(i, j, 0, pic1_in);
			v1 = extract_int(pic1_in, p1);
			if (v1 == ignore) continue;

			ok = 0;
			for(m=-search; m<(search+1); m++) {
				if((j+m)<0 || (j+m)>=y) continue;
				for(n=-search; n<(search+1); n++) {
					if((i+n)<0 || (i+n)>=x) continue;

					p2 = cpos(i+n,j+m,0,pic2_in);
					v2 = extract_int(pic2_in, p2); 

					if (v2 == ignore) continue;
					ok = 1;

					curval = ((float)v1-(float)v2)*((float)v1-(float)v2);
					solution[(m+search)*(s_dia) + (n+search)] += curval;
					wt[(m+search)*(s_dia) + (n+search)] += 1;
				}
			}
			count += ok;
		}
	} else {
		/*
		** Exhaustive search, skipping pixels that are blank in both
		*/
		for (count = 0 ; count < V_DSIZE(pic1_in) ; count++) {
			v1 = extract_int(pic1_in, count);
			if (v1 == ignore) continue;

			/*
			v2 = extract_int(pic2_in, count);
			if (v2 == ignore) continue;
			*/

			j = count/x;
			i = count%x;

			for(m=-search; m<(search+1); m++) {
				if((j+m)<0 || (j+m)>=y) continue;
				for(n=-search; n<(search+1); n++) {
					if((i+n)<0 || (i+n)>=x) continue;

					p2 = cpos(i+n,j+m,0,pic2_in);
					v2 = extract_int(pic2_in, p2); 

					if (v2 == ignore) continue;

					curval = ((float)v1-(float)v2)*((float)v1-(float)v2);
					solution[(m+search)*(s_dia) + (n+search)] += curval;
					wt[(m+search)*(s_dia) + (n+search)] += 1;
				}
			}
		}
	}

	a = -search;
	b = -search;
	lowval = solution[0]/wt[0];

	for(m=-search; m<(search+1); m++) {
		for(n=-search; n<(search+1); n++) {
			p1 = (m+search)*s_dia + n+search;
			if (wt[p1] > 0) {
				solution[p1] = (float)solution[p1]/(float)wt[p1];
				if(solution[p1] < lowval) {
					lowval = solution[p1];     /* set lowval */
					a = n;                     /* x position of lowval */
					b = m;                     /* y position of lowval */
				}
			}
		}
	}

	if (verbose > 0) {
		pos = (int *) malloc(sizeof(int) * 2);
		pos[0] = a;
		pos[1] = b;

		out = new_struct(2);
		add_struct(out, "space", newVal(BSQ, s_dia, s_dia, 1, FLOAT, solution));
		add_struct(out, "wt", newVal(BSQ, s_dia, s_dia, 1, INT, wt));
		add_struct(out, "position", newVal(BSQ, 2, 1, 1, INT, pos));
		add_struct(out, "count", newInt(count));
		printf("count=%d/%d\n", count, total);
		return(out);
	}

	free(solution);
	free(wt);
	pos = (int *)malloc(sizeof(int)*2);
	pos[0] = a;
	pos[1] = b;
	out = newVal(BSQ, 2, 1, 1, INT, pos);
	return out;
}


#include "window.h"
Var *ff_coreg2(vfuncptr func, Var * arg)
{
    Var *obj1 = NULL;
    Var *obj2 = NULL;
    Var *out = NULL;
    float ignore = 0;
    int *solution = NULL;
    int verbose = 0;
    int x, y;
    int i, j, k, m;
	int size = 10;
	float v1, v2;
    Window *w;
	Var *sval;
	int diameter;
	int *answer;
	int maxval;


    Alist alist[8];
    alist[0] = make_alist("obj1", ID_VAL, NULL, &obj1);
    alist[1] = make_alist("obj2", ID_VAL, NULL, &obj2);
    alist[2] = make_alist("size", INT, NULL, &size);
    alist[3] = make_alist("ignore", FLOAT, NULL, &ignore);
    alist[4] = make_alist("verbose", INT, NULL, &verbose);
    alist[5].name = NULL;

    if (parse_args(func, arg, alist) == 0)
        return (NULL);

    if (obj1 == NULL || obj2 == NULL) {
        parse_error("%s: no objects specified.\n", func->name);
        return NULL;
    }

    x = GetX(obj1);
    y = GetY(obj2);
    if (x != GetX(obj2) || y != GetY(obj2)) {
        parse_error("%s: images are not same size.\n", func->name);
        return (NULL);
    }

    if (size <= 0) {
        parse_error("Invalid value: %s(...size=%d)\n", func->name);
        return (NULL);
    }

	
	diameter = size*2+1;
    solution = (int *) calloc(sizeof(int), diameter*diameter);
	sval = newVal(BSQ, diameter, diameter, 1, INT, solution);

    /*
     ** Exhaustive search, skipping pixels that are blank in both
	 **
	 ** Right now this just counts non-ignore pixels that align.
     */
    for (i = 0 ; i < x ; i++) {
        for (j = 0 ; j < y ; j++) {
            v1 = extract_float(obj1, cpos(i, j, 0, obj1));
            if (v1 == ignore) continue;

			/* 
			** TODO: i*j load_windows will be expensive, but 
			** we're expecting sparse data for now, so it'll be
			** cheaper than i*j rolls 
			*/
			for (k = -size ; k <= size ; k++) {
				for (m = -size ; m <= size ; m++) {
					if (i+k >= 0 && i+k < x && j+m >= 0 && j+m < y) {
						v2 = extract_float(obj2, cpos(i+k, j+m, 0, obj2));
						if (v2 == ignore) continue;
						((int *)(V_DATA(sval)))[cpos(k+size, m+size, 0, sval)] += 1;
					}
				}
			}
        }
    }

	answer = (int *)calloc(2, sizeof(int));
	maxval = extract_int(sval, cpos(size, size, 0, sval));
	for (k = -size ; k <= size ; k++) {
		for (m = -size ; m <= size ; m++) {

			v1 = extract_int(sval, cpos(k+size, m+size, 0, sval));
			if (v1 > maxval) {
				maxval = v1;
				answer[0] = k;
				answer[1] = m;
			}
		}
	}

	if (verbose) {
		Var *s = new_struct(2);
		add_struct(s, "position", newVal(BSQ, 2, 1, 1, INT, answer));
		add_struct(s, "counts", sval);
		return(s);
	} else {
		return(newVal(BSQ, 2, 1, 1, INT, answer));
	}
}

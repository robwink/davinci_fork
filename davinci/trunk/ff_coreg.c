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
  int force=0;

  Alist alist[8];
  alist[0] = make_alist("pic1",     ID_VAL,	NULL,	&pic1_in);
  alist[1] = make_alist("pic2",     ID_VAL,     NULL,   &pic2_in);
  alist[2] = make_alist("search",   INT,        NULL,   &search);
  alist[3] = make_alist("ignore",   INT,        NULL,   &ignore);
  alist[4] = make_alist("verbose",    INT,        NULL,   &verbose);
  alist[5] = make_alist("random",   INT,        NULL,   &random);
  alist[6] = make_alist("force",    INT,        NULL,   &force);
  alist[7].name = NULL;
  
  if (parse_args(func, arg, alist) == 0) return(NULL);
  
  /* if two pics did not get passed to the function */
  if (pic1_in == NULL || pic2_in == NULL) {
  	parse_error("images are no same size.\n");
    return NULL;
  }
  
  x = GetX(pic1_in);
  y = GetY(pic1_in);
  if (x != GetX(pic2_in) || y != GetY(pic2_in)) {
  	parse_error("images are not same size.\n");
	return(NULL);
  }
  
  
  if(search < 0) {
    parse_error("please don't be dumb, use only positive search radii");
    parse_error("radius being reset to 10");
    search = 10;
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

			v2 = extract_int(pic2_in, count);
			if (v2 == ignore) continue;

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

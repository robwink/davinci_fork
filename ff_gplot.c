#include "parser.h"

#ifdef _WIN32
	#define GPLOT_CMD "gnuplot"
#elif  defined(HAVE_LIBX11)
	#define GPLOT_CMD "gplot"
#endif /* _WIN32 */

FILE *gplot_pfp=NULL;
#ifdef GPLOT_CMD
extern FILE *pfp;
#endif /* GPLOT_CMD */
float g_xlow;
float g_xhigh;
float g_ylow;
float g_yhigh;



Var *
ff_gplot(vfuncptr func, Var *arg)
{
#ifdef HAVE_LIBX11
    Var *xlow=NULL, *xhigh=NULL, *ylow=NULL, *yhigh=NULL;
    Var *object, *e;
    int i;
    FILE *fp;
    char *fname;

    struct keywords kw[] = {
        { "object", NULL },
        { "xlow",NULL },
        { "xhigh",NULL },
        { "ylow",NULL },
        { "yhigh",NULL },
        { "axis",NULL},
        { "Xaxis",NULL},
        { NULL, NULL }
    };
    if (evaluate_keywords(func, arg, kw)) {
        return(NULL);
    }

    if ((object = get_kw("object",kw)) == NULL) {
        if (gplot_pfp != NULL) pclose(gplot_pfp);
        gplot_pfp = NULL;
        return(NULL);
    }

    if ((e = eval(object)) != NULL) object = e;

    if (V_TYPE(object) != ID_VAL) {
        fprintf(stderr, "Expected value for keyword: object\n");
        return(NULL);
    }

    if (get_kw("xlow", kw)) {
        if (KwToFloat("xlow", kw, &g_xlow) == -1) return(NULL);
    }
    if (get_kw("xhigh", kw)) {
        if (KwToFloat("xhigh", kw, &g_xhigh) == -1) return(NULL);
    }
    if (get_kw("ylow", kw)) {
        if (KwToFloat("ylow", kw, &g_ylow) == -1) return(NULL);
    }
    if (get_kw("y_high", kw)) {
        if (KwToFloat("yhigh", kw, &g_yhigh) == -1) return(NULL);
    }

    if (gplot_pfp == NULL) gplot_pfp = popen("gnuplot","w");

    fname = tempnam(NULL,NULL);
    fp = fopen(fname, "w");

    for (i = 0 ; i < V_DSIZE(object) ; i++) {
        switch (V_FORMAT(object)) {
        case BYTE:
        case SHORT:
        case INT:
            fprintf(fp, "%d\n", extract_int(object, i)); break;
        case FLOAT:
        case DOUBLE:
            fprintf(fp, "%g\n", extract_double(object, i)); break;
        }
    }


    fclose(fp);

    fprintf(gplot_pfp, "plot ");
    fprintf(gplot_pfp, "[ ");
    if (xlow) fprintf(gplot_pfp, "%g", g_xlow);
    fprintf(gplot_pfp, ":");
    if (xhigh) fprintf(gplot_pfp, "%g", g_xhigh);
    fprintf(gplot_pfp, "] ");

    fprintf(gplot_pfp, "[ ");
    if (ylow) fprintf(gplot_pfp, "%g", g_ylow);
    fprintf(gplot_pfp, ":");
    if (yhigh) fprintf(gplot_pfp, "%g", g_yhigh);
    fprintf(gplot_pfp, "] \"%s\" with linespoints\n", fname);
    fflush(gplot_pfp);

    free(fname);
#endif
    return(NULL);
}

Var *
ff_plot(vfuncptr func, Var *arg)
{
    Var *s,*v;
    FILE *fp;
    char *fname;
    int x,y,z,i;
    int type;
    char buf[5120];		/* added factor of 10 */
    int s0,s1;
    int count = 0;
    
    if (arg && arg->next == NULL && V_TYPE(arg) == ID_STRING) {
        strcpy(buf, V_STRING(arg));
    } else {
        strcpy(buf, "plot ");
        for (s = arg ; s != NULL ; s=s->next) {
            if (V_TYPE(s) == ID_STRING) {
                strcat(buf, V_STRING(s));
            } else {
                if ((v= eval(s)) == NULL) v = s;
                
                type = V_FORMAT(v);
                
                fname = tempnam(NULL,NULL);
                fp = fopen(fname, "w");
                
                s0 = V_SIZE(v)[0];
                s1 = V_SIZE(v)[1] * V_SIZE(v)[0];
                
                x = V_SIZE(v)[orders[V_ORG(v)][0]];
                y = V_SIZE(v)[orders[V_ORG(v)][1]];
                z = V_SIZE(v)[orders[V_ORG(v)][2]];
                
                for (i = 0 ; i < V_DSIZE(v) ; i++) {
                    if (x == 1 && y > 1 && z == 1) {
                        fprintf(fp, "%d ", i+1);
                    }
                    switch (type) {
                    case BYTE:
                    case SHORT:
                    case INT:
                        fprintf(fp, "%d", extract_int(v,i)); break;
                    case FLOAT:
                    case DOUBLE:
                        fprintf(fp, "%.12g", extract_double(v,i)); break;
                    }
                    if (((i+1) % s0) == 0) {
                        fputc('\n', fp);
                    } else {
                        fputc('\t', fp);
                    }
                    if (((i+1) % s1) == 0) {
                        fputc('\n', fp);
                        fputc('\n', fp);
                    }
                }
                fclose(fp);
                if (count++) strcat(buf, ",");
                sprintf(buf + strlen(buf), "'%s'", fname);
                if (V_NAME(v)) 
                    sprintf(buf + strlen(buf), "title '%s'", V_NAME(v));
                free(fname);
            }
        }
    }
    send_to_plot(buf);
    return(NULL);
}

Var *
ff_splot(vfuncptr func, Var *arg)
{
#ifdef HAVE_LIBX11
    Var *s,*v;
    FILE *fp;
    char *fname;
    int i;
    int s0, s1;
    int type;
    char buf[512];
    int count=0;
    
    
    if (arg && arg->next == NULL && V_TYPE(arg) == ID_STRING) {
        strcpy(buf, V_STRING(arg));
    } else {
        strcpy(buf, "splot ");
        for (s = arg ; s != NULL ; s=s->next) {
            if (V_TYPE(s) == ID_STRING) {
                strcat(buf, V_STRING(s));
            } else {
                if ((v= eval(s)) == NULL) v = s;
                type = V_FORMAT(v);
                
                fname = tempnam(NULL,NULL);
                fp = fopen(fname, "w");
                
                s0 = V_SIZE(v)[0];
                s1 = V_SIZE(v)[1] * V_SIZE(v)[0];
                for (i = 0 ; i < V_DSIZE(v) ; i++) {
                    if ((i && (i % s0) == 0) || s0 == 1) fputc('\n', fp);
                    switch (type) {
                    case BYTE:
                    case SHORT:
                    case INT:
                        fprintf(fp, "%d %d %d\n", 
                                (i % s0)+1,
                                (i / s0)+1,
                                extract_int(v,i));
                        break;
                    case FLOAT:
                    case DOUBLE:
                        fprintf(fp, "%d %d %.12g\n", 
                                (i % s0)+1,
                                (i / s0)+1,
                                extract_double(v,i));
                        break;
                    }
                }
                fclose(fp);
                if (count++) strcat(buf, ",");
                sprintf(buf + strlen(buf), "\"%s\"", fname);
                if (V_NAME(v)) 
                    sprintf(buf + strlen(buf), "title '%s'", V_NAME(v));
                free(fname);
            }
        }
    }
    send_to_plot(buf);
#endif
    return(NULL);
}


int
send_to_plot(char *s)
{
#ifdef GPLOT_CMD
    char *gplot_cmd;

    if (pfp == NULL) {
		if (getenv("GPLOT_CMD") != NULL) {
			gplot_cmd = strdup(getenv("GPLOT_CMD"));
		} else {
			gplot_cmd = GPLOT_CMD;
		}
        if ((pfp = popen(gplot_cmd, "w")) == NULL) {
            fprintf(stderr, "Unable to open gplot.\n");
            return(0);
        }
        send_to_plot("set data style linespoints\n");
        send_to_plot("set parametric\n");
    }
    if (write(fileno(pfp), s, strlen(s)) < 0) {
        pfp=NULL;
        send_to_plot(s);
    }
    write(fileno(pfp), "\n", 1);
#endif /* GPLOT_CMD */
    return(1);
}

void 
Find_Axis( char *R,Var *Obj)
{
    int axis=0;

    if (GetSamples(V_SIZE(Obj), V_ORG(Obj))==1) axis |= 1;
    if (GetLines(V_SIZE(Obj), V_ORG(Obj))==1) axis |= 2;
    if (GetBands(V_SIZE(Obj), V_ORG(Obj))==1) axis |= 4;

	
    if (axis==7) *R='\0';

    if (axis == 6) *R='X';	
    else if (axis == 5) *R='Y';
    else if (axis == 3) *R='Z';
    else *R='Y';
}

Var *
ff_xplot(vfuncptr func, Var *arg)
{
/*      Modified Plot; Created:
 **   Mon Jul 31 10:21:29 MST 2000
 **   --Ben
 */

#ifdef HAVE_LIBX11
	Var *s, *v;
	FILE *fp;
	char *fname;

	char buf[5120] = {0};			/* added factor of 10 */
	int count = 0;


	Var *Xaxis = NULL;

	int Mode[3];

	int Ord[3];
	int XOrd[3];

	int CE[3];

	int xFlag = 0;
	char *Axis = NULL;
	int obj_index;
	float *x, *y;
	int i, j, k;
	int idx;
	int Sep = 0;

	char axs;

	int ac;
	Var **av;

	make_args(&ac, &av, func, arg);		/*chop up the args into an array
						   parse and remove them from the list */

	for (i = 0; i < ac; i++) {
		if (av[i] == NULL) continue;

		if (V_TYPE(av[i]) == ID_KEYWORD) {
			if (!(strcmp(av[i]->name, "Xaxis"))) {
				Xaxis = eval(V_KEYVAL(av[i]));
				if (Xaxis == NULL) {
					parse_error("Variable not found: Xaxis=...");
					free(av);
					return (NULL);
				}
			} else if (!(strcmp(av[i]->name, "axis"))) {
				v = V_KEYVAL(av[i]);
				if (V_TYPE(v) == ID_STRING)
					Axis = V_STRING(v);
				else
					Axis = V_NAME(v);

			} else if (!(strcmp(av[i]->name, "separate"))) {
				Sep = 1;
			} else {
				parse_error("Illegal keyword %s\n", av[i]->name);
				free(av);
				return (NULL);
			}
			av[i] = NULL;
		}
	}

	if (Xaxis != NULL)
		xFlag = 1;

	if (Axis == NULL) {
		for (idx = 1; idx < ac; idx++) {
			if (av[idx] == NULL)
				continue;

			s = av[idx];

			if (V_TYPE(s) == ID_UNK) {
				if ((s = eval(av[idx])) == NULL) {
					parse_error("Unknown Variable\n");
					free(av);
					return (NULL);
				}
			}
			if (V_TYPE(s) != ID_VAL)
				continue;

			Find_Axis(&axs, s);
			Axis = strdup(&axs);
			break;
		}
		if (Axis == NULL) {
			parse_error("xplot: No variables.\n");
			return(NULL);
		}
	}
	switch (*Axis) {
	    case 'X':
	    case 'x':
		    Mode[0] = 0;
		    Mode[1] = 1;
		    Mode[2] = 2;
		    break;
	    case 'Y':
	    case 'y':
		    Mode[0] = 1;
		    Mode[1] = 0;
		    Mode[2] = 2;
		    break;
	    case 'Z':
	    case 'z':
		    Mode[0] = 2;
		    Mode[1] = 0;
		    Mode[2] = 1;
		    break;
	    default:
		    Mode[0] = 1;
		    Mode[1] = 0;
		    Mode[2] = 2;
		    break;
	}


	if (Xaxis != NULL) {
		XOrd[0] = GetSamples(V_SIZE(Xaxis), V_ORG(Xaxis));
		XOrd[1] = GetLines(V_SIZE(Xaxis), V_ORG(Xaxis));
		XOrd[2] = GetBands(V_SIZE(Xaxis), V_ORG(Xaxis));

	}
	if (arg && arg->next == NULL && V_TYPE(arg) == ID_STRING) {
		strcpy(buf, V_STRING(arg));
	} else {
		strcpy(buf, "plot ");
		for (idx = 1; idx < ac; idx++) {
			s = av[idx];

			if (s == NULL) continue;

			if (V_TYPE(s) == ID_UNK) {
				if ((s = eval(av[idx])) == NULL) {
					parse_error("Unknown Variable\n");
					free(av);
					return (NULL);
				}
			}
			switch (V_TYPE(s)) {

			    case ID_STRING:
				    strcat(buf, V_STRING(s));
				    count++;
				    break;
			    case ID_VAL:
				    if (!(Sep)) {
					    fname = tempnam(NULL, NULL);
					    fp = fopen(fname, "w");
				    }
				    if ((v = eval(s)) == NULL)
					    v = s;

				    Ord[0] = GetSamples(V_SIZE(v), V_ORG(v));
				    Ord[1] = GetLines(V_SIZE(v), V_ORG(v));
				    Ord[2] = GetBands(V_SIZE(v), V_ORG(v));

				    if (xFlag) {

					    if (XOrd[Mode[0]] != Ord[Mode[0]]) {
						    parse_error("Given X-Axis doesn't agree with given data set");
						    free(av);
						    return (NULL);
					    } else if ((XOrd[1] != 1 && XOrd[2] != 1) && (XOrd[1] != Ord[Mode[1]] && XOrd[2] != Ord[Mode[2]])) {
						    parse_error("Given X-Axis doesn't agree with given data set");
						    free(av);
						    return (NULL);
					    }
				    }
				    x = calloc(Ord[Mode[0]], sizeof(float));
				    y = calloc(Ord[Mode[0]], sizeof(float));

				    for (i = 0; i < Ord[Mode[2]]; i++) {
					    for (j = 0; j < Ord[Mode[1]]; j++) {

						    if (Sep) {
							    fname = tempnam(NULL, NULL);
							    fp = fopen(fname, "w");
						    }
						    for (k = 0; k < Ord[Mode[0]]; k++) {
							    CE[Mode[2]] = i;
							    CE[Mode[1]] = j;
							    CE[Mode[0]] = k;
							    obj_index = cpos(CE[0], CE[1], CE[2], v);
							    switch (V_FORMAT(v)) {
								case BYTE:
								case SHORT:
								case INT:
									y[k] = (float) extract_int(v, obj_index);
								case FLOAT:
								case DOUBLE:
									y[k] = extract_float(v, obj_index);
							    }

							    if (xFlag) {
								    switch (V_FORMAT(v)) {
									case BYTE:
									case SHORT:
									case INT:
										x[k] = (float) extract_int(Xaxis, rpos(obj_index, v, Xaxis));
									case FLOAT:
									case DOUBLE:
										x[k] = extract_float(Xaxis, rpos(obj_index, v, Xaxis));
								    }
							    } else {
								    x[k] = (float) k;
							    }
							    fprintf(fp, "%g\t %g\n", x[k], y[k]);
						    }

						    if (Sep) {
							    fclose(fp);
							    if (count++)
								    strcat(buf, ",");
							    sprintf(buf + strlen(buf), "'%s'", fname);
							    if (V_NAME(v))
								    sprintf(buf + strlen(buf), "title '%s'", V_NAME(v));
							    free(fname);
						    } else
							    fprintf(fp, "\n");
					    }
				    }
				    free(x);
				    free(y);
				    if (!(Sep)) {
					    fclose(fp);
					    if (count++)
						    strcat(buf, ",");
					    sprintf(buf + strlen(buf), "'%s'", fname);
					    if (V_NAME(v))
						    sprintf(buf + strlen(buf), "title '%s'", V_NAME(v));
					    free(fname);
				    }
			}
		}
	}
	send_to_plot(buf);
	free(av);
#endif
	return (NULL);
}

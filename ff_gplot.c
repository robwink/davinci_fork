#include "parser.h"

FILE *gplot_pfp=NULL;
extern FILE *pfp;
float g_xlow;
float g_xhigh;
float g_ylow;
float g_yhigh;



Var *
ff_gplot(vfuncptr func, Var *arg)
{
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
        fprintf(stderr, "Expected value for keyowrd: object\n");
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
    char buf[512];
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
    return(NULL);
}

int
send_to_plot(char *s)
{
    if (pfp == NULL) {
        if ((pfp = popen("gplot", "w")) == NULL) {
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
    return(1);
}

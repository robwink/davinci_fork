#include "parser.h"


Var *
ff_display(vfuncptr func, Var *arg)
{
    Var *object, *e;
    FILE *fp;
    char *fname;
    int i,j,count, x, y;
    int bands;
    char buf[256];
    int max;

    struct keywords kw[] = {
        { "object", NULL },
        { "max", NULL },
        { NULL, NULL }
    };

    if (evaluate_keywords(func, arg, kw)) {
        return(NULL);
    }

    if ((object = get_kw("object", kw)) == NULL) {
        fprintf(stderr, "Expected value for keyowrd: object\n");
        return(NULL);
    }

    if ((e = eval(object)) != NULL) object = e;
    if (V_TYPE(object) != ID_VAL) {
        fprintf(stderr, "Expected value for keyowrd: object\n");
        return(NULL);
    }

    bands = GetBands(V_SIZE(object), V_ORG(object));
    x = GetSamples(V_SIZE(object), V_ORG(object)),
    y = GetLines(V_SIZE(object), V_ORG(object));

    if (V_FORMAT(object) > INT) {
        sprintf(error_buf, "Unable to display FLOAT or DOUBLE data\n");
        parse_error(NULL);
        return(NULL);
    }
    if (bands != 1 && bands != 3) {
        sprintf(error_buf, "Object must have exactly 1 or 3 bands\n");
        parse_error(NULL);
        return(NULL);
    }

    if (bands == 3 && V_ORG(object) != BIP) {
        sprintf(error_buf, "Unable to display RGB.  Must be in BIP format.\n");
        parse_error(NULL);
        return(NULL);
    }
    if (bands == 3 && V_FORMAT(object) != BYTE) {
        sprintf(error_buf, "Unable to display RGB except as a byte.\n");
        parse_error(NULL);
        return(NULL);
    }

    fname = tempnam(NULL,NULL);
    fp = fopen(fname, "w");

    if (bands == 1) {
        fprintf(fp, "P2\n%d %d\n", x, y);
                        
        if (KwToInt("max",kw,&max) > 0) {
            fprintf(fp,"%d\n",max);
        } else {
            fprintf(fp,"%d\n", (2 << (NBYTES(V_FORMAT(object))*8-1)));
        }
        count = 0;
        for (i = 0 ; i < y ; i++) {
            for (j = 0 ; j < x ; j++) {
                fprintf(fp, "%d ", extract_int(object, count++));
            }
            fprintf(fp,"\n");
        }
    } else {
        fprintf(fp, "P6\n%d %d\n255\n", x, y);
        fwrite(V_DATA(object), x*y, 3, fp);
    }
    fclose(fp);
    sprintf(buf, "xv %s &", fname);
    free(fname);
    system(buf);

    return(NULL);
}

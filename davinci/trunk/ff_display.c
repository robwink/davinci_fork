#include "parser.h"
#ifdef _WIN32
#include <process.h>
#endif /* _WIN32 */
#include <errno.h>

#define DV_DEFAULT_VIEWER "xv"

Var *
ff_display(vfuncptr func, Var *arg)
{
    Var *object, *e;
    Var *name=NULL;
    FILE *fp;
    char *fname;
    char *title=NULL;
    int i,j,count;
    int bands;
    char buf[256];
    int max,r,g,b;
    char *viewer = NULL;

    struct keywords kw[] = {
	{ "object", NULL },
	{ "max", NULL },
	{ "title",NULL},
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

	if ((name = get_kw("title", kw)) != NULL) {	
		if (V_STRING(name) !=NULL)
			title=V_STRING(name);
		else if ((e = eval(name)) != NULL)
			title=V_STRING(e);
		else
			title=(char *)strdup("NoName");
	}

	


    if (bands == 3 && V_ORG(object) == BIP) {
	fname = tempnam(NULL,NULL);
	fp = fopen(fname, "w");
	fprintf(fp, "P3\n%d %d\n",
		GetSamples(V_SIZE(object), V_ORG(object)),
		GetLines(V_SIZE(object), V_ORG(object)));
	if (KwToInt("max",kw,&max) > 0) {
	    fprintf(fp,"%d\n",max);
	} else {
	    fprintf(fp,"%d\n", (2 << (NBYTES(V_FORMAT(object))*8-1)));
	}
	count = 0;
	for (i = 0 ; i < GetLines(V_SIZE(object), V_ORG(object)) ; i++) {
	    for (j = 0 ; j < GetSamples(V_SIZE(object), V_ORG(object)) ; j++) {
		r=extract_int(object, count);
		g=extract_int(object, (count+1));
		b=extract_int(object, (count+2));
		fprintf(fp,"%d %d %d ",r,g,b);
		count+=3;
	    }
	    fprintf(fp,"\n");
	}
	fclose(fp);
    }

    else if (bands == 1) {
	fname = tempnam(NULL,NULL);
	fp = fopen(fname, "w");
	fprintf(fp, "P5\n%d %d\n",
		GetSamples(V_SIZE(object), V_ORG(object)),
		GetLines(V_SIZE(object), V_ORG(object)));

	
	switch(V_FORMAT(object)) {
	    case BYTE:
		fprintf(fp, "255\n");
		fwrite(V_DATA(object), 1, V_DSIZE(object), fp);
		break;
	    case SHORT:
		fprintf(fp, "65535\n");
		fwrite(V_DATA(object), 2, V_DSIZE(object), fp);
		break;
	    default:
		fprintf(stderr, "Unable to write format: %d\n", V_FORMAT(object));
		return(NULL);
	}
	fclose(fp);
    }

    viewer=getenv("DV_VIEWER");
    if (viewer == NULL){ viewer=DV_DEFAULT_VIEWER; }
	if (strcmp(viewer,DV_DEFAULT_VIEWER) == 0 && title != NULL){
    	sprintf(buf, "%s -na \"%s\" %s &", viewer,title,fname);
	}
	else {
		sprintf(buf, "%s %s &", viewer, fname);
	}

#ifdef _WIN32
    if (_spawnlp(_P_NOWAIT, viewer, viewer, fname, NULL) == -1){
       parse_error("Error spawning the viewer %s. Reason: %s.",
          viewer, strerror(errno));
    }
#else
    system(buf);
#endif /* _WIN32 */
    free(fname);

    return(NULL);
}

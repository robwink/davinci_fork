#include "parser.h"
#ifdef __CYGWIN__
#include <process.h>
#endif /* _CYGWIN__ */
#include <errno.h>

#define DV_DEFAULT_VIEWER "xv"

Var *
ff_display(vfuncptr func, Var *arg)
{
	Var *obj=NULL, *e, *geom = NULL;
	Var *name=NULL;
	char *fname;
	char *title=NULL;
	char buf[256];
	int max;
	char *viewer = NULL;
	int x,y,z;

	Alist alist[5];
	alist[0] = make_alist( "object",    ID_VAL,    NULL, &obj);
	alist[1] = make_alist( "max",       INT,       NULL, &max);
	alist[2] = make_alist( "title",     ID_STRING, NULL, &title);
	alist[3] = make_alist( "geometry",  ID_VAL,    NULL, &geom);
	alist[4].name = NULL;

	if (parse_args(func, arg, alist) == 0)
		return(NULL);

	if (obj == NULL) {
		parse_error("%s: No object specified\n", func->name);
		return(NULL);
	}

	x = GetX(obj);
	y = GetY(obj);
	z = GetZ(obj);
	if (V_FORMAT(obj) > INT) {
		sprintf(error_buf, "Unable to display FLOAT or DOUBLE data\n");
		parse_error(NULL);
		return(NULL);
	}
	if (z != 1 && z != 3) {
		sprintf(error_buf, "Object must have exactly 1 or 3 bands\n");
		parse_error(NULL);
		return(NULL);
	}
	if (geom) {
		if (GetX(geom) != 2 || GetY(geom) != 1 || GetZ(geom) != 1) {
			sprintf(error_buf,
				"Geometry object must be 2x1x1, specifying the width/height in pixels!\n");
			parse_error(NULL);
			return(NULL);
		}
	}

	fname = (char *)make_temp_file_path();
	if (fname == NULL) {
		parse_error("%s: unable to open temp file", func->name);
		return(NULL);
	}
	if (z == 1) {
		dv_WritePGM(obj, fname, 1);
	} else {
		dv_WritePPM(obj, fname, 1);
	}

	viewer=getenv("DV_VIEWER");
	if (viewer == NULL) {
		viewer=DV_DEFAULT_VIEWER;
	}
	if (strcmp(viewer,DV_DEFAULT_VIEWER) == 0 && title != NULL){
		if (geom != NULL) {
			sprintf(buf, "%s -geometry %dx%d -na \"%s\" %s &",
				viewer,extract_int(geom,0),extract_int(geom,1),title,fname);
		} else {
			sprintf(buf, "%s -na \"%s\" %s &", viewer,title,fname);
		}
	}
	else {
		if (geom != NULL) {
			sprintf(buf, "%s -geometry %dx%d %s &",
				viewer, extract_int(geom,0), extract_int(geom,1), fname);
		} else {
			sprintf(buf, "%s %s &", viewer, fname);
		}
	}

#ifdef __CYGWIN__
	fprintf(stderr, "spawning xv\n");
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

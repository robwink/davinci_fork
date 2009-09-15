#include "parser.h"
#include "dvio.h"

#ifdef __CYGWIN__
#include <process.h>
#endif /* _CYGWIN__ */
#include <errno.h>

// This is probably overriden in config.h
#ifndef  DV_VIEWER
#define DV_VIEWER "xv"
#endif

Var *
ff_display(vfuncptr func, Var *arg)
{
    Var *obj=NULL;
    char *fname;
    char *title=NULL;
    char buf[256];
    int max;
    char *viewer = NULL;
    int x,y,z;

    Alist alist[4];
    alist[0] = make_alist( "object",    ID_VAL, NULL,    &obj);
    alist[1] = make_alist( "max",       INT,    NULL,    &max);
    alist[2] = make_alist( "title",     ID_STRING,    NULL, &title);
    alist[3].name = NULL;

    if (parse_args(func, arg, alist) == 0) return(NULL);

    if (obj == NULL) {
        parse_error("%s: No object specified\n", func->name);
        return(NULL);
    }

    x = GetX(obj);
    y = GetY(obj);
    z = GetZ(obj);
    if (V_FORMAT(obj) > SHORT) {
        sprintf(error_buf, "Unable to display FLOAT or DOUBLE data\n");
        parse_error(NULL);
        return(NULL);
    }
    if (z != 1 && z != 3) {
        sprintf(error_buf, "Object must have exactly 1 or 3 bands\n");
        parse_error(NULL);
        return(NULL);
    }

    fname = make_temp_file_path();
    if (fname == NULL) {
        parse_error("%s: unable to open temp file", func->name);
        return(NULL);
    }

#ifdef HAVE_LIBPNG
    dv_WriteIOM(obj, fname, "png", 1);
#else
    dv_WriteIOM(obj, fname, "jpg", 1);
#endif

    viewer=getenv("DV_VIEWER");
    if (viewer == NULL){
      viewer = (char *)DV_VIEWER;
    }


#if  (defined(__CYGWIN__) || defined(__MINGW32__))
    //copy viewer to buf
    sprintf(buf, "%s", viewer);
    if (_spawnlp(_P_NOWAIT, buf, buf, fname, NULL) == -1){
        parse_error("Error spawning the viewer %s. Reason: %s.",
                    viewer, strerror(errno));
    }
#else
    if (strcmp(viewer,DV_VIEWER) == 0 && title != NULL){
        sprintf(buf, "%s -na \"%s\" %s &", viewer,title,fname);
    }
    else {
        sprintf(buf, "%s %s &", viewer, fname);
    }
    system(buf);
#endif /* _WIN32 */
    free(fname);


    return(NULL);
}

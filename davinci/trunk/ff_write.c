#include <errno.h>
#include "parser.h"
#include "dvio.h"

#if    defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#ifdef _WIN32
#include <io.h>
extern Swap_Big_and_Little(Var *);
#else
#include <unistd.h>
#endif



Var *
ff_write(vfuncptr func, Var *arg)
{

    Var  *ob = NULL;
    char *filename = NULL;
    char *title = NULL;
    char *type=NULL;
    int   force=0;            /* Force file overwrite */
    char  GFX_type[5];
    struct iom_iheader h;
    void *data = NULL;
    FILE *fp = NULL;

    int ac;
    Var **av;

    Alist alist[6];
    alist[0] = make_alist("object",   ID_UNK,    NULL, &ob);
    alist[1] = make_alist("filename", ID_STRING, NULL, &filename);
    alist[2] = make_alist("type",     ID_ENUM,   NULL, &type);
    alist[3] = make_alist("title",    ID_STRING, NULL, &title);
    alist[4] = make_alist("force",    INT,       NULL, &force);
    alist[5].name = NULL;

    if (parse_args(func, arg, alist) == 0) return(NULL);
    
    /**
    ** Make sure user specified an object
    **/
    if (ob == NULL){
        parse_error("write what?  Specify data object");
        return(NULL);
    }

    /**
    ** get filename.  Verify type
    **/
    if (filename == NULL){
        parse_error("No filename specified.");
        return(NULL);
    }

    if (type == NULL) {
        parse_error("No type specified.");
        return(NULL);
    }

    /*
    ** Get title string
    */
    if (title == NULL) {
        title = "DV data product";
    }

    if      (!strcasecmp(type, "raw"))    dv_WriteRaw(ob, filename, force);
    else if (!strcasecmp(type, "vicar"))  dv_WriteVicar(ob, filename, force);
    else if (!strcasecmp(type, "grd"))    dv_WriteGRD(ob, filename, force, title, "daVinci");
    else if (!strcasecmp(type, "pgm"))    dv_WritePGM(ob, filename, force);
    else if (!strcasecmp(type, "ppm"))    dv_WritePPM(ob, filename, force);
    else if (!strcasecmp(type, "ascii")){
        if (!force && access(filename, F_OK)){
            parse_error("File %s already exists.\n", filename);
            return NULL;
        }
        WriteAscii(ob, NULL, filename);
    }
    else if (!strcasecmp(type, "ers"))    dv_WriteERS(ob, filename, force);
    else if (!strcasecmp(type, "imath"))  dv_WriteIMath(ob, filename, force);
    else if (!strcasecmp(type, "isis"))   dv_WriteISIS(ob, filename, force, title);
    else if (!strcasecmp(type, "specpr")){
        if (!force && access(filename, F_OK)){
            parse_error("File %s already exists.\n", filename);
            return NULL;
        }
        WriteSpecpr(ob, filename, title);
    }

/*
** Below here are optional packages
*/
#ifdef HAVE_LIBHDF5
    else if (!strcasecmp(type, "hdf")){
        if (!force && access(filename, F_OK)){
            parse_error("File %s already exists.\n", filename);
            return NULL;
        }
        
        /* Byte-swap data first */
        /* force ? */
        WriteHDF5(-1, filename, ob);
    }
#endif

#ifdef HAVE_LIBMAGICK
	else if (dvio_ValidGfx(type, GFX_type))    dv_WriteGFX_Image(ob, filename, force, GFX_type);
#endif
    else {
        sprintf(error_buf, "Unrecognized type: %s", type);
        parse_error(NULL);
        return(NULL);
    }

    return(NULL);
}

#include <errno.h>
#include "parser.h"
#include "iomedley.h"

#if    defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#ifdef _WIN32
#include <io.h>
extern Swap_Big_and_Little(Var *);
#else
#include <unistd.h>
#endif

/*
** Test for the types recognized by ImageMagic
*/
static int
ValidGfx(char *type,char *GFX_type)
{
 
    int nt=43;  /* Number of types
                 * modify this number if you change the number of types
                 */

    char *Gfx_Types[]={"avs","bmp","cmyk",
                       "gif","gifc","gifg",
                       "hist","jbig","jpeg",
                       "jpg","map","matte",
                       "miff","mpeg","mpgg",
                       "mpgc","mtv","pcd",
                       "pcx","pict","pm",
                       "pbm","pgm","ppm",
                       "pnm","ras","rgb",
                       "rgba","rle","sgi",
                       "sun","tga","tif",
                       "tiff","tile","vid",
                       "viff","xc","xbm",
                       "xpm","xv","xwd","yuv"};
    int i;

    for (i = 0 ; i < strlen(type) ; i++) {
        if (isupper(type[i])) type[i] = tolower(type[i]);
    }

    for (i=0 ; i < nt ; i++){
        if (!(strcmp(type, Gfx_Types[i]))){
            strcpy(GFX_type, Gfx_Types[i]);
            return (1);
        }
    }

    return (0);
}


Var *
ff_write2(vfuncptr func, Var *arg)
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
    alist[0] = make_alist("object",   ID_VAL,    NULL, &ob);
    alist[1] = make_alist("filename", ID_STRING, NULL, &filename);
    alist[2] = make_alist("type",     ID_STRING, NULL, &type);
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

    var2iom_iheader(ob, &h);
    
    data = V_DATA(ob);

    if      (!strcasecmp(type, "raw"))    iom_WriteRaw(filename, data, &h, force);
    else if (!strcasecmp(type, "vicar"))  iom_WriteVicar(filename, data, &h, force);
    else if (!strcasecmp(type, "grd"))    iom_WriteGRD(filename, data, &h, force, title, "daVinci");
    else if (!strcasecmp(type, "pgm"))    iom_WritePNM(filename, data, &h, force);
    else if (!strcasecmp(type, "ppm"))    iom_WritePNM(filename, data, &h, force);
    else if (!strcasecmp(type, "ascii")){
        if (!force && access(filename, F_OK)){
            parse_error("File %s already exists.\n", filename);
            iom_cleanup_iheader(&h);
            return NULL;
        }
        WriteAscii(ob, NULL, filename);
    }
    else if (!strcasecmp(type, "ers"))    iom_WriteERS(filename, data, &h, force);
    else if (!strcasecmp(type, "imath"))  iom_WriteIMath(filename, data, &h, force);
    else if (!strcasecmp(type, "isis"))   iom_WriteISIS(filename, data, &h, force, title);
    else if (!strcasecmp(type, "specpr")){
        if (!force && access(filename, F_OK)){
            parse_error("File %s already exists.\n", filename);
            iom_cleanup_iheader(&h);
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
            iom_cleanup_iheader(&h);
            return NULL;
        }
        
        /* Byte-swap data first */
        /* force ? */
        WriteHDF5(-1, filename, ob);
    }
#endif

#ifdef HAVE_LIBMAGICK
	else if (ValidGfx(type, GFX_type))    iom_WriteGFXImage(filename, data, &h, force, GFX_type);
#endif
    else {
        sprintf(error_buf, "Unrecognized type: %s", type);
        parse_error(NULL);
        iom_cleanup_iheader(&h);
        return(NULL);
    }
    iom_cleanup_iheader(&h);
    return(NULL);
}

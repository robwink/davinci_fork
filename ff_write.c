#include "parser.h"

#ifdef HAVE_LIBMAGICK
int ValidGfx(char *type,char *GFX_type)
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

    //	fprintf(stderr,"%s is not a supported image file type\n",type);

    return (0);
}
#endif


Var *
ff_write(vfuncptr func, Var *arg)
{
    Var *v, *e, *ob;
    char *filename;
    char *title;
    char *type=NULL;
    int force=0;
    char GFX_type[5];

    struct keywords kw[] = {
        { "object", NULL },
        { "filename", NULL },
        { "type", NULL },
        { "title", NULL },
        { "force", NULL},
        { NULL, NULL }
    };

    if (evaluate_keywords(func, arg, kw)) {
        return(NULL);
    }

    /**
    ** Make sure user specified an object
    **/
    if ((v = get_kw("object", kw)) == NULL) {
        parse_error("write what?  Specify data object");
        return(NULL);
    }

    e = eval(v);
    if (e == NULL) {
        sprintf(error_buf, "Variable not found: %s", V_NAME(v));
        parse_error(NULL);
        return(NULL);
    }
    ob = e;

    /**
    ** get filename keyword.  Verify type
    **/

    if ((v = get_kw("filename", kw)) == NULL) {
        parse_error("No filename specified.");
        return(NULL);
    }

    if (V_TYPE(v) != ID_STRING) {
        e = eval(v);
        if (e == NULL || V_TYPE(e) != ID_STRING) {
            parse_error("Illegal argument (... filename=... )");
            return(NULL);
        }
        v = e;
    }

    filename = V_STRING(v);

    /**
    ** Figure out what output type.
    **/

    if ((v = get_kw("type", kw)) == NULL) {
        parse_error("No type specified.");
        return(NULL);
    }

    /**
    ** check for variable dereference
    **/
    e = eval(v);
    if (e != NULL) {
        v = e;
    }

    if (V_TYPE(v) == ID_STRING) {
        type = V_STRING(v);
    }  else {
        type = V_NAME(v);
    }

    if ((v = get_kw("force", kw)) != NULL) force=1;

    if ((access(filename, F_OK) == 0) && strcasecmp(type, "specpr") && !force) {
        fprintf(stderr, "File exists.\n");
        return(NULL);
    }
    /*
    ** Get title string
    */
    if ((v = get_kw("title", kw)) == NULL) {
        title = "DV data product";
    } else {
        if (V_TYPE(v) != ID_STRING) {
            e = eval(v);
            if (e == NULL || V_TYPE(e) != ID_STRING) {
                parse_error("Illegal argument (... title=... )");
                return(NULL);
            }
            v = e;
        }
        title = V_STRING(v);
    }

    if      (!strcasecmp(type, "raw"))    WriteRaw(ob, NULL, filename);
    else if (!strcasecmp(type, "vicar"))  WriteVicar(ob, NULL, filename); 
    else if (!strcasecmp(type, "grd"))    WriteGRD(ob, NULL, filename);
    else if (!strcasecmp(type, "pgm"))    WritePGM(ob, NULL, filename);
    else if (!strcasecmp(type, "ppm"))    WritePPM(ob, NULL, filename);
    else if (!strcasecmp(type, "ascii"))  WriteAscii(ob, NULL, filename);
    else if (!strcasecmp(type, "ers"))    WriteERS(ob, NULL, filename);
    else if (!strcasecmp(type, "imath"))  WriteIMath(ob, NULL, filename);
    else if (!strcasecmp(type, "isis"))   WriteISIS(ob, NULL, filename, title);
    else if (!strcasecmp(type, "specpr")) WriteSpecpr(ob, filename, title);
    else if (!strcasecmp(type, "hdf"))    WriteHDF5(-1, filename, ob);
#ifdef HAVE_LIBMAGICK
    else if (ValidGfx(type,GFX_type))     WriteGFX_Image(ob,filename,GFX_type);
#endif
    else {
        sprintf(error_buf, "Unrecognized type: %s", type);
        parse_error(NULL);
        return(NULL);
    }
    return(NULL);
}

int
WriteRaw(Var *s, FILE *fp, char *filename)
{
    if (fp == NULL) {
        if ((fp = fopen(filename, "w")) == NULL) {
            return(0);
        }
    }

    fwrite(V_DATA(s), NBYTES(V_FORMAT(s)), V_DSIZE(s), fp);
    fclose(fp);

    return(1);
}

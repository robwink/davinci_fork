#include "config.h"

#ifdef HAVE_LIBMAGICK

#include "parser.h"
#include "dvio.h"

#include <magick/api.h>

Var *
dv_LoadGFX_Image(FILE *fp, char *filename, struct iom_iheader *s)
{
    Var *v;
    struct iom_iheader h;
    void *image_data = NULL;
    int   status;
	char  hbuf[HBUFSIZE];

	//    if (!(status = iom_GetGFXHeader(fp, filename, &h))){ return NULL; }

    if (s != NULL) {
        /** 
         ** Set subsets
         **/
        iom_MergeHeaderAndSlice(&h, s);
    }

    image_data = iom_read_qube_data(fileno(fp), &h);
    if (image_data){
        v = iom_iheader2var(&h);
        V_DATA(v) = image_data;
    }
    else {
        parse_error("Read from ImageMagick file %s failed.", filename);
        v = NULL;
    }

    sprintf(hbuf, "%s: via Magick %s image: %dx%dx%d, %d bits",
            filename, iom_Org2Str(h.org),
            iom_GetSamples(h.dim, h.org), 
            iom_GetLines(h.dim, h.org), 
            iom_GetBands(h.dim, h.org), 
            iom_NBYTESI(h.format)*8);
    if (VERBOSE > 1) {
        parse_error(hbuf);
    }
    
    iom_cleanup_iheader(&h);

    return v;
}



int
dv_WriteGFX_Image(Var *ob, char *filename, int force, char *GFX_type)
{
	Image *image;
  	ImageInfo image_info;
	struct iom_iheader h;
    int status;

    int x, y, z;
    int format,org;

    format=V_FORMAT(ob);
    org=V_ORG(ob);
    x=GetSamples(V_SIZE(ob), V_ORG(ob));
    y=GetLines(V_SIZE(ob), V_ORG(ob));
    z=GetBands(V_SIZE(ob), V_ORG(ob));


    if(z>3 && (strcmp(GFX_type,"mpgc") && strcmp(GFX_type,"mpgg") && 
	       strcmp(GFX_type,"gifc") && strcmp(GFX_type,"gifg"))){
      parse_error("A movie type must be specified if you have more than 3 bands");
      return 0;
    }

    if (z==2) {
      parse_error("Incorrect number of bands to make an image...aborting");
      return 0;
    }
    
    else if (org==BIL || org==BIP){
      parse_error("BIL and BIP formats are not allowable...aborting");
      return 0;
    }
    
    else if (format!=BYTE){
      parse_error("Only BYTE type data is allowed...aborting");
      return 0;
    }
    
    var2iom_iheader(ob, &h);
    //    status = iom_WriteGFXImage(filename, V_DATA(ob), &h, force, GFX_type);
    iom_cleanup_iheader(&h);
    
    if (status == 0){
      parse_error("Failed writing file %s.\n", filename);
      return 0;
    }

    return 1;
}

static Image *
Var2Miff(Var *v)
{
    int x, y, z;
    int format, org;
    Image *image = NULL;

    x = GetSamples(V_SIZE(v), V_ORG(v));
    y = GetLines(V_SIZE(v), V_ORG(v));
    z = GetBands(V_SIZE(v), V_ORG(v));

    format = V_FORMAT(v);
    org = V_ORG(v);

	if (org==BIL || org==BIP){
		parse_error("BIL and BIP formats are not allowable...aborting");
		return (NULL);
	}

	else if (format!=BYTE){
		parse_error("Only BYTE type data is allowed...aborting");
		return (NULL);
	}
    
	//    image = iom_ToMiff(V_DATA(v), x, y, z);

    return image;
}


Var *
dv_Miff2Var(Image *image) /*  Read */
{
    int x, y, z;
    char *data = NULL;

    //    if (!iom_ExtractMiffData(image, &x, &y, &z, (void *)&data)){ return NULL; }
    //return(newVal(BSQ,x,y,z,BYTE,data));
    return NULL;
}    

/*
** Test for the types recognized by ImageMagic
*/
int
dvio_ValidGfx(char *type,char *GFX_type)
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


#if 0
#ifdef HAVE_LIBX11

#include <magick/xwindows.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xatom.h>

extern Widget top;

Var *
ff_XImage_Display2(vfuncptr func, Var * arg)
{
	Var *Obj;
    int ac,i;
    Var **av, *v;
    Alist alist[2];
	Image *image,*tmp_image;
	ImageInfo image_info;
	QuantizeInfo quantize_info;
	char disp[1024];
	Display *display;
	unsigned long state;
  	XResourceInfo resource;
  	XrmDatabase resource_database;

    alist[0] = make_alist("object", ID_VAL, NULL, &Obj);
    alist[1].name = NULL;
	disp[0]='\0';

	if (parse_args(func, arg, alist) == 0) return(NULL);


	if((image=Var2Miff(Obj))==NULL){
		return (NULL);
	}


  	GetImageInfo(&image_info);
	GetQuantizeInfo(&quantize_info);

	display=XOpenDisplay((char *) NULL);

  	if (display == (Display *) NULL){
    		parse_error("Unable to display image...can't open X server");
		return (NULL);
	}

  	XSetErrorHandler(XError);
  	resource_database=XGetResourceDatabase(display,"davinci");
  	XGetResourceInfo(resource_database,"davinci",&resource);
  	resource.image_info=(&image_info);
  	resource.quantize_info=(&quantize_info);
  	state=DefaultState;

  	tmp_image=XDisplayImage(display,&resource,(char **) NULL,0,&image,&state);

  	XCloseDisplay(display);

	if (tmp_image!=NULL)
		return(dv_Miff2Var(tmp_image));
	else
		return(NULL);

}
#endif /* HAVE_LIBX11 */
#endif /* 0 */

#endif /* HAVE_LIBMAGICK */

#if !defined(_WIN32) && defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#if defined(HAVE_LIBMAGICK) && defined(HAVE_LIBX11)
#include <magick/magick.h>
#include <magick/api.h>
#include <magick/xwindows.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xatom.h>
#include "parser.h"
#include "dvio.h"

extern Widget top;

Var *
dv_LoadGFX_Image(FILE *fp, char *filename, struct iom_iheader *s)
{
    Var *v;
    struct iom_iheader h;
    void *image_data = NULL;
    int   status;

    if (!(status = iom_GetGFXHeader(fp, filename, &h))){ return NULL; }

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
    status = iom_WriteGFXImage(filename, V_DATA(ob), &h, force, GFX_type);
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
    
    image = iom_ToMiff(V_DATA(v), x, y, z);

    return image;
}


Var *
dv_Miff2Var(Image *image) /*  Read */
{
    int x, y, z;
    char *data = NULL;

    if (!iom_ExtractMiffData(image, &x, &y, &z, (void *)&data)){ return NULL; }
	return(newVal(BSQ,x,y,z,BYTE,data));
}    


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

#endif

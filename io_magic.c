#ifndef _WIN32
#include "config.h"
#endif
#ifdef HAVE_LIBMAGICK
#ifdef HAVE_LIBX11
#include <magick.h>
#include <magick/api.h>
#include <magick/xwindows.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xatom.h>

#include "parser.h"

extern Widget top;

Image *Var2Miff(Var *ob)//Write
{
	int i,j,k;
	Image *image,*tmp_image,*start_image;
	int layer2,layer3;

	int	format,org;
	int	x,y,z;

	int Frames=1;
	int Frame_Size=0;
	int First_Time=1;
	int Gray=0;
	int scene=0;
	QuantizeInfo quantize_info;
	ImageInfo	tmp_info,image_info;
	Display *display;
	unsigned long state;
  	XResourceInfo resource;
  	XrmDatabase resource_database;

	char *data=(char *)V_DATA(ob);

	unsigned char r,g,b;

	format=V_FORMAT(ob);
	org=V_ORG(ob);
        y=GetLines(V_SIZE(ob), V_ORG(ob));
        x=GetSamples(V_SIZE(ob), V_ORG(ob));
        z=GetBands(V_SIZE(ob), V_ORG(ob));


	if (org==BIL || org==BIP){
		parse_error("BIL and BIP formats are not allowable...aborting");
		return (NULL);
	}

	else if (format!=BYTE){
		parse_error("Only BYTE type data is allowed...aborting");
		return (NULL);
	}



	if ((z % 3)){
		Frames=z;
		Gray=1;
	}
	else
		Frames = z/3;

	GetImageInfo(&image_info);

	while (Frames > 0) {
	  	image=AllocateImage(&image_info);

		if (image == (Image *) NULL){
			parse_error("Can't allocate memory for image write");
			return(NULL);
		}

		if (First_Time)
			start_image=image;


		image->columns=x;
  		image->rows=y;
 		image->packets=image->columns*image->rows;
		image->pixels=(RunlengthPacket *) malloc(image->packets*sizeof(RunlengthPacket));
  		if (image->pixels == (RunlengthPacket *) NULL) {
			parse_error("Can't allocate memory for image write");
      			DestroyImage(image);
			return (NULL);
		}
	
		layer2=x*y;
		layer3=2*x*y;
  		for (i=0; i < y ; i++) {
			for (j = 0; j < x; j++){
				if (Gray){
					r=g=b=data[i*x+j+Frame_Size];
				}
				else {
					r=data[i*x+j+Frame_Size];
					g=data[i*x+j+layer2+Frame_Size];
					b=data[i*x+j+layer3+Frame_Size];
				}
		    		image->pixels[i*x+j].red=r;
    				image->pixels[i*x+j].green=g;
   			 	image->pixels[i*x+j].blue=b;
  			  	image->pixels[i*x+j].index=0;
 			   	image->pixels[i*x+j].length=0;
			}
  		}
		Frame_Size+=((Gray) ? (x*y) : (3*x*y));
		Frames--;
		image->scene=scene++;
//		CondenseImage(image);
		if (First_Time){
			tmp_image=image;
			First_Time=0;
		}

		else {
			tmp_image->next=image;
			image->previous=tmp_image;
			tmp_image=image;
		}

	}
	image=start_image;
      	return(image);
}

Var *Miff2Var(Image *image)//Read
{

	Image *tmp_image;
	ImageInfo image_info;
	ImageType it;
	int Gray=0;


	int layer2,layer3;

	int Frames=1;
	int Frame_Size=0;

	int x,y,i;

	char *data;

	GetImageInfo(&image_info);
      	if (image == (Image *) NULL)
		return (NULL);

	it=GetImageType(image);
	x=image->columns;
	y=image->rows;
	if (it==GrayscaleType){
		Gray=1;
	}

	tmp_image=image;
	while (tmp_image->next!=NULL){
		Frames++;
		tmp_image=tmp_image->next;
	}

	if (Gray){
		data=(char *)calloc(Frames*x*y,sizeof(char));
		tmp_image=image;
		while (tmp_image!=NULL) {
			UncondenseImage(tmp_image);
			for (i=0;i<(x*y);i++){
				data[i+Frame_Size]=tmp_image->pixels[i].red;
			}
			Frame_Size+=(x*y);
			tmp_image=tmp_image->next;
		}
	}

	else{
		int total=x*y;
		int count=0;
		layer2=total;
		layer3=2*total;
		data=(char *)calloc(Frames*total*3,sizeof(char));
		tmp_image=image;
		while(tmp_image!=NULL){
			UncondenseImage(tmp_image);
			for (i=0;i<total;i++){
				data[i+Frame_Size]=tmp_image->pixels[i].red;
				data[i+layer2+Frame_Size]=tmp_image->pixels[i].green;
				data[i+layer3+Frame_Size]=tmp_image->pixels[i].blue;
			}
			Frame_Size+=(x*y*3);
			tmp_image=tmp_image->next;
		}
	}

	DestroyImage(image); 

	return(newVal(BSQ,x,y,(Gray ? (Frames*1) : (Frames*3)),BYTE,data));

}


Var *LoadGFX_Image(char *filename)
{
	Image *image;
	ImageInfo image_info;

	GetImageInfo(&image_info);
	strcpy(image_info.filename,filename);
	image=ReadImage(&image_info);
      	if (image == (Image *) NULL)
		return (NULL);

	else 
		return (Miff2Var(image));

}

void WriteGFX_Image(Var *ob,char *filename,char *GFX_type)
{
	char *newfn=(char *)calloc(strlen(filename)+strlen(GFX_type)+1,sizeof(char));
	Image *image;
  	ImageInfo image_info;

	int z,format,org;

	format=V_FORMAT(ob);
	org=V_ORG(ob);
        z=GetBands(V_SIZE(ob), V_ORG(ob));


	if(z>3 && (strcmp(GFX_type,"mpgc") && strcmp(GFX_type,"mpgg") && 
		strcmp(GFX_type,"gifc") && strcmp(GFX_type,"gifg"))){
		parse_error("A movie type must be specified if you have more than 3 bands");
		return;
	}

	if (z==2) {
		parse_error("Incorrect number of bands to make an image...aborting");
		return;
	}

	else if (org==BIL || org==BIP){
		parse_error("BIL and BIP formats are not allowable...aborting");
		return;
	}

	else if (format!=BYTE){
		parse_error("Only BYTE type data is allowed...aborting");
		return;
	}


	if (!strcmp(GFX_type,"gifc"))
		strcpy(GFX_type,"gif");
	if (!strcmp(GFX_type,"gifg"))
		strcpy(GFX_type,"gif");
	if (!strcmp(GFX_type,"mpgc"))
		strcpy(GFX_type,"mpg");
	if (!strcmp(GFX_type,"mpgg"))
		strcpy(GFX_type,"mpg");
	

	strcpy(newfn,GFX_type);		//<GFX_type:filename> will designate desired file type
	strcat(newfn,":");
	strcat(newfn,filename);

	image=Var2Miff(ob);
	GetImageInfo(&image_info);
	strcpy(image->filename,newfn);

	WriteImage(&image_info,image);
      	DestroyImage(image);
	free(newfn);
}

Var *ff_XImage_Display(vfuncptr func, Var * arg)
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
		return(Miff2Var(tmp_image));
	else
		return(NULL);

}

#endif
#endif

#include "parser.h"
#include <magick/api.h>
#include <magick/image.h>
/*

  DaVinci loadable module for using ImageMagick features.

  Copyright (c) 2002, Mars Space Flight Facility,
  Department of Geological Sciences, Arizona State University

  Written January 8, 2002 by Randy Kaelber

*/

static Var *dvWriteImage(vfuncptr f, Var *args);
static Var *dvCropImage(vfuncptr f, Var *args);
static Var *dvHackVars(vfuncptr f, Var *args);

/* 
   davinci usage:
   
    dvmagick.WriteImage(object, filename, toformat)
    Writes a davinci object as an image file.

*/


/* Begin DaVinci set up structures and constructors, destructors */
static dvModuleFuncDesc export_funcs [] = {
  {"WriteImage", (void *)dvWriteImage},
  {"CropImage", (void *)dvCropImage},
  {"HackVars", (void *)dvHackVars},
  {NULL, NULL}
};

static dvDepAttr deps[] = {
  {NULL, NULL}
};

static dvModuleInitStuff dvinit = {
  export_funcs, 3, NULL, 0
};

DV_MOD_EXPORT int dv_module_init(const char *modname,
				  dvModuleInitStuff *ini)
{
  int imversion;
  char shortversion[10];
  *ini = dvinit;

  /* Gahhh!  ImageMagick apparently has changed from MagickIncarnate() to
     InitializeMagick(), but only in the header files!  The lib still only
     contains the definition for MagickIncarnate().  This will need to be
     addressed soon with newer versions of IM.  Right now the compiler
     whinges about the implicit definition, but it's okay, it *will* work.
     Ignore it for now.
  */

  InitializeMagick("davinci");
  (void) GetMagickVersion(&imversion);
  snprintf(shortversion, 10, "%i.%i.%i", imversion >> 8, 
	   (imversion & 0xF0)  >> 4, (imversion & 0xF));
  parse_error("Module %s is using ImageMagick version %s", modname, shortversion);
  return 1;
}

DV_MOD_EXPORT void dv_module_fini(const char *modname)
{
  DestroyMagick();
  parse_error("%s Unloaded.", modname);
}

/* End of preliminaries.  Let's get to work. */

/* strictly internal routines used by the module */

static inline StorageType dv_to_im_storage_type(Var * img)
{
  /* Determine the ImageMagick Storage Type to use, based on the type
     of a DaVinci storage type. */
     
  switch (V_FORMAT(img)) {
  case BYTE:
    return CharPixel;
  case SHORT:
    return ShortPixel;
  case INT:
    return IntegerPixel;
  case FLOAT:
    return FloatPixel;
  case VAX_FLOAT:
    return FloatPixel; /* When you make an assumption... */
  case VAX_INTEGER:
    return IntegerPixel;
  case DOUBLE:
    return DoublePixel;
  default:
    return CharPixel; /* Unknown DaVinci Type!  You'll need to add it here. */
  }
}


static int write_band_inter_pixel(Var * obj, char * fn, char * type, int force)
{
  /* Routine to write image(s) from band interlace by pixel data set */
  ExceptionInfo imexc;
  Image *img, *tmp;
  ImageInfo *img_info;
  int x,y,z,j;
  register int i;
  register PixelPacket *pixels;
  unsigned int rtncode = 0;
  void * dptr;
  GetExceptionInfo(&imexc);
  img_info = CloneImageInfo((ImageInfo *) NULL);
  x = GetSamples(V_SIZE(obj), V_ORG(obj));
  y = GetLines(V_SIZE(obj), V_ORG(obj));
  z = GetBands(V_SIZE(obj), V_ORG(obj));
  switch (z) {
  case 1:
    img = ConstituteImage(x,y,"I",dv_to_im_storage_type(obj), V_DATA(obj), &imexc);
    break;
  case 3:
    img = ConstituteImage(x,y,"RGB",dv_to_im_storage_type(obj), V_DATA(obj), &imexc);
    break;
  default:
    parse_error("Unsupported Z dimension.");
    return 0;
  }
  if (img == (Image *) NULL) {
    parse_error("Couldn't constitute image: %s", imexc.reason);
    return 0;
  }
  strcpy(img->filename, fn);
  rtncode = WriteImage(img_info, img);
  if (rtncode == 0) parse_error("Error in WriteImage: %i", img->exception);
  DestroyImageInfo(img_info);
  DestroyImage(img);
  return rtncode;
}




static Var * dvWriteImage(vfuncptr f, Var *args)
{
  /* Converts the image specified in arg1 to the type specified in arg2 */
  /* Validate our arguments */
  Var *image = NULL, *forceobj = NULL, *rtn = NULL;

  char *itype = NULL, *fname = NULL;
  int z = 0, org = BSQ, force = 0, rtncode = 0;
  Alist alist[5];
  
  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("filename", ID_STRING, NULL, &fname);
  alist[2] = make_alist("type", ID_ENUM, NULL, &itype);
  alist[3] = make_alist("force", ID_VAL, NULL, &forceobj);
  alist[4].name = NULL;

  if (parse_args(f, args, alist) == 0) return(NULL);
  if (forceobj == NULL) force = 0; else force = V_INT(forceobj);
  if (force != 0 && force != 1) {
    parse_error("force value must be 0 or 1.");
    return(NULL);
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is currently supported. Try dvmagick.WriteImage(byte(a)...");
    return(NULL);
  }

  z = GetBands(V_SIZE(image), V_ORG(image));

  org = V_ORG(image);
  
  /* z rules:
     z = 1:  monochrome image format
     z = 2:  monochrome move format
     z = 3:  RGB image format
     z > 3:  monochrome movie format
  */
  if (((z == 2) || (z > 3)) && (strcmp(itype, "mpgc") && strcmp(itype, "mpgg") &&
			      strcmp(itype, "gifc") && strcmp(itype, "gifg"))) {
    parse_error("A movie type is required for an image with %i bands.", z);
    return (NULL);
  }

  switch (org) {
  case BIP:    
    rtncode = write_band_inter_pixel(image, fname, itype, force);
    break; 
    /* case BSQ:
       rtncode = write_band_sequential(image, fname, itype, force);
       break; */
       /* case BIL:
	  rtncode = write_band_inter_line(image, fname, itype, force); 
	  break; */ /* unsupported for now, while I figure this out */
  default:
    parse_error("Unsupported organization format.");
    rtncode = 0;
    break;
  }

  rtn = newVar();  /* try to return a value */
  V_TYPE(rtn) = ID_VAL;
  V_DSIZE(rtn) = V_SIZE(rtn)[0] = V_SIZE(rtn)[1] = V_SIZE(rtn)[2] = 1;
  V_ORG(rtn) = BSQ;
  V_FORMAT(rtn) = INT;
  V_DATA(rtn) = calloc(1, sizeof(int));
  V_INT(rtn) = rtncode;

  return(rtn);
}



static Var *dvCropImage(vfuncptr f, Var *args)
{
  return(NULL);
}


static Var * dvHackVars(vfuncptr f, Var *args)
{
  /* This routine is purely for Randy's edification and enjoyment.  DaVinci's
     call mechanism seems to be much like Python's, just without the cool 
     toys that simplify argument parsing. */

  /* Or not.  5 minutes with Noel saved me much time, it appears. */
  
  /* I assume the args get passed as an array of Vars... I'll ignore
     that for the moment, to figure out what goes on in this structure. */
  parse_error("arg type is: %i and its name is: %s", args->type, args->name);
  /* Nope! I'm wrong.  It's a linked list, which is much easier, actually. */
  parse_error("what about the next ptr? %i", args->next);
  /* And I suppose we return an array (or perhaps a linked list) 
     of Vars.  Boy, this really needs some documentation. */
  return NULL;
}

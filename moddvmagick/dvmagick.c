/*

  DaVinci loadable module for using ImageMagick features.

  Copyright 2002, Mars Space Flight Facility,
  Department of Geological Sciences, Arizona State University

  Written January 8-15, 2002 by Randy Kaelber

  Current Bug List:

  1. ReadImage not supported yet.

  2. WriteImage can only write BSQ and BIP objects.

  3. BIP color animations are "weird".

  4. Image manipulation API not yet implemented.

  Gotchas:
  
  Because the ImageMagick library doesn't include methods to carry out
  certain important operations on its structures (most notably anything
  to do with making animations and movies), this code is highly sensitive
  to changes in the ImageMagick libraries that affect structure layout.

  Nichevo.

  This module should work with ImageMagick 5.4.2 and above. If you upgrade
  ImageMagick and have problems with this,  you MUST recompile this module.
  If after recompiling, please contact the author at Randy.Kaelber@asu.edu

*/

/* DaVinci Includes */
#include "parser.h"

/* ImageMagick Includes */
#include <magick/api.h>
#include <magick/image.h>

/* System Includes */
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

/* Local Function prototypes */

static Var *dvWriteImage(vfuncptr f, Var *args);
static Var *dvReadImage(vfuncptr f, Var *args);

/* 
   davinci usage:
   
    dvmagick.WriteImage(object, filename, toformat, force)
    Writes a davinci object as an image file.

    a = dvmagick.ReadImage(filename, org)
    Returns a Davinci object 

*/


/* Begin DaVinci set up structures and constructors, destructors */
static dvModuleFuncDesc export_funcs [] = {
  {"WriteImage", (void *)dvWriteImage},
  {"ReadImage", (void *)dvReadImage},
  {NULL, NULL}
};

/* No dependencies that DaVinci needs to load.  However, ImageMagick relies
   on a scad of libraries to do its thing.  This list can vary depending
   on your architecture, operating system, and environment.  To find out what
   you need, run:

   Magick-config --ldflags --libs

   to find out.
*/

static dvDepAttr deps[] = {
  {NULL, NULL}
};

/* The structure that defines what is accessible from DaVinci */

static dvModuleInitStuff dvinit = {
  export_funcs, 2, NULL, 0
};


/* dv_module_init is called by DaVinci when you load the module.
   The second argument to this subroutine passes back the pointer
   to initialization stucture defined above.  This module is analogous
   to _init() when you do C-style shared libraries, so it's the perfect
   place to initialize anything the module will require for the duration.
 */
DV_MOD_EXPORT int dv_module_init(const char *modname,
				  dvModuleInitStuff *ini)
{
  int imversion;
  char shortversion[10];
  *ini = dvinit;
  if (deps == NULL); /* no op to keep gcc from carping about deps */
  InitializeMagick("davinci");
  (void) GetMagickVersion(&imversion);
  snprintf(shortversion, 10, "%i.%i.%i", imversion >> 8, 
	   (imversion & 0xF0)  >> 4, (imversion & 0xF));
  parse_error("Module %s is using ImageMagick version %s", modname, shortversion);
  if (imversion < 0x542)
    parse_error("For best results, consider upgrading to ImageMagick 5.4.2 or greater.");
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


    
    
static Image * constitute_dvobject(Var * obj, char * planes, unsigned int z) {
  ExceptionInfo exc;
  Image * rtn;
  unsigned long x,y;

  GetExceptionInfo(&exc);
  x = GetX(obj);
  y = GetY(obj);


  rtn = ConstituteImage(x,y,planes,CharPixel,
			V_DATA(obj)+cpos(0,0,z,obj), &exc);
  if (rtn == (Image *) NULL) {
    parse_error("Image Constitute failed: %s", exc.reason);
  }
  return rtn;
}

static unsigned int write_im_image(Image * img, char * fn, char * type) {
  /* Write an ImageMagick image out to disk.

  Arguments:
  img: the Imagemagick Image

  fn: the filename to store the file

  type: Identifier to specifiy the output file format.  If the empty string,
  the file format will be divined from the filename suffix.

  Returns: 1 on success, 0 on failure.

  */
  char * fullfile;
  unsigned int flen, rtn;
  ImageInfo *img_info;
  img_info = CloneImageInfo((ImageInfo *) NULL);
  flen = strlen(fn) + strlen(type) + 2;
  if ((fullfile = (char *)malloc(flen)) == NULL) {
    parse_error("malloc() failed! (Memory condition).");
    return 0;
  }
  if (!(strncmp(type, "mpg", 3) && strncmp(type, "gif", 3))) type[3] = 0;
  snprintf(fullfile, flen, "%s:%s", type, fn);
  strcpy(img->filename, fullfile);
  if ((rtn = WriteImage(img_info, img)) == 0)
    parse_error("Error in WriteImage: %s", img->exception.reason);
  free(fullfile);
  DestroyImageInfo(img_info);
  return rtn;
}
  
static unsigned int composite_color_layers(Image * red, Image * green, 
						  Image * blue) {
  /* Function to add three identically sized images together, good for
     RGB compositing.  You don't really have to pass the images in RGB 
     order, but it will help you keep things clear.  Note that your
     red composite layer is destroyed by this function, as it is what
     returns the composited image.  If you will need it after calling,
     I suggest Cloning it first.

     Arguments:
     red: The red layer image.  This is also the finished composite image
     when we're done.

     green: The green layer image.

     blue: The blue layer image.

     Returns: 1 on success, 0 on failure
  */
  unsigned int rtn;
  rtn = 1 - CompositeImage(red, AddCompositeOp, green, 0, 0);
  rtn += 2 * (1 - CompositeImage(red, AddCompositeOp, blue, 0, 0));
  switch (rtn) {
  case 3:
    parse_error("All Image compositing failed.");
    break;
  case 2:
    parse_error("Red/Blue compositing failed.");
    break;
  case 1:
    parse_error("Red/Green compositing failed.");
    break;
  case 0:
    return 1;
  }
  return(1 - (rtn/rtn));
}



static Image * assemble_image(Var * obj, unsigned int ctype, 
			      unsigned int zplane) {
  /* Routine to build a single ImageMagick greyscale or color image from a 
     DaVinci cube, given the cube object, the type of image desired, and
     which z plane we should begin at. 

     Arguments: 
     obj - DaVinci object we will build the image from

     type - 1 for greyscale, 3 for RGB color

     zplane - which z plane(s) we wish to capture in this frame. If it is
     greyscale, this refers to the band we will use to build the picture.
     If RGB, zplane is the red plane, zplane+1 is the green plane, and
     zplane+2 is the blue plane.

*/
  Image *img = NULL;
  ImageInfo * img_info;
  ExceptionInfo exc;
  Var *dvzplane[3] = {NULL, NULL, NULL}, *o;
  Range dvrange;
  PixelPacket * line_pixels;
  unsigned int y,z;
  register unsigned int x;

  img_info = CloneImageInfo((ImageInfo *) NULL);
  GetExceptionInfo(&exc);
  /* quickie checks to make sure we won't screw the pooch first */
  if ((GetZ(obj) - ctype + 1) < zplane) {
    parse_error("Invalid band or insufficient bands to build image.");
    return(NULL);
  }


  /* At this point, we have a blank image in the appropriate dimensions. 
     We now need to slush through the DaVinci object to get the right data 
     into the ImageMagick image. */
  
  /* Make DaVinci objects of the z plane(s) in question */

  if (ctype == 3) {

    /* build the image we will use */
    img = AllocateImage(img_info);
    if (img == NULL) {
      parse_error("AllocateImage() Failed: %s", exc.reason);
      goto jump_error_cond;
    }
    img->columns = GetX(obj);
    img->rows = GetY(obj);
    for (z=0; z<3; z++) {
      dvrange.lo[0] = 1; dvrange.hi[0] = GetX(obj);
      dvrange.lo[1] = 1; dvrange.hi[1] = GetY(obj);
      dvrange.lo[2] = z+zplane; dvrange.hi[2] = z+zplane ;
      dvrange.step[0] = 1; dvrange.step[1] = 1; dvrange.step[2] = 1;
      
      if ((dvzplane[z] = extract_array(obj, &dvrange)) == NULL) {
	parse_error("extract_array() failed."); 
	goto jump_error_cond;
      }
    }
    img->columns = GetX(obj);
    img->rows=GetY(obj);
    for (y=0; y<img->rows; y++) {
      line_pixels = SetImagePixels(img, 0,y, img->columns, 1);
      if (line_pixels == NULL) {
	parse_error("SetImagePixels() failed.");
	goto jump_error_cond;
      }
      for (x=0; x<img->columns; x++) {
	o = dvzplane[0];
	line_pixels->red = (extract_int(o, cpos(x,y,0,o))/255.0)*MaxRGB;
	o = dvzplane[1];
	line_pixels->green = (extract_int(o, cpos(x,y,0,o))/255.0)*MaxRGB;
	o = dvzplane[2];
	line_pixels->blue = (extract_int(o, cpos(x,y,0,o))/255.0)*MaxRGB;
	line_pixels++;
      }
      if (!SyncImagePixels(img)) {
	parse_error("SyncImagePixels() failed.");
	goto jump_error_cond;
      }
    }
    /*    for(z = 0; z<3; z++) {
	  dvrange.lo[0] = 1; dvrange.hi[0] = GetX(obj);
	  dvrange.lo[1] = 1; dvrange.hi[1] = GetY(obj);
	  dvrange.lo[2] = z+zplane; dvrange.hi[2] = z+zplane ;
	  dvrange.step[0] = 1; dvrange.step[1] = 1; dvrange.step[2] = 1;
	  
	  if ((dvzplane = extract_array(obj, &dvrange)) == NULL) {
	  parse_error("extract_array() failed."); 
	  goto jump_error_cond;
	  }
	  if ((subimg[z] = constitute_dvobject(dvzplane, itype[z], 0)) == NULL) {
	  goto jump_error_cond;
	  }
	  }
	  if (composite_color_layers(subimg[0], subimg[1], subimg[2]) == 0) {
	  goto jump_error_cond;
	  }
	  img = subimg[0]; */
  }
  else {
    /* easy does it */
      dvrange.lo[0] = 1;
      dvrange.hi[0] = GetX(obj);
      dvrange.lo[1] = 1;
      dvrange.hi[1] = GetY(obj);
      dvrange.lo[2] = zplane;
      dvrange.hi[2] = zplane;
      dvrange.step[0] = 1;
      dvrange.step[1] = 1;
      dvrange.step[2] = 1;
    if ((dvzplane[0] = extract_array(obj, &dvrange)) == NULL) {
      parse_error("extract_array() failed.");
      goto jump_error_cond;
    }
    if ((img = constitute_dvobject(dvzplane[0], "I", 0)) == NULL) {
      goto jump_error_cond;
      }
    }
  goto normal_exit;
 jump_error_cond:  
  if (img != NULL) DestroyImage(img);
 normal_exit: /* clean up the trash before exiting */
  // if (dvzplane != NULL) free(dvzplane);
  return(img);
}  

static Var * im_to_dv(Image * img) {
  /* im_to_dv: Takes and ImageMagick image or animation and returns
     a DaVinci object that represents the image file.

     Arguments:
     img: The ImageMagick Image Object we want to convert

     Returns:
     A pointer to a DaVinci cube on success.  NULL pointer on failure.

  */
  parse_error("Unimplemented.");
  return (NULL);

}

static Image * dv_to_im(Var * obj, int type)
{
  /* dv_to_im:  Takes a davinci cube in any format and returns a
     representative ImageMagick object.  
     
     Arguments:
     obj: The DaVinci object we are translating
     
     type: Identify the movie type we want for movies.  Pass 1 for
     greyscale and 2 for color.  This value is meaningless for still
     images.

     Returns: an ImageMagick Image object that suitably represents the 
     Davinci object in arg 1 if successful.  NULL if it fails.
     
  */

  ExceptionInfo imexc;
  ImageInfo * img_info;
  Image *img = NULL, *curframe, *prvframe;
  int x,y,z, curz, movietype;
  unsigned int scene=0;


  img_info = CloneImageInfo((ImageInfo *) NULL);
  GetExceptionInfo(&imexc);
  x = GetX(obj);
  y = GetY(obj);
  z = GetZ(obj);
  switch (z) {
  case 1:
    img = assemble_image(obj, 1, 1);
    break;
  case 3:
    img = assemble_image(obj, 3, 1);
    break;
  default:
    switch (type) {
    case 2: /* a color movie is expressly requested */
      if (z % 3 != 0) { /* already checked, but let's be safe... */
	parse_error("Color movies require a band count that is a multiple of 3");
	return (Image *) NULL;
      }
      movietype = 3;
      break;
    case 1: /* we can always do a grayscale */
      movietype = 1;
      break;
    default:
      /* caller hasn't been explicit.  We'll pick for them. Assume color if
	 it's doable */
      if (z % 3) movietype = 1; else movietype=3;
    }
    prvframe = NULL;
    for (curz=1; curz<GetZ(obj); curz+=movietype) {
      curframe = assemble_image(obj, movietype, curz);
      if (curframe == NULL) goto jump_error;
      if (prvframe == NULL) img = curframe; else prvframe->next = curframe;
      curframe->previous = prvframe;
      curframe->scene = scene++;
      prvframe = curframe;
    }
        
  }
  goto normal_exit;
 jump_error:
  if (img != NULL) {
    DestroyImage(img);
    img = NULL;
    parse_error("Error generating image.");
  }
 normal_exit:
  return img;
}


static Var * dvReadImage(vfuncptr f, Var *args)
{
  Var * rtn = NULL;
  parse_error("Unimplemented.");

  rtn = newVar();  /* try to return a value */
  V_TYPE(rtn) = ID_VAL;
  V_DSIZE(rtn) = V_SIZE(rtn)[0] = V_SIZE(rtn)[1] = V_SIZE(rtn)[2] = 1;
  V_ORG(rtn) = BSQ;
  V_FORMAT(rtn) = INT;
  V_DATA(rtn) = calloc(1, sizeof(int));
  V_INT(rtn) = 0;

  return(rtn);
}  

static Var * dvWriteImage(vfuncptr f, Var *args)
{
  /* Converts the image specified in arg1 to the type specified in arg2 */

  /* Validate our arguments */
  Var *image = NULL, *forceobj = NULL;
  Image * imimg;
  struct stat sbuf;
  char *itype = NULL, *fname = NULL;
  int z = 0, org = BSQ, force = 0, rtncode = 0, fd, i, movietype = 0;
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
  
  /* check file existence. */
  if (stat(fname, &sbuf) == -1) {
    switch (errno) {
    case ENOENT:  /* File doesn't exist. That's OK */
      break;
    default:
      /* Hmm... a permissions problem or a misspelled path component? */
      parse_error("Cannot open %s: %s", fname, strerror(errno));
      return(NULL);
    }
  }
  else {
    /* Don't overwrite files if user told us not to */
    if (force == 0) {
      parse_error("File exists.  Specify force=1.");
      return(NULL);
    }
    /* If we can't open the file in write mode, we have a problem. */
    if ((fd = open(fname, O_WRONLY) == -1)) {
      parse_error("Cannot open %s: %s", fname, strerror(errno));
      return(NULL);
    }
    else {
      close(fd);
    }
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is currently supported.");
    return(NULL);
  }

  z = GetZ(image);

  org = V_ORG(image);
  
  /* z rules:
     z = 1:  monochrome image format
     z = 2:  monochrome movie format
     z = 3:  RGB image format
     z > 3:  monochrome or color movie format
  */

  for (i=0; i<strlen(itype); i++) itype[i] = tolower(itype[i]);

  if (z == 2 || z > 3) {
    if (strcmp(itype, "mpgc") && strcmp(itype, "mpgg") &&
	strcmp(itype, "gifc") && strcmp(itype, "gifg")) {
      parse_error("You must specify a movie format for %i bands.");
      return(NULL);
    }
    if (itype[3] == 'c' && z % 3 != 0) {
      parse_error("Color movies require a band count that is a multiple of 3.");
      return(NULL);
    }
    if (itype[3] == 'c') movietype = 2;
    if (itype[3] == 'g') movietype = 1;
  }

  imimg = dv_to_im(image, movietype);
  if (imimg != NULL) rtncode = write_im_image(imimg, fname, itype);

  return newInt(rtncode);  /* try to return a value */
}



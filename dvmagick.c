/*

  DaVinci loadable module for using ImageMagick features.

  Copyright 2002, Mars Space Flight Facility,
  Department of Geological Sciences, Arizona State University

  Written January 8-24, 2002 by Randy Kaelber

  Notes:

  This code can provide access to a good chunk of the ImageMagick Library
  as a module, but it has been hacked to also provide built-in ReadImage and
  WriteImage support for the base DaVinci package's read() and write()
  functions.
  
  Bugs:
 
  1. Any manipulation routine that uses convolve, plus a few mathematically
  intensive ones that don't, produce a bunch of unwanted noise in the 
  resulting image.  I think this is a bug due to my imperfect understanding of
  ImageMagick, and ImageMagick's far short of perfect documentation.  I am
  still researching ImageMagick's source code trying to figure out how the
  mogrify command works, and I have a question pending on the ImageMagick
  user list.

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
#include "help.h"

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

#ifdef DAVINCI_MODULE
#define MODULE_TYPE static
#else
#define MODULE_TYPE
#endif

/* Local Function prototypes */

MODULE_TYPE Var *dvWriteImage(vfuncptr f, Var *args);
MODULE_TYPE  Var *dvReadImage(vfuncptr f, Var *args);

#ifdef DAVINCI_MODULE

static Var *dvAddNoiseImage(vfuncptr f, Var *args);
static Var *dvAppendImages(vfuncptr f, Var *args);
static Var *dvAverageImages(vfuncptr f, Var *args);
static Var *dvBlurImage(vfuncptr f, Var *args);
static Var *dvCharcoalImage(vfuncptr f, Var *args);
static Var *dvChopImage(vfuncptr f, Var *args);
static Var *dvColorizeImage(vfuncptr f, Var *args);
static Var *dvContrastImage(vfuncptr f, Var *args);
static Var *dvCropImage(vfuncptr f, Var *args);
static Var *dvDespeckleImage(vfuncptr f, Var *args);
static Var *dvEdgeImage(vfuncptr f, Var *args);
static Var *dvEmbossImage(vfuncptr f, Var *args);
static Var *dvEnhanceImage(vfuncptr f, Var *args);
static Var *dvEqualizeImage(vfuncptr f, Var *args);
static Var *dvFlattenImages(vfuncptr f, Var *args);
static Var *dvFlipImage(vfuncptr f, Var *args);
static Var *dvFlopImage(vfuncptr f, Var *args);
static Var *dvGammaImage(vfuncptr f, Var *args);
static Var *dvGaussianBlurImage(vfuncptr f, Var *args);
static Var *dvImplodeImage(vfuncptr f, Var *args);
static Var *dvLevelImage(vfuncptr f, Var *args);
static Var *dvMagnifyImage(vfuncptr f, Var *args);
static Var *dvMedianFilterImage(vfuncptr f, Var *args);
static Var *dvMinifyImage(vfuncptr f, Var *args);
static Var *dvModulateImage(vfuncptr f, Var *args);
static Var *dvMorphImages(vfuncptr f, Var *args);
static Var *dvMosaicImages(vfuncptr f, Var *args);
static Var *dvMotionBlurImage(vfuncptr f, Var *args);
static Var *dvNegateImage(vfuncptr f, Var *args);
static Var *dvNormalizeImage(vfuncptr f, Var *args);
static Var *dvOilPaintImage(vfuncptr f, Var *args);
static Var *dvReduceNoiseImage(vfuncptr f, Var *args);
static Var *dvResizeImage(vfuncptr f, Var *args);
static Var *dvRollImage(vfuncptr f, Var *args);
static Var *dvRotateImage(vfuncptr f, Var *args);
static Var *dvSampleImage(vfuncptr f, Var *args);
static Var *dvScaleImage(vfuncptr f, Var *args);
static Var *dvShadeImage(vfuncptr f, Var *args);
static Var *dvSharpenImage(vfuncptr f, Var *args);
static Var *dvShaveImage(vfuncptr f, Var *args);
static Var *dvShearImage(vfuncptr f, Var *args);
static Var *dvSpreadImage(vfuncptr f, Var *args);
static Var *dvSteganoImage(vfuncptr f, Var *args);
static Var *dvStereoImage(vfuncptr f, Var *args);
static Var *dvSwirlImage(vfuncptr f, Var *args);
static Var *dvUnsharpMaskImage(vfuncptr f, Var *args);
static Var *dvWaveImage(vfuncptr f, Var *args);


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
  {"AddNoiseImage", (void *)dvAddNoiseImage},
  {"AppendImages", (void *)dvAppendImages},
  {"AverageImages", (void *)dvAverageImages},
  {"BlurImage", (void *)dvBlurImage},
  {"CharcoalImage", (void *)dvCharcoalImage},
  {"ChopImage", (void *)dvChopImage},
  {"ColorizeImage", (void *)dvColorizeImage},
  {"ContrastImage", (void *)dvContrastImage},
  {"CropImage", (void *)dvCropImage},
  {"DespeckleImage", (void *)dvDespeckleImage},
  {"EdgeImage", (void *)dvEdgeImage},
  {"EmbossImage", (void *)dvEmbossImage},
  {"EnhanceImage", (void *)dvEnhanceImage},
  {"EqualizeImage", (void *)dvEqualizeImage},
  {"FlattenImages", (void *)dvFlattenImages},
  {"FlipImage", (void *)dvFlipImage},
  {"FlopImage", (void *)dvFlopImage},
  {"GammaImage", (void *)dvGammaImage},
  {"GaussianBlurImage", (void *)dvGaussianBlurImage},
  {"ImplodeImage", (void *)dvImplodeImage},
  {"LevelImage", (void *)dvLevelImage},
  {"MagnifyImage", (void *)dvMagnifyImage},
  {"MedianFilterImage", (void *)dvMedianFilterImage},
  {"MinifyImage", (void *)dvMinifyImage},
  {"ModulateImage", (void *)dvModulateImage},
  {"MorphImages", (void *)dvMorphImages},
  {"MosaicImages", (void *)dvMosaicImages},
  {"MotionBlurImage", (void *)dvMotionBlurImage},
  {"NegateImage", (void *)dvNegateImage},
  {"NormalizeImage", (void *)dvNormalizeImage},
  {"OilPaintImage", (void *)dvOilPaintImage},
  {"ReduceNoiseImage", (void *)dvReduceNoiseImage},
  {"ResizeImage", (void *)dvResizeImage},
  {"RollImage", (void *)dvRollImage},
  {"RotateImage", (void *)dvRotateImage},
  {"SampleImage", (void *)dvSampleImage},
  {"ScaleImage", (void *)dvScaleImage},
  {"ShadeImage", (void *)dvShadeImage},
  {"SharpenImage", (void *)dvSharpenImage},
  {"ShaveImage", (void *)dvShaveImage},
  {"ShearImage", (void *)dvShearImage},
  {"SpreadImage", (void *)dvSpreadImage},
  {"SteganoImage", (void *)dvSteganoImage},
  {"StereoImage", (void *)dvStereoImage},
  {"SwirlImage", (void *)dvSwirlImage},
  {"UnsharpMaskImage", (void *)dvUnsharpMaskImage},
  {"WaveImage", (void *)dvWaveImage},

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
  export_funcs, 49, NULL, 0
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
  parse_error("%s Unloaded.", modname);
}

/* End of preliminaries.  Let's get to work. */

/* strictly internal routines used by the module */

#endif /* DAVINCI_MODULE */

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

Image * read_im_image(char * fn) {
  /* Read an ImageMagick image from disk.

  Arguments:
  fn:  Where to find the file on disk

  Returns:
  a pointer to an ImageMagick Image object on success, NULL on failure 
  */

  ExceptionInfo exc;
  ImageInfo * img_info;
  Image * rtn;

  GetExceptionInfo(&exc);
  img_info=CloneImageInfo((ImageInfo *) NULL);
  strcpy(img_info->filename, fn);
  rtn = ReadImage(img_info,&exc);
  if (rtn == NULL) {
    parse_error("Image read failed: %s", exc.reason);
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
  register PixelPacket * line_pixels;
  unsigned int y = 0,z = 0;
  register unsigned int x = 0;

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
  /* im_to_dv: Takes an ImageMagick image or animation and returns
     a DaVinci object that represents the image.

     Arguments:
     img: The ImageMagick Image Object we want to convert

     Returns:
     A pointer to a DaVinci cube on success.  NULL pointer on failure.

  */
  
  unsigned long x = 0, y = 0, z = 0, i = 0, xi = 0, yi = 0, imgcount = 0;
  ExceptionInfo exc;
  PixelPacket p;
  Image * cur_frame;
  unsigned char *dvdata, mono;

  GetExceptionInfo(&exc);
  cur_frame = img;
  while (cur_frame != NULL) {
    z++;
    imgcount++;
    cur_frame = cur_frame->next;
  }
  mono = 0;
  /* if the image is color, then we have 3 bands for every image.
     If it's not color, we'll set our mono flag. 

     Aside: the ImageMagick documents are wrong about the IsGrayImage()
     method.  You must supply the exception structure as well.  Word
     to the wise when dealing with IM:  forget the docs, read the source
     code. 
  */

  if (!IsGrayImage(img, &exc)) z *= 3; else mono = 1;

  x = img->columns;
  y = img->rows;

  /* allocate the storage space for the the davinci cube data */
  if ((dvdata = calloc(x*y*z, sizeof(unsigned char))) == NULL) {
    parse_error("calloc() call failed.");
    return (NULL);
  }
  /* start with the first image.  If this is a still image, we'll
     never move away from it */
  cur_frame = img;
  /* we'll keep track of the bands by the image number, and just
     multiply by three when we deal with color */
  for(i=0; i<imgcount; i++) {

    /* get each pixel, left to right, top to bottom */
    for(yi=0; yi < y; yi++) {
      for(xi=0; xi < x; xi++) {
	if (mono) {
	  /* greyscale: just set one element.
	     Again, watch the differences between the documented API
	     and the one you actually find in the library. Doh!
	  */
	  p = GetOnePixel(cur_frame, xi, yi);

	  /* This index arithmetic may look frightful, but it's really
	     simple if you break it down. We'll always generate the
	     structure in BSQ format, so:

	     add xi bytes to represent that we are in column xi(ndex) of x
	     add x*yi to represent that we are in line yi(ndex) of y
	     add yi*xi*(bandno) to represent the current band.

	     bandno is just the image index for monochrome, so it's simply
	     i.  It's a little trickier for color. Band i through i+2 
	     represent red, green, and blue bands respectively.  So, red
	     becomes i*3, green i*3+1, and blue is i*3+2.

	  */
	  dvdata[xi+(x*yi)+(x*y*i)] = p.red;
	}
	else {
	  /* color: set an element in each of three bands */
	  p = GetOnePixel(cur_frame, xi, yi);
	  dvdata[xi+(x*yi)+(x*y*(i*3))] = p.red;
	  dvdata[xi+(x*yi)+(x*y*(i*3+1))] = p.green;
	  dvdata[xi+(x*yi)+(x*y*(i*3+2))] = p.blue;
	}
      }
    }
    /* advance to the next image if there's one */
    cur_frame = cur_frame->next;
    if (cur_frame == NULL) break; /* extra safety */
  }
  return newVal(BSQ, x, y, z, BYTE, dvdata);  
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
  int x,y,z, curz = 0, movietype = 0;
  unsigned int scene= 0;


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


MODULE_TYPE Var * dvReadImage(vfuncptr f, Var *args)
{
  Var * rtn = NULL;
  Image * img = NULL;
  Alist alist[3];
  int use_org = BSQ, i = 0;
  char * fname = NULL, * org = NULL;
  
  InitializeMagick("davinci");
  alist[0] = make_alist("filename", ID_STRING, NULL, &fname);
  alist[1] = make_alist("org", ID_ENUM, NULL, &org);
  alist[2].name = NULL;

  if (parse_args(f, args, alist) == 0) return(NULL);
  
  if (fname == NULL) {
    parse_error("Filename required.");
    goto error_exit;
  }
  if (org != NULL) {
  for (i=0; i<strlen(org); i++) org[i] = tolower(org[i]);
  if (!strcmp("bsq", org))
    use_org = BSQ;
  else if (!strcmp("bil", org))
    use_org = BIL;
  else if (!strcmp("bip", org))
    use_org = BIP;
  else
    parse_error("Unrecognized data format.  Will use BSQ.");
  }
  img = read_im_image(fname);
  if (img != NULL) rtn = im_to_dv(img);

  /* We'll use org eventually to translate this into the appropriate format */
 error_exit:
  if (img != NULL) DestroyImage(img);
  DestroyMagick();
  return(rtn);
}  


MODULE_TYPE int paramdvWriteImage(Var * image, char * fname, char *itype, int force) {
  struct stat sbuf;
  Image * imimg;
  int rtncode = 0, fd = -1, z = 0, org = BSQ, i = 0, movietype = 0;

  /* Parametric write function, to divorce argument parsing from the
     actual file writing tasks
  */
  
  /* check file existence. */
  if (stat(fname, &sbuf) == -1) {
    switch (errno) {
    case ENOENT:  /* File doesn't exist. That's OK */
      break;
    default:
      /* Hmm... a permissions problem or a misspelled path component? */
      parse_error("Cannot open %s: %s", fname, strerror(errno));
      goto error_exit;
    }
  }
  else {
    /* Don't overwrite files if user told us not to */
    if (force == 0) {
      parse_error("File exists.  Specify force=1.");
      goto error_exit;
    }
    /* If we can't open the file in write mode, we have a problem. */
    if ((fd = open(fname, O_WRONLY) == -1)) {
      parse_error("Cannot open %s: %s", fname, strerror(errno));
      goto error_exit;
    }
    else {
      close(fd);
    }
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
      goto error_exit;
    }
    if (itype[3] == 'c' && z % 3 != 0) {
      parse_error("Color movies require a band count that is a multiple of 3.");
      goto error_exit;
    }
    if (itype[3] == 'c') movietype = 2;
    if (itype[3] == 'g') movietype = 1;
  }

  imimg = dv_to_im(image, movietype);
  if (imimg != NULL) rtncode = write_im_image(imimg, fname, itype);
 error_exit:
  DestroyMagick();
  return (rtncode);
}



MODULE_TYPE Var * dvWriteImage(vfuncptr f, Var *args)
{
  /* Converts the image specified in arg1 to the type specified in arg2 */

  /* Validate our arguments */
  Var *image = NULL;
  char *itype = NULL, *fname = NULL;
  int  force = 0, rtncode = 0;
  char *valid_types[]={"avs","bmp","cmyk","gif","gifc","gifg",
		     "hist","jbig","jpeg","jpg","map","matte",
		     "miff","mpeg","mpgg","mpgc","mtv","pcd",
		     "pcx","pict","pm","pbm","pgm","ppm",
		     "pnm","ras","rgb","rgba","rle","sgi",
		     "sun","tga","tif","tiff","tile","vid",
		     "viff","xc","xbm","xpm","xv","xwd","yuv"};
  Alist alist[5];
  
  InitializeMagick("davinci");
  alist[0] = make_alist("object", ID_UNK, NULL, &image);
  alist[1] = make_alist("filename", ID_STRING, NULL, &fname);
  alist[2] = make_alist("type", ID_ENUM, valid_types, &itype);
  alist[3] = make_alist("force", INT, NULL, &force);
  alist[4].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument is required.");
    goto error_exit;
  }
  if (fname == NULL) {
    parse_error("Filename argument is required.");
    goto error_exit;
  }

  if (force != 0 && force != 1) {
    parse_error("force value must be 0 or 1.");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is currently supported.");
    goto error_exit;
  }
  
  rtncode = paramdvWriteImage(image, fname, itype, force);
 error_exit:
  return newInt(rtncode);
}


#ifdef DAVINCI_MODULE

static Var * dvAddNoiseImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick AddNoiseImage method.

      b = AddNoiseImage(a, noisetype)

      a: is an input image cube
      noisetype: is one of uniform, gaussian, multiplicative, impulse,
      laplacian, or poisson
      
      returns a new cube representing the transform performed on the object
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  int i;
  char * noisetype;
  char * noisetypes[] = {"uniform", "gaussian", "multiplicative", "impulse",
			 "laplacian", "poisson", ""};
  NoiseType ntype[] = {UniformNoise, GaussianNoise, MultiplicativeGaussianNoise,
		 ImpulseNoise, LaplacianNoise, PoissonNoise};
  NoiseType noise = UniformNoise;
  Alist alist[3];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);
  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("type", ID_ENUM, noisetypes, &noisetype); 
  alist[2].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (noisetype != NULL) {
    for (i=0; strlen(noisetypes[i]) > 0; i++) {
      if (!strcmp(noisetypes[i], noisetype)) {
	noise = ntype[i];
	break;
      }
    }
    if (strlen(noisetypes[i]) == 0) {
      parse_error("Unknown noise type %s.  Uniform assumed.\n", noisetype);
    }
  }
  	   
  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = AddNoiseImage(imimg, noise, &exc);

  if (outimg != NULL) { 
    dvoutput = im_to_dv(outimg);
  }
  else {
    parse_error("AddNoiseImage Failed: %s", exc.reason);
  }

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}



static Var * dvAppendImages(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick AppendImages method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  Alist alist[3];
  char * stackval[] = {"horiz", "vert", ""};
  char * stack;
  int stk;
  
  InitializeMagick("davinci");
  GetExceptionInfo(&exc);
  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("stack", ID_ENUM, stackval, &stack);
  alist[2].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  stk = 0;
  if (stack != NULL) {
    if (stack[0] == 'h') stk = 0; else stk = 1;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = AppendImages(imimg, stk, &exc); 

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvAverageImages(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick AverageImages method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  Alist alist[2];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);

  alist[1].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = AverageImages(imimg, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvBlurImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick BlurImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  double radius = 0.0, sigma = 0.0;
  ExceptionInfo exc;
  Alist alist[4];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("radius", DOUBLE, NULL, &radius);
  alist[2] = make_alist("sigma", DOUBLE, NULL, &sigma);
  alist[3].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  parse_error("Applying blur: radius=%f sigma=%f", radius, sigma);
  if (radius <= sigma) 
    parse_error("Warning: sigma larger than radius.  Strange results ahead...");

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = BlurImage(imimg, radius, sigma, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvCharcoalImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick CharcoalImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  double radius = 0.0, sigma = 0.0;
  ExceptionInfo exc;
  Alist alist[4];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("radius", DOUBLE, NULL, &radius);
  alist[2] = make_alist("sigma", DOUBLE, NULL, &sigma);
  alist[3].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = CharcoalImage(imimg, radius, sigma, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvChopImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick ChopImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  RectangleInfo rect;
  ExceptionInfo exc;
  Alist alist[6];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("x", INT, NULL, &rect.x);
  alist[2] = make_alist("y", INT, NULL, &rect.y);
  alist[3] = make_alist("width", INT, NULL, &rect.width);
  alist[4] = make_alist("height", INT, NULL, &rect.height);
  alist[5].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = ChopImage(imimg, &rect, &exc); 

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvColorizeImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick ColorizeImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  int opacity;
  char opacity_arg[10];
  PixelPacket brush;
  ExceptionInfo exc;
  Alist alist[6];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("opacity", INT, NULL, &opacity);
  alist[2] = make_alist("red", INT, NULL, &(brush.red));
  alist[3] = make_alist("green", INT, NULL, &(brush.green));
  alist[4] = make_alist("blue", INT, NULL, &(brush.blue));
  alist[5].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (opacity < 0 || opacity > 100) {
    parse_error("Opacity must be between 0 and 100, inclusive (percent).");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  sprintf(opacity_arg, "%i", opacity);

  outimg = ColorizeImage(imimg, opacity_arg, brush, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvContrastImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick ContrastImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL;
  unsigned int sharpen = 0;
  unsigned int rtn = 0;
  Alist alist[3];

  InitializeMagick("davinci");

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("sharpen", INT, NULL, &sharpen);
  alist[2].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  rtn = ContrastImage(imimg, sharpen);

  if (rtn == 1) dvoutput = im_to_dv(imimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvCropImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick CropImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  RectangleInfo rect;
  ExceptionInfo exc;
  Alist alist[6];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("x", INT, NULL, &rect.x);
  alist[2] = make_alist("y", INT, NULL, &rect.y);
  alist[3] = make_alist("width", INT, NULL, &rect.width);
  alist[4] = make_alist("height", INT, NULL, &rect.height);
  alist[5].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = CropImage(imimg, &rect, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}



static Var * dvDespeckleImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick DespeckleImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  Alist alist[2];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);

  alist[1].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = DespeckleImage(imimg, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvEdgeImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick EdgeImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  double radius = 0.0;
  ExceptionInfo exc;
  Alist alist[3];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("radius", DOUBLE, NULL, &radius);
  alist[2].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = EdgeImage(imimg, radius, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvEmbossImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick EmbossImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  double radius = 0.0, sigma = 0.0;
  Alist alist[4];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("radius", DOUBLE, NULL, &radius);
  alist[2] = make_alist("sigma", DOUBLE, NULL, &sigma);
  alist[3].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = EmbossImage(imimg, radius, sigma, &exc); 

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvEnhanceImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick EnhanceImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  Alist alist[2];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);

  alist[1].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = EnhanceImage(imimg, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}

static Var * dvEqualizeImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick EqualizeImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL;
  unsigned int rtn = 0;
  Alist alist[2];

  InitializeMagick("davinci");

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  rtn = EqualizeImage(imimg);

  if (rtn == 1) dvoutput = im_to_dv(imimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  DestroyMagick();
  return(dvoutput);
}



static Var * dvFlattenImages(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick FlattenImages method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  Alist alist[2];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);

  alist[1].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = FlattenImages(imimg, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvFlipImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick FlipImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  Alist alist[2];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);

  alist[1].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = FlipImage(imimg, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvFlopImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick FlopImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  Alist alist[2];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);

  alist[1].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = FlopImage(imimg, &exc); 

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}

static Var * dvGammaImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick GammaImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL;
  unsigned int rtn = 0;
  float gamma[4]  = {1.0, 1.0, 1.0, 1.0};
  char gamma_arg[256];
  int x;
  Var *gammaobj = NULL;
  Alist alist[3];

  InitializeMagick("davinci");

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("gamma", ID_VAL, NULL, &gammaobj);
  alist[2].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }
  
  if (gammaobj != NULL) {
  
    if (GetY(gammaobj) != 1 || GetZ(gammaobj) != 1 || GetX(gammaobj) > 4) {
      parse_error("Gamma object must be an nX1X1 cube, with n <= 4");
      goto error_exit;
    }
    for(x=1; x<=GetX(gammaobj); x++) {
      gamma[x-1] = extract_float(gammaobj, x);
    }
    if (GetX(gammaobj) == 1) {
      for(x=1; x<4; x++) gamma[x] = gamma[0];
    }
  }

  sprintf(gamma_arg, "%lf,%lf,%lf,%lf", gamma[0], gamma[1], gamma[2], gamma[3]);

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  rtn = GammaImage(imimg, gamma_arg);

  if (rtn == 1) dvoutput = im_to_dv(imimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  DestroyMagick();
  return(dvoutput);
}




static Var * dvGaussianBlurImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick GaussianBlurImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  double radius = 0.0, sigma = 0.0;
  Alist alist[4];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("radius", DOUBLE, NULL, &radius);
  alist[2] = make_alist("sigma", DOUBLE, NULL, &sigma);
  alist[3].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = GaussianBlurImage(imimg, radius, sigma, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvImplodeImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick ImplodeImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  double amount = 0.0;
  Alist alist[3];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("amount", DOUBLE, NULL, &amount);
  alist[2].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = ImplodeImage(imimg, amount, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}



static Var * dvLevelImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick LevelImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL;
  unsigned int rtn = 0;
  double blackpoint = 0.0, midpoint = 1.0, whitepoint = MaxRGB;

  char level_arg[256];
  Alist alist[5];

  InitializeMagick("davinci");

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("blackpoint", DOUBLE, NULL, &blackpoint);
  alist[2] = make_alist("midpoint", DOUBLE, NULL, &midpoint);
  alist[3] = make_alist("whitepoint", DOUBLE, NULL, &whitepoint);
  alist[4].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }
  
  sprintf(level_arg, "%lf,%lf,%lf", blackpoint, midpoint, whitepoint);

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  rtn = LevelImage(imimg, level_arg);

  if (rtn == 1) dvoutput = im_to_dv(imimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvMagnifyImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick MagnifyImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  Alist alist[2];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);

  alist[1].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = MagnifyImage(imimg, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvMedianFilterImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick MedianFilterImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  double radius = 0.0;
  Alist alist[3];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("radius", DOUBLE, NULL, &radius);
  alist[2].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = MedianFilterImage(imimg, radius, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvMinifyImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick MinifyImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  Alist alist[2];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);

  alist[1].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = MinifyImage(imimg, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}



static Var * dvModulateImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick LevelImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL;
  unsigned int rtn = 0;
  double brightness = 100.0, saturation = 100.0, hue = 100.0;

  char modulate_arg[256];
  Alist alist[5];

  InitializeMagick("davinci");

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("brightness", DOUBLE, NULL, &brightness);
  alist[2] = make_alist("saturation", DOUBLE, NULL, &saturation);
  alist[3] = make_alist("hue", DOUBLE, NULL, &hue);
  alist[4].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }
  
  sprintf(modulate_arg, "%lf,%lf,%lf", brightness, saturation, hue);

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  rtn = ModulateImage(imimg, modulate_arg);

  if (rtn == 1) dvoutput = im_to_dv(imimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  DestroyMagick();
  return(dvoutput);
}



static Var * dvMorphImages(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick MorphImages method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  unsigned int framecount;
  Alist alist[3];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("iterations", INT, NULL, &framecount);
  alist[2].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = MorphImages(imimg, framecount, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvMosaicImages(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick MosaicImages method.
      This function is not terribly useful until we can actually figure
      out how to use the page structure properly.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  Alist alist[2];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);

  alist[1].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = MosaicImages(imimg, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvMotionBlurImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick MotionBlurImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  double radius = 0.0, sigma = 0.0, angle = 0.0;
  Alist alist[5];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("radius", DOUBLE, NULL, &radius);
  alist[2] = make_alist("sigma", DOUBLE, NULL, &sigma);
  alist[3] = make_alist("angle", DOUBLE, NULL, &angle);
  alist[4].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = MotionBlurImage(imimg, radius, sigma, angle, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvNegateImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick NegateImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL;
  unsigned int grayscale = 0;
  unsigned int rtn = 0;
  Alist alist[3];

  InitializeMagick("davinci");

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("grayscale", INT, NULL, &grayscale);
  alist[2].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  rtn = NegateImage(imimg, grayscale);

  if (rtn == 1) dvoutput = im_to_dv(imimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvNormalizeImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick NormalizeImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL;
  unsigned int rtn = 0;
  Alist alist[2];

  InitializeMagick("davinci");

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  rtn = NormalizeImage(imimg);

  if (rtn == 1) dvoutput = im_to_dv(imimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  DestroyMagick();
  return(dvoutput);
}




static Var * dvOilPaintImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick OilPaintImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  double radius = 0.0;
  Alist alist[3];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("radius", DOUBLE, NULL, &radius);
  alist[2].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = OilPaintImage(imimg, radius, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvReduceNoiseImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick ReduceNoiseImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  double radius = 0.0;
  Alist alist[3];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("radius", DOUBLE, NULL, &radius);
  alist[2].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = ReduceNoiseImage(imimg, radius, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvResizeImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick ResizeImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  unsigned int columns = 0, rows = 0;
  char * filtertype = NULL;
  char * filtertypes[] = {"bessel", "blackman", "box", "catrom", "cubic",
			  "gaussian", "hanning", "hermite", "lanczos",
			  "mitchell", "point", "quadratic", "sinc",
			  "triangle", ""};
  FilterTypes ftype[] = {BesselFilter, BlackmanFilter, BoxFilter, CatromFilter,
			CubicFilter, GaussianFilter, HanningFilter,
			HermiteFilter, LanczosFilter, MitchellFilter,
			PointFilter, QuadraticFilter, SincFilter,
			TriangleFilter};
  double blur = 0.0;
  FilterTypes filter = UndefinedFilter;
  int i;
  float ratio = 0.0;
  Alist alist[6];
  
  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("columns", INT, NULL, &columns);
  alist[2] = make_alist("rows", INT, NULL, &rows);
  alist[3] = make_alist("type", ID_ENUM, filtertypes, &filtertype);
  alist[4] = make_alist("blur", DOUBLE, NULL, &blur);
  alist[5].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (filtertype != NULL) {
    for (i=0; strlen(filtertypes[i]) > 0; i++) {
      if (!strcmp(filtertypes[i], filtertype)) {
	filter = ftype[i];
	break;
      }
    }
  }
  else {
    parse_error("Filter type is required.");
    goto error_exit;
  }

  if (strlen(filtertypes[i]) == 0) {
    parse_error("Unknown filter type %s.", filtertype);
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  if (columns == 0 && rows == 0) {
    parse_error("Cannot specify zero for both rows and columns.");
    goto error_exit;
  }
  /* little extra feature:  We will keep the aspect ratio if we just
     supply one dimension. */
  if (columns == 0) {
    ratio = (float)rows / (float)GetY(image);
    columns = GetX(image) * ratio;
  }
  else if (rows == 0) {
    ratio = (float)columns / (float)GetX(image);
    rows = GetY(image) * ratio;
  }

  outimg = ResizeImage(imimg, columns, rows, filter, blur, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvRollImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick RollImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  Alist alist[4];
  long x = 0, y = 0;

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("x", INT, NULL, &x);
  alist[2] = make_alist("y", INT, NULL, &y);
  alist[3].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = RollImage(imimg, x,y, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvRotateImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick RotateImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  double angle = 0.0;
  Alist alist[2];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("angle", DOUBLE, NULL, &angle);
  alist[2].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = RotateImage(imimg, angle, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvSampleImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick SampleImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  unsigned int rows = 0, columns = 0;
  float ratio = 0.0;
  Alist alist[4];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("columns", INT, NULL, &columns);
  alist[2] = make_alist("rows", INT, NULL, &rows);
  alist[3].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;
  /* little extra feature:  We will keep the aspect ratio if we just
     supply one dimension. */
  if (columns == 0) {
    ratio = (float)rows / (float)GetY(image);
    columns = GetX(image) * ratio;
  }
  else if (rows == 0) {
    ratio = (float)columns / (float)GetX(image);
    rows = GetY(image) * ratio;
  }

  outimg = SampleImage(imimg, columns, rows, &exc); 

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvScaleImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick ScaleImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  unsigned int rows = 0, columns = 0;
  float ratio = 0.0;
  Alist alist[4];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("columns", INT, NULL, &columns);
  alist[2] = make_alist("rows", INT, NULL, &rows);
  alist[3].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  /* little extra feature:  We will keep the aspect ratio if we just
     supply one dimension. */
  if (columns == 0) {
    ratio = (float)rows / (float)GetY(image);
    columns = GetX(image) * ratio;
  }
  else if (rows == 0) {
    ratio = (float)columns / (float)GetX(image);
    rows = GetY(image) * ratio;
  }

  outimg = ScaleImage(imimg, columns, rows, &exc); 

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvShadeImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick ShadeImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  unsigned int colorshade = 0;
  double azimuth = 0.0, elevation = 0.0;

  Alist alist[5];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("colorshade", INT, NULL, &colorshade);
  alist[2] = make_alist("azimuth", DOUBLE, NULL, &azimuth);
  alist[3] = make_alist("elevation", DOUBLE, NULL, &elevation);
  alist[4].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = ShadeImage(imimg, colorshade, azimuth, elevation, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvSharpenImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick SharpenImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  double radius = 0.0, sigma = 0.0;
  Alist alist[4];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("radius", DOUBLE, NULL, &radius);
  alist[2] = make_alist("sigma", DOUBLE, NULL, &sigma);
  alist[3].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = SharpenImage(imimg, radius, sigma, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvShaveImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick ShaveImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  Alist alist[6];
  RectangleInfo rect;
  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("x", INT, NULL, &rect.x);
  alist[2] = make_alist("y", INT, NULL, &rect.y);
  alist[3] = make_alist("width", INT, NULL, &rect.width);
  alist[4] = make_alist("height", INT, NULL, &rect.height);
  alist[5].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = ShaveImage(imimg, &rect, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvShearImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick ShearImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  double xshear = 0.0, yshear = 0.0;
  Alist alist[4];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("xshear", DOUBLE, NULL, &xshear);
  alist[2] = make_alist("yshear", DOUBLE, NULL, &yshear);
  alist[3].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = ShearImage(imimg, xshear, yshear, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvSpreadImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick SpreadImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  unsigned int amount;
  Alist alist[3];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("amount", INT, NULL, &amount);
  alist[2].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = SpreadImage(imimg, amount, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvSteganoImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick SteganoImage method.
 */
  Var *image = NULL, *steg = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *stegimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  Alist alist[3];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("stegimage", ID_VAL, NULL, &steg);
  alist[2].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.");
    goto error_exit;
  }
  if (steg == NULL) {
    parse_error("Steganographic image required.");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;
  if ((stegimg = dv_to_im(steg, 0)) == NULL) goto error_exit;

  outimg = SteganoImage(imimg, stegimg, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);
  
 error_exit:
  if (stegimg != NULL) DestroyImage(stegimg);
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvStereoImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick StereoImage method.
 */
  Var *image = NULL, *offsetimg =  NULL, *dvoutput = NULL;
  Image *imimg = NULL, *osimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  Alist alist[3];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("offset", ID_VAL, NULL, &offsetimg);
  alist[2].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }
  if (offsetimg == NULL) {
    parse_error("Offset image required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;
  if ((osimg = dv_to_im(offsetimg, 0)) == NULL) goto error_exit;

  outimg = StereoImage(imimg, osimg, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (osimg != NULL) DestroyImage(osimg);
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvSwirlImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick SwirlImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  double angle = 0.0;
  Alist alist[3];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("angle", DOUBLE, NULL, &angle);
  alist[2].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = SwirlImage(imimg, angle, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvUnsharpMaskImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick UnsharpMaskImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  Alist alist[6];
  double radius = 0.0, sigma = 0.0, amount = 0.0, threshold = 0.0;
  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("radius", DOUBLE, NULL, &radius);
  alist[2] = make_alist("sigma", DOUBLE, NULL, &sigma);
  alist[3] = make_alist("amount", DOUBLE, NULL, &amount);
  alist[4] = make_alist("threshold", DOUBLE, NULL, &threshold);
  alist[5].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = UnsharpMaskImage(imimg, radius, sigma, amount, threshold, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}


static Var * dvWaveImage(vfuncptr f, Var *args) {
  /*
      Implements the ImageMagick WaveImage method.
 */
  Var *image = NULL, *dvoutput = NULL;
  Image *imimg = NULL, *outimg = NULL;
  ExceptionInfo exc;
  double amplitude = 0.0, length = 0.0;
  Alist alist[4];

  InitializeMagick("davinci");
  GetExceptionInfo(&exc);

  alist[0] = make_alist("object", ID_VAL, NULL, &image);
  alist[1] = make_alist("amplitude", DOUBLE, NULL, &amplitude);
  alist[2] = make_alist("length", DOUBLE, NULL, &length);
  alist[3].name = NULL;

  if (parse_args(f, args, alist) == 0) goto error_exit;
  if (image == NULL) {
    parse_error("Object argument required.\n");
    goto error_exit;
  }

  if (V_FORMAT(image) != BYTE) {
    parse_error("Only byte data is supported.\n");
    goto error_exit;
  }

  if ((imimg = dv_to_im(image, 0)) == NULL) goto error_exit;

  outimg = WaveImage(imimg, amplitude, length, &exc);

  if (outimg != NULL) dvoutput = im_to_dv(outimg);

 error_exit:
  if (imimg != NULL) DestroyImage(imimg);
  if (outimg != NULL) DestroyImage(outimg);
  DestroyMagick();
  return(dvoutput);
}

#endif /* DAVINCI_MODULE */

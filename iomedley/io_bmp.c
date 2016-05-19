/**************************** io_bmp.c ***************************
 *
 * Cary Knott
 * Nov 17, 2002
 * Based _HEAVILY_ on xvbmp.c, xv-3.10a and other code in this 
 * directory
 *
 * Reads in a bitmap file (1, 4, 8, 24 and 32 bit)
 *
 *****************************************************************/

#ifdef HAVE_CONFIG_H
#include <iom_config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#ifndef _WIN32
#include <unistd.h>
#endif /* _WIN32 */
#include <sys/types.h>
#include <string.h>
#include "iomedley.h"

#define BMP_MAGIC   "BM"

/* These defines are from xvbmp.c xv-3.10a */
#define BI_RGB  0
#define BI_RLE8 1
#define BI_RLE4 2

/*
 * These defines are also from xvbmp.c  This information tells me 
 * what kind of BMP header I am reading.  The types have changed
 * from short to int and new variables have been stored as opposed
 * to computing.  In BITMAPCOREHEADER, everything is prfixed with
 * "bc" and BITMAPINFOHEADER is prefixed with "bi".  Will use the
 * "bi" notation in this code since it is the new format.
 */
#define WIN_OS2_OLD 12
#define WIN_NEW     40
#define OS2_NEW     64

/* values 'picType' can take */
#define F_FULLCOLOR 0
#define F_GRAYSCALE 1
#define F_BWDITHER  2
#define F_REDUCED   3
#define F_MAXCOLORS 4
#define FERROR(fp) (ferror(fp) || feof(fp))
/* MONO returns total intensity of r,g,b triple (i = .33R + .5G + .17B) */
#define MONO(rd,gn,bl) ( ((int)(rd)*11 + (int)(gn)*16 + (int)(bl)*5) >> 5)


#define DEBUG 0

/* function prototypes */
int iom_isBMP(FILE *                    /* Open filehandle */
	      );		       	/* read the first few chars to make
			      		 * sure it is the correct type      */

int iom_GetBMPHeader(FILE *,                /* Open filehandle */
		     char *,                /* Filename */
		     struct iom_iheader *   /* Image data & geometry */
		     );
int iom_ReadBMP(FILE *,                     /* Open filehandle */
		char *,                     /* Filename */
		int *,                      /* x dim output */
		int *,                      /* y dim output */
		int *,                      /* z dim output */
		unsigned char **                   /* data output */
		);
int iom_WriteBMP(char *,                    /* Filename */
		 unsigned char *,                  /* Data */
		 struct iom_iheader *,      /* Header/geometry */
		 int                        /* Force overwrite */
		 );

/* Different read/write routines based on bits */
int loadBMP1(FILE *, unsigned char *, int, int, int, unsigned char*, unsigned char*, unsigned char*);
int loadBMP4(FILE *, unsigned char *, int, int, int, int, unsigned char*, unsigned char*, unsigned char*);
int loadBMP8(FILE *, unsigned char *, int, int, int, int, unsigned char*, unsigned char*, unsigned char*);
int loadBMP24(FILE *, unsigned char *, int, int, int);
void writeBMP1(FILE *, unsigned char *, int, int);
void writeBMP4(FILE *, unsigned char *, int, int);
void writeBMP8(FILE *, unsigned char *, int, int);
void writeBMP24(FILE *, unsigned char *, int, int);

unsigned int getshort(FILE *);
unsigned int getint(FILE *);
void  putshort(FILE *, int);
void  putint(FILE *, int);

static unsigned char pc2nc[256];

/* Functions */

/*
 * Returns 1 if fp is a BMP file, 0 otherwise. This is the same as 
 * reading the bfType field from the BITMAPFILEHEADER struct 
 */

int iom_isBMP(FILE *fp)
{
  int len;
  char buf[16];
  
  rewind(fp);
  len = fread(buf, 1, strlen(BMP_MAGIC), fp);
  return (len == strlen(BMP_MAGIC) && !strncmp(buf, BMP_MAGIC, len));
}

int iom_GetBMPHeader(FILE *fp, char *filename, struct iom_iheader *h)
{

  int           x, y, z;
  unsigned char *data;

  if (!iom_ReadBMP(fp, filename, &x, &y, &z, &data)) 
    {
      return 0;
    }

  // printf ("Initializing Header Now\n");
  iom_init_iheader(h);

  // printf ("x: %d, y: %d, z: %d", x, y, z);

  /* What is the right order? width(x) height(y) depth(z)? */
  h->size[0] = z;
  h->size[1] = x;
  h->size[2] = y;

  /* Where do I get this information? Assuming basics since this is BMP*/
  h->org = iom_BIP;
  h->eformat = iom_MSB_INT_1;
  h->format = iom_BYTE;

  /* bit data from bmp */
  h->data = data;
  // printf("Header set!\n");

  return 1;
  
} /* end iom_GetBMPHeader */

/*
 * iom_ReadBMP()
 *
 * Reads an open BMP file and returns image data and geometry.
 *
 * Returns: 1 on success, 0 on failure.
 *
 * Args:
 *
 * fp         open file
 * xout       image width passback
 * yout       image height passback
 * zout       number/bands passback (always 3; API consistency)
 * bits       bits per sample passback (always 8;  API consistency)
 * dout       image data buffer passback (allocated by this function)
 *
 */
int iom_ReadBMP (FILE *fp, 
                 char *filename, 
                 int  *xout, 
                 int  *yout, 
                 int  *zout, 
                 unsigned char **dout)
{
  int          i, c, rv;

  /* header info */
  unsigned int bfSize, bfOffBits, biSize, biPlanes;
  unsigned int biBitCount, biCompression, biSizeImage, biXPelsPerMeter;
  unsigned int biYPelsPerMeter, biClrUsed, biClrImportant;
  
  int           bPad;
  char          *cmpstr;
  unsigned char red[256], green[256], blue[256];

  *dout = (unsigned char *) NULL;
  
  /* start from the beginning */
  rewind(fp);
  
  /* We already know it is BMP or we wouldn't be here. */
  getc(fp);  
  getc(fp);
  
  bfSize = getint(fp);
  getshort(fp); getshort(fp);  /* reserved and ignored */
  
  bfOffBits = getint(fp); 
  
  /* This tells us if we are reading the CORE or INFO header */
  biSize          = getint(fp); 
  
  if (biSize == WIN_NEW || biSize == OS2_NEW) 
    {
      *xout           = getint(fp); // biWidth
      *yout           = getint(fp); // biHeight
      biPlanes        = getshort(fp);
      biBitCount      = getshort(fp);
      biCompression   = getint(fp);
      biSizeImage     = getint(fp);
      biXPelsPerMeter = getint(fp);
      biYPelsPerMeter = getint(fp);
      biClrUsed       = getint(fp);
      biClrImportant  = getint(fp);
    }
  
  else 
    {    /* old bitmap format */
      *xout           = getshort(fp);          /* Types have changed ! */
      *yout           = getshort(fp);
      biPlanes        = getshort(fp);
      biBitCount      = getshort(fp);
      
      /* Not in old versions so have to compute them*/
      biSizeImage = (((biPlanes * biBitCount * *xout)+ 32 -1)/32)*3* *yout;
      biCompression   = BI_RGB; 
      biXPelsPerMeter = biYPelsPerMeter = 0;
      biClrUsed       = biClrImportant  = 0;
    }
  
  /* error checking */
  if ((biBitCount!=1 && biBitCount!=4 && biBitCount!=8 && biBitCount!=24
       && biBitCount!=32 ) || 
      biPlanes!=1 || biCompression>BI_RLE4) 
    {
      if (iom_is_ok2print_errors())
	{
	  printf("Bogus BMP File!  (bitCount=%d, Planes=%d, Compression=%d)",
		  biBitCount, biPlanes, biCompression);
	}
      
      /* bmpError(bname, buf); */
      
      return 0;
    }
  
  if (((biBitCount==1 || biBitCount==24 || biBitCount!=32 ) && biCompression != BI_RGB) ||
      (biBitCount==4 && biCompression==BI_RLE8) ||
      (biBitCount==8 && biCompression==BI_RLE4)) 
    {
      if (iom_is_ok2print_errors())
	{
	  printf("Bogus BMP File!  (bitCount=%d, Compression=%d)",
		  biBitCount, biCompression);
	}
      return 0;
    }

  /* color map stuff here! grab from bottom if needed */
  bPad = 0;
  if (biSize != WIN_OS2_OLD) 
    {
      /* skip ahead to colormap, using biSize */
      c = biSize - 40;    /* 40 bytes read from biSize to biClrImportant */
      for (i=0; i<c; i++) getc(fp);
      bPad = bfOffBits - (biSize + 14);
      // printf("bpad set! %d\n");
    }
  
  *zout = 1; // number of color bands used - 1 for grayscale, 3 for rgb
  // assume grayscale until told otherwise

  /* load up colormap, if any */
  if (biBitCount!=24 && biBitCount !=32) 
    {
      int i, cmaplen;
      
      // printf("Loading a color map!\n");
      cmaplen = (biClrUsed) ? biClrUsed : 1 << biBitCount;
      for (i=0; i<cmaplen; i++) 
	{
	  blue[i]  = getc(fp);
	  green[i] = getc(fp);
	  red[i]   = getc(fp);
	  if (biSize != WIN_OS2_OLD) 
	    {
	      getc(fp);
	      bPad -= 4;
	    }
	  if ((*zout == 1) && ((red[i] != blue[i]) || (red[i] != green[i])))
	    {
	      *zout = 3; // color, not grayscale
	    }
	}

      /*      if (FERROR(fp))  */
      /* { bmpError(bname,"EOF reached in BMP colormap"); goto ERROR; } */

      // debug stuff to print out color map
      //printf("LoadBMP:  BMP colormap:  (RGB order)\n");
      // for (i=0; i<cmaplen; i++) 
      //	{
      // 	  printf("%02x%02x%02x  ", red[i],green[i],blue[i]);
      //	}
      //       printf("\n\n");
    }
  
  else 
    {
      *zout = 3; // rgb bands, not greyscale
      //   printf("No Colormap needed!\n");
    }
  
  // printf("bpad %d: ", bPad);
  if (biSize != WIN_OS2_OLD) 
    {
      /* Waste any unused bytes between the colour map (if present)
	 and the start of the actual bitmap data. */
      while (bPad > 0) 
	{
	  // printf("*");
	  (void) getc(fp);
	  bPad--;
	}
      // printf("\n");
    }
  /* end of color map stuff */
  
  /* width * height * # bands - rgb or grayscale? */
  *dout = (unsigned char *) calloc((size_t) *xout * *yout * *zout,
				   (size_t) 1);

  // printf("biBitCount: %d\n", biBitCount);

  /* load up the image, have to pass down the color map for 1, 4, and 8 bit */
  if      (biBitCount == 1) rv = loadBMP1(fp,*dout,*xout,*yout,*zout,red,blue,green);
  else if (biBitCount == 4) rv = loadBMP4(fp,*dout,*xout,*yout,*zout,
					  biCompression,red, blue, green);
  else if (biBitCount == 8) rv = loadBMP8(fp,*dout,*xout,*yout,*zout,
					  biCompression,red, blue, green);
  else                      rv = loadBMP24(fp,*dout,*xout,*yout,
					   biBitCount);
  // printf("Returned from loadBMP call: %d\n", rv);

  if (rv) printf("File appears truncated.  Winging it.\n");
  
  if      (biCompression == BI_RLE4) cmpstr = (char *)", RLE4 compressed";
  else if (biCompression == BI_RLE8) cmpstr = (char *)", RLE8 compressed";
  else cmpstr = (char *)"";

  //  printf("%sBMP\n", 
  //	 ((biSize==WIN_OS2_OLD) ? "Old OS/2 " :
  //	  (biSize==WIN_NEW)     ? "Windows "  : ""));
  //  printf("%d bit%s per pixel%s\n", biBitCount, (biBitCount == 1) ? "" : "s", cmpstr);
  
  //  printf("%dx%d BMP.\n", *xout, *yout);

  return 1;
} /* End iom_ReadBMP */

/*******************************************/
int loadBMP1(FILE *fp, unsigned char *data, int w, int h, int band, unsigned char *red, unsigned char *blue, unsigned char *green)
{
  int   i, j, c, bitnum, padw;
  unsigned char *pp;

  c = 0;
  padw = ((w + 31)/32) * 32;  
  
  // printf("Width: %d, Height: %d, padded Width: %d\n", w, h, padw);
  for (i=h-1; i>=0; i--)
    {
      pp = data + (i * w * band);
      for (j=bitnum=0; j<padw; j++,bitnum++) 
	{
	  if ((bitnum&7) == 0) 
	    { /* read the next unsigned char */
	      c = getc(fp);
	      bitnum = 0;
	    }
	  if (j<w) 
	    {
	      /* this is where I can put in the rgb values easily */
	      int s = (c & 0x80) ? 1 : 0;
	      *pp = red[s];
	      pp++;
	      if (band == 3)
		{
		  *pp = green[s];
		  pp++;
		  *pp = blue[s];
		  pp++;
		  // pp-=3;
		  //printf("blue: %02x ", (*pp));
		  // pp++;
		  // printf("green: %02x ", (*pp));
		  // pp++;
		}
		  // printf("red: %02x ", (*pp));
		  // pp++;
	      
	      c <<= 1;
	    }
	}
      if (FERROR(fp)) break;
    }
  return (FERROR(fp));
}  



/*******************************************/
int loadBMP4(FILE *fp, unsigned char *data, int w, int h, int band, int comp, unsigned char *red, unsigned char *blue, unsigned char *green)
{
  int   i,j,c,c1,x,y,nybnum,rv, padw;
  unsigned char *pp;
  
  
  rv = 0;
  c = c1 = 0;
  
  /* read uncompressed data */
  if (comp == BI_RGB) 
    {   
      /* 'w' padded to a multiple of 8pix (32 bits) */
      padw = ((w + 7)/8) * 8;  
    
      for (i=h-1; i>=0; i--) 
	{
	  pp = data + (i * w * band);
	  
	  for (j=nybnum=0; j<padw; j++,nybnum++) 
	    {
	      /* read next unsigned char */
	      if ((nybnum & 1) == 0) 
		{ 
		  c = getc(fp);
		  nybnum = 0;
		}
	
	      if (j<w) 
		{
		  int s = (c & 0xf0) >> 4;
		  *pp = red[s];
		  pp++;
		  if (band == 3)
		    {
		      
		      *pp = green[s];
		      pp++;
		      *pp = blue[s];
		      pp++;
		    }
		  c <<= 4;

		}
	    }
	  if (FERROR(fp)) break;
	}
    }

  /* read RLE4 compressed data */  
  else if (comp == BI_RLE4) 
    {  
      x = y = 0;  
      pp = data + x + (h-y-1)*w*band; /* 3 bands */
      
      while (y<h) 
	{
	  c = getc(fp);  
	  if (c == EOF) { rv = 1;  break; }

	  /* encoded mode */
	  if (c) 
	    {
	      c1 = getc(fp);
	      for (i=0; i<c; i++,x++,pp++) 
		{
		  int s = (i&1) ? (c1 & 0x0f) : ((c1>>4)&0x0f);
		  *pp = red[s];
		  pp++;
		  if (band == 3)
		    {
		      *pp = green[s];
		      pp++;
		      *pp = blue[s];
		      pp++;
		    }
 		}
	    }
      
	  /* c==0x00  :  escape codes */
	  else 
	    { 
	      c = getc(fp);  
	      if (c == EOF) { rv = 1;  break; }
	
	      /* end of line */
	      if (c == 0x00) 
		{
		  x=0;  
		  y++;  
		  pp = data + x + (h-y-1)*w*band;
		} 
	
	      else if (c == 0x01) break;               /* end of data */
	
	      /* delta */
	      else if (c == 0x02) 
		{
		  c = getc(fp);  x += c;
		  c = getc(fp);  y += c;
		  pp = data + x + (h-y-1)*w*band;
		}
	      
	      /* absolute mode */
	      else 
		{
		  for (i=0; i<c; i++, x++, pp++) 
		    {
		      int s;
		      if ((i&1) == 0) c1 = getc(fp);
		      s = (i&1) ? (c1 & 0x0f) : ((c1>>4)&0x0f);
		      *pp = red[s];
		      pp++;
		      if (band == 3)
			{
			  *pp = green[s];
			  pp++;
			  *pp = blue[s];
			  pp++;
			}
		    }
	  
		  if (((c&3)==1) || ((c&3)==2)) getc(fp);  /* read pad unsigned char */
		}
	    }  /* escape processing */
	  if (FERROR(fp)) break;
	}  /* while */
    }
  else 
    {
      fprintf(stderr,"unknown BMP compression type 0x%0x\n", comp);
    }
  
  if (FERROR(fp)) rv = 1;
  return rv;
}  



/*******************************************/
int loadBMP8(FILE *fp, unsigned char *data, int w, int h, int band, int comp, unsigned char *red, unsigned char *blue, unsigned char *green)
{
  int   i,j,c,c1,x,y,rv, padw;
  unsigned char *pp, *pend;
  
  rv = 0;

  pend = data + w * h * band;

  /* read uncompressed data */
  if (comp == BI_RGB) 
    {   
      /* 'w' padded to a multiple of 4pix (32 bits) */
      padw = ((w + 3)/4) * 4; 
      
      for (i=h-1; i>=0; i--) 
	{
	  pp = data + (i * w * band);
	  
	  for (j=0; j<padw; j++) 
	    {
	      c = getc(fp);  if (c==EOF) rv = 1;
	      if (j<w){
		*pp = red[c];
		pp++;
		if (band == 3)
		  {
		    *pp = green[c];
		    pp++;
		    *pp = blue[c];
		    pp++;
		  }
	      }
	    }
	  if (FERROR(fp)) break;
	}
    }
  
  /* read RLE8 compressed data */
  else if (comp == BI_RLE8) 
    {
      x = y = 0;  
      pp = data + x + (h-y-1)* w * band;

      while (y<h && *pp<=*pend) 
	{
	  c = getc(fp);  if (c == EOF) { rv = 1;  break; }

	  /* encoded mode */	  
	  if (c) 
	    {
	      c1 = getc(fp);
	      for (i=0; i<c && pp <=pend; i++,x++,pp++)
		{
		  *pp = red[c1];
		  pp++;
		  if (band == 3)
		    {
		      *pp = green[c1];
		      pp++;
		      *pp = blue[c1];
		      pp++;
		    }
		}
	    }

	  /* c==0x00  :  escape codes */
	  else 
	    {
	      c = getc(fp);  if (c == EOF) { rv = 1;  break; }

	      /* end of line */
	      if (c == 0x00) 
		{
		  x=0;  y++;  
		  pp = data + x + (h-y-1)*w*band;
		} 

	      else if (c == 0x01) break;               /* end of data */

	      /* delta */
	      else if (c == 0x02) 
		{
		  c = getc(fp);  x += c;
		  c = getc(fp);  y += c;
		  pp = data + x + (h-y-1)*w*band;
		}

	      /* absolute mode */	      
	      else 
		{
		  for (i=0; i<c && pp<=pend; i++, x++, pp++) {
		    c1 = getc(fp);
		    *pp = red[c1];
		    pp++;
		    if (band == 3)
		      {
			
			*pp = green[c1];
			pp++;
			*pp = blue[c1];
			pp++;
		      }
		  }
		  
		  if (c & 1) getc(fp);  /* odd length run: read an extra pad unsigned char */
		}
	    }  /* escape processing */
	  if (FERROR(fp)) break;
	}  /* while */
    }
  
  else 
    {
      fprintf(stderr,"unknown BMP compression type 0x%0x\n", comp);
    }

  if (FERROR(fp)) rv = 1;
  return rv;
}  



/*******************************************/
int loadBMP24(FILE *fp, unsigned char *data, int w, int h, int bits)
{
  int   i,j,padb,rv, r, g, b;
  unsigned char *pp;
  
  rv = 0;
  
  padb = (4 - ((w*3) % 4)) & 0x03;  /* # of pad bytes to read at EOscanline */
  
  for (i=h-1; i>=0; i--) {
    // printf("I: %d\n", i);
    pp = data + (i * w * 3);
    // printf("pp: %d\n", pp);
    
    for (j=0; j<w; j++) {
      // printf("J: %d\n", j);
      b = getc(fp);
      g = getc(fp);
      r = getc(fp);
      *pp = r; pp++;
      *pp = g; pp++;
      *pp = b; pp++;
    }

    for (j=0; j<padb; j++) getc(fp);

    rv = (FERROR(fp));
    if (rv) break;
  }
  return rv;
}  


/*
 * Writes the image data to a GIF,
 * quantizing down to 256 colors if necessary.
 *
 * Returns: 1 on success, 0 on failure.
 *
 * filename   GIF file for output.
 * data       Image data matrix.
 * h          iomedley header containing image geometry and org info.
 * force      Boolean, force file overwrite.
 *
 */

int iom_WriteBMP(char *filename, 
		 unsigned char *indata,
		 struct iom_iheader *h,
		 int force
		 )
{
  int x, y, z;
  unsigned char *data;
  FILE *fp = NULL;
  int i, nc, nbits, bperlin, cmaplen;
  unsigned char graymap[256];
  
  if (!force && access(filename, F_OK) == 0)
    {
      if (iom_is_ok2print_errors())
	{
	  fprintf(stderr, "File %s already exists.\n", filename);
	}
      return 0;
    }
  
  /* Yank geometry and organization details out of iomedley header. */
  // printf("Setting up size: ");
  if (h->org == iom_BIP) 
    {
      z = h->size[0];
      x = h->size[1];
      y = h->size[2];
    } 
  else if (h->org == iom_BSQ) 
    {
      x = h->size[0];
      y = h->size[1];
      z = h->size[2];
    } 
  else if (h->org == iom_BIL) 
    {
      x = h->size[0];
      z = h->size[1];
      y = h->size[2];
    } 
  else 
    {
      if (iom_is_ok2print_unsupp_errors()) 
	{
	  fprintf(stderr, "ERROR: org %d not supported by iom_WriteBMP()", h->org);
	}
      return 0;
    }
  
  //printf("x: %d, y: %d, z: %d\n", x, y, z);
  
  /* Convert data to BIP if not already BIP. */
  
  if (h->org == iom_BIP) 
    {
      data = indata;
    } 
  else 
    {
      /* iom__ConvertToBIP allocates memory, don't forget to free it! */
      if (!iom__ConvertToBIP(indata, h, &data)) 
	{
	  free(data);
	  return 0;
	}
    }
  
  /* Open file. */
  
  if ((fp = fopen(filename, "wb")) == NULL) 
    {
      if (iom_is_ok2print_unsupp_errors()) 
	{
	  fprintf(stderr, "ERROR: couldn't open %s for writing\n", filename);
	}
      if (h->org != iom_BIP) 
	{
	  free(data);
	}
      return 0;
    }
  
  nc = nbits = cmaplen = 0;
  if (z == 3) {  /* is F_FULLCOLOR */
    nbits = 24;
    // write out a 24-bit file
  }
  
  else /* if z = 1 */
    { 
      /* figure out what to do for mono */
      // printf("Band = 1.   Fill in Code Here\n");
      
      // Since it's grayscale, know you have 00 through ff -
      // 256 colors - write out an 8-bit file
      nbits = 8;
      cmaplen=256;
      nc = 256;
      // printf("setting up graymap\n");
      for(i = 0; i < 256; i++)
	{
	  graymap[i] = i;
	}
      // printf("Finished setting up graymap\n");
    }

  bperlin = ((x * nbits + 31) / 32) * 3;   /* # bytes written per line */

  putc('B', fp);  putc('M', fp);           /* BMP file magic number */
  // printf ("Just set BM\n");

  /* compute filesize and write it */
  i = 14 +                /* size of bitmap file header */
      40 +                /* size of bitmap info header */
      (nc * 4) +          /* size of colormap */
      bperlin * y;        /* size of image data */

  // printf("File Size: %d\n", i);

  putint(fp, i);
  putshort(fp, 0);        /* reserved1 */
  putshort(fp, 0);        /* reserved2 */
  putint(fp, 14 + 40 + (nc * 4));  /* offset from BOfile to BObitmap */

  putint(fp, 40);         /* biSize: size of bitmap info header */
  putint(fp, x);          /* biWidth */
  putint(fp, y);          /* biHeight */
  putshort(fp, 1);        /* biPlanes:  must be '1' */
  putshort(fp, nbits);    /* biBitCount: 1,4,8, or 24 */
  putint(fp, BI_RGB);     /* biCompression:  BI_RGB, BI_RLE8 or BI_RLE4 */
  putint(fp, bperlin*y);  /* biSizeImage:  size of raw image data */
  putint(fp, 75 * 39);    /* biXPelsPerMeter: (75dpi * 39" per meter) */
  putint(fp, 75 * 39);    /* biYPelsPerMeter: (75dpi * 39" per meter) */
  putint(fp, nc);         /* biClrUsed: # of colors used in cmap */
  putint(fp, nc);         /* biClrImportant: same as above */


  /* write out the colormap - only colormap we have is grayscale colormap*/
  for (i=0; i<nc; i++) {
    putc(graymap[i],fp);
    putc(graymap[i],fp);
    putc(graymap[i],fp);
    putc(0,fp);
  }

  /* write out the image */
  // if      (nbits ==  1) writeBMP1 (fp, data, x, y);
  // else if (nbits ==  4) writeBMP4 (fp, data, x, y);
  // else if (nbits ==  8) writeBMP8 (fp, data, x, y);
  if (nbits ==  8) writeBMP8 (fp, data, x, y);
  else if (nbits == 24) writeBMP24(fp, data, x, y);

  free(data);
  fclose(fp);
#ifndef VMS
  if (FERROR(fp)) return -1;
#else
  if (!FERROR(fp)) return -1;
#endif
  
  return 1;
}


	  
	  
/*******************************************/
void writeBMP1(FILE *fp, unsigned char *data, int w, int h)
{
  int   i,j,c,bitnum,padw;
  unsigned char *pp;

  padw = ((w + 31)/32) * 32;  /* 'w', padded to be a multiple of 32 */

  for (i=h-1; i>=0; i--) {
    pp = data + (i * w);  

    for (j=bitnum=c=0; j<=padw; j++,bitnum++) {
      if (bitnum == 8) { /* write the next unsigned char */
	putc(c,fp);
	bitnum = c = 0;
      }
      
      c <<= 1;

      if (j<w) {
	c |= (pc2nc[*pp++] & 0x01);
      }
    }
  }
}  



/*******************************************/
void writeBMP4(FILE *fp, unsigned char *data, int w, int h)
{
  int   i,j,c,nybnum,padw;
  unsigned char *pp;


  padw = ((w + 7)/8) * 8; /* 'w' padded to a multiple of 8pix (32 bits) */

  for (i=h-1; i>=0; i--) {
    pp = data + (i * w);

    for (j=nybnum=c=0; j<=padw; j++,nybnum++) {
      if (nybnum == 2) { /* write next unsigned char */
	putc((c&0xff), fp);
	nybnum = c = 0;
      }

      c <<= 4;

      if (j<w) {
	c |= (pc2nc[*pp] & 0x0f);
	pp++;
      }
    }
  }
}  



/*******************************************/
void writeBMP8(FILE *fp, unsigned char *data, int w, int h)
{
  // Since we are using 8 bit for greyscale, we can just assume that
  // the value of the pixel pointer (pp) data is the grayscale value -
  // and the grayscale value corresponds to the colormap value - 0 to 255.

  int   i,j,padw;
  unsigned char *pp;

  padw = ((w + 3)/4) * 4; /* 'w' padded to a multiple of 4pix (32 bits) */

  for (i=h-1; i>=0; i--) {
    pp = data + (i * w);
    for (j=0; j<w; j++) 
      { 
	// printf("%d\t", *pp);
	putc(*pp, fp); 
	pp++;
      }
    // printf("\n");
    for ( ; j<padw; j++) putc(0, fp);
  }
}  


/*******************************************/
void writeBMP24(FILE *fp, unsigned char *data, int w, int h)
{
  int   i,j,padb;
  unsigned char *pp;

  padb = (4 - ((w*3) % 4)) & 0x03;  /* # of pad bytes to write at EOscanline */

  for (i=h-1; i>=0; i--) {
    pp = data + (i * w * 3);

    for (j=0; j<w; j++) {
      putc(pp[2], fp);
      putc(pp[1], fp);
      putc(pp[0], fp);
      pp += 3;
    }

    for (j=0; j<padb; j++) putc(0, fp);
  }
}  


/*******************************************/
unsigned int getshort(FILE *fp)
{
  int c, c1;
  c = getc(fp);  c1 = getc(fp);
  return ((unsigned int) c) + (((unsigned int) c1) << 8);
}


/*******************************************/
unsigned int getint(FILE *fp)
{
  int c, c1, c2, c3;
  c = getc(fp);  c1 = getc(fp);  c2 = getc(fp);  c3 = getc(fp);
  return ((unsigned int) c) +
         (((unsigned int) c1) << 8) + 
	 (((unsigned int) c2) << 16) +
	 (((unsigned int) c3) << 24);
}


/*******************************************/
void putshort(FILE *fp, int i)
{
  int c, c1;

  c = ((unsigned int ) i) & 0xff;  c1 = (((unsigned int) i)>>8) & 0xff;
  putc(c, fp);   putc(c1,fp);
}


/*******************************************/
void putint(FILE *fp, int i)
{
  int c, c1, c2, c3;
  c  = ((unsigned int ) i)      & 0xff;  
  c1 = (((unsigned int) i)>>8)  & 0xff;
  c2 = (((unsigned int) i)>>16) & 0xff;
  c3 = (((unsigned int) i)>>24) & 0xff;

  putc(c, fp);   putc(c1,fp);  putc(c2,fp);  putc(c3,fp);
}

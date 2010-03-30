#include "parser.h"
#include <math.h>


//stack friendly and fast floodfill algorithm
static void floodFillScanline(int x, int y, int h, int w,  int newColor, int oldColor, unsigned char *screenBuffer)
{
	if(oldColor == newColor) return;
	if(screenBuffer[y*w + x] != oldColor) return;
  
	int y1;
  
	//draw current scanline from start position to the top
	y1 = y;
	while(y1 < h && screenBuffer[y1*w + x] == oldColor)
	{
		screenBuffer[y1*w + x] = newColor;
		y1++;
	}    
  
	//draw current scanline from start position to the bottom
	y1 = y - 1;
	while(y1 >= 0 && screenBuffer[y1*w + x] == oldColor)
	{
		screenBuffer[y1*w + x] = newColor;
		y1--;
	}
  
	//test for new scanlines to the left
	y1 = y;
	while(y1 < h && screenBuffer[y1*w + x] == newColor)
	{
		if(x > 0 && screenBuffer[x - 1 + y1*w] == oldColor) 
		{
			floodFillScanline(x - 1, y1, h, w, newColor, oldColor, screenBuffer);
		} 
		y1++;
	}
	y1 = y - 1;
	while(y1 >= 0 && screenBuffer[y1*w + x] == newColor)
	{
		if(x > 0 && screenBuffer[x - 1 + y1*w] == oldColor) 
		{
			floodFillScanline(x - 1, y1, h,w, newColor, oldColor, screenBuffer);
		}
		y1--;
	} 
  
	//test for new scanlines to the right 
	y1 = y;
	while(y1 < h && screenBuffer[y1*w + x] == newColor)
	{
		if(x < w - 1 && screenBuffer[x + 1 + y1*w] == oldColor) 
		{           
			floodFillScanline(x + 1, y1, h,w, newColor, oldColor, screenBuffer);
		} 
		y1++;
	}
	y1 = y - 1;
	while(y1 >= 0 && screenBuffer[y1*w + x] == newColor)
	{
		if(x < w - 1 && screenBuffer[x + 1 + y1*w] == oldColor) 
		{
			floodFillScanline(x + 1, y1, h,w, newColor, oldColor, screenBuffer);
		}
		y1--;
	}
}


Var *ff_flood_fill(vfuncptr func, Var * arg)
{

  Var    *pic = NULL;            /* the orignial dcs pic */
  Var    *out = NULL;            /* the output pic */
  unsigned char *w_pic;                 /* modified image */
  int     i, j;               /* loop indices */
  int     x=0, y=0;      			   /* size of the picture */
	int			sx=0,sy=0;						 /* starting x and y positions*/
	int     fill=0,val=0;

  Alist alist[6];
  alist[0] = make_alist("data",      ID_VAL,     NULL,  &pic);
	alist[1] = make_alist("xpos",      INT,        NULL,  &sx);
	alist[2] = make_alist("ypos",      INT,        NULL,  &sy);
	alist[3] = make_alist("fill",      INT,        NULL,  &fill);
  alist[4] = make_alist("value",     INT,        NULL,  &val);
  alist[5].name = NULL;
  
  if (parse_args(func, arg, alist) == 0) return(NULL);

  if (pic == NULL) {
		parse_error("flood_fill() - 12/1/2007");
    parse_error("\nUsed to flood fill an image from one value to another");
    parse_error("Takes an input value and position and fills all pixels");
		parse_error("with the same value that touch the given pixel to a new value\n");
    parse_error("data = picture to be filled (byte data only)");
    parse_error("xpos = start x search position");
    parse_error("ypos = start y search position");
    parse_error("fill = fill value");
    parse_error("value = search value\n");
    parse_error("c.edwards 11/15/07\n");
    return NULL;
  }

	if(GetZ(pic) > 1 ) {
		parse_error("Single band data only\n");
		return NULL;
	}
  /* x, y and z dimensions of the data */
  x = GetX(pic);
  y = GetY(pic);
  
  /* allocate memory for the picture */
  w_pic = (unsigned char *)calloc(sizeof(char), x*y);
  
  if(w_pic == NULL) return NULL;

  /* loop through data and extract new points with null value of 0 ignored */
	for(i=0; i<x; i++) {
		for(j=0; j<y; j++) {
		 	w_pic[j*x + i] = (unsigned char)extract_int(pic, cpos(i,j,0, pic));
		}
  }

	floodFillScanline(sx, sy, y, x, fill, val, w_pic);
	  
  /* return the modified data */
  out = newVal(BSQ, x, y, 1, BYTE, w_pic);
  return out;
}

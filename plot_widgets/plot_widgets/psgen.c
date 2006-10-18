#include <X11/StringDefs.h>
#include <X11/Intrinsic.h> 
#include <X11/Xmd.h>
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#if XmVersion == 1002
#include <Xm/PrimitiveP.h>
#endif
#include <Xm/DrawingA.h>
#include <X11/Xutil.h>
/*?*/
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "2DGeom.h"
#include "2DHistDefs.h"
#include "2DHist.h"
#include "uniLab.h"
#include "axes.h"
#include "labels.h"		
#include "2DHistP.h"
#include "imaggen.h"
#include "apsgen.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif  

/* Amount of space to leave on the edge of the printer page in 72nds of an
   inch.  If this is too big, part of the page will be wasted, too small,
   and some (espescially color) printers will clip off part of the plot */
#define PAGE_MARGIN 18

static void drawPsArrows (Hist2DWidget w,  FILE *psfile);
static void drawLabels (Hist2DWidget w, FILE * psfile);
static void putData(FILE *psfile, discreteData *zData, 
  discreteData *zTopErrors, discreteData *zBottomErrors, int dataWidth,
  int i, int j);

#define FONT_DELTA 2

/* 	
** static void drawPsArrows (Hist2DWidget w, FILE *psfile )
**
** 	w	Specifies the widget
**	*psfile	Specifies output file for PostScript program being generated
**
**		Generates PostScript code for arrows droing according current
**		widget state and puts it in file specifies.
**		Does not change any widget information, uses it read only
**		Does not perform any operation with file, but "fprintf" only
**		Does not check any I/O errors
**		File should be opened before call
*/
static void drawPsArrows (Hist2DWidget w, FILE *psfile )
{
    vector v;
    vector u;
    discreteMap *dMap;
    range2d rv, rh;
    if (w->hist2D.discrMap==NULL) return;
    if (w->hist2D.sourceHistogram == NULL) return;
    dMap= w->hist2D.discrMap;
    
    fprintf(psfile,"/ar_length  %d def  \n",ARROW_LENGTH << LOWBITS);
    fprintf(psfile,"/ar_angle   %d def  \n",(int)((double)ARROW_ANGLE / M_PI *180.));
    
    fprintf(psfile,"/arrow		%% x1 y1 x0 y0 =>		\n");
    fprintf(psfile,"%% fuction to draw arrow, (x0 y0) - coordinates	\n");
    fprintf(psfile,"%% of arrow end, ((0 0), (x1 y1)) - any vector	\n");
    fprintf(psfile,"%% whith the direction opposite to arrow		\n");	
    fprintf(psfile,"{    						\n");
    fprintf(psfile,"    gsave						\n");
    fprintf(psfile,"    newpath						\n");
    fprintf(psfile,"    moveto						\n");
    fprintf(psfile,"    currentpoint					\n");
    fprintf(psfile,"    translate					\n");
    fprintf(psfile,"    newpath						\n");
    fprintf(psfile,"    0 0 moveto					\n");
    fprintf(psfile,"    2 copy						\n");
    fprintf(psfile,"    ar_length 3 div					\n");
    fprintf(psfile,"    set_len						\n");
    fprintf(psfile,"    lineto						\n");
    fprintf(psfile,"    2 copy						\n");
    fprintf(psfile,"    ar_length set_len				\n");
    fprintf(psfile,"    ar_angle					\n");
    fprintf(psfile,"    rotate						\n");
    fprintf(psfile,"    2 copy						\n");
    fprintf(psfile,"    lineto						\n");
    fprintf(psfile,"    closepath					\n");
    fprintf(psfile,"    0 setgray					\n");
    fprintf(psfile,"    fill						\n");
    fprintf(psfile,"    ar_angle					\n");
    fprintf(psfile,"    neg						\n");
    fprintf(psfile,"    rotate						\n");
    fprintf(psfile,"    newpath						\n");
    fprintf(psfile,"    0 0 moveto					\n");
    fprintf(psfile,"    2 copy						\n");
    fprintf(psfile,"    ar_length 3 div					\n");
    fprintf(psfile,"    set_len						\n");
    fprintf(psfile,"    lineto						\n");
    fprintf(psfile,"    2 copy						\n");
    fprintf(psfile,"    ar_length set_len				\n");
    fprintf(psfile,"    ar_angle					\n");
    fprintf(psfile,"    neg						\n");
    fprintf(psfile,"    rotate						\n");
    fprintf(psfile,"    2 copy						\n");
    fprintf(psfile,"    lineto						\n");
    fprintf(psfile,"    closepath					\n");
    fprintf(psfile,"    0 setgray					\n");
    fprintf(psfile,"    fill						\n");
    fprintf(psfile,"    grestore					\n");
    fprintf(psfile,"} def						\n");
    
    if ((w->hist2D.bins != NULL || w->hist2D.aHist != NULL)
      && dMap->zFactor < -12)
/* if histogam data exists and picture height allow to draw Z arrows 	   */
    {
    	v = subVectors(dMap->vertex,yColomn(dMap->map));
    	u = makeVector(0, dMap->zFactor);
        if (w->hist2D.zMin < w->hist2D.zVisibleStart)
/* draw Z bottom arrow if needed					   */
            fprintf(psfile,"%d %d %d %d arrow\n", u.x, u.y,
              v.x << LOWBITS, v.y << LOWBITS);
        v = addVectors(v, makeVector(0,dMap->zFactor));
    	u = makeVector(0, -dMap->zFactor);
        if (w->hist2D.zMax > w->hist2D.zVisibleEnd)
/* draw Z top arrow if needed						   */
            fprintf(psfile,"%d %d %d %d arrow\n", u.x, u.y,
              v.x << LOWBITS, v.y << LOWBITS);
    }
/* rotate visible and histogram XY ranges range to current position  	   */    
    rv = transformRange2d (w->hist2D.xyVisibleRange, dMap->rotMatr);
    rh = transformRange2d (w->hist2D.xyHistoRange, dMap->rotMatr);
    if (vectorLength(xColomn(dMap->map)) > 12)
/*  if enougt space along front left edge of histogram base (virtual X axis)*/
    {
    	v = dMap->vertex;
    	u = mulVector(xColomn(dMap->map),-1);
        if (rv.end.x != rh.end.x)
/* draw rigth arrow if needed						    */
            fprintf(psfile,"%d %d %d %d arrow\n", u.x, u.y,
              v.x << LOWBITS, v.y << LOWBITS);
	v = subVectors(v,xColomn(dMap->map));
    	u = xColomn(dMap->map);
    	if (rv.start.x!=rh.start.x)
/* draw left arrow if needed						    */
            fprintf(psfile,"%d %d %d %d arrow\n", u.x, u.y,
              v.x << LOWBITS, v.y << LOWBITS);
    }
    if (vectorLength(yColomn(dMap->map)) > 12)
/*  if enough space along front right edge of histogram base (virtual Y axis)*/
    {
    	v = dMap->vertex;
    	u = mulVector(yColomn(dMap->map),-1);
        if (rv.end.y!=rh.end.y)
/* draw left arrow if needed						    */
            fprintf(psfile,"%d %d %d %d arrow\n", u.x, u.y,
              v.x << LOWBITS, v.y << LOWBITS);
	v = subVectors(v,yColomn(dMap->map));
    	u = yColomn(dMap->map);
    	if (rv.start.y!=rh.start.y)
/* draw right arrow if needed						    */
            fprintf(psfile,"%d %d %d %d arrow\n", u.x, u.y,
              v.x << LOWBITS, v.y << LOWBITS);
    }
}

/*
** 
**  static void putData(FILE *psfile, discreteData *zData, 
**    discreteData *zTopErrors, discreteData *zBottomErrors, int dataWidth,
**    int i, int j)
**
**  psfile	PostScript file being generated
**  zData	prescaled array of bin data
**  zTopError	prescaled array of histogramm top errors
**  zBottomError prescaled array of histogramm bottom errors
**  dataWidth	wdith of data arrays above
**  i, j	row and column indexes of particular bin 
**
**  	Put bin data for bin determining by <i,j> into psfile.
**
*/
static void putData(FILE *psfile, discreteData *zData, 
  discreteData *zTopErrors, discreteData *zBottomErrors, int dataWidth,
  int i, int j)
{
    if (zBottomErrors != NULL)
	fprintf(psfile,"%d %d\n", zBottomErrors[i * dataWidth + j].data -
	  zData[i * dataWidth + j].data, 
	  (zBottomErrors[i * dataWidth + j].clipped) ? 1 : 0);
    if (zTopErrors != NULL)
	fprintf(psfile,"%d %d\n", zTopErrors[i * dataWidth + j].data -
	  zData[i * dataWidth +j ].data, 
	  (zTopErrors[i * dataWidth + j].clipped) ? 1 : 0);
    fprintf(psfile,"%d %d\n", zData[i * dataWidth + j].data, 
      (zData[i * dataWidth + j].clipped) ? 1 : 0);
}

/* 	
** void hist2DmakePsImage(Widget wid, char * fileName)
**
** 	wid	Specifies the widget
**	*fileName	
**		Specifies output file name for PostScript program being generated
**
**		Generates PostScript code to draw current widget state graphics
**		creates a file with file name specified and puts code it in the file,
**		if the widget has any graphics generated.
**		Othervise (if the widget in current state has not enough information 
**		to produce any graphics) does not create the file
**		Does not change any widget information, uses it read only
**		Does nothing if fails to open the file
**		Does not check any other I/O errors
**		Closes file before return
*/

void hist2DmakePsImage(Widget wid, char * fileName)
{
    Hist2DWidget w =(Hist2DWidget) wid;
    FILE *psfile;
    time_t tt;

/* check does the widget has data to generate graphics				*/    
    if ((w->hist2D.sourceHistogram==NULL)||(w->hist2D.discrMap==NULL))
        return;
/* open file and return if failed						*/
    psfile=fopen(fileName,"w");
    if (psfile == NULL)
        return;   

    fprintf(psfile, "%%!PS-Adobe-3.0 EPSF-3.0\n");
    fprintf(psfile, "%%%%BoundingBox: %d %d %d %d \n", PAGE_MARGIN, PAGE_MARGIN,
    	    w->core.width + PAGE_MARGIN, w->core.height + PAGE_MARGIN);
    fprintf(psfile, "%%%%Title: %s \n", fileName);
    time(&tt);
    fprintf(psfile, "%%%%CreationDate: %s \n\n", ctime(&tt));
    fprintf(psfile,"%% Set up a small margin around the\n");
    fprintf(psfile,"%% page since most PostScript printers\n");
    fprintf(psfile,"%% can't print all of the way to the edge.\n");
    fprintf(psfile,"%d	%d translate\n", PAGE_MARGIN, PAGE_MARGIN); 
    
    hist2DwritePs(wid, psfile);

    fprintf(psfile,"showpage\n");
    fclose (psfile);
}

/* 	
** void hist2DwritePs(Widget wid, FILE *psfile)
**
** 	wid	Specifies the widget
**	*psfile	Specifies output file for PostScript program being generated
**
**		Generates PostScript code to draw current widget state graphics
**		puts PostScript code in the file, to draw the widget's image
**		if the widget has any graphics generated.  Othervise (if the
**		widget in current state has not enough information 
**		to produce any graphics) does not write anything in the file
**		Does not change any widget information, uses it read only
**
**		The image extends from (0, 0) to the width and height of
**		the widget.
*/
void hist2DwritePs(Widget wid, FILE *psfile)
{
    Hist2DWidget w =(Hist2DWidget) wid;
    time_t tt;
    discreteMap *dMap;
    matrix2 map;
    vector vertex;
    short i,j;
    int xp, yp;
    int errorBinWidth = ERROR_BIN_WIDTH;
    discreteData *zData;
    discreteData *zTopErrors;
    discreteData *zBottomErrors;
    binPosDescr binCellPositions;/* bin cellspositions in virtual coordinates */
    binPosDescr binPositions;	/* bin positions in virtual coordinates    */
    binPosDescr er_binPositions;	/* bin positions in virtual coordinates    */
    Boolean errorBarsOn;
/* check does the widget has data to generate graphics				*/    
    if ((w->hist2D.sourceHistogram==NULL)||(w->hist2D.discrMap==NULL))
        return;

    dMap=w->hist2D.discrMap;
    vertex=(dMap->vertex);
    map = (dMap->map);

    fprintf(psfile,"/set_len 	%% x y l => x/sqrt(x*x + y*y)*l 	\n");
    fprintf(psfile,"%%			    y/sqrt(x*x + y*y)*l		\n");
    fprintf(psfile,"%% Pull (or squeeze) vector to requested length	\n");
    fprintf(psfile,"%% x, y - vector (assume ((0,0) (x,y))), l -length	\n"); 
    fprintf(psfile,"{							\n");
    fprintf(psfile,"    3 copy	%% x y l x y l				\n");
    fprintf(psfile,"    pop	%% x y l x y 				\n");
    fprintf(psfile,"    dup	%% x y l x y y				\n");
    fprintf(psfile,"    mul	%% x y l x y*y				\n");
    fprintf(psfile,"    exch	%% x y l y*y x				\n");
    fprintf(psfile,"    dup	%% x y l y*y x x			\n");
    fprintf(psfile,"    mul	%% x y l y*y x*x			\n");
    fprintf(psfile,"    add	%% x y l y*y+x*x			\n");
    fprintf(psfile,"    sqrt	%% x y l sqrt(y*y+x*x)			\n");
    fprintf(psfile,"    div	%% x y l/sqrt(y*y+x*x)			\n");
    fprintf(psfile,"    dup	%% x y l/sqrt(y*y+x*x) l/sqrt(y*y+x*x)	\n");
    fprintf(psfile,"    4 3 roll%% y l/sqrt(y*y+x*x) l/sqrt(y*y+x*x) x	\n");
    fprintf(psfile,"    mul	%% y l/sqrt(y*y+x*x) x*l/sqrt(y*y+x*x)	\n");
    fprintf(psfile,"    3 1 roll%% x*l/sqrt(y*y+x*x) y l/sqrt(y*y+x*x)	\n");
    fprintf(psfile,"    mul	%% x*l/sqrt(y*y+x*x) y*l/sqrt(y*y+x*x)	\n");
    fprintf(psfile,"} def  						\n");

    fprintf(psfile,"/window_row_start 		%%xobj => row_xw row_yw	\n");
    fprintf(psfile,"%% xobj - X coordinates of bar row start in the	\n");
    fprintf(psfile,"%% object coordinate system, relatively to current	\n");
    fprintf(psfile,"%% visible region. 				\n");
    fprintf(psfile,"%% Corresponding Y coordinate of bar row start is 	\n");
    fprintf(psfile,"%% alwase 0 (relatively to current visible region	\n");
    fprintf(psfile,"%% so it is not used				\n");
    fprintf(psfile,"{							\n");
    fprintf(psfile,"    dup     %% xobj  xobj 				\n");
    fprintf(psfile,"    %d mul	%% xobj  ..				\n", 
      map.x.x << LOWBITS);
    fprintf(psfile,"    %d div	%% .. / mapLength.x			\n",
      dMap->mapLength.x);
    fprintf(psfile,"    %d add	%% xobj xw 				\n", 
      vertex.x << LOWBITS);
    fprintf(psfile,"    exch   		 				\n");
    fprintf(psfile,"    %d mul	%%  .. 					\n", 
      map.y.x << LOWBITS);
    fprintf(psfile,"    %d div	%% .. / mapLength.x			\n",
      dMap->mapLength.x);
    fprintf(psfile,"    %d add	%%  yw					\n", 
      vertex.y << LOWBITS);
    fprintf(psfile,"} def						\n");

    fprintf(psfile,"/window_bar_start 		%% yobj => xw yw	\n");
    fprintf(psfile,"%% yobj - Y coordinates of bar start in the		\n");
    fprintf(psfile,"%% object coordinate system, relatively to current	\n");
    fprintf(psfile,"%% visible region. 					\n");
    fprintf(psfile,"%% xw yw - window coordinates (multiplied by 	\n");
    fprintf(psfile,"%% MAXPIC for PostSript) of this object point under \n");
    fprintf(psfile,"%% current mapping					\n");
    fprintf(psfile,"%% THIS FUNCTION USE row_xw AND row_yw which assumed\n");
    fprintf(psfile,"%% to have a coorect value setted up  		\n");
    fprintf(psfile,"{							\n");
    fprintf(psfile,"    dup     %% yobj  yobj 				\n");
    fprintf(psfile,"    %d mul	%% yobj  ..				\n", 
      map.x.y << LOWBITS);
    fprintf(psfile,"    %d div	%% .. / mapLength.y			\n",
      dMap->mapLength.y);
    fprintf(psfile,"    row_xw add 	%% yobj xw 			\n"); 
    fprintf(psfile,"    exch	%% xw yobj  				\n");
    fprintf(psfile,"    %d mul	%% xw .. 				\n", 
      map.y.y << LOWBITS);
    fprintf(psfile,"    %d div	%% .. / mapLength.y			\n",
      dMap->mapLength.y);
    fprintf(psfile,"    row_yw add	%% xw yw			\n"); 
    fprintf(psfile,"} def						\n");

    fprintf(psfile,"/er_window_bar_start 		%% yobj => xw yw	\n");
    fprintf(psfile,"%% yobj - Y coordinates of bar start in the		\n");
    fprintf(psfile,"%% object coordinate system, relatively to current	\n");
    fprintf(psfile,"%% visible region. 					\n");
    fprintf(psfile,"%% xw yw - window coordinates (multiplied by 	\n");
    fprintf(psfile,"%% MAXPIC for PostSript) of this object point under \n");
    fprintf(psfile,"%% current mapping					\n");
    fprintf(psfile,"%% THIS FUNCTION USE row_xw AND row_yw which assumed\n");
    fprintf(psfile,"%% to have a coorect value setted up  		\n");
    fprintf(psfile,"{							\n");
    fprintf(psfile,"    dup     %% yobj  yobj 				\n");
    fprintf(psfile,"    %d mul	%% yobj  ..				\n", 
      map.x.y << LOWBITS);
    fprintf(psfile,"    %d div	%% .. / mapLength.y			\n",
      dMap->mapLength.y);
    fprintf(psfile,"    er_row_xw add 	%% yobj xw 			\n"); 
    fprintf(psfile,"    exch	%% xw yobj  				\n");
    fprintf(psfile,"    %d mul	%% xw .. 				\n", 
      map.y.y << LOWBITS);
    fprintf(psfile,"    %d div	%% .. / mapLength.y			\n",
      dMap->mapLength.y);
    fprintf(psfile,"    er_row_yw add	%% xw yw			\n"); 
    fprintf(psfile,"} def						\n");

    fprintf(psfile,"/tic    %% tic_length v1 u1 x0 y0 =>		\n");
    fprintf(psfile,"%% draw an axes ticmark				\n");
    fprintf(psfile,"%% tic_length - tic length				\n");
    fprintf(psfile,"%% v1, u1 - tic dirction vector 			\n");
    fprintf(psfile,"%% x0, y0 - tic start point 			\n");
    fprintf(psfile,"{							\n");
    fprintf(psfile,"    newpath 					\n");
    fprintf(psfile,"    moveto						\n");
    fprintf(psfile,"    3 2 roll  					\n");
    fprintf(psfile,"    set_len						\n");
    fprintf(psfile,"    rlineto						\n");
    fprintf(psfile,"    stroke						\n");
    fprintf(psfile,"} def						\n");

    fprintf(psfile,"/x_tic_dir  {%d %d} def				\n",
      map.y.x, -map.x.x);
    fprintf(psfile,"%% tic direction (vector) for X axis tics		\n");
    fprintf(psfile,"/y_tic_dir  {%d %d} def				\n",
      -map.y.y, map.x.y );
    fprintf(psfile,"%% tic direction (vector) for X axis tics		\n");

    fprintf(psfile,"/draw_x_tics %% tic_len bar_No ... tic_len bar_No 	\n");
    fprintf(psfile,"		 %% No_of_tics =>			\n");
    fprintf(psfile,"%% draw all tics for X axis				\n");
    fprintf(psfile,"{							\n");
    fprintf(psfile,"    { do_x_tic } repeat				\n");
    fprintf(psfile,"} def						\n");

    fprintf(psfile,"/draw_y_tics %% tic_len bar_No ... tic_len bar_No 	\n");
    fprintf(psfile,"		 %% No_of_tics =>			\n");
    fprintf(psfile,"%% draw all tics for Y axis				\n");
    fprintf(psfile,"{							\n");
    fprintf(psfile,"    { do_y_tic }repeat				\n");
    fprintf(psfile,"} def						\n");

    fprintf(psfile,"/do_x_tic	%% tic_len bar_No =>			\n");
    fprintf(psfile,"%% draw one tic for X axis				\n");
    fprintf(psfile,"%% tic_len - length of tic				\n");
    fprintf(psfile,"%% bar_No - number of bar (relatevely to visible	\n");
    fprintf(psfile,"%% area origin)   tic is attached to  		\n");
    fprintf(psfile,"%% (to front edge)    				\n");
    fprintf(psfile,"{							\n");
    fprintf(psfile,"    dup -1 ne {2 mul} {pop 1} ifelse		\n"); 
    fprintf(psfile,"    x_bin_cell_start_array exch get			\n");
    fprintf(psfile,"    window_row_start				\n");
    fprintf(psfile,"    /row_yw exch def				\n");
    fprintf(psfile,"	/row_xw exch def				\n");
    fprintf(psfile,"    0 window_bar_start				\n");
    fprintf(psfile,"    x_tic_dir					\n");
    fprintf(psfile,"    4 2 roll					\n");
    fprintf(psfile,"    tic						\n");
    fprintf(psfile,"} def						\n");

    fprintf(psfile,"/do_y_tic	%% tic_len bar_No =>			\n");
    fprintf(psfile,"%% draw one tic for Y axis				\n");
    fprintf(psfile,"%% tic_len - length of tic				\n");
    fprintf(psfile,"%% bar_No - number of bar (relatevely to visible	\n");
    fprintf(psfile,"%% area origin)   tic is attached to  		\n");
    fprintf(psfile,"%% (to front edge)    				\n");
    fprintf(psfile,"{							\n");
    fprintf(psfile,"    dup -1 ne {2 mul} {pop 1} ifelse		\n"); 
    fprintf(psfile,"    0						\n");
    fprintf(psfile,"    window_row_start				\n");
    fprintf(psfile,"    /row_yw exch def				\n");
    fprintf(psfile,"	/row_xw exch def				\n");
    fprintf(psfile,"    y_bin_cell_start_array exch get			\n");
    fprintf(psfile,"    window_bar_start				\n");
    fprintf(psfile,"    y_tic_dir					\n");
    fprintf(psfile,"    4 2 roll					\n");
    fprintf(psfile,"    tic						\n");
    fprintf(psfile,"} def						\n");
    
    fprintf(psfile,"/bar_itself  	%% z clip bar_x bar_y =>        \n");
    fprintf(psfile,"%% Draw a bar					\n");
    fprintf(psfile,"%% bar_x, bar_y - start point in PS coordinates	\n");
    fprintf(psfile,"%% z - height of the bar in windows coordinates	\n");
    fprintf(psfile,"%% (PS coordinates value / MAXPIC)			\n");
    fprintf(psfile,"%% clip - if 1, mark the bar as clipped by shading	\n");
    fprintf(psfile,"%% its top, if 0, regular wireframe			\n");
    fprintf(psfile,"%% Assume, that bar is known in outside		\n");
    fprintf(psfile,"%% environment, i.e. 				\n");
    fprintf(psfile,"%% edges lengths variables is setted up correctly	\n");
    fprintf(psfile,"{							\n");
    fprintf(psfile,"    newpath   					\n");
    fprintf(psfile,"    moveto						\n");
    fprintf(psfile,"    0 eq {regular_bar} {clipped_bar} ifelse		\n");
    fprintf(psfile,"}def						\n");
    
    fprintf(psfile,"/regular_bar   		%% z => 		\n");
    fprintf(psfile,"%% Draw a wireframe bar				\n");
    fprintf(psfile,"%% z - height of the bar in windows coordinates	\n");
    fprintf(psfile,"%% (PS coordinates value / MAXPIC)			\n");
    fprintf(psfile,"%% Assume that bar start point is current point	\n");
    fprintf(psfile,"%% and edges lengths variables is setted up correct \n");
    fprintf(psfile,"%% This fuction fills place for the bar with white 	\n");
    fprintf(psfile,"%% first, to provide hidden line removal		\n");
    fprintf(psfile,"{							\n");
    fprintf(psfile,"x_x 0 ne						\n");
    fprintf(psfile,"x_y 0 ne						\n");
    fprintf(psfile,"or							\n");
    fprintf(psfile,"y_x 0 ne						\n");
    fprintf(psfile,"y_y 0 ne						\n");
    fprintf(psfile,"or							\n");
    fprintf(psfile,"and							\n");
    fprintf(psfile,"    {						\n");
    fprintf(psfile,"    %d mul						\n",
      MAXPIC);
    fprintf(psfile,"    x_x x_y rlineto					\n");
    fprintf(psfile,"    dup 0 exch rlineto				\n");
    fprintf(psfile,"    y_x y_y rlineto	 				\n");
    fprintf(psfile,"    x_x neg x_y neg  rlineto			\n");
    fprintf(psfile,"    dup neg 0 exch rlineto				\n");
    fprintf(psfile,"    closepath					\n");
    fprintf(psfile,"    gsave						\n");
    fprintf(psfile,"    1 setgray					\n");
    fprintf(psfile,"    fill						\n");
    fprintf(psfile,"    grestore					\n");
    fprintf(psfile,"    0 exch rlineto					\n");
    fprintf(psfile,"    currentpoint					\n");
    fprintf(psfile,"    currentpoint					\n");
    fprintf(psfile,"    x_x x_y rlineto					\n");
    fprintf(psfile,"    moveto						\n");
    fprintf(psfile,"    y_x y_y rlineto					\n");
    fprintf(psfile,"    stroke						\n");
    fprintf(psfile,"    moveto						\n");
    fprintf(psfile,"	} {pop} ifelse					\n");
    fprintf(psfile,"}def						\n");

    fprintf(psfile,"/clipped_bar   		%% z => 		\n");  
    fprintf(psfile,"%% Draw a wireframe bar with shaded top		\n");
    fprintf(psfile,"%% z - height of the bar in windows coordinates	\n");
    fprintf(psfile,"%% (PS coordinates value / MAXPIC)			\n");
    fprintf(psfile,"%% Assume that bar start point is current point	\n");
    fprintf(psfile,"%% and edges lengths variables is setted up correct \n");
    fprintf(psfile,"%% This fuction fills place for the nonshaded part  \n");
    fprintf(psfile,"%% of the bar with white 				\n");
    fprintf(psfile,"%% first, to provide hidden line removal		\n");
    fprintf(psfile,"{							\n");
    fprintf(psfile,"x_x 0 ne						\n");
    fprintf(psfile,"x_y 0 ne						\n");
    fprintf(psfile,"or							\n");
    fprintf(psfile,"y_x 0 ne						\n");
    fprintf(psfile,"y_y 0 ne						\n");
    fprintf(psfile,"or							\n");
    fprintf(psfile,"and							\n");
    fprintf(psfile,"    {						\n");
    fprintf(psfile,"    %d mul						\n",
      MAXPIC);
    fprintf(psfile,"    x_x x_y rlineto				        \n");
    fprintf(psfile,"    dup 0 exch rlineto				\n");
    fprintf(psfile,"    x_x neg x_y neg  rlineto			\n");
    fprintf(psfile,"    y_x y_y rlineto	 				\n");
    fprintf(psfile,"    dup neg 0 exch rlineto				\n");
    fprintf(psfile,"    closepath					\n");
    fprintf(psfile,"    gsave						\n");
    fprintf(psfile,"    1 setgray					\n");
    fprintf(psfile,"    fill						\n");
    fprintf(psfile,"    grestore					\n");
    fprintf(psfile,"    0 exch rlineto					\n");
    fprintf(psfile,"    currentpoint					\n");
    fprintf(psfile,"    stroke						\n");
    fprintf(psfile,"    moveto						\n");
    fprintf(psfile,"    gsave						\n");
    fprintf(psfile,"    x_x x_y rlineto					\n");
    fprintf(psfile,"    y_x y_y rlineto	 				\n");
    fprintf(psfile,"    x_x neg x_y neg  rlineto			\n");
    fprintf(psfile,"    closepath					\n");
    fprintf(psfile,"    fill						\n");
    fprintf(psfile,"    grestore					\n");
    fprintf(psfile,"	} {pop} ifelse					\n");
    fprintf(psfile,"}def						\n");

    fprintf(psfile,"/bar_itself_2  	%% z clip bar_x bar_y =>        \n");
    fprintf(psfile,"%% the same as bar_itself, but uses er_x_x 		\n");
    fprintf(psfile,"%% instead x_x, etc as edges lengths		\n");
    fprintf(psfile,"%% Draw a bar					\n");
    fprintf(psfile,"%% bar_x, bar_y - start point in PS coordinates	\n");
    fprintf(psfile,"%% z - height of the bar in windows coordinates	\n");
    fprintf(psfile,"%% (PS coordinates value / MAXPIC)			\n");
    fprintf(psfile,"%% clip - if 1, mark the bar as clipped by shading	\n");
    fprintf(psfile,"%% its top, if 0, regular wireframe			\n");
    fprintf(psfile,"%% Assume, that bar is known in outside		\n");
    fprintf(psfile,"%% environment, i.e. 				\n");
    fprintf(psfile,"%% edges lengths variables is setted up correctly	\n");
    fprintf(psfile,"{							\n");
    fprintf(psfile,"    newpath   					\n");
    fprintf(psfile,"    moveto						\n");
    fprintf(psfile,"    0 eq {regular_bar_2} {clipped_bar_2} ifelse		\n");
    fprintf(psfile,"}def						\n");
    
    fprintf(psfile,"/regular_bar_2   		%% z => 		\n");
    fprintf(psfile,"%% the same as regular_bar, but uses er_x_x 		\n");
    fprintf(psfile,"%% instead x_x, etc as edges lengths		\n");
    fprintf(psfile,"%% Draw a wireframe bar				\n");
    fprintf(psfile,"%% z - height of the bar in windows coordinates	\n");
    fprintf(psfile,"%% (PS coordinates value / MAXPIC)			\n");
    fprintf(psfile,"%% Assume that bar start point is current point	\n");
    fprintf(psfile,"%% and edges lengths variables is setted up correct \n");
    fprintf(psfile,"%% This fuction fills place for the bar with white 	\n");
    fprintf(psfile,"%% first, to provide hidden line removal		\n");
    fprintf(psfile,"{							\n");
    fprintf(psfile,"er_x_x 0 ne						\n");
    fprintf(psfile,"er_x_y 0 ne						\n");
    fprintf(psfile,"or							\n");
    fprintf(psfile,"er_y_x 0 ne						\n");
    fprintf(psfile,"er_y_y 0 ne						\n");
    fprintf(psfile,"or							\n");
    fprintf(psfile,"and							\n");
    fprintf(psfile,"    {						\n");
    fprintf(psfile,"    %d mul						\n",
      MAXPIC);
    fprintf(psfile,"    er_x_x er_x_y rlineto					\n");
    fprintf(psfile,"    dup 0 exch rlineto				\n");
    fprintf(psfile,"    er_y_x er_y_y rlineto	 			\n");
    fprintf(psfile,"    er_x_x neg er_x_y neg  rlineto			\n");
    fprintf(psfile,"    dup neg 0 exch rlineto				\n");
    fprintf(psfile,"    closepath					\n");
    fprintf(psfile,"    gsave						\n");
    fprintf(psfile,"    1 setgray					\n");
    fprintf(psfile,"    fill						\n");
    fprintf(psfile,"    grestore					\n");
    fprintf(psfile,"    0 exch rlineto					\n");
    fprintf(psfile,"    currentpoint					\n");
    fprintf(psfile,"    currentpoint					\n");
    fprintf(psfile,"    er_x_x er_x_y rlineto				\n");
    fprintf(psfile,"    moveto						\n");
    fprintf(psfile,"    er_y_x er_y_y rlineto				\n");
    fprintf(psfile,"    stroke						\n");
    fprintf(psfile,"    moveto						\n");
    fprintf(psfile,"    } {pop} ifelse					\n");
    fprintf(psfile,"}def						\n");

    fprintf(psfile,"/clipped_bar_2  		%% z => 		\n");  
    fprintf(psfile,"%% the same as clipped_bar, but uses er_x_x 		\n");
    fprintf(psfile,"%% instead x_x, etc as edges lengths		\n");
    fprintf(psfile,"%% Draw a wireframe bar with shaded top		\n");
    fprintf(psfile,"%% z - height of the bar in windows coordinates	\n");
    fprintf(psfile,"%% (PS coordinates value / MAXPIC)			\n");
    fprintf(psfile,"%% Assume that bar start point is current point	\n");
    fprintf(psfile,"%% and edges lengths variables is setted up correct \n");
    fprintf(psfile,"%% This fuction fills place for the nonshaded part  \n");
    fprintf(psfile,"%% of the bar with white 				\n");
    fprintf(psfile,"%% first, to provide hidden line removal		\n");
    fprintf(psfile,"{							\n");
    fprintf(psfile,"er_x_x 0 ne						\n");
    fprintf(psfile,"er_x_y 0 ne						\n");
    fprintf(psfile,"or							\n");
    fprintf(psfile,"er_y_x 0 ne						\n");
    fprintf(psfile,"er_y_y 0 ne						\n");
    fprintf(psfile,"or							\n");
    fprintf(psfile,"and							\n");
    fprintf(psfile,"    {						\n");
    fprintf(psfile,"    %d mul						\n",
      MAXPIC);
    fprintf(psfile,"    er_x_x er_x_y rlineto			        \n");
    fprintf(psfile,"    dup 0 exch rlineto				\n");
    fprintf(psfile,"    er_x_x neg er_x_y neg  rlineto			\n");
    fprintf(psfile,"    er_y_x er_y_y rlineto	 			\n");
    fprintf(psfile,"    dup neg 0 exch rlineto				\n");
    fprintf(psfile,"    closepath					\n");
    fprintf(psfile,"    gsave						\n");
    fprintf(psfile,"    1 setgray					\n");
    fprintf(psfile,"    fill						\n");
    fprintf(psfile,"    grestore					\n");
    fprintf(psfile,"    0 exch rlineto					\n");
    fprintf(psfile,"    currentpoint					\n");
    fprintf(psfile,"    stroke						\n");
    fprintf(psfile,"    moveto						\n");
    fprintf(psfile,"    gsave						\n");
    fprintf(psfile,"    er_x_x er_x_y rlineto				\n");
    fprintf(psfile,"    er_y_x er_y_y rlineto	 			\n");
    fprintf(psfile,"    er_x_x neg er_x_y neg  rlineto			\n");
    fprintf(psfile,"    closepath					\n");
    fprintf(psfile,"    fill						\n");
    fprintf(psfile,"    grestore					\n");
    fprintf(psfile,"    } {pop} ifelse					\n");
    fprintf(psfile,"}def						\n");

    fprintf(psfile,"/bar_itself_3  	%% z clip bar_x bar_y =>        \n");
    fprintf(psfile,"%% the same as bar_itself_2, but without 		\n");
    fprintf(psfile,"%% erase of bar area				\n");
    fprintf(psfile,"%% Draw a bar					\n");
    fprintf(psfile,"%% bar_x, bar_y - start point in PS coordinates	\n");
    fprintf(psfile,"%% z - height of the bar in windows coordinates	\n");
    fprintf(psfile,"%% (PS coordinates value / MAXPIC)			\n");
    fprintf(psfile,"%% clip - if 1, mark the bar as clipped by shading	\n");
    fprintf(psfile,"%% its top, if 0, regular wireframe			\n");
    fprintf(psfile,"%% Assume, that bar is known in outside		\n");
    fprintf(psfile,"%% environment, i.e. 				\n");
    fprintf(psfile,"%% edges lengths variables is setted up correctly	\n");
    fprintf(psfile,"{							\n");
    fprintf(psfile,"    newpath   					\n");
    fprintf(psfile,"    moveto						\n");
    fprintf(psfile,"    0 eq {regular_bar_3} {clipped_bar_3} ifelse		\n");
    fprintf(psfile,"}def						\n");
    
    fprintf(psfile,"/regular_bar_3   		%% z => 		\n");
    fprintf(psfile,"%% the same as clipped_bar_2, but without 		\n");
    fprintf(psfile,"%% erase of bar area 				\n");
    fprintf(psfile,"%% z - height of the bar in windows coordinates	\n");
    fprintf(psfile,"%% (PS coordinates value / MAXPIC)			\n");
    fprintf(psfile,"%% Assume that bar start point is current point	\n");
    fprintf(psfile,"%% and edges lengths variables is setted up correct \n");
    fprintf(psfile,"{							\n");
    fprintf(psfile,"er_x_x 0 ne						\n");
    fprintf(psfile,"er_x_y 0 ne						\n");
    fprintf(psfile,"or							\n");
    fprintf(psfile,"er_y_x 0 ne						\n");
    fprintf(psfile,"er_y_y 0 ne						\n");
    fprintf(psfile,"or							\n");
    fprintf(psfile,"and							\n");
    fprintf(psfile,"    {						\n");
    fprintf(psfile,"[%d] 0 setdash					\n",
      MAXPIC);
    fprintf(psfile,"    %d mul						\n",
      MAXPIC);
    fprintf(psfile,"    er_x_x er_x_y rlineto			        \n");
    fprintf(psfile,"    dup 0 exch rlineto				\n");
    fprintf(psfile,"    er_x_x neg er_x_y neg  rlineto			\n");
    fprintf(psfile,"    er_y_x er_y_y rlineto	 			\n");
    fprintf(psfile,"    dup neg 0 exch rlineto				\n");
    fprintf(psfile,"    closepath					\n");
    fprintf(psfile,"    0 exch rlineto					\n");
    fprintf(psfile,"    currentpoint					\n");
    fprintf(psfile,"    stroke						\n");
    fprintf(psfile,"    moveto						\n");
    fprintf(psfile,"    } {pop} ifelse					\n");
    fprintf(psfile,"}def						\n");
    
    fprintf(psfile,"/clipped_bar_3   		%% z => 		\n");
    fprintf(psfile,"%% the same as clipped_bar_2, but without 		\n");
    fprintf(psfile,"%% erase of bar area 				\n");
    fprintf(psfile,"%% z - height of the bar in windows coordinates	\n");
    fprintf(psfile,"%% (PS coordinates value / MAXPIC)			\n");
    fprintf(psfile,"%% Assume that bar start point is current point	\n");
    fprintf(psfile,"%% and edges lengths variables is setted up correct \n");
    fprintf(psfile,"{							\n");
    fprintf(psfile,"er_x_x 0 ne						\n");
    fprintf(psfile,"er_x_y 0 ne						\n");
    fprintf(psfile,"or							\n");
    fprintf(psfile,"er_y_x 0 ne						\n");
    fprintf(psfile,"er_y_y 0 ne						\n");
    fprintf(psfile,"or							\n");
    fprintf(psfile,"and							\n");
    fprintf(psfile,"    {						\n");
    fprintf(psfile,"    dup						\n");
    fprintf(psfile,"    1 ge 	    					\n");
    fprintf(psfile,"    {						\n");
    fprintf(psfile,"    1 sub 						\n");
    fprintf(psfile,"[%d] 0 setdash					\n",
      MAXPIC);
    fprintf(psfile,"    %d mul						\n",
      MAXPIC);
    fprintf(psfile,"    er_x_x er_x_y rlineto			        \n");
    fprintf(psfile,"    dup 0 exch rlineto				\n");
    fprintf(psfile,"    er_x_x neg er_x_y neg  rlineto			\n");
    fprintf(psfile,"    er_y_x er_y_y rlineto	 			\n");
    fprintf(psfile,"    dup neg 0 exch rlineto				\n");
    fprintf(psfile,"    closepath					\n");
    fprintf(psfile,"    0 exch rlineto					\n");
    fprintf(psfile,"    currentpoint					\n");
    fprintf(psfile,"    stroke						\n");
    fprintf(psfile,"    moveto						\n");
    fprintf(psfile,"    er_x_x er_x_y rlineto			        \n");
    fprintf(psfile,"    0 %d rlineto					\n",
      MAXPIC);
    fprintf(psfile,"    er_x_x neg er_x_y neg  rlineto			\n");
    fprintf(psfile,"    currentpoint					\n");
    fprintf(psfile,"    er_y_x er_y_y rlineto	 			\n");
    fprintf(psfile,"    0 %d rlineto					\n",
      - MAXPIC);
    fprintf(psfile,"    closepath					\n");
    fprintf(psfile,"    fill						\n");
    fprintf(psfile,"    moveto						\n");
    fprintf(psfile,"    }						\n");
    fprintf(psfile,"    {						\n");
    fprintf(psfile,"    %d mul						\n",
      MAXPIC);
    fprintf(psfile,"    dup 0 exch rmoveto				\n");
    fprintf(psfile,"    currentpoint					\n");
    fprintf(psfile,"    er_x_x er_x_y rlineto				\n");
    fprintf(psfile,"    moveto						\n");
    fprintf(psfile,"    er_y_x er_y_y rlineto	 			\n");
    fprintf(psfile,"    currentpoint					\n");
    fprintf(psfile,"    stroke						\n");
    fprintf(psfile,"[%d] 0 setdash					\n",
      MAXPIC);
    fprintf(psfile,"    moveto						\n");
    fprintf(psfile,"    dup neg 0 exch rlineto				\n");
    fprintf(psfile,"    er_y_x neg er_y_y neg rlineto			\n");
    fprintf(psfile,"    currentpoint					\n");
    fprintf(psfile,"    er_x_x er_x_y rlineto				\n");
    fprintf(psfile,"    2 index 0 exch rlineto 				\n");
    fprintf(psfile,"    moveto						\n");
    fprintf(psfile,"    0 exch rlineto 					\n");
    fprintf(psfile,"    currentpoint					\n");
    fprintf(psfile,"    stroke						\n");
    fprintf(psfile,"    moveto						\n");
    fprintf(psfile,"    } ifelse					\n");
    fprintf(psfile,"    } {pop} ifelse					\n");
    fprintf(psfile,"}def						\n");

    fprintf(psfile,"%% Start of main PostScript programm, 		\n");
    fprintf(psfile,"%% initial settings					\n");
    fprintf(psfile,"1 setlinecap	        			\n");
    fprintf(psfile,"1 setlinejoin					\n");
    fprintf(psfile,"0 setlinewidth					\n");
    fprintf(psfile,"/inch {72 mul} def					\n");
    fprintf(psfile,"%% Make mirror transformation for Y to have the same\n");
    fprintf(psfile,"%% orintation of the coodinate system as in the 	\n");
    fprintf(psfile,"%% widget window (X11 style). With the origin in	\n");
    fprintf(psfile,"%% left upper corner and Y axis directed from top to\n");
    fprintf(psfile,"%% bottom.  It looks like PostScript has no 	\n");
    fprintf(psfile,"%% predefined transformations of that kind, so some	\n");
    fprintf(psfile,"%% direct work with translation matrix is needed.	\n");
    fprintf(psfile,"%% Because of this trick it is important to 	\n");
    fprintf(psfile,"%% transform font translation matrix in a similair  \n");
    fprintf(psfile,"%% way later othervise we'll get mirrored font 	\n");
    fprintf(psfile,"%% printing						\n");
    fprintf(psfile,"0 %d %% heigth of the widget window			\n",
       w->core.height); 
    fprintf(psfile,"translate 						\n");
    fprintf(psfile,"matrix currentmatrix 				\n");
    fprintf(psfile,"dup dup 2 get 					\n");
    fprintf(psfile,"neg 						\n");
    fprintf(psfile,"2 exch put 						\n");
    fprintf(psfile,"dup dup 3 get 					\n");
    fprintf(psfile,"neg 						\n");
    fprintf(psfile,"3 exch put 						\n");
    fprintf(psfile,"setmatrix 						\n");
    fprintf(psfile,"%% set up new scale so one widget window pixel 	\n");
    fprintf(psfile,"%% corresponds MAXPIC == %d new PostScript user 	\n",
       MAXPIC);
    fprintf(psfile,"%% units to avoid extra aliasing on higth resolution\n");
    fprintf(psfile,"%% PostScript devices				\n");
    fprintf(psfile,"1 %d div dup  					\n",
       MAXPIC);
    fprintf(psfile,"scale 						\n");
    fprintf(psfile,"%% Initialise PS variables				\n");
    fprintf(psfile,"%% Define bar edges vectors,  		 	\n");
    fprintf(psfile,"%% actual values will be setted later. 		\n");
    fprintf(psfile,"/x_x 0 def						\n");
    fprintf(psfile,"/x_y 0 def						\n");
    fprintf(psfile,"/y_x 0 def						\n");
    fprintf(psfile,"/y_y 0 def						\n");
    

/* draw Z tics, just copy ones built for screen window  		*/
    if (w->hist2D.currentPicture != NULL)
        if (w->hist2D.currentPicture->zTics!= NULL &&
          w->hist2D.currentPicture->nZTics != 0)
        {
           fprintf(psfile,"%% draw Z axis ticmarks:			\n");
            fprintf(psfile,"newpath					\n");
            for (i=w->hist2D.currentPicture->nZTics-1; i>=0; i--)
            {
                fprintf(psfile,"%d %d moveto 				\n",
                  w->hist2D.currentPicture->zTics[i].x1 << LOWBITS,
                  w->hist2D.currentPicture->zTics[i].y1 << LOWBITS);
                fprintf(psfile,"%d %d lineto 				\n",
                  w->hist2D.currentPicture->zTics[i].x2 << LOWBITS,
                  w->hist2D.currentPicture->zTics[i].y2 << LOWBITS);
            }
            fprintf(psfile,"stroke				\n");
        }
    
/* draw back planes, just copy ones built for screen window  */
    if (w->hist2D.currentPicture != NULL)
        if (w->hist2D.currentPicture->backPlanes!= NULL &&
          w->hist2D.currentPicture->nBackPlanes != 0)
        {
           fprintf(psfile,"%% draw backplanes with dotted lines:	\n");
            fprintf(psfile,"gsave					\n");
            fprintf(psfile,"[%d] 0 setdash				\n",
              MAXPIC);
            fprintf(psfile,"newpath					\n");
            for (i=w->hist2D.currentPicture->nBackPlanes-1; i>=0; i--)
            {
                fprintf(psfile,"%d %d moveto 				\n",
                  w->hist2D.currentPicture->backPlanes[i].x1 << LOWBITS,
                  w->hist2D.currentPicture->backPlanes[i].y1 << LOWBITS);
                fprintf(psfile,"%d %d lineto 				\n",
                  w->hist2D.currentPicture->backPlanes[i].x2 << LOWBITS,
                  w->hist2D.currentPicture->backPlanes[i].y2 << LOWBITS);
            }
            fprintf(psfile,"stroke					\n");
            fprintf(psfile,"grestore					\n");
        }

/* draw Z axis								*/
   fprintf(psfile,"%% draw Z axis:					\n");
    fprintf(psfile,"newpath						\n");
    fprintf(psfile," %d %d moveto					\n",
      (dMap->vertex.x - map.x.y)<< LOWBITS, 
      (dMap->vertex.y  - map.y.y)<< LOWBITS);
    fprintf(psfile," %d %d rlineto					\n",
      0, dMap->zFactor<< LOWBITS);
    fprintf(psfile,"stroke						\n");

/* it's simpler to do this all times, the dandger is possible
   aliasing effects on front base edges (dubbling line width), 
   but they might appear in some cases anyway.
** #if !(MAIN_BIN_WIDTH - MAXPIC) 
**    if (zData == NULL) 
** #endif
*/
    {
    /* draw X & Y axes	(base)					*/
	fprintf(psfile,"newpath			\n");
	fprintf(psfile," %d %d moveto		\n",
	  (dMap->vertex.x - map.x.y)<< LOWBITS, 
	  (dMap->vertex.y  - map.y.y)<< LOWBITS);
	fprintf(psfile," %d %d rlineto		\n",
	  -map.x.x << LOWBITS, -map.y.x<< LOWBITS);
	fprintf(psfile," %d %d rlineto		\n",
	  map.x.y<< LOWBITS,  map.y.y<< LOWBITS);
	fprintf(psfile," %d %d rlineto		\n",
	  map.x.x<< LOWBITS, map.y.x<< LOWBITS);
	fprintf(psfile," %d %d rlineto		\n",
	  -map.x.y<< LOWBITS,  -map.y.y<< LOWBITS);
	fprintf(psfile,"stroke			\n");
    }
    
/*  put bar drawing data (and functions calls) 			**
** to the PosScript file					*/
/*  prepare data from the widget				*/
    if (w->hist2D.aHist != NULL)
        makeAHistPSImage(w, psfile);
    else
    {
 	errorBarsOn = w->hist2D.errorBarsOn && (w->hist2D.topErrors!=NULL ||
	  w->hist2D.bottomErrors!=NULL);
	zData = rescaleZ(w->hist2D.bins,w->hist2D.sourceHistogram->nYBins, 
	  w->hist2D.discrMap, w->hist2D.xyVisibleRange, w->hist2D.zVisibleStart,
	  w->hist2D.zVisibleEnd, w->hist2D.zLogScaling, 1);
	if (w->hist2D.topErrors!=NULL)
            zTopErrors = rescaleZ(w->hist2D.topErrors,
            w->hist2D.sourceHistogram->nYBins, w->hist2D.discrMap, 
            w->hist2D.xyVisibleRange, w->hist2D.zVisibleStart,
            w->hist2D.zVisibleEnd, w->hist2D.zLogScaling, 1);
	else if (errorBarsOn)
            zTopErrors = zData;
	else 
            zTopErrors = NULL;    
	if (w->hist2D.bottomErrors!=NULL)
            zBottomErrors = rescaleZ(w->hist2D.bottomErrors,
            w->hist2D.sourceHistogram->nYBins, w->hist2D.discrMap, 
            w->hist2D.xyVisibleRange, w->hist2D.zVisibleStart,
            w->hist2D.zVisibleEnd, w->hist2D.zLogScaling, 1);
	else if (errorBarsOn)
            zBottomErrors = zData;
	else 
            zBottomErrors = NULL;    
	makeBinPosDescr (w->hist2D.xyVisibleRange, dMap, MAIN_BIN_WIDTH, 
	  &binPositions);  
	makeBinPosDescr (w->hist2D.xyVisibleRange, dMap, MAXPIC, 
	  &binCellPositions);  
	if (errorBinWidth > MAIN_BIN_WIDTH) errorBinWidth = MAIN_BIN_WIDTH;   
	makeBinPosDescr (w->hist2D.xyVisibleRange, dMap, errorBinWidth, 
	  &er_binPositions); 

	fprintf(psfile,"/set_bin_row_prms		%%  => 		\n");
	fprintf(psfile,"%% set variables needed to draw a bin:	 	\n");
	fprintf(psfile,"%% X-edge vector for bin and error bars		\n");
	fprintf(psfile,"%% X-edge vector are setted once for row of bins\n");
	fprintf(psfile,"%% Uses variables: bin_start_array,  		\n");
	fprintf(psfile,"%% er_bin_start_array, j 			\n");
	fprintf(psfile,"%% Sets variables: x_x, x_y, er_x_x, er_x_y, 	\n");
	fprintf(psfile,"%% row_xw, row_yw, er_row_xw			\n");
	fprintf(psfile,"{						\n");    
	fprintf(psfile,"	x_bin_start_array j get 		\n");
	fprintf(psfile,"	window_row_start			\n");
	fprintf(psfile,"	x_bin_start_array j 1 add get		\n");
	fprintf(psfile,"	window_row_start			\n");
	fprintf(psfile,"	2 index					\n");
	fprintf(psfile,"	sub					\n");
	fprintf(psfile,"	/x_y exch def				\n");
	fprintf(psfile,"	2 index					\n");
	fprintf(psfile,"	sub					\n");
	fprintf(psfile,"	/x_x exch def				\n");
	fprintf(psfile,"	/row_yw exch def			\n");
	fprintf(psfile,"	/row_xw exch def			\n");
	if (errorBarsOn)
	{
	    fprintf(psfile,"	er_x_bin_start_array j get		\n");
	    fprintf(psfile,"	window_row_start			\n");
	    fprintf(psfile,"	er_x_bin_start_array j 1 add get	\n");
	    fprintf(psfile,"	window_row_start			\n");
	    fprintf(psfile,"	2 index					\n");
	    fprintf(psfile,"	sub					\n");
	    fprintf(psfile,"	/er_x_y exch def			\n");
	    fprintf(psfile,"	2 index					\n");
	    fprintf(psfile,"	sub					\n");
	    fprintf(psfile,"	/er_x_x exch def			\n");
	    fprintf(psfile,"	/er_row_yw exch def			\n");
	    fprintf(psfile,"	/er_row_xw exch def			\n");
	}
	fprintf(psfile,"	/j j 2 add def				\n");
	fprintf(psfile,"} def						\n");
 						   
	fprintf(psfile,"/set_bin_prms		%%  => 			\n");
	fprintf(psfile,"%% set variables needed to draw a bin:	 	\n");
	fprintf(psfile,"%% Y-edge vector for bin and error bars		\n");
	fprintf(psfile,"%% and shift for the main bar start point to the\n");
	fprintf(psfile,"%% error bar start point.			\n"); 
	fprintf(psfile,"%% X-edge vector are setted once for row of bins\n");
	fprintf(psfile,"%% Uses variables: bin_start_array,  		\n");
	fprintf(psfile,"%% er_bin_start_array, i, row_xw, row_yw, er_row_xw \n");
	fprintf(psfile,"%% er_row_yw.					\n");
	fprintf(psfile,"%% Sets variables: y_x, y_y, er_y_x, er_y_y, csh_x, \n");
	fprintf(psfile,"%% csh_y					\n");
	fprintf(psfile,"{						\n");    
	fprintf(psfile,"	y_bin_start_array i get 		\n");
	fprintf(psfile,"	window_bar_start			\n");
	fprintf(psfile,"	y_bin_start_array i 1 add get		\n");
	fprintf(psfile,"	window_bar_start			\n");
	fprintf(psfile,"	2 index					\n");
	fprintf(psfile,"	sub					\n");
	fprintf(psfile,"	/y_y exch def				\n");
	fprintf(psfile,"	2 index					\n");
	fprintf(psfile,"	sub					\n");
	fprintf(psfile,"	/y_x exch def				\n");
	if (errorBarsOn)
	{
	    fprintf(psfile,"	er_y_bin_start_array i get		\n");
	    fprintf(psfile,"	er_window_bar_start			\n");
	    fprintf(psfile,"	er_y_bin_start_array i 1 add get	\n");
	    fprintf(psfile,"	er_window_bar_start			\n");
	    fprintf(psfile,"	2 index					\n");
	    fprintf(psfile,"	sub					\n");
	    fprintf(psfile,"	/er_y_y exch def			\n");
	    fprintf(psfile,"	2 index					\n");
	    fprintf(psfile,"	sub 					\n");
	    fprintf(psfile,"	/er_y_x exch def			\n");

	    fprintf(psfile,"	2 index	sub				\n");
	    fprintf(psfile,"	/csh_y exch def				\n");
	    fprintf(psfile,"	2 index sub				\n");
	    fprintf(psfile,"	/csh_x exch def				\n");
	}
	fprintf(psfile,"	/i i 2 add def				\n");
	fprintf(psfile,"} def						\n");

	if (zData != NULL)
	{
	    fprintf(psfile,"/bar %% [z clip] [z clip] z clip bar_x bar_y => \n");
	    fprintf(psfile,"%% Draw a bar whith error marks, if histogram	\n");
	    fprintf(psfile,"%% errors was defined				\n");
	    fprintf(psfile,"%% Assume bar edges (x_x x_y y_x y_y) and shift	\n");
	    fprintf(psfile,"%% to center (clsh_x clsh_y) variables are stted\n");
	    fprintf(psfile,"%% up propely					\n");
	    fprintf(psfile,"{						\n");
	    fprintf(psfile,"    bar_itself   				\n");
	    if (errorBarsOn)
	    {
        	fprintf(psfile,"csh_x csh_y rmoveto				\n");
        	if (zTopErrors != NULL) 					
        	{
		    fprintf(psfile,"currentpoint				\n");
        	    fprintf(psfile,"gsave					\n");
        	    fprintf(psfile,"bar_itself_2				\n");
        	    fprintf(psfile,"grestore				\n");
        	}
        	if (zBottomErrors != NULL) 					
        	{
		    fprintf(psfile,"currentpoint				\n");
        	    fprintf(psfile,"gsave					\n");
        	    fprintf(psfile,"bar_itself_3				\n");
        	    fprintf(psfile,"grestore				\n");
        	}
            }
	    fprintf(psfile,"} def				 		\n");
	    fprintf(psfile,"%% parameters to calculate bin positions  	\n");
	    for (i = 2 * binPositions.nXPoints - 2, xp =0 ; i >=0; i-- )
		xp -= (binPositions.xSteps)[i];
	    fprintf(psfile,"[					  	\n");
	    for (i = 2 * binPositions.nXPoints - 2; i >0; )
	    {
		xp += (binPositions.xSteps)[i];
		i--;
		fprintf(psfile,"%d %d					\n",
		  xp + (binPositions.xSteps)[i], xp);
		xp += (binPositions.xSteps)[i];
		i--;
	    }
	    fprintf(psfile,"]						\n");
	    fprintf(psfile,"/x_bin_start_array exch def			\n");
	    for (i = 2 * binPositions.nYPoints - 2, yp =0 ; i >=0; i-- )
		yp -= (binPositions.ySteps)[i];
	    fprintf(psfile,"[					  	\n");
	    for (i = 2 * binPositions.nYPoints - 2 ; i >0; )
	    {
		yp += (binPositions.ySteps)[i];
		i--;
		fprintf(psfile,"%d %d					\n",
		  yp + (binPositions.ySteps)[i], yp);
		yp += (binPositions.ySteps)[i];
		i--;
	    }
	    fprintf(psfile,"]						\n");
	    fprintf(psfile,"/y_bin_start_array exch def			\n");
	    fprintf(psfile,"%% parameters to calculate bin positions  	\n");
	    for (i = 2 * binCellPositions.nXPoints - 2, xp =0 ; i >=0; i-- )
		xp -= (binCellPositions.xSteps)[i];
	    fprintf(psfile,"[					  	\n");
	    for (i = 2 * binCellPositions.nXPoints - 2; i >0; )
	    {
		xp += (binCellPositions.xSteps)[i];
		i--;
		fprintf(psfile,"%d %d					\n",
		  xp + (binCellPositions.xSteps)[i], xp);
		xp += (binCellPositions.xSteps)[i];
		i--;
	    }
	    fprintf(psfile,"]						\n");
	    fprintf(psfile,"/x_bin_cell_start_array exch def			\n");
	    for (i = 2 * binCellPositions.nYPoints - 2, yp =0 ; i >=0; i-- )
		yp -= (binCellPositions.ySteps)[i];
	    fprintf(psfile,"[					  	\n");
	    for (i = 2 * binCellPositions.nYPoints - 2 ; i >0; )
	    {
		yp += (binCellPositions.ySteps)[i];
		i--;
		fprintf(psfile,"%d %d					\n",
		  yp + (binCellPositions.ySteps)[i], yp);
		yp += (binCellPositions.ySteps)[i];
		i--;
	    }
	    fprintf(psfile,"]						\n");
	    fprintf(psfile,"/y_bin_cell_start_array exch def			\n");
	    if (errorBarsOn )
            {
		for (i = 2 * er_binPositions.nXPoints - 2, xp =0 ; i >=0; i-- )
		    xp -= (er_binPositions.xSteps)[i];
		fprintf(psfile,"[					  	\n");
		for (i = 2 * er_binPositions.nXPoints - 2 ; i >0; )
		{
		    xp += (er_binPositions.xSteps)[i];
		    i--;
		    fprintf(psfile,"%d %d					\n",
		      xp + (er_binPositions.xSteps)[i], xp);
		    xp += (er_binPositions.xSteps)[i];
		    i--;
		}
		fprintf(psfile,"]						\n");
		fprintf(psfile,"/er_x_bin_start_array exch def			\n");
		for (i = 2 * er_binPositions.nYPoints - 2, yp =0 ; i >=0; i-- )
		    yp -= (er_binPositions.ySteps)[i];
		fprintf(psfile,"[					  	\n");
		for (i = 2 * er_binPositions.nYPoints - 2 ; i >0; )
		{
		    yp += (er_binPositions.ySteps)[i];
		    i--;
		    fprintf(psfile,"%d %d					\n",
		      yp + (er_binPositions.ySteps)[i], yp);
		    yp += (er_binPositions.ySteps)[i];
		    i--;
		}
		fprintf(psfile,"]						\n");
		fprintf(psfile,"/er_y_bin_start_array exch def			\n");
	    }
	    fprintf(psfile,"%% init i,j for bar drawing			\n");
	    fprintf(psfile,"/j 0 def					\n");	
            for ( i = binPositions.nXPoints-2; i >=0; i--)
            {
		fprintf(psfile,"/i 0 def					\n");
		fprintf(psfile,"%% set up parameters to draw a row   	\n");
		fprintf(psfile,"set_bin_row_prms			  	\n");
		for ( j = 0; j < binPositions.nYPoints-1; j++)
	            putData(psfile, zData, zTopErrors, zBottomErrors, 
                      binPositions.nYPoints-1, i, j);
		fprintf(psfile,"%d { 					\n",
		  binPositions.nYPoints-1);
		fprintf(psfile,"    set_bin_prms				\n");
		fprintf(psfile,"    bar					\n");
		fprintf(psfile,"} repeat 					\n");
	    }
            MYFREE(zData);
            MYFREE(zTopErrors);
            MYFREE(zBottomErrors);
	    MYFREE(binPositions.xSteps);
	    MYFREE(binPositions.ySteps);
	    MYFREE(binCellPositions.xSteps);
	    MYFREE(binCellPositions.ySteps);
	    MYFREE(er_binPositions.xSteps);
	    MYFREE(er_binPositions.ySteps);
	}
    }
/* draw X & Y tics 							*/
    if (w->hist2D.aHist == NULL && w->hist2D.binEdgeLabeling)
    {
    /* binned labeling 							*/
	fprintf(psfile,"%% parameters for X axis tic drawing:		\n");
	if (w->hist2D.currentPicture != NULL)
            if (w->hist2D.currentPicture->xTicStruct != NULL &&
              w->hist2D.currentPicture->nXTics !=0)
        	  for (i=0; i<w->hist2D.currentPicture->nXTics; i++)
    /* put a tic parameters to PostScript file 				*/             
                      fprintf(psfile,"%d %d					\n",
                	w->hist2D.currentPicture->xTicStruct[i].length << LOWBITS,
                	binCellPositions.nXPoints - 2 - 
                	w->hist2D.currentPicture->xTicStruct[i].bar);
	fprintf(psfile,"%% number of X tics:				\n");
	fprintf(psfile,"%d	\n",w->hist2D.currentPicture->nXTics);                  
	fprintf(psfile,"draw_x_tics	\n");
	if (w->hist2D.currentPicture != NULL)
	fprintf(psfile,"%% parameters for Y axis tic drawing:		\n");
            if (w->hist2D.currentPicture->yTicStruct != NULL &&
              w->hist2D.currentPicture->nYTics !=0)
        	  for (i=0; i<w->hist2D.currentPicture->nYTics; i++)
                      fprintf(psfile,"%d %d\n",
                	w->hist2D.currentPicture->yTicStruct[i].length << LOWBITS,
                	binCellPositions.nYPoints - 2 - 
                	w->hist2D.currentPicture->yTicStruct[i].bar);
	fprintf(psfile,"%% number of Y tics:				\n");
	fprintf(psfile,"%d	\n",w->hist2D.currentPicture->nYTics);
	fprintf(psfile,"draw_y_tics	\n");
    }
    else
    {
/* unbinned labeling, use new axis handling functions			*/

/* set standard scale for makePSTics fuctions (1 pixel == 1 PS unit) 	*/
        fprintf(psfile,"gsave %d dup scale  			\n",
          MAXPIC);
        makePSTics (w->hist2D.currentPicture->xSegment,  psfile );
        makePSTics (w->hist2D.currentPicture->ySegment,  psfile );
        fprintf(psfile,"grestore 				\n");
    }         
    drawPsArrows ( w, psfile );
    drawLabels ( w, psfile);
}  

static void drawLabels (Hist2DWidget w, FILE * psfile)
{
    objectsToDraw *oD= w->hist2D.currentPicture;
    labelsToDraw *tmp;
    int j;
    XFontStruct *fs = w->hist2D.fs;
    
    fprintf(psfile,"/Times-Roman findfont			\n");
    fprintf(psfile,"%d						\n", 
	(w->hist2D.fs->ascent + w->hist2D.fs->descent)<<LOWBITS);
    fprintf(psfile,"%d sub 					\n",
      FONT_DELTA << LOWBITS);
    fprintf(psfile,"scalefont 					\n");
    fprintf(psfile,"%% Make Y mirror				\n");
    fprintf(psfile,"matrix dup	 				\n");			
    fprintf(psfile,"0 1 put 					\n");
    fprintf(psfile,"dup 					\n");
    fprintf(psfile,"3 -1 put 					\n");					
    fprintf(psfile,"makefont 					\n");				
    fprintf(psfile,"setfont 					\n");				

    fprintf(psfile,"/print_center 	%% string x y => 	\n");
    fprintf(psfile,"{    					\n");
    fprintf(psfile,"    %d sub					\n",
      FONT_DELTA << LOWBITS);
    fprintf(psfile,"    exch    				\n");
    fprintf(psfile,"    2 index    				\n");
    fprintf(psfile,"    stringwidth    				\n");
    fprintf(psfile,"    pop    					\n");
    fprintf(psfile,"    2 div    				\n");
    fprintf(psfile,"    sub    					\n");
    fprintf(psfile,"    exch    				\n");
    fprintf(psfile,"    moveto    				\n");
    fprintf(psfile,"    show    				\n");
    fprintf(psfile,"}def    					\n");

    fprintf(psfile,"/print_right 	%% string x y => 	\n");
    fprintf(psfile,"{    					\n");
    fprintf(psfile,"    %d sub					\n",
      FONT_DELTA << (LOWBITS - 1));
    fprintf(psfile,"    exch    				\n");
    fprintf(psfile,"    2 index    				\n");
    fprintf(psfile,"    stringwidth    				\n");
    fprintf(psfile,"    pop    					\n");
    fprintf(psfile,"    sub    					\n");
    fprintf(psfile,"    exch    				\n");
    fprintf(psfile,"    moveto    				\n");
    fprintf(psfile,"    show    				\n");
    fprintf(psfile,"}def    					\n");


    fprintf(psfile,"/print_left 	%% string x y => 	\n");
    fprintf(psfile,"{    					\n");
    fprintf(psfile,"    %d sub					\n",
      FONT_DELTA << (LOWBITS - 1));
    fprintf(psfile,"    moveto    				\n");
    fprintf(psfile,"    show    				\n");
    fprintf(psfile,"}def    					\n");

    fprintf(psfile,"/max 		%% x y => max{x,y}	\n");
    fprintf(psfile,"{    					\n");
    fprintf(psfile,"    2 copy    				\n");
    fprintf(psfile,"    lt {exch} if    			\n");
    fprintf(psfile,"    pop    					\n");
    fprintf(psfile,"} def        				\n");

    fprintf(psfile,"/str_ex         %% string oldWidth max => max'\n");
    fprintf(psfile,"{    					\n");
    fprintf(psfile,"    3 2 roll    				\n");
    fprintf(psfile,"    stringwidth    				\n");
    fprintf(psfile,"    pop		%% oldWidth max newWidth\n");
    fprintf(psfile,"    3 2 roll    				\n");
    fprintf(psfile,"    %d mul					\n",
      MAXPIC);
    fprintf(psfile,"    div    					\n");
    fprintf(psfile,"    max    					\n");
    fprintf(psfile,"}def   					\n");
/* XWidth field in labelsToDraw sructure is NOT supported for all
   kind of labels by other functions. Fix it here  	*/            
    for(tmp=oD->xLabels;tmp!=NULL;tmp=tmp->next)
        tmp->XWidth = XTextWidth(fs,tmp->label,tmp->length);
    for(tmp=oD->yLabels;tmp!=NULL;tmp=tmp->next)
        tmp->XWidth = XTextWidth(fs,tmp->label,tmp->length);
    for(tmp=oD->zLabels;tmp!=NULL;tmp=tmp->next)
        tmp->XWidth = XTextWidth(fs,tmp->label,tmp->length);
 
    if ((tmp=oD->xName)!=NULL)
        tmp->XWidth = XTextWidth(fs,tmp->label,tmp->length);
    if ((tmp=oD->yName)!=NULL)
        tmp->XWidth = XTextWidth(fs,tmp->label,tmp->length);
    if ((tmp=oD->zName)!=NULL)
        tmp->XWidth = XTextWidth(fs,tmp->label,tmp->length);

    j=0;
    
    if (w->hist2D.aHist == NULL && w->hist2D.binEdgeLabeling)
    {
/* binned labeling							*/    
	for(tmp=oD->xLabels;tmp!=NULL;tmp=tmp->next,j++)
            fprintf(psfile,"(%s) %d\n",tmp->label,tmp->XWidth);
	for(tmp=oD->yLabels;tmp!=NULL;tmp=tmp->next,j++)
            fprintf(psfile,"(%s) %d\n",tmp->label,tmp->XWidth);
    }
    
    for(tmp=oD->zLabels;tmp!=NULL;tmp=tmp->next,j++)
        fprintf(psfile,"(%s) %d\n",tmp->label,tmp->XWidth);

    if ((tmp=oD->xName)!=NULL)
    {
        fprintf(psfile,"(%s) %d\n",tmp->label,tmp->XWidth);
        j++;
    }
    if ((tmp=oD->yName)!=NULL)
    {
        fprintf(psfile,"(%s) %d\n",tmp->label,tmp->XWidth);
        j++;
    }
    if ((tmp=oD->zName)!=NULL)
    {
        fprintf(psfile,"(%s) %d\n",tmp->label,tmp->XWidth);
        j++;
    }
 

    if (j!=0)
    {
        fprintf(psfile,"1 %d {str_ex} repeat  			\n",j);
	fprintf(psfile,"dup 1 ge {				\n");
	fprintf(psfile,"    1 exch div				\n");
	fprintf(psfile,"    currentfont				\n");
	fprintf(psfile,"    matrix dup				\n");
	fprintf(psfile,"    0					\n");
	fprintf(psfile,"    5 4 roll				\n");
	fprintf(psfile,"    put					\n");
	fprintf(psfile,"    dup					\n");
	fprintf(psfile,"    3 1 put				\n");
	fprintf(psfile,"    makefont				\n");
	fprintf(psfile,"    setfont				\n");
	fprintf(psfile,"} if					\n");

	if (w->hist2D.aHist == NULL && w->hist2D.binEdgeLabeling)
	{
/* binned labeling							*/    
            for(tmp=oD->xLabels;tmp!=NULL;tmp=tmp->next,j++)
        	if (tmp->sidePlacing)
                    fprintf(psfile,"(%s) %d %d print_left 		\n",
                      tmp->label,tmp->x << LOWBITS,tmp->y << LOWBITS);
        	else 
                    fprintf(psfile,"(%s) %d %d print_center		\n",
                      tmp->label, (2 * tmp->x + tmp->XWidth) << (LOWBITS-1), 
                      tmp->y << LOWBITS);
            for(tmp=oD->yLabels;tmp!=NULL;tmp=tmp->next,j++)
        	if (tmp->sidePlacing)
                    fprintf(psfile,"(%s) %d %d print_right 		\n",
                      tmp->label, (tmp->x + tmp->XWidth - 1) << LOWBITS, 
                      tmp->y << LOWBITS);
        	else 
                    fprintf(psfile,"(%s) %d %d print_center		\n",
                      tmp->label, (2 * tmp->x + tmp->XWidth) << (LOWBITS-1),
                      tmp->y << LOWBITS );
	}                  
        for(tmp=oD->zLabels;tmp!=NULL;tmp=tmp->next,j++)
                fprintf(psfile,"(%s) %d %d print_left 		\n",
                  tmp->label, (tmp->x ) << LOWBITS, 
                  tmp->y << LOWBITS);
        if ((tmp=oD->xName)!=NULL)
            fprintf(psfile,"(%s) %d %d print_center		\n",
              tmp->label, (2 * tmp->x + tmp->XWidth) << (LOWBITS-1), 
                  tmp->y << LOWBITS);
        if ((tmp=oD->yName)!=NULL)
            fprintf(psfile,"(%s) %d %d print_center		\n",
              tmp->label, (2 * tmp->x + tmp->XWidth) << (LOWBITS-1), 
                  tmp->y << LOWBITS);
        if ((tmp=oD->zName)!=NULL)
            fprintf(psfile,"(%s) %d %d print_center		\n",
              tmp->label, (2 * tmp->x + tmp->XWidth) << (LOWBITS-1), 
                  tmp->y << LOWBITS);
    }
    if (!(w->hist2D.aHist == NULL && w->hist2D.binEdgeLabeling))
    {
/* unbinned labeling							*/

/* set standard scale for makePSTics fuctions (1 pixel == 1 PS unit) 	*/
        fprintf(psfile,"gsave %d dup scale  			\n",
          MAXPIC);
 	makePSLabels (fs, oD->xSegment, psfile );
 	makePSLabels (fs, oD->ySegment, psfile );
        fprintf(psfile,"grestore 				\n");
    }	 	
}              


    


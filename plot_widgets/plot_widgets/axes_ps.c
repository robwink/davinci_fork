#include <X11/Intrinsic.h> 
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "2DGeom.h"
#include "uniLab.h"
#include "axes.h"

/* WARNING:  Prototyps for this file is in axes.h		*/

#define FONT_DELTA 2  /* font for printing will be smaller 
			 by this number of pixcels than
			 display font				*/
			 
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* 	
** void makePSAxis (axis *a, FILE *psfile )
**
** 	*a	Specifies the axis
**	*psfile	Specifies output file for PostScript program being generated
**
**		Generates PostScript code for axis image drawing according 
**		current axis state and puts it in a file specified.
**		Does not perform any operation with file, but "fprintf" only
**		Does not check any I/O errors
**		File should be opened before call
*/

void makePSAxis (axis *a, FILE *psfile )
{
    if (a == NULL) return;
    if (!a->mapped) return;
    makePSArrows (a, psfile );
    makePSTics (a->curSegment, psfile );
    makePSLabels (a->fs, a->curSegment, psfile );
    makePSName (a->fs, a->curSegment, psfile );
    fprintf(psfile,"gsave  						\n");
    fprintf(psfile,"newpath  						\n");
    fprintf(psfile,"%d %d moveto  					\n",
      a->axisImage.x1, a->axisImage.y1);
    fprintf(psfile,"%d %d lineto  					\n",
      a->axisImage.x2, a->axisImage.y2);
    fprintf(psfile,"stroke						\n");
    fprintf(psfile,"grestore  						\n");
}

/* 	
**  void makePSArrows (axis *a, FILE *psfile )
**
** 	a	Specifies the axis
**	*psfile	Specifies output file for PostScript program being generated
**
**		Generates PostScript code for arrows drawing according current
**		axis state and puts it in a file specified.
**		Does not perform any operation with file, but "fprintf" only
**		Does not check any I/O errors
**		File should be opened before call
*/
void makePSArrows (axis *a, FILE *psfile )
{
    if (a==NULL) return;
    if (!a->mapped) return;
    if (a->curSegment== NULL) return;

    fprintf(psfile,"save  						\n");

    fprintf(psfile,"/ar_length  %d def  \n",ARROW_LENGTH );

    fprintf(psfile,"/ar_angle   %d def  \n",(int)((double)ARROW_ANGLE / M_PI *180.));

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
    fprintf(psfile,"    ar_length set_len				\n");
    fprintf(psfile,"    ar_angle					\n");
    fprintf(psfile,"    neg						\n");
    fprintf(psfile,"    rotate						\n");
    fprintf(psfile,"    lineto						\n");
    fprintf(psfile,"    closepath					\n");
    fprintf(psfile,"    0 setgray					\n");
    fprintf(psfile,"    fill						\n");
    fprintf(psfile,"    grestore					\n");
    fprintf(psfile,"} def						\n");
    
    if (a->visible0 != NULL)
    {
	if (*(a->visible0) != a->minLimit && *(a->visible0) != a->maxLimit)
	{
	    fprintf(psfile,"%d %d					\n", 
        	a->axisImage.x2 - a->axisImage.x1, 
        	a->axisImage.y2 - a->axisImage.y1);
            fprintf(psfile,"%d %d					\n", 
        	a->axisImage.x1, a->axisImage.y1);
            fprintf(psfile,"arrow					\n");
        }    
    }
    if (a->visible1 != NULL)
    {
	if (*(a->visible1) != a->minLimit && *(a->visible1) != a->maxLimit)
	{
	    fprintf(psfile,"%d %d					\n", 
        	a->axisImage.x1 - a->axisImage.x2, 
        	a->axisImage.y1 - a->axisImage.y2);
            fprintf(psfile,"%d %d					\n", 
        	a->axisImage.x2, a->axisImage.y2);
            fprintf(psfile,"arrow					\n");
	}            
    }            
    fprintf(psfile,"restore						\n");

}
/* 	
** void makePSTics (labeledSegment *lS, FILE *psfile )
**
** 	a	Specifies the labeled segment;
**	*psfile	Specifies output file for PostScript program being generated
**
**		Generates PostScript code tics drawing according current
**		axis state and puts it in a file specified.
**		Does not perform any operation with file, but "fprintf" only
**		Does not check any I/O errors
**		File should be opened before call
*/
void makePSTics (labeledSegment *lS, FILE *psfile )
{
    vector ticDir;
    int i;
    if (lS == NULL) return;
    if (psfile == NULL) return;
    if (lS->nTics == 0) return;
    if (lS->ticPrms == NULL) return;
    if (lS->v0 == lS->v1) return;

    fprintf(psfile,"save  						\n");

    ticDir = mulVector(ortogonal(makeVector(lS->axisImage.x2 - lS->axisImage.x1, 
      lS->axisImage.y2 - lS->axisImage.y1)), -1);

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
    
    fprintf(psfile,"/tic_direction    %%  put tic direction vector on the stack	\n");
    fprintf(psfile,"{							\n");
    fprintf(psfile,"%d %d						\n",
      ticDir.x, ticDir.y);
    fprintf(psfile,"} def						\n");


    fprintf(psfile,"/tic_start_point   %%  tic_value  => x0 y0		\n");
    fprintf(psfile,"%% tic_value - value in object coordinates		\n");
    fprintf(psfile,"{							\n");
    fprintf(psfile,"%g sub						\n",
      lS->v0);
    fprintf(psfile,"%g div						\n",
      lS->v1 - lS->v0);
    fprintf(psfile,"dup							\n");
    fprintf(psfile,"%d cvr mul						\n",
      lS->axisImage.x2 - lS->axisImage.x1);
    fprintf(psfile,"%d cvr add						\n",
      lS->axisImage.x1);
    fprintf(psfile,"exch						\n");
    fprintf(psfile,"%d cvr mul						\n",
      lS->axisImage.y2 - lS->axisImage.y1);
    fprintf(psfile,"%d cvr add						\n",
      lS->axisImage.y1);
    fprintf(psfile,"} def						\n");         


    fprintf(psfile,"/tic    %%  tic_length tic_value  =>		\n");
    fprintf(psfile,"%% draw an axis ticmark				\n");
    fprintf(psfile,"%% tic_length - tic length				\n");
    fprintf(psfile,"%% tic_value - value in object coordinates		\n");
    fprintf(psfile,"{							\n");
    fprintf(psfile,"    newpath 					\n");
    fprintf(psfile,"    tic_start_point					\n");
    fprintf(psfile,"    moveto						\n");
    fprintf(psfile,"    tic_direction					\n");
    fprintf(psfile,"    3 2 roll  					\n");
    fprintf(psfile,"    set_len						\n");
    fprintf(psfile,"    rlineto						\n");
    fprintf(psfile,"    stroke						\n");
    fprintf(psfile,"} def						\n");

    for (i = 0; i < lS->nTics; i++)
        fprintf(psfile,"%d %g						\n",
          lS->ticPrms[i].length, lS->ticPrms[i].value);

    fprintf(psfile,"%d { tic } repeat					\n",
        lS->nTics);

    fprintf(psfile,"    restore						\n");
}        

/* 	
** void makePSLabels (XFontStruct * fs, labeledSegment *lS, 
**	FILE *psfile )
**
**	*fs	Specifies font structure used while labels has been
**		being constructed
** 	*lS	Specifies labeled segment
**	*psfile	Specifies output file for PostScript program being generated
**
**		Generates PostScript code for label drawing according current
**		axis state and puts it in a file specified.
**		Does not perform any operation with file, but "fprintf" only
**		Does not check any I/O errors
**		File should be opened before call
*/
void makePSLabels (XFontStruct * fs, labeledSegment *lS, FILE *psfile )
{
    labelsToDraw2 *label;
    vector seg;
    vector ticDir; 
    int i;

    if (lS== NULL) return;
    if (lS->nTics == 0) return; 
    if (lS->ticPrms == NULL) return;
    if (lS->labels == NULL) return;
    if (lS->v0 == lS->v1) return;
    if (fs == NULL) return;
    
    fprintf(psfile,"save					\n");
    
    fprintf(psfile,"/Times-Roman findfont			\n");
    fprintf(psfile,"%d						\n", 
	(fs->ascent + fs->descent) );
    fprintf(psfile,"%d sub 					\n",
      FONT_DELTA);
    fprintf(psfile,"scalefont 					\n");
    fprintf(psfile,"%% Make Y mirror				\n");
    fprintf(psfile,"matrix dup	 				\n");			
    fprintf(psfile,"0 1 put 					\n");
    fprintf(psfile,"dup 					\n");
    fprintf(psfile,"3 -1 put 					\n");					
    fprintf(psfile,"makefont 					\n");				
    fprintf(psfile,"setfont 					\n");				



    fprintf(psfile,"/char0_height 	%%   => height		\n");
    fprintf(psfile,"{						\n");
    fprintf(psfile,"    gsave					\n");
    fprintf(psfile,"    newpath					\n");
    fprintf(psfile,"    0 0 moveto				\n");
    fprintf(psfile,"    (0) false				\n");
    fprintf(psfile,"    charpath				\n");
    fprintf(psfile,"    flattenpath				\n");
    fprintf(psfile,"    pathbbox					\n");
    fprintf(psfile,"    grestore				\n");
    fprintf(psfile,"    exch					\n");
    fprintf(psfile,"    pop					\n");
    fprintf(psfile,"    exch					\n");
    fprintf(psfile,"    sub					\n");
    fprintf(psfile,"    exch					\n");
    fprintf(psfile,"    pop					\n");
    fprintf(psfile,"} def					\n");




    seg = makeVector(lS->axisImage.x2 - lS->axisImage.x1,
      lS->axisImage.y2 - lS->axisImage.y1);

    if (seg.y > 0)
    {
        if (lS->sidePlacing)
/* side placing 							*/
    	{	
/*  yShift=  ((double)PSdigWidth/2.+1.) * 
**           (double) seg.x /(double)seg.y + (double)PSdigHeight/2.  
*/                  
	    fprintf(psfile,"/yShift 	%%   => yShift			\n");
	    fprintf(psfile,"{						\n");
	    fprintf(psfile,"    (0)					\n");
	    fprintf(psfile,"    stringwidth				\n");
	    fprintf(psfile,"    pop					\n");
	    fprintf(psfile,"    char0_height				\n");
	    fprintf(psfile,"    dup 0 lt {neg} if   %% abs(top)		\n");
	    fprintf(psfile,"    2 div					\n");
	    fprintf(psfile,"    exch					\n");
	    fprintf(psfile,"    2 div					\n");
	    fprintf(psfile,"    1 add					\n");
	    fprintf(psfile,"    %g mul					\n",
        	(double) seg.x /(double)seg.y);
	    fprintf(psfile,"    add					\n"); 
	    fprintf(psfile,"} def 					\n");    

/*    
** label->wX = (lS->tics)[label->ticNumber].x2 - 1 - label->XWidth
** label->wY = (lS->tics)[label->ticNumber].y2 + vShift;
*/
	    fprintf(psfile,"/print_label 	%% string x y => 	\n");
	    fprintf(psfile,"%% string - label to print			\n");
	    fprintf(psfile,"%% x y - tic end coordinates		\n");
	    fprintf(psfile,"{						\n");
	    fprintf(psfile,"    yShift add				\n");
	    fprintf(psfile,"    exch					\n");
	    fprintf(psfile,"    2 index					\n");
	    fprintf(psfile,"    stringwidth				\n");
	    fprintf(psfile,"    pop    					\n");
	    fprintf(psfile,"    sub					\n");
	    fprintf(psfile,"    1 sub					\n");
	    fprintf(psfile,"    exch					\n");
	    fprintf(psfile,"    moveto    				\n");
	    fprintf(psfile,"    show    				\n");
	    fprintf(psfile,"}def    					\n");
	}    
 	else
 	{
/* top/down placing */	
	    fprintf(psfile,"/print_label 	%% string x y => 	\n");
	    fprintf(psfile,"%% string - label to print			\n");
	    fprintf(psfile,"%% x y - tic end coordinates		\n");
/*              	 
** label->wX = (lS->tics)[label->ticNumber].x2 -  label->XWidth + center + 1;
** label->wY = (lS->tics)[label->ticNumber].y2 + (seg.x > 0 ? 1 + digHeight : -1);
*/	    
	    fprintf(psfile,"{						\n");
	    fprintf(psfile,"    2 index					\n");
	    fprintf(psfile,"    stringwidth				\n");
	    fprintf(psfile,"    pop					\n");
	    fprintf(psfile,"    char0_height				\n");
	    if (seg.x > 0)
	    {
	        fprintf(psfile,"    dup 0 lt {neg} if   %% y => abs(y)	\n");
	        fprintf(psfile,"    1 add				\n");
	    }
	    else
	        fprintf(psfile,"    pop -1				\n"); 
	    fprintf(psfile,"    2 index					\n");
	    fprintf(psfile,"    add					\n");
/* y coordinate is done							*/
	    fprintf(psfile,"    exch					\n");

	    fprintf(psfile,"    neg					\n");

	    fprintf(psfile,"    dup					\n");
	    fprintf(psfile,"    neg					\n");
	    fprintf(psfile,"    2 div  					\n");
/* if (center > hShift) center=hShift;					*/
	    if (seg.y != 0)
	        fprintf(psfile,"%g 2 copy gt {exch} if pop 		\n",
	          (double)(LONG_TIC_LEN>MED_TIC_LEN ?
        	  LONG_TIC_LEN - MED_TIC_LEN : 1) *
        	  sqrt((double)(seg.x * seg.x + seg.y * seg.y)) /
        	  (double)seg.y);   
	    fprintf(psfile,"    add					\n");
	    fprintf(psfile,"    1 add					\n");
	    fprintf(psfile,"    3 index					\n");
	    fprintf(psfile,"    add					\n");
	    fprintf(psfile,"    exch					\n");
	    fprintf(psfile,"    moveto					\n");
	    fprintf(psfile,"    pop pop					\n");
	    fprintf(psfile,"    show					\n");
	    fprintf(psfile," } def					\n");
	}
    }		    
    else 
    {
/* seg.y <=0 */
        if (lS->sidePlacing)
/* side placing 							*/
    	{	
/*  yShift=  ((double)PSdigWidth/2.+1.) * 
**           (double) seg.x /(double)( - seg.y) + (double)PSdigHeight/2.  
*/                  
	    fprintf(psfile,"/yShift 	%%   => yShift			\n");
	    fprintf(psfile,"{						\n");
	    fprintf(psfile,"    (0)					\n");
	    fprintf(psfile,"    stringwidth				\n");
	    fprintf(psfile,"    pop					\n");
	    fprintf(psfile,"    char0_height				\n");
	    fprintf(psfile,"    dup 0 lt {neg} if   %% abs(top)		\n");
	    fprintf(psfile,"    2 div					\n");
	    fprintf(psfile,"    exch					\n");
	    fprintf(psfile,"    2 div					\n");
	    fprintf(psfile,"    1 add					\n");
	    fprintf(psfile,"    %g mul					\n",
        	(double) seg.x /(double)( -seg.y));
	    fprintf(psfile,"    add					\n"); 
	    fprintf(psfile,"} def 					\n");    
/*    
** label->wX = (lS->tics)[label->ticNumber].x2 + 1 + 1
** label->wY = (lS->tics)[label->ticNumber].y2 + vShift;
*/
	    fprintf(psfile,"/print_label 	%% string x y => 	\n");
	    fprintf(psfile,"%% string - label to print			\n");
	    fprintf(psfile,"%% x y - tic end coordinates		\n");
	    fprintf(psfile,"{						\n");
	    fprintf(psfile,"    yShift add				\n");
	    fprintf(psfile,"    exch					\n");
	    fprintf(psfile,"    1 add					\n");
	    fprintf(psfile,"    1 add					\n");
	    fprintf(psfile,"    exch					\n");
	    fprintf(psfile,"    moveto    				\n");
	    fprintf(psfile,"    show    				\n");
	    fprintf(psfile,"}def    					\n");
	}    
 	else
 	{
/* top/down placing	*/
	    fprintf(psfile,"/print_label 	%% string x y => 	\n");
	    fprintf(psfile,"%% string - label to print			\n");
	    fprintf(psfile,"%% x y - tic end coordinates		\n");
/*              	 
** label->wX = (lS->tics)[label->ticNumber].x2  - center + 1;
** label->wY = (lS->tics)[label->ticNumber].y2 + (seg.x > 0 ? 1 + digHeight : -1);
*/	    
	    fprintf(psfile,"{						\n");
	    fprintf(psfile,"    2 index					\n");
	    fprintf(psfile,"    stringwidth				\n");
	    fprintf(psfile,"    pop					\n");
	    fprintf(psfile,"    char0_height				\n");
	    if (seg.x > 0)
	    {
	        fprintf(psfile,"    dup 0 lt {neg} if   %% y => abs(y)	\n");
	        fprintf(psfile,"    1 add				\n");
	    }
	    else
	        fprintf(psfile,"    pop -1				\n"); 
	    fprintf(psfile,"    2 index					\n");
	    fprintf(psfile,"    add					\n");
/* y coordinate is done							*/
	    fprintf(psfile,"    exch					\n");

	    fprintf(psfile,"    2 div  					\n");
/* if (center > hShift) center=hShift;					*/
	    if (seg.y != 0)
	        fprintf(psfile,"%g 2 copy gt {exch} if pop 		\n",
	          (double)(LONG_TIC_LEN>MED_TIC_LEN ?
        	  LONG_TIC_LEN - MED_TIC_LEN : 1) *
        	  sqrt((double)(seg.x * seg.x + seg.y * seg.y)) /
        	  (double)abs(seg.y));   
	    fprintf(psfile,"    neg					\n");
	    fprintf(psfile,"    1 add					\n");
	    fprintf(psfile,"    3 index					\n");
	    fprintf(psfile,"    add					\n");
	    fprintf(psfile,"    exch					\n");
	    fprintf(psfile,"    moveto					\n");
	    fprintf(psfile,"    pop pop					\n");
	    fprintf(psfile,"    show					\n");
	    fprintf(psfile," } def					\n");
	}
    }

    ticDir = mulVector(ortogonal(makeVector(lS->axisImage.x2 - lS->axisImage.x1, 
      lS->axisImage.y2 - lS->axisImage.y1)), -1);

    fprintf(psfile,"/tic_direction    %%  put tic direction vector on the stack	\n");
    fprintf(psfile,"{							\n");
    fprintf(psfile,"%d %d						\n",
      ticDir.x, ticDir.y);
    fprintf(psfile,"} def						\n");

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
    
    fprintf(psfile,"/tic_start_point   %%  tic_value  => x0 y0		\n");
    fprintf(psfile,"%% tic_value - value in object coordinates		\n");
    fprintf(psfile,"{							\n");
    fprintf(psfile,"%g sub						\n",
      lS->v0);
    fprintf(psfile,"%g div						\n",
      lS->v1 - lS->v0);
    fprintf(psfile,"dup							\n");
    fprintf(psfile,"%d cvr mul						\n",
      lS->axisImage.x2 - lS->axisImage.x1);
    fprintf(psfile,"%d cvr add						\n",
      lS->axisImage.x1);
    fprintf(psfile,"exch						\n");
    fprintf(psfile,"%d cvr mul						\n",
      lS->axisImage.y2 - lS->axisImage.y1);
    fprintf(psfile,"%d cvr add						\n",
      lS->axisImage.y1);
    fprintf(psfile,"} def						\n");         

    fprintf(psfile,"/tic_end_point   %%  tic_length tic_value  => x0 y0	\n");
    fprintf(psfile,"%% draw an axis ticmark				\n");
    fprintf(psfile,"%% tic_length - tic length				\n");
    fprintf(psfile,"%% tic_value - value in object coordinates		\n");
    fprintf(psfile,"{							\n");
    fprintf(psfile,"    newpath 					\n");
    fprintf(psfile,"    tic_start_point					\n");
    fprintf(psfile,"    moveto						\n");
    fprintf(psfile,"    tic_direction					\n");
    fprintf(psfile,"    3 2 roll  					\n");
    fprintf(psfile,"    set_len						\n");
    fprintf(psfile,"    rmoveto						\n");
    fprintf(psfile,"    currentpoint					\n");
    fprintf(psfile,"} def						\n");

    i = 0;
    for(label = lS->labels;label != NULL; label = label->next)
    {
        fprintf(psfile,"(%s) %d %g					\n",
          label->label, lS->ticPrms[label->ticNumber].length, 
          lS->ticPrms[label->ticNumber].value);
	i++;
    }
    fprintf(psfile,"%d {tic_end_point print_label} repeat		\n",
        i);
    
    fprintf(psfile,"restore 						\n"); 

}   	
/* 	
** void makePSName (XFontStruct * fs, labeledSegment *lS, 
**	FILE *psfile )
**
** FOR "ONE STRING" NAME ONLY !!!
**
**	*fs	Specifies font structure used while labels has been
**		being constructed
** 	*lS	Specifies labeled segment
**	*psfile	Specifies output file for PostScript program being generated
**
**		Generates PostScript code for label drawing according current
**		axis state and puts it in a file specified.
**		Does not perform any operation with file, but "fprintf" only
**		Does not check any I/O errors
**		File should be opened before call
*/
void makePSName (XFontStruct * fs, labeledSegment *lS, FILE *psfile )
{
    labelsToDraw2 *label;
    int i;

    if (lS== NULL) return;
    if (lS->name == NULL) return;
    if (lS->name->label == NULL) return;
    
    fprintf(psfile,"save					\n");
    
    fprintf(psfile,"/Times-Roman findfont			\n");
    fprintf(psfile,"%d						\n", 
	(fs->ascent + fs->descent) );
    fprintf(psfile,"%d sub 					\n",
      FONT_DELTA);
    fprintf(psfile,"scalefont 					\n");
    fprintf(psfile,"%% Make Y mirror				\n");
    fprintf(psfile,"matrix dup	 				\n");			
    fprintf(psfile,"0 1 put 					\n");
    fprintf(psfile,"dup 					\n");
    fprintf(psfile,"3 -1 put 					\n");					
    fprintf(psfile,"makefont 					\n");				
    fprintf(psfile,"setfont 					\n");
    fprintf(psfile,"/side_placing %d def			\n",
        lS->nameSidePlacing);
    fprintf(psfile,"/ort %d def					\n",
        lS->ortDirection);
    fprintf(psfile,"/clip_x0 %d def				\n",
        lS->clipX0);
    fprintf(psfile,"/clip_x1 %d def				\n",
        lS->clipX1);
    fprintf(psfile,"/center_x %d def				\n",
        lS->centerX);
    fprintf(psfile,"/y %d def					\n",
        lS->name->wY);
    fprintf(psfile,"/name (%s) def				\n",
        lS->name->label);
 
    fprintf(psfile,"/name_width name stringwidth pop def	\n");

    fprintf(psfile,"side_placing 1 eq 				\n");
    fprintf(psfile,"{						\n");
    fprintf(psfile,"    ort 0 lt 				\n");
    fprintf(psfile,"    {					\n");
    fprintf(psfile,"	/start clip_x1 name_width sub def	\n");
    fprintf(psfile,"    }					\n");
    fprintf(psfile,"    {  					\n");
    fprintf(psfile,"         /start clip_x0 def			\n");
    fprintf(psfile,"    } 					\n");
    fprintf(psfile,"    ifelse					\n");
    fprintf(psfile,"}						\n");
    fprintf(psfile,"{   					\n");
    fprintf(psfile,"    ort 0 gt				\n");
    fprintf(psfile,"    {					\n");
    fprintf(psfile,"        /xLim center_x name_width 2 div sub def \n");
    fprintf(psfile,"        /start clip_x0 def			\n");
    fprintf(psfile,"        start xLim lt			\n");
    fprintf(psfile,"        {					\n");
    fprintf(psfile,"            clip_x1 name_width 2 div sub xLim lt \n");
    fprintf(psfile,"            {				\n");
    fprintf(psfile,"                /start clip_x1 name_width sub def \n");
    fprintf(psfile,"            }				\n");
    fprintf(psfile,"            {				\n");
    fprintf(psfile,"                /start xLim  def		\n");
    fprintf(psfile,"            }				\n");
    fprintf(psfile,"            ifelse				\n");
    fprintf(psfile,"        }					\n");
    fprintf(psfile,"        if    				\n");
    fprintf(psfile,"    }					\n");
    fprintf(psfile,"    {					\n");
    fprintf(psfile,"        /xLim center_x name_width 2 div sub def \n");          
    fprintf(psfile,"        /start clip_x1 name_width sub def	\n");
    fprintf(psfile,"        start xLim gt			\n");
    fprintf(psfile,"        {					\n");
    fprintf(psfile,"            clip_x0 xLim gt			\n");
    fprintf(psfile,"            {				\n");
    fprintf(psfile,"                /start clip_x0 def		\n");
    fprintf(psfile,"            }				\n");
    fprintf(psfile,"            {				\n");
    fprintf(psfile,"                /start xLim  def		\n");
    fprintf(psfile,"            }				\n");
    fprintf(psfile,"            ifelse				\n");
    fprintf(psfile,"        }					\n");
    fprintf(psfile,"        if					\n");
    fprintf(psfile,"   }					\n");
    fprintf(psfile,"   ifelse					\n");
    fprintf(psfile,"}						\n");
    fprintf(psfile,"ifelse         				\n");                                
    fprintf(psfile,"name start y moveto show			\n");
    fprintf(psfile,"restore 					\n"); 

}   	

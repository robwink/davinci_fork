#include <X11/Intrinsic.h> 
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
/* for interface function only	in test version				*/
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include "2DGeom.h"
/* below is that is really needed					*/
#include "uniLab.h"
#include "segment_name.h"

#define PIXELS_BETWEEN_LINES 1
static char *clipString (XFontStruct *fs, char *name, int XWidth);
static int makeFShiftValue(XSegment *axisImage, XSegment *tics, int nTics,
    labelsToDraw2 *lTD, vector ort, int ascent);

static char *clipString (XFontStruct *fs, char *name, int XWidth)
{
    char *nameP;
    char *clipName;
    char *chTmp;
    char *blank = " ";
    char *term = "...";
    int blankLen;
    int termLen;
    int XBlankWidth;
    int XTermWidth;
    int width;
    
    if (name == NULL)
        return(NULL); 
    
    blankLen = strlen (blank);
    termLen = strlen (term);
    
    XBlankWidth = XTextWidth(fs, blank, blankLen);
    XTermWidth = XTextWidth(fs, term, termLen);
    
/* make a private copy of the name			*/
    nameP = XtMalloc(sizeof(char) * (strlen(name) + 1));
    strcpy(nameP, name);
/* parse the name					*/

    XWidth += XBlankWidth;

    chTmp = strtok (nameP, "  \n\t");
    
    if (chTmp == NULL)
    { 
        XtFree(nameP); 
        return(NULL); 
    }

    width = 0;    
    while (chTmp != NULL)
    {
        width += XTextWidth(fs, chTmp, strlen(chTmp)) + XBlankWidth;
        chTmp = strtok (NULL, " \n\t");
    }	        
	
    strcpy(nameP, name);
    chTmp = strtok (nameP, "  \n\t");
    
    if (width <= XWidth)
    {
        clipName = XtMalloc(sizeof(char) * (strlen(name) + 1));
        strcpy(clipName, chTmp); 
        chTmp = strtok (NULL, " \n\t");
        while (chTmp != NULL)
    	{
            strcat(clipName, blank);
            strcat(clipName, chTmp);
            chTmp = strtok (NULL, " \n\t");
	}
	XtFree(nameP);
	return (clipName);
    }
    else
    {
        XWidth -= (XBlankWidth + XTermWidth);
        if (XWidth < 0)
        { 
            XtFree(nameP); 
            return(NULL); 
        }
        clipName = XtMalloc(sizeof(char) * (strlen(name) + 1 + 
          blankLen + termLen));
        if (XWidth >= XTextWidth(fs, chTmp, strlen(chTmp)) + XBlankWidth)
        {
            XWidth -= XTextWidth(fs, chTmp, strlen(chTmp)) + XBlankWidth;
            strcpy(clipName, chTmp); 
            strcat(clipName, blank);
            chTmp = strtok (NULL, " \n\t");
	    while (XWidth >= XTextWidth(fs, chTmp, strlen(chTmp)) + XBlankWidth)
            {
        	XWidth -= XTextWidth(fs, chTmp, strlen(chTmp)) + XBlankWidth;
        	strcat(clipName, chTmp); 
        	strcat(clipName, blank);
        	chTmp = strtok (NULL, " \n\t");
	    }  
	    strcat(clipName, term);   
	}
	else
	    strcpy(clipName, term);
	XtFree(nameP);
	return (clipName);
    }
}               	            

  
void placeName (labeledSegment *lS, XFontStruct *fs, char* name, 
    XSegment *axisImage, int clipX0, int clipX1,
    XSegment *tics, int nTics, labelsToDraw2 *ticMarks )
{
    vector ort;
    vector center;
    labelsToDraw2 *lTD;
    int ascent = fs->ascent;
    int descent = fs->descent;
    int vSpacing = ascent + descent + PIXELS_BETWEEN_LINES;
    int hSpacing;
    int f;
    int x, y;
    char *fn;
    
    if (lS == NULL) return;
    lS->name = NULL;
    if (fs == NULL) return ;
    if (name == NULL) return  ;
    if (axisImage->x1 == axisImage->x2 && axisImage->y1 == axisImage->y2)
         return ;

    hSpacing = XTextWidth(fs, " ", 1);
    
    ort = mulVector(ortogonal(makeVector(axisImage->x2 - axisImage->x1,
      axisImage->y2 - axisImage->y1)), -1);

    f = makeFShiftValue(axisImage, tics, nTics, ticMarks,  ort,  ascent);
    
    center = makeVector ((axisImage->x2 + axisImage->x1) /2,
      (axisImage->y2 + axisImage->y1) / 2);
    center = intVector( daddVectors (doubleVector(center), 
      dmulVector(doubleVector(ort), (double)(f - scalarProduct(ort,center)) /
      (double)(scalarProduct(ort,ort)))));
      
    if ( (scalarProduct(ort, ortogonal(makeVector(hSpacing, ascent))) >= 0 &&
      scalarProduct(ort, ortogonal(makeVector(-hSpacing, ascent))) >= 0 ) ||
      ( scalarProduct(ort, ortogonal(makeVector(hSpacing, ascent))) <= 0 &&
      scalarProduct(ort, ortogonal(makeVector(-hSpacing, ascent))) <= 0 ))   
    {
/* side placing								*/
        y = (int) floor(0.5 + (double) center.y + (double) ascent / 2. + 
          (double)hSpacing * 1.5 / (double)abs(ort.x ) * (double) ort.y);
        lS->nameSidePlacing = 1;  
	if (ort.x <0)
	{
	    x = center.x - hSpacing;
	    if (clipX1 > x)
	        clipX1 = x;
	    if (clipX0 >= clipX1)
	        return;
	    fn = clipString (fs, name, clipX1 - clipX0);
	    if (fn == NULL)
	        return;
 	    lTD = (labelsToDraw2 *)XtMalloc(sizeof(labelsToDraw2));
	    lTD->label = fn;	 
	    lTD->length = strlen(fn); 	 
	    lTD->XWidth = XTextWidth(fs, lTD->label, lTD->length);		 
	    lTD->lValue = 0;	 
	    lTD->wX = clipX1 - lTD->XWidth;	 
	    lTD->wY = y;
	    lTD->ticNumber = 0; 
	    lTD->next = NULL;
	}
	else
	{
	    x = center.x + hSpacing;
	    if (clipX0 < x)
	        clipX0 = x;
	    if (clipX0 >= clipX1)
	        return;
	    fn = clipString (fs, name, clipX1 - clipX0);
	    if (fn == NULL)
	        return;
 	    lTD = (labelsToDraw2 *)XtMalloc(sizeof(labelsToDraw2));
	    lTD->label = fn;	 
	    lTD->length = strlen(fn); 	 
	    lTD->XWidth = XTextWidth(fs, lTD->label, lTD->length);		 
	    lTD->lValue = 0;	 
	    lTD->wX = clipX0;	 
	    lTD->wY = y;
	    lTD->ticNumber = 0; 
	    lTD->next = NULL;
	}
    }
    else
    {
/* top/down placing							*/
 	lS->nameSidePlacing = 0;
        if (ort.y < 0)
        {
        
            y = center.y - ascent;
            if (ort.x != 0)
	         x = (f - y * ort.y)/ort.x;
	}
        else 
        {
            y = center.y + 2 * ascent;              	
	    if (ort.x != 0)
	         x = (f - (y - ascent) * ort.y)/ort.x;
	}
	if (ort.x > 0)
	{
	    int xLimit;
	    x += hSpacing;

	    if (clipX0 < x)
	        clipX0 = x;
	    if (clipX0 >= clipX1)
	        return;
	    fn = clipString (fs, name, clipX1 - clipX0);
	    if (fn == NULL)
	        return;
 	    lTD = (labelsToDraw2 *)XtMalloc(sizeof(labelsToDraw2));
	    lTD->label = fn;	 
	    lTD->length = strlen(fn); 	 
	    lTD->XWidth = XTextWidth(fs, lTD->label, lTD->length);		 
	    lTD->lValue = 0;	 

	    xLimit = center.x - lTD->XWidth/2;
	    lTD->wX = clipX0;	 
	    if (lTD->wX < xLimit)
	    {
	        if (clipX1 - lTD->XWidth/2 < xLimit)
	            lTD->wX = clipX1 - lTD->XWidth;
	        else
	            lTD->wX = xLimit;    
	    }        
	    lTD->wY = y;
	    lTD->ticNumber = 0; 
	    lTD->next = NULL;
	}
	else if (ort.x < 0)	    
	{
	    int xLimit;
	    x -= hSpacing;
	    if (clipX1 > x)
	        clipX1 = x;
	    if (clipX0 >= clipX1)
	        return;
	    fn = clipString (fs, name, clipX1 - clipX0);
	    if (fn == NULL)
	        return;
 	    lTD = (labelsToDraw2 *)XtMalloc(sizeof(labelsToDraw2));
	    lTD->label = fn;	 
	    lTD->length = strlen(fn); 	 
	    lTD->XWidth = XTextWidth(fs, lTD->label, lTD->length);		 
	    lTD->lValue = 0;	
	    
	    xLimit = center.x - lTD->XWidth/2;
	    lTD->wX = clipX1 - lTD->XWidth;
	    if (lTD->wX > xLimit)
	    {
	        if (clipX0 > xLimit)
	            lTD->wX = clipX0;
	        else
	            lTD->wX = xLimit;    
	    }        
	    lTD->wY = y;
	    lTD->ticNumber = 0; 
	    lTD->next = NULL;
	} 
	else
	{
	    int xLimit;
	    if (clipX0 >= clipX1)
	        return;
	    fn = clipString (fs, name, clipX1 - clipX0);
	    if (fn == NULL)
	        return;
 	    lTD = (labelsToDraw2 *)XtMalloc(sizeof(labelsToDraw2));
	    lTD->label = fn;	 
	    lTD->length = strlen(fn); 	 
	    lTD->XWidth = XTextWidth(fs, lTD->label, lTD->length);		 
	    lTD->lValue = 0;	
	    
	    xLimit = center.x - lTD->XWidth/2;
	    lTD->wX = clipX1 - lTD->XWidth;
	    if (lTD->wX > xLimit)
	    {
	        if (clipX0 > xLimit)
	            lTD->wX = clipX0;
	        else
	            lTD->wX = xLimit;    
	    }        
	    lTD->wY = y;
	    lTD->ticNumber = 0; 
	    lTD->next = NULL;
	}
    }
    lS->clipX0 = clipX0;
    lS->clipX1 = clipX1;
    lS->centerX = center.x;
    lS->ortDirection = (ort.x == 0 ? 0 : (ort.x > 0 ? 1 : -1));
    lS->name = lTD;
}    		    
	
 
/*
** calculate boundary functional value, for the linear functoinal defined
** by <ort> to "cut" space with tics and ticmarks
**
*/
static int makeFShiftValue(XSegment *axisImage, XSegment *tics, int nTics,
    labelsToDraw2 *lTD, vector ort, int ascent)
{
    int f;
    int i;
    int x;
    int y;
 
    f = ort.x * axisImage->x1 + ort.y * axisImage->y1;
    if (tics != NULL && nTics != 0)
    	for (i = 0; i< nTics; i++)
    	{
    	    if (ort.x * tics[i].x2 + ort.y * tics[i].y2 > f)
    	        f = ort.x * tics[i].x2 + ort.y * tics[i].y2;
 	}    	        
    while (lTD != NULL)
    {
	if (ort.x > 0)
	    x = lTD->wX + lTD->XWidth;
	else
	    x = lTD->wX;
	if (ort.y < 0)
	    y = lTD->wY - ascent;
	else    
	    y = lTD->wY;
	if (ort.x * x + ort.y * y > f)
	    f = ort.x * x + ort.y * y;
 	lTD = lTD->next;
    }
   return (f);
}                      		            	                
                 

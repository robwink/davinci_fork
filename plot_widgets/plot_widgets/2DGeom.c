#include <math.h>
#include <X11/Xlib.h>
#include "2DGeom.h"

/*
** make vector from coordinates
*/
vector makeVector(int x, int y)
{
   vector v;
   v.x=x;
   v.y=y;
   return(v);
}
/*
** compare vectors 	
*/
Boolean eqVectors(vector v1, vector v2)
{
    return (v1.x==v2.x && v1.y==v2.y);
}
/*
** make dvector from coordinates
*/
dvector dmakeVector(double x, double y)
{
   dvector v;
   v.x=x;
   v.y=y;
   return(v);
}
/*
** conversions
*/
vector intVector(dvector v)
{
    vector iv;
    iv.x= floor (v.x + 0.5);
    iv.y= floor (v.y + 0.5);
    return(iv);
}
dvector doubleVector(vector v)
{
    dvector dv;
    dv.x=v.x;
    dv.y=v.y;
    return(dv);
}
XPoint xPoint(vector v)
{
    XPoint p;
    p.x=v.x;
    p.y=v.y;
    return(p);
}
range rangeOf(vector v)
{
    range r;
    r.start=v.x;
    r.end=v.y;
    return(r);
}
vector vectorOf(range r)
{
    vector v;
    v.x=r.start;
    v.y=r.end;
    return(v);
}    
/*
** add vectors 
*/
vector addVectors(vector v1, vector v2)
{
    v1.x += v2.x;
    v1.y += v2.y;
    return (v1);
}
/* 
** difference (vector shift) between two vectors - 
*/
vector subVectors(vector v1, vector v2)
{
    v1.x -= v2.x;
    v1.y  -= v2.y;
    return (v1);
}
/* 
** multiplay vector by integer - 
*/
vector mulVector(vector v, int i)
{
    v.x *= i;
    v.y *= i;
    return (v);
}
/* 
** divide vector by integer - 
*/
vector divVector(vector v, int i)
{
    v.x /= i;
    v.y /= i;
    return (v);
}
/* 
** shift vector by integer - 
*/
vector lShiftVector(vector v, int i)
{
    v.x = v.x << i;
    v.y = v.y << i;
    return (v);
}
vector rShiftVector(vector v, int i)
{
    v.x = v.x >> i;
    v.y = v.y >> i;
    return (v);
}
/*
** add vectors 
*/
dvector daddVectors(dvector v1, dvector v2)
{
    v1.x += v2.x;
    v1.y += v2.y;
    return (v1);
}
/* 
** difference (vector shift) between two vectors - 
*/
dvector dsubVectors(dvector v1, dvector v2)
{
    v1.x -= v2.x;
    v1.y  -= v2.y;
    return (v1);
}
/* 
** multiplay vector by double - 
*/
dvector dmulVector(dvector v, double i)
{
    v.x *= i;
    v.y *= i;
    return (v);
}
/* 
** divide vector by double - 
*/
dvector ddivVector(dvector v, double i)
{
    v.x /= i;
    v.y /= i;
    return (v);
}
/* 
** scalar product of two vectors 		
** care should be taken to avoid overflow    
** while using
*/
int scalarProduct (vector v1, vector v2)
{
    return(v1.x * v2.x + v1.y * v2.y);
} 
/*
** double version of the same function 
*/
double dscalarProduct (dvector v1, dvector v2)
{
    return( v1.x *  v2.x + v1.y *  v2.y);
}
/* 
** produce ortogonal vector, counterclockwise rotation	
*/
vector ortogonal(vector v)
{
    int tmp = v.x;
    v.x = v.y;
    v.y = -tmp;
    return(v);
}
dvector dortogonal(dvector v)
{
    double tmp = v.x;
    v.x = v.y;
    v.y = -tmp;
    return(v);
}
/*
** length of vector
*/ 
double vectorLength(vector v)
{
    return dvectorLength(doubleVector (v));
}
double dvectorLength(dvector v)
{
    return sqrt(dscalarProduct(v,v));
}
/*
** length of the projection v1 to v2
*/
double projectionLength(vector v1, vector v2)
{
    int v2l = vectorLength(v2);
    if (v2l == 0) return (0);
    return( dscalarProduct(doubleVector (v1),doubleVector (v2)) /(double)v2l);
}  
double dprojectionLength(dvector v1, dvector v2)
{
    double v2l = dvectorLength(v2);
    if (v2l == 0.0) return (0.0);
    return( dscalarProduct(v1,v2) / v2l);
}  
/*
** "pull" vector to requested length
** because of integer result, accuracy is not very good, and sign of
** eeror is "/" operator implementation dependent
*/
vector setLength(vector v, int l)
{
/* some heuristic vector rounding technic assuming next goals:
   to keep angle accuracy is more important then length accuracy
   to make vector longer is better then to make it shorter then requested
*/
    vector vL;
    vector vs;
    dvector vd, vo;
    double an[6];
    double ln[6];
    int i,j;
    if (vectorLength(v) > .5)  /* it is integer vector */
    {
        vd.x = ((double)l * (double)v.x / vectorLength(v));
        vd.y = ((double)l * (double)v.y / vectorLength(v));
        if (vd.x < 0)
        {
            vs.x = -1;
            vd.x = -vd.x;
        }
        else
            vs.x = 1;
        if (vd.y < 0)
        {
            vs.y = -1;
            vd.y = -vd.y;
        }
        else
            vs.y = 1;
        vo = dortogonal(vd);
        vd.x = floor (vd.x);
        vd.y = floor (vd.y);
        an[0]=fabs(dscalarProduct(vo,vd));
        ln[0]=dvectorLength(vd);
        vd.y +=1.;
        an[1]=fabs(dscalarProduct(vo,vd));
        ln[1]=dvectorLength(vd);
        vd.x +=1.;
        vd.y -=1.;
        an[2]=fabs(dscalarProduct(vo,vd));
        ln[2]=dvectorLength(vd);
        vd.y +=1.;
        an[3]=fabs(dscalarProduct(vo,vd));
        ln[3]=dvectorLength(vd);
        vd.y +=1.;
        an[4]=fabs(dscalarProduct(vo,vd));
        ln[4]=dvectorLength(vd);
        vd.x +=1.;
        vd.y -=1;
        an[5]=fabs(dscalarProduct(vo,vd));
        ln[5]=dvectorLength(vd);
        vd.x -=2;
        vd.y -=1;
	j=6;
	for (i=5;i>=0;i--)
	     if (ln[i] + .3 > l || ln[i] - 0.8 < l )
	         j=i;
	if (j!=6)
        {
	    for (i=2;i>=0;i--)
	        if (ln[i] + .3 > l || ln[i] - 0.8 < l )
	            if (an[i] < an [j] )
	                j=i;
        }
	else
	{
	/* must not happen ever */
	    j=3;
	    for (i=2;i>=0;i--)
	        if (an[i] < an [j] )
	            j=i;
	}
	switch (j) {
	    case 0: break;
	    case 1: vd.y += 1.; break;
	    case 2: vd.x += 1.; break;
	    case 3: vd.x += 1.; vd.y += 1.; break;
	    case 4: vd.x += 1.; vd.y += 2.; break;
	    case 5: vd.x += 2.; vd.y += 1.; break;
	}
	vL.x = vd.x;
	vL.y = vd.y;
	vL.x *= vs.x;
	vL.y *= vs.y;
    }
    else vL=makeVector(0,0);
    return(vL);
}

dvector dsetLength(dvector v, double l)
{
    dvector vL;
    if (dvectorLength(v) != 0.0 )  /* ???????????? - floating value */
    {
        vL.x = l * v.x / dvectorLength(v);
        vL.y = l * v.y / dvectorLength(v);
    }
    else vL=dmakeVector(0.,0.);
    return(vL);
}
/* locate point (vector) in a band with boundarys paased through b and
** b + w and are ortogonal to w. 
*/ 
double dpointLocation(dvector v,dvector b,dvector w)
{
    double f, f1, f2;
    f=dscalarProduct(v,w);
    f1=dscalarProduct(b,w);
    f2=f1+dscalarProduct(w,w);
    return ((f-f1)/(f2-f1));
}
double pointLocation(vector v,vector b,vector w)
{
    return(dpointLocation(doubleVector(v),doubleVector(b),doubleVector(w)));
}
Boolean pointInBand (vector v, vector b, vector w)
{
    double l=pointLocation(v, b, w);
    return( l > 0 && l <=1 );
}
Boolean dpointInBand (dvector v, dvector b, dvector w)
{
    double l=dpointLocation(v, b,w);
    return( l > 0 && l <=1 );
}
/*
** make range from parameters
*/
range makeRange (int start, int end)
{
    range r;
    r.start=start;
    r.end=end;
    return(r);
}

range normalizeRange(range r)
{
   if (r.start > r.end)
       return(makeRange(r.end,r.start));
   else
       return (r);
}   
/* 
** check if value is in the range, if not correct value
** to nearest in the range.
*/
int adjustValueToRange(int value, range limit)
{
    if(value < limit.start) return(limit.start);
    if(value >= limit.end) return(limit.end-1);
    return(value);
}
/* 
** check if range is in a limit range. If not,
** change it to an intersection with the limit range.
** If the intersection is empty, change it to
** first (last) point of limit range, what is closer to source range.
*/
range adjustRangeToRange(range new, range limits, int minL)
{
    new.start = adjustValueToRange(new.start,limits);
    new.end = adjustValueToRange(new.end-1,makeRange(new.start,limits.end))+1;
    if (new.end - new.start < minL)
    {
	if (limits.end - limits.start < minL)
	{ 
            new.start = limits.start;
            new.end = limits.end;
	} 
	else
	{
	    int i= minL - (new.end - new.start);
	    int j = i/2;
	    i -=j;
	    if (new.start == limits.start)
	        new.end = new.start + minL;
	    else if (new.end == limits.end)
	        new.start = new.end - minL;
	    else if (new.start - j >= limits.start)
	    {
	        if (new.end + i <= limits.end)
	        {
	            new.start -=j;
	            new.end +=i;
	        }
	        else
	        {
	            new.end = limits.end;
	            new.start = new.end - minL;
	        }
	     }
	     else
	     {
	         new.start =limits.start;
	         new.end= new.start + minL;
	     }
	 }
    }
    return (new);
}
/* 
** check if range is in a limit range. If not,
** "move" it to the limit range.
** If it larger then the limit, return limit
*/
range moveRangeToRange(range new, range limits)
{
    if (new.end > limits.end) 
    { 
        new.start -= new.end - limits.end;
        new.end = limits.end;
    } 
    if ((new.start) < (limits.start))
    {
        new.end += limits.start - new.start; 
        new.start = limits.start;
    }
    new.end = adjustValueToRange(new.end-1,makeRange(new.start,limits.end))+1;
    return (new);    
}
/*
** make 2d range from ranges
*/
range2d makeRange2dFromRanges (range x, range y)
{
    range2d r;
    r.start.x = x.start;
    r.start.y = y.start;
    r.end.x = x.end;
    r.end.y = y.end;
    return(r);
}

/*
** make 2d range from vectors
*/
range2d makeRange2dFromVectors (vector start, vector end)
{
    range2d r;
    r.start = start;    
    r.end = end;
    return(r);
}
/*
** compare ranges	
*/
Boolean eqRanges2d(range2d r1, range2d r2)
{
    return (eqVectors(r1.start,r2.start) && eqVectors(r1.end,r2.end));
}
/*
** get x range from 2d range
*/
range xRange(range2d r2d)
{
    range r;
    r.start = r2d.start.x;    
    r.end = r2d.end.x;
    return(r);
}
/*
** get y range from 2d range
*/
range yRange(range2d r2d)
{
    range r;
    r.start = r2d.start.y;    
    r.end = r2d.end.y;
    return(r);
}
/*
** get start vector from 2d range
*/
vector range2dStart(range2d r)
{
    return(r.start);
}
/*
** get end vector from 2d range
*/
vector range2dEnd(range2d r)
{
    return(r.end);
}

range2d normalizeRange2d(range2d r)
{
    return(makeRange2dFromRanges(
        normalizeRange(xRange(r)),normalizeRange(yRange(r))));
}

/* 
** check if vector is in 2d-range,  correct if not  
*/
vector adjustVectorToRange2d(vector v, range2d limit)
{

    return (makeVector(adjustValueToRange(v.x,xRange(limit)),
      adjustValueToRange(v.y,yRange(limit))));
}
/* 
** check if vector is in 2d-range,  correct if not  
*/
range2d adjustRange2dToRange2d(range2d r, range2d limit, int minL)
{
    return(makeRange2dFromRanges(adjustRangeToRange(xRange(r),xRange(limit),minL),
      adjustRangeToRange(yRange(r),yRange(limit),minL)));
}
range2d moveRange2dToRange2d(range2d r, range2d limit)
{
    return(makeRange2dFromRanges(moveRangeToRange(xRange(r),xRange(limit)),
      moveRangeToRange(yRange(r),yRange(limit))));
}
/*
** make matrix from two vectors
*/
matrix2 makeMatrix(vector v1, vector v2)
{
    matrix2 m;
    m.x=v1;
    m.y=v2;
    return (m);
}
/*
** get first (x) colomn of matrix 
*/
vector xColomn(matrix2 m)
{
    vector v;
    v.x=m.x.x;
    v.y=m.y.x;
    return (v);
}
/*
** get second (y) colomn fo matrix 
*/
vector yColomn(matrix2 m)
{
    vector v;
    v.x=m.x.y;
    v.y=m.y.y;
    return (v);
}
/*
** make transposed matrix from matrix
*/
matrix2 transposeMatrix(matrix2 m)
{
    matrix2 new;
    new.x.x=m.x.x;
    new.x.y=m.y.x;
    new.y.x=m.x.y;
    new.y.y=m.y.y;
    return (new);
}
/* the same, but more functional style 
{
    return(makeMatrix2(xColomn(m),yColomn(m));
}
*/
/*
** multiply matrix by vector (M * V)
*/
vector matrixMultVector(matrix2 m,vector v)
{
    vector new;
    new.x=scalarProduct(m.x,v);
    new.y=scalarProduct(m.y,v);
    return (new);
}
/*
** multiply matrix by matrix (M * M)
*/
matrix2 matrixMultMatrix(matrix2 m1, matrix2 m2)
{
    matrix2 new;
    new.x=matrixMultVector(m1,xColomn(m2));
    new.y=matrixMultVector(m1,yColomn(m2));
    return (transposeMatrix(new));
}

range2d transformRange2d (range2d r, matrix2 m)
{
    return(normalizeRange2d(makeRange2dFromVectors(matrixMultVector(m,r.start),
      matrixMultVector(m,r.end))));
}   

range2d translateRange2d (range2d r, vector v)
{
    r.start=addVectors(r.start,v);
    r.end=addVectors(r.end,v);
    return(r);
}

range translateRange (range r, int v)
{
    r.start += v;
    r.end += v;
    return(r);
}

/*
**  Boolean pointInPara(vector vertex, vector edge1, vector edge2, vector point)
**
**  vertex	- vertex of the paprallelogram
**  edge1	- vector of the parallelogram edge
**  edge2	- vector of the parallelogram edge
**
**  	the parallelogram is the one with vertices
**      vertex, vertex + edge1, vertex + edge2, vertex + edge1 + edge2
**	where <+> is 2d vector sum
**
**  Returns TRUE if the point is in the parallelogram described above,
**  boundaries included, (but as actual boundaries, not there representation
**  on the pixcel grid),
**  and FALSE otherwise.
*/
Boolean pointInPara(vector vertex, vector edge1, vector edge2, vector point)
{
    vector eo1, eo2;
    int s1, s11, s12, s2, s21, s22;
    eo1 = ortogonal (edge1);
    eo2 = ortogonal (edge2);
    s1 = scalarProduct(point, eo1);
    s11 = scalarProduct(vertex, eo1);
    s12 = s11 + scalarProduct(edge2, eo1); 
    s2 = scalarProduct(point, eo2);
    s21 = scalarProduct(vertex, eo2);
    s22 = s21 + scalarProduct(edge1, eo2);
    return ( ( ((s11 <= s1 ) && (s1 <= s12)) ||
               ((s12 <= s1 ) && (s1 <= s11)) ) &&
             ( ((s21 <= s2 ) && (s2 <= s22)) ||
               ((s22 <= s2 ) && (s2 <= s21)) ) );
}               
 
    

#ifndef _2DGEOM_H
#define _2DGEOM_H
#include <X11/Intrinsic.h> /* for type Boolean only */
/* vector of 2d vector space 			*/
typedef struct _vector {
    int x;
    int y;
} vector;

/* vector of 2d vector space 			*/
typedef struct _dvector {
    double x;
    double y;
} dvector;

/* integer range				*/
typedef struct _range  {
    int start;
    int end;
} range;

/* plane integer range				*/
typedef struct _range2d  {
    vector start;
    vector end;
} range2d;

/* 2 x 2 matrix					*/
typedef struct _matrix2 {
    vector x;		/* first row		*/
    vector y;		/* second row		*/
} matrix2;
/* so "indexes" looks like
**       / x.x  x.y \
**       \ y.x  y.y /
*/

/* make vector from coordinates			*/
vector makeVector(int x, int y);
dvector dmakeVector(double x, double y);

/* compare vectors 				*/
Boolean eqVectors(vector v1, vector v2);

/* conversions					*/
vector intVector(dvector v);
dvector doubleVector(vector v);
XPoint xPoint(vector v);
range rangeOf(vector v);
vector vectorOf(range r);

/* add vectors 					*/
vector addVectors(vector v1, vector v2);
dvector daddVectors(dvector v1, dvector v2);

/* difference (vector shift) between two vectors */
vector subVectors(vector v1, vector v2);
dvector dsubVectors(dvector v1, dvector v2);

/* multiplay vector by number			*/
vector mulVector(vector v, int i);
dvector dmulVector(dvector v, double i);

/* divide vector by number			*/
vector divVector(vector v, int i);
dvector ddivVector(dvector v, double i);

/* shift vector by integer - 			*/
vector lShiftVector(vector v, int i);
vector rShiftVector(vector v, int i);

/* scalar product of two vectors 		*/	
int scalarProduct (vector v1, vector v2);
double dscalarProduct (dvector v1, dvector v2);

/* ortogonal vector, counterclockwise rotation	*/
vector ortogonal(vector v);
dvector dortogonal(dvector v);

/* length of vector, does not lead to  overflow	*/
double vectorLength(vector v);
double dvectorLength(dvector v);

/* length of the projection v1 to v2		*/
double projectionLength(vector v1, vector v2);
double dprojectionLength(dvector v1, dvector v2);

/* "pull" vector to requested length		*/
vector setLength(vector v, int length);
dvector dsetLength(dvector v, double l);

/* locate point (vector) in a band 		*/
double pointLocation(vector v,vector b,vector w);
double dpointLocation(dvector v,dvector b,dvector w);
Boolean pointInBand (vector v, vector b, vector w);
Boolean dpointInBand (dvector v, dvector b, dvector w);

/* check if  point is in a parallelogram	*/ 
Boolean pointInPara(vector vertex, vector edge1, vector edge2, vector point);

/* make range from parameters			*/
range makeRange (int start, int end);

range normalizeRange(range r);

/* check if value is in range,  correct if not  */
int adjustValueToRange(int value, range limit);

/* check if range is in other range, correct if not*/
range adjustRangeToRange(range new, range limits, int minL);
range moveRangeToRange(range new, range limits);

/* make 2d range from ranges			*/
range2d makeRange2dFromRanges (range x, range y);

/* make 2d range from vector			*/
range2d makeRange2dFromVectors (vector start, vector end);

/* compare ranges				*/
Boolean eqRanges2d(range2d r1, range2d r2);

/* get x range from 2d range			*/
range xRange(range2d r);

/* get y range from 2d range			*/
range yRange(range2d r);

/* get start vector from 2d range		*/
vector range2dStart(range2d r);

/* get end vector from 2d range			*/
vector range2dEnd(range2d r);

range2d normalizeRange2d(range2d r);

/* check if vector is in 2d-range,  correct if not  */
vector adjustVectorToRange2d(vector v, range2d limit);

/* check if vector is in 2d-range,  correct if not */
range2d adjustRange2dToRange2d(range2d r, range2d limit, int minL);
range2d moveRange2dToRange2d(range2d r, range2d limit);

/* make matrix from two vectors			*/
matrix2 makeMatrix(vector v1, vector v2);

/* get second (y) colomn fo matrix 		*/
vector xColomn(matrix2 m);

/* get second (y) colomn fo matrix 		*/
vector yColomn(matrix2 m);

/* make transposed matrix from matrix		*/
matrix2 transposeMatrix(matrix2 m);

/* multiply matrix by vector (M * V)		*/
vector matrixMultVector(matrix2 m,vector v);

/* multiply matrix by matrix (M * M)		*/
matrix2 matrixMultMatrix(matrix2 m1, matrix2 m2);

range2d transformRange2d (range2d r, matrix2 m);

range2d translateRange2d (range2d r, vector v);

range translateRange (range r, int v);

Boolean pointInPara(vector vertex, vector edge1, vector edge2, vector point);

#endif

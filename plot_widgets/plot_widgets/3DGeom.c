/* Some of 3D geometry utilities for 3D widgets.
** 
*/
#include <math.h>
#include "2DGeom.h"
#include "3DGeom.h"
   
/*
** make vector from coordinates
*/ 
vector3 makeVector3(int x, int y, int z)
{
   vector3 v;
   v.x=x;
   v.y=y;
   v.z=z;
   return(v);
}
/*
** get 2D vector from 3D
*/ 
vector vector2of3(vector3 v)
{
    vector v2;
    v2.x = v.x;
    v2.y = v.y;
    return(v2);
} 
   

dvector3 dmakeVector3(double x, double y, double z)
{
   dvector3 dv;
   dv.x=x;
   dv.y=y;
   dv.z=z;
   return(dv);
}

dvector3 doubleVector3(vector3 v)
{
    dvector3 dv;
    dv.x = v.x;
    dv.y = v.y;
    dv.z = v.z;
    return(dv);
}    
/* 
** multiplay vector by integer - 
*/
vector3 mulVector3(vector3 v, int i)
{
    v.x *= i;
    v.y *= i;
    v.z *= i;
    return (v);
}
/* 
** multiplay dvector   
*/
dvector3 dmulVector3(dvector3 v, double i)
{
    v.x *= i;
    v.y *= i;
    v.z *= i;
    return (v);
}
/* 
** divide vector by integer - 
*/
vector3 divVector3(vector3 v, int i)
{
    v.x /= i;
    v.y /= i;
    v.z /= i;
    return (v);
}
/* 
** divide vector   
*/
dvector3 ddivVector3(dvector3 v, double i)
{
    v.x /= i;
    v.y /= i;
    v.z /= i;
    return (v);
}

/*
** add vectors 
*/
vector3 addVectors3(vector3 v1, vector3 v2)
{
    v1.x += v2.x;
    v1.y += v2.y;
    v1.z += v2.z;
    return (v1);
}

dvector3 daddVectors3(dvector3 v1, dvector3 v2)
{
    v1.x += v2.x;
    v1.y += v2.y;
    v1.z += v2.z;
    return (v1);
}

/*
** sub vectors 
*/
vector3 subVectors3(vector3 v1, vector3 v2)
{
    v1.x -= v2.x;
    v1.y -= v2.y;
    v1.z -= v2.z;
    return (v1);
}

dvector3 dsubVectors3(dvector3 v1, dvector3 v2)
{
    v1.x -= v2.x;
    v1.y -= v2.y;
    v1.z -= v2.z;
    return (v1);
}
/* 
** scalar product of two vectors 		
** care should be taken to avoid overflow    
** while using
*/
int scalarProduct3 (vector3 v1, vector3 v2)
{
    return(v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
} 
/*
** double version of the same function 
*/
double dscalarProduct3 (dvector3 v1, dvector3 v2)
{
    return(v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
}

/*
** length of the projection v1 to v2
*/
double projectionLength3(vector3 v1, vector3 v2)
{
    int v2l = vectorLength3(v2);
    if (v2l == 0) return (0);
    return( dscalarProduct3(doubleVector3 (v1),doubleVector3 (v2)) /(double)v2l);
}  
double dprojectionLength3(dvector3 v1, dvector3 v2)
{
    double v2l = dvectorLength3(v2);
    if (v2l == 0.0) return (0.0);
    return( dscalarProduct3(v1,v2) / v2l);
} 
/*
** length of vector
*/ 
double vectorLength3(vector3 v)
{
    return dvectorLength3(doubleVector3 (v));
}
double dvectorLength3(dvector3 v)
{
    return sqrt(dscalarProduct3(v,v));
}

/*
** "pull" vector to requested length
** 
*/
dvector3 dsetLength3(dvector3 v, double l)
{
    dvector3 vL;
    if (dvectorLength3(v) != 0 )  /* ???????????? - floating value */
    {
        vL.x = l * v.x / dvectorLength3(v);
        vL.y = l * v.y / dvectorLength3(v);
        vL.z = l * v.z / dvectorLength3(v);
    }
    else vL=dmakeVector3(0.0, 0.0, 0.0);
    return(vL);
}
/*
** dvector3 dortogonalize3To1(dvector3 dv1, dvector3 dv2);
**
** Return a vector otogonal to <dv1> in the plane defined by <dv1>, <dv2>
** by subtracting of the collinear component.
** So the length of the projection of <dv2> and the result to the vector
** ortogonal to <dv1> are the same.
** If no actual plane is defined, the result is 
** either <dv2> or 0 depending on the case
*/
dvector3 dortogonalize3To1(dvector3 dv1, dvector3 dv2)
{
    double v1len;
    v1len = dvectorLength3(dv1);
    if (v1len == 0.0) return (dv2);
    return (dsubVectors3(dv2, dsetLength3(dv1,dscalarProduct3(dv1, dv2)/v1len)));
}

/*
** dvector3 dortogonalize3To2(dvector3 dv1, dvector3 dv2, dvector3 dv3);
**
** return a vector otogonal to <dv1> , <dv2> which have
** the same projection to the vector ortogonal to the (<dv1> , <dv2>) palne 
** as <dv3>.
** In singular cases the result is generally undefined.
*/
dvector3 dortogonalize3To2(dvector3 dv1, dvector3 dv2, dvector3 dv3)
{
    dvector3 v2o;
    dvector3 v3o;
    v2o = dortogonalize3To1(dv1, dv2);
    v3o = dortogonalize3To1(dv1, dv3);
    return (dortogonalize3To1(v2o, v3o));
}

/*
** vector3 ortogonal3To2(vector3 dv1, vector3 dv2);
**
** return a vector otogonal to <dv1> , <dv2> 
** The triple (<dv1> , <dv2>, <result>) is right oriented, length of the
** <result> - rather arbitrary (but "long" - be aware of overflow).
** 
** In singular cases the result is generally undefined.
*/
vector3 ortogonal3To2(vector3 dv1, vector3 dv2)
{
    vector3 vo;
    vo.x = dv1.y * dv2.z - dv1.z * dv2.y;
    vo.y = dv1.z * dv2.x - dv1.x * dv2.z;
    vo.z = dv1.x * dv2.y - dv1.y * dv2.x;
    return (vo);
}

/*
** dvector3 dortogonal3To2(dvector3 dv1, dvector3 dv2);
**
** return a vector otogonal to <dv1> , <dv2> 
** (The triple (<dv1> , <dv2>, <result>) is right oriented, length of the
** <result> - rather arbitrary.
** 
** In singular cases the result is generally undefined.
*/
dvector3 dortogonal3To2(dvector3 dv1, dvector3 dv2)
{
    dvector3 vo;
    vo.x = dv1.y * dv2.z - dv1.z * dv2.y;
    vo.y = dv1.z * dv2.x - dv1.x * dv2.z;
    vo.z = dv1.x * dv2.y - dv1.y * dv2.x;
    return (vo);
}
/*
** dvector3 dtransformVector3(double m[3][3],dvector3 v) 
**
**	m		 matrix to multiply by.
**	v		 vector to transform.
**
** Apply matrix M to a vector.
**
**
*/
dvector3 dtransformVector3(double m[3][3],dvector3 v)
{
    dvector3 d;
    d.x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z;
    d.y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z;
    d.z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z;
    return (d);
}    

#include <float.h>
#include <math.h>
#include "geometry.h"
#include "2DGeom.h"
#include "3DGeom.h"
#include "3DRot.h"

#define ROT_DELTA FLT_MIN
static void mcheck (double m[3][3]);
/* The rotation model:
   View to the object coordinate system:

	Z ^
	  |
	  |
	  |
	 (.)--------->
        X 		Y
   (projection of the object space to the window, the object is
    rotating in object system, to provide different views)
   The extended window coordinate system (with Z axis added):
   
       Z 		X
        (x)---------->
         |
         |
         |
         v
       Y  

     The transfer matrix:
*/
static double objToWin [3][3] = {
      0.0,  1.0,  0.0,
      0.0,  0.0, -1.0,
     -1.0,  0.0,  0.0
}; 

/*
**   makeRotMatr (double m[3][3], double fi, double psi, double theta)
**
**  fi    - rotation in (object) XY plane 
**  psi   - rotation in (object) XZ plane (arond Y)
**  theta - rotation in (object) YZ plane (arond Z)   
**
**  build transfer matrix
**  rotations are applying in order fi, psi, theta.
**
**  So, it might be treated as if V is a vector from the origin to a viewer
**  and fi and psi its sphere coordinates angles, the object should be
**  rotated by -fi, psi. Theta actually defines rotation in the window plane
**  (v1, v2) plane.
**
*/

void makeRotMatr (double m[3][3], double fi, double psi, double theta)
{
    m[0][0] = 1.0; m[0][1] = 0.0; m[0][2] = 0.0; 
    m[1][0] = 0.0; m[1][1] = 1.0; m[1][2] = 0.0; 
    m[2][0] = 0.0; m[2][1] = 0.0; m[2][2] = 1.0; 
    ViewRotZ(fi,m);
    ViewRotY(psi,m); 
    ViewRotX(theta,m); 
    MultM(objToWin,m,m);            	
}

/*
**  makeDragRotMatr(double m[3][3], dvector3  v1, dvector3  v2)
**
**  generate a rotation matrix which transfer 
**  normalized v1 to normalized v2 and does not
**  affect a vector which is ortogonal to both v1 and v2.
**  So, the rotation is generated along big circle through projections of
**  v1 and v2 to a sphere, i.e. rotation axis is ortogonal to the 
** 
*/
void makeDragRotMatr(double m[3][3], dvector3  v1, dvector3  v2)
{
    dvector3 v3;
    double prj_v1_v2 [3][3];
    double embed_v1_v2 [3][3];
    double rot_v1_v2 [3][3];
    double v2_v1;
    double v2_ort_v1;
    int i,j;
    
    m[0][0] = 1.0; m[0][1] = 0.0; m[0][2] = 0.0; 
    m[1][0] = 0.0; m[1][1] = 1.0; m[1][2] = 0.0; 
    m[2][0] = 0.0; m[2][1] = 0.0; m[2][2] = 1.0; 
    v1 = dsetLength3(v1, 1.0);
    v2 = dsetLength3(v2, 1.0);
    v3 = dsubVectors3(v1,v2);
    if (dvectorLength3(v3) < ROT_DELTA) return;
    v2_v1 = dprojectionLength3(v2,v1);
    v3 = dsubVectors3(v2, dsetLength3(v1,v2_v1));
    v2_ort_v1 = dvectorLength3(v3);
/* normalised vector  in  v1 v2 plane, ortogonal to v2			*/ 
    v3 = dsetLength3(v3, 1.0); 
/* projector from 3D to v1 v2 plane, to a plane coordinates !!!		*/
    prj_v1_v2[0][0] = v1.x; prj_v1_v2[0][1] = v1.y; prj_v1_v2[0][2] = v1.z; 
    prj_v1_v2[1][0] = v3.x; prj_v1_v2[1][1] = v3.y; prj_v1_v2[1][2] = v3.z; 
    prj_v1_v2[2][0] = 0.0;  prj_v1_v2[2][1] = 0.0;  prj_v1_v2[2][2] = 0.0; 
/* embedding plane to 3D (transpose matrix)				*/
    for(i=0; i<3; i++)
        for(j=0;j<3;j++)
            embed_v1_v2[i][j] = prj_v1_v2[j][i];
/* build a rotation in the plane v1 v2 whcih transfer v1 to v2		*/           
    rot_v1_v2[0][0] = v2_v1;     rot_v1_v2[0][1] = -v2_ort_v1; rot_v1_v2[0][2] = 0.0; 
    rot_v1_v2[1][0] = v2_ort_v1; rot_v1_v2[1][1] = v2_v1;      rot_v1_v2[1][2] = 0.0; 
    rot_v1_v2[2][0] = 0.0;       rot_v1_v2[2][1] = 0.0;        rot_v1_v2[2][2] = 0.0; 
/* assemble all, it's easy to proof that requested m is
   m = embed_v1_v2 * rot_v1_v2 * prj_v1_v2 + I - embed_v1_v2,m,m * prj_v1_v2 */
   MultM(rot_v1_v2,prj_v1_v2,m);
   MultM(embed_v1_v2,m,m);
/* projector to v1 v2 in 3D (real projector in 3D)			*/
   MultM(embed_v1_v2,  prj_v1_v2, prj_v1_v2);
/* add the projector to ortogonal 1d space (i.e., add I and subtract the
   projector to   v1, v2)			 			*/

    for(i=0; i<3; i++)
        m[i][i] += 1.0;
    for(i=0; i<3; i++)
        for(j=0;j<3;j++)
             m[i][j] -= prj_v1_v2 [i][j];
    mcheck(m);        	 
}
/*
makeAngles ()
{
    double wintToObj[3][3];
    double matr[3][3];
    int i,j;
    for(i=0; i<3; i++)
        for(j=0;j<3;j++)
             wintToObj[i][j] -= objToWin [j][i];
    MultM (wintToObj,m,matr);            


}    
*/
/*
**  makeDragRotMatrWLock(double m[3][3], dvector3  v1, dvector3  v2,
**    dvector3 tst, dvector3 index, double lock)
**
**  Try generate a rotation matrix which transfer 
**  normalized v1 to normalized v2 and does not
**  affect a vector which is ortogonal to both v1 and v2.
**  So, the rotation is generated along big circle through projections of
**  v1 and v2 to a sphere, i.e. rotation axis is ortogonal to the 
**  (v1, v2) plane.
**  If the generated rotation satisfy condition
**
**      (index, m * tst) > lock
**
**  where ( , ) scalar  product of vetors and * - matrix by vector multiplication,
**  keep this rotation and return.
**
**  Otherwise keep the axis of the rotation, but make the angle smaler, so
**      (index, m * tst) == lock
**
**
**  IMPORTANT: assume (index,tst) >= lock on entry
*/
void makeDragRotMatrWLock(double m[3][3], dvector3  v1, dvector3  v2,
  dvector3 tst, dvector3 index, double lock)
{
    dvector3 v3;
/* projection to <v1,v2> plane to a palne coordinates			  
   (first two coordinates of a 3D vector is used for representation  	*/
    double p_12_2d [3][3];
/* embedding of a plane to <v1,v2> plane in3D				*/    
    double embed_12 [3][3];
/* "real" projector ot <v1,v2> plane in 3d				*/
    double p_12[3][3];
/* projector to the vector ortogonal <v1,v2> plane in 3d		*/    
    double p_ort_12[3][3];
    double rot_12 [3][3];
    double v2_v1;
    double v2_ort_v1;
    dvector3 p_index, p_tst, tp_tst, cor_tp_tst, lk_point, lk_dir;
    int sign;
    int i,j;
    
    m[0][0] = 1.0; m[0][1] = 0.0; m[0][2] = 0.0; 
    m[1][0] = 0.0; m[1][1] = 1.0; m[1][2] = 0.0; 
    m[2][0] = 0.0; m[2][1] = 0.0; m[2][2] = 1.0; 
    v1 = dsetLength3(v1, 1.0);
    v2 = dsetLength3(v2, 1.0);
    v3 = dsubVectors3(v1,v2);
    if (dvectorLength3(v3) < ROT_DELTA) return;
    v2_v1 = dprojectionLength3(v2,v1);
    v3 = dsubVectors3(v2, dsetLength3(v1,v2_v1));
    v2_ort_v1 = dvectorLength3(v3);
/* normalised vector  in  v1 v2 plane, ortogonal to v2			*/ 
    v3 = dsetLength3(v3, 1.0); 
/* projector from 3D to v1 v2 plane, to a plane coordinates !!!		*/
    p_12_2d[0][0] = v1.x; p_12_2d[0][1] = v1.y; p_12_2d[0][2] = v1.z; 
    p_12_2d[1][0] = v3.x; p_12_2d[1][1] = v3.y; p_12_2d[1][2] = v3.z; 
    p_12_2d[2][0] = 0.0;  p_12_2d[2][1] = 0.0;  p_12_2d[2][2] = 0.0; 
/* embedding plane to 3D (transpose matrix)				*/
    for(i=0; i<3; i++)
        for(j=0;j<3;j++)
            embed_12[i][j] = p_12_2d[j][i];
/* projector to v1 v2 in 3D (real projector in 3D)			*/
   MultM(embed_12,  p_12_2d, p_12);
/* projector to the vector ortogonal <v1,v2> plane in 3d		*/
    for(i=0; i<3; i++)
        for(j=0;j<3;j++)
             p_ort_12[i][j] = -p_12 [i][j];
    for(i=0; i<3; i++)
        p_ort_12[i][i] += 1.0;
        
/* build a rotation in the plane v1 v2 whcih transfer v1 to v2		*/           
    rot_12[0][0] = v2_v1;     rot_12[0][1] = -v2_ort_v1; rot_12[0][2] = 0.0; 
    rot_12[1][0] = v2_ort_v1; rot_12[1][1] = v2_v1;      rot_12[1][2] = 0.0; 
    rot_12[2][0] = 0.0;       rot_12[2][1] = 0.0;        rot_12[2][2] = 0.0; 

/* start lock condition checking					*/
   lock -= dscalarProduct3(index,dtransformVector3(p_ort_12,tst));

/* project index and tst to 2d						*/
    p_index = dtransformVector3(p_12_2d,index);
    p_tst = dtransformVector3(p_12_2d,tst);
    tp_tst = dtransformVector3(rot_12,p_tst);
   
/* check lock condition							*/
    if (dscalarProduct3(p_index,tp_tst) >= lock)
    {
/* rotation is OK, build it in 3D					
   m = embed_12,m,m * rot_12 * p_12_2d + p_ort_12 			*/
       MultM(rot_12,p_12_2d,m);
       MultM(embed_12,m,m);
	for(i=0; i<3; i++)
            for(j=0;j<3;j++)
        	 m[i][j] += p_ort_12[i][j];
	mcheck(m);        	 
    }
    else 
    {          	 
/* correction is required						*/
        lk_point = dmulVector3(p_index,lock / dscalarProduct3(p_index,p_index));
/* ortogonal to p_index in the plane					*/
        lk_dir.x =  p_index.y;
        lk_dir.y = -p_index.x;
        lk_dir.z = 0;
        sign = dscalarProduct3(lk_dir,tp_tst) >= 0 ? 1 : -1;
        cor_tp_tst = drestoreVectorByLen(lk_point,lk_dir,
          dscalarProduct3(p_tst,p_tst), sign);
/* go out of troubles now						*/          
        makeDragRotMatr(m, dtransformVector3(embed_12,p_tst),
          dtransformVector3(embed_12,cor_tp_tst)); 
    }
}    	          

/*
**  dvector3 drestoreVectorByLen(dvector3 point, dvector3
**   viewDirection, double len2, int sign)
**
** 
**
** point - point in the projection to the window, which is situated
**	   in the projection of the face
** viewDirection - view direction through the point in 3D (might be
**	   defferent in case of perspective usage)
** len2	 - square of the requested length to restore to.
** sign	 - selector to choose between two roots, if any, see below
**         (Should mot be zero to have a choice making sence)
**
** Restoration routine for use in parallelepiped motion control
** The result is looking in a form:
**
** result = <embedded to 3d> point + factor * viewDirection,
** length(result) = sqrt(len2);
**
** where factor is unknown. If there are two solution, the one with
** the biggest Z coordinate is chosen if the sign >= 0 and
** with the lowest otherwise
**
** If there are no solution, returns the one with factor==0, which
** correspond to one of motion lock conditions.
**   
*/
dvector3 drestoreVectorByLen(dvector3 point, dvector3
  viewDirection, double len2, int sign)
{
    double r2, vd2, rv, factor1, factor2;
    double discr;
    r2 = dscalarProduct3(point,point);
    vd2 = dscalarProduct3(viewDirection,viewDirection);
    rv = dscalarProduct3(point,viewDirection); 
    discr = rv * rv - 4.0 * vd2 * (r2 - len2);
    if (discr < 0)
        return(point);
    factor1 = (- rv - sqrt(discr))/ vd2 / 2.0;
    factor2 = (- rv + sqrt(discr))/ vd2 / 2.0;
    if(factor1  * sign >= factor2 * sign)
        return(daddVectors3(point,dmulVector3(viewDirection,factor1)));
    else
        return(daddVectors3(point,dmulVector3(viewDirection,factor2)));                
}

/*
**  dvector3 restoreVectorByLen(vector3 point, vector3
**   viewDirection, double len2, int sign)
**
** 
**
** point - point in the projection to the window, which is situated
**	   in the projection of the face
** viewDirection - view direction through the point in 3D (might be
**	   defferent in case of perspective usage)
** len2	 - square of the requested length to restore to.
** sign	 - selector to choose between two roots, if any, see below
**         (Should mot be zero to have a choice making sence)
**
** Restoration routine for use in parallelepiped motion control
** The result is looking in a form:
**
** result = <embedded to 3d> point + factor * viewDirection,
** length(result) = sqrt(len2);
**
** where factor is unknown. If there are two solution, the one with
** the biggest Z coordinate is chosen if the sign >= 0 and
** with the lowest otherwise
**
** If there are no solution, returns the one with factor==0, which
** correspond to one of motion lock conditions.
**   
*/
dvector3 restoreVectorByLen(vector point, vector3
  viewDirection, double len2, int sign)
{
    dvector3 dVD;
    dvector3 restored;
    dVD = doubleVector3(viewDirection);
    restored.x = point.x;
    restored.y = point.y;
    restored.z = 0.0;
    return(drestoreVectorByLen(restored,dVD, len2, sign));
}    

static void mcheck (double m[3][3])
{
    int i, j;
    double m1 [3][3];
    double m2 [3][3];
    double mx;
    for (i=0;i<3;i++)
        for(j=0; j<3;j++)
            m1[i][j] = m[j][i];
    MultM(m1,m,m2);
    mx = 0.0;
    for (i=0;i<3;i++)
        if(fabs(1.0-m2[i][i]) > mx)
            mx = fabs(1.0-m2[i][i]);
    for (i=0;i<3;i++)
        for(j=i+1; j<3;j++)
            if(fabs(m2[i][j]) > mx)            
                mx = fabs(m2[i][j]);
    if (mx > 0.001)
    {
       i = j;
    }
}                    

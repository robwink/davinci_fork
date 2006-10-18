#ifndef _3DGEOM_H
#define _3DGEOM_H
/* sphere coordinate vector						*/
typedef struct _spVector {
    double fi;
    double psi;
    double radius;
} spVector;

/* some 3D stuff 				*/
/* vector of 3d vector space 			*/
typedef struct _vector3 {
    int x;
    int y;
    int z;
} vector3;

typedef struct _dvector3 {
    double x;
    double y;
    double z;
} dvector3;

vector3 makeVector3(int x, int y, int z);
vector vector2of3(vector3 v);
dvector3 dmakeVector3(double x, double y, double z);
dvector3 doubleVector3(vector3 v);
vector3 mulVector3(vector3 v, int i);
dvector3 dmulVector3(dvector3 v, double i);
vector3 divVector3(vector3 v, int i);
dvector3 ddivVector3(dvector3 v, double i);
vector3 addVectors3(vector3 v1, vector3 v2);
dvector3 daddVectors3(dvector3 v1, dvector3 v2);
vector3 subVectors3(vector3 v1, vector3 v2);
dvector3 dsubVectors3(dvector3 v1, dvector3 v2);
int scalarProduct3 (vector3 v1, vector3 v2);
double dscalarProduct3 (dvector3 v1, dvector3 v2);
double projectionLength3(vector3 v1, vector3 v2);
double dprojectionLength3(dvector3 v1, dvector3 v2);
double dvectorLength3(dvector3 v);
double vectorLength3(vector3 v);
double dvectorLength3(dvector3 v);
dvector3 dsetLength3(dvector3 v, double l);
dvector3 dortogonalize3To1(dvector3 dv1, dvector3 dv2);
dvector3 dortogonalize3To2(dvector3 dv1, dvector3 dv2, dvector3 dv3);
vector3 ortogonal3To2(vector3 dv1, vector3 dv2);
dvector3 dortogonal3To2(dvector3 dv1, dvector3 dv2);
dvector3 dtransformVector3(double m[3][3],dvector3 v);

#endif    

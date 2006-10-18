#ifndef _3DROT_C
#define _3DROT_C
void makeRotMatr (double m[3][3], double fi, double psi, double theta);
void makeDragRotMatr(double m[3][3], dvector3  v1, dvector3  v2);
void makeDragRotMatrWLock(double m[3][3], dvector3  v1, dvector3  v2,
  dvector3 tst, dvector3 index, double lock);
dvector3 drestoreVectorByLen(dvector3 point, dvector3
  viewDirection, double len2, int sign);
dvector3 restoreVectorByLen(vector point, vector3
  viewDirection, double len2, int sign);

#endif

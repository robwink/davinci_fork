#include <X11/Intrinsic.h> 
#include <stdio.h>
#include "2DGeom.h"
#include "3DGeom.h"
#include "3DPara.h"

static void parShift(paraVertex *vertex, paraEdge *edge, paraFace *face, 
  int *verticesNo, int *edgesNo, int *facesNo, 
  int *shiftNo, vector3 shift, int ax);
static void makeConvexHull(paraState *ps);
static void makeLabledEdge(paraState *ps);
static int min3AnagleVector(vector3 vect[3], int shiftNo[3]);
static int minAnagleVector(vector3 vect[2], int shiftNo[2]);
static dvector3 makeOrt(paraState *ps, paraFace *face);
static void markBackFrontObjects(paraState *ps);
static void drawEdges (Display *d, Drawable dr, GC gc, 
  paraState *ps, paraEdge *edge);
static void makePSEdges (paraState *ps, paraEdge *edge, FILE *psfile );



static void parShift(paraVertex *vertex, paraEdge *edge, paraFace *face, 
  int *verticesNo, int *edgesNo, int *facesNo, 
  int *shiftNo, vector3 shift, int ax)
{
    int i, j, vn, en, fn;
/* make parallel shift-and-copy of existent faces			*/
    for (i = 0, fn = *facesNo; i < *facesNo; i++, fn++)
    {
        face[fn] = face[i];
/* correct links 							*/
 	face[fn].vertex[0] += *verticesNo;
 	face[fn].vertex[1] += *verticesNo;
 	face[fn].vertex[2] += *verticesNo;
 	face[fn].vertex[3] += *verticesNo;
 	face[fn].edge[0] += *edgesNo;
 	face[fn].edge[1] += *edgesNo;
 	face[fn].edge[2] += *edgesNo;
 	face[fn].edge[3] += *edgesNo;
 	
    }    
/* make parallel shift-and-copy of existent edges and add faces	*/    
    for (i = 0, en = *edgesNo;
      i < *edgesNo; i++, en++, fn++)
    {
        edge[en] = edge[i];
/* create a new face     						*/
        face[fn].vertex[0] = edge[en].vertex[0];
        face[fn].vertex[1] = edge[en].vertex[1]; 
/* referencies to existent and future edges				*/
        face[fn].edge[0] = i;
        face[fn].edge[1] = en;
        face[fn].edge[2] =  2 * (*edgesNo) + face[fn].vertex[0];
        face[fn].edge[3] =  2 * (*edgesNo) + face[fn].vertex[1];      
/* change links to shifted vertices, the idex difference for
   vertex and its shifted image in the array is always *verticesNo	*/
        edge[en].vertex[0] += *verticesNo;
        edge[en].vertex[1] += *verticesNo; 
        face[fn].vertex[2] = edge[en].vertex[0];
        face[fn].vertex[3] = edge[en].vertex[1];         
    }
/* make parallel shift of copies of pre-existent vertices and 
   add new edges							*/    
    for (i = 0, vn = *verticesNo; 
      i <*verticesNo; i++, vn ++, en++)
    {
/* create new vertex - parallel transfer image for one which exist	*/
        vertex[vn] = vertex[i];
        vertex[vn].point = addVectors3(vertex[vn].point, shift);
/* correct links to edges - move them to new shifted edges,
   the idex difference for the edge and its shifted image in the 
   array is always *edgesNo						*/
        for (j = 0; j < *shiftNo; j++)
          vertex[vn].edge[j] += *edgesNo;          
/* create a new edge - link between source vertex and transferred image */
        vertex[i].edge[*shiftNo] = 
          vertex[vn].edge[*shiftNo] = en; 
        edge[en].vertex[0] = i;
        edge[en].vertex[1] = vn;
        edge[en].aIdx = ax;
    }
    *verticesNo = vn;
    *edgesNo = en;
    *facesNo = fn;
    *shiftNo += 1;
}

void buildPara(paraState *ps, vector3 center,
  vector3 shiftX_2, vector3 shiftY_2, vector3 shiftZ_2, 
  int axX, int axY, int axZ)
{
    int vn, en, fn, sn;
    vector3 shiftX, shiftY, shiftZ;
    ps->center = center;
/* start from the only vertex						*/    
    shiftX = mulVector3(shiftX_2,2);
    shiftY = mulVector3(shiftY_2,2);
    shiftZ = mulVector3(shiftZ_2,2);
    ps->vertex[0].point = subVectors3(subVectors3(subVectors3(center,
      shiftX_2),shiftY_2),shiftZ_2);
    vn = 1;
    en = 0;
    sn = 0;
    fn = 0;
/* and shift the object three times along each of axes			*/
    parShift(ps->vertex, ps->edge, ps->face, &vn, &en, &fn, &sn, shiftX,  axX);
    parShift(ps->vertex, ps->edge, ps->face, &vn, &en, &fn, &sn, shiftY,  axY);
    parShift(ps->vertex, ps->edge, ps->face, &vn, &en, &fn, &sn, shiftZ,  axZ);
    makeConvexHull(ps);
    markBackFrontObjects(ps);
    makeLabledEdge(ps);
}    

void destroyPara(paraState *ps)
{
    if (ps == NULL) return;
    XtFree((char*)ps);
}        

/*
** static void makeConvexHull(paraState *ps)
** 
** paraState *ps    parallelepiped geometry description structure to process
**
** Create <boundaryEdge>, <labeldEdge>, <unlabeledEdge> lists 
**
** Choose parallelepiped vertices and edges which form a boundary 
** (2D convex hull) of the parallelepiped projection from the all 
** parallelepiped edges. 
** Edges with 2D projection length 0 are dropped, so the convex hall 
** can consit of 0, 2, 4 or 6 edges.
** Each chosen vertex is supplied with a pointer to the edge wich
** connect it to the next vertex in the boundary UNLESS this edge
** has 0 length and dropped from the edge list. In the last case
** corresponding pointer setted up to NULL.
** If two of source parallelepiped edges are projected into the same line,
** the edge which is closer to a viewer will be taken. 
** Edges then linked into a ring countclokwise.
** The <boundaryEdge> pointer points to an edge which starts from the leftmost
** (exposed) vertex and go two the next vertex countclockwise. If there are 
** two leftmost verteces, the edge between will be chosen as a start.
** If there are three leftmost verteces, (two exposed vertical 
** edges with the same window X, the lower (bigger Y) will be chosen
**
*/
static void makeConvexHull(paraState *ps)
{
/* the processing is based on the assumption, that it has a parallelotop
   as input (which is true in this case)				*/
/* not the most efficient way to solve this problem, but hopfully
   obvious								*/   

    paraEdge *edge;
    paraVertex *vertex;
    paraEdge *edgePtr;
    paraVertex *vertexPtr;
    int shiftNo[3];
    vector3 vect[3];
    int curVertex;
    int startVertex; 
    int curEdge;
    int nextEdge;      
    int minX;
    int minY;
    int minZ;
    int v, vn;
    int i,j;
    
    if (ps == NULL) return;
    edge = ps->edge;
    vertex = ps->vertex;
    
/* choose letfmost vertex						*/
    for (minX = vertex[0].point.x, i=0; i < 8; i++)
        if (vertex[i].point.x < minX)
            minX = vertex[i].point.x;
            
/* the next to moves (to minY and minZ) are needed to assure that
   first chosen vertex is really visible, but not hidded by any
   of parallelepiped edges. (It's evedent, that it cannot be hide by 
   parallelepiped interior or by any parallelepiped faces).
   It's not very difficult to proof, that finally we'll
   get what we want							*/ 
              
/* go to the vertex with minY, if there are several with x == minX	*/
    for (i=0; i < 8; i++)
        if (vertex[i].point.x == minX) break;
    for (minY = vertex[i].point.y ; i < 8; i++)
        if (vertex[i].point.x == minX && vertex[i].point.y < minY ) 
             minY = vertex[i].point.y;

/* now go to minZ by the similar way					*/        
    for (i=0; i < 8; i++)
        if (vertex[i].point.x == minX && vertex[i].point.y == minY) break;
    for (minZ = vertex[i].point.z ; i < 8; i++)
        if (vertex[i].point.x == minX && vertex[i].point.y == minY &&
          vertex[i].point.z < minZ)
            minZ = vertex[i].point.z;

/* get the index of chosen vertex					*/       
    for (i=0; i < 8; i++)
        if (vertex[i].point.x == minX && vertex[i].point.y == minY
          && vertex[i].point.z == minZ) break;
    
    curVertex = i;
    startVertex = i;       
    ps->boundaryVertex = &(vertex[i]);
    vertexPtr = ps->boundaryVertex;      

/* choose the first edge						*/
/* build edge vectors to compare					*/
    for (i = 0; i<3; i++)
    {
        nextEdge = vertex[curVertex].edge[i];
	if( edge[nextEdge].vertex[0] == curVertex)
           v=edge[nextEdge].vertex[1];
	else
           v=edge[nextEdge].vertex[0];
	shiftNo[i] = i;
	vect[i] = subVectors3(vertex[v].point, vertex[curVertex].point);
    }	
    if (vect[0].x != 0 || vect[0].y != 0 || vect[1].x != 0 || vect[1].y != 0
      || vect[2].x != 0 || vect[2].y != 0)
    {  
/* choose the vector							*/
	vn = min3AnagleVector(vect, shiftNo);
	curEdge = vertex[curVertex].edge[shiftNo[vn]];
/* and make insertion into boundary edge and vertex lists		*/
        edgePtr = ps->boundaryEdge = 
        vertexPtr->bndEdge = &(edge[curEdge]);
	if( edge[curEdge].vertex[0] == curVertex)
           v=edge[curEdge].vertex[1];
	else
           v=edge[curEdge].vertex[0];
        curVertex = v;	
	vertexPtr->nextBnd = &(vertex[v]); 
	vertexPtr = vertexPtr->nextBnd;
    }
    else
    {
/* singular case 							*/
/* there is  no parallelepiped, just a point in the projection 			*/
/* init lists and return					      	*/
	ps->boundaryVertex = NULL;
	ps->boundaryEdge = NULL;
	return;
    }
/* continue to build boundary (general case)				*/    
    do
    {
/* get edges and build vectors							*/
	for (i = 0, j = 0; i<3; i++)
	{
            nextEdge = vertex[curVertex].edge[i];
	    if (nextEdge != curEdge)
	    {
		if( edge[nextEdge].vertex[0] == curVertex)
        	   v=edge[nextEdge].vertex[1];
		else
        	   v=edge[nextEdge].vertex[0];
		shiftNo[j] = i;
		vect[j] = subVectors3(vertex[v].point, vertex[curVertex].point);
		j++;
	    }
	}	
/* compare vectors by angle						*/
	if (vect[0].x != 0 || vect[0].y != 0 || 
	  vect[1].x != 0 || vect[1].y != 0)
	{
/* choose the vector							*/
	    vn = minAnagleVector(vect, shiftNo);
	    curEdge = vertex[curVertex].edge[shiftNo[vn]];
/* and make insertion into boundary edge and vertex lists		*/
            edgePtr->nextBnd = 
            vertexPtr->bndEdge = &(edge[curEdge]);
            edgePtr = edgePtr->nextBnd;
	    if( edge[curEdge].vertex[0] == curVertex)
               v=edge[curEdge].vertex[1];
	    else
               v=edge[curEdge].vertex[0];
            curVertex = v;	
	    vertexPtr->nextBnd = &(vertex[v]); 
	    vertexPtr = vertexPtr->nextBnd;
	}
	else
	{
/* singular case 							*/
/* go by the edge with lowerst shiftNo but don't insert zero length 
   edge into edge lists							*/
	    curEdge = vertex[curVertex].edge[shiftNo[0]];
/* and make insertion into boundary edge and vertex lists		*/
            vertexPtr->bndEdge = NULL;
	    if( edge[curEdge].vertex[0] == curVertex)
               v=edge[curEdge].vertex[1];
	    else
               v=edge[curEdge].vertex[0];
            curVertex = v;	
	    vertexPtr->nextBnd = &(vertex[v]); 
	    vertexPtr = vertexPtr->nextBnd;
            
	}
    }	
    while (curVertex != startVertex);
    while (vertexPtr->bndEdge == NULL)
        vertexPtr = vertexPtr->nextBnd;
    edgePtr->nextBnd = vertexPtr->bndEdge;    
/* got two linked rings  of vertices and of edges, with references 
   from vertices to edges						*/
}              

/*
**
** static void makeLabledEdge(paraState *ps)
**
** ps 	- pointer to a paraState sttructure
**
** On entry it is asserted that <ps> is processed by <makeConvexHull>
** and/or <boundaryEdge> and <boundaryVertex> lists are biult and correct
**
** Creates <labeledEdge> and <unLabeledEdge> lists
*/
static void makeLabledEdge(paraState *ps)
{
    paraEdge **pe;
    paraEdge *edge;
    paraVertex *vert, *svert;
    int i,j;
    edge = ps->boundaryEdge;
    if (edge == NULL)
    {
        ps->labeledEdge = NULL;
        ps->unLabeledEdge = NULL;
        return;
    }        
/* count number of boundary edges				*/
    i=0;
    do
    {
        i++;
        edge = edge->nextBnd;
    }
    while (edge != ps->boundaryEdge);
/* choose start vertex						*/
    vert = ps->boundaryVertex;
    svert = NULL;
    do
    {
         if (vert->point.y < ps->boundaryVertex->point.y &&
           vert->bndEdge != NULL)
             svert = vert;
             vert = vert->nextBnd;
    }
    while (vert != ps->boundaryVertex);
    if (svert == NULL)
    {
        svert = vert;
        while (svert->bndEdge == NULL)
            svert = svert->nextBnd;
/* because <boundaryEdge> list non empty, svert will be found	*/            
    }
    edge = svert->bndEdge;
    j = i/2;
    pe = &(ps->labeledEdge);
    for (j = i/2; j >0 ; j--)
    {
        *pe = edge;
        pe = &(edge->next);
        edge = edge->nextBnd;
    }
    *pe = NULL;
    pe = &(ps->unLabeledEdge);
    for (j = i/2; j >0 ; j--)
    {
        *pe = edge;
        pe = &(edge->next);
        edge = edge->nextBnd;
    }
    *pe = NULL;
}        
                 
         



static int min3AnagleVector(vector3 vect[3], int shiftNo[3])
{
    int i;
    int sh[2];
    vector3 vt[2];
    if (vect[0].x == 0 && vect[0].y == 0 && vect[1].x == 0 && vect[1].y == 0)
        return(2);
    sh[0] = shiftNo[0];
    sh[1] = shiftNo[1];
    vt[0] = vect[0];
    vt[1] = vect[1];	
    i = minAnagleVector(vt, sh);
    sh[0] = shiftNo[i];
    sh[1] = shiftNo[2];
    vt[0] = vect[i];
    vt[1] = vect[2];
    if (minAnagleVector(vt, sh) == 0) 
        return(i);
    else
        return(2);
}	
	    
/* don't use both vectors with zero projection				*/
static int minAnagleVector(vector3 vect[2], int shiftNo[2])
{
/* assume that at least one of the vectors have non-zero projectoin 
   to XY plane								*/
    if (vect[0].x == 0 && vect[0].y == 0)
       return(1);
    if (vect[1].x == 0 && vect[1].y == 0)
       return(0);
    if (vect[0].x * vect[1].y < vect[0].y * vect[1].x)
        return(0);
    else if (vect[0].x * vect[1].y > vect[0].y * vect[1].x)
        return(1);
/* both vectors have exacttly the same direction in projection, try to
   separate by z							*/
    else if ( (vect[0].x * vect[1].x + vect[0].y * vect[1].y) * vect[1].z <
      (vect[1].x* vect[1].x + vect[1].y * vect[1].y) * vect[0].z)
        return(1);
    else if ( (vect[0].x * vect[1].x + vect[0].y * vect[1].y) * vect[1].z >
      (vect[1].x * vect[1].x + vect[1].y * vect[1].y) * vect[0].z)
        return(0);
/* vectors are colinear, choose the shortest one to have it well defined
   (essential to keep the symmetry of cnvex hull constructed)		*/
    else if(vect[1].x * vect[1].x + vect[1].y * vect[1].y + 
      vect[1].z * vect[1].z < vect[0].x * vect[0].x + vect[0].y * vect[0].y + 
      vect[0].z * vect[0].z)
        return(1);
    else if(vect[1].x * vect[1].x + vect[1].y * vect[1].y + 
      vect[1].z * vect[1].z > vect[0].x * vect[0].x + vect[0].y * vect[0].y + 
      vect[0].z * vect[0].z)
        return(0);
/* vectors are equal, to keep structural simmetry, we probably should
   choose the one, which has lower shift index (a little complicated
   because data structure does not support this properties explicitly   */        
    else if (shiftNo [1] < shiftNo[0])
        return(1);
    else
        return(0);
}        

/*
**  paraFace *pointInFace (paraState *ps, paraFace *faceList, vector point)
**
**  ps 		- parallelepiped state
**  faceList 	- pointer to a list of parallelogram faces to check
**  point	- point in the window
**
**  check if <point> is situated in one of the faces projection to
**  the window for the faces from the faceList.
**  If it is, returns a pointer to a first face in the list the point
**  is situated in.
**  Returns NULL otherwise
*/
paraFace *pointInFace (paraState *ps, paraFace *faceList, vector point)
{
    vector vertex;
    vector v1;
    vector v2;
    while (faceList != NULL)
    {
        vertex = vector2of3 (ps->vertex[faceList->vertex[0]].point);
        v1 = vector2of3 (ps->vertex[faceList->vertex[1]].point);
        v2 = vector2of3 (ps->vertex[faceList->vertex[2]].point);
        if (pointInPara(vertex, subVectors(v1, vertex), subVectors(v2, vertex),
          point))
            break; 
        faceList = faceList->next;    
    }
    return (faceList);   
}
/*
** dvector3 restoreVector(paraState *ps, paraFace *face, vector point)
** void restoreVector(paraState *ps, paraFace *face, vector point, vector3
**   viewDirection, dvector3 *restored, dvector3 *ort)
**
** ps	 - pointer to a paraState structure
** face  - pointer to a face of the cube in the SAME *ps structure
** point - point in the projection to the window, which is situated
**	   in the projection of the face
** viewDirection - view direction through the point in 3D (might be
**	   defferent in case of perspective usage)
**
** *restored - output only - restored 3D vector on the face (from the 
**	   center of the parallelepid) which projected to the point.
** *ort  - output only - ort to the face
**/
void restoreVector(paraState *ps, paraFace *face, vector point, vector3
  viewDirection, dvector3 *restored, dvector3 *ort)
{
    dvector3 dVD;
    double ortLen2;
    double f, df;
    dVD = doubleVector3(viewDirection);
    *ort = makeOrt(ps, face);
    ortLen2 = dscalarProduct3(*ort, *ort);
    (*restored).x = point.x;
    (*restored).y = point.y;
    (*restored).z = 0.0;
    *restored = dsubVectors3(*restored, doubleVector3(ps->center));
    f = dscalarProduct3(*restored,*ort);
    df = dscalarProduct3(dVD,*ort);
    if (df != 0)
        *restored = daddVectors3(*restored,
          dmulVector3(dVD,(ortLen2 - f) /df));
}
    



/*
** dvector3 restoreVector(paraState *ps, paraFace *face, vector point)
**
** ps	 - pointer to a paraState structure
** face  - pointer to a face of the cube in the SAME *ps structure
** point - point in the projection to the window, which is situated
**	   in the projection of the face
**
** Return restored 3D vector on the face (from the center of the parallelepiped
** - assumed origin) and projected to the point  
**/
/*
dvector3 restoreVector(paraState *ps, paraFace *face, vector point)
{
    dvector3 ort;
    dvector3 restored;
    double ortLen2;
    double f;
    ort = makeOrt(ps, face);
    ortLen2 = dscalarProduct3(ort, ort);
    restored.x = point.x;
    restored.y = point.y;
    restored.z = 0.0;
    restored = dsubVectors3(restored, doubleVector3(ps->center));
    f = dscalarProduct3(restored,ort);
    if (ort.z != 0.0)
        restored.z = (ortLen2 - f) / ort.z;
    return (restored);
}
*/


/*
** static dvector3 makeOrt(paraState *ps, paraFace *face)
**
** ps	- pointer to a paraState structure
** face - pointer to a face of the cube in the SAME *ps structure
**
** return a vector (from the center of the parallelepiped - assumed origin)
** to a point of the face plane, ortogonal to this plane.
** The returned vector is inside of the face itself)
*/
static dvector3 makeOrt(paraState *ps, paraFace *face)
{
    dvector3 planeVec1, planeVec2, vectorToPlane, planeOrtVector;
    double l;
    planeVec1 = doubleVector3(subVectors3(ps->vertex[face->vertex[1]].point, 
      ps->vertex[face->vertex[0]].point));
    planeVec2 = doubleVector3(subVectors3(ps->vertex[face->vertex[2]].point, 
      ps->vertex[face->vertex[0]].point));
    vectorToPlane = doubleVector3(subVectors3(ps->vertex[face->vertex[0]].point, 
      ps->center));
/* build an ort								*/      
    planeOrtVector = dortogonal3To2(planeVec1, planeVec2);
    if ((l=dvectorLength3(planeOrtVector))!=0)
        return(dsetLength3(planeOrtVector, 
          dscalarProduct3(planeOrtVector,vectorToPlane) / l));
    else
        return(planeOrtVector);          
/*   alternative variant of ort building, hopefully the above has more
**   accuracy, but this never was checked     
**    return (dortogonalize3To2(planeVec1, planeVec2, vectorToPlane));
*/
}

/*
** markBackFrontObjects(paraState *ps)
** 
** ps	- pointer to a paraState structure
**
** parse the *ps and create <frontFace>, <backFace>, <frontEdge>, <backEdge>
** lists.
*/
static void markBackFrontObjects(paraState *ps)
{
    vector3 planeVec1, planeVec2, ort, vertex;
    paraFace **pff, **pfb;
    paraFace *face;
    paraEdge **pe;
    int s;
    int edgesCount[12];
    int i;
    pff = &ps->frontFace;
    pfb = &ps->backFace;
    for (i = 0, face = ps->face; i<6; i++, face++)
    {
        vertex = ps->vertex[face->vertex[0]].point;
	planeVec1 = subVectors3(ps->vertex[face->vertex[1]].point,vertex); 
	planeVec2 = subVectors3(ps->vertex[face->vertex[2]].point,vertex); 
	vertex = subVectors3(ps->center, vertex);
        ort.x = planeVec1.y * planeVec2.z - planeVec1.z * planeVec2.y;
        ort.y = planeVec1.z * planeVec2.x - planeVec1.x * planeVec2.z;
        ort.z = planeVec1.x * planeVec2.y - planeVec1.y * planeVec2.x;
/* product of the ort to plane and the vector from plane to center	*/        
        s = scalarProduct3(vertex,ort);
	if (ort.z != 0)
	{   									        
/* <-ort.z> is a product of the <ort> and (0,0,-1) - vector directed to
   a viewer. So, if <s> and <-ort.z> has the same sign (product positive),
   the face is a back face (invisible) and front face otherwise.
   Zero in <s> means singular case, (is NOT processed yet), ort.z == 0
   means, that exactly an edge of the face is seen, so it is not back face
   neither front face. 
   It is not possible use actual product here because of overflow	*/
            s = (s == 0 ) ? 0 : ( (s < 0) ?  ort.z : -ort.z );
            if (s < 0)
            {
        	*pff = face;
        	pff = &(face->next);
            }
            else if (s > 0)
            {
        	*pfb = face;
        	pfb = &(face->next);
            }
	}
    }
    *pff = NULL;
    *pfb = NULL;
/* create lists of front and back edges 				*/    	
    for (i=0; i<12; i++)
        edgesCount[i] = 0;
    for (face = ps->frontFace; face != NULL; face = face->next)
        for (i=0; i<4; i++)
            edgesCount[face->edge[i]]++;
    pe = &ps->frontEdge;
    for (i=0; i<12; i++)
    {
        if (edgesCount[i] == 2)
        {
            *pe = &(ps->edge[i]);
            pe = &(ps->edge[i].next);
	}
    }
    *pe = NULL;
    for (i=0; i<12; i++)
        edgesCount[i] = 0;
    for (face = ps->backFace; face != NULL; face = face->next)
        for (i=0; i<4; i++)
            edgesCount[face->edge[i]]++;
    pe = &ps->backEdge;
    for (i=0; i<12; i++)
    {
        if (edgesCount[i] == 2)
        {
            *pe = &(ps->edge[i]);
            pe = &(ps->edge[i].next);
	}
    }
    *pe = NULL;	            
}                   

static void drawEdges (Display *d, Drawable dr, GC gc, 
  paraState *ps, paraEdge *edge)
{
    paraEdge *tmp;
    int i;
    XSegment seg[12];
    for( tmp = edge, i = 0; tmp != NULL; tmp = tmp->next, i++)
    {
        seg[i].x1 = ps->vertex[tmp->vertex[0]].point.x;
        seg[i].y1 = ps->vertex[tmp->vertex[0]].point.y;
        seg[i].x2 = ps->vertex[tmp->vertex[1]].point.x;
        seg[i].y2 = ps->vertex[tmp->vertex[1]].point.y;
    }
    if (i==0) return;
    XDrawSegments(d, dr, gc, seg, i);
} 

void drawParaFrontEdges (Display *d, Drawable dr, GC gc, 
  paraState *ps)
{
    if (ps != NULL)
        drawEdges (d, dr, gc, ps, ps->frontEdge);
}
    
void drawParaBackEdges (Display *d, Drawable dr, GC gc, 
  paraState *ps)
{
    if (ps != NULL)
        drawEdges (d, dr, gc, ps, ps->backEdge);
}

void drawParaLabeledEdges (Display *d, Drawable dr, GC gc, 
  paraState *ps)
{
    if (ps != NULL)
        drawEdges (d, dr, gc, ps, ps->labeledEdge);
}

void drawParaUnLabeledEdges (Display *d, Drawable dr, GC gc, 
  paraState *ps)
{
    if (ps != NULL)
      	drawEdges (d, dr, gc, ps, ps->unLabeledEdge);
}

static void makePSEdges (paraState *ps, paraEdge *edge, FILE *psfile )
{
    paraEdge *tmp;
    int i;
    fprintf(psfile,"gsave  						\n");
    if (edge != NULL)
    {
        fprintf(psfile,"newpath						\n");
        for( tmp = edge, i = 0; tmp != NULL; tmp = tmp->next, i++)
        {
            fprintf(psfile,"%d %d moveto			        \n",
              ps->vertex[tmp->vertex[0]].point.x, 
              ps->vertex[tmp->vertex[0]].point.y);		  
            fprintf(psfile,"%d %d lineto			        \n",
              ps->vertex[tmp->vertex[1]].point.x, 
              ps->vertex[tmp->vertex[1]].point.y);		  
    	}
    	fprintf(psfile,"stroke						\n");
    }
    fprintf(psfile,"grestore  						\n");
} 

void makePSParaFrontEdges ( paraState *ps, FILE *psfile)
{
    if (ps != NULL)
        makePSEdges (ps, ps->frontEdge,  psfile);
}
    
void makePSParaBackEdges ( paraState *ps, FILE *psfile)
{
    if (ps != NULL)
        makePSEdges (ps, ps->backEdge,  psfile);
}

void makePSParaLabeledEdges ( paraState *ps, FILE *psfile)
{
    if (ps != NULL)
        makePSEdges (ps, ps->labeledEdge,  psfile);
}

void makePSParaUnLabeledEdges ( paraState *ps, FILE *psfile)
{
    if (ps != NULL)
        makePSEdges (ps, ps->unLabeledEdge,  psfile);
}

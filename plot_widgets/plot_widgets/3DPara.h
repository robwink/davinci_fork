#ifndef _3DPARA_H
#define _3DPARA_H

typedef struct _paraFace {
    int vertex[4];		/* indices of incident vertices in a
    				   vertex array				*/
    int edge[4];		/* indices of incident vertices in a
    				   edge array				*/ 				   
    struct _paraFace *next;	/* link for manipulations in lists	*/    				   
} paraFace;

typedef struct _paraEdge {
    int vertex[2];		/* indices of incident vertices in a
    				   vertex array				*/
    int aIdx;			/* index which might be provided from the
    				   outside for each of axis (directions)
    				   (X,Y,Z) in parallelepiped bulding request.
    				   Allow to determine edge direction in
    				   3D object space, and hence link the
    				   edge with, i.e. data for labeling
    				   routines.				*/
    struct _paraEdge *nextBnd; 	/* pointer to link boundary edges 
    				   into a list				*/
    struct _paraEdge *next;	/* pointer to link labeled, unlabeled
    				   (boundary ones which are not labeled),
    				   front and back edges into a list	*/
} paraEdge;

typedef struct _paraVertex {
    int edge[3];	        /* indices of incident edges in an
    				   edge array				*/
    vector3 point;		/* coordinates after mapping to the 	*/
    				/* window				*/
    struct _paraVertex *nextBnd; /* next vertex in the projection
    				   boundary (convex hull) countclockwise*/
    struct _paraEdge *bndEdge;	/* boudary edge between current and
    				   next vertices in the boundary,
    				   if they are connected by the edge, 
    				   NULL othevise  			*/  				   
    				       				
} paraVertex;

typedef struct _paraState {
    paraVertex vertex[8];
    paraEdge edge[12];
    paraFace face[6];
    vector3 center;
    paraVertex *boundaryVertex;
    paraEdge *boundaryEdge;
    paraEdge *labeledEdge;      /* boundary edges which are labeled 	*/
    paraEdge *unLabeledEdge; 	/* boundary edges which are unlabeled 	*/
    paraEdge *frontEdge;
    paraEdge *backEdge;
    paraFace *frontFace;
    paraFace *backFace;
} paraState;

void buildPara(paraState *ps, vector3 center,
  vector3 shiftX_2, vector3 shiftY_2, vector3 shiftZ_2, 
  int axX, int axY, int axZ);
void destroyPara(paraState *ps);
paraFace *pointInFace (paraState *ps, paraFace *faceList, vector point);
void restoreVector(paraState *ps, paraFace *face, vector point, vector3
  viewDirection, dvector3 *restored, dvector3 *ort);

/* drawing routines							*/
void drawParaFrontEdges (Display *d, Drawable dr, GC gc, 
  paraState *ps);
void drawParaBackEdges (Display *d, Drawable dr, GC gc, 
  paraState *ps);
void drawParaLabeledEdges (Display *d, Drawable dr, GC gc, 
  paraState *ps);
void drawParaUnLabeledEdges (Display *d, Drawable dr, GC gc, 
  paraState *ps);

void makePSParaFrontEdges ( paraState *ps, FILE *psfile);
void makePSParaBackEdges ( paraState *ps, FILE *psfile);
void makePSParaLabeledEdges ( paraState *ps, FILE *psfile);
void makePSParaUnLabeledEdges ( paraState *ps, FILE *psfile);

#endif

#include <X11/Intrinsic.h>
#include <math.h> 
#include <stdio.h>
#include "2DGeom.h"
#include "3DGeom.h"
#include "geometry.h"
#include "3DRot.h"
#include "3DPara.h"
#include "3DRotCube.h"

/* control of cuberotation strategy				*/

static Boolean markInsCubeState(cubeDescr* cd, XEvent *event);
static Boolean markOutCubeState(cubeDescr* cd, XEvent *event);
static void dragInsCube(cubeDescr* cd, XEvent *event, double m[3][3]);
static void dragOutCube(cubeDescr* cd, XEvent *event, double m[3][3]);

/*  cubeDescr* createCubeDescr (vector cubeCenter, int cubeSize, double fi,
**    double psi, double theta, int ax, int ay, int az)
**
**  cubeCenter	- windows coordinates of cube center in the window
**  cubeSize 	_ size of cube edge in pixels (before projection)
**
**  	view angles to set up:
**		  
**  fi		- asimuth angle in XY object plane (from X axis countclockwise)
**  psi	        - polar angle (from XY plane)
**  theta	- atually rotation in the window, countclockwise)
**
**  	see also comments in <3DRot> file
**
**  	indices in axis attribute array (somewhere outside) - for this
**	program it could be arbitrary integes
**	The 3DRotCube routines will support links between cube edges
** 	images and this attributes, so for a given edge image it is
**	possible to get attribute index to get acsess to assotiated
**	information in other modules.
**
**  ax	- see above
**  ay
**  az
**
**  Initialize cube desriptor and build initial cube state
*/  
cubeDescr* createCubeDescr (vector cubeCenter, int cubeSize, double fi,
  double psi, double theta, int ax, int ay, int az)
{
    cubeDescr* cd;
    cd = (cubeDescr*) XtMalloc(sizeof(cubeDescr));
    cd->curCube = NULL;
    cd->newCube = NULL;
    makeRotMatr (cd->rotMatr, fi, psi, theta);
    cd->dragging = FALSE;
    cd->relativeMotion = FALSE;
    cd->bigCircleMotion = FALSE; 
    cd->lockMotionVisible = TRUE; 
    cd->outMotionEnabled = TRUE;
    cd->cubeSize=cubeSize;
    cd->cubeCenter = cubeCenter;
    cd->ax = ax;
    cd->ay = ay;
    cd->az = az;
/* set up initial image for posible redisplay calls			*/    
    cd->changed = TRUE;
    return (cd);
}  
/*
**  void destroyCubeDescr(cubeDescr* cd)
**
**  cd	- cube descriptor to destroy
**
**  destroy cube descriptor
*/
void destroyCubeDescr(cubeDescr* cd)
{
    if (cd != NULL) 
    {   
        destroyPara (cd->curCube);
        destroyPara (cd->newCube);
        XtFree((char*)cd);
    }        
}      

/*
**  Boolean markCubeState(cubeDescr* cd, XEvent *event);
**
**  cd		- cube descriptor
**  event 	- event to check
**
**  checks is the <event> relevant to <cd> (i.e. initialize cube rotation).
**  If it is, saves current state
**  of the cube rotation matrix and 3D vector associated with the
**  event as "marked", set up dragging flag in <cd> and returns
**  TRUE.
**  Does nothing and return FALSE otherwise
*/
Boolean markCubeState(cubeDescr* cd, XEvent *event)
{
    if (cd == NULL) return (FALSE);
    if (event == NULL) return (FALSE);
    if (cd->curCube == NULL) return (FALSE);
    if (markInsCubeState(cd, event))
        return(TRUE);
    return(markOutCubeState(cd, event));
} 


/*
**  Boolean markInsCubeState(cubeDescr* cd, XEvent *event);
**
**  cd		- cube descriptor
**  event 	- event to check
**
**  assume (cd != NULL) && (event != NULL) && (cd->curCube != NULL)
**  (provided by calling routine)
**
**  checks is the <event> is inside cube image (and so can be located 
**  on one of its faces in 3D).
**  If it is, saves current state
**  of the cube rotation matrix and 3D vector associated with the
**  event as "marked", set up dragging flag in <cd>,
**  set up inside dragging  flag and returns
**  TRUE.
**  Does nothing and return FALSE otherwise
*/
static Boolean markInsCubeState(cubeDescr* cd, XEvent *event)
{
    paraFace *fTmp;
    vector cursorPos;
    cursorPos = makeVector(event->xbutton.x,event->xbutton.y);
    fTmp = pointInFace(cd->curCube, cd->curCube->frontFace, cursorPos);
    if (fTmp == NULL) return(FALSE);
/* restore a 3D vector which is on the    			*/
    restoreVector(cd->curCube, fTmp, cursorPos, makeVector3(0, 0, 1),
      &cd->markedVector, &cd->markedFaceOrt);
    cd->markedZsign = (cd->markedVector.z >= 0) ? 1 : -1;
    cd->markedLen2 = dscalarProduct3(cd->markedVector,cd->markedVector); 
    CopyM(cd->rotMatr, cd->markedRotMatr);
    cd->insideDragging = TRUE;
    cd->dragging = TRUE;
    return (TRUE);
} 

/*
**  Boolean markOutCubeState(cubeDescr* cd, XEvent *event);
**
**  cd		- cube descriptor
**  event 	- event to check
**
**  assume (cd != NULL) && (event != NULL) && (cd->curCube != NULL)
**  (provided by calling routine)
**
**  Grabs any event and considers it as a data to start
**  auxilary cube rotation cycle. (The cycle currently inmplemented
**  as rotation in the window palne around window Z axis)
**  Saves current state
**  of the cube rotation matrix and 3D vector associated with the
**  event as "marked", set up dragging flag in <cd>,
**  clear inside dragging  flag and returns
**  TRUE.
*/
static Boolean markOutCubeState(cubeDescr* cd, XEvent *event)
{
    if (! cd->outMotionEnabled) return(FALSE);
    cd->markedVector.x = event->xbutton.x - cd->cubeCenter.x;
    cd->markedVector.y = event->xbutton.y - cd->cubeCenter.y; 
    cd->markedVector.z = 0;
    CopyM(cd->rotMatr, cd->markedRotMatr);
    cd->insideDragging = FALSE;
    cd->dragging = TRUE;
    return(TRUE);
} 
    

/*
**  Boolean dragCube(cubeDescr* cd, XEvent *event);
**
**  cd		- cube descriptor
**  event 	- motion event to check
**
**   	motion even processing for rotating cube
**  Checks is cube in dragging state.
**  If it is, consider the event as the one relevant to the cube control.
**  recalculate cube rotation matrix  and returns  TRUE.
**  Does nothing and return FALSE otherwise
*/

Boolean dragCube (cubeDescr *cd, XEvent *event)
{
    double m[3][3];
    if (cd == NULL) return (FALSE);
    if (event == NULL) return (FALSE);
    if (cd->curCube == NULL) return (FALSE);
    if (!(cd-> dragging)) return (FALSE);
    if (cd->insideDragging)
        dragInsCube(cd, event, m);
    else
        dragOutCube(cd, event, m);  
    MultM(m, cd->markedRotMatr, cd->rotMatr);         
    if (cd->relativeMotion)
    {
        CopyM(cd->rotMatr, cd->markedRotMatr);
        cd->markedVector = dtransformVector3(m, cd->markedVector);
        cd->markedFaceOrt = dtransformVector3(m, cd->markedFaceOrt); 
    }           
    cd->changed = TRUE;
    return (TRUE);
}           

/*
** static void dragInsCube(cubeDescr* cd, XEvent *event, double m[3][3])
**
**  cd		- cube descriptor
**  event 	- motion event to check
**  m[3][3]	- rotation matrix to build
**
**
**  assume (cd != NULL) && (event != NULL) && (cd->curCube != NULL)
**    && (cd->dragging) && (cd->insideDragging)
**  (provided by calling routine)
**
**   	motion even processing for rotating cube (inside draggind mode)
**
**  Recalculate cube rotation matrix.
**
*/
static void dragInsCube(cubeDescr* cd, XEvent *event, double m[3][3])
{
    dvector3 newVector;
    dvector3 dvFrom, dvTo;
    vector cursorPos;
    
    cursorPos = makeVector(event->xbutton.x,event->xbutton.y);
/* restore the requested position of marked vector, 
   using marked  length^2					*/
    newVector = restoreVectorByLen(subVectors (cursorPos, cd->cubeCenter), 
      makeVector3(0, 0, 1), cd->markedLen2, cd->markedZsign);
    if (cd->bigCircleMotion)
    {
        dvFrom = cd->markedVector;
        dvTo = newVector;
    }
    else
    {
/* XYPlaneAxisMotion 						*/
/* calculate rotation axis in XY plane define rotation		*/
    	dvector3 rotAxis;
    	rotAxis = dortogonal3To2(dsubVectors3(cd->markedVector,newVector),
          dmakeVector3(0.0, 0.0, 1.0));

/*    	rotAxis = dortogonal3To2(dsubVectors3(cd->markedVector,newVector),
          cd->markedFaceOrt);
*/
        dvFrom = dortogonalize3To1(rotAxis,cd->markedVector);
        dvTo = dortogonalize3To1(rotAxis,newVector);
    }        
    if (cd->lockMotionVisible)
        makeDragRotMatrWLock(m, dvFrom, dvTo, cd->markedFaceOrt,
         dmakeVector3(0.0, 0.0, -1.0), 0.);
    else         
        makeDragRotMatr(m, dvFrom, dvTo);
}
/*
** static void dragOutCube(cubeDescr* cd, XEvent *event, double m[3][3])
**
**  cd		- cube descriptor
**  event 	- motion event to check
**  m[3][3]	- rotation matrix to build
**
**
**  assume (cd != NULL) && (event != NULL) && (cd->curCube != NULL)
**    && (cd->dragging) && ( ! cd->insideDragging)
**  (provided by calling routine)
**
**   	motion even processing for rotating cube (outside draggind mode)
**
**  Calculate cube rotation matrix.
**
*/
static void dragOutCube(cubeDescr* cd, XEvent *event, double m[3][3])
{
    dvector3 newVector;

    newVector.x = event->xbutton.x - cd->cubeCenter.x;
    newVector.y = event->xbutton.y - cd->cubeCenter.y;
    newVector.z = 0;
    
    makeDragRotMatr(m, cd->markedVector, newVector);

}

/*
**  Boolean stopCubeDragging(cubeDescr* cd, XEvent *event);
**
**  cd		- cube descriptor
**  event 	- BtnUp event to check
**
**   	motion even processing for rotating cube
**  Checks is cube in dragging state.
**  If it is, consider the event as the one relevant to the cube control.
**  Cancels dragging state of the cube, and returns  TRUE.
**  Does nothing and return FALSE otherwise
*/
Boolean stopCubeDragging (cubeDescr *cd, XEvent *event)
{
    if (cd == NULL) return (FALSE);
    if (event == NULL) return (FALSE);
    if (!cd->dragging) return (FALSE);
    cd->dragging = FALSE;
    return(TRUE);
}   

void setCubeViewAngles(cubeDescr* cd, double fi, double psi, double theta)
{
    if (cd == NULL) return;
    makeRotMatr (cd->rotMatr, fi, psi, theta);
    cd->changed = TRUE;
}
void setCubeSize (cubeDescr* cd, int cubeSize)
{
    if (cd == NULL) return;
    cd->cubeSize=cubeSize;
    cd->changed = TRUE;
}  

void setCubeLocation (cubeDescr* cd, vector cubeCenter)
{
    if (cd == NULL) return;
    cd->cubeCenter = cubeCenter;
    cd->changed = TRUE;
}  
   
                
void drawNewCubeFrontEdges (Display *d, Drawable dr, GC gc, 
  cubeDescr *cd)
{
    if (cd == NULL) return;
    drawParaFrontEdges (d, dr, gc,cd->newCube);
}        
      
void drawNewCubeBackEdges (Display *d, Drawable dr, GC gc, 
  cubeDescr *cd)
{
    if (cd == NULL) return;
    drawParaBackEdges (d, dr, gc,cd->newCube);
}        
void drawNewCubeLabeledEdges (Display *d, Drawable dr, GC gc, 
  cubeDescr *cd)
{
    if (cd == NULL) return;
    drawParaLabeledEdges (d, dr, gc,cd->newCube);
}        
void drawNewCubeUnLabeledEdges (Display *d, Drawable dr, GC gc, 
  cubeDescr *cd)
{
    if (cd == NULL) return;
    drawParaUnLabeledEdges (d, dr, gc,cd->newCube);
}        
void drawCurCubeFrontEdges (Display *d, Drawable dr, GC gc, 
  cubeDescr *cd)
{
    if (cd == NULL) return;
    drawParaFrontEdges (d, dr, gc,cd->curCube);
}   
     
void drawCurCubeBackEdges (Display *d, Drawable dr, GC gc, 
  cubeDescr *cd)
{
    if (cd == NULL) return;
    drawParaBackEdges (d, dr, gc,cd->curCube);
}        
void drawCurCubeLabeledEdges (Display *d, Drawable dr, GC gc, 
  cubeDescr *cd)
{
    if (cd == NULL) return;
    drawParaLabeledEdges (d, dr, gc,cd->curCube);
}        
void drawCurCubeUnLabeledEdges (Display *d, Drawable dr, GC gc, 
  cubeDescr *cd)
{
    if (cd == NULL) return;
    drawParaUnLabeledEdges (d, dr, gc,cd->curCube);
}        

void saveCubeImage(cubeDescr *cd)
{
    if (cd == NULL) return;
    destroyPara(cd->curCube);
    cd->curCube = cd->newCube;
    cd->newCube = NULL;
    cd->changed = FALSE;
}        


void makeNewCubeImage(cubeDescr* cd )
{
    vector3 shiftX_2;
    vector3 shiftY_2;
    vector3 shiftZ_2;
    vector3 center;
    paraState *cs;  
    if (cd == NULL) return;  
/* culculate half-length vectors of cube edge basis to calculate
    shift from cube center to "0" vertex					*/
    shiftX_2.x = (int) floor (cd->rotMatr[0][0] * cd->cubeSize/2. + 0.5);         
    shiftX_2.y = (int) floor (cd->rotMatr[1][0] * cd->cubeSize/2. + 0.5);         
    shiftX_2.z = (int) floor (cd->rotMatr[2][0] * cd->cubeSize/2. + 0.5);         
    shiftY_2.x = (int) floor (cd->rotMatr[0][1] * cd->cubeSize/2. + 0.5);         
    shiftY_2.y = (int) floor (cd->rotMatr[1][1] * cd->cubeSize/2. + 0.5);         
    shiftY_2.z = (int) floor (cd->rotMatr[2][1] * cd->cubeSize/2. + 0.5);         
    shiftZ_2.x = (int) floor (cd->rotMatr[0][2] * cd->cubeSize/2. + 0.5);         
    shiftZ_2.y = (int) floor (cd->rotMatr[1][2] * cd->cubeSize/2. + 0.5);         
    shiftZ_2.z = (int) floor (cd->rotMatr[2][2] * cd->cubeSize/2. + 0.5); 
    center.x = cd->cubeCenter.x;
    center.y = cd->cubeCenter.y;
    center.z = 0;        
    cs = (paraState*) XtMalloc(sizeof(paraState));  
    buildPara(cs, center,
    shiftX_2,  shiftY_2,  shiftZ_2, cd->ax, cd->ay, cd->az);
    destroyPara(cd->newCube);
    cd->newCube = cs;
}

void makePSCubeFrontEdges (cubeDescr *cd, FILE *psfile)
{
    if (cd == NULL) return;
    makePSParaFrontEdges (cd->curCube,  psfile);
}   
     
void makePSCubeBackEdges (cubeDescr *cd, FILE *psfile)
{
    if (cd == NULL) return;
    makePSParaBackEdges (cd->curCube,  psfile);
}        
void makePSCubeLabeledEdges (cubeDescr *cd, FILE *psfile)
{
    if (cd == NULL) return;
    makePSParaLabeledEdges (cd->curCube,  psfile);
}        
void makePSCubeUnLabeledEdges (cubeDescr *cd, FILE *psfile)
{
    if (cd == NULL) return;
    makePSParaUnLabeledEdges (cd->curCube,  psfile);
}        

                

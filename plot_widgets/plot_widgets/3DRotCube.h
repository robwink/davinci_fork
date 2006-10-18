#ifndef _3DROTCUBE_H
#define _3DROTCUBE_H


typedef struct _cubeDescr {
/* cube state(s) inforrmation */
    paraState *curCube;		/* cube state drawn in the windows	*/
    paraState *newCube;		/* next cube state (as the result of
    				   event or "set" call processing
    				   which is not drawn yet		*/
    double rotMatr [3][3];	/* current state of cube rotation matrix*/
    double markedRotMatr [3][3];/* state of cube rotation matrix marked by 
    				   BtnDown event			*/
    dvector3 markedVector;	/* point in 3D space marked by BtnDown
    				   event				*/
    dvector3 markedFaceOrt;     /* Ort to the face mark event occure in.
    				   (for locking motion in visible area) */
    double markedLen2;		/* Square of the length of the initially
       				   marked vector for inside motion. Is not 
       				   changeing during otion event processing 
       				   even in <relativeMotion> mode.	*/
    int markedZsign;		/* Sign of Z coordinate of initially 
    				   marked vector. Is not changeing during
    				   motion event processing even
    				   in <relativeMotion> mode. (markedVector.z
    				   might apparently become zero in last mode).
    				   Is used for motion locking, so that image
    				   of initial <markedVector> has the same
    				   sign of Z coordinate during motion cycle */         				           				   
    Boolean dragging;		/* TRUE if dragging is in progress	*/
    Boolean insideDragging;	/* Dragging mode flag. If TRUE the current 
    				   dragging cycle was initialized inside 
    				   the cube, if FALSE - outside. Dragging cycles
    				   are generalluy defferrent when initialized
    				   inside the cube image and outside it	*/
/* motion control parameters */					
    				   
    Boolean relativeMotion;   	/* If TRUE, each motion event processing
     				   is applyed to the state reached after the 
     				   last motion even. Otherwise the processing
     				   is applyed to the state which was saved
     				   during push button (markCubeState) event.
    				   Technically, if the parameter is TRUE,
          			   <markedVector>  and <markedRotMatr> are
          			   changing each motion event
    				   processing cycle, to set up a new
    				   position for next motion even processing.
    				   Otherwise  <markedVector>  and
    				   <markedRotMatr> setted up by <markCubeState>
    				   at the begiining of dragging are used for
    				   the processing of each motion event.
    				   Rotation behavior will be different  */
    Boolean bigCircleMotion;	/* If TRUE each transfer (either between
    				   two motion events or between initial
    				   state in dragging cycle, see above) is
    				   constructed so, that marked point on the
    				   cube surface is moving along big circle
    				   of a sphere with the same center as the
    				   parallelepiped.
    				   If FALSE, the XYPlaneAxisMotion is performed,
    				   which means that the marked piont is moving
    				   using the rotation with the rotation
    				   axis in the plane Z=0, i.e. XY plane or
    				   window plane				*/
    Boolean lockMotionVisible;	/* If TRUE, the inside motion will be locked
    				   in the region where the grabbed face
    				   of the parallelepiped is visible.
    				   If FALSE this lock will not be in use.
    				   The motion is locked in any case when
    				   the rotation image of <markedVector>
    				   has zero Z coordinate.		*/ 
    Boolean outMotionEnabled;	/* If TRUE, the parallepiped will grab events
    				   outside its image area to initialize
    				   outside motion cycle. This cycle
    				   realizes just rotation in the window
    				   plane.
    				   If FALSE, the events outside its image area
    				   will be ignored			*/
    int cubeSize;		/* size of cube edge in window pixes    */
    vector cubeCenter;	        /* cube center in windows coordinates   */
    int ax;			/* X axis attribute index		*/
    int ay;			/* Y axis attribute index		*/
    int az;			/* Z axis attribute index		*/
    Boolean changed;    	/* TRUE means, that some changes were
    				   made in the contents of the object,
    				   so it is no more in consistens with
    				   <curCube>.
    				   Setted to FALSE by any "set" or
    				   "dragging" function
    				   and to TRUE by <saveCubeImage>	*/ 
} cubeDescr;   

/* create and destroy functions						*/
cubeDescr* createCubeDescr (vector cubeCenter, int cubeSize, double fi,
  double psi, double theta, int ax, int ay, int az);
void destroyCubeDescr(cubeDescr* cd);

/* event processing functions						*/
Boolean markCubeState(cubeDescr* cd, XEvent *event);
Boolean dragCube (cubeDescr *cd, XEvent *event);
Boolean stopCubeDragging (cubeDescr *cd, XEvent *event);

/* set / get interface functions	*/
void setCubeSize(cubeDescr* cd, int cubeSize);
void setCubeLocation(cubeDescr* cd, vector cubeCenter);
void setCubeViewAngles(cubeDescr* cd, double fi, double psi, double theta);

/* -- not implemnted			*
void setCubeAttributes(cubeDescr* cd, int ax, int ay, int az);
*/ 

void drawCurCubeFrontEdges (Display *d, Drawable dr, GC gc, 
  cubeDescr *cd);
void drawCurCubeBackEdges (Display *d, Drawable dr, GC gc, 
  cubeDescr *cd);
void drawCurCubeLabeledEdges (Display *d, Drawable dr, GC gc, 
  cubeDescr *cd);
void drawCurCubeUnLabeledEdges (Display *d, Drawable dr, GC gc, 
  cubeDescr *cd);
void drawNewCubeFrontEdges (Display *d, Drawable dr, GC gc, 
  cubeDescr *cd);
void drawNewCubeBackEdges (Display *d, Drawable dr, GC gc, 
  cubeDescr *cd);
void drawNewCubeLabeledEdges (Display *d, Drawable dr, GC gc, 
  cubeDescr *cd);
void drawNewCubeUnLabeledEdges (Display *d, Drawable dr, GC gc, 
  cubeDescr *cd);
void saveCubeImage(cubeDescr *cd);
void makeNewCubeImage(cubeDescr* cd );

void makePSCubeFrontEdges (cubeDescr *cd, FILE *psfile);
void makePSCubeBackEdges (cubeDescr *cd, FILE *psfile);
void makePSCubeLabeledEdges (cubeDescr *cd, FILE *psfile);
void makePSCubeUnLabeledEdges (cubeDescr *cd, FILE *psfile);

#endif

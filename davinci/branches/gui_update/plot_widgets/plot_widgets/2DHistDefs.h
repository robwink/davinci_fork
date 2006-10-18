#ifndef _2DHIST_DEFS
#define _2DHIST_DEFS


typedef struct _discreteMap {
    matrix2 map;	 	/* See comments just below typedef 	*/    
    matrix2 rotMatr;		/* See comments just below typedef 	*/
    int zFactor; 
    int cubeSize;	   	/* max size of cube which can be (parallel) 
    			      	   projected to the window, in pixels	*/
    vector vertex;  		/* Window coordinates of the 
    			 	   nearmost cube vertex on the lower cube 
    			 	   base */
    vector mapLength;		/* length of image as.....		*/
    				   
} discreteMap;
#endif

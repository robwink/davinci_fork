#include "parser.h"
#ifdef HAVE_LIBPROJ
/**
 ** projection()
**/

/**
 ** compute stereographic projection of an image
 **/


#define NUM_PARAM  5    /*Currently the number of parameters used to define the projection */


/*      This define and include if for the projection library,
        limiting the build to just on projection type           */

/* #define PJ_LIST_H "my_list.h" */


#ifdef __cplusplus
extern "C" {
#endif
#include "projects.h"
#ifdef __cplusplus
}
#endif

Var *ff_projection(vfuncptr func, Var * arg)
{
	Var *obj = NULL, *pLat=NULL, *pLon=NULL, *pRadius=NULL;
	
	void *data;
	int newX,newY;
	int x,y,z;
	int ix,iy; 
	float Lat=90.0,Lon=0.0,radius=30.0;
	int ppd;
	int format,size,index;
	float xmin,xmax,ymin,ymax;
	float tempX,tempY;
	float temp_i;
	int type=0;
	char *pType=NULL;

        char            *parameters[NUM_PARAM];
        float           lon_mod_1,lon_mod_2;
        float           max_lat,min_lat;

        int i,j,k;
	int step,jump;/*X and Z indices into data with size_of(format) modifcation*/

	char *options[]={"MARS","EARTH","SPHERE",NULL};

	projUV	pj_data;
	projPJ	*ref;

        int ac;
        Var **av;
        Alist alist[7];


        alist[0] = make_alist( "obj",    	ID_VAL,      NULL,     &obj);
        alist[1] = make_alist( "type", 		ID_ENUM,     options,  &pType);
        alist[2] = make_alist( "lat",	  	ID_VAL,       NULL,     &pLat);
        alist[3] = make_alist( "lon", 		ID_VAL,       NULL,     &pLon);
        alist[4] = make_alist( "rad", 		ID_VAL,       NULL,     &pRadius);
        alist[5] = make_alist( "ppd", 		INT,         NULL,     &ppd);
        alist[6].name = NULL;


	if (parse_args(func, arg, alist) == 0) return(NULL);

        if (obj == NULL) {
                parse_error("%s: No object specified\n", func->name);
                return(NULL);
        }

        for(i=0;i<NUM_PARAM;i++){
                parameters[i]=(char *)calloc(20,sizeof(char));
        }


    if (pType == NULL) {
        parse_error("Argument required: %s(%s)", func->name, "type");
        return(NULL);
    } 
    else if (!strcasecmp(pType, "MARS")) type = 1;
    else if (!strcasecmp(pType, "EARTH")) type = 2;
    else if (!strcasecmp(pType, "SPHERE")) type = 3;

    else {
        parse_error("Unrecognized value: %s(...%s=...)", func->name, "type");
        return(NULL);
    }
        	strcpy(parameters[0],"proj=stere"); 
			/* //Needed to initialize the projection library for Stereo */
		Lat=extract_float(pLat,0);
		Lon=extract_float(pLon,0);
		radius=extract_float(pRadius,0);
		if (Lat==0.0)
			Lat=0.001; 
			/* //Odd bug: Lat must not = 0.0 else it will fail.  This is close. */
		

	switch (type){ 
	/* //Setting up the parameter list for Projection initalization */
	case 1:
       		sprintf(parameters[1],"lon_0=%f",Lon);
        	sprintf(parameters[2],"lat_0=%f",Lat);
        	sprintf(parameters[3],"a=%f",3399200.0); /* //Major equitorial radius (meters) */
        	sprintf(parameters[4],"b=%f",3376100.0); /* //Polar radius (meters) */
		break;
	case 2:
       		sprintf(parameters[1],"lon_0=%f",Lon);
        	sprintf(parameters[2],"lat_0=%f",Lat);
        	strcpy(parameters[3],"ellps=MERIT");
        	strcpy(parameters[4],"");
		break;
	case 3:
       		sprintf(parameters[1],"lon_0=%f",Lon);
        	sprintf(parameters[2],"lat_0=%f",Lat);
		strcpy(parameters[3],"");
		strcpy(parameters[4],"");
		break;
	}


        if (!(ref = pj_init(NUM_PARAM,parameters))){
                fprintf(stderr,"%s:Projection init failed\n","davinci");
                exit(1);
        }

/*Here we need to find the minimum and maximum X & Y values in the stereo world. 
**	Having done that, we can scale any 0->1 value to the edges of the stereo image.
**	Before we find the min and max X &Y values we need to check and see if the projection
**	Goes over either pole; if it does, we need to "flip" the longitude to the other
**	side of the world.
*/
        if ((Lat+radius) > 90.0){
                max_lat=90.0-((Lat+radius)-90.0);
                lon_mod_1=180.0;
        }
        else {
                max_lat=Lat+radius;
                lon_mod_1=0.0;
        }

        if ((Lat-radius) < -90.0){
                min_lat=(-90.0-((Lat-radius)+90.0));
                lon_mod_2=180.0;
        }
        else {
                min_lat=Lat-radius;
                lon_mod_2=0.0;
        }

/*YMAX & XMAX*/
pj_data.u=fmod(Lon-lon_mod_1,360.0);
pj_data.v=max_lat;
pj_data.u*=DEG_TO_RAD;
pj_data.v*=DEG_TO_RAD;
pj_data=pj_fwd(pj_data,ref);
ymax=pj_data.v;
xmax=pj_data.v;
/*YMIN & XMIN*/
pj_data.u=fmod(Lon-lon_mod_2,360.0);
pj_data.v=min_lat;
pj_data.u*=DEG_TO_RAD;
pj_data.v*=DEG_TO_RAD;
pj_data=pj_fwd(pj_data,ref);
ymin=pj_data.v;
xmin=pj_data.v;

/*Now we want to know the boundary of our input data*/

        x = GetSamples(V_SIZE(obj), V_ORG(obj));
        y = GetLines(V_SIZE(obj), V_ORG(obj));
        z = GetBands(V_SIZE(obj), V_ORG(obj));

/* //Its format and size (in bytes) */
	format=V_FORMAT(obj);
	size=NBYTES(format);
	newX=newY=ppd*2*radius;

/* //Set aside enough space in our output dataset variable */
	data=calloc(z*newX*newY,size);

/*Now we loop through stereo space and project backwards to Lat/Lon space, find the appropriate
**	Pixel value along the Z and store it in our BIP organized output dataset
*/
	for (i=0;i<newY;i++){
		temp_i=((double)(newY-1-i)/(double)(newY-1))*(ymax-ymin)+ymin;
		jump=i*newX*z*size;
		for(j=0;j<newX;j++){
			pj_data.u=((double)(newX-1-j)/(double)(newX-1))*(xmax-xmin)+xmin;
			pj_data.v=temp_i;
			pj_data=pj_inv(pj_data,ref);
			pj_data.u*=RAD_TO_DEG;
			pj_data.v*=RAD_TO_DEG;
			tempX=(pj_data.u-180.0)/-360.0;
			tempY=((pj_data.v*-1.0)+90.0)/180.0;
			ix=tempX*(double)(x-1);
			iy=tempY*(double)(y-1);
			step=j*size*z;
			for (k=0;k<z;k++){
				index=cpos(ix,iy,k,obj);
				memcpy(&((char *)data)[jump+step+k*size],
                                       &((char *)V_DATA(obj))[index*size],
                                       size);
			}
		}
	}

	return(newVal(BIP,z,newX,newY,format,data));
}

#endif

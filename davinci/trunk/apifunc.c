#include "parser.h"
#ifdef INCLUDE_API
=======

#include "api.h"


APIDEFS *
api_lookup(char *aname){
    int lup,apicount;

    apicount = sizeof(apidefs) / sizeof(APIDEFS);

    for(lup = 0; lup < apicount; lup++){
	if(!strcmp(apidefs[lup].apiname, aname))
	    return(&apidefs[lup]);
    }
    return(NULL);
}

int
find_argbin(int argc, APIARGS *args, char *argname){
    int lup;

    for(lup = 0; lup < argc; lup++){
	if(!strcmp(argname,args[lup].argname))
	    return(lup);
    }
    return(-1);
}

int
find_free_argbin(int argc,APIARGS *args){
    int lup;

    for(lup = 0; lup < argc; lup++){
	if(args[lup].argusp == NULL)
	    return(lup);
    }
    return(-1);
}

int 
api_extract_int(int fmt,void * v, int i)
{
    switch (fmt) {
    case BYTE:
        return ((int) ((u_char *) v)[i]);
    case SHORT:
        return ((int) ((short *) v)[i]);
    case INT:
        return ((int) ((int *) v)[i]);
    case FLOAT:
        return ((int) ((float *) v)[i]);
    case DOUBLE:
        return ((int) ((double *) v)[i]);
    }
    return (0);
}
float 
api_extract_float(int fmt,void * v, int i)
{
    switch (fmt) {
    case BYTE:
        return ((float) ((u_char *) v)[i]);
    case SHORT:
        return ((float) ((short *) v)[i]);
    case INT:
        return ((float) ((int *) v)[i]);
    case FLOAT:
        return ((float) ((float *) v)[i]);
    case DOUBLE:
        return ((float) ((double *) v)[i]);
    }
    return (0);
}
double 
api_extract_double(int fmt,void * v, int i)
{
    switch (fmt) {
    case BYTE:
        return (((u_char *) v)[i]);
    case SHORT:
        return (((short *) v)[i]);
    case INT:
        return (((int *) v)[i]);
    case FLOAT:
        return (((float *) v)[i]);
    case DOUBLE:
        return (((double *) v)[i]);
    }
    return (0);
}

int
typeconvert_args(int srctype,void *src,int dsttype,void *dst,int dsize){
    int lup,*ip;
    short *sp;
    float *fp;
    double *dp;
    char *cp;

    if(srctype == dsttype){
	memcpy(dst,src,dsize*NBYTES(dsttype));
	return(0);
    }
    
    switch(dsttype){
    case BYTE:
	cp = (char *) dst;
	for(lup = 0; lup < dsize; lup++){
	    cp[lup] = saturate_byte(api_extract_int(srctype,src,lup));
	}
	break;
    case SHORT:
	sp = (short *) dst;
	for(lup = 0; lup < dsize; lup++){
	    sp[lup] = saturate_short(api_extract_int(srctype,src, lup));
	}
	break;
    case INT:
	ip = (int *) dst;
	for(lup = 0; lup < dsize; lup++){
	    ip[lup] = api_extract_int(srctype,src, lup);
	}
	break;
    case FLOAT:
	fp = (float *) dst;
	for(lup = 0; lup < dsize; lup++){
	    fp[lup] = api_extract_float(srctype,src, lup);
	}
	break;
    case DOUBLE:
	dp = (double *) dst;
	for(lup = 0; lup < dsize; lup++){
	    dp[lup] = api_extract_double(srctype,src, lup);
	}
	break;
    default:
	printf("Unknown data format\n");
	return(-1);
	break;
    }
    return(0);
}

int
link_argvalue(Var *arg,APIARGS *aarg){
    Var *e;
    int lup;
    void *argmem;

    e = arg;
    if(V_TYPE(arg) == ID_UNK){
	if((e = eval(arg)) == NULL){
	    printf("No such variable '%s'\n",V_NAME(arg));
	    return(-1);
	}
    }

    if(V_TYPE(e) == ID_STRING){
	if((aarg->argtype & DTMASK) == BYTE
	   && (aarg->argtype & PTRBIT) == PTRBIT)
	    aarg->argval = strdup(V_STRING(e));
	else {
	    printf("Data type mismatch\n");
	    return(-1);
	}
    } else if(V_TYPE(e) == ID_VAL){
	aarg->argval = calloc(V_DSIZE(e),NBYTES(aarg->argtype & DTMASK));
	if((aarg->argtype & VOLBIT) == 0)
	    return(typeconvert_args(V_FORMAT(e),V_DATA(e),
				    aarg->argtype & DTMASK,aarg->argval,
				    V_DSIZE(e)));
    }
    return(0);
}

Var *
dispatch_api(APIDEFS *api,Var *garg){
    int lup,givenac,abin,aargc,dtype;
    Var *ait,*vv;
    APIARGS *aargv;

    aargc = api->argc - 1;	/* because [0] contains return type */
    aargv = &(api->apiargs[1]);

    for(lup = 0; lup < aargc; lup++)
	aargv[lup].argusp = NULL;
    
    for(givenac=0,ait=garg; ait!=NULL; ait=V_NEXT(ait),givenac++)
	;
    
    if(givenac != aargc){
	printf("%s() requires %d arguments\n",api->apiname,aargc);
	return(NULL);
    }

    for(ait = garg; ait != NULL; ait = V_NEXT(ait)){
	if(V_TYPE(ait) == ID_KEYWORD){
	    if((abin=find_argbin(aargc,aargv,V_NAME(ait))) < 0){
		printf("Unknown keyword to %s(... %s= ...)\n",api->apiname,
		       V_NAME(ait));
		return(NULL);
	    } else {
		if(link_argvalue(V_KEYVAL(ait),&aargv[abin]) < 0)
		    return(NULL);
		aargv[abin].argusp = V_KEYVAL(ait);
	    }
	}
    }

    for(ait = garg; ait != NULL; ait = V_NEXT(ait)){
	if(V_TYPE(ait) != ID_KEYWORD){
	    abin = find_free_argbin(aargc,aargv);	/* abin can't be -1 */
	    if(link_argvalue(ait,&aargv[abin]) < 0)
		return(NULL);
	    aargv[abin].argusp = ait;
	}
    }

    for(lup = 0; lup < aargc; lup++){
	if(aargv[lup].argusp == NULL){
	    printf("%s: No value specified for arg #%d\n",api->apiname,lup+1);
	    return(NULL);
	}
    }

    /* call the function */
    api->apifptr(aargc,api->apiargs);

    for(lup = 0; lup < aargc; lup++){
	dtype = aargv[lup].argtype;
	vv = aargv[lup].argusp;
	if((dtype & CONSTBIT) == 0 && (dtype & PTRBIT) == PTRBIT
	   && V_NAME(vv) != '\0'){
	    vv = eval(vv);
	    if(V_TYPE(vv) == ID_STRING){
		if(V_STRING(vv) != NULL)
		    free(V_STRING(vv));
		V_STRING(vv) = strdup(aargv[lup].argval);
	    } else {
	        typeconvert_args(dtype&DTMASK,aargv[lup].argval,V_FORMAT(vv),
			         V_DATA(vv),V_DSIZE(vv));
	    }
	}
	free(aargv[lup].argval);
    }

    if((dtype=(api->apiargs[0].argtype&DTMASK)) != VOID){
	vv = newVal(BSQ,1,1,1,dtype,api->apiargs[0].argval);
    } else {
	vv = NULL;
    }
    
    return(vv);
}

#endif

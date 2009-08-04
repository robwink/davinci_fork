#include "parser.h"
#if defined(HAVE_LIBGEN_H) && !defined(_AIX)
/* JAS FIX: conflicts with regex.h on AIX */
#include <libgen.h>
#endif

#include "system.h"

 

Var *
ff_copy(vfuncptr func, Var * arg)
{
   int force = 0;
    char *src = NULL;
    char *dest = NULL;
    char *srcLocated = NULL;
    char *destLocated = NULL;
    char *tmp = NULL;
    struct stat sbuf;

    Alist alist[4];
    alist[0] = make_alist("src",    ID_STRING,     NULL,     &src);
    alist[1] = make_alist("dest",    ID_STRING,     NULL,     &dest);
    alist[2] = make_alist("force",    INT,       NULL, &force);
    alist[3].name = NULL;
    
    if (parse_args(func, arg, alist) == 0) return(NULL);

    if (src == NULL) {
        parse_error("%s: No source filename specified", func->name);
	 return(NULL);
    }else if (src == NULL) {
        parse_error("%s: No destination filename specified", func->name);
	 return(NULL);
    } 
    if ((srcLocated = dv_locate_file(src)) == NULL) {
	parse_error("Unable to expand filename %s\n", src);
	return (NULL);
    }
    if ((destLocated = dv_locate_file(dest)) == NULL) {
	parse_error("Unable to expand filename %s\n", dest);
	free(srcLocated);	
	return (NULL);
    }

    //Check if the destination is a directory. If yes, the file to be put in there
    stat(destLocated, &sbuf);
    if(S_ISDIR(sbuf.st_mode) && access(destLocated, F_OK) == 0){
	tmp = (char *) malloc( strlen(destLocated) + strlen(basename(src)) + 2);  //2 is for '\0' and '/'
	sprintf(tmp, "%s/%s",destLocated, basename(src));
	free(destLocated);
	destLocated = tmp;
    }

    //Check whether to overwrite the file.
    if (!force && access(destLocated, F_OK) == 0) {
	parse_error("%s: Destination filename %s already exists! Use force to overwrite", func->name, destLocated);
	free(srcLocated);	
	free(destLocated);	
	return(NULL);
    }

    /** Do the actual Copying **/
    FILE *srcFile;
    FILE *destFile;
    int Byte;
    int i;

    srcFile = fopen(srcLocated, "rb");
    destFile = fopen(destLocated, "wb");
            
    if(srcFile==NULL){    
	parse_error("Error: Can't Open source file.");
	free(srcLocated);	
	free(destLocated);	
	return (NULL);
    }else if(destFile==NULL){
	parse_error("Error: Can't Open dest file.");
	free(srcLocated);	
	free(destLocated);	
	return (NULL);
    }else{
	Byte=fgetc(srcFile);
	while(Byte!=EOF){
	   fputc(Byte,destFile);
	   Byte=fgetc(srcFile);
	}
	fclose(srcFile);
	fclose(destFile);

	free(srcLocated);	
	return(newString(strdup(destLocated)));
    }

    return(NULL);
}

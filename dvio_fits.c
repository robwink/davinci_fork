#include "parser.h"

#ifdef HAVE_LIBCFITSIO
#include <cfitsio/fitsio.h>

char     DVIO_FITS_err_text[64];

#define QUERY_FITS_ERROR(status)    if (status) { \
      fits_get_errstatus(status,DVIO_FITS_err_text); \
      parse_error("cfitsio ERROR occured:\n\t%s\n",DVIO_FITS_err_text); \
      return(NULL); }

Var *
Read_FITS_Image(fitsfile *fptr)
{
   char *data;
   int format;
   int fits_type;
   int i;
   int dim;
   int size[3]={0,0,0};
   int fpixel[3]={1,1,1};
   int datatype;
   int status=0;

   fits_get_img_dim(fptr,&dim,&status);
   QUERY_FITS_ERROR(status);

   if (dim > 3) {
      parse_error("Data objects of greater than 3 dimensions are not handled.");
      return(NULL);
   }

   fits_get_img_type(fptr,&fits_type,&status);
   QUERY_FITS_ERROR(status);

   fits_get_img_size(fptr,dim,(long *)size,&status);
   QUERY_FITS_ERROR(status);
   
   for(i=0;i<3;i++) 
      if(!size[i]) {
         size[i]=1;
         fpixel[i]=0;
      }
   

   switch (fits_type) {

      case BYTE_IMG:    
                     format = BYTE;
                     datatype = TBYTE;
                     break;

      case SHORT_IMG:    
                     format = SHORT;
                     datatype = TSHORT;
                     break;

      case LONG_IMG:    
                     format = INT;
                     datatype = TINT;
                     break;

      case FLOAT_IMG:    
                     format = FLOAT;
                     datatype = TFLOAT;
                     break;

      case DOUBLE_IMG:    
                     format = DOUBLE;
                     datatype = TDOUBLE;
                     break;

   }

   data = (char *)calloc(size[0]*size[1]*size[2],NBYTES(format));

   fits_read_pix(fptr,datatype,(long *)fpixel,(size[0]*size[1]*size[2]),NULL,(void *)data,NULL,&status);
   QUERY_FITS_ERROR(status);

   

   return(newVal(BSQ,size[0],size[1],size[2],format,data));
}


Var *
Read_FITS_Table(fitsfile *fptr)
{
   return(NULL);
}

Var *
makeVarFromFITSLabel(char *fits_value,char key_type)
{
   int   *i;
   float *f;

   Var *v;

   switch (key_type) {

   case 'C':
   case 'L':
               v = newString(strdup(fits_value));
               break;

   case 'I':   
               i = (int *)calloc(1,sizeof(int));
               *i = atoi(fits_value);
               v = newVal(BSQ,1,1,1,INT,i);
               break;

   case 'F':
               f = (float *)calloc(1,sizeof(float));
               *f = atof(fits_value);
               v = newVal(BSQ,1,1,1,FLOAT,f);
               break;

   case 'X':
               parse_error("Unclear how to parse this...keeping as string");
               v = newString(strdup(fits_value));
               break;
   }

   return(v);
}
               

void
Parse_Name(char *card, char **name)
{

   char *p;
   char *in = strdup(card);

   if (strlen(in) > 80) {
      *name=NULL;
      free(in);
      return;
   }

   if(!(p=strstr(in,"="))){ //no key/value pair...toss it for now
      *name=NULL;
      free(in);
      return;
   }

   if (p-in > 8) { //yeah, it's an ='s sign, but deep in line, probably a comment...toss it for now
      *name=NULL;
      free(in);
      return;
   }

   *p='\0';

// Need to trim name:

   p--; // _=_ should put us at first _ location

   while (*p == ' ' && p!=(*name))
      p--;
   p++;
   *p='\0';
   *name=strdup(in);
   free(in);
}
   

/*
** FITS file entry function.  This provides davinci with
** an API (well, one function anyway ;) to read FITS type files
** and can be called from ANY internal davinci function.
** The only parameter it takes is legal FITS type filename
** which is a path, a file name and an option extension
** string which is concatined onto the filename ([] encloses
** the extension).  A return value of NULL indicates a read
** failutre.  Other-wise, a Var * to a structure containing
** the FITS data is returned.
*/
Var *
FITS_Read_Entry(char *fits_filename)
{
   fitsfile *fptr;
   char     header_entry[FLEN_CARD];
   int      status=0;
   int      i,j;
   int      num_objects;
   int      cur_object;
   int      num_header_entries;
   int      object_type;


   char     *name;

   char     fits_value[128],fits_comment[128];

   Var      *head=new_struct(0);
   Var      *sub;
   Var      *davinci_data;

   char     obj_name[64];
   char     key_type;

   fits_open_file(&fptr,fits_filename,READONLY,&status);
   fits_get_num_hdus(fptr,&num_objects,&status);
   
   for(i=1;i<=num_objects;i++){ //stupid jerks think this is fortran!
      fits_movabs_hdu(fptr,i,&object_type,&status);
   
      fits_get_hdrspace(fptr,&num_header_entries,NULL,&status);

      sub=new_struct(0);
      sprintf(obj_name,"object_%d",i);

      for(j=1;j<=num_header_entries;j++){
         fits_read_record(fptr,j,header_entry,&status);

         //Parse Record for the name
         Parse_Name(header_entry,&name);
         if (name == NULL) // no ='s found ... toss it for now
            continue;

         fits_parse_value(header_entry,fits_value,fits_comment,&status);
         fits_get_keytype(fits_value,&key_type,&status);

         QUERY_FITS_ERROR(status);

         //Add to sub
         add_struct(sub,name,makeVarFromFITSLabel(fits_value,key_type));
   
      }
      // Read data : we'll need a data type switch
      fits_get_hdu_type(fptr,&object_type,&status);

      QUERY_FITS_ERROR(status);

      if (object_type == IMAGE_HDU)
         davinci_data=Read_FITS_Image(fptr);

      else if (object_type == ASCII_TBL || object_type == BINARY_TBL)
         davinci_data=Read_FITS_Table(fptr);

      else {
         parse_error("Unknown data object in fits file at HDU location: %d\nSkipping data portion",i);
         davinci_data=NULL;
      }

      // Add to sub
      if (davinci_data)
         add_struct(sub,"data",davinci_data);


      // Add sub to head
      add_struct(head,obj_name,sub);
   } 


   fits_close_file(fptr,&status);


  

   return(head); 

}

/*
** This is one entry point for crearting a FITS file.
** This function take a davinci strucutre as input.
** The object is first validated to ensure it contains 
** the minimum label information needed for a FITS file.  
** If the strcuture sucsessfully passes validation, its
** contents are translated.  More than one
** data object can be contained in the structure.  For
** each data object and associated information in the
** strucutre a FITS object and associated label will 
** be generated in the file.
*/

Var *
FITS_Write_Structure(char *fits_filename, Var *obj, int force)
{
}


/* This is the second entry for creating a FITS file.
**	This function is called is the davinci object to
** written out is NOT a structure, but is instead a
** Var or cube object.  If this is the case, the 
** minimum label information needed for a FITS object
** is generated, and the VAR is written out as the 
** associated data.  This rouine produces only a single
** FITS object within the FITS file.
*/

/* 
** Thu Nov 11 15:19:16 MST 2004:
**		We currently only output image/cube objects...no tables
*/

Var *
FITS_Write_Var(char *fits_filename, Var *obj, int force)
{
	char		*name;
	int		status=0;
	fitsfile *fptr;
	int		naxis;
	long		naxes[3];
	int		bitpix;
	int		datatype;
	int		*size;
	int		i;
	int		fpixel[3]={1,1,1};

	if (V_ORG(obj) != BSQ) {
		parse_error("Only BSQ ordered objects can be written out");
		return(NULL);
	}

	if (force) {
		name = (char *)calloc(strlen(fits_filename)+2,1);
		strcpy(name,"!");
		strcat(name,fits_filename); /* the '!'-prefix tells the cfitsio 
												 lib to overwrite existing file */
	}
	else
		name = strdup(fits_filename);

	fits_create_file(&fptr,name,&status);
	QUERY_FITS_ERROR(status);

	switch (V_FORMAT(obj)) {
		case BYTE:
			bitpix = BYTE_IMG;
			datatype = TBYTE;
			break;

		case SHORT:
			bitpix = SHORT_IMG;
			datatype = TSHORT;
			break;

		case INT:
			bitpix = LONG_IMG; /* Yeah, I know, it says long...but it means 32-bit */
			datatype = TINT; /*Future's so bright...*/
			break;

		case FLOAT:
			bitpix = FLOAT_IMG;
			datatype = TFLOAT;
			break;

		case DOUBLE:
			bitpix = DOUBLE_IMG;
			datatype = TDOUBLE;
			break;
	}

	size = V_SIZE(obj);
	naxis = 0;
	for(i=0;i<3;i++) {
		if (size[i]) {
			naxis++;
			naxes[i] = size[i];
		}
		else
			naxes[i] = 0;
	}
	for(i=0;i<3;i++) 
		if(!(naxes[i])) naxes[i]=1;

	fits_create_img(fptr,bitpix,naxis,naxes,&status);
	QUERY_FITS_ERROR(status);

	fits_write_pix(fptr,datatype,(long *)fpixel,size[0]*size[1]*size[2],(void *)V_DATA(obj),&status);
	QUERY_FITS_ERROR(status);

	fits_close_file(fptr,&status);

	QUERY_FITS_ERROR(status);

	return(NULL);

}


/*
** Davinci wrapper function.
** This function takes an object, either a structure
** or a Var and write out a FITS object.  Different
** entry point routines are called depending
** on the objects type (Var or Structure).  Any
** other type is not valid, and is rejected.
*/

Var *
WriteFITS(vfuncptr func, Var * arg)
{
	Var *obj=NULL;
	char *filename=NULL;
	int force=0;

	char *name;

   Alist alist[4];
   alist[0] = make_alist("obj", ID_UNK, NULL, &obj);
   alist[1] = make_alist("filename", ID_STRING, NULL, &filename);
	alist[2] = make_alist("force",    INT,       NULL, &force);
   alist[3].name = NULL;

   if (parse_args(func, arg, alist) == 0)
      return (NULL);

	if (obj == NULL) {
		parse_error("No object specified.");
		return(NULL);
	}

	if (filename == NULL) {
		parse_error("No filename specified.");
		return(NULL);
	}

	if (V_TYPE(obj) == ID_VAL)
		return(FITS_Write_Var(filename,obj,force));

	else if (V_TYPE(obj) == ID_STRUCT)
		return(FITS_Write_Structure(filename,obj,force));

	else
		parse_error("You have submitted an invalid object to be written out as a FITS file");


	return(NULL);

}

/*
** Davinci wrapper function.
** This function receives a filename an optional extension.
** If extension is used, it is concatinated onto the filename
** which is how the cfitsio library expects it to be.
** The final filename is used to call the entry function.
** This function returns NULL if the read failed or a Var *
** pointing to the structure representing the contents 
** of the fits_file.
*/
Var *
ReadFITS(vfuncptr func, Var * arg)
{
   char *filename = NULL;
   char *extension = NULL;
   char *fe=NULL;
   
   Var  *data=NULL;


   Alist alist[3];
   alist[0] = make_alist("filename", ID_STRING, NULL, &filename);
   alist[1] = make_alist("extension",ID_STRING, NULL, &extension);
   alist[2].name = NULL;

   if (parse_args(func, arg, alist) == 0)
      return (NULL);

   if (extension) {
      fe = (char *)calloc(strlen(extension)+strlen(filename)+1,1);
      fe = strcat(filename,extension);
   }
   else
      fe = strdup (filename);

   data = FITS_Read_Entry(fe);
   free (fe);

   if (!data) {
      parse_error("%s failed to load\n",filename);
      return(NULL);
   }

   return(data);
}

#endif

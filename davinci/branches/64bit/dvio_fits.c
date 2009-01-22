#include "parser.h"

#if defined(HAVE_LIBCFITSIO) && ( defined(HAVE_CFITSIO_FITSIO_H) || defined(HAVE_FITSIO_H))


#ifdef HAVE_FITSIO_H
#include <fitsio.h>
#else
#include <cfitsio/fitsio.h>
#endif


#include <dvio_fits.h>

int
Write_FITS_Image(fitsfile *fptr, Var *obj)
{
  int		naxis;
  long		naxes[3];
  int		bitpix;
  int		datatype;
  int  	       *size;
  int		i;
  int		status=0;
  int		fpixel[3]={1,1,1};
  
  VarType2FitsType(obj,&bitpix,&datatype);
  
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
  fits_create_img(fptr,bitpix,naxis,naxes,&status);
  QUERY_FITS_ERROR(status);
  
  for(i=0;i<3;i++) 
    if(!(naxes[i])) size[i]=1; //I don't think this is needed, but just in case V_SIZE() returns a 0 in one of the size slots
  
  fits_write_pix(fptr,datatype,(long *)fpixel,size[0]*size[1]*size[2],(void *)V_DATA(obj),&status);
  QUERY_FITS_ERROR(status);
  
  return(1);
}


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
    v = newString(strdup(fits_value));
    break;
    
  case 'I':   
    i = (int *)calloc(1,sizeof(int));
    *i = atoi(fits_value);
    v = newVal(BSQ,1,1,1,INT,i);
    break;
  case 'L':
    i = (int *)calloc(1,sizeof(int));
    if (!strcmp(fits_value,"T"))
      *i=1;
    else
      *i=0;
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
   char      header_entry[FLEN_CARD];
   int       status=0;
   int       i,j;
   int       num_objects;
   int       cur_object;
   int       num_header_entries;
   int       object_type;
   char     *name;
   char      fits_value[128],fits_comment[128];

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
int
VarType2FitsType(Var *obj, int *bitpix, int *datatype)
{
	switch (V_FORMAT(obj)) {
		case BYTE:
			*bitpix = BYTE_IMG;
			*datatype = TBYTE;
			break;

		case SHORT:
			*bitpix = SHORT_IMG;
			*datatype = TSHORT;
			break;

		case INT:
			*bitpix = LONG_IMG; /* Yeah, I know, it says long...but it means 32-bit */
			*datatype = TINT; /*Future's so bright...*/
			break;

		case FLOAT:
			*bitpix = FLOAT_IMG;
			*datatype = TFLOAT;
			break;

		case DOUBLE:
			*bitpix = DOUBLE_IMG;
			*datatype = TDOUBLE;
			break;

		default:
			return(1);

	}

	return(0);
}

void ScratchFITS(fitsfile *fptr, char *name)
{
	int status=0;

	fits_close_file(fptr,&status);

	unlink(name);
}

/*
** Valid Object: 
** This method takes a structure and checks for a minimum of exiting
** key/value pairs that would make the strucutre a legal label for
** a FITS object in a FITS file.  The int idx flags the function
** as to whether this is the FIRST object to go into the FITS file
** and therefore as some different rule checks or if its > 1st object.
*/

int ValidObject(Var *obj, int idx)
{
	return(1);
}

int  Write_FITS_Record(fitsfile *fptr,Var *obj, char *obj_name)
{
	char	key[9];
	char	val[72];
	char	card[81];
	int	status=0;
	int	i;
	int	len = strlen(obj_name);
	int	datatype;
	int	bitpix;


/*
** Currently we deal with two davinci object types:
**   ID_STRING
**   ID_VAL (of dim 1,1,1; ie Byte, Short, etc...)
**	
**	If it's a string, we have to build an 80 character "card".
** If it's a value, we'll let the library handle formating.
*/


	if (V_TYPE(obj) == ID_STRING) {
		for(i=0;i<8;i++){
			if (i < len)
				key[i] = obj_name[i];
			else
				key[i]=' ';
		}
		key[8]='\0';

		//This is currently a mindless <72 char copy...no multi-line possibilities at this point
		strncpy(val,V_STRING(obj),70);

		val[71]='\0'; //just in case

		sprintf(card,"%s= %s",key,val);

		fits_write_record(fptr,card,&status);
		QUERY_FITS_ERROR(status);

		return(1);

	}

	if (V_TYPE(obj) ==ID_VAL) {
		if (!strcasecmp(obj_name,"simple")) //special case for the first entry for the first obj
			datatype = TLOGICAL;
		else
			VarType2FitsType(obj,&bitpix,&datatype);

		fits_write_key(fptr,datatype,obj_name,V_DATA(obj),NULL,&status);
		QUERY_FITS_ERROR(status);

		return(1);
	}

	return(1);
}


/*
** WriteSingleStrucutre:
**	This function takes a Var structure, and parses it
** using it's members names for the FITS label keys
** and member values for the key's values.  If the item
** contains a member named 'data', it is used to create
** an image object with all the assoicted bit/axes information.
** Before the object is parsed and written out the the FITS
** files, it is validated for minimal label content and correctness.
** If validation fails, a non-zero value is returned, other-wise
** the structure is written out and a zero value is returned.
*/
int WriteSingleStructure(fitsfile *fptr,Var *obj,int index)
{
	Var *element;
	int i;
	int count;
	char	*obj_name;

	if (!ValidObject(obj,index))
		return(1);

	count=get_struct_count(obj);

	for(i=0;i<count;i++){
		get_struct_element(obj,i,&obj_name,&element);
		if(!(strcasecmp(obj_name,"data"))){

			if (!(int )Write_FITS_Image(fptr,element))
				return(1);

		}

		else {
			if (!(int )Write_FITS_Record(fptr,element,obj_name))
				return(1);
		}
	}

	return(0);
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
	int i;
	Var *Tmp;
	int count;
	char *name;
	char *obj_name;
	fitsfile *fptr;
	int status=0;
/* 
** All incoming FITS object are themselves placed into a structure, thus
** obj should be strcuture of structures (at least one, anyway).  The first
** portion of validation is to check that each object in obj IS of type
** ID_STRUCT.  If it is, a single strucutre writter is called.
** Slightly different rules exist for the first structure as opposed to the
** rest, so that fact (whether this obj is the first or not the first) is
** is signaled in the parameter list.  If any strucutre fails validation,
** the file is scratched and nothing is written.
*/

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

	count = get_struct_count(obj);

	for(i=0;i<count;i++){
		get_struct_element(obj,i,&obj_name,&Tmp);

		if (V_TYPE(Tmp) != ID_STRUCT) {
			parse_error("Encountered a non-FITS item in the list of items: %s\n",obj_name);
			ScratchFITS(fptr,name);
			return(NULL);
		}

		if (WriteSingleStructure(fptr,Tmp,i)){
			parse_error("Invalid items in structure labeled: %s\nCannot write this object as a FITS file\n",obj_name);
			ScratchFITS(fptr,name);
			return(NULL);
		}
	}
	
	fits_close_file(fptr,&status);

	QUERY_FITS_ERROR(status);

	return(NULL);

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

/*
** Here we would descide if the Var was a table 
** or an image, and then do the right thing, but
** for now, we only do images.
*/

	if (!(int)Write_FITS_Image(fptr,obj)) {
		parse_error("An error was generated trying to write your data as a FITS image\n");
		ScratchFITS(fptr,name);
		return(NULL);
	}

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

   if (parse_args(func, arg, alist) == 0) return (NULL);

   if (filename == NULL) {
     parse_error("Expected filename");
     return(NULL);
   }

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

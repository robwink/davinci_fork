char     DVIO_FITS_err_text[64];

#define QUERY_FITS_ERROR(status,null_return)    if (status) { \
      fits_get_errstatus(status,DVIO_FITS_err_text); \
      parse_error("cfitsio ERROR occured:\n\t%s\n",DVIO_FITS_err_text); \
      return(null_return); }

int	Write_FITS_Image(fitsfile *fptr, Var *obj);
Var * Read_FITS_Image(fitsfile *fptr);
Var * Read_FITS_Table(fitsfile *fptr);
Var * makeVarFromFITSLabel(char *fits_value,char key_type);
void	Parse_Name(char *card, char **name);
Var * FITS_Read_Entry(char *fits_filename);
int	VarType2FitsType(Var *obj, int *bitpix, int *datatype);
void	ScratchFITS(fitsfile *fptr, char *name);
int	ValidObject(Var *obj, int idx);
int	WriteSingleStructure(fitsfile *fptr,Var *obj,int index);
Var * FITS_Write_Structure(char *fits_filename, Var *obj, int force);
Var * FITS_Write_Var(char *fits_filename, Var *obj, int force);
int	Write_FITS_Record(fitsfile *fptr,Var *obj, char *obj_name);










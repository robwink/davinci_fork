#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif /* _WIN32 */
#include "iomedley.h"

void * Read_Int(FILE *fp,int *size);
void * Read_Float(FILE *fp,int *size);
void * Read_String(FILE *fp,int *size);
void * Read_Many_Ints(FILE *fp,int *size);
void * Read_Many_Floats(FILE *fp,int *size);
void * Read_Many_Strings(FILE *fp,int *size);

typedef void * (*FuncPtr)(FILE *,int *);

char *Labels[]={"samples","lines","bands","header offset",
					 "data type","interleave","byte order","file type",NULL};
int Types[]={0,0,0,0,0,2,0,2};

#define HEADER_ENTRIES	8

typedef struct 
{
	int	samples;
	int	lines;
	int	bands;
	int	header_offset;
	char	data_type; /*1-15 valid values*/
	iom_order org;
	int	byte_order;

} ENVI_Header ; 
	
FuncPtr Read_Value[]={&Read_Int,&Read_Float,&Read_String,
							 &Read_Many_Ints,&Read_Many_Floats,
							 &Read_Many_Strings,NULL};

int 	Read_Label(FILE *,char *);
int	Find_Label_and_Type(char *,int *, int *);
void	Assign_Value(int ,void *, int ,ENVI_Header *);

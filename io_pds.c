#include "header.h"
#include "parser.h"
#include "io_lablib3.h"


void Set_Col_Var(Var **,FIELD **,LABEL * ,int *size, char **);

static char keyword_prefix[]="PTR_TO_";
static int keyword_prefix_length=8;

typedef struct _dataKeys
{
	char *Name; /*Name Entry in Table; Used for searching*/
	Var  *Obj;  /*Pointer to Parent Var obj to which the data will be assigned*/
	char *KeyValue; /*PTR_TO_<OBJECT> keyword entry's value*/
	char *FileName; /*Name of the file the PDS object is from, 
							could be different than the one originally given */
}dataKey;

/*To add dataKey words for more readable types do the following 3 steps:*/

/*Step 1: Add READ function/wrapper declaration here*/
int rf_QUBE(char *, Var *,char *);
int rf_TABLE(char *, Var *,char *);
int rf_IMAGE(char *, Var *,char *);
int rf_HISTOGRAM_IMAGE(char *, Var *,char *);
int rf_HISTORY(char *, Var *,char *);

static dataKey	*dK;
typedef int (*PFI)(char *, Var *, char *);

/*Step2: Add the name of the READ function/wrapper here*/
PFI VrF[]={rf_QUBE,rf_TABLE,rf_IMAGE,rf_HISTOGRAM_IMAGE,rf_HISTORY};


/*Step 3: Add the dataKey word to this here (before the NULL!)*/
static char *keyName[]={"QUBE","TABLE","IMAGE","HISTOGRAM_IMAGE","HISTORY",NULL};
static int num_data_Keys;



void
Init_Obj_Table(void)
{
	int i=0;
	while(keyName[i]!=NULL)
		i++;

	num_data_Keys=i;

	if (dK!=NULL){
		free(dK);
	}

	dK=(dataKey *)calloc(num_data_Keys,sizeof(dataKey));

	for (i=0;i<num_data_Keys;i++){
		dK[i].Name=keyName[i];
		dK[i].Obj=NULL;
		dK[i].KeyValue=NULL;
	}
}	

int
Match_Key_Obj(char *obj)
{
	/*Want to know if this Object is in our table*/
	
	int i;

	for (i=0;i<num_data_Keys;i++){
		if(!(strcmp(dK[i].Name,obj)))

			/*If it is, then return it's index(+1)
				By adding one, we can use 0 to indicate no match.
				Just need to remember and subtract 1 on it's return 	
				below...this should be transperent to the caller. */

			return(i+1);
	}

	return(0);
}

void
Add_Key_Word_Value(char *Val,int Index)
{
	if (Index < 1) {
		printf("Error: Table Underflow\n");
		exit(0);
	}

	if (Index > num_data_Keys) {
		printf("Error: Table Overflow\n");
		exit(0);
	}

	/*Need to subtract 1 (we added one when we sent it the first time)*/

	Index--;

	if (dK[Index].KeyValue!=NULL) {
		printf("Error: Redudant Value Entry at Index:%d\n",Index);
		exit(0);
	}

	dK[Index].KeyValue=strdup(Val);
}

void Add_Key_Obj(Var *Ob,char *fn,int Index)
{
	if (Index < 1) {
		printf("Error: Table Underflow\n");
		exit(0);
	}

	if (Index > num_data_Keys) {
		printf("Error: Table Overflow\n");
		exit(0);
	}

	/*Need to subtract 1 (we added one when we sent it the first time)*/
	Index--;

	if (dK[Index].Obj!=NULL) {
		printf("Error: Redudant Obj Entry at Index:%d\n",Index);
		exit(0);
	}

	dK[Index].Obj=Ob;
	dK[Index].FileName=strdup(fn);
}


void
Read_Object(Var *v)
{
	int i;
	for (i=0;i<num_data_Keys;i++){
		if (dK[i].Obj!=NULL && dK[i].Name!=NULL && dK[i].KeyValue!=NULL)
/*			printf("%p %s %s %s\n",dK[i].Obj,dK[i].FileName,dK[i].Name,dK[i].KeyValue);*/
			(VrF[i])(dK[i].FileName,dK[i].Obj,dK[i].KeyValue);
	}
}

int
make_int(char *number)
{
	char *base;
	char *radix;
	int r_flag=0;
	int len;
	int i=0;
	int count=0;
	int offset;
	len=strlen(number);

	/*Looking for # which signifies a Base notation integer*/

	while (i<len) {
		if (number[i]=='#'){
			r_flag=1;
			break;
		}
		i++;
	}

	if(!(r_flag)) /*Didn't find it, regular int*/
		return(atoi(number));
	
	/*Gotta convert it!*/
	
	number[i]='\0'; /*Null it at first #*/
	radix=strdup(number);
	i++;
	offset=i; /*Start string here now*/

	while (i<len) {
		if (number[i]=='#') { /*Other one*/
			number[i]='\0'; 
			base=strdup((number+offset));
			return((int) strtoll(base,NULL,atoi(radix)));
		}
		i++;
	}

	return(0); /*No 2nd #? Then it's junk*/
}
/*
void  
do_obj(OBJDESC *ob,Var *v)
{
	char *name;
	KEYWORD *kwd;
	Var *o=new_struct(0);

	if (ob->class != NULL)
		name=strdup(ob->class);

	else if((kwd = OdlFindKwd(ob, "NAME", NULL,1, ODL_THIS_OBJECT))!=NULL)
		name=strdup(kwd->name);

	else{
		parse_error("Object with no name is unusable");
		return(NULL);
	}
	add_struct(v,name,o);
}
*/
		

Var * 
do_key(OBJDESC *ob, KEYWORD* key)
{

	unsigned short kwv;
	Var *o=NULL;
	int	*i;
	float *f;

			kwv=OdlGetKwdValueType(key);
	
			switch (kwv) {
	
			case ODL_INTEGER:
				i=(int *)calloc(1,sizeof(int));
				*i=make_int(key->value);
				o=newVal(BSQ,1,1,1,INT,i);
				break;
			case ODL_REAL:
				f=(float *)calloc(1,sizeof(float));
				*f=atof(key->value);
				o=newVal(BSQ,1,1,1,FLOAT,f);
				break;
			case ODL_SYMBOL:
			case ODL_DATE:
			case ODL_DATE_TIME:
				o=newVar();
				V_TYPE(o)=ID_STRING;
				V_STRING(o)=strdup(key->value);
				break;	
			case ODL_TEXT:
				{
					int len=strlen(key->value);
					int i,count=0,delta;
					char *start,*stop;
					for (i=0;i<len;i++){
						if(key->value[i]=='\n')
							count++;
					}
					count++;/*In case text doesn't end with a /n*/
					o=newVar();
					if (count==0){
						V_TYPE(o)=ID_STRING;
						V_STRING(o)=strdup(key->value);
						break;
					}
					V_TYPE(o)=ID_TEXT;
					V_TEXT(o).Row=count;
					V_TEXT(o).text=(unsigned char**)calloc(count,sizeof(char *));
					count=0;
					start=key->value;
					stop=strchr(key->value,'\n');
					while(stop!=NULL){
						delta=stop-start;
						V_TEXT(o).text[count]=(unsigned char*)calloc(delta+1,sizeof(char));
						memcpy(V_TEXT(o).text[count],start,(delta+1));
						V_TEXT(o).text[count][delta]='\0';
						count++;
						start=stop+1;
						stop=strchr(start,'\n');
					}
					delta=strlen(start);
					if (delta){
						V_TEXT(o).text[count]=(unsigned char*)calloc(delta+1,
																					sizeof(char));
						memcpy(V_TEXT(o).text[count], start, delta);
						V_TEXT(o).text[count][delta]='\0';
					}
				}
					
				break;
			case ODL_SEQUENCE:
			case ODL_SET:
				{
					char **stuff;
					int num;
					int i;
					num=OdlGetAllKwdValuesArray(key,&stuff);
					if (num){
						o=newVar();
						V_TYPE(o)=ID_TEXT;
						V_TEXT(o).Row=num;
						V_TEXT(o).text=(unsigned char **)calloc(num,sizeof(char *));
						for (i=0;i<num;i++){
							V_TEXT(o).text[i]=strdup(stuff[i]);
						}
					}
					else {
						o=newVar();
						V_TYPE(o)=ID_STRING;
						V_STRING(o)=(char *)calloc(1,sizeof(char));
						V_STRING(o)[0]='\0';
					}
				}
				break;

			default:
					parse_error("Unknown PDS value type...Setting as string");
					o=newVar();
					V_TYPE(o)=ID_STRING;
					if (key->value!=NULL)
						V_STRING(o)=strdup(key->value);
					else {
						V_STRING(o)=(char *)calloc(1,sizeof(char));
						V_STRING(o)[0]='\0';
					}

			}	
	return (o);
}

char *
mod_name_if_necessary(char *name)
{
	char *new_name;
	if (name[0]!='^')
		return(name);

	new_name=(char *)calloc(strlen(&name[1])+
							keyword_prefix_length,sizeof(char));

	strcpy(new_name,keyword_prefix);
	strcat(new_name,&name[1]);
	return(new_name);
}

void
Traverse_Tree(OBJDESC * ob,Var *v)
{
	KEYWORD * key;
	int offset=strlen(keyword_prefix);
	OBJDESC *next_ob=NULL;
	unsigned short scope=ODL_CHILDREN_ONLY;
	int count=0;
	Var *next_var;
	char *name;
	char *keyname;
	KEYWORD *tmp_key;
	int idx;


	/*count the number of child OBJECTS of ob */
	next_ob=OdlNextObjDesc(ob,0,&scope);
	if (next_ob!=NULL)
			count=1;
	while (next_ob != NULL){
		next_ob=OdlNextObjDesc(next_ob,0,&scope);
		if (next_ob != NULL)
			count++;
	}

	/*Reset next_ob back to the first child (if there is one)*/
	scope=ODL_CHILDREN_ONLY;
	next_ob=OdlNextObjDesc(ob,0,&scope);


	/*Iterate through keywords and objects until we run out of one or the other*/

	key=OdlGetFirstKwd(ob);

	while (key!=NULL && count) {


		/*If Key and Next_ob are from different files, keyword takes precedent*/
		if ((strcmp(key->file_name,next_ob->file_name))){
			keyname=mod_name_if_necessary(key->name);

			/*Check to see if this is a pointer*/
			if (key->is_a_pointer){/* then check to see if it's an object we want*/
				if ((idx=Match_Key_Obj(&key->name[1]))) {/* It is */
					Add_Key_Word_Value(key->value,idx);
				}
			}

			/*Now add the name to the structure*/
			add_struct(v,keyname,do_key(ob,key));
         key=OdlGetNextKwd(key);
      }

		/*Same filename, so check line numbers*/
		/*Keyword before object*/
		else if (key->line_number < next_ob->line_number) {
				keyname=mod_name_if_necessary(key->name);

				/*Check to see if this is a pointer*/
				if (key->is_a_pointer){/* then check to see if it's an object we want*/
					if ((idx=Match_Key_Obj(&key->name[1]))) {/* It is */
						Add_Key_Word_Value(key->value,idx);
					}
				}

				add_struct(v,keyname,do_key(ob,key));
				key=OdlGetNextKwd(key);
		}


		/*object before keyword*/
		else {
			name=NULL;
			tmp_key=(OdlFindKwd(next_ob, "NAME", NULL,1, ODL_THIS_OBJECT));
			if(tmp_key!=NULL)
				if(tmp_key->value!=NULL) 
					name=tmp_key->value;

			if (name==NULL)/*Still no name*/
				name=next_ob->class;

			next_var=new_struct(0);
			add_struct(v,name,next_var);

			/*Check to see if this is an object we want*/
			if ((idx=Match_Key_Obj(next_ob->class))) {/* It is */
				Add_Key_Obj(next_var,next_ob->file_name,idx);
			}
			
			Traverse_Tree(next_ob,next_var);

			next_ob= OdlNextObjDesc(next_ob,0,&scope);

			count--;
		}
	}

	/*Okay, somebody is depleted; keyword or children of ob */

	if (key==NULL) {/*Only children of ob left on this level*/
		while(count){
			name=NULL;
			tmp_key=(OdlFindKwd(next_ob, "NAME", NULL,1, ODL_THIS_OBJECT));
			if(tmp_key!=NULL)
				if(tmp_key->value!=NULL) 
					name=tmp_key->value;
			
			if(name==NULL)
				name=next_ob->class;

			next_var=new_struct(0);
			add_struct(v,name,next_var);

			/*Check to see if this is an object we want*/
			if ((idx=Match_Key_Obj(next_ob->class))) {/* It is */
				Add_Key_Obj(next_var,next_ob->file_name,idx);
			}

			Traverse_Tree(next_ob,next_var);
			next_ob=OdlNextObjDesc(next_ob,0,&scope);
			count--;
		}
	}

	else {/*Only keywords left*/
		while (key!=NULL){
			keyname=mod_name_if_necessary(key->name);

			/*Check to see if this is a pointer*/
			if (key->is_a_pointer){/* then check to see if it's an object we want*/
				if ((idx=Match_Key_Obj(&key->name[1]))) {/* It is */
					Add_Key_Word_Value(key->value,idx);
				}
			}

			add_struct(v,keyname,do_key(ob,key));
			key=OdlGetNextKwd(key);
		}
	}
/*everything finished for this ob so return*/
}




Var *
ReadPDS(vfuncptr func, Var *arg)
{

   Alist alist[2];
   int ac;
   Var **av;

	OBJDESC * ob;
	char *err_file=NULL;
	int	obn=1;
	char *fn;

	FILE *fp;

	Var *v = new_struct(0);

   alist[0] = make_alist( "filename", ID_STRING,   NULL,     &fn);
   alist[1].name = NULL;

   make_args(&ac, &av, func, arg);
   
   if (parse_args(ac, av, alist)) return(NULL);
   


	if((fp=fopen(fn,"r"))==NULL){
		parse_error("Can't find file");
		return(NULL);
	}

	fclose(fp);


	Init_Obj_Table();

	ob = (OBJDESC *)OdlParseLabelFile(fn, err_file,ODL_EXPAND_STRUCTURE, 0);

	
	Traverse_Tree(ob,v);

	Read_Object(v);

	return(v);
}
int rf_QUBE(char *fn, Var *ob,char * val)
{
	FILE *fp;
	Var *data=NULL;
	fp=fopen(fn,"r");
	data=LoadISIS(fp,fn,NULL);
	if (data!=NULL){
		add_struct(ob,"DATA",data);
		return(0);
	}

	return(1);
}
int rf_TABLE(char *fn, Var *ob,char * val)
{
	LABEL *label;
	Var *Data;
	char **Bufs;
	char *tmpbuf;
	int i,j;
	int fp;
	int Offset;
	FIELD **f;
	int step;
	int *size;
	int err;

	label=LoadLabel(fn);
	f = (FIELD **) label->fields->ptr;

/*Add new structure to parent ob*/
	Data=new_struct(0);

	/*Initialize a set of buffers to read in the data*/
	
	Bufs=(char **)calloc(label->nfields,sizeof(char *));
	tmpbuf=(char *)calloc(label->reclen,sizeof(char));
	size=(int *)calloc(label->nfields,sizeof(int));
	for (j=0;j<label->nfields;j++){

		if (f[j]->dimension)
			size[j]=f[j]->size*f[j]->dimension;
		else
			size[j]=f[j]->size;

		Bufs[j]=(char *)calloc((label->nrows*size[j]),sizeof(char));
	}
 	Offset=(atoi(val)-1)*label->reclen;	
	fp=open(fn,O_RDONLY,0);
	lseek(fp,Offset,SEEK_SET);
	for (i=0;i<label->nrows;i++){

		/*Read each row as a block*/
		if((err=read(fp,tmpbuf,label->reclen))!=label->reclen){
			fprintf(stderr,"Bad data in file:%s\n Read %d byte(s), should be %d\n",
								fn,err,label->reclen);
			return(1);
		}

		step=0;

		for(j=0;j<label->nfields;j++){

			/*Place in the approiate buffer*/
			memcpy((Bufs[j]+i*size[j]),(tmpbuf+step),size[j]);
			step+=size[j];
		}
	}
	close(fp);

	/*Set each field Var's data and parameter information*/
	Set_Col_Var(&Data,f,label,size,Bufs);

	add_struct(ob,"DATA",Data);

	free(tmpbuf);
	free(size);
	for (j=0;j<label->nfields;j++){	
		free(Bufs[j]);
	}
	free(Bufs);
	return(0);
}
double
Scale(int size, void *ptr, FIELD *f)
{
	char	*cp;
	int	*ip;
	short *sp;
	float *fp;
	double *dp;
	char num[9];

/*Set up pointer casts for our data type*/

	cp=ptr;
	ip=ptr;
	sp=ptr;
	fp=ptr;
	dp=ptr;



	switch(f->eformat) {

		case MSB_INTEGER:
      case MSB_UNSIGNED_INTEGER:
			switch(f->size) {
			case 4:
					return((double)ip[0]*f->scale+f->offset);
			case 2:
					return((double)sp[0]*f->scale+f->offset);
			case 1:
					return((double)cp[0]*f->scale+f->offset);
			}
			break;
		
		case IEEE_REAL:
			switch(f->size) {
			case 8:
					return(dp[0]*f->scale+f->offset);
			case 4:
					return((double)fp[0]*f->scale+f->offset);
			}
       break;
		case ASCII_INTEGER:
			memcpy(num,cp,f->size);
			num[f->size]='\0';
			return((double)(atoi(num))*f->scale+f->offset);
        
         break;
      case ASCII_REAL:
			memcpy(num,cp,f->size);
			num[f->size]='\0';
			return((double)(atof(num))*f->scale+f->offset);
		}
	return (0);
}

	
char *
DoScale(FIELD **f,LABEL *label,char *ob)
{
	char *Bufs;
	int rows;
	int size;
	int dim;
	void *ptr;
	
	double Val;
	int index=0;
	int i,j;


	rows=label->nrows;
	size=f[0]->size;
	dim=(f[0]->dimension ? f[0]->dimension : 1);

	Bufs=(char *)calloc(rows*8*dim,sizeof(char));
	ptr=ob;
	for(i=0;i<rows;i++){
		for (j=0;j<dim;j++){	
			Val=Scale(size,ptr,f[0]);
			memcpy((Bufs+index),&Val,8);
			index+=8;
			ptr+=size;
		}
	}
	f[0]->eformat=IEEE_REAL;
	f[0]->size=8;
	free(ob);
	return(Bufs);
}

void
Set_Col_Var(Var **Data,FIELD **f,LABEL *label,int *size, char **Bufs)
{
	int j,i;
	void *data;
	char **text;
	Var *v;
	char num[9];
	int inum;
	double fnum;
	int step;
	int dim;
	

	for (j=0;j<label->nfields;j++){
		dim=(f[j]->dimension ? f[j]->dimension : 1);
		step=0;
		if (f[j]->scale)
			Bufs[j]=DoScale(&f[j],label,Bufs[j]);
		switch(f[j]->eformat) {
		case CHARACTER:
			text=(char **)calloc(label->nrows,sizeof(char *));
			for (i=0;i<label->nrows;i++){
				text[i]=(unsigned char *)calloc(size[j]+1,sizeof(char));
				memcpy(text[i],(Bufs[j]+i*size[j]),size[j]);
				text[i][size[j]]='\0';
			}
			v=newText(label->nrows,text);
			break;

		case MSB_INTEGER:
		case MSB_UNSIGNED_INTEGER:
			data=calloc(size[j]*label->nrows,sizeof(char));
			memcpy(data,Bufs[j],size[j]*label->nrows);
			switch (f[j]->size) {
				case 4:
					v=newVal(BSQ,dim,label->nrows,1,INT,data);
					break;
				case 2:
					v=newVal(BSQ,dim,label->nrows,1,SHORT,data);
					break;
				case 1:
					v=newVal(BSQ,dim,label->nrows,1,BYTE,data);
			 }
			break;

		case IEEE_REAL:
			/*Easier to make a newVal, so free current instance*/
			data=calloc(size[j]*label->nrows,sizeof(char));
			memcpy(data,Bufs[j],size[j]*label->nrows);
			switch (f[j]->size) {
				case 8:
					v=newVal(BSQ,dim,label->nrows,1,DOUBLE,data);
					break;
				case 4:
					v=newVal(BSQ,dim,label->nrows,1,FLOAT,data);
					break;
			}
			break;

		case ASCII_INTEGER:
			data=calloc(sizeof(int)*label->nrows,sizeof(char));
			for (i=0;i<(label->nrows*dim);i++){
				memcpy(num,Bufs[j]+f[j]->size*i,f[j]->size);
				num[f[j]->size]='\0';
				inum=atoi(num);
				memcpy(data+step,&inum,sizeof(int));
				step+=sizeof(int);
			}

			v=newVal(BSQ,dim,label->nrows,1,INT,data);
			break;

		case ASCII_REAL:
			data=calloc(sizeof(double)*label->nrows,sizeof(char));
			for (i=0;i<(label->nrows*dim);i++){
				memcpy(num,Bufs[j]+f[j]->size*i,f[j]->size);
				num[f[j]->size]='\0';
				fnum=atof(num);
				memcpy(data+step,&fnum,sizeof(double));
				step+=sizeof(double);
			}

			v=newVal(BSQ,dim,label->nrows,1,DOUBLE,data);
			break;

		case BYTE_OFFSET:
			data=calloc(size[j]*label->nrows,sizeof(char));
			memcpy(data,Bufs[j],size[j]*label->nrows);
			v=newVal(BSQ,dim,label->nrows,1,INT,data);
			break;

		case MSB_BIT_FIELD:
			data=calloc(size[j]*label->nrows,sizeof(char));
			memcpy(data,Bufs[j],size[j]*label->nrows);
			v=newVal(BSQ,dim,label->nrows,1,INT,data);
			break;

		}
		add_struct(*Data,f[j]->name,v);
	}	
}

int rf_IMAGE(char *fn, Var *ob,char * val)
{
	return(0);
}
int rf_HISTOGRAM_IMAGE(char *fn, Var *ob,char * val)
{
	return(0);
}
int rf_HISTORY(char *fn, Var *ob,char * val)
{
	return(0);
}


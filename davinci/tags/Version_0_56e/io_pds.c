#include "parser.h"
#include "/tes/src/vanilla/io_lablib3.h"

void
Indent(int i)
{
	int j;
	for (j=0;j<i;j++){
		printf("  ");
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
				}
				break;
			}	
	return (o);
}


void
Traverse_Tree(OBJDESC * ob,Var *v)
{
	KEYWORD * key;
	OBJDESC *next_ob=NULL;
	unsigned short scope=ODL_CHILDREN_ONLY;
	int count=0;
	Var *next_var;
	char *name;


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
			add_struct(v,key->name,do_key(ob,key));
         key=OdlGetNextKwd(key);
      }

		else {
			/*Same filename, so check line numbers*/
			/*Keyword before object*/
			if (key->line_number < next_ob->line_number && !(key->is_a_pointer)) {
				add_struct(v,key->name,do_key(ob,key));
				key=OdlGetNextKwd(key);
			}

			/*special case of obj/key entry, next_ob should be it*/
			else if (key->is_a_pointer){
				next_var=new_struct(0);
				if (key->name[0]=='^') /*Eeww, Don't like carrots */
					add_struct(v,(key->name+1),next_var);
				else
					add_struct(v,key->name,next_var);

				Traverse_Tree(next_ob,next_var);
			
				next_ob= OdlNextObjDesc(next_ob,0,&scope);

				count--;

				/*Need to step past this keyword since it was really an object*/
				key=OdlGetNextKwd(key);
			}

			/*object before keyword*/
			else {
				name=(OdlFindKwd(next_ob, "NAME", NULL,1, ODL_THIS_OBJECT))->value;
				if (name==NULL){/*Oh man, no name!*/
					name=next_ob->class;
				}

				next_var=new_struct(0);
				add_struct(v,name,next_var);
				
				Traverse_Tree(next_ob,next_var);
	
				next_ob= OdlNextObjDesc(next_ob,0,&scope);

				count--;
			}
		}
	}

	/*Okay, somebody is depleted; keyword or children of ob */

	if (key==NULL) {/*Only children of ob left on this level*/
		while(count){
			name=(OdlFindKwd(next_ob, "NAME", NULL,1, ODL_THIS_OBJECT))->value;
			if (name==NULL){/*Oh man, no name!*/
				name=next_ob->class;
			}

			next_var=new_struct(0);
			add_struct(v,name,next_var);
			Traverse_Tree(next_ob,next_var);
			next_ob=OdlNextObjDesc(next_ob,0,&scope);
			count--;
		}
	}

	else {/*Only keywords left*/
		while (key!=NULL){
			add_struct(v,key->name,do_key(ob,key));
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


	ob = (OBJDESC *)OdlParseLabelFile(fn, err_file,ODL_EXPAND_STRUCTURE, 0);

	
	Traverse_Tree(ob,v);

	return(v);
}


#include "parser.h"
#include <libgen.h>

/**
 ** read a text file into BYTE data
 **/
Var *
ff_text(vfuncptr func, Var *arg)
{
    char    *filename;
    Var     *v, *e, *s;
    char    *fname;
    FILE *fp;
    char *ptr;
    int rlen;

    unsigned char *cdata;
    int count=0;

    int i,j,k;

    int dsize;
    int x=0;
    int y=0;

    struct keywords kw[] = {
        { "filename", NULL },   /* filename to read */
        { NULL, NULL }
    };

    if (evaluate_keywords(func, arg, kw)) {
        return(NULL);
    }

    if ((v = get_kw("filename", kw)) == NULL) {
        sprintf(error_buf, "No filename specified: %s()", func->name);
        parse_error(NULL);
        return(NULL);
    }
    if (V_TYPE(v) != ID_STRING) {
        e = eval(v);
        if (e == NULL || V_TYPE(e) != ID_STRING) {
            sprintf(error_buf, "Illegal argument: %s(... filename=...)", 
                    func->name);
            parse_error(NULL);
            return(NULL);
        }
        v = e;
    }
    filename = V_STRING(v);

    if ((fname = locate_file(filename)) == NULL) {
        sprintf(error_buf, "Cannot find file: %s\n", filename);
        parse_error(NULL);
        return(NULL);
    }
    if ((fp = fopen(fname, "r")) == NULL) {
        sprintf(error_buf, "Cannot open file: %s\n", fname);
        parse_error(NULL);
        return(NULL);
    }
        /**
         ** Determine the file size.  X is max of all line lengths.
         **/

	x = y = 0;
	count = 0;
	while(getline(&ptr, fp) != EOF) {
		if ((int)strlen(ptr) > x) 
			x = (int)strlen(ptr);
		y++;
		count += (int)strlen(ptr);
	}
	rewind(fp);

    dsize = x*y;
    cdata = (unsigned char *)calloc(1, dsize);

	for (j = 0 ; j < y ; j++) {
		if ((rlen = getline(&ptr, fp)) == -1) break;
		memcpy(cdata+(x*j), ptr, strlen(ptr));
	}

    if (VERBOSE > 1) {
		fprintf(stderr, "Read TEXT file: %dx%d (%d bytes)\n", x,y,count);
    }

    s = newVar();
    V_TYPE(s) = ID_VAL;
    V_DATA(s) = cdata;
    V_FORMAT(s) = BYTE;
    V_ORG(s) = BSQ;
    V_DSIZE(s) = dsize;
    V_SIZE(s)[0] = x;
    V_SIZE(s)[1] = y;
    V_SIZE(s)[2] = 1;

    return(s);
}

Var *
ff_textarray(vfuncptr func, Var *arg)
{
    char    *filename;
    Var     *v, *e, *s;
    char    *fname;
    FILE *fp;
    char *ptr;
    int rlen;
	
	 Var *o;

    int i,j,k;
	 int count;

    struct keywords kw[] = {
        { "filename", NULL },   /* filename to read */
        { NULL, NULL }
    };

    if (evaluate_keywords(func, arg, kw)) {
        return(NULL);
    }

    if ((v = get_kw("filename", kw)) == NULL) {
        sprintf(error_buf, "No filename specified: %s()", func->name);
        parse_error(NULL);
        return(NULL);
    }
    if (V_TYPE(v) != ID_STRING) {
        e = eval(v);
        if (e == NULL || V_TYPE(e) != ID_STRING) {
            sprintf(error_buf, "Illegal argument: %s(... filename=...)", 
                    func->name);
            parse_error(NULL);
            return(NULL);
        }
        v = e;
    }
    filename = V_STRING(v);

    if ((fname = locate_file(filename)) == NULL) {
        sprintf(error_buf, "Cannot find file: %s\n", filename);
        parse_error(NULL);
        return(NULL);
    }
    if ((fp = fopen(fname, "r")) == NULL) {
        sprintf(error_buf, "Cannot open file: %s\n", fname);
        parse_error(NULL);
        return(NULL);
    }
        /**
         ** Determine the file size.  X is max of all line lengths.
         **/


	o=newVar();
	

	count = 0;
	while(getline(&ptr, fp) != EOF) {
		count ++;
	}
	rewind(fp);

    V_TEXT(o).Row=count;
    V_TEXT(o).text = (unsigned char **)calloc(sizeof(char *), V_TEXT(o).Row);

	for (j = 0 ; j < count;j++) {
		if ((rlen = getline(&ptr, fp)) == -1) break;
		V_TEXT(o).text[j]=strdup(ptr);
		if (V_TEXT(o).text[j][strlen(V_TEXT(o).text[j])-1]=='\n')
			V_TEXT(o).text[j][strlen(V_TEXT(o).text[j])-1]='\0';
	}

    if (VERBOSE > 1) {
		fprintf(stderr, "Read TEXT file: %d lines\n", count);
    }

    V_TYPE(o) = ID_TEXT;

    return(o);
}

/**
 ** extract from BYTE using delim
 **/

Var *
ff_delim(vfuncptr func, Var *arg)
{
	Var *ob, *cval, *s, *v, *e;
	char *delim;
	char *cdata;
	char *ptr;
	int item, count;

    struct keywords kw[] = {
        { "object", NULL },   	/* object to read */
		{ "delim",    NULL },   /* string of delimters to use */
		{ "count",    NULL },   /* string of delimters to use */
        { NULL, NULL }
    };

    if (evaluate_keywords(func, arg, kw)) {
        return(NULL);
    }

    if ((v = get_kw("object",kw)) == NULL) {
        sprintf(error_buf, "No object specified: %s()\n", func->name);
        parse_error(NULL);
        return(NULL);
    }
    e = eval(v);
    if (e == NULL) {
        parse_error("Illegal argument to read_text(...object=...)");
        return(NULL);
    }
	if (V_TYPE(e) == ID_VAL) {
		if (V_FORMAT(e) != BYTE) {
			parse_error("Illegal argument to read_text(...object=...), must be BYTE");
			return(NULL);
		}
	} else if (V_TYPE(e) != ID_STRING) {
        parse_error("Illegal argument to read_text(...object=...), must be BYTE");
        return(NULL);
	}
    ob = e;
	
	if ((cval = RequireKeyword("count",  kw, ID_VAL, INT,  func)) == NULL) 
		return(NULL);
	item = V_INT(cval);

	if (item <= 0)  {
		sprintf(error_buf, "%s(), count must be greater than 0", func->name);
		parse_error(NULL);
		return(NULL);
	}

    if ((v = get_kw("delim", kw)) != NULL) {
		if ((e = eval(v)) != NULL) v = e;
		if (V_TYPE(v) != ID_STRING) {
			sprintf(error_buf, "%s(), delim must be a STRING", func->name);
		}
    }
	delim = V_STRING(v);

	if (V_TYPE(ob) == ID_VAL) {
		cdata = strdup((char *)V_DATA(ob));
	} else {
		cdata = strdup((char *)V_STRING(ob));
	}

	ptr = cdata;
	count = 0;
	while((ptr = strtok(ptr, delim)) != NULL) {
		count++;
		if (count == item) {
			s = newVar();
			V_TYPE(s) = ID_STRING;
			V_STRING(s) = strdup(ptr);
			free(cdata);
			return(s);
		}
		ptr = NULL;
	}

	if (VERBOSE) {
		fprintf(stderr, "%s(), Unable to find delimiter %d\n", func->name, item);
	}
	return(NULL);
}


Var *
textarray_subset(Var *v, Var *range)
{
	Range *r=V_RANGE(range);

	int i,lo[2],hi[2],step[2];

	Var *o;

	int counter=0;

	for (i=0;i<2;i++){
		lo[i]=r->lo[i];
		hi[i]=r->hi[i];
		step[i]=r->step[i];
		if (lo[i]==0) lo[i]=1;
		if (hi[i]==0)
			if (i==1)
				hi[i]=V_TEXT(v).Row;
			else
				hi[i]=INT_MAX; /*This is to fool it into using full length of string on given row*/
		lo[i]--;
		hi[i]--;
		if (hi[i] < lo[i]){
			 parse_error("Illegal Range value\n");
         return(NULL);
      }

		if (lo[i] < 0 || hi[i] < 0 || step[i] < 0){
			parse_error("Illegal Range value\n");
			return(NULL); 
		}

		if (step[i] == 0) step[i]=1;
	}

	if (hi[1]>=V_TEXT(v).Row){
		parse_error("Illegal range value\n");
		return(NULL);
	}


	o=newVar();

	V_TEXT(o).Row=((hi[1]-lo[1])/step[1])+1;
	if (V_TEXT(o).Row <=0){
		printf("Hey! You can't do that!\n");
		return(NULL);
	}

	V_TEXT(o).text=(unsigned char **)calloc(sizeof(char *),V_TEXT(o).Row);
	V_TYPE(o)=ID_TEXT;

	for (i=lo[1];i<=hi[1];i+=step[1]){
		if (lo[0] >= strlen(V_TEXT(v).text[i])) {
			V_TEXT(o).text[counter]=(unsigned char *)calloc(1,1);
			V_TEXT(o).text[counter][0]='\0';
		}
		else /*if (hi[0]!=0)*/{
			if ((hi[0]-lo[0]+1) > strlen(V_TEXT(v).text[i])){
				V_TEXT(o).text[counter]=strdup((V_TEXT(v).text[i]+lo[0]));
			}
			else {
				V_TEXT(o).text[counter]=(unsigned char *)calloc(1,(hi[0]-lo[0]+1)+1);
				memcpy(V_TEXT(o).text[counter],(V_TEXT(v).text[i]+lo[0]),(hi[0]-lo[0]+1));
				V_TEXT(o).text[counter][(hi[0]-lo[0]+1)]='\0';
			}
		}
/*
		else {
			V_TEXT(o).text[counter]=strdup((V_TEXT(v).text[i]+lo[0]));
		}
*/

		counter++;
	}
	

	if (counter==1){ /*What were really have is a string */

		V_TYPE(o)=ID_STRING;
		V_STRING(o)=(V_TEXT(o).text[0]);
	}
		
	return(o);
}

char *
string_dirname(Var *ob1)
{
	char *s;
	int i;
	int Flag=1;

	if (V_STRING(ob1)==NULL)
		return(NULL);

	i=strlen(V_STRING(ob1));

	while ((i--)>=0) {
		if (V_STRING(ob1)[i]=='/'){
			s=(char *)calloc(sizeof(char),i+1);
			strncpy(s,V_STRING(ob1),i+1);
			s[i]='\0';
			return (s);
		}
	}
	s=strdup(".");
	return(s);
}

Var *
text_dirname(Var *ob1)
{
	int i;
	Var *S=newVar();
	Var *Tmp=newVar();
	V_TYPE(Tmp)=ID_STRING;
	V_TYPE(S)=ID_TEXT;
	V_TEXT(S).Row=V_TEXT(ob1).Row;
	V_TEXT(S).text=(unsigned char **)calloc(sizeof(char *),V_TEXT(ob1).Row);
	for (i=0;i<V_TEXT(ob1).Row;i++){
		V_STRING(Tmp)=V_TEXT(ob1).text[i];
		V_TEXT(S).text[i]=string_dirname(Tmp);
	}
	V_STRING(Tmp)=NULL;
// 	free_var(Tmp);	
	return(S);
}




char *
string_basename(Var *ob1)
{
	char *s;
	int i;
	int len;

	if (V_STRING(ob1)==NULL)
		return(NULL);

	len=i=strlen(V_STRING(ob1));
	

	while ((i--)>=0) {
		if (V_STRING(ob1)[i]=='/'){
			if ((i+1)==len){/*No name*/
				s=(char *)calloc(1,1);
				s[0]='\0';
			}
			else {
				s=strdup((V_STRING(ob1)+i+1));
			}
			return(s);
		}
	}

	s=strdup((V_STRING(ob1)));

	return(s);
}

Var *
text_basename(Var *ob1)
{
	int i;
	Var *S=newVar();
	Var *Tmp=newVar();
	V_TYPE(Tmp)=ID_STRING;
	V_TYPE(S)=ID_TEXT;
	V_TEXT(S).Row=V_TEXT(ob1).Row;
	V_TEXT(S).text=(unsigned char **)calloc(sizeof(char *),V_TEXT(ob1).Row);
	for (i=0;i<V_TEXT(ob1).Row;i++){
		V_STRING(Tmp)=V_TEXT(ob1).text[i];
		V_TEXT(S).text[i]=string_basename(Tmp);
	}
	V_STRING(Tmp)=NULL;
// 	free_var(Tmp);	
	return(S);
}


Var *
ff_filename(vfuncptr func, Var * arg) 
{

	 Var *ob1;
    int ac;
    Var **av;
	 Var *S;
	 int filefunc;
    Alist alist[2];
    alist[0] = make_alist( "obj", ID_UNK,   NULL,     &ob1);
    alist[1].name = NULL;

	 filefunc=func->fdata;
	 if (filefunc==NULL){
		parse_error("Bad function");
		return(NULL);
	 }	

    make_args(&ac, &av, func, arg);
    if (parse_args(ac, av, alist)) return(NULL);

	 if (ob1==NULL){
		return(NULL);
	 }


	 if (V_TYPE(ob1)==ID_STRING){
	 	S=newVar();
		V_TYPE(S)=ID_STRING;
		if (filefunc==1){
			V_STRING(S)=string_basename(ob1);
		}

	   else if (filefunc==2){
			V_STRING(S)=string_dirname(ob1);
		}

		else {
			parse_error("Bad Functions");
			return(NULL);
		}
		return(S);
	 }
	
 	 else if (V_TYPE(ob1)==ID_TEXT){
		if (filefunc==1){
			S=text_basename(ob1);
		}
		else if (filefunc==2){
			S=text_dirname(ob1);
		}

		else {
         parse_error("Bad Functions");
         return(NULL);
      }
		return(S);
	 }

	 else {
		parse_error("Only STRING and TEXT types are allowed");
		return(NULL);
	 }

}


Var *
ff_grep(vfuncptr func, Var * arg)
{

    Var *ob1;
    int ac;
    Var **av;
    Var *S;
	 char *s1=NULL,*newcursor=NULL,*ptr=NULL;
    int count=0;
	 int index=0;
	 int i;
    Alist alist[3];
    alist[0] = make_alist( "obj", ID_UNK,   NULL,     &ob1);
    alist[1] = make_alist( "pattern", ID_STRING,   NULL,     &s1);
    alist[2].name = NULL;
    
    make_args(&ac, &av, func, arg);
    if (parse_args(ac, av, alist)) return(NULL);
    
    if (ob1==NULL || s1==NULL){
      return(NULL);
    }

	 if (V_TYPE(ob1)!=ID_TEXT){
		parse_error("Can only grep Text Arrays\n");
		return(NULL);	
	 }
	 ptr = regcmp(s1, (char *)0);/*Compiles expression space*/
	 for (i=0;i<V_TEXT(ob1).Row;i++){
		newcursor = regex(ptr, V_TEXT(ob1).text[i]);/*Does pattern search using compiled space*/
		if (newcursor!=NULL)
			count++;
	 }

	 if (count==0){
		parse_error("No Match");
		return(NULL);
	 }

	 S=newVar();
	 V_TYPE(S)=ID_TEXT;
	 V_TEXT(S).Row=count;
	 V_TEXT(S).text=(unsigned char **)calloc(sizeof(char *),count);
	 for (i=0;i<V_TEXT(ob1).Row;i++){
		newcursor = regex(ptr, V_TEXT(ob1).text[i]);
		if (newcursor!=NULL){
			V_TEXT(S).text[index++]=strdup(V_TEXT(ob1).text[i]);
	   }
	 }

	 return(S);
}


/**
 ** Program: kmp.c - KMP search algorithm
 ** Author: Noel Gorelick 
 ** Synopsis:       char *kmp_search(char *s1, char *s2);
 **
 ** This routine takes 2 strings, and searches for the first occurance of s2
 ** in s1, using the KMP search algorithm.  It returns the substring of s1
 ** where s2 begins, or NULL if s2 is not found in s1.
 **
 ** The KMP algorithm uses a dynamically computed alignment array to detect
 ** partial matches, which is computed each time this routine is called.
 **/

int
kmp(char *s1, char *s2)
{
    int l1 = strlen(s1);                        /* length of text */
    int l2 = strlen(s2);                        /* length of substring */
    int i,j, *align = (int *)calloc(l2, sizeof(int));

    /* compute the align array */

    align[0] = -1;                              /* initial value */
    for (i = 0; s2[i] != '\0'; i++) {
        j = align[i]+1;                         /* add one to prev value */
        while (j > 0 && s2[i] != s2[j-1]) {     /* check if still matches */
            j = align[j-1] + 1;                 /* step backwards if not */
        }
        align[i+1] = j;                         /* save new value */
    }

    /* search the string, using the align array to detect partial matches */

    j = 0;                                      /* j: index into s2 (sub) */
    for (i = 0 ; i < l1 ; i++) {                /* i: index into s1 (text) */
        for (;;) {
            if (s1[i] == s2[j]) {               /* got a match? */
                j++;                                /* move s2 forward */
                if (j == l2) {                      /* out of characters? */
                    free(align);
                    return(i-j+1);                   /* return match */
                }
            } else if (j != 0) {                /* no match; 0 is special */
                j = align[j];                   /* step backwards in align */
                continue;                       /* repeat inner loop */
            }
            break;
        }
    }
    free(align);
    return(-1);                          /* return NULL */
}

int 
ff_string_strstr(char *s1, char *s2)
{
	return ((kmp(s1,s2)+1));
}

Var *
ff_text_strstr(Var *ob1,char *s1)
{
	int i;
	int *data=calloc(sizeof(int),V_TEXT(ob1).Row);
	
	for (i=0;i<V_TEXT(ob1).Row;i++){
		data[i]=ff_string_strstr(V_TEXT(ob1).text[i],s1);
	}

	return(newVal(BSQ,1,V_TEXT(ob1).Row,1,INT,data));
}

Var *
ff_strstr(vfuncptr func, Var * arg)
{

    Var *ob1;
    int ac;
    Var **av;
    Var *S;
    char *s1=NULL,*newcursor=NULL,*ptr=NULL;
    int count=0;
    int index=0;
    int i;
    Alist alist[3];
    alist[0] = make_alist( "obj", ID_UNK,   NULL,     &ob1);
    alist[1] = make_alist( "pattern", ID_STRING,   NULL,     &s1);
    alist[2].name = NULL;

    make_args(&ac, &av, func, arg);
    if (parse_args(ac, av, alist)) return(NULL);

    if (ob1==NULL || s1==NULL){
      return(NULL);
    }

	 if (V_TYPE(ob1)==ID_TEXT){
		return(ff_text_strstr(ob1,s1));
	 }

	 if (V_TYPE(ob1)==ID_STRING){
		int *v=calloc(sizeof(int),1);
		*v=ff_string_strstr(V_STRING(ob1),s1);
		return(newVal(BSQ,1,1,1,INT,v));
	 }

	 else {
		parse_error("Invalid type...but thanks for playing!");
		return(NULL);
	}
}

/**
**	Text Subseting assignment:
**	unlike pp_var_set, we can "push" in text objects
**	of different sizes.  Therefore we need to copy the
**	original, release the original's copy, and then copy
**	in the modified version...oh, the fun never ends!
**/
	
Var *
set_text(Var *to,Range *r, Var *from)
{
	Var *temp;

	int i,Row;
	int lo[2],hi[2],step[2];
	char *string;
	int length;
	int height;
	int string_length;

	
	for (i=0;i<2;i++){
		lo[i]=r->lo[i];
		hi[i]=r->hi[i];
		step[i]=r->step[i];
		if (lo[i]==0) lo[i]=1;
		if (hi[i]==0)
			if (i==1)
				hi[i]=V_TEXT(to).Row;
			else
				hi[i]=INT_MAX; /*This is to fool it into using full length of string on given row*/
		lo[i]--;
		hi[i]--;
		if (hi[i] < lo[i]){
			 parse_error("Illegal Range value\n");
         return(NULL);
      }

		if (lo[i] < 0 || hi[i] < 0 || step[i] < 0){
			parse_error("Illegal Range value\n");
			return(NULL); 
		}

		if (step[i] == 0) step[i]=1;
	}
	
	string=V_STRING(from);
	length = (hi[0]-lo[0]+1);
	height = (hi[1]-lo[1])/step[1]+1;
	string_length=strlen(string);
	

	temp=V_DUP(to);

	if (V_TYPE(from)==ID_STRING){
		/*there are 4 cases: {height=1,height>1} x {string_lenght=length, string_length!=length}*/
		if (height==1){/*Don't need to destroy original array, just one line,maybe*/
			if (string_length==length){/*yeah, right*/
				for (i=0;i<length;i++){
				}
			}
		}
			
		
	}

	else {
	}




	return(from);
}

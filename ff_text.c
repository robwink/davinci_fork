#include "parser.h"
#include <sys/types.h>
#include <stdlib.h>
#include <regex.h>

#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif

extern char *__loc1; /*Global char * for regex */

Var *
newString(char *str)
{
    Var *v=newVar();
    V_TYPE(v)=ID_STRING;
    V_STRING(v) = str;
	return(v);
}

Var *
newText(int rows, char **text)
{
    Var *v=newVar();
    V_TYPE(v)=ID_TEXT;
    V_TEXT(v).Row=rows;
    V_TEXT(v).text=text;
	return(v);
}

/**
 ** read a text file into BYTE data
 **/
Var *
ff_read_text(vfuncptr func, Var *arg)
{
    char    *filename;
    Var     *v, *e, *s;
    char    *fname;
    FILE *fp;
    char *ptr;
    int rlen;

    char *cdata;
    int count=0;

    int i,j;

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

    if ((fname = dv_locate_file(filename)) == NULL) {
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
    cdata = (char *)calloc(dsize,sizeof(char));

    for (j = 0 ; j < y ; j++) {
        if ((rlen = getline(&ptr, fp)) == -1) break;
        memcpy(cdata+(x*j), ptr, strlen(ptr));
    }
	fclose(fp);

    if (VERBOSE > 1) {
        fprintf(stderr, "Read TEXT file: %dx%d (%d bytes)\n", x,y,count);
    }

	s = newVal(BSQ, x, y, 1, BYTE, cdata);

    return(s);
}

Var *
ff_read_lines(vfuncptr func, Var *arg)
{
    char    *filename;
    char    *fname;
    FILE *fp;
    char *ptr;
    int rlen;
    char **t;
    int size;
	
    Var *o;

    int count;

    Alist alist[2];
    alist[0] = make_alist( "filename", ID_STRING,   NULL,     &filename);
    alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

    if ((fname = dv_locate_file(filename)) == NULL) {
        sprintf(error_buf, "Cannot find file: %s\n", filename);
        parse_error(NULL);
        return(NULL);
    }
    if ((fp = fopen(fname, "r")) == NULL) {
        sprintf(error_buf, "Cannot open file: %s\n", fname);
        parse_error(NULL);
        return(NULL);
    }

    count = 0;
    size = 64;
    t = calloc(size, sizeof(char *));

    while((rlen = getline(&ptr, fp)) != EOF) {
        if (ptr[rlen-1] == '\n') ptr[rlen-1] = '\0';
        if (size == count) {
            t = realloc(t, size*2*sizeof(char *));
            size *= 2;
        }
        t[count++] = strdup(ptr);
    }
	 fclose(fp);

    t = realloc(t, count*sizeof(char *));

    o=newVar();
    V_TEXT(o).Row=count;
    V_TEXT(o).text = t;

    if (VERBOSE > 1) {
        fprintf(stderr, "Read TEXT file: %d lines\n", count);
    }

    V_TYPE(o) = ID_TEXT;

    return(o);
}

/**
 ** extract collum from ID_TEXT using a delimiter
 **/

Var *
ff_delim_textarray(Var *ob,int item,char *delim)
{
    int i;
    int j;
    int Row;
    int End=0;
    int length;
    char *text;
    int hit;
    int index=0;

    Var *s;

    int Max=25;
    char *buffer;

    if (strlen(delim) > 1) {
        parse_error("Single character delimiters only");
        return(NULL);
    }

    s=newVar();
    V_TYPE(s)=ID_TEXT;
    V_TEXT(s).Row=Row=V_TEXT(ob).Row;
    V_TEXT(s).text=(char **)calloc(Row,sizeof(char *));
    buffer=(char *)calloc(Max,sizeof(char));


    for (i=0;i<Row;i++){
        j=0; hit=0; index=0;
        End=0;
        text=V_TEXT(ob).text[i];
        length=strlen(text);	
        memset(buffer,0,Max);
        while((j<length) && !(End)){
            if (text[j]==*delim) {
                /*Need to handle multiples copies of adjactent delimiters:
                  xxxxxxxcccxxxxxxxxcxxxxxxcc where delim=c */
                while (j<length && text[j] == *delim)
                    j++;
                j--; /*We'll be advancing j up ahead,so we need to back off one*/
                hit++;
            }
            else if ((hit+1)==item){
                if (j>=Max){
                    Max+=Max;	
                    buffer=realloc(buffer,Max*sizeof(char));
                    memset((buffer+(Max/2)),0,(Max/2));
                }
                buffer[index++]=text[j];
            }

            else if (hit==item)
                End=1;
					
            j++;
        }

        V_TEXT(s).text[i]=strdup(buffer);
    }
	
    return(s);
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
    } else if (V_TYPE(e) != ID_STRING && V_TYPE(e) != ID_TEXT) {
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

    if (V_TYPE(ob)==ID_TEXT)
        return(ff_delim_textarray(ob,item,delim));

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
        if (hi[i]==0) {
	    if (i==1) {
                hi[i]=V_TEXT(v).Row;
	    } else {
                hi[i]=MAXINT; /*This is to fool it into using full length of string on given row*/
	    }
	}
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

    V_TEXT(o).text=(char **)calloc(V_TEXT(o).Row,sizeof(char *));
    V_TYPE(o)=ID_TEXT;

    for (i=lo[1];i<=hi[1];i+=step[1]){
        if (lo[0] >= strlen(V_TEXT(v).text[i])) {
            V_TEXT(o).text[counter]=(char *)calloc(1,1);
            V_TEXT(o).text[counter][0]='\0';
        }
        else /*if (hi[0]!=0)*/{
            if ((hi[0]-lo[0]+1) > strlen(V_TEXT(v).text[i])){
                V_TEXT(o).text[counter]=strdup((V_TEXT(v).text[i]+lo[0]));
            }
            else {
                V_TEXT(o).text[counter]=(char *)calloc((hi[0]-lo[0]+1)+1,sizeof(char));
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

    if (V_STRING(ob1)==NULL)
        return(NULL);

    i=strlen(V_STRING(ob1));

    while ((i--)>=0) {
        if (V_STRING(ob1)[i]=='/'){
            s=(char *)calloc(i+1,sizeof(char));
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
    V_TEXT(S).text=(char **)calloc(V_TEXT(ob1).Row,sizeof(char *));
    for (i=0;i<V_TEXT(ob1).Row;i++){
        V_STRING(Tmp)=V_TEXT(ob1).text[i];
        V_TEXT(S).text[i]=string_dirname(Tmp);
    }
    V_STRING(Tmp)=NULL;
    return(S);
}



/* This was badly written and wrong */
/*
** basename() can both modify its argument and return static memory.  
** Be careful about return values.
*/

char *
string_basename(Var *ob1, char *ext)
{
    char *s, *q;

    if (V_STRING(ob1)==NULL)
        return(NULL);

	q = strdup(V_STRING(ob1));
	s = basename(q);

	if (ext) {
		if (!strcmp(s+strlen(s)-strlen(ext), ext)) {
			*(s + strlen(s) - strlen(ext)) = '\0';
		}
	}

	s = strdup(s);
	free(q);
    return(s);
}

Var *
text_basename(Var *ob1,char *ext)
{
    int i;
    Var *S=newVar();
    Var *Tmp=newVar();
    V_TYPE(Tmp)=ID_STRING;
    V_TYPE(S)=ID_TEXT;
    V_TEXT(S).Row=V_TEXT(ob1).Row;
    V_TEXT(S).text=(char **)calloc(V_TEXT(ob1).Row,sizeof(char *));
    for (i=0;i<V_TEXT(ob1).Row;i++){
        V_STRING(Tmp)=V_TEXT(ob1).text[i];
        V_TEXT(S).text[i]=string_basename(Tmp,ext);
    }
    V_STRING(Tmp)=NULL;
    return(S);
}


Var *
ff_filename(vfuncptr func, Var * arg) 
{

    Var *ob1;
    Var *S = NULL;
    char *ext=NULL;
    int filefunc;
    Alist alist[3];
    alist[0] = make_alist( "obj", ID_UNK,   NULL,     &ob1);
    alist[1] = make_alist( "ext", ID_STRING,   NULL,     &ext);
    alist[2].name = NULL;

    filefunc=(int)func->fdata;
    if (filefunc==0){
        parse_error("Bad function");
        return(NULL);
    }	

	if (parse_args(func, arg, alist) == 0) return(NULL);

    if (ob1==NULL) {
        return(NULL);
    }


    if (V_TYPE(ob1)==ID_STRING) {
        if (filefunc==1){
			S = newString(string_basename(ob1, ext));
        } else if (filefunc==2){
			S = newString(string_dirname(ob1));
        } else {
            parse_error("Bad Functions");
            return(NULL);
        }
        return(S);
    } else if (V_TYPE(ob1)==ID_TEXT){
        if (filefunc==1){
            S=text_basename(ob1,ext);
        } else if (filefunc==2){
            S=text_dirname(ob1);
        } else {
            parse_error("Bad Functions");
            return(NULL);
        }
        return(S);
    } else {
        parse_error("Only STRING and TEXT types are allowed");
        return(NULL);
    }
}


Var *
ff_grep(vfuncptr func, Var * arg)
{
    Var *ob1;
    Var *S;
    char *s1=NULL;
	int newcursor = 0;
    int count=0;
    int index=0;
    int i;
	regex_t compiled;
    Alist alist[3];
    alist[0] = make_alist( "obj", ID_UNK,   NULL,     &ob1);
    alist[1] = make_alist( "pattern", ID_STRING,   NULL,     &s1);
    alist[2].name = NULL;
    
	if (parse_args(func, arg, alist) == 0) return(NULL);
    
    if (ob1==NULL || s1==NULL){
        return(NULL);
    }

    if (V_TYPE(ob1)!=ID_TEXT){
        parse_error("Can only grep Text Arrays\n");
        return(NULL);	
    }
    /* Compile expression space */
    regcomp(&compiled, s1, REG_EXTENDED);
    for (i=0;i<V_TEXT(ob1).Row;i++){
        newcursor = regexec(&compiled, V_TEXT(ob1).text[i], 0, NULL, 0);
        if (newcursor == 0)
            count++;
    }

    if (count==0){
        parse_error("No Match");
        return(NULL);
    }

    S=newVar();
    V_TYPE(S)=ID_TEXT;
    V_TEXT(S).Row=count;
    V_TEXT(S).text=(char **)calloc(count,sizeof(char *));
    for (i=0;i<V_TEXT(ob1).Row;i++){
        newcursor = regexec(&compiled, V_TEXT(ob1).text[i], 0, NULL, 0);
        if (newcursor == 0){
            V_TEXT(S).text[index++]=strdup(V_TEXT(ob1).text[i]);
        }
    }
	regfree(&compiled);
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

int *
kmp_compile(char *s2)
{
    int l2 = strlen(s2);                        /* length of substring */
    int i,j, *align = (int *)calloc(l2+1, sizeof(int));

    /* compute the align array */

    align[0] = -1;                              /* initial value */
    for (i = 0; s2[i] != '\0'; i++) {
        j = align[i]+1;                         /* add one to prev value */
        while (j > 0 && s2[i] != s2[j-1]) {     /* check if still matches */
            j = align[j-1] + 1;                 /* step backwards if not */
        }
        align[i+1] = j;                         /* save new value */
    }

	return (align);
}

int kmp(char *s1, char *s2,int *align)
{
    int l1 = strlen(s1);                        /* length of text */
    int l2 = strlen(s2);                        /* length of substring */
    int i,j;

    /* search the string, using the align array to detect partial matches */

    j = 0;                                      /* j: index into s2 (sub) */
    for (i = 0 ; i < l1 ; i++) {                /* i: index into s1 (text) */
        for (;;) {
            if (s1[i] == s2[j]) {               /* got a match? */
                j++;                                /* move s2 forward */
                if (j == l2) {                      /* out of characters? */
/*                  free(align);*/
                    return(i-j+1);                   /* return match */
                }
            } else if (j != 0) {                /* no match; 0 is special */
                j = align[j];                   /* step backwards in align */
                continue;                       /* repeat inner loop */
            }
            break;
        }
    }
/*    free(align);*/
    return(-1);                          /* return NULL */
}

Var *
ff_text_strstr(Var *ob1,char *s1)
{
    int i;
    int *data=calloc(V_TEXT(ob1).Row,sizeof(int));
	 int *align=kmp_compile(s1);
	
    for (i=0;i<V_TEXT(ob1).Row;i++){
        data[i]=kmp(V_TEXT(ob1).text[i],s1,align)+1;
    }

	 free(align);

    return(newVal(BSQ,1,V_TEXT(ob1).Row,1,INT,data));
}

Var *
ff_strstr(vfuncptr func, Var * arg)
{

    Var *ob1;
    char *s1=NULL,*newcursor=NULL;
    Alist alist[3];
    alist[0] = make_alist( "obj", ID_UNK,   NULL,     &ob1);
    alist[1] = make_alist( "pattern", ID_STRING,   NULL,     &s1);
    alist[2].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

    if (ob1==NULL || s1==NULL){
        return(NULL);
    }

    if (V_TYPE(ob1)==ID_TEXT){
        return(ff_text_strstr(ob1,s1));
    }

    if (V_TYPE(ob1)==ID_STRING){
        int *v=calloc(1,sizeof(int));
		  int *align=kmp_compile(s1);
        *v=kmp(V_STRING(ob1),s1,align)+1;
		  free(align);
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
    Var *src;
    Var *dest;

    int i;
    int lo[2],hi[2],step[2];
    char *string = NULL;
    int length;
    int height;
    int string_length = 0;
    int cur_line_leng;
    int tmp_hi;
	 int counter=0;


	
    for (i=0;i<2;i++){
        lo[i]=r->lo[i];
        hi[i]=r->hi[i];
        step[i]=r->step[i];
        if (lo[i]==0) lo[i]=1;
        if (hi[i]==0) {
	    if (i==1) {
                hi[i]=V_TEXT(to).Row;
            } else {
                hi[i]=MAXINT; /*This is to fool it into using full length of string on given row*/
	    }
	}
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
	
    height = (hi[1]-lo[1])/step[1]+1;
	
	dest=V_DUP(to);
	src=V_DUP(from);

	if (V_TYPE(from)==ID_STRING){
		string=V_STRING(from);
		string_length=strlen(string);
	}

	else {
		if (((hi[1]-lo[1]/step[1]+1) != V_TEXT(from).Row) && (V_TEXT(from).Row > 1)) {
			parse_error("Can't subset text arrays of different Row sizes");
			return(NULL);
		}
	}


	for (i=lo[1];i<=hi[1];i+=step[1]){
		if (V_TYPE(from)==ID_TEXT){
			string=V_TEXT(from).text[counter];
			string_length=strlen(string);
			counter++;
		}

		cur_line_leng=strlen(V_TEXT(to).text[i]);
		if (lo[0] >= cur_line_leng) continue; /*Skip it*/
		tmp_hi=min(hi[0],(cur_line_leng-1));
		length = (tmp_hi-lo[0]+1);
		free(V_TEXT(to).text[i]);
		V_TEXT(to).text[i]=(char *)calloc(string_length+
				cur_line_leng-length+1,sizeof(char));
		memcpy(V_TEXT(to).text[i],V_TEXT(dest).text[i],lo[0]);
		memcpy((V_TEXT(to).text[i]+lo[0]),string,string_length);
		memcpy((V_TEXT(to).text[i]+lo[0]+string_length),
				 (V_TEXT(dest).text[i]+tmp_hi+1),
					(cur_line_leng-tmp_hi-1));
		V_TEXT(to).text[i][lo[0]+
			string_length+
			(cur_line_leng-tmp_hi-1)]='\0';
	}


	return(src);
}


Var *
where_text(Var *id, Var *where, Var *exp)
{
    int i;
    Var *temp;
    int len = 0;
    char *text;

    if (V_TEXT(id).Row != V_SIZE(where)[1]){
        parse_error("Target and source need to have the same number of rows");
        return(NULL);
    }

    if (V_TYPE(exp)==ID_STRING)
        len=0;

    temp=V_DUP(exp);

    for (i=0;i<V_TEXT(id).Row;i++){
        if(extract_int(where,i)){
            text=(len ? V_TEXT(exp).text[i] : V_STRING(exp));
            free(V_TEXT(id).text[i]);
            V_TEXT(id).text[i]=strdup(text);
        }
    }
		
    return(exp);	
	

}

char *single_replace(char *line, regex_t *preg, char *replace)
{
    regmatch_t pmatch[10];
    int nmatch=10;
    int eflags=0;
    char *newtext;
    char *Marker;

    int result;
	
	
    int  index=0;

    int  Max=100;

    int i;

    int len=strlen(replace);

	int numeral;

	int delta;

	newtext=malloc(Max);


	result=regexec(preg,line,nmatch,pmatch,eflags);
	Marker=line;

	/*search string for every occurence of preg*/
	while (!(result)) {

		/*Check size limitation on newtext, extend if needed*/
		if (index >= (Max-(Max/10))){ /*Keep a 10% margin of safty */
			Max+=Max;
			newtext=realloc(newtext,Max);
		}

		/*Copy everything over, upto the occurence*/
		memcpy((newtext+index),Marker,pmatch[0].rm_so);
		index+=pmatch[0].rm_so;

		/*copy replacement string into newtext*/
		
		i=0;
		while(i < len) {
			/*look for special characters*/
			if (replace[i]=='&') {
				/*insert whole match pattern*/
				delta=pmatch[0].rm_eo-pmatch[0].rm_so;
				memcpy((newtext+index), Marker + pmatch[0].rm_so, delta);
				index+=delta;
			}
			else if (replace[i]=='\\'){
				if (i < (len-1)){
					i++;
					if (replace[i] >= '0' && replace[i] <= '9'){ /*substring*/
						numeral=replace[i]-'0';
						if (pmatch[numeral].rm_so >= 0){ /*requested valid substring*/
							delta=pmatch[numeral].rm_eo-pmatch[numeral].rm_so;
							memcpy((newtext+index),Marker + pmatch[numeral].rm_so, delta);
							index+=delta;
						}
					}
					else { /*Otherwise just copy it over*/
						newtext[index]=replace[i];
						index++;
					}
				}
			}

			else {
				newtext[index]=replace[i];
				index++;
			}

			i++;
		}

		/*Set marker to end of match*/
		Marker+=pmatch[0].rm_eo;
		/* repeat as necessary */
		result=regexec(preg,Marker,nmatch,pmatch,eflags);
	}

	/*copy over any trailing chars after last match*/
	if (strlen(Marker)){
			memcpy((newtext+index),Marker,strlen(Marker));
			index+=strlen(Marker);
	}

    newtext[index]='\0';

    /*Shrink to fit*/
    newtext=realloc(newtext,strlen(newtext)+1);
	
    return(newtext);
}


Var *ff_stringsubst(vfuncptr func, Var *arg)
{
	Var *ob=NULL;
	char *match=NULL;
	char *subst=NULL;

	int i,Row;	
	Var *result;

	regex_t preg;
	int cflags=REG_EXTENDED;

	Alist alist[4];
	alist[0] = make_alist( "obj", ID_UNK,   NULL,     &ob);
	alist[1] = make_alist( "match", ID_STRING,   NULL,     &match);
	alist[2] = make_alist( "substitute", ID_STRING,   NULL,     &subst);
	alist[3].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	/*User error checking...silly user! */

	if (ob==NULL || match==NULL){ /*No target or source */
		return(NULL);
	}

	if (V_TYPE(ob) != ID_STRING && V_TYPE(ob) != ID_TEXT){
		parse_error("This only works with string objects");
		return(NULL);
	}

	if (subst==NULL) {/*Nothing to do */
		return(NULL);
	}


	result=newVar();

	/*Compile expression*/
	if (regcomp(&preg,match,cflags)){	
		parse_error("error compiling regular expression");
		return(NULL);
	}

	/*If our object is just a string, run the single line replace function
		and return it in the result object
	*/

	if (V_TYPE(ob)==ID_STRING){
		V_TYPE(result)=ID_STRING;
		if((V_STRING(result)=single_replace(V_STRING(ob),&preg,subst))==NULL){
			V_STRING(result)=(char *)calloc(1,1);
			V_STRING(result)='\0';
		}
	}

	/*If our object is a text array, run the single line replace function
		for each Row in the array and store each replacement in the result
		object
	*/

	else {
		Row=V_TEXT(ob).Row;
		V_TEXT(result).Row=Row;
		V_TYPE(result)=ID_TEXT;
		V_TEXT(result).text=(char **)calloc(Row,sizeof(char *));
		for (i=0;i<Row;i++){
			if((V_TEXT(result).text[i]=single_replace(V_TEXT(ob).text[i],&preg,subst))==NULL){
				V_TEXT(result).text[i]=(char *)calloc(1,1);
				V_TEXT(result).text[i]='\0';
			}
		}
	}
	 regfree(&preg);
    return(result);
}

char *
rtrim_string(char *string, char *trim)
{
	char *new_string;
	int len;
	int i;

	len=strlen(string);

	i=len-1;
	while (i >= 0) {
		if (string[i]!=*trim)
			break;
		i--;
	}

	if (i==(len-1)) { /*String didn't end with trim character*/
		new_string=strdup(string);
	}
	else if (i < 0 ) { /*String was nothing but trim character*/
		new_string=(char *)calloc(1,sizeof(char));
		new_string[0]='\0';
	}

	else { /*We trimmed some*/
		new_string=(char *)calloc((i+2),sizeof(char));
		strncpy(new_string,string,(i+1));
		new_string[i+1]='\0';
	}

	return(new_string);
}
		


Var *
ff_rtrim(vfuncptr func, Var *arg)
{
	Var *ob=NULL;
	char *trim=NULL;

	int i,Row;	
	Var *s = NULL;

	Alist alist[3];
	alist[0] = make_alist( "obj", ID_UNK,   NULL,     &ob);
	alist[1] = make_alist( "trim", ID_STRING,   NULL,     &trim);
	alist[2].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	/*Error checks*/

	if (ob==NULL || trim==NULL){
		parse_error("Not enough information provided");
		return(NULL);
	}

	
	if (V_TYPE(ob)==ID_STRING){
		s=newVar();
		V_TYPE(s)=ID_STRING;
		V_STRING(s)=rtrim_string(V_STRING(ob),trim);
		return(s);
	}

	if (V_TYPE(ob)!=ID_TEXT){
		parse_error("Rtrim only works on strings or text_arrays");
		return(NULL);
	}
	
	Row=V_TEXT(ob).Row;
	s=newVar();
	V_TYPE(s)=ID_TEXT;
	V_TEXT(s).Row=Row;
	V_TEXT(s).text= (char **)calloc(Row,sizeof(char *));

	for (i=0;i<Row;i++){
		V_TEXT(s).text[i]=rtrim_string(V_TEXT(ob).text[i],trim);
	}

	return(s);
}

#include "parser.h"

/**
 ** Convert a string to int representation
 **/

Var *
ff_atoi(vfuncptr func, Var *arg)
{
	int i;
	Var *v,*s;

	if (arg->next != NULL) {
        parse_error("Too many arguments to function: %s()", func->name);
        return (NULL);
   }
   if ((v = eval(arg)) == NULL) {
        parse_error("Variable not found: %s", V_NAME(arg));
        return (NULL);
   }
   if (V_TYPE(v) != ID_STRING || V_TYPE(v)!=ID_TEXT ){
		parse_error("Invalid object type");
		return(NULL);
	}

	s = newVar();

	if (V_TYPE(v) == ID_STRING){
		i = (int)strtod(V_STRING(v), NULL);
		V_SIZE(s)[0] = V_SIZE(s)[1] = V_SIZE(s)[2] = V_DSIZE(s) = 1;
		V_INT(s) = i;
	}

	else {
		int l;
		int *data = (int *)calloc(V_TEXT(v).Row, sizeof(int));
		for (l=0;l<V_TEXT(v).Row;l++){
			data[l]=strtod(V_TEXT(v).text[l],NULL);
		}
		V_SIZE(s)[0] = V_SIZE(s)[2] = 1;
		V_SIZE(s)[1] = l;
		V_DSIZE(s) = l;
		V_DATA(v)=(void *)data;
	}
			

	V_TYPE(s) = ID_VAL;
	V_ORG(s) = BSQ;
	V_FORMAT(s) = INT;

	return(s);
}

Var *
ff_atof(vfuncptr func, Var *arg)
{
	Var *v, *s;
	float f;

	if (arg->next != NULL) {
        parse_error("Too many arguments to function: %s()", func->name);
        return (NULL);
   }
   if ((v = eval(arg)) == NULL) {
        parse_error("Variable not found: %s", V_NAME(arg));
        return (NULL);
   }
   if (V_TYPE(v) != ID_STRING || V_TYPE(v)!=ID_TEXT ){
		parse_error("Invalid object type");
		return(NULL);
	}

	s = newVar();

	if (V_TYPE(v) == ID_STRING){
		f = (float)strtod(V_STRING(v), NULL);
		V_SIZE(s)[0] = V_SIZE(s)[1] = V_SIZE(s)[2] = V_DSIZE(s) = 1;
		V_FLOAT(s) = f;
	}

	else {
		int l;
		float *data = (float *)calloc(V_TEXT(v).Row, sizeof(float));
		for (l=0;l<V_TEXT(v).Row;l++){
			data[l]=strtod(V_TEXT(v).text[l],NULL);
		}
		V_SIZE(s)[0] = V_SIZE(s)[2] = 1;
		V_SIZE(s)[1] = l;
		V_DSIZE(s) = l;
		V_DATA(v)=(void *)data;
	}
			

	V_TYPE(s) = ID_VAL;
	V_ORG(s) = BSQ;
	V_FORMAT(s) = FLOAT;

	return(s);
}


/**
 ** pp_add_strings()
 **
 ** This is called from pp_math.c
 **/

Var *
pp_add_strings(Var *a, Var *b)
{
	int len;
	Var *v,*s = NULL;
	char buf[256];
	char *ptr,*p1;

	if (V_TYPE(a) == ID_STRING && V_TYPE(b) == ID_STRING) {
		len = strlen(V_STRING(a)) + strlen(V_STRING(b)) +1;
		ptr = (char *)calloc(1, len);
		strcpy(ptr, V_STRING(a));
		strcat(ptr, V_STRING(b));
	} else {
		if (V_TYPE(b) == ID_VAL) {
			v = b;
			p1 = V_STRING(a);
		} else {
			v = a;
			p1 = V_STRING(b);
		}

		switch(V_FORMAT(v)) {
			case BYTE:
			case SHORT:
			case INT:
				sprintf(buf, "%d", extract_int(v,0)); break;
			case FLOAT:
			case DOUBLE:
				sprintf(buf, "%.*g", SCALE, extract_double(v,0)); break;
		}

		len = strlen(buf) + strlen(p1) + 1;
		ptr = (char *)calloc(1, len);

		if (V_TYPE(a) == ID_VAL) {
			strcpy(ptr, buf);
			strcat(ptr, p1);
		} else {
			strcpy(ptr, p1);
			strcat(ptr, buf);
		}
	}
	s = newVar();
	V_TYPE(s) = ID_STRING;
	V_STRING(s) = ptr;

	return(s);
}


/*
Var *
ff_basename(vfuncptr func, Var *arg)
{
	Var *v;
	char *ptr, *p;
	Var *path, *suffix;

    struct keywords kw[] = {
        { "path", NULL },
        { "suffix", NULL },
        { NULL, NULL }
    };
    
    if (evaluate_keywords(func, arg, kw)) {
        return(NULL);
    }

	if (arg == NULL) return(NULL);
	
    if ((v = get_kw("path",kw)) == NULL) {
        parse_error("No path specified to basename()");
        return(NULL);
    }
    path = eval(v);
    if (V_TYPE(path) != ID_STRING) {
    	parse_error("basename: expected string for path");
    	return(NULL);
    }

    if ((v = get_kw("suffix",kw)) == NULL) {
        parse_error("No path specified to basename()");
        return(NULL);
    }

    suffix = eval(v);
    if (V_TYPE(suffix) != ID_STRING) {
    	parse_error("basename: expected string for path");
    	return(NULL);
    }

	ptr = V_STRING(path);
	if ((p = strrchr(ptr, '/')) != NULL) {
		p++;
	} else {
		p = ptr;
	}


	v = newVar();
	V_TYPE(v) = ID_STRING;
	V_STRING(v) = strdup(p);

	if (suffix) {
		ptr = V_STRING(v);
		p = ptr + strlen(ptr) - strlen(V_STRING(suffix));
		if (!strcmp(V_STRING(suffix), p)) {
			*p = '\0';
		}
	}
	return(v);
}

*/
char *
str3dup(char *s1, char *s2, char *s3)
{
	char *s;
	s = (char *)malloc(strlen(s1)+strlen(s2)+strlen(s3)+1);
	strcpy(s,s1);
	strcat(s,s2);
	strcat(s,s3);
	return(s);
}

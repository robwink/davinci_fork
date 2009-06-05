#include "header.h"
#include "parser.h"
#include "io_lablib3.h"
#include "endian_norm.h"
#include <ctype.h>
#include "dvio.h"
#include <sys/types.h>
#include <regex.h>

#define MAXOBJ 10

#ifdef LITTLE_ENDIAN
extern char *var_endian(Var * v);
#endif


Var *write_PDS_Qube(Var * core, Var * side, Var * bottom, Var * back,
                    FILE * fp);

/*
** If the object is a QUBE AND has suffix planes, we need to run special software to process,
** therefore we need to save the suffix structures without interfering with the object
** information
*/

enum _PDS_Object { PDS_QUBE, SUFFIX, PDS_IMAGE, PDS_HISTORY, PDS_TABLE,
        PDS_HISTOGRAM_IMAGE };
typedef enum _PDS_Object PDS_Object;

typedef struct _objectInfo {
    int count;
    int ptr_count;

    PDS_Object *obj_type;       /*What kind of object is it */
    int *obj_ptr;               /*File Byte offset of ^OBJECT = */
    int *obj_size;              /*In RECORD_BYTES */
    void **obj_data;            /*Duh */
    int *obj_dirty;             /*Check this if the object needs to be freed when we're finished */

    Var *sample_suffix;
    int total_sample_count;     /*This is needed for record alignment */
    Var *line_suffix;
    int total_line_count;       /*This is needed for record alignment */
    Var *band_suffix;
    int total_band_count;       /*This is needed for record alignment */
    int Qub_Pad;                /*We can't pad the qub data block, because it doesn't get merged with the suffix
                                   planes until later, so we need to save the pad amount, and do it AFTER the qube is
                                   written to disk */

} objectInfo;


void Set_Col_Var(Var **, FIELD **, LABEL *, int *size, char **);
void ProcessGroupIntoLabel(FILE * fp, int record_bytes, Var * v,
                           char *name);
void ProcessObjectIntoLabel(FILE * fp, int record_bytes, Var * v,
                            char *name, objectInfo * oi);
Var *ProcessIntoLabel(FILE * fp, int record_bytes, Var * v, int depth,
                      int *label_ptr, objectInfo * oi);


static char keyword_prefix[] = "PTR_TO_";
static int keyword_prefix_length = 8;

typedef struct _dataKeys {
    char *Name;                 /*Name Entry in Table; Used for searching */
    Var *Obj;                   /*Pointer to Parent Var obj to which the data will be assigned */
    char *KeyValue;             /*PTR_TO_<OBJECT> keyword entry's value */
    char *FileName;             /*Name of the file the PDS object is from, 
                                   could be different than the one originally given */
    int dptr;                   /*offset into file where data begins */

} dataKey;

/*To add dataKey words for more readable types do the following 3 steps:*/

/*Step 1: Add READ function/wrapper declaration here*/
int rf_QUBE(char *, Var *, char *, int);
int rf_TABLE(char *, Var *, char *, int);
int rf_IMAGE(char *, Var *, char *, int);
int rf_HISTOGRAM_IMAGE(char *, Var *, char *, int);
int rf_HISTORY(char *, Var *, char *, int);

static dataKey *dK = NULL;
typedef int (*PFI) (char *, Var *, char *, int);

/*Step2: Add the name of the READ function/wrapper here*/
PFI VrF[] =
    { rf_QUBE, rf_QUBE, rf_TABLE, rf_IMAGE, rf_HISTOGRAM_IMAGE,
rf_HISTORY };


/*Step 3: Add the dataKey word to this here (before the NULL!)*/
static char *keyName[] =
    { "SPECTRAL_QUBE", "QUBE", "TABLE", "IMAGE", "HISTOGRAM_IMAGE",
"HISTORY", NULL };
static int num_data_Keys;


/*
** We want to note the Record_Bytes.  
** If we have multiple objects in this file
** this is value which everything is aligned to.
** Therefore, if we have a qube and a table, while the table
** maybe have rows of size N, the record bytes might be of
** size M.  Therefore, any pointers in the file will
** be in units of M (ie ^TABLE = Q ==> the table is at
** address Q*M.  However, after reaching the table, we'll
** be reading it in chunks of N.  So....
*/
void Add_Key_Offset(char *Val, int Index, int record_bytes)
{
    if (Index < 1) {
        printf("Error: Table Underflow\n");
        exit(0);
    }

    if (Index > num_data_Keys) {
        printf("Error: Table Overflow\n");
        exit(0);
    }

    /*Need to subtract 1 (we added one when we sent it the first time) */

    Index--;

//I don't understand, but when record_bytes was N and the object Point M dptr was always N*(M-1)...oh well
    dK[Index].dptr = record_bytes * (atoi(Val) - 1);
}

char *fix_name(char *input_name)
{
    char *name = strdup(input_name);
    int len = strlen(name);
    int i;
    int val;

    if (len < 1)
        return ("Foo!");

    i = 0;
    while (i < len) {
        val = (int) (name[i]) & 0xff;
        if (isupper(val)) {
            val = tolower(val) & 0xff;
            name[i] = (char) val;
        }
        i++;
    }

    return (name);
}


void Init_Obj_Table(void)
{
    int i = 0;
    while (keyName[i] != NULL)
        i++;

    num_data_Keys = i;

    if (dK != NULL) {
        free(dK);
    }

    dK = (dataKey *) calloc(num_data_Keys, sizeof(dataKey));

    for (i = 0; i < num_data_Keys; i++) {
        dK[i].Name = keyName[i];
        dK[i].Obj = NULL;
        dK[i].KeyValue = NULL;
        dK[i].dptr = 0;
    }
}

int Match_Key_Obj(char *obj)
{
    /*Want to know if this Object is in our table */

    int i;

    for (i = 0; i < num_data_Keys; i++) {
        if (!(strcmp(dK[i].Name, obj)))

            /*If it is, then return it's index(+1)
               By adding one, we can use 0 to indicate no match.
               Just need to remember and subtract 1 on it's return  
               below...this should be transperent to the caller. */

            return (i + 1);
    }

    return (0);
}

void Add_Key_Word_Value(char *Val, int Index)
{
    if (Index < 1) {
        printf("Error: Table Underflow\n");
        exit(0);
    }

    if (Index > num_data_Keys) {
        printf("Error: Table Overflow\n");
        exit(0);
    }

    /*Need to subtract 1 (we added one when we sent it the first time) */

    Index--;

    if (dK[Index].KeyValue != NULL) {
        printf("Error: Redudant Value Entry at Index:%d\n", Index);
        exit(0);
    }

    dK[Index].KeyValue = strdup(Val);
}

void Add_Key_Obj(Var * Ob, char *fn, int Index)
{
    if (Index < 1) {
        printf("Error: Table Underflow\n");
        exit(0);
    }

    if (Index > num_data_Keys) {
        printf("Error: Table Overflow\n");
        exit(0);
    }

    /*Need to subtract 1 (we added one when we sent it the first time) */
    Index--;

    if (dK[Index].Obj != NULL) {
        printf("Error: Redudant Obj Entry at Index:%d\n", Index);
        exit(0);
    }

    dK[Index].Obj = Ob;
    dK[Index].FileName = strdup(fn);
}


void Read_Object(Var * v)
{
    int i;
    for (i = 0; i < num_data_Keys; i++) {
        if (dK[i].Obj != NULL && dK[i].Name != NULL
            && dK[i].KeyValue != NULL)
/*                      printf("%p %s %s %s\n",dK[i].Obj,dK[i].FileName,dK[i].Name,dK[i].KeyValue);*/
            (VrF[i]) (dK[i].FileName, dK[i].Obj, dK[i].KeyValue,
                      dK[i].dptr);
    }
}

int make_int(char *number)
{
    char *base;
    char *radix;
    int r_flag = 0;
    int len;
    int i = 0;
    int count = 0;
    int offset;
    len = strlen(number);

    /*Looking for # which signifies a Base notation integer */

    while (i < len) {
        if (number[i] == '#') {
            r_flag = 1;
            break;
        }
        i++;
    }

    if (!(r_flag))              /*Didn't find it, regular int */
        return (atoi(number));

    /*Gotta convert it! */

    number[i] = '\0';           /*Null it at first # */
    radix = strdup(number);
    i++;
    offset = i;                 /*Start string here now */

    while (i < len) {
        if (number[i] == '#') { /*Other one */
            number[i] = '\0';
            base = strdup((number + offset));
            return ((int) strtol(base, NULL, atoi(radix)));
        }
        i++;
    }

    return (0);                 /*No 2nd #? Then it's junk */
}

/*
void  
do_obj(OBJDESC *ob,Var *v)
{
        char *name;
        KEYWORD *kwd;
        Var *o=new_struct(0);

        if (ob->obj_class != NULL)
                name=strdup(ob->obj_class);

        else if((kwd = OdlFindKwd(ob, "NAME", NULL,1, ODL_THIS_OBJECT))!=NULL)
                name=strdup(kwd->name);

        else{
                parse_error("Object with no name is unusable");
                return(NULL);
        }
        add_struct(v,name,o);
}
*/


Var *do_key(OBJDESC * ob, KEYWORD * key)
{

    unsigned short kwv;
    Var *o = NULL;
    int *i;
    double *f;

    kwv = OdlGetKwdValueType(key);

    switch (kwv) {

    case ODL_INTEGER:
        i = (int *) calloc(1, sizeof(int));
        *i = make_int(key->value);
        o = newVal(BSQ, 1, 1, 1, INT, i);
        break;
    case ODL_REAL:
        f = (double *) calloc(1, sizeof(double));
        *f = strtod(key->value, NULL);
        o = newVal(BSQ, 1, 1, 1, DOUBLE, f);
        break;
    case ODL_SYMBOL:
    case ODL_DATE:
    case ODL_DATE_TIME:
        o = newVar();
        V_TYPE(o) = ID_STRING;
        V_STRING(o) = strdup(key->value);
        break;
    case ODL_TEXT:

        o = newVar();
        V_TYPE(o) = ID_STRING;
        V_STRING(o) = strdup(key->value);

        break;

    case ODL_SEQUENCE:
    case ODL_SET:
        {
            char **stuff;
            int ii;
			int ptype;
            int num = OdlGetAllKwdValuesArray(key, &stuff);

            if (num) {
				/*
				** Previous assumption was that all types are the same.
				**
				** This turns out to be bad if the first value is a real
				** that just happens to not have a decimal part, and the
				** rest do.  So, check that either everything is INT or
				** promote to REAL.  If we get a value that's neither INT
				** or REAL, abort and use that type (STRING is all that's left)
				*/
				ptype = OdlDataType(stuff[0]);
				if (ptype == ODL_INTEGER) {
					int i;
					int ltype;
					for (i = 0 ; i < num ; i++) {
						ltype = OdlDataType(stuff[i]);
						if (ltype == ODL_REAL) ptype = ODL_REAL;
						if (ltype != ODL_INTEGER && ltype != ODL_REAL) {
							ptype = ltype;
							break;
						}
					}
				}

                switch (ptype) {
                case ODL_INTEGER:
                    i = (int *) malloc(num * sizeof(int));
                    for (ii = 0; ii < num; ii++)
                        i[ii] = atoi(stuff[ii]);
                    o = newVal(BSQ, num, 1, 1, INT, i);
                    break;

                case ODL_REAL:
                    f = (double *) malloc(num * sizeof(double));
                    for (ii = 0; ii < num; ii++)
                        f[ii] = strtod(stuff[ii], NULL);
                    o = newVal(BSQ, num, 1, 1, DOUBLE, f);
                    break;

                default:
                    o = newVar();
                    V_TYPE(o) = ID_TEXT;
                    V_TEXT(o).Row = num;
                    V_TEXT(o).text = (char **) calloc(num, sizeof(char *));
                    for (ii = 0; ii < num; ii++) {
                        V_TEXT(o).text[ii] = strdup(stuff[ii]);
                    }
                }
            } else {
                o = newVar();
                V_TYPE(o) = ID_STRING;
                V_STRING(o) = (char *) calloc(1, sizeof(char));
                V_STRING(o)[0] = '\0';
            }
        }
        break;

    default:
        parse_error("Unknown PDS value type...Setting as string");
        o = newVar();
        V_TYPE(o) = ID_STRING;
        if (key->value != NULL)
            V_STRING(o) = strdup(key->value);
        else {
            V_STRING(o) = (char *) calloc(1, sizeof(char));
            V_STRING(o)[0] = '\0';
        }

    }
    return (o);
}

char *mod_name_if_necessary(char *name)
{
    char *new_name;
    if (name[0] != '^')
        return (name);

    new_name = (char *) calloc(strlen(&name[1]) +
                               keyword_prefix_length, sizeof(char));

    strcpy(new_name, keyword_prefix);
    strcat(new_name, &name[1]);
    return (new_name);
}

void ProcessGroup(Var * v, KEYWORD ** key, OBJDESC * ob)
{
    Var *next_var;
    Var *tmp_var;
    char *keyname;

    *key = OdlGetNextKwd(*key);

    while (*key != NULL) {

        if (!(strcmp("GROUP", (*key)->name))) {
            next_var = new_struct(0);
            add_struct(v, fix_name((*key)->value), next_var);
            tmp_var = newVar();
            V_TYPE(tmp_var) = ID_STRING;
            V_STRING(tmp_var) = strdup("GROUP");
            add_struct(next_var, "Object", tmp_var);
            ProcessGroup(next_var, key, ob);
            (*key) = OdlGetNextKwd(*key);
            continue;
        }

        else if (!(strcmp("END_GROUP", (*key)->name)))
            return;

        else {
            keyname = mod_name_if_necessary((*key)->name);
            add_struct(v, fix_name(keyname), do_key(ob, *key));
            *key = OdlGetNextKwd(*key);
        }
    }
}

static char *
gen_next_unused_name_instance(
    char *keyname,
    Var  *s
    )
{
    char *ser_key_name;
    int   i;
    int   max_ser_no = 1000;
    Var  *v;

    /* alloc a ridiculously large key name buffer */
    ser_key_name = (char *)calloc(strlen(keyname)+64, sizeof(char));

    for(i = 1; i < max_ser_no; i++){
        /* generate a key with the next free serial number */
        sprintf(ser_key_name, "%s_%d", keyname, i);
        
        if (find_struct(s, ser_key_name, &v) < 0){
            /* if this serial number is unused, return this key */
            return ser_key_name;
        }
    }

    free(ser_key_name);
    
    return NULL; /* no such instance found */
}

void Traverse_Tree(OBJDESC * ob, Var * v, int record_bytes)
{
    KEYWORD *key;
    int offset = strlen(keyword_prefix);
    OBJDESC *next_ob = NULL;
    unsigned short scope = ODL_CHILDREN_ONLY;
    int count = 0;
    Var *next_var;
    Var *tmp_var;
    char *name;
    char *keyname;
    KEYWORD *tmp_key;
    int idx;


    /*count the number of child OBJECTS of ob */
    next_ob = OdlNextObjDesc(ob, 0, &scope);
    if (next_ob != NULL)
        count = 1;
    while (next_ob != NULL) {
        next_ob = OdlNextObjDesc(next_ob, 0, &scope);
        if (next_ob != NULL)
            count++;
    }

    /*Reset next_ob back to the first child (if there is one) */
    scope = ODL_CHILDREN_ONLY;
    next_ob = OdlNextObjDesc(ob, 0, &scope);


    /*Iterate through keywords and objects until we run out of one or the other */

    key = OdlGetFirstKwd(ob);

    while (key != NULL && count) {

        /*If Key and Next_ob are from different files, keyword takes precedent */
        if ((strcmp(key->file_name, next_ob->file_name))) {
            keyname = mod_name_if_necessary(key->name);

            /*Check to see if this is a pointer */
            if (key->is_a_pointer) {    /* then check to see if it's an object we want */
                if ((idx = Match_Key_Obj(&key->name[1]))) {     /* It is */
                    Add_Key_Word_Value(key->value, idx);
                    Add_Key_Offset(key->value, idx, record_bytes);
                }
            }

            /*Now add the name to the structure */
            add_struct(v, fix_name(keyname), do_key(ob, key));
            key = OdlGetNextKwd(key);
        }

        /*Same filename, so check line numbers */
        /*Keyword before object */
        else if (key->line_number < next_ob->line_number) {
            keyname = mod_name_if_necessary(key->name);

/*
** Okay, here we break from the previous smooth transitioning
*/
            if (!(strcmp("GROUP", key->name))) {        /* we have the begining of a group! */
                next_var = new_struct(0);
                keyname = fix_name(key->value);
                
                if (find_struct(v, keyname, &tmp_var) >= 0){
                    /* the group already exists within history, generate a
                       serializaed group name from the given name */
                    keyname = gen_next_unused_name_instance(keyname, v);
                }
                
                add_struct(v, keyname, next_var);
                tmp_var = newVar();
                V_TYPE(tmp_var) = ID_STRING;
                V_STRING(tmp_var) = strdup("GROUP");
                add_struct(next_var, "Object", tmp_var);
                ProcessGroup(next_var, &key, ob);
                key = OdlGetNextKwd(key);
                continue;
            }


            /*Check to see if this is a pointer */
            if (key->is_a_pointer) {    /* then check to see if it's an object we want */
                if ((idx = Match_Key_Obj(&key->name[1]))) {     /* It is */
                    Add_Key_Word_Value(key->value, idx);
                    Add_Key_Offset(key->value, idx, record_bytes);
                }
            }

            add_struct(v, fix_name(keyname), do_key(ob, key));
            key = OdlGetNextKwd(key);
        }


        /*object before keyword */
        else {
            name = NULL;
            tmp_key =
                (OdlFindKwd(next_ob, "NAME", NULL, 1, ODL_THIS_OBJECT));
            if (tmp_key != NULL)
                if (tmp_key->value != NULL)
                    name = tmp_key->value;

            if (name == NULL)   /*Still no name */
                name = next_ob->obj_class;

            next_var = new_struct(0);
            add_struct(v, fix_name(name), next_var);
            /*NEW: Add variable Object with value "class" right under our structure */
            tmp_var = newVar();
            V_TYPE(tmp_var) = ID_STRING;
            V_STRING(tmp_var) = strdup(next_ob->obj_class);
            add_struct(next_var, "Object", tmp_var);

            /*Check to see if this is an object we want */
            if ((idx = Match_Key_Obj(next_ob->obj_class))) {        /* It is */
                Add_Key_Obj(next_var, next_ob->file_name, idx);

            }

            Traverse_Tree(next_ob, next_var, record_bytes);

            next_ob = OdlNextObjDesc(next_ob, 0, &scope);

            count--;
        }
    }

    /*Okay, somebody is depleted; keyword or children of ob */

    if (key == NULL) {          /*Only children of ob left on this level */
        while (count) {
            name = NULL;
            tmp_key =
                (OdlFindKwd(next_ob, "NAME", NULL, 1, ODL_THIS_OBJECT));
            if (tmp_key != NULL)
                if (tmp_key->value != NULL)
                    name = tmp_key->value;

            if (name == NULL)
                name = next_ob->obj_class;

            next_var = new_struct(0);
            add_struct(v, fix_name(name), next_var);
            /*NEW: Add variable Object with value "class" right under our structure */
            tmp_var = newVar();
            V_TYPE(tmp_var) = ID_STRING;
            V_STRING(tmp_var) = strdup(next_ob->obj_class);
            add_struct(next_var, "Object", tmp_var);

            /*Check to see if this is an object we want */
            if ((idx = Match_Key_Obj(next_ob->obj_class))) {        /* It is */
                Add_Key_Obj(next_var, next_ob->file_name, idx);

            }

            Traverse_Tree(next_ob, next_var, record_bytes);
            next_ob = OdlNextObjDesc(next_ob, 0, &scope);
            count--;
        }
    }

    else {                      /*Only keywords left */
        while (key != NULL) {
            keyname = mod_name_if_necessary(key->name);
/*
** Okay, here we break from the previous smooth transitioning
*/
            if (!(strcmp("GROUP", key->name))) {        /* we have the begining of a group! */
                next_var = new_struct(0);
                keyname = fix_name(key->value);
                
                if (find_struct(v, keyname, &tmp_var) >= 0){
                    /* the group already exists within history, generate a
                       serializaed group name from the given name */
                    keyname = gen_next_unused_name_instance(keyname, v);
                }
                
                add_struct(v, keyname, next_var);
                tmp_var = newVar();
                V_TYPE(tmp_var) = ID_STRING;
                V_STRING(tmp_var) = strdup("GROUP");
                add_struct(next_var, "Object", tmp_var);
                ProcessGroup(next_var, &key, ob);
                key = OdlGetNextKwd(key);
                continue;
            }


            /*Check to see if this is a pointer */
            if (key->is_a_pointer) {    /* then check to see if it's an object we want */
                if ((idx = Match_Key_Obj(&key->name[1]))) {     /* It is */
                    Add_Key_Word_Value(key->value, idx);
                    Add_Key_Offset(key->value, idx, record_bytes);
                }
            }

            add_struct(v, fix_name(keyname), do_key(ob, key));
            key = OdlGetNextKwd(key);
        }
    }
/*everything finished for this ob so return*/
}

char *remangle_name(char *filename)
{
    return filename;
}

char *correct_name(char *name)
{
    char *newname = strdup(name);
    int i = strlen(newname) - 1;
    while (i >= 0) {
        newname[i] = (char) toupper((int) (0xff & newname[i]));
        i--;
    }

    return (newname);
}

void
ProcessGroupIntoLabel(FILE * fp, int record_bytes, Var * v, char *name)
{
    Var *tmpvar;
    Var *v2;
    int i;
    int count;
    char *struct_name;
    char *tmpname;
    char *tmpname2;

    tmpname2 = correct_name(name);
    fprintf(fp, "GROUP = %s\r\n", tmpname2);

    get_struct_element(v, 0, &struct_name, &tmpvar);    /*Get the OBJECT Tag and temporarily NULLify it */
/*      tmpname=strdup(struct_name); */
/*      *struct_name='\0'; */

    struct_name[0] = '\0';

    ProcessIntoLabel(fp, record_bytes, v, 0, NULL, NULL);

/*      memcpy(struct_name,tmpname,strlen(tmpname)); */
/*      free(tmpname); */

    struct_name[0] = 'O';       /* Replace the O in Object */

    fprintf(fp, "END_GROUP = %s\r\n", tmpname2);
    free(tmpname2);

}

char *ProcessVarIntoString(Var * element, char *name)
{
    int flag = 0;
    int idx;
    int count;
    char tmp_string[1024];
    char *string = (char *) malloc(1024 * 32 * sizeof(char));
    char *tmpname;

    if (GetSamples(V_SIZE(element), V_ORG(element)) > 1)
        flag += 1;
    if (GetLines(V_SIZE(element), V_ORG(element)) > 1)
        flag += 2;
    if (GetBands(V_SIZE(element), V_ORG(element)) > 1)
        flag += 4;

    if (flag) {
        if (((flag & 1) && (flag & 2)) ||
            ((flag & 1) && (flag & 4)) || ((flag & 2) && (flag & 4))) {

            parse_error
                ("Can't write out a plane or a cube object into the history!");
            sprintf(string, "%s = \"Error\"\r\n", correct_name(name));
            return (string);
        }
    }

    else {

        if (V_FORMAT(element) == INT) {
            sprintf(tmp_string, "%d", V_INT(element));
        }

        else if (V_FORMAT(element) == FLOAT) {
            sprintf(tmp_string, "%f", V_FLOAT(element));
        }

        else if (V_FORMAT(element) == DOUBLE) {
            sprintf(tmp_string, "%.10g", V_DOUBLE(element));
        }

        else {
            parse_error("Found a scalar which isn't covered");
            strcpy(tmp_string, "\"Error\"\r\n");
        }

        sprintf(string, "%s = ", correct_name(name));
        strcat(string, tmp_string);
        strcat(string, "\r\n");
        return (string);
    }



/*Okay, we're dealing with a vector, NOT a scale so we have to build a list */
    sprintf(string, "%s = (", correct_name(name));
    for (idx = 0; idx < 3; idx++)
        if (flag & (1 << idx))
            break;

    switch (idx) {

    case 0:
        count = GetSamples(V_SIZE(element), V_ORG(element));
        break;

    case 1:
        count = GetLines(V_SIZE(element), V_ORG(element));
        break;

    case 2:
        count = GetBands(V_SIZE(element), V_ORG(element));
        break;
    }


    for (idx = 0; idx < count; idx++) {
        switch (V_FORMAT(element)) {

        case BYTE:

            break;

        case SHORT:

            break;

        case INT:
            sprintf(tmp_string, "%d, ", extract_int(element, idx));
            break;

        case FLOAT:
            sprintf(tmp_string, "%f, ", extract_float(element, idx));
            break;

        case DOUBLE:
            sprintf(tmp_string, "%.10g, ", extract_double(element, idx));
            break;

        }
        strcat(string, tmp_string);

    }

    /*Need to back up two chars and replace them with a ')' */
    strcpy((string + strlen(string) - 2), ")");
    strcat(string, "\r\n");
    return (string);

}


void ProcessHistoryIntoString(Var * v, char **theString, int *ptr)
{

    int i;
    int count;
    Var *element;
    char *name;
    char *tmpname;
    char string[1024 * 32];
    char num[1024];
    static int current_size;

    if (!(*ptr)) {              /*first time through, malloc up some memory */
        current_size = (1024 * 64);
        (*theString) = (char *) malloc(current_size);
    }


/*Grow theString if needed*/
    if (*ptr >= (current_size / 2)) {
        current_size *= 2;
        (*theString) = realloc(*theString, current_size);
    }

    count = get_struct_count(v);
    for (i = 0; i < count; i++) {
        get_struct_element(v, i, &name, &element);
        if (V_TYPE(element) == ID_STRUCT) {
            tmpname = correct_name(name);
            sprintf(string, "GROUP = %s\r\n", tmpname);
            memcpy((*theString + (*ptr)), string, strlen(string));
            *ptr += strlen(string);

            ProcessHistoryIntoString(element, theString, ptr);

            sprintf(string, "END_GROUP = %s\r\n", tmpname);
            memcpy((*theString + (*ptr)), string, strlen(string));
            *ptr += strlen(string);

            free(tmpname);

        }

        else if (!(strcasecmp(name, "Object")))
            continue;

        /*Check for non-string values and convert */
        else if (V_TYPE(element) == ID_VAL) {
            tmpname = ProcessVarIntoString(element, name);
            memcpy((*theString + (*ptr)), tmpname, strlen(tmpname));
            *ptr += strlen(tmpname);
            free(tmpname);
        }

        else if (V_TYPE(element) == ID_STRING) {

            tmpname = correct_name(name);
            sprintf(string, "%s = %s\r\n", tmpname, V_STRING(element));
            memcpy((*theString + (*ptr)), string, strlen(string));
            *ptr += strlen(string);
            free(tmpname);
        }

        else {
            parse_error
                ("History Object contains an unknown object...skipping it!");
        }

    }
}

int set_suffix(Var * suffix, Var ** final)
{
    int v_count = get_struct_count(suffix);
    Var *final_block;

#ifdef LITTLE_ENDIAN

/*
** If we're little endian, we need to step through the suffix structure and var_endian
** EACH structure element, relpacing the copy's V_DATA item with the return object
** and don't forget to free up the old V_DATA object!
*/
    Var *tmp = V_DUP(suffix);   /*copy the item (it's a structure!) */
    int i = 0;
    Var *element;
    char *innername;
    void *v_data;
    for (i = 0; i < v_count; i++) {     /*Need to skip first element, it's the Object=Name element */
        get_struct_element(tmp, i, &innername, &element);
        v_data = V_DATA(element);
        V_DATA(element) = (void *) var_endian(element);
        free(v_data);
    }
    final_block = tmp;
#else
    final_block = suffix;
#endif

    *final = final_block;

    return (v_count);
}


void
ProcessObjectIntoLabel(FILE * fp, int record_bytes, Var * v, char *name,
                       objectInfo * oi)
{

    Var *tmpvar;
    int i;
    int count;
    char *struct_name;
    int rem;
    int samples;
    int lines;
    int bands;

    char **name_list;
    int sample_idx = (-1);
    int line_idx = (-1);
    int band_idx = (-1);

    char *tmpname;
    char *tmpname2;
    fprintf(fp, "OBJECT = %s\r\n", name);

    if (!(strcasecmp("table", name))) { /*We're processing a table! */
        get_struct_element(v, 0, &struct_name, &tmpvar);        /*Get the OBJECT Tag and temporarily NULLify it */

/*              tmpname=strdup(struct_name); */
/*              *struct_name='\0'; */

        struct_name[0] = '\0';

        ProcessIntoLabel(fp, record_bytes, v, 0, NULL, oi);

/*              memcpy(struct_name,tmpname,strlen(tmpname)); */
/*              free(tmpname); */

        struct_name[0] = 'O';

        /*Need to make use of the data object here */
        oi->obj_size[oi->count] = 0;
        oi->obj_data[oi->count] = (void *) NULL;
        oi->obj_dirty[oi->count] = 0;
        oi->obj_type[oi->count] = PDS_TABLE;
        oi->count++;
    }

    else if ((!(strcasecmp("history", name)))) {
        char end[] = "END\n";
        int ptr = 0;
        int size;

        count = get_struct_count(v);
        for (i = 0; i < count; i++) {   /*We could assume the DATA object is last, but that could lead to trouble */
            get_struct_element(v, i, &struct_name, &tmpvar);
            if (!(strcasecmp(struct_name, "data"))) {   /*Found it! */
                ProcessHistoryIntoString(tmpvar,
                                         (char **) &oi->obj_data[oi->
                                                                 count],
                                         &ptr);
                memcpy(((char *) oi->obj_data[oi->count] + ptr), end,
                       strlen(end));
                ptr += strlen(end);

                size = (ptr / record_bytes) + 1;
                oi->obj_data[oi->count] =
                    (void *) realloc(oi->obj_data[oi->count],
                                     size * record_bytes);
                if (ptr % record_bytes) {
                    int rem = size * record_bytes - ptr;
                    memset((char *) oi->obj_data[oi->count] + ptr, 0x20,
                           rem);
                    ptr += rem;
                }
                oi->obj_size[oi->count] = (ptr / record_bytes);
                oi->obj_dirty[oi->count] = 1;
                oi->obj_type[oi->count] = PDS_HISTORY;
                oi->count++;
                break;
            }


        }

        get_struct_element(v, 0, &struct_name, &tmpvar);        /*Get the OBJECT Tag and temporarily NULLify it */

/*              tmpname=strdup(struct_name); */
/*              *struct_name='\0'; */

        struct_name[0] = '\0';

        ProcessIntoLabel(fp, record_bytes, v, 1, NULL, oi);

/*              memcpy(struct_name,tmpname,strlen(tmpname)); */
/*              free(tmpname); */


        struct_name[0] = 'O';

    }

/*
** The QUBE (SPECTRAL_QUBE):
** The Qube requires that we not only calculate it's size but that we figure out
** if there are additional pieces of data, ie SUFFIX_PLANES.  We must traverse
** the entire spectral_qube structure looking for 4 things:
**                      1) data 
**                      2) sample_suffix
**                      3) line_suffix
**                      4) band_suffix
**
**      The first one (data) is of course mandatory.  The suffix objects however,
** are not and can appear altogether, in any partial combination, or not
** at all.  Therefore, we MUST determine if any are present and adjust the qube's
** size accordingly.
*/


    else if ((!(strcasecmp("spectral_qube", name))) || (!(strcasecmp("qube", name))) || (!(strcasecmp("image", name)))) {       /*We're processing a qube */

        count = get_struct_names(v, &name_list, NULL);

        for (i = 0; i < count && name_list[i] != NULL; i++) {
            if (!(strcasecmp("sample_suffix", name_list[i])))
                sample_idx = i;
            else if (!(strcasecmp("line_suffix", name_list[i])))
                line_idx = i;
            else if (!(strcasecmp("band_suffix", name_list[i])))
                band_idx = i;

            free(name_list[i]);
        }

        free(name_list);

        if (sample_idx >= 0) {
            get_struct_element(v, sample_idx, &struct_name, &tmpvar);
            oi->total_sample_count =
                set_suffix(tmpvar, &oi->sample_suffix);
        }

        if (line_idx >= 0) {
            get_struct_element(v, line_idx, &struct_name, &tmpvar);
            oi->total_line_count = set_suffix(tmpvar, &oi->line_suffix);
        }

        if (band_idx >= 0) {
            get_struct_element(v, band_idx, &struct_name, &tmpvar);
            oi->total_band_count = set_suffix(tmpvar, &oi->band_suffix);
        }


        count = get_struct_count(v);
        for (i = 0; i < count; i++) {   /*We could assume the DATA object is last, but that could lead to trouble */
            get_struct_element(v, i, &struct_name, &tmpvar);

            if (!(strcasecmp(struct_name, "data"))) {   /*Found it! */

#ifdef LITTLE_ENDIAN
                Var *data = V_DUP(tmpvar);
                char *tmp = V_DATA(data);
                V_DATA(data) = var_endian(data);
                free(tmp);

                oi->obj_data[oi->count] = data;
                oi->obj_dirty[oi->count] = 1;
#else
                oi->obj_data[oi->count] = tmpvar;
                /*Now we should have a pointer to the data */
                oi->obj_dirty[oi->count] = 0;
#endif

                oi->obj_size[oi->count] =
                    (V_SIZE(tmpvar)[0] * V_SIZE(tmpvar)[1] *
                     V_SIZE(tmpvar)[2] * NBYTES(V_FORMAT(tmpvar)));

/*
** We've calculated the size of the CORE but we need to check for SUFFIX
** information which will have already been processed above.
*/

                samples = GetX(tmpvar);
                lines = GetY(tmpvar);
                bands = GetZ(tmpvar);

                oi->obj_size[oi->count] += bands * lines * oi->total_sample_count * 4 + bands * samples * oi->total_line_count * 4 + samples * lines * oi->total_band_count * 4 + bands * oi->total_sample_count * oi->total_line_count * 4;    /*Need to add 2nd corner info! */




                /* Check for alignment and pad as needed */

                if (oi->obj_size[oi->count] % record_bytes) {
                    /* We're not aligned to record bytes..copy and pad */

                    rem = (oi->obj_size[oi->count] / record_bytes);
                    rem++;
                    rem *= record_bytes;
                    oi->Qub_Pad = rem - oi->obj_size[oi->count];        /*#-Bytes we'll need to pad */

                    oi->obj_size[oi->count] = rem / record_bytes;
                } else
                    oi->obj_size[oi->count] /= record_bytes;

                if (!(strcasecmp("image", name)))
                    oi->obj_type[oi->count] = PDS_IMAGE;
                else
                    oi->obj_type[oi->count] = PDS_QUBE;

                oi->count++;
                break;
            }
        }

        /*finish processing the object label */
        /*We send in nulls because at this point....we SHOULD NOT find any labels or objects that we want! */

        get_struct_element(v, 0, &struct_name, &tmpvar);        /*Get the OBJECT Tag and temporarily NULLify it */

/*              tmpname=strdup(struct_name); */
/*              *struct_name='\0'; */

        struct_name[0] = '\0';

        ProcessIntoLabel(fp, record_bytes, v, 0, NULL, oi);

/*              memcpy(struct_name,tmpname,strlen(tmpname)); */
/*              free(tmpname); */

        struct_name[0] = 'O';

    }

    else {
        ProcessIntoLabel(fp, record_bytes, v, 0, NULL, oi);
    }

    fprintf(fp, "END_OBJECT = %s\r\n", name);
}

/*Any Var which has more than a single value must be "iterated" */
void output_big_var(FILE * out, Var * data, char *inset, char *name)
{
    int numelements = V_SIZE(data)[0] * V_SIZE(data)[1] * V_SIZE(data)[2];
    int i;
    char *cp;
    short *sp;
    int *ip;
    float *fp;
    double *dp;
    char dmrk[10];



    fprintf(out, "%s%s = (", inset, name);
    strcpy(dmrk, ", ");
    for (i = 0; i < numelements; i++) {
        if ((i + 1) == numelements)
            strcpy(dmrk, ")\r\n");

        switch (V_FORMAT(data)) {

        case BYTE:
            cp = ((char *) V_DATA(data));
            fprintf(out, "%d%s", cp[i], dmrk);
            break;

        case SHORT:
            sp = ((short *) V_DATA(data));
            fprintf(out, "%d%s", sp[i], dmrk);
            break;


        case INT:
            ip = ((int *) V_DATA(data));
            fprintf(out, "%d%s", ip[i], dmrk);
            break;


        case FLOAT:
            fp = ((float *) V_DATA(data));
            fprintf(out, "%f%s", fp[i], dmrk);
            break;


        case DOUBLE:
            dp = ((double *) V_DATA(data));
            fprintf(out, "%.10g%s", dp[i], dmrk);
            break;
        }
    }
}

Var *ProcessIntoLabel(FILE * fp, int record_bytes, Var * v, int depth,
                      int *label_ptrs, objectInfo * oi)
{
    int i;
    /* int label_ptrs[3]; 0=FILE_RECORDS; 1=LABEL_RECORDS */

    int count;
    Var *data = newVar();
    Var *tmp_var = newVar();
    char *name;
    char pad[26] = { 0 };
    char *pds_filename;
    char inset[1024] = { '\0' };
    char *tmpname;

    depth++;

    if (depth > 0) {
        memset(inset, 0x0, depth + 1);  /* if depth is > 1023 I'll eat my hat! */
        memset(inset, '\t', depth);
    } else
        inset[0] = '\0';


    memset(pad, 0x20, 25);

    count = get_struct_count(v);
    i = 0;

    while (count > 0) {
        get_struct_element(v, i++, &name, &data);

        if (name == NULL) {
            parse_error("Found a NULL element...skipping");
            count--;
            continue;
        } else if (*name == '\0') {
            count--;
            continue;
        }

        /* There are a set of FIXED label items we look for */

        /*Don't want the first label indented...screws up OUR readers */
        if (!(strcasecmp((name), "PDS_VERSION_ID"))) {
            fprintf(fp, "PDS_VERSION_ID = \"PDS3\"\r\n");
        }


        else if (!(strcasecmp((name), "FILE_RECORDS"))) {
            label_ptrs[0] = ftell(fp) + strlen("FILE_RECORDS = ");
            fprintf(fp, "%sFILE_RECORDS = %s\r\n", inset, pad);
        }

        else if (!(strcasecmp(name, "LABEL_RECORDS"))) {
            label_ptrs[1] = ftell(fp) + strlen("LABEL_RECORDS = ");
            fprintf(fp, "%sLABEL_RECORDS = %s\r\n", inset, pad);
        }


        else if (!(strncasecmp(name, "PTR_TO_", 7))) {
            oi->obj_ptr[oi->ptr_count++] = ftell(fp) + strlen((name + 7)) + 4;  /* +4 for the space and ='s */
            tmpname = correct_name((name + 7));
            fprintf(fp, "%s^%s = %s\r\n", inset, tmpname, pad);
            free(tmpname);
        }

        else if (!(strncasecmp(name, "END", 3))) {
            return;             /*This function is used recusively from the group/object functions */
        }

        else if (!(strcasecmp(name, "DATA"))) {
            count--;
            continue;           /*Skip over the data object */
        }

        else if (V_TYPE(data) == ID_STRUCT) {
            char *newname;
            get_struct_element(data, 0, &newname, &tmp_var);

            if ((!(strcasecmp("sample_suffix", name))) ||
                (!(strcasecmp("line_suffix", name))) ||
                (!(strcasecmp("band_suffix", name)))) {
                count--;
                continue;
            }


            else if (strcmp("Object", (newname))) {
                parse_error("Parsing unknown structure");
                ProcessIntoLabel(fp, record_bytes, data, depth, label_ptrs,
                                 oi);
            }

            else {

                if (!(strcmp("GROUP", (V_STRING(tmp_var)))))
                    ProcessGroupIntoLabel(fp, record_bytes, data, name);

                else
                    ProcessObjectIntoLabel(fp, record_bytes, data,
                                           V_STRING(tmp_var), oi);
            }
        }

        else if (V_TYPE(data) == ID_TEXT) {
            int ti;
            tmpname = correct_name(name);
            fprintf(fp, "%s%s = (", inset, tmpname);
            free(tmpname);
            for (ti = 0; ti < V_TEXT(data).Row - 1; ti++)
                fprintf(fp, "%s, ", V_TEXT(data).text[ti]);
            fprintf(fp, "%s)\r\n", V_TEXT(data).text[ti]);
        }




        else if (V_TYPE(data) == ID_STRING) {
            tmpname = correct_name(name);
            fprintf(fp, "%s%s = %s\r\n", inset, tmpname, V_STRING(data));
            free(tmpname);

        }

        else if (V_TYPE(data) == ID_VAL) {

            if (V_SIZE(data)[0] > 1 ||
                V_SIZE(data)[1] > 1 || V_SIZE(data)[2] > 1) {
                int axis_count = 0;
                if (V_SIZE(data)[0] > 1)
                    axis_count++;
                if (V_SIZE(data)[1] > 1)
                    axis_count++;
                if (V_SIZE(data)[2] > 1)
                    axis_count++;
                if (axis_count > 1) {
                    parse_error
                        ("I'm not writing out anything larger than a vector (ie no planes or qubes OTHER than data)");
                    count--;
                    continue;
                }
                output_big_var(fp, data, inset, correct_name(name));
            }

            else {

                tmpname = correct_name(name);
                switch (V_FORMAT(data)) {

                case BYTE:
                    fprintf(fp, "%s%s = %d\r\n", inset, tmpname,
                            (0xff & V_INT(data)));
                    break;

                case SHORT:
                    fprintf(fp, "%s%s = %d\r\n", inset, tmpname,
                            (0xffff & V_INT(data)));
                    break;

                case INT:
                    fprintf(fp, "%s%s = %d\r\n", inset, tmpname,
                            V_INT(data));
                    break;

                case FLOAT:
                    fprintf(fp, "%s%s = %12.6f\r\n", inset, tmpname,
                            V_FLOAT(data));
                    break;

                case DOUBLE:
                    fprintf(fp, "%s%s = %.10g\r\n", inset, tmpname,
                            (*((double *) V_DATA(data))));
                    break;
                }               /*Switch */

                free(tmpname);
            }                   /*else */

        }
        /*else if */
        else {
            parse_error("What the hell is this: %d", V_TYPE(data));
        }

        count--;
        fflush(fp);
    }
}




void
Fix_Label(FILE * fp, int record_bytes, int *label_ptr, objectInfo * oi)
{
    int rem;
    char *pad;
    int size;
    int size_in_records;
    int i;
    int total;

    rewind(fp);
/*
** label_ptr[0] is the #-bytes into fp where the FILE_RECORDS=
** label_ptr[1] is the #-bytes into fp where the LABEL_RECORDS=
** label_ptr[2] is the #-bytes into fp where the label ends.
*/

    size = label_ptr[2];
    fseek(fp, size, SEEK_SET);  /*back to end! */
    size_in_records = size / record_bytes;
    rem = (size % record_bytes);
    if (rem) {                  /*there is always the possibility that we're on the money! */
        size_in_records++;
        rem = (size_in_records * record_bytes) - size;
        pad = (char *) malloc(rem);
        memset(pad, 0x20 /*space */ , rem - 1);
        memset(pad + rem - 1, 0x0a /* \n */ , 1);
        fwrite(pad, sizeof(char), rem, fp);     /*Okay! now our label is padded appropriately */
        fflush(fp);
    }



    total = size_in_records;

/*Now to go back and fix the  label spots*/

    rewind(fp);
    fseek(fp, label_ptr[1] + 1, SEEK_SET);
    fprintf(fp, "%d", size_in_records);
    rewind(fp);

    size_in_records++;

    for (i = 0; i < oi->count; i++) {
        rewind(fp);
        fseek(fp, oi->obj_ptr[i] + 1, SEEK_SET);
        fprintf(fp, "%d", size_in_records);
        size_in_records += oi->obj_size[i];
        total += oi->obj_size[i];
    }

    rewind(fp);
    fseek(fp, label_ptr[0] + 1, SEEK_SET);
    fprintf(fp, "%d", total);

}

Var *WritePDS(vfuncptr func, Var * arg)
{
    Var *v = NULL;
    FILE *fp;
    int force = 0;
    char output[1024];
    char *name;
    Var *data = newVar();
    int flag;
    int count;
    int i;
    char *filename = NULL, *fname = NULL;
    int record_bytes;           /*            Gotta keep track of this baby! */
    Var *result;
    int label_ptr[4];
    objectInfo oi;

    Alist alist[4];
    alist[0] = make_alist("object", ID_STRUCT, NULL, &v);
    alist[1] = make_alist("filename", ID_STRING, NULL, &filename);
    alist[2] = make_alist("force", INT, NULL, &force);
    alist[3].name = NULL;

    if (parse_args(func, arg, alist) == 0)
        return (NULL);

    if (v == NULL) {
        parse_error("%s: No object specfiied.", func->name);
        return (NULL);
    }

    if (filename == NULL) {
        parse_error("%s: No filename specfiied.", func->name);
        return (NULL);
    }

    if ((fname = dv_locate_file(filename)) == (char*)NULL) {
        parse_error("%s: Unable to expand filename %s\n", func->name,
                    filename);
        return (NULL);
    }

    if (!force && access(fname, F_OK) == 0) {
        parse_error("%s: File %s already exists.", func->name, filename);
        return (NULL);
    }

    if ((fp = fopen(fname, "wb")) == NULL) {
        fprintf(stderr, "%s: Unable to open file: %s\n", func->name,
                filename);
        return (NULL);
    }

    oi.obj_ptr = (int *) malloc(sizeof(int) * MAXOBJ);
    oi.obj_size = (int *) malloc(sizeof(int) * MAXOBJ);
    oi.obj_dirty = (int *) malloc(sizeof(int) * MAXOBJ);
    oi.obj_type = (PDS_Object *) malloc(sizeof(PDS_Object) * MAXOBJ);
    oi.obj_data = (void **) malloc(sizeof(void *) * MAXOBJ);
    oi.count = 0;
    oi.ptr_count = 0;
    oi.sample_suffix = NULL;
    oi.total_sample_count = 0;
    oi.line_suffix = NULL;
    oi.total_line_count = 0;
    oi.band_suffix = NULL;
    oi.total_band_count = 0;
    oi.Qub_Pad = 0;

    count = get_struct_count(v);
    i = 0;
    while (count >= 0) {
        get_struct_element(v, i++, &name, &data);       /* check and see if this is what it should be */
        if (!(strcasecmp("record_bytes", name))) {
            record_bytes = V_INT(data); /*Contains the integer value for the size of a record */
            break;
        }
        count--;
    }
    if (count < 0) {
        parse_error
            ("Your object doesn't contain the necessary elements for a PDS label");
        fclose(fp);
        return (NULL);
    }

    ProcessIntoLabel(fp, record_bytes, v, -1, label_ptr, &oi);
    fprintf(fp, "END\r\n");
    label_ptr[2] = ftell(fp);
    Fix_Label(fp, record_bytes, label_ptr, &oi);
    fseek(fp, 0, SEEK_END);
/*
** At this point, the data object should have themselves
**  been padded as needed so that we can we can just dump
** 'em
*/

    for (i = 0; i < oi.count; i++) {
        if (oi.obj_type[i] != PDS_QUBE) {
            fwrite(oi.obj_data[i], sizeof(char),
                   (oi.obj_size[i] * record_bytes), fp);
            if (oi.obj_dirty[i]) {
                free(oi.obj_data[i]);
            }
        } else {
            if (write_PDS_Qube
                ((Var *) oi.obj_data[i], oi.sample_suffix, oi.line_suffix,
                 oi.band_suffix, fp) == NULL) {
                parse_error("Error Writing Qube!");
                fclose(fp);
                return (NULL);
            }
            if (oi.Qub_Pad) {
                char *pad = (char *) malloc(oi.Qub_Pad * sizeof(char));
                memset(pad, 0x20, oi.Qub_Pad);
                fwrite(pad, sizeof(char), oi.Qub_Pad, fp);
            }
        }
    }
    fclose(fp);
    return (NULL);
}


#ifdef HAVE_LIBISIS
Var *
ff_write_isis_cub(vfuncptr func, Var *args)
{
    Var   *obj = NULL;
    char  *fname = NULL;
    int    force = 0;
    Alist  alist[4];

    alist[0] = make_alist("obj",      ID_STRUCT, NULL, &obj);
    alist[1] = make_alist("filename", ID_STRING, NULL, &fname);
    alist[2] = make_alist("force",    INT,       NULL, &force);
    alist[3].name = NULL;

    if (parse_args(func, args, alist) == 0) return NULL;

    if (obj == NULL){
        parse_error("%s(): Missing \"%s\".", func->name, alist[0].name);
		parse_error("Usage: %s(obj=STRUCT, filename=STRING [, force=1])", func->name);
		parse_error("Example: a=load_pds(\"foo.cub\"); %s(a, \"bar.cub\")", func->name);
        return NULL;
    }
    
    if (fname == NULL){
        parse_error("%s(): Missing \"%s\".\n", func->name, alist[1].name);
        return NULL;
    }

    return newInt(dv_WriteISISStruct(obj, fname, force));
}
#endif /* HAVE_LIBISIS */


/**
 ** Historically ReadPDS did not load suffix data. Thus loading
 ** suffix data by default may break davinci scripts left and right.
 ** Also, integrating this conditional load of suffix data does not
 ** fit very well in the current call structure of rf_XXXX functions.
 ** In order for a proper fit to happen, we may want an element by
 ** the name of "function_data", which holds the special data needed
 ** by one particular reader function.
 **
 ** For now, I am controlling loading of suffix data using the
 ** following global variable.
 **/
Var *do_loadPDS(vfuncptr func, char *filename, int data, int suffix_data);

static int LOAD_SUFFIX_DATA = 0;
Var *ReadPDS(vfuncptr func, Var * arg)
{
    Var *fn = NULL;
    char *filename = NULL;
    int data = 1;
    int suffix_data = 0;
	int i;

    Alist alist[4];
    alist[0] = make_alist("filename", ID_UNK, NULL, &fn);
    alist[1] = make_alist("data", INT, NULL, &data);
    alist[2] = make_alist("suffix_data", INT, NULL, &suffix_data);
    alist[3].name = NULL;

    if (parse_args(func, arg, alist) == 0)
        return (NULL);

    /* see the comment above LOAD_SUFFIX_DATA's declaration */
    LOAD_SUFFIX_DATA = suffix_data;

	/* Handle loading many filenames */
    if (V_TYPE(fn) == ID_TEXT) {
		Var *s = new_struct(V_TEXT(fn).Row);
		for (i = 0; i < V_TEXT(fn).Row; i++) {
			filename = strdup(V_TEXT(fn).text[i]);
			Var *t = do_loadPDS(func, filename, data, suffix_data);
			if (t) { 
				add_struct(s, filename, t);
			}
		}
		if (get_struct_count(s)) {
			return (s);
		} else {
			free_struct(s);
			return (NULL);
		}
    } else if (V_TYPE(fn) == ID_STRING) {
        filename = V_STRING(fn);
		return(do_loadPDS(func, filename, data, suffix_data));
    } else {
        parse_error("Illegal argument to function %s(%s), expected STRING",
                    func->name, "filename");
        return (NULL);
    }
}


Var *do_loadPDS(vfuncptr func, char *filename, int data, int suffix_data)
{
    int ac;
    Var **av;

    OBJDESC *ob;
    KEYWORD *key;
    char *err_file = NULL;
    int obn = 1;
    Var *fn = NULL;
    int record_bytes;
	char *fname;

    FILE *fp;

    Var * v;

    if (filename == NULL) {
        parse_error("%s: No filename specified\n", func->name);
        return (NULL);
    }
    if ((fname = dv_locate_file(filename)) == (char*)NULL) {
        parse_error("%s: Unable to expand filename %s\n", func->name,
                    filename);
        return (NULL);
    }

    if (access(fname, R_OK) != 0) {
        parse_error("%s: Unable to find file %s.", func->name, filename);
        return (NULL);
    }

    Init_Obj_Table();

/**
*** What about compression?
**/
    if ((fp = fopen(fname, "rb")) != NULL) {
        if (iom_is_compressed(fp)) {
             /* fprintf(stderr, "is compressed\n");    FIX: remove */
             fclose(fp);
             fname = iom_uncompress_with_name(fname);
             fp = fopen(fname, "rb");
		} 
	}

/* In load_pds case, we will allow IO modules to take a crack at it first,
   chiefly because our ODL parser is a very spammy parser and doesn't detect
   that it may be weird until it does a lot of screen output.  If we can
   sidetrack it with a more elegant, purpose-built IO module, let's do so.

   Of course, there's no guarantee that an I/O module won't be spammy either.
*/

    if ((v = load_pds_from_io_module(fp, fname, data, suffix_data)) != NULL) {
        fclose(fp); 
        return v;
    }
    ob = (OBJDESC *) OdlParseLabelFile(fname, err_file,
                                       ODL_EXPAND_STRUCTURE, VERBOSE == 0);
	fclose(fp);

    if ((key =
         OdlFindKwd(ob, "RECORD_BYTES", NULL, 0,
                    ODL_THIS_OBJECT)) == NULL) {
        parse_error("Hey! This doesn't look like a PDS file");
		record_bytes = 0;
    } else {
		record_bytes = atoi(key->value);
	}
    v = new_struct(0);
    Traverse_Tree(ob, v, record_bytes);

    if (data) {
        Read_Object(v);
    }

    return (v);
}


int rf_QUBE(char *fn, Var * ob, char *val, int dptr)
{
    FILE *fp;
    Var *data = NULL, *suffix_data = NULL;

    
    fp = fopen(fn, "rb");
    
    data = (Var *) dv_LoadISISFromPDS(fp, fn, dptr);
    if (data == NULL) { fclose(fp); return 1; }
    add_struct(ob, fix_name("DATA"), data);

    if (LOAD_SUFFIX_DATA){
        suffix_data = (Var *) dv_LoadISISSuffixesFromPDS(fp, fn);
        if (suffix_data == NULL){ fclose(fp); return 1; }
        
        /* create a suffix part only when there are suffixes, i.e.
           avoid blank suffix structure */
        if (get_struct_count(suffix_data) > 0){
            add_struct(ob, fix_name("SUFFIX_DATA"), suffix_data);
        }
        else {
            mem_claim(suffix_data);
            free_struct(suffix_data);
            /* NOTE: if one does not do mem_claim and free_struct
               the garbage collector will take care of it */
        }
    }

    fclose(fp);
    return (0);
}

int
rf_BitField(int *j, char **Bufs, char *tmpbuf, FIELD ** f, int ptr,
            int row, int *size)
{
    int i = (*j);
/*
** We've already loaded the entire contents of the all the bit collumns into a single value,
** now we're going to iterate through each bit-collumn and place it (but first we step one forward
** so that we are now on the first bit collumn
*/
    i++;
    while (f[i]->bitfield != NULL) {
        memcpy(Bufs[i] + row * size[i], (tmpbuf + ptr), size[i]);       /*Copy the whole thing in we'll fix it later */
        i++;
    }
    *j = (i - 1);
}



int rf_TABLE(char *fn, Var * ob, char *val, int dptr)
{
    LABEL *label;
    Var *Data;
    char **Bufs;
    char *tmpbuf;
    int i, j;
    int fp;
    int Offset;
    FIELD **f;
    int step;
    int *size;
    int err;
    int num_items = 0;

    label = LoadLabel(fn);
    if (label == NULL) {
        fprintf(stderr, "Unable to load label from \"%s\".\n",
                ((fn == NULL) ? "(null)" : fn));
        return 1;
    }

    f = (FIELD **) label->fields->ptr;

    num_items = label->fields->number;  /*This is a count of BOTH columns AND bit-columns */

/*Add new structure to parent ob*/
    Data = new_struct(0);

    /*Initialize a set of buffers to read in the data */

    Bufs = (char **) calloc(num_items, sizeof(char *));
    tmpbuf = (char *) calloc(label->reclen, sizeof(char));
    size = (int *) calloc(num_items, sizeof(int));
    for (j = 0; j < num_items; j++) {

        if (f[j]->dimension)
            size[j] = f[j]->size * f[j]->dimension;
        else
            size[j] = f[j]->size;

        Bufs[j] = (char *) calloc((label->nrows * size[j]), sizeof(char));
    }

    Offset = dptr;

#ifdef __CYGWIN__
    fp = open(fn, O_RDONLY | O_BINARY, 0);
#else
    fp = open(fn, O_RDONLY, 0);
#endif                          /* __CYGWIN__ */
    lseek(fp, Offset, SEEK_SET);
    for (i = 0; i < label->nrows; i++) {

        /*Read each row as a block */
        if ((err = read(fp, tmpbuf, label->reclen)) != label->reclen) {
            fprintf(stderr,
                    "Bad data in file:%s\n Read %d byte(s), should be %d\n",
                    fn, err, label->reclen);
            return (1);
        }

        step = 0;

        for (j = 0; j < num_items; j++) {
            /*Place in the approiate buffer */

            memcpy((Bufs[j] + i * size[j]), (tmpbuf + step), size[j]);
            step += size[j];

            if (f[j]->eformat == MSB_BIT_FIELD)
                rf_BitField(&j, Bufs, tmpbuf, f, (step - size[j]), i,
                            size);
        }
    }
    close(fp);

    /*Set each field Var's data and parameter information */
    Set_Col_Var(&Data, f, label, size, Bufs);

    add_struct(ob, fix_name("DATA"), Data);

    free(tmpbuf);
    free(size);
    for (j = 0; j < num_items; j++) {
        free(Bufs[j]);
    }
    free(Bufs);
    return (0);
}
double Scale(int size, void *ptr, FIELD * f)
{
    unsigned char *cp;
    unsigned int *ip;
    unsigned short *sp;
    float *fp;
    double *dp;
    char num[256];              /* this was WAY too small at [9] */

/*Set up pointer casts for our data type*/

    cp = ptr;
    ip = ptr;
    sp = ptr;
    fp = ptr;
    dp = ptr;

    switch (f->eformat) {
    case MSB_INTEGER:
    case MSB_UNSIGNED_INTEGER:
    case LSB_INTEGER:
    case LSB_UNSIGNED_INTEGER:
        switch (f->size) {
        case 4:
            return ((double) ip[0] * f->scale + f->offset);
        case 2:
            return ((double) sp[0] * f->scale + f->offset);
        case 1:
            return ((double) cp[0] * f->scale + f->offset);
        }

        break;

    case IEEE_REAL:
    case PC_REAL:
        switch (f->size) {
        case 8:
            return (dp[0] * f->scale + f->offset);
        case 4:
            return ((double) fp[0] * f->scale + f->offset);
        }

        break;
    case ASCII_INTEGER:
        memcpy(num, cp, f->size);
        num[f->size] = '\0';
        return ((double) (atoi(num)) * f->scale + f->offset);

        break;
    case ASCII_REAL:
        memcpy(num, cp, f->size);
        num[f->size] = '\0';
        return ((double) (atof(num)) * f->scale + f->offset);
    }

    return (0);
}


char *DoScale(int rows, FIELD * f, char *in)
{
    double *out;
    void *ptr;
    int count = 0;
    double Val;
    int i, j;
    int dim = (f->dimension ? f->dimension : 1);
    int size = f->size;

    out = (double *) calloc(rows * dim, sizeof(double));
    ptr = in;
    for (i = 0; i < rows; i++) {
        for (j = 0; j < dim; j++) {
            out[count++] = Scale(size, ptr, f);
            ptr = (unsigned char *) ptr + size;
        }
    }

    free(in);
    return ((char *) out);
}

void
Set_Col_Var(Var ** Data, FIELD ** f, LABEL * label, int *size, char **Bufs)
{
    int j, i;
    void *data;
    char **text;
    Var *v;
    char num[9];
    int inum;
    double fnum;
    int step;
    int dim;
    int num_items;

    num_items = label->fields->number;

/**
*** Note: the calloc in all these routines is stupid, as it
***       allocates (nbytes*nitems, 1)
**/

    for (j = 0; j < num_items; j++) {
        dim = (f[j]->dimension ? f[j]->dimension : 1);
        step = 0;

        /* Do appropriate endian conversion */
        switch (f[j]->eformat) {
        case LSB_INTEGER:
        case LSB_UNSIGNED_INTEGER:
        case LSB_BIT_FIELD:
            LSB(Bufs[j], label->nrows * dim, f[j]->size);
            break;
        case MSB_INTEGER:
        case MSB_UNSIGNED_INTEGER:
        case MSB_BIT_FIELD:
            MSB(Bufs[j], label->nrows * dim, f[j]->size);
            break;
        case IEEE_REAL:
            MSB(Bufs[j], label->nrows * dim, f[j]->size);
            break;
        case PC_REAL:
            LSB(Bufs[j], label->nrows * dim, f[j]->size);
            break;
        }

        if (f[j]->scale) {
            /* 
               ** special case: result is always a double 
               ** regardless of input size
             */
            Bufs[j] = DoScale(label->nrows, f[j], Bufs[j]);
            data = calloc(dim * 8 * label->nrows, sizeof(char));
            memcpy(data, Bufs[j], dim * 8 * label->nrows);
            v = newVal(BSQ, dim, label->nrows, 1, DOUBLE, data);
        } else {
            switch (f[j]->eformat) {
            case CHARACTER:
                text = (char **) calloc(label->nrows, sizeof(char *));
                for (i = 0; i < label->nrows; i++) {
                    text[i] = (char *) calloc(size[j] + 1, sizeof(char));
                    memcpy(text[i], (Bufs[j] + i * size[j]), size[j]);
                    text[i][size[j]] = '\0';
                }
                v = newText(label->nrows, text);
                break;

            case MSB_INTEGER:
            case MSB_UNSIGNED_INTEGER:
            case LSB_INTEGER:
            case LSB_UNSIGNED_INTEGER:
                data = calloc(size[j] * label->nrows, sizeof(char));
                memcpy(data, Bufs[j], size[j] * label->nrows);

                switch (f[j]->size) {
                case 4:
                    v = newVal(BSQ, dim, label->nrows, 1, INT, data);
                    break;
                case 2:
                    v = newVal(BSQ, dim, label->nrows, 1, SHORT, data);
                    break;
                case 1:
                    v = newVal(BSQ, dim, label->nrows, 1, BYTE, data);
                }
                break;


            case IEEE_REAL:
            case PC_REAL:
                data = calloc(size[j] * label->nrows, sizeof(char));
                memcpy(data, Bufs[j], size[j] * label->nrows);

                switch (f[j]->size) {
                case 8:
                    v = newVal(BSQ, dim, label->nrows, 1, DOUBLE, data);
                    break;
                case 4:
                    v = newVal(BSQ, dim, label->nrows, 1, FLOAT, data);
                    break;
                }
                break;

            case ASCII_INTEGER:
                data = calloc(sizeof(int) * label->nrows, sizeof(char));
                for (i = 0; i < (label->nrows * dim); i++) {
                    memcpy(num, Bufs[j] + f[j]->size * i, f[j]->size);
                    num[f[j]->size] = '\0';
                    inum = atoi(num);
                    memcpy((char *) data + step, &inum, sizeof(int));
                    step += sizeof(int);
                }

                v = newVal(BSQ, dim, label->nrows, 1, INT, data);
                break;

            case ASCII_REAL:
                data = calloc(sizeof(double) * label->nrows, sizeof(char));
                for (i = 0; i < (label->nrows * dim); i++) {
                    memcpy(num, Bufs[j] + f[j]->size * i, f[j]->size);
                    num[f[j]->size] = '\0';
                    fnum = atof(num);
                    memcpy((char *) data + step, &fnum, sizeof(double));
                    step += sizeof(double);
                }

                v = newVal(BSQ, dim, label->nrows, 1, DOUBLE, data);
                break;

            case BYTE_OFFSET:
                data = calloc(size[j] * label->nrows, sizeof(char));
                memcpy(data, Bufs[j], size[j] * label->nrows);
                v = newVal(BSQ, dim, label->nrows, 1, INT, data);
                break;

            case MSB_BIT_FIELD:
            case LSB_BIT_FIELD:
                data = calloc(size[j] * label->nrows, sizeof(char));
                memcpy(data, Bufs[j], size[j] * label->nrows);
                v = newVal(BSQ, dim, label->nrows, 1, INT, data);
                break;

            }
        }
        add_struct(*Data, fix_name(f[j]->name), v);
    }
}

int rf_IMAGE(char *fn, Var * ob, char *val, int dptr)
{
    FILE *fp;
    Var *data = NULL;
    fp = fopen(fn, "rb");
    data = (Var *) dv_LoadISIS(fp, fn, NULL);
    if (data != NULL) {
        add_struct(ob, fix_name("DATA"), data);
        fclose(fp);
        return (0);
    }

    fclose(fp);
    return (1);
}

int rf_HISTOGRAM_IMAGE(char *fn, Var * ob, char *val, int dptr)
{
    return (0);
}

char *history_parse_buffer(FILE * in)
{
    char buf[1024];
    int max = 2048;
    int ptr = 0;
    char *TheString;

    TheString = (char *) malloc(max);


    while (1) {
        fgets(buf, 1023, in);   /*Suck in the next line */

        if ((ptr + strlen(buf)) >= max) {
            max *= 2;
            TheString = (char *) realloc((TheString), max);
        }

        strcpy((TheString + ptr), buf);
        ptr += strlen(buf);

        if (!(strncasecmp("END", buf, 3)))
            if (!(strncasecmp("END_", buf, 4)))
                continue;
            else
                break;

    }

    TheString = (char *) realloc(TheString, strlen(TheString));

    return (TheString);
}

/*
** history_remove_isis_indents - removes the history-indent
** marks from ISIS history objects. It removes these indents
** blindly.
** Returned history is new memory block - it is the
** caller's responsibility to free() it.
*/
static char *
history_remove_isis_indents(const char *history)
{
    char *src_hist = strdup(history);
	int   src_hist_len = strlen(history);
	char *tgt_hist = NULL;
    char *line, **lines;
    char *p;
    LIST *lines_list;
    regex_t indent_regex;
    regmatch_t matches[1];
    int i, n;
    int rc;

    rc = regcomp(&indent_regex, "^(([|]{1,5})|([|][0-9]{3}[|]))(----)?", REG_EXTENDED);
    if (rc != 0){
        parse_error("regcomp() failed for indents pattern.");
        return NULL;
    }
    
    lines_list = new_list();
    for(p = src_hist; line = strtok(p, "\n"); p = NULL){
        list_add(lines_list, line);
    }

    
    n = list_count(lines_list);
    lines = (char **)list_data(lines_list);
    
    for(i = 0; i < n; i++){
        /* remove ISIS style vertical-bar indent-end marker */
        rc = regexec(&indent_regex, lines[i], sizeof(matches)/sizeof(regmatch_t), matches, 0);
        if (rc == 0){
            /* a match was found: get rid of indent */
            strcpy(lines[i], &lines[i][matches[0].rm_eo]);
        }
    }

    regfree(&indent_regex);

	tgt_hist = (char *)calloc(sizeof(char), src_hist_len+10);
    strcpy(tgt_hist, "");
    for(i = 0; i < n; i++){
        if (i > 0){ strcat(tgt_hist, "\n"); }
        strcat(tgt_hist, lines[i]);
    }

    list_free(lines_list);
    free(src_hist);
    
	return tgt_hist;
}

int rf_HISTORY(char *fn, Var * ob, char *val, int dptr)
{
    int i;
    int count = get_struct_count(ob);
    Var *tmp;
    char *name;
    int size = 0;
    FILE *in;
    Var *data = new_struct(0);
    char *history, *p;
    OBJDESC *top;


    in = fopen(fn, "rb");
    fseek(in, dptr, SEEK_SET);
    tmp = newVar();
    V_TYPE(tmp) = ID_STRING;
    V_STRING(tmp) = strdup("HISTORY");
    add_struct(data, "Object", tmp);


/*Read in the history object as a giant string*/
    history = history_parse_buffer(in);
    fclose(in);

    /* Assuming ISIS history object: remove any indent marks */
    p = history_remove_isis_indents(history);
	if (p != NULL){
		/* replace history text with history-text-without-indent-marks */
		free(history);
		history = p;
	}

/*Call the OdlParseLabelString fucntion and make it an ODL object*/
    top =
        OdlParseLabelString(history, NULL, ODL_EXPAND_STRUCTURE,
                            VERBOSE == 0);
    Traverse_Tree(top, data, 0);

    if (get_struct_count(data))
        add_struct(ob, fix_name("DATA"), data);

    return (0);
}

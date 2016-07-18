#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <strings.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <pwd.h>
#include <math.h>
#include "parser.h"
#include "io_loadmod.h"
/* #include "dvio.h" */
/* ISIS3 API includes */
#include "Cube.h"
#include "Brick.h"
#include "iException.h"
#include "Table.h"
#include "History.h"

#define DV_NAMEBUF_MAX 1025
#define DV_ISIS_STRUCT_TYPE "isis_struct_type"
#define DI3_VERSION "0.9.2 (ISIS 3.3.x)"
static Typelist t[] = {
    {(char*)"isis3", NULL, NULL, NULL},
    {NULL, NULL, NULL, NULL}
};

#define string_equal(a,b) !strcmp(a,b)
#define string_equal_ic(a,b) !strcasecmp(a,b)
#define KWD_TO_CHAR_PTR(x) (char *)((x).ToQt().toAscii().data())

#define CUBE_ASSOC "cube_association"
#define SAMPLE_STR "Sample"
#define LINE_STR   "Line"
#define BAND_STR   "Band"
#define NONE_STR   "None"

#define dvnewstr(x) newString(strdup(x))
#define TD_FORMAT "%Y-%m-%dT%H:%M:%S"

static int check_ISIS_env() {
    /* If your ISISROOT is not set properly, the ISIS3 API fails HARD with a
       SIGSEGV.  This check should be run before any ISIS3 Object is instantiated,
       or the program will crash if the environment is incorrect.  Also on this note,
       ALWAYS declare pointers to ISIS3 objects and instantiate them with new.  If
       you do not, they will instantiate when davinci starts and trigger the SEGV
       before you even get started.

       Addendum 14 May 2009 (rsk): This bug is theoretically fixed in ISIS
       release 3.1.19, but the code is kept here because ISIS still needs its
       prefs to work correctly, but at least it doesn't segfault any more.
    */
    static int setup_correct = 0;
    struct stat dummy;
    std::string * isisprefs;
    if (setup_correct) return 1;
    if (getenv("ISISROOT") != NULL) {
        isisprefs = new std::string(getenv("ISISROOT"));
        isisprefs->append("/IsisPreferences");
        if (stat(isisprefs->data(), &dummy) == 0) {
            setup_correct = 1;
        }
        delete isisprefs;
    }
    else {
        //parse_error("ISISROOT is null. setup_correct = %i", setup_correct);
    }
    if (setup_correct == 0) {
        parse_error("Warning: ISIS Environment not setup correctly to use ISIS3 API.\nPlease set your ISISROOT environment variable to the root of an ISIS3 install,\nor undesired operation may result.");
    }
    return setup_correct;
}

extern "C" void
dv_iomod_init(TypelistPtr * t1) {
    parse_error("Initializing davinci/ISIS3 IO module version %s", DI3_VERSION);
    check_ISIS_env();
    *t1 = t;
}

static void make_unique_name(char * buffer, Var * dv_struct) {
    /*

        Make unique names to be inserted in the davinci structure dv_struct by
        checking to see if the candidate name already exists. If it does not,
        the name remains unchanged.  If it does, it appends a _ and an integer
        value (starting at 2) and keeps trying, incrementing the integer by one,
        until a unique name is found.

        The buffer value is changed to the unique name by this function.

    */
    int start_count = 2;
    char work_buf[DV_NAMEBUF_MAX];
    Var * dummy_sptr;
    strncpy(work_buf, buffer, DV_NAMEBUF_MAX-1);
    while (find_struct(dv_struct, work_buf, &dummy_sptr) != -1) {
        if (snprintf(work_buf, DV_NAMEBUF_MAX-1, "%s_%i", buffer, start_count++) >= DV_NAMEBUF_MAX-1) {
            parse_error("Warning: attempt to find unique name for '%s' results in truncated strings.\nCannot rename. Please use shorter names in your ISIS files!");
            return;
        }
    }
    strncpy(buffer, work_buf, DV_NAMEBUF_MAX-1);
}



static char get_keyword_datatype(char * value, char * name) {
    /* Determines the davinci datatype to apply to a value string, given its value
       and its name for hints. */
    char * endptr;
    int intval;
    double floatval;
    int is_int = 0, is_double = 0;
    errno = 0;

    /* check for int value. Note that the whole string must convert to be an int
    or it is more likely a string like a date/time value */

    intval = strtol(value, &endptr, 10);
    if (errno) is_int = 0; else is_int = 1;
    if (endptr != value+strlen(value)) is_int = 0;

    /* check for float value. Note that like the int, the whole string must convert 
    to be a float or it is more likely a string like a date/time value */

    floatval = strtod(value, &endptr);
    if (errno) is_double = 0; else is_double = 1;
    if (endptr != value+strlen(value)) is_double = 0;

    /* if both tests pass, check to see if they are numerically equal. Floating point
       values only occasionally integral, and davinci can do the appropriate casting
       in any case, so it's a good compromise. */

    /*  If the name of the field contains the string 'Version', don't translate it 
        to a number, but keep it as a string.
    */
    if (name != NULL && strstr(name, "Version") != NULL) {
        is_int = 0;
        is_double = 0;
    }
    if (name != NULL && strstr(name, "SpacecraftClockCount") != NULL) {
        is_int = 0;
        is_double = 0;
    }
    if (is_int && is_double) {
        if (intval == floatval) {
            is_double = 0;
        }
        else {
            is_int = 0;
        }
    }

    if (is_double) return 'd';
    if (is_int) return 'i';
    return 'c';

}

static void add_converted_to_struct(Var * dv_struct, char * name, char * value) {
    /* Attempts to convert keyword values to integers and floats to determine if
    the value supplied is a numeric value. The rules are:
    Try to make it an int and a float.
    If they are both, compare the values and if they are equal, make it an int.
    If they are not, make it a float.
    If only one conversion worked, use that conversion.
    If neither conversion worked, then make it a string.
    */

    char datatype;

    datatype = get_keyword_datatype(value, name);

    if (datatype == 'i') {
        add_struct(dv_struct, name, newInt(atol(value)));
    }
    else if (datatype == 'd') {
        add_struct(dv_struct, name, newDouble(atof(value)));
    }
    else {
        add_struct(dv_struct, name, newString(strdup(value)));
    }
}

static void explode_keyword(Isis::PvlKeyword kwd, Var * dv_struct) {
   /*
        Takes an ISIS3 keyword object and makes a davinci name value pair
        for it to be appended to the davinci structure passed in dv_struct.

        The value is type converted by attempting to coerce the string value
        into numeric and array types, and a suitable davinci container is found
        for the value. 
   */

    Var * varlist;
    double * float_block;
    int * int_block;
    char namebuf[DV_NAMEBUF_MAX];
    char dtype, lc_dtype = 'i';

    snprintf(namebuf, DV_NAMEBUF_MAX-1, "%s", (char *)(kwd.Name().data()));
    namebuf[DV_NAMEBUF_MAX-1] = '\0';
    make_unique_name(namebuf, dv_struct);
    if (kwd.Size() == 1) {
        add_converted_to_struct(dv_struct, namebuf, KWD_TO_CHAR_PTR(kwd[0]));
    }
    else {
        for (int j=0; j<kwd.Size(); j++) {
            dtype = get_keyword_datatype(KWD_TO_CHAR_PTR(kwd[j]), namebuf);
            if (dtype == 'c') {
                lc_dtype = 'c';
                break;
            }
            if (dtype == 'd') {
                lc_dtype = 'd';
            }
        }
        switch (lc_dtype) {
          case 'c':
            /* At least one string type in this array. All we can do is make it
               an anonymous, mixed structure. */
            varlist = new_struct(0);
            for (int j=0; j<kwd.Size(); j++) {
                add_converted_to_struct(varlist, NULL, KWD_TO_CHAR_PTR(kwd[j]));
            }
            break;
          case 'd':
            /* The values in this array can at least be interpreted as floats.
               Allocate a range of doubles and make a sizex1x1 array of them */
            float_block = (double *)calloc((size_t)kwd.Size(), sizeof(double));
            if (float_block == NULL) {
                parse_error("calloc in explode_keyword failed for %i doubles required for keyword '%s'.\nMemory is probably low.", kwd.Size(), namebuf);
                varlist = newInt(0);
                break;
            }
            for (int j=0; j<kwd.Size(); j++)
                float_block[j] = atof(KWD_TO_CHAR_PTR(kwd[j]));
              varlist = newVal(BSQ, kwd.Size(), 1, 1, DOUBLE, float_block);
            break;
          case 'i':
            /* The values are integers... make an array of ints. */
            int_block = (int *)calloc((size_t)kwd.Size(), sizeof(int));
            if (int_block == NULL) {
                parse_error("calloc in explode_keyword failed for %i ints required for keyword '%s'.\nMemory is probably low.", kwd.Size(), namebuf);
                varlist = newInt(0);
                break;
            }
            for (int j=0; j<kwd.Size(); j++)
                int_block[j] = atoi(KWD_TO_CHAR_PTR(kwd[j]));
              varlist = newVal(BSQ, kwd.Size(), 1, 1, INT, int_block);
            break;
          default:
            parse_error("Unknown datatype for array '%s'. This is a bug!\nPlease report this bug to http://davinci.asu.edu/", namebuf);
            varlist = newInt(0);
            break;
        }
        add_struct(dv_struct, namebuf, varlist);
    }
}

static void get_keywords(Isis::PvlContainer * pvl, Var * dv_struct, int suppress_name_kwd) {
    /* Extract keywords from any pvl instance */
    if (pvl->Keywords() > 0) {
        for (int i=0; i<pvl->Keywords(); i++) {
            if (suppress_name_kwd && ((*pvl)[i].Name() == "Name")) continue;
            explode_keyword((*pvl)[i], dv_struct);
        }
    }
}

static Var * explode_object(Isis::PvlObject o) {

    /* takes any ISIS3 PvlObject and returns a davinci structure representing
       it.  Works recursively to catch objects contained within objects.
    */

    Var * rtn_value;
    Var * kwds;
    Var * cur_obj;
    rtn_value = new_struct(0);
    char namebuf[DV_NAMEBUF_MAX];

    /* first, add keywords at this level */
    get_keywords(&o, rtn_value,0);


    /* get sub-objects */
    if (o.Objects() > 0) {
        for (int i=0; i<o.Objects(); i++) {
            snprintf(namebuf, DV_NAMEBUF_MAX-1, "%s", (char *)o.Object(i).Name().data());
            namebuf[DV_NAMEBUF_MAX-1] = '\0';
            make_unique_name(namebuf, rtn_value);
            cur_obj = explode_object(o.Object(i));
            add_struct(cur_obj, DV_ISIS_STRUCT_TYPE, newString(strdup("object")));
            add_struct(rtn_value, namebuf, cur_obj);
        }
    }
   
    /* get groups at this level */
    if (o.Groups() > 0) {
        for (int i=0; i<o.Groups(); i++) {
            snprintf(namebuf, DV_NAMEBUF_MAX-1, "%s", (char *)o.Group(i).Name().data());
            namebuf[DV_NAMEBUF_MAX-1] = '\0';
            make_unique_name(namebuf, rtn_value);
            kwds = new_struct(0);
            get_keywords(&(o.Group(i)), kwds, 0);
            add_struct(kwds, DV_ISIS_STRUCT_TYPE, newString(strdup("group")));
            add_struct(rtn_value, namebuf, kwds);
        }
    }

    return rtn_value;
}



static void get_history(Isis::Cube * cube, char * hname, Var * dv_struct) {
    /* get an ISIS 3 history blob from an ISIS3 cube for the given name in
       hname.  Turn the history blob into a davinci representation and append
       it to dv_struct. */

    Isis::History * history;
    Isis::Pvl hpvl;
    Var * current_var, * hist_var;
    char namebuf[DV_NAMEBUF_MAX];

    history = new Isis::History(hname);
    cube->read(*history);
    hpvl = history->ReturnHist();
    if (hpvl.Objects() > 0 || hpvl.Groups() > 0 || hpvl.Keywords() > 0) {
        hist_var = new_struct(0); 
        for (int i=0; i<hpvl.Objects(); i++) {
            current_var = explode_object(hpvl.Object(i));
            snprintf(namebuf, DV_NAMEBUF_MAX-1, "%s", (char *)hpvl.Object(i).Name().data());
            make_unique_name(namebuf, hist_var);
            add_struct(hist_var, namebuf, current_var);
            add_struct(current_var, DV_ISIS_STRUCT_TYPE, newString(strdup("object")));
        }
        add_struct(hist_var, DV_ISIS_STRUCT_TYPE, newString(strdup("history")));
        add_struct(dv_struct, "History", hist_var);  
    }
}

static char get_tablefield_datatype(Isis::TableField * tf) {
    /* Get the data type for an Isis::TableField object as a simple character,
       suitable of if else if or switch/case logic.
    */

    char rtn_val;
    if (tf->IsInteger()) rtn_val = 'i';
    else if (tf->IsDouble()) rtn_val = 'd';
    else if (tf->IsText()) rtn_val = 'c';
    else if (tf->IsReal()) rtn_val = 'f';
    else rtn_val = '?';
    return rtn_val;
}

static char get_tablecollective_datatype(Isis::Table t) {

    /* Get the common data type for an ISIS table record. 
       Values are:
       i - Integer
       d - double precision floating point
       f - single precision floating point
       c - character strings
       m - mixed data

   */

    Isis::TableRecord tr;
    char rtn_val = '\0';
    char cur_type;
    tr = t[0];
    for (int i=0; i<tr.Fields(); i++) {
        cur_type = get_tablefield_datatype(&tr[i]);
        if (rtn_val != '\0' and rtn_val != cur_type) {
            rtn_val = 'm';
            break;
        }
        rtn_val = cur_type;
    }
    return rtn_val;
}

static void get_data_for_table(Isis::Table t, Var * dv_struct) {

    /* extract the actual data from an ISIS table into a davinci array.

       Currently only supports unmixed tables of double, float, or integer where
       the data size of all fields are 1. Mixed tables are unimplemented, and the
       vector parts of tables are not loaded. TODO

    */

    Isis::TableRecord cur_rec;
    Isis::TableField * cur_fld;
    char collective_type;
    void * dv_table_data;
    size_t x=0, y=0, xpos;

    cur_rec = t[0];
    /* Get x and y dimensions of table */
    //parse_error("Getting table '%s' data.", t.Name().data());
    for (int i=0; i<cur_rec.Fields(); x += (size_t)cur_rec[i++].Size());
    y = (size_t)t.Records();

    collective_type = get_tablecollective_datatype(t);
    if (collective_type == 'd') {
        dv_table_data = calloc(x*y, sizeof(double));
        if (dv_table_data == NULL) {
            parse_error("calloc for double table failed in ISIS3 module get_data_for_table.\nMemory is probably low.");
            return;
        }
        for (int i=0; i<t.Records(); i++) {
            cur_rec = t[i];
            xpos = 0;
            for (int j=0; j<cur_rec.Fields(); j++) {
                cur_fld = &(cur_rec[j]);
                if (cur_fld->Size() == 1) {
                    double dbl_store = double(*cur_fld);
                    
                    memcpy((unsigned char *)(dv_table_data) + ((xpos+(i*x))*sizeof(double)), &dbl_store, sizeof(double));
                    xpos++;
                }
                else {
                    // TODO copy from vector to davinci block
                    xpos += cur_fld->Size();
                }
            }
        }
        add_struct(dv_struct, "data", newVal(BSQ, x, y, 1, DOUBLE, dv_table_data));
    }
    else if (collective_type == 'i') {
        dv_table_data = calloc(x*y, sizeof(int));
        if (dv_table_data == NULL) {
            parse_error("calloc for int table failed in ISIS3 module get_data_for_table.\nMemory is probably low.");
            return;
        }
        for (int i=0; i<t.Records(); i++) {
            cur_rec = t[i];
            xpos = 0;
            for (int j=0; j<cur_rec.Fields(); j++) {
                cur_fld = &(cur_rec[j]);
                if (cur_fld->Size() == 1) {
                    int int_store = int(*cur_fld);
                    
                    memcpy((unsigned char *)(dv_table_data) + ((xpos+(i*x))*sizeof(int)), &int_store, sizeof(int));
                    xpos++;
                }
                else {
                    // TODO copy from vector to davinci block
                    xpos += cur_fld->Size();
                }
            }
        }
        add_struct(dv_struct, "data", newVal(BSQ, x, y, 1, INT, dv_table_data));
    }
    else if (collective_type == 'f') {
        dv_table_data = calloc(x*y, sizeof(float));
        if (dv_table_data == NULL) {
            parse_error("calloc for float table failed in ISIS3 module get_data_for_table.\nMemory is probably low.");
            return;
        }
        for (int i=0; i<t.Records(); i++) {
            cur_rec = t[i];
            xpos = 0;
            for (int j=0; j<cur_rec.Fields(); j++) {
                cur_fld = &(cur_rec[j]);
                if (cur_fld->Size() == 1) {
                    float float_store = float(*cur_fld);
                    
                    memcpy((unsigned char *)(dv_table_data) + ((xpos+(i*x))*sizeof(float)), &float_store, sizeof(float));
                    xpos++;
                }
                else {
                    // TODO copy from vector to davinci block
                    xpos += cur_fld->Size();
                }
            }
        }
        add_struct(dv_struct, "data", newVal(BSQ, x, y, 1, FLOAT, dv_table_data));
    }
    else if (collective_type == 'm') {
        add_struct(dv_struct, "data", newString(strdup("Mixed tables unimplemented.")));
    }
    else if (collective_type == 'c') {
        add_struct(dv_struct, "data", newString(strdup("String tables unimplemented.")));
    }
}


static void get_table(Isis::PvlObject o, Isis::Cube * cube, Var * dv_struct) {

    /* Get the table object referred to by PvlObject o from cube, make a davinci
       representation of it, and append it to dv_struct
    */

    Isis::Table * i3table = NULL;
    char namebuf[DV_NAMEBUF_MAX];
    Var * tbl_list;
    Var * tbl_struct;
    Var * kwds;

    if (find_struct(dv_struct, "TableList", &tbl_list) == -1) {
        tbl_list = new_struct(0);
        add_struct(dv_struct, "TableList", tbl_list);
    }

    for (int i=0; i<o.Keywords(); i++) {
        if (o[i].Name() == "Name") {
            i3table = new Isis::Table(KWD_TO_CHAR_PTR(o[i][0]));
            break;
        }
    }
    if (i3table != NULL) {
        cube->read(*i3table);
        tbl_struct = new_struct(0);
        if (o.Groups() > 0) {
            int fsize = 0, position = 0;
            for (int i=0; i<o.Groups(); i++) {
                
                for (int j=0; j<o.Group(i).Keywords(); j++) {
                    if (o.Group(i)[j].Name() == "Name") {
                        snprintf(namebuf, DV_NAMEBUF_MAX-1, "%s", KWD_TO_CHAR_PTR(o.Group(i)[j][0]));
                    }
                    if (o.Group(i)[j].Name() == "Size") {
                        fsize = atoi(KWD_TO_CHAR_PTR(o.Group(i)[j][0]));
                    }
                }
                namebuf[DV_NAMEBUF_MAX-1] = '\0';
                make_unique_name(namebuf, tbl_struct);
                kwds = new_struct(0);
                get_keywords(&(o.Group(i)), kwds, 1);
                position += fsize;
                add_struct(kwds, "data_column_start", newInt(position));
                add_struct(kwds, DV_ISIS_STRUCT_TYPE, newString(strdup("group")));
                add_struct(tbl_struct, namebuf, kwds);
            }
        }
        get_data_for_table(*i3table, tbl_struct);
        if (i3table->IsSampleAssociated()) {
            add_struct(tbl_struct, CUBE_ASSOC, newString(strdup(SAMPLE_STR)));
        }
        else if (i3table->IsLineAssociated()) {
            add_struct(tbl_struct, CUBE_ASSOC, newString(strdup(LINE_STR)));
        }
        else if (i3table->IsBandAssociated()) {
            add_struct(tbl_struct, CUBE_ASSOC, newString(strdup(BAND_STR)));
        }
        else {
            add_struct(tbl_struct, CUBE_ASSOC, newString(strdup(NONE_STR)));
        }
        add_struct(tbl_struct, DV_ISIS_STRUCT_TYPE, newString(strdup("table")));
        snprintf(namebuf, DV_NAMEBUF_MAX-1, "%s", i3table->Name().data());
        make_unique_name(namebuf, tbl_struct);
        add_struct(tbl_list, namebuf, tbl_struct);
    }
}



static void get_groups(Isis::Cube * cube, Var * dv_struct) {
    /* Extract the groups from a cube object */

    Isis::Pvl * label;
    Var * kwd_struct;
    char namebuf[DV_NAMEBUF_MAX];
    label = cube->getLabel();
    if (label->Groups() > 0) {
        for (int i=0; i<label->Groups(); i++) {
            //add_struct(names, NULL, newString(strdup((char *)label->Group(i).Name().data())));
            snprintf(namebuf, DV_NAMEBUF_MAX-1, "%s", (char *)label->Group(i).Name().data());
            namebuf[DV_NAMEBUF_MAX-1] = '\0';
            make_unique_name(namebuf, dv_struct);
             
            kwd_struct = new_struct(0);
            get_keywords(&(label->Group(i)), dv_struct, 0);
            add_struct(kwd_struct, DV_ISIS_STRUCT_TYPE, newString(strdup("group")));
            add_struct(dv_struct, namebuf, kwd_struct);
 
        }
    }
}

static void get_objects(Isis::Cube * cube, Var * dv_struct, int include_suffix) {
    /* get the root level objects from a cube */
    Isis::Pvl * label;
    Var * current_var;
    char namebuf[DV_NAMEBUF_MAX];
    label = cube->getLabel();
    if (label->Objects() > 0) {
        for (int i=0; i<label->Objects(); i++) {
            if (label->Object(i).Name() == "Table") {
                if (!include_suffix) continue;
                get_table(label->Object(i), cube, dv_struct);
            }
            else if (label->Object(i).Name() == "Label") {
                /* Skip Label, it's irrelevant to davinci, and will be
                   automatically added on write. */
            }
            else if (label->Object(i).Name() == "History") {
                /* history objects are associated with objects, and will be picked
                   up in the object load. */
            }
            else if (label->Object(i).Name() == "OriginalLabel") {
                /* Not preserving the original label for now.  Blob mechanics are
                   not conducive to doing this in davinci easily and its usefulness
                   is questionable at best. */
            }
            else {
                current_var = explode_object(label->Object(i));
                snprintf(namebuf, DV_NAMEBUF_MAX-1, "%s", (char *)label->Object(i).Name().data());
                namebuf[DV_NAMEBUF_MAX-1] = '\0';
                make_unique_name(namebuf, dv_struct);
                get_history(cube, (char *)label->Object(i).Name().data(), current_var);
                add_struct(current_var, DV_ISIS_STRUCT_TYPE, newString(strdup("object")));
                add_struct(dv_struct, namebuf, current_var);
            }
        }
    }
}

static Var *
read_isis3(FILE *fp, char *filename, int include_headers, int include_data, int include_suffix) {

    /* Top level reader code to open the ISIS cube using the API and return a 
       davinci structure representing it. */

    Isis::Cube * cube;
    Isis::Brick * brick;
    Var * rtnvar;
    void * dvdata, * isisdata;
    size_t b,l,x,y,z,size;
    int fmt;

    if (!check_ISIS_env()) return NULL;
    try {
        cube = new Isis::Cube;
        cube->open(filename);
    }
    catch (Isis::iException &e) {
        delete cube;
        return NULL;
    } 
    parse_error("Using ISIS3 API to read file '%s'.", filename);
    if (include_data) {
        x = (size_t)cube->getSampleCount();
        y = (size_t)cube->getLineCount();
        z = (size_t)cube->getBandCount();
        brick = new Isis::Brick(*cube, x, 1, 1);
        switch (cube->getPixelType()) {
          case (Isis::UnsignedByte):
            fmt = BYTE;
            size = sizeof(unsigned char);
            break;
          case (Isis::SignedWord):
            fmt = SHORT;
            size = sizeof(short);
          case (Isis::Real):
            fmt = FLOAT;
            size = sizeof(float);
            break;
          default:
            fmt = FLOAT;
            size = sizeof(float);
            break;
        }
        dvdata = malloc(size*x*y*z);
        brick->begin();
        for (b=0; b<z; b++) {
            for (l=0; l<y; l++) {
                cube->read(*brick);
                isisdata = (void *)brick->RawBuffer();
                memcpy((unsigned char *)(dvdata) + (((l*x) + (b*x*y))*size), isisdata, size * x);
                (*brick)++;
            }
        }
    }
    if (include_headers) {
        rtnvar = new_struct(0);
        if (include_data) add_struct(rtnvar, "cube", newVal(BSQ, x, y, z, fmt, dvdata));
        get_objects(cube, rtnvar, include_suffix);
//        if (objs != NULL) add_struct(rtnvar, "object", objs);
        get_groups(cube, rtnvar);
        get_keywords(cube->getLabel(), rtnvar, 0);
    }
    else {
        rtnvar = newVal(BSQ, x, y, z, fmt, dvdata);
    }
    cube->close();
    delete cube;
    if (include_data) delete brick;
    return rtnvar;

}


static Isis::iString dv_value_to_istring(Var * sdata, char * sname) {
    switch (V_TYPE(sdata)) {
      case ID_STRING:
        return Isis::iString(V_STRING(sdata));
      case ID_VAL:
        switch (V_FORMAT(sdata)) {
          case BYTE:
          case SHORT:
          case INT:
          case VAX_INTEGER:
          case INT64:
              return Isis::iString((int)extract_int(sdata, 0));
          case FLOAT:
          case VAX_FLOAT:
              return Isis::iString((double)extract_float(sdata, 0));
          case DOUBLE:
              return Isis::iString(extract_double(sdata, 0));
        }
    }
    parse_error("Unknown keyword translation for davinci value '%s'", sname);
    return Isis::iString("undefined");     
}

static Isis::PvlKeyword * dv_value_to_isis_keyword(Var * sdata, char * sname) {
    Isis::PvlKeyword * rtnobj = NULL;
    Var * ssdata;
    char * ssname;

    if ((V_TYPE(sdata) == ID_STRING) || (GetX(sdata)*GetY(sdata)*GetZ(sdata) == 1)) {
        rtnobj = new Isis::PvlKeyword(std::string(sname), dv_value_to_istring(sdata, sname));
    }
    else {
        if (GetY(sdata) + GetZ(sdata) > 2) {
            parse_error("value '%s' must be single dimensional in the X-dimension.");
            rtnobj = new Isis::PvlKeyword(std::string(sname), Isis::iString("undefined"));
        }
        else if (V_TYPE(sdata) == ID_STRUCT) {
            rtnobj = new Isis::PvlKeyword(std::string(sname));
            for (int i=0; i<get_struct_count(sdata); i++) {
                get_struct_element(sdata, i, &ssname, &ssdata);
                rtnobj->AddValue(dv_value_to_istring(ssdata, sname));
            }
        }
        else {
            rtnobj = new Isis::PvlKeyword(std::string(sname));
            switch (V_FORMAT(sdata)) {
              case BYTE:
              case SHORT:
              case INT:
              case VAX_INTEGER:
              case INT64:
                for (int i=0; i<GetX(sdata); i++)
                    rtnobj->AddValue(Isis::iString(extract_int(sdata, i)));
                break;
              case FLOAT:
              case VAX_FLOAT:
                for (int i=0; i<GetX(sdata); i++)
                    rtnobj->AddValue(Isis::iString(extract_float(sdata, i)));
                break;
              case DOUBLE:
                for (int i=0; i<GetX(sdata); i++)
                    rtnobj->AddValue(Isis::iString(extract_double(sdata, i)));
                break;
              default:
                rtnobj->AddValue(Isis::iString("undefined"));
                parse_error("Unknown davinci format %i for '%s' value", V_FORMAT(sdata), sname);
                break;
            }
        }
    }
    return rtnobj;
}

static Isis::PvlGroup * dv_struct_to_isis_group(Var * sdata, char * sname) {
    Isis::PvlGroup * rtnobj = NULL;
    Isis::PvlKeyword * i3_cur_kwd;
    Var * ssdata;
    char * ssname;
    rtnobj = new Isis::PvlGroup(std::string(sname));
    for (int i=0; i<get_struct_count(sdata); i++) {
        get_struct_element(sdata, i, &ssname, &ssdata);
        if (string_equal(ssname, DV_ISIS_STRUCT_TYPE)) continue;
        i3_cur_kwd = dv_value_to_isis_keyword(ssdata, ssname);
        if (i3_cur_kwd != NULL) rtnobj->AddKeyword(*i3_cur_kwd);
    }
    return rtnobj;
}


static void get_timestamp(char * buf) {
    time_t t;
    struct tm *ts;
    t = time(NULL);
    ts = gmtime(&t);
    strftime(buf, 128, TD_FORMAT, ts);
    return;
}

extern char *version;


static Var * getusername() {
    uid_t u;
    struct passwd *pw;
    u = geteuid();
    pw = getpwuid(u);
    if (pw) {
        return dvnewstr(pw->pw_name);
    }
    else {
        return dvnewstr("Unknown");
    }
}

static void addupd_davinci_isis3_history_entry(Isis::History * h, Var * v) {
    Var * hstr = NULL;
    char buf[128];

    find_struct(v, "davisis3", &hstr);
    if (hstr != NULL) { // we update by deleting the old davisis3 object
        return;  // do nothing for now, these functions need to be made C++  in davinci
        //p = (Var *)Narray_delete(V_STRUCT(v), "davisis3");
        //free_var(p);
    }
    hstr = new_struct(0);
    add_struct(v, "davisis3", hstr);
    add_struct(hstr, "DavinciVersion", dvnewstr(version+5));
    add_struct(hstr, "DavinciIsis3ModuleVersion", dvnewstr(DI3_VERSION));
    get_timestamp(buf);
    add_struct(hstr, "ExecutionDateTime", dvnewstr(buf));
    gethostname(buf, 128);
    add_struct(hstr, "HostName", dvnewstr(buf));
    add_struct(hstr, "UserName", getusername());
    add_struct(hstr, "Description", dvnewstr("ISIS3 cube written from davinci."));
}

// Circular isis_history/isis_object funtion reference require a prototype here
static Isis::PvlObject * dv_struct_to_isis_object(Var *, char *, Isis::Cube *);

static Isis::History * dv_struct_to_isis_history(Var * sdata, char * pname, Isis::Cube * cube) {
    Isis::History * rtnobj = NULL;
    Isis::PvlObject * cur_entry = NULL;
    Var * ssdata;
    char * ssname;
    rtnobj = new Isis::History(std::string(pname));
    addupd_davinci_isis3_history_entry(rtnobj, sdata);
    // I think history can only contain objects. If not, this code would need to
    // elaborated on a bit to handle groups and keyword pairs.  It will treat a group
    // like an object, and just ignore keywords. I think this because AddEntry can
    // add a null entry, or a PvlObject entry only.
    for (int i=0; i<get_struct_count(sdata); i++) {
        get_struct_element(sdata, i, &ssname, &ssdata);
        if (V_TYPE(ssdata) != ID_STRUCT) continue;
        cur_entry = dv_struct_to_isis_object(ssdata, ssname, cube);
        if (cur_entry != NULL) rtnobj->AddEntry(*cur_entry);
    }
    return rtnobj;
}


static Isis::PvlObject * dv_struct_to_isis_object(Var * sdata, char * sname, Isis::Cube * cube) {
    Isis::PvlObject * rtnobj = NULL;
    char * ssname;
    char * isis_struct_type;
    Var * ssdata = NULL;
    Var * sstype_var = NULL;
    Isis::PvlObject * cur_i3_obj;
    Isis::PvlGroup * cur_i3_grp;
    Isis::PvlKeyword * cur_i3_kwd;
    Isis::History * cur_i3_hist;

    rtnobj = new Isis::PvlObject(std::string(sname)); 
    for (int i=0; i<get_struct_count(sdata); i++) {
        get_struct_element(sdata, i, &ssname, &ssdata);
        if (string_equal(ssname, DV_ISIS_STRUCT_TYPE)) continue;
        if (string_equal(ssname, "Core")) continue; // Writing the core ourselves 
                                                    // will confuse things. Skip it.
        if (V_TYPE(ssdata) == ID_STRUCT) {
            find_struct(ssdata, DV_ISIS_STRUCT_TYPE, &sstype_var);
            if ((sstype_var == NULL) || V_TYPE(sstype_var) != ID_STRING) {
                isis_struct_type = (char*)"object";
            }
            else {
                isis_struct_type = V_STRING(sstype_var);
            }
            if (string_equal(isis_struct_type, "object")) {
                cur_i3_obj = dv_struct_to_isis_object(ssdata, ssname, cube);
                if (cur_i3_obj != NULL) {
                    if (string_equal(sname, "IsisCube")) {
                        cube->getLabel()->Object(0).AddObject(*cur_i3_obj);
                    }
                    else {
                        rtnobj->AddObject(*cur_i3_obj);
                    }
                }
            }
            else if (string_equal(isis_struct_type, "group")) {
                cur_i3_grp = dv_struct_to_isis_group(ssdata, ssname);
                if (cur_i3_grp != NULL) {
                    if (string_equal(sname, "IsisCube")) {
                        cube->getLabel()->Object(0).AddGroup(*cur_i3_grp);
                    }
                    else {
                    rtnobj->AddGroup(*cur_i3_grp);
                    }
                }
            }
            else if (string_equal(isis_struct_type, "history")) {
                cur_i3_hist = dv_struct_to_isis_history(ssdata, sname, cube);
                if (cur_i3_hist != NULL) cube->write(*cur_i3_hist);
            }
            else {
            parse_error("Undefined ISIS struct type '%s'. Skipping.", isis_struct_type);
            }
        }
        else {
            // keyword/value pair
            cur_i3_kwd = dv_value_to_isis_keyword(ssdata, ssname);
            if (cur_i3_kwd != NULL) {
                if (string_equal(sname, "IsisCube")) {
                    cube->getLabel()->Object(0).AddKeyword(*cur_i3_kwd);
                }
                else {
                    rtnobj->AddKeyword(*cur_i3_kwd);
                }
            }
        }
    }
    return rtnobj;
}

static void add_struct_member_to_cube(Var * sdata, char * sname, Isis::Cube * cube) {
    char * isis_struct_type;
    Var * stype_var = NULL;
    Isis::PvlObject * cur_i3_obj = NULL;
    Isis::PvlGroup * cur_i3_grp = NULL;
    Isis::History * cur_i3_hist = NULL;
    Isis::PvlKeyword * cur_i3_kwd = NULL;

    if (V_TYPE(sdata) == ID_STRUCT) {
        find_struct(sdata, DV_ISIS_STRUCT_TYPE, &stype_var);
        if ((stype_var == NULL) || V_TYPE(stype_var) != ID_STRING) {
            isis_struct_type = (char*)"object"; // punt to object, rather than abort. 
                                                // No reason to be a dick.
        }
        else {
            isis_struct_type = V_STRING(stype_var);
        }
        if (string_equal(isis_struct_type, "object")) {
            cur_i3_obj = dv_struct_to_isis_object(sdata, sname, cube);
            if (cur_i3_obj != NULL && cur_i3_obj->Name() != "IsisCube") cube->getLabel()->AddObject(*cur_i3_obj);
                   
        }
        else if (string_equal(isis_struct_type, "group")) {
            cur_i3_grp = dv_struct_to_isis_group(sdata, sname);
            if (cur_i3_grp != NULL) cube->putGroup(*cur_i3_grp);
        }
        else if (string_equal(isis_struct_type, "history")) {
            cur_i3_hist = dv_struct_to_isis_history(sdata, sname, cube);
            if (cur_i3_hist != NULL) cube->write(*cur_i3_hist);
        }
        else {
            parse_error("Undefined ISIS struct type '%s'", isis_struct_type);
            return;
        }
    }
    else {
        cur_i3_kwd = dv_value_to_isis_keyword(sdata, sname);
        if (cur_i3_kwd != NULL) cube->getLabel()->AddKeyword(*cur_i3_kwd);
    }
}

static Isis::TableField::Type get_isis_type_from_fdef(char * fname, Var * fdef) {
    Var * type_fld;
    find_struct(fdef, "Type", &type_fld);
    if (type_fld == NULL) {
       parse_error("Table field '%s' lacks type definition. Assuming double.", fname);
        return Isis::TableField::Double;
    }
    if (V_TYPE(type_fld) != ID_STRING) {
        parse_error("Table field '%s' type definition is not a string. Assuming double.", fname);
        return Isis::TableField::Double;
    }
    if (string_equal_ic(V_STRING(type_fld), "Double")) 
        return Isis::TableField::Double;
    if (string_equal_ic(V_STRING(type_fld), "Integer")) 
        return Isis::TableField::Integer;
    if (string_equal_ic(V_STRING(type_fld), "Text")) 
        return Isis::TableField::Text;
    if (string_equal_ic(V_STRING(type_fld), "Real")) 
        return Isis::TableField::Real;
    parse_error("Table field '%s' has unknown type '%s'. Assuming double.", fname, V_STRING(type_fld)); 
    return Isis::TableField::Double;
}

static int get_size_from_fdef(char * fname, Var * fdef) {
    Var * size_fld;
    find_struct(fdef, "Size", &size_fld);
    if (size_fld == NULL) {
        parse_error("Table field '%s' lacks size definition. Assuming 1.", fname);
        return 1;
    }
    if (V_TYPE(size_fld) != ID_VAL) {
        parse_error("Table field '%s' size definition cannot evaluate to integer. Assuming 1.", fname);
        return 1;
    }
    return V_INT(size_fld);
}


static void add_table_data(Isis::Table * t, Isis::TableRecord * r, Var * d, char * ty) {
    int x,y,z;

    if (d == NULL) return;
    x = GetX(d); y = GetY(d); z=GetZ(d);
    if (z > 1) {
        parse_error("ISIS3 tables should have band cardinality of 1. Extra bands ignored.");
        if (V_ORG(d) != BSQ) {
            parse_error("Additionally, table data not in BSQ format. Probably not what you want, but proceeding anyway.");
        }
    }
    for (int i=0; i<y; i++) {
        for(int j=0; j<x; j++) {
            size_t offset = (i*x)+j;
            if (string_equal(ty, "Double")) {
                double * dbval = ((double *)V_DATA(d))+offset;
                (*r)[j] = *dbval;
            }
            else if (string_equal(ty, "Integer")) {
                int * ival = ((int *)V_DATA(d))+offset;
                (*r)[j] = *ival;
            }
            else if (string_equal(ty, "Real")) {
                float * fval = ((float *)V_DATA(d))+offset;
                (*r)[j] = *fval;
            }
            else {
                parse_error("Data type %s not supported in ISIS3 tables.", ty);
            }

        }
        *t += *r; 
    }
}

static void add_table_to_cube(char * table_name, Var * table_struct, Isis::Cube * cube) {
    char * fname;
    Var *fdata, *data_val=NULL, *assoc_val=NULL, *type_val, *size_val, *column_val;
    int table_is_suspect = 0;
    Isis::Table * newtable;
    Isis::TableRecord * tbrec;
    Isis::TableField * tbfld;

    tbrec = new Isis::TableRecord();

    for(int i=0; i<get_struct_count(table_struct); i++) {
        get_struct_element(table_struct, i, &fname, &fdata);
        if (string_equal(fname, "data")) { // save the data value for use later
            data_val = fdata;
            continue;
        }
        if (string_equal(fname, CUBE_ASSOC)) {
            assoc_val = fdata;
            continue;
        }
        if (V_TYPE(fdata) != ID_STRUCT) continue; // non-structs are not fields
        type_val = NULL;
        size_val = NULL;
        find_struct(fdata, "Type", &type_val);
        if (type_val == NULL) {
            parse_error("Type is required for a table field '%s'. Value ignored.", fname);
            table_is_suspect = 1;
            continue;
        }
        find_struct(fdata, "Size", &size_val);
        if (size_val == NULL) {
            parse_error("Size is required for a table field '%s'. Value ignored.", fname);
            table_is_suspect = 1;
            continue;
        }
        find_struct(fdata, "data_column_start", &column_val); 
        //parse_error("Field '%s': type: %s size: %i c: %i  ", fname, V_STRING(type_val), 
        //            V_INT(size_val), V_INT(column_val));
        tbfld = new Isis::TableField(fname, get_isis_type_from_fdef(fname, fdata), 
                                     get_size_from_fdef(fname, fdata));
        *tbrec += *tbfld;
    }
    newtable = new Isis::Table(table_name,*tbrec);
    if (assoc_val != NULL) {
        if (string_equal(V_STRING(assoc_val), SAMPLE_STR)) {
            newtable->SetAssociation(Isis::Table::Samples);
        }
        else if (string_equal(V_STRING(assoc_val), LINE_STR)) {
            newtable->SetAssociation(Isis::Table::Lines);
        }
        else if (string_equal(V_STRING(assoc_val), BAND_STR)) {
            newtable->SetAssociation(Isis::Table::Bands);
        }
        else if (string_equal(V_STRING(assoc_val), NONE_STR)) {
            newtable->SetAssociation(Isis::Table::None);
        }
        else {
            parse_error("Unknown cube association value '%s' for table '%s'. Assuming None.", V_STRING(assoc_val), table_name);
        }
    }    
    add_table_data(newtable, tbrec, data_val, V_STRING(type_val));
    cube->write(*newtable);
    if (table_is_suspect) {
        parse_error("table '%s' written, but errors may be present.", table_name);
    }

}


static void add_tables_to_cube(Var * dv_obj, Isis::Cube * cube) {
    int i = 0;
    char * sname;
    Var * sdata;

    for(i=0; i<get_struct_count(dv_obj);i++) {
        get_struct_element(dv_obj, i, &sname, &sdata);
        //parse_error("Adding table '%s' to cube.", sname);
        add_table_to_cube(sname, sdata, cube);   
    }
}

static void add_isis3_headers(Var * dv_obj, Isis::Cube * cube) {
    int i = 0;
    char * sname;
    Var * sdata;

    if (V_TYPE(dv_obj) != ID_STRUCT) {
        parse_error("Can only create ISIS3 headers for structs.");
        return;
    }
    for (i=0; i<get_struct_count(dv_obj);i++) {
        get_struct_element(dv_obj, i, &sname, &sdata);
        if (string_equal(sname, "cube")) continue; // skip the 'cube' member
        if (string_equal(sname, "TableList")) {
            add_tables_to_cube(sdata, cube);
            continue;
        }
        add_struct_member_to_cube(sdata, sname, cube); 
    }
}

static size_t full_write(int fd, const void * buf, size_t count) {
    // Ensures a write completes, hangs, or returns an error.
    // Will throw error warning messages and go semi-quiescent
    // if a lot of short writes occur.

    // This may seem strange, but we've had issues at MSFF where
    // network disks under heavy load return short writes. I wanted
    // to make a buffering write that would be polite in these
    // circumstances and yield to other less aware (read: greedy)
    // products.

    ptrdiff_t remaining = count;
    ptrdiff_t write_result;
    int short_write_count = 0;
    unsigned char * rbuf = (unsigned char *)buf;

    while (remaining > 0) {
        write_result = write(fd, rbuf, remaining);
        if (write_result == -1) return -1;
        if (write_result < remaining) {
            if (++short_write_count % 100 == 0) {
                parse_error("%i short writes in one full_write() call. Throttling.", short_write_count);
                sleep(5);
            }
        }
        remaining -= write_result;
        rbuf += write_result;
    }
    return count; 
}

static void status_update(int total_steps, int current_step) {
    if (!isatty(2)) return;
    fprintf(stderr, "Finishing: %0.2f%%\r", ((float)current_step/((float)total_steps-1.0))*100.0);
}

static int convert_dv_to_isis3(Var * dv_obj, char * fn) {
    Var * cube_data = NULL;
    Isis::Cube * cube = NULL;
    int rtnval = 0, descend = 0, core_start, fd;
    int i,j,k,x,y,z;
    size_t dsize;
    unsigned char * line_buffer;
    if (V_TYPE(dv_obj) == ID_STRUCT) {
        // We're doing a more complete ISIS 3 cube, which means we have to conform a bit
        // to a standard structure. If it has a cube member, that should be an array that
        // we can use Isis::Bricks to write in. Other members will be structures with
        // isis_struct_type as a member to identify the type of object it is.
        descend = 1;
        find_struct(dv_obj, "cube", &cube_data);    
    }
    else if (V_TYPE(dv_obj) == ID_VAL) {
        // This is a stupid simple ISIS3 cube, and will do the minimum required to make a 
        // legit ISIS 3 cube. It's only a few headers away from being a raw file.
        cube_data = dv_obj;
    }
    else {
        parse_error("Cannot save this object as an ISIS3 cube.");
        rtnval = 1;
    }
    if (rtnval == 0) {
        // If we actually have a cube thing:
        if (cube_data != NULL) {
	    x = GetX(cube_data);
	    y = GetY(cube_data);
	    z = GetZ(cube_data);
            cube = new Isis::Cube();
            cube->setLabelsAttached(1);
            cube->setFormat(Isis::Cube::Bsq);
            cube->setDimensions(x,y,z);
            
            switch (V_FORMAT(cube_data)) {
              case BYTE:
                cube->setPixelType(Isis::UnsignedByte);
                dsize = sizeof(unsigned char);
                break;
              case SHORT:
                cube->setPixelType(Isis::SignedWord);
                dsize = sizeof(short int);
                break;
              case FLOAT:
                cube->setPixelType(Isis::Real);
                dsize = sizeof(float);
                break;
              default:
                parse_error("cube data for ISIS3 can only be byte, short, or float. Sorry.");
                rtnval = 1;
                goto error_exit; // Eeek! A GOTO! Run for your lives!
            }
            cube->create(std::string(fn));
            if (descend) {
                add_isis3_headers(dv_obj, cube);
            }
        }
        else {
            // A cube without a cube. Interesting. I think we can handle it.
            cube->create(std::string(fn));
            if (descend) {
                add_isis3_headers(dv_obj, cube);
            }
        } // if (cube_data != NULL)

        // Get the data start offset from the file
        core_start = ((int)cube->getLabel()->Object(0).Object(0)[0])-1;
        cube->close();
        if (cube != NULL) delete cube; // remove the no longer needed cube object
        cube = NULL;
    }

    // The ISIS3 API for writing data outside of the ISIS system is irretrievably
    // broken. But we have the data in memory, and the filename, and an offset (in
    // core_start) for where the data should be. We can open the file and write the
    // data ourselves. Everything else is done at this point in time.

    // Note that ISIS 3 only supports BSQ and Tiled data, so we will always write
    // out a BSQ format cube. It may be neat to handle tiled data here in the
    // future, but really if someone needs a tiled cube, they should just run
    // it through the ISIS 3 toolset and convert it.

    if (cube_data != NULL) {
        fd = open(fn, O_WRONLY);
        size_t offset;
        lseek(fd, core_start, SEEK_SET);
        switch (V_ORG(cube_data)) {
          case BSQ:
            for (i=0; i<y*z; i++) {
                offset = (x*i*dsize);
                full_write(fd, (unsigned char *)V_DATA(cube_data)+offset, x*dsize);
                status_update(y*z, i);
            }
            break;
          case BIL:
            for (k=0; k<z; k++) {
                for (j=0; j<y; j++) {
                    offset = (k+j*z)*(x*dsize);
                    full_write(fd, 
                          (unsigned char *)V_DATA(cube_data)+offset, 
                          x*dsize);
                    status_update(y*z, k*y + j);
                }
            }
            break;
          case BIP:
            if ((line_buffer = (unsigned char *)malloc(x*dsize)) == NULL) {
                parse_error("line_buffer malloc failed.");
                goto error_exit;
            }
            for (k=0; k<z; k++) {
                for (j=0; j<y; j++) {
                    for (i=0; i<x; i++) {
                        offset = ((j*x*z)+(i*z)+k)*dsize;
                        memcpy(line_buffer+(i*dsize), 
                               (unsigned char *)V_DATA(cube_data)+offset, 
                               dsize);
                    }
                    full_write(fd, line_buffer, x*dsize);
                    status_update(y*z, k*y + j);
                }
            }
            free(line_buffer);
            break;
        }
        parse_error("Closing file...              ");
        if (close(fd) == -1) {
            parse_error("Close of file failed. errno=%i", errno);
        } 
    }

  error_exit:
    return rtnval;
}

extern "C" Var *
dv_modread(FILE * fp, char *fname) {
    /* Shim to access read_isis3 for the load/read davinci functions
    */
    return read_isis3(fp, fname, 0, 1, 0);
}

extern "C" Var *
dv_modload_pds(FILE * fp, char *fname, int data, int suffix) {
    /* Shim to access read_isis for the load_pds function
    */
    return read_isis3(fp, fname, 1, data, suffix);
}

extern "C" int
dv_modwrite(Var * output, char * filetype, FILE * fh, char * fn) {
    char * alt_fn = NULL, * subptr = NULL;
    int rtncode, free_it = 0;
    // ISIS thinks it is our mommy and daddy and knows best how to name
    // our files. I pass the read function a file with .cub extension
    // added if we don't already have one. Then on return, move the .cub file 
    // to what the user really requested if it was not a .cub file.

    parse_error("Using ISIS3 API to write file '%s'.", fn);
    if (strlen(fn) > 4) {
        subptr = fn + strlen(fn) - 4;
        if (string_equal(subptr, ".cub")) alt_fn = fn;
    }
    if (alt_fn == NULL) {
        alt_fn = (char *)malloc(strlen(fn)+5);
        sprintf(alt_fn, "%s.cub", fn);
        free_it = 1;
    }
    rtncode = convert_dv_to_isis3(output, alt_fn);
    if (rtncode == 0 && !string_equal(fn, alt_fn)) {
        if (rename(alt_fn, fn) == -1) {
            parse_error("Write finished, but rename from %s to %s failed (errno = %i).", alt_fn, fn, errno);
            rtncode = 0;
        }
    }
    if (free_it) free(alt_fn);
    return rtncode;
}

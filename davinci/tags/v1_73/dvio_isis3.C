#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_ISIS3
#include <strings.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <regex.h>
#include "parser.h"
#include "dvio.h"
/* ISIS3 API includes */
#include "Cube.h"
#include "Brick.h"
#include "iException.h"


static int check_ISIS_env() {
    /* If your ISISROOT is not set properly, the ISIS3 API fails HARD with a
       SIGSEGV.  This check should be run before any ISIS3 Object is instantiated,
       or the program will crash if the environment is incorrect.  Also on this note,
       ALWAYS declare pointers to ISIS3 objects and instantiate them with new.  If
       you do not, they will instantiate when davinci starts and trigger the SEGV
       before you even get started.
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
        parse_error("Warning: ISIS Environment not setup correctly to use ISIS3 API.\nPlease set your ISISROOT environment variable to the root of an ISIS3 install.");
    }
    return setup_correct;
}

#define KWD_TO_CHAR_PTR(x) strdup((char *)((x).ToQt().toAscii().data()))

static void explode_keyword(Isis::PvlKeyword kwd, Var * names, 
                             Var * values) {
    /* Take an ISIS keyword, append its name to the davinci structure pointed
       at in names, and append its value to the davinci structure pointed at
       by values.  If the ISIS keyword contains a single value, the value
       appended is a a string.  If the ISIS keyword contains multiple values,
       the value appended is a structure of strings, each member containing
       one value.  Function returns nothing, but the Var structures pointed
       at by names and values are altered.
    */
    Var * varlist;
    add_struct(names, NULL, newString(strdup((char *)(kwd.Name().data()))));
    if (kwd.Size() == 1) {
        add_struct(values, NULL, newString(KWD_TO_CHAR_PTR(kwd[0])));
    }
    else {
        varlist = new_struct(kwd.Size());
        for (int j=0; j<kwd.Size(); j++) {
        add_struct(varlist, NULL, newString(KWD_TO_CHAR_PTR(kwd[j])));
        }
        add_struct(values, NULL, varlist);
    }
}

static Var * get_keywords(Isis::PvlContainer * pvl) {
    /* Extract keywords from any pvl instance */
    Var * rtn_value;
    Var * names;
    Var * values;
    rtn_value = NULL;
    if (pvl->Keywords() > 0) {
        rtn_value = new_struct(2);
        names = new_struct(pvl->Keywords());
        values = new_struct(pvl->Keywords());
        for (int i=0; i<pvl->Keywords(); i++)
            explode_keyword((*pvl)[i], names, values);
        add_struct(rtn_value, "name", names);
        add_struct(rtn_value, "value", values);
    }
    return rtn_value;
}

static Var * explode_object(Isis::PvlObject o) {
    Var * rtn_value;
    Var * objects;
    Var * obj_names;
    Var * obj_structures;
    Var * groups;
    Var * group_names;
    Var * group_structures;
    Var * kwds;
    rtn_value = new_struct(3);
    if (o.Objects() > 0) {
        objects = new_struct(2);
        obj_names = new_struct(o.Objects());
        obj_structures = new_struct(o.Objects());
        for (int i=0; i<o.Objects(); i++) {
            add_struct(obj_names, NULL, newString(strdup((char *)o.Object(i).Name().data())));
            add_struct(obj_structures, NULL, explode_object(o.Object(i)));
        }
        add_struct(objects, "name", obj_names);
        add_struct(objects, "structure", obj_structures);
        add_struct(rtn_value, "object", objects);
    }
    if (o.Groups() > 0) {
        groups = new_struct(2);
        group_names = new_struct(o.Groups());
        group_structures = new_struct(o.Groups());
        for (int i=0; i<o.Groups(); i++) {
            add_struct(group_names, NULL, newString(strdup((char *)o.Group(i).Name().data())));
            add_struct(group_structures, NULL, get_keywords(&(o.Group(i))));
        }
        add_struct(groups, "name", group_names);
        add_struct(groups, "keyword", group_structures);
        add_struct(rtn_value, "group", groups); 
    }
    if ((kwds = get_keywords(&o)) != NULL) {
        add_struct(rtn_value, "keyword", kwds);
    }    
    return rtn_value;
}

static Var * get_keywords_from_cube(Isis::Cube * cube) {
    /* Extract the keywords from a cube object */
    Var * rtn_value = NULL;
    Isis::Pvl * label;
    Var * names;
    Var * values;
    Var * varlist;
    Isis::PvlKeyword  * current_kwd;
    label = cube->Label();
    rtn_value = new_struct(2);
    names = new_struct(label->Keywords());
    values = new_struct(label->Keywords());
    for (int i=0; i<label->Keywords(); i++) 
        explode_keyword((*label)[i], names, values);
    add_struct(rtn_value, "name", names);
    add_struct(rtn_value, "value", values);
    return rtn_value;
}


static Var * get_groups(Isis::Cube * cube) {
    /* Extract the groups from a cube object */
    Var * rtn_value = NULL;
    Isis::Pvl * label;
    Var * names;
    Var * structures;
    Var * current_var;
    label = cube->Label();
    if (label->Groups() > 0) {
        rtn_value = new_struct(2);
        names = new_struct(label->Groups());
        structures = new_struct(label->Groups());
        for (int i=0; i<label->Groups(); i++) {
            add_struct(names, NULL, newString(strdup((char *)label->Group(i).Name().data())));
            add_struct(structures, NULL, get_keywords(&(label->Group(i))));
        }
        add_struct(rtn_value, "name", names);
        add_struct(rtn_value, "keyword", structures);
    }
    return rtn_value;
}

static Var * get_objects(Isis::Cube * cube) {
    /* get the root level objects from a cube */
    Isis::Pvl * label;
    Var * current_var;
    Var * rtn_value = NULL;
    Var * names;
    Var * structures;
    char namebuf[80];
    label = cube->Label();
    if (label->Objects() > 0) {
        rtn_value = new_struct(2);
        names = new_struct(label->Objects());
        structures = new_struct(label->Objects());
    
        for (int i=0; i<label->Objects(); i++) {
            current_var = explode_object(label->Object(i));
            add_struct(names, NULL, newString(strdup((char*)label->Object(i).Name().data())));
            add_struct(structures, NULL, current_var);
        }
        add_struct(rtn_value, "name", names);
        add_struct(rtn_value, "structure", structures);
    }
    return rtn_value;
}


extern "C" Var *
dv_LoadISIS3(FILE *fp, char *filename, struct iom_iheader *sub) {
    Isis::Cube * cube;
    Isis::Brick * brick;
    Var * rtnvar;
    void * dvdata, * isisdata;
    int x,y,z;
    int size, fmt;
    int s,l,b;

    if (!check_ISIS_env()) return NULL;
    try {
        cube = new Isis::Cube;
        cube->Open(filename);
    }
    catch (Isis::iException &e) {
        delete cube;
        return NULL;
    } 
    parse_error("Using ISIS3 API to read file %s.", filename);
    x = cube->Samples();
    y = cube->Lines();
    z = cube->Bands();
    brick = new Isis::Brick(*cube, x, 1, 1);
    switch (cube->PixelType()) {
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
            cube->Read(*brick);
            isisdata = (void *)brick->RawBuffer();
            bcopy(isisdata, (unsigned char *)(dvdata) + (((l*x) + (b*x*y))*size), size * x);
            (*brick)++;
        }
    }
    rtnvar = new_struct(4);
    Var * objs = get_objects(cube);
    if (objs != NULL) add_struct(rtnvar, "object", objs);
    Var * grps = get_groups(cube);
    if (grps != NULL) add_struct(rtnvar, "group", grps);
    Var * kwds = get_keywords(cube->Label());
    if (kwds != NULL) add_struct(rtnvar, "keyword", kwds);
    add_struct(rtnvar, "cube", newVal(BSQ, x, y, z, fmt, dvdata));
    cube->Close();
    delete cube;
    delete brick;
    return rtnvar;

}



extern "C" int 
iom_isISIS3(char * fn) {
    Isis::Cube * cube;
    if (!check_ISIS_env()) return 0;
    try {
        cube = new Isis::Cube;
        cube->Open(fn);
    }
    catch (Isis::iException &e) {
        return 0;
    }
    /* ISIS 3 could parse it... probably okay, then. */
    cube->Close();
    delete cube;
    return 1;
}

#endif /* HAVE_ISIS3 */


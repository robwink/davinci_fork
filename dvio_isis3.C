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
    }
    if (setup_correct == 0) {
        parse_error("Warning: ISIS Environment not setup correctly to use ISIS3 API.\nPlease set your ISISROOT environment variable to the root of an ISIS3 install.");
    }
    return setup_correct;
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
        parse_error("Nope.\n");
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
    cube->Close();
    delete cube;
    delete brick;
    rtnvar = newVal(BSQ, x, y, z, fmt, dvdata);
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


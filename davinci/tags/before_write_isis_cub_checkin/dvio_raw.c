#include "parser.h"
#include "dvio.h"

int
dv_WriteRaw(
    Var *ob,
    char *filename,
    int force
    )
{
    struct iom_iheader h;
    int status;

    var2iom_iheader(ob, &h);
    status = iom_WriteRaw(filename, V_DATA(ob), &h, force);
    iom_cleanup_iheader(&h);

    if (status == 0){
        parse_error("Failed to write RAW file %s.", filename);
        return 0;
    }

    return 1;
}


/*
** dv_ReadRaw() should go in here
*/

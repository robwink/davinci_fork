#include "parser.h"
#include "dvio.h"

/**
 ** Write data as ER Mapper ERS data set
 **/


int
dv_WriteERS(Var *s, char *filename, int force)
{
    struct iom_iheader h;
    int status;
    
    if (V_TYPE(s) != ID_VAL) {
        sprintf(error_buf, "Var is not a value: %s", V_NAME(s));
        parse_error(NULL);
        return 0;
    }

	if (GetBands(V_SIZE(s), V_ORG(s)) > 1 && V_ORG(s) != BIL) {
        parse_error("ERS files must be BIL format");
        return 0;
    }

    
    var2iom_iheader(s, &h);
    status = iom_WriteERS(filename, V_DATA(s), &h, force);
    iom_cleanup_iheader(&h);
    
    if (status == 0){
        parse_error("Failed writing ERS file %s.\n", filename);
        return 0;
    }
    
    return 1;
}

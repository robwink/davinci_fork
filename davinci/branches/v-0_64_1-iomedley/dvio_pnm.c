#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "iomedley.h"

Var *
dv_LoadPNM(
    FILE *fp,
    char *filename,
    struct iom_iheader *s
    )
{
    int x,y,z;
    int i;
    u_char *data;
    int bits;
    struct iom_iheader h;
    int status;

    Var *v;

    
    /*
    ** NOTE: We completely ignore the _iheader structure here.
    */


    /**
    *** Get format
    **/

    /*
    ** This reads and attaches the data to h->data
    */
    if (!(status = iom_GetPNMHeader(fp, filename, &h))){
        return NULL;
    }
	x = h.size[0];
	y = h.size[1];
	z = h.size[2];

    data = iom_detach_iheader_data(&h);
    
    if (data == NULL){
        iom_cleanup_iheader(&h);
        return(NULL);
    }

    v = newVar();
    V_TYPE(v) = ID_VAL;
    V_DATA(v) = data;
    if (z == 1) {
        V_SIZE(v)[0] = x;
        V_SIZE(v)[1] = y;
        V_SIZE(v)[2] = z;
        V_ORG(v) = BSQ;
    } else {
        V_SIZE(v)[0] = z;
        V_SIZE(v)[1] = x;
        V_SIZE(v)[2] = y;
        V_ORG(v) = BIP;
    }
    V_DSIZE(v) = x*y*z;
    if (bits <= 8) {
        V_FORMAT(v) = BYTE;
    } else if (bits == 16) {
		int *data = (int *)calloc(4, x*y*z);
		unsigned short *sdata = (unsigned short *)V_DATA(v);

		for (i = 0 ; i < V_DSIZE(v) ; i++) {
			data[i] = sdata[i];
		}
		free(sdata);
		V_DATA(v) = data;
        V_FORMAT(v) = INT;
		parse_error("Warning: 16 bit PGM file being promoted to 32-bit int.");
	}

    return(v);
}

void
dv_WritePGM(Var *obj, FILE *fp, char *filename)
{
    struct iom_iheader h;

    if (V_TYPE(obj) != ID_VAL || V_FORMAT(obj) != BYTE) {
        sprintf(error_buf, "Data for PGM file must be byte()");
        parse_error(NULL);
        return;
    }

    iom_var2iheader(obj, &h);

    if (GetBands(V_SIZE(obj), V_ORG(obj)) != 1) {
        sprintf(error_buf, "Data for PGM file must have depth=1");
        parse_error(NULL);
        return;
    }

    if (fp == NULL) {
        fp = fopen(filename, "w");
    }
    if (fp == NULL) return;

    if (VERBOSE > 1)  {
        fprintf(stderr, "Writing %s: %dx%d PGM file.\n",
                filename, h.dim[0], h.dim[1]);
    }   

    iom_WritePNM(fp, filename, V_DATA(obj), &h);
    fclose(fp);
    
    return;
}

void
dv_WritePPM(Var *obj, FILE *fp, char *filename)
{
    struct iom_iheader h;

    if (V_TYPE(obj) != ID_VAL || V_FORMAT(obj) != BYTE) {
        sprintf(error_buf, "Data for PPM output must be byte()");
        parse_error(NULL);
        return;
    }

    iom_var2iheader(obj, &h);

    if (GetBands(V_SIZE(obj), V_ORG(obj)) != 3) {
        sprintf(error_buf, "Data for PPM output must have depth=3");
        parse_error(NULL);
        return;
    }

    if (V_ORG(obj) != BIP) {
        sprintf(error_buf, "Data for PPM output must be in BIP format");
        parse_error(NULL);
        return;
    }

    if (fp == NULL) {
        fp = fopen(filename, "w");
    }
    if (fp == NULL) return;

    if (VERBOSE > 1)  {
        fprintf(stderr, "Writing %s: %dx%d PPM file.\n",
                filename, h.dim[0], h.dim[1]);
    }   

    iom_WritePNM(fp, filename, V_DATA(obj), &h);
    fclose(fp);
    
    return;
}

#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "dvio.h"

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

    
    if (iom_isPNM(fp) == 0){ return NULL; }

    if (!(status = iom_GetPNMHeader(fp, filename, &h))){
        return NULL;
    }

    if (s != NULL) {
        /** 
         ** Set subsets
         **/
        iom_MergeHeaderAndSlice(&h, s);
    }

    data = iom_read_qube_data(fileno(fp), &h);
    if (data){
        v = iom_iheader2var(&h);
        V_DATA(v) = data;
    }
    else {
        v = NULL;
    }

    iom_cleanup_iheader(&h);


#if 0    
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
#endif

    return(v);
}

int
dv_WritePGM(Var *obj, char *filename, int force)
{
    struct iom_iheader h;
    int status;

    if (V_TYPE(obj) != ID_VAL || V_FORMAT(obj) != BYTE) {
        sprintf(error_buf, "Data for PGM file must be byte()");
        parse_error(NULL);
        return 0;
    }

    if (GetBands(V_SIZE(obj), V_ORG(obj)) != 1) {
        sprintf(error_buf, "Data for PGM file must have depth=1");
        parse_error(NULL);
        return 0;
    }

    if (VERBOSE > 1)  {
        fprintf(stderr, "Writing %s: %dx%d PGM file.\n",
                filename, h.size[0], h.size[1]);
    }   

    var2iom_iheader(obj, &h);
    status = iom_WritePNM(filename, V_DATA(obj), &h, force);
    iom_cleanup_iheader(&h);
    
    if (status == 0){
        parse_error("Writing of PGM file %s failed.\n", filename);
        return 0;
    }
    
    return 1;
}

int
dv_WritePPM(Var *obj, char *filename, int force)
{
    struct iom_iheader h;
    int status;

    if (V_TYPE(obj) != ID_VAL || V_FORMAT(obj) != BYTE) {
        sprintf(error_buf, "Data for PPM output must be byte()");
        parse_error(NULL);
        return 0;
    }

    if (GetBands(V_SIZE(obj), V_ORG(obj)) != 3) {
        sprintf(error_buf, "Data for PPM output must have depth=3");
        parse_error(NULL);
        return 0;
    }

    if (V_ORG(obj) != BIP) {
        sprintf(error_buf, "Data for PPM output must be in BIP format");
        parse_error(NULL);
        return 0;
    }

    if (VERBOSE > 1)  {
        fprintf(stderr, "Writing %s: %dx%dx%d PPM file.\n",
                filename, h.dim[0], h.dim[1], h.dim[2]);
    }   

    var2iom_iheader(obj, &h);
    status = iom_WritePNM(filename, V_DATA(obj), &h, force);
    iom_cleanup_iheader(&h);
    
    if (status == 0){
        parse_error("Writing of PPM file %s fialed.\n", filename);
        return 0;
    }
    
    return 1;
}

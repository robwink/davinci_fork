#include "parser.h"

int
get_int(FILE *fp)
{
    int i;
    char c;

    while((c = getc(fp)) != EOF) {
        if (c == '#') {
            while(c != '\n')
                c = getc(fp);
        }
        if (c >= '0' && c <= '9') {
            i = 0;
            while (c >= '0' && c <= '9') {
                i = i*10 + (c - '0');
                c = getc(fp);
            }
            return(i);
        }
    }
    return(-1);
}

int
getbit(FILE *fp)
{
    char c;

    while((c = getc(fp)) != EOF) {
        if (c == '#') {
            while(c != '\n') 
                c = getc(fp);
        }
        if (c == '0' || c == '1') {
            return(c-'0');
        }
    }
    return(-1);
}

char *
is_PNM(FILE *fp, char *fstr)
{
    char id,format;
    rewind(fp);
    id = fgetc(fp);
    format = fgetc(fp);

    if (id == 'P') {
        if (format == '1' || format == '4') strcpy(fstr, "pbm");
        if (format == '2' || format == '5') strcpy(fstr, "pgm");
        if (format == '3' || format == '6') strcpy(fstr, "ppm");
        return(fstr);
    }
    return(NULL);
}


int
ReadPNM(FILE *fp, char *filename, int *xout, int *yout, 
        int *zout, int *bits, void **dout)
{
    char id,format;
    int x,y,z,count;
    int i,j,k,d;
    int bitshift;
    int maxval;
    unsigned char *data;

    /**
    *** Get format
    **/
    rewind(fp);

    id = fgetc(fp);
    format = fgetc(fp);

    if (id != 'P') {
        return(0);
    }
    if (format < '1' || format > '6') {
        return(0);
    }
    x = get_int(fp);
    y = get_int(fp);
    z = 1;

    count = 0;
    switch (format) {
    case '1':                 /* plain pbm format */
        data = (unsigned char *)calloc(1, x*y);
        for (i = 0 ; i < y ; i++) {
            for (j = 0 ; j < x ; j++) {
                data[count++] = getbit(fp);
            }
        }
        *bits = 1;
        break;
    case '2':                 /* plain pgm format */
        maxval = get_int(fp);
        if (maxval == 255) {
            data = (unsigned char *)calloc(1, x*y);
            k = x*y;
            for (i = 0 ; i < k ; i++) {
                data[count++] = get_int(fp);
            }
            *bits = 8;
        } else if (maxval == 65535) {
            unsigned short *sdata = (unsigned short *)calloc(2, x*y);
            k = x*y;
            for (i = 0 ; i < k ; i++) {
                sdata[count++] = get_int(fp);
            }
            data = (u_char *)sdata;
            *bits = 16;
        } else {
            parse_error("Unable to read this pgm file.  Odd maxval");
            return(0);
        }
        break;
    case '3':                 /* plain ppm format */
        maxval = get_int(fp);
        data = (unsigned char *)calloc(3, x*y);
        for (i = 0 ; i < y ; i++) {
            for (j = 0 ; j < x ; j++) {
                data[count++] = get_int(fp);
                data[count++] = get_int(fp);
                data[count++] = get_int(fp);
            }
        }
        z=3;
        *bits = 8;
        break;
    case '4':                 /* raw pbm format */
        /**
        *** this code is roughly identical to the libpbm code.
        *** It does not appear to be correctly compatable with
        *** the code to convert raw to plain. Oh well.
        **/
        data = (unsigned char *)calloc(1, x*y);
        for (i = 0 ; i < y ; i++) {
            bitshift = -1;
            for (j = 0 ; j < x ; j++) {
                if (bitshift == -1) {
                    d = fgetc(fp);
                    bitshift = 7;
                }
                data[count++] = ((d >> bitshift) & 1);
                bitshift--;
            }
        }
        *bits = 1;
        break;
    case '5':                 /* raw pgm format */
        maxval = get_int(fp);
        if (maxval == 255) {
            data = (unsigned char *)calloc(1, x*y);
            fread(data, 1, x*y, fp);
            *bits = 8;
        } else if (maxval == 65535) {
            data = (unsigned char *)calloc(2, x*y);
            fread(data, 2, x*y, fp);
            *bits = 16;
        } else {
			parse_error("Unable to read this pgm file.  Odd maxval");
            return(0);
        }
        break;
    case '6':                 /* raw ppm format */
        maxval = get_int(fp);
        if (maxval > 255) {
            parse_error("Unable to read ppm files with maxval > 255");
            return(0);
        }
        data = (unsigned char *)calloc(3, x*y);
        fread(data, 3, x*y, fp);
        z=3;
        *bits = 8;
        break;
    default:
        return(0);
    }
    fclose(fp);

    *xout = x;
    *yout = y;
    *zout = z;
    *dout = data;

    return(1);
}

int
ReadPNMHeader(FILE *fp, char *filename, int *xout, int *yout, 
              int *zout, int *bits)
{
    char id,format;
    int x,y,z,count;
    int bitshift;
    int maxval;
    u_char *data;

    /**
    *** Get format
    **/
    rewind(fp);

    id = fgetc(fp);
    format = fgetc(fp);

    if (id != 'P') {
        return(0);
    }
    if (format < '1' || format > '6') {
        return(0);
    }
    x = get_int(fp);
    y = get_int(fp);
    z = 1;

    count = 0;
    switch (format) {
    case '1':                 /* plain pbm format */
        *bits = 1;
        break;
    case '2':                 /* plain pgm format */
        maxval = get_int(fp);
        break;
    case '3':                 /* plain ppm format */
        maxval = get_int(fp);
        z=3;
        *bits = 8;
        break;
    case '4':                 /* raw pbm format */
        *bits = 1;
        break;
    case '5':                 /* raw pgm format */
        maxval = get_int(fp);
        *bits = 8;
        break;
    case '6':                 /* raw ppm format */
        maxval = get_int(fp);
        z=3;
        *bits = 8;
        break;
    default:
        return(0);
    }
    fclose(fp);

    *xout = x;
    *yout = y;
    *zout = z;

    return(1);
}


Var *
LoadPNM(FILE *fp, char *filename, struct _iheader *s)
{
    int x,y,z;
    int i;
    u_char *data;
    int bits;

    Var *v;

    /**
    *** Get format
    **/
    if (ReadPNM(fp, filename, &x, &y, &z, &bits, (void **)&data) != 1) {
        return(NULL);
    }

    v = new(Var);
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
    if (bits == 8) {
        V_FORMAT(v) = BYTE;
    } else if (bits == 16) {
		int *data = (int *)calloc(4, x*y*z);
		u_short *sdata = (u_short *)V_DATA(v);

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
WritePGM(Var *obj, FILE *fp, char *filename)
{
    int x,y,z;

    if (V_TYPE(obj) != ID_VAL || V_FORMAT(obj) != BYTE) {
        sprintf(error_buf, "Data for PGM file must be byte()");
        parse_error(NULL);
        return;
    }


    x = GetSamples(V_SIZE(obj), V_ORG(obj));
    y = GetLines(V_SIZE(obj), V_ORG(obj));
    z = GetBands(V_SIZE(obj), V_ORG(obj));

    if (z != 1) {
        sprintf(error_buf, "Data for PGM file must have depth=1");
        parse_error(NULL);
        return;
    }

    if (fp == NULL) {
        fp = fopen(filename, "w");
    }
    if (fp == NULL) return;

    if (VERBOSE > 1)  {
        fprintf(stderr, "Writing %s: %dx%d PGM file.\n", filename, x,y);
    }   

    fprintf(fp, "P5\n%d %d\n255\n", x, y);
    fwrite(V_DATA(obj), 1, x*y, fp);

    fclose(fp);
    return;
}

void
WritePPM(Var *obj, FILE *fp, char *filename)
{
    int x,y,z;

    if (V_TYPE(obj) != ID_VAL || V_FORMAT(obj) != BYTE) {
        sprintf(error_buf, "Data for PPM output must be byte()");
        parse_error(NULL);
        return;
    }


    x = GetSamples(V_SIZE(obj), V_ORG(obj));
    y = GetLines(V_SIZE(obj), V_ORG(obj));
    z = GetBands(V_SIZE(obj), V_ORG(obj));

    if (z != 3) {
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
        fprintf(stderr, "Writing %s: %dx%d PPM file.\n", filename, x,y);
    }   

    fprintf(fp, "P6\n%d %d\n255\n", x, y);
    fwrite(V_DATA(obj), 3, x*y, fp);

    fclose(fp);
    return;
}

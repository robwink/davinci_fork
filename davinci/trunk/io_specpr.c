#include "parser.h" 
#include "io_specpr.h"

char *
decode_time(int s,char *buf)
{
    int hour;
    int minutes;
    int tsec;
    int sec;

    
    tsec = s/24000;
    hour = tsec/3600;
    minutes = (tsec - hour*3600)/60;
    sec = tsec - hour*3600 - minutes*60;

    sprintf(buf, "%2.2d:%2.2d:%2.2d", hour, minutes, tsec);
    buf[8] = '\0';
    return(buf);
}


char *
decode_date(int jday,char *buf)
{
    double ab,a,b,c,d,e,m,y,f;
    double day;
    int iday;

    day = (double)jday/10.0;
    iday = jday/10;
    jday = iday;

    f = day - (double)(iday) + 0.5;

    if (f >= 1.0){
        jday = jday + 1;
    }

    ab = floor((double)(jday/36524.25)-51.12264);
    a = (double)jday + 1.0 + ab - floor(ab/4.0);
    b = a + 1524.0;
    c = floor((b/365.25)-0.3343);
    d = floor(365.25*c);
    e = floor((b-d)/30.61);
    d = b - d - floor(30.61*e);
    m = e-1.0;
    y = c-4716.0;

    if (e > 13.5){
        m = m-12;
    }
    if (m < 2.5) {
        y=y+1;
    }
    if (y > 1900) y -= 1900;
    if (y < 0) y = 0;

    sprintf(buf, "%2.2d/%2.2d/%2.2d", (int) m, (int) d, (int) y);
    buf[8] = '\0';
    return(buf);
}

int
max_rec(int fd)
{
/*
  struct stat buf;
  stat(path, &buf);
  */
    int i;
    int f;
    f = dup(fd);
    i = lseek(f, 0, SEEK_END);
    close(f);
    return(i / LABELSIZE -1);
}

int
read_record(int fd, int i, char *label)
{
    if (lseek(fd, LABELSIZE * i, 0) == -1) {
        /* some error */
        return(-1);
    }
    if (read(fd, label, LABELSIZE) != LABELSIZE) {
        return(-2);
    }
    return(check_bit(((int*)label)[0], 0));
}

/**
 ** read_specpr() - read a specpr record.
 **
 ** returns: 	1 on success
 ** 			0 on failure (specified record is a continuation record)
 **			   -1 on file failure (EOF, error)
 **
 ** Pass data=NULL to not malloc and return data
 **/

int
read_specpr(int fd, int i, struct _label *label, char **data)
{
    struct _label label2;
    int count = 0;
    int j;
    int size;
    struct _tlabel *tlabel;

    if ((j = read_record(fd, i, (char *)label)) == 0) {
        tlabel = (struct _tlabel *)label;
        if (check_bit(label->icflag,1)) {
            /* text */
            if (data) {
                size = 1476;
                *data = (char *)calloc(1,size);
                memcpy(*data,tlabel->itext, size);
            }
        } else {
            if (data) {
                size = 256*sizeof(float);
                *data = (char *)malloc(size);
                memcpy(*data, label->data, size);
            }
        }
        while(read_record(fd, ++i, (char *)&label2)==1) {
            count++;
            if (data) {
                *data = (char *)my_realloc(*data,(size+count*1532));
                memcpy(*data+(size+(count-1)*1532), ((char *)&label2)+4, 1532);
            }
        }

        return(count+1);
    }
    return((j == 1 ? 0 : j));
}



int
write_record(int fd, int i, struct _label *label)
{
    if (i < 0) {
        if ((i = lseek(fd, 0, SEEK_END)) == 0) {
            char buf[1536];
            memset(buf, ' ', 1536);
            write(fd, buf, 1536);
            i = lseek(fd, 0, SEEK_END);
        }
        i /= LABELSIZE;
    } else if (lseek(fd, LABELSIZE * i, 0) == -1) {
        /* some error */
        return(-1);
    }

    if (check_bit(label->icflag, 0) == 0 && check_bit(label->icflag,1) == 0) {
        label->irecno = i;
    }

    if (write(fd, label, LABELSIZE) != LABELSIZE) {
        return(-2);
    }
    return(1);
}


int
specpr_open(char *path)
{
    int fout;
    char *p;

    if ((fout = open(path, O_RDONLY)) >= 0) {
        close(fout);
        fout = open(path, O_WRONLY | O_CREAT | O_APPEND);
        return(fout);
    } else {
        /* it doesn't exist.  Create it. */
        /**
        ** this needs to prepend the SPECPR_MAGIC cookie to record 0
        **/

        fout = open(path, O_WRONLY | O_CREAT | O_APPEND, 0777);
        if (fout < 0) return(fout);

        p = (char *)malloc(LABELSIZE);
        memset(p, '\0', LABELSIZE);
        memcpy(p, SPECPR_STAMP, strlen(SPECPR_STAMP));
        write(fout, p, LABELSIZE);
        free(p);
        return(fout);
    }
}


void
write_specpr(int fd, int i, struct _label *label, char *data)
{
    struct _tlabel *tlabel;
    struct _tlabel tmpl;
    int size;
    int offset;
    int count;

    tlabel = (struct _tlabel *)label;

    switch(check_bit(label->icflag,1)) {
    case 0:
        /* data */
        size = label->itchan*sizeof(float);
        offset = 256*sizeof(float);
        set_bit(tmpl.icflag,1,0);
        if (data != NULL) {
            memcpy(label->data, data, offset);
        }
        break;
    case 1:
        /* text */
        size = tlabel->itxtch;
        offset = 1476;
        set_bit(tmpl.icflag,1,1);
        if (data != NULL) {
            memcpy(tlabel->itext, data, offset);
        }
        break;
    }
    write_record(fd, i, label);

    count = 0;
    size -= offset;
    set_bit(tmpl.icflag,0,1);
    while(size > 0) {
        memcpy(((char *)&tmpl)+4, data+(offset+1532*count), 1532);
        write_record(fd, (i < 0 ? i : i+count+1), (struct _label *)&tmpl);
        size -= 1532;
        count++;
    }
}

void
julian_date(int *secs, int *date)
{
/* sets date to Julian day * 10 and time to secs since 0:00 hours UT */
    int jda, jsec, nday, isec;

    jda = 24405875;
    jsec = time(0);

    nday = jsec/(3600*24);
    isec = jsec - (nday*3600*24);
    jda = jda+nday*10;

    *date = jda;
    *secs = isec;
}


struct _label *
make_label(int npixels, int waves, char *title, char *ahist, char *mhist)
{
    int i;
    struct _label label, *lbl;

    label.icflag = 0;
    set_bit(label.icflag, 4, 1);
    set_bit(label.icflag, 5, 1);

    sprintf(label.usernm,"%s",getenv("USER"));
    julian_date(&label.iscta,&label.jdatea);
    label.iscta = label.isctb = label.iscta * 24000;
    label.jdateb = label.jdatea;
    label.istb = 0;
    label.isra = label.isdec = 0;
    label.itchan = npixels;
    label.irmas = label.revs = label.iband[0] = label.iband[1] = 1;
    label.irespt = label.itpntr = 0;
    label.siangl = label.seangl = label.sphase = 0;
    label.itimch = 1;
    label.xnrm = 1;
    label.scatin = 1;
    label.timint = 1;
    label.tempd = 273;

    label.irwav = waves ;/* wavelengths pointer */

    label.irecno = 0; /* record number pointer */
    
    label.ihist[0] = '\0';
    label.mhist[0] = '\0';
    label.ititl[0] = '\0';

    if (title != NULL) 
        for (i = 0 ; i < 40 ; i++)
            label.ititl[i] = (i >= strlen(title) ? ' ' : title[i]);
    if (ahist != NULL) 
        for (i = 0 ; i < 60 ; i++)
            label.ihist[i] = (i >= strlen(ahist) ? ' ' : ahist[i]);
    if (mhist != NULL) 
        for (i = 0 ; i < 296 ; i++)
            label.mhist[i] = (i >= strlen(mhist) ? ' ' : mhist[i]);

    label.iwtrns = label.nruns = npixels; /* number of blocks averaged */
    label.data[0] = 0; /* averaged spectra data */

    lbl = (struct _label *)malloc(sizeof(struct _label));
    memcpy(lbl, &label, sizeof(struct _label));
    return(lbl);
}


/**
 ** is_specpr() - detect the magic cookie for specpr files.
 ** returns:
 **		0: on failure
 **		1: on success
 **/
int
is_specpr(FILE *fp)
{
    int len;
    char buf[16];

    rewind(fp);
    len = fread(buf, 1, strlen(SPECPR_MAGIC), fp);
    return (len == strlen(SPECPR_MAGIC) && !strncmp(buf, SPECPR_MAGIC, len));
}

#ifndef STANDALONE
/**
 ** LoadSpecpr() - Get record out of a specpr file, and load into a Var struct
 **/

Var *
LoadSpecpr(FILE *fp,char *filename,int rec)
{
    struct _label label;
    float *data;
    Var *s;

    /**
    ** Verify file type.
    **/
    if (!is_specpr(fp)) return(NULL); 

    if (rec == 0) {
        parse_error("No record specified.");
        return(NULL);
    }

    if (read_specpr(fileno(fp), rec, &label, (char **)&data) > 0) {
        s = newVar();
        V_TYPE(s) = ID_VAL;
        V_DATA(s) = data;
        V_DSIZE(s) = label.itchan;
        V_SIZE(s)[0] = 1;
        V_SIZE(s)[1] = 1;
        V_SIZE(s)[2] = label.itchan;
        V_FORMAT(s) = FLOAT;
        V_ORDER(s) = BSQ;

        V_TITLE(s) = (char *)malloc(41);
        strncpy(V_TITLE(s), label.ititl, 40);
        V_TITLE(s)[40] = '\0';

        if (VERBOSE > 1)  {
            fprintf(stderr, "%s#%d: SpecPR record: %d channels\nTitle: %s\n",
                    filename, rec, V_DSIZE(s), V_TITLE(s));
        }

        return(s);
    } else {
        sprintf(error_buf, "continuation record %s#%d", filename, rec);
        parse_error(NULL);
        return(NULL);
    }
}


/**
 ** This will append data to a specpr file or create a new one if it
 ** doesn't exist.
 **/

int
WriteSpecpr(Var *v, char *filename, char *title)
{
    Var *e;
    float *fdata;
    int i;
    FILE *fp;
    int fd;
    struct _label *label;
    char buf[256];
    char ahist[256];
    char mhist[1024];

    e = eval(v);

    if (e == NULL)  {
        sprintf(error_buf, "Variable not found: %s", V_NAME(v));
        parse_error(NULL);
        return(0);
    }
    v = e;

    if (V_TYPE(v) != ID_VAL) {
        sprintf(error_buf, "Invalid type to write: %s", V_NAME(v));
        parse_error(NULL);
        return(0);
    }

    if (((V_SIZE(v)[0] == 1) + (V_SIZE(v)[1]==1) + (V_SIZE(v)[2]==1)) < 2) {
        sprintf(error_buf, "Variable has too many dimensions: %s", V_NAME(v));
        parse_error(NULL);
        return(0);
    }

    /**
    ** Open the file
    **/

    fp = fopen(filename, "r+");
    if (fp == NULL) {
        fd = specpr_open(filename);
    } else {
        /**
        ** Verify its a specpr file
        **/
        fread(buf,sizeof(SPECPR_MAGIC),1,fp);
        if (strncmp(buf, SPECPR_MAGIC, strlen(SPECPR_MAGIC))) {
            sprintf(error_buf, "Not a SPECPR file: %s", V_NAME(v));
            parse_error(NULL);
            fclose(fp);
            return(0);
        }
        fd = fileno(fp);
    }

    if (V_FORMAT(v) == FLOAT) {
        fdata = (float *)V_DATA(v);
    } else {
        fdata = (float *)calloc(V_DSIZE(v), sizeof(float));
        for (i = 0 ; i < V_DSIZE(v) ; i++) {
            fdata[i] = extract_float(v, i);
        }
    }

    if (VERBOSE > 1) {
        fprintf(stderr, "Writing %s: SpecPR record: %d channels\nTitle: %s\n",
                filename, V_DSIZE(v), title);
    }

    sprintf(ahist, "%60.60s", "daVinci generated record");
    ahist[60] = '\0';
    mhist[0] = '\0';

    label = make_label(V_DSIZE(v), 0, title, ahist, mhist);
    write_specpr(fd, -1, label, (char *)fdata);
    free(label);

    if (V_DATA(v) != fdata) free(fdata);
    fclose(fp);

    return 1;
}


/**
 ** LoadSpecprHeader() - extract a specific header element from a specpr file
 **/
int
LoadSpecprHeader(FILE *fp, char *filename, int rec, char *element, Var **val)
{
    struct _label label;
    struct _tlabel *tlabel;
    float *data;
    Var *v = NULL;
    int *ival = NULL;
    float *fval = NULL;
    char *tval = NULL;
    int range = 1;

    /**
    ** Verify file type.
    **/
    if (!is_specpr(fp)) return(0); 

    if (rec == 0) {
        parse_error("header(): Must specify record for SpecPR file.");
        return(1);
    }

    if (read_specpr(fileno(fp), rec, &label, (char **)&data) <= 0) {
        sprintf(error_buf, "header(): record is SpecPR continuation record %s#%d", filename, rec);
        parse_error(NULL);
        return(1);
    }
    tlabel = (struct _tlabel *)&label;

    if (!strcasecmp(element, "icflag")) ival = &label.icflag;
    else if (!strcasecmp(element, "ititl")) {
        tval = label.ititl;
        range = 40;
    }
    else if (!strcasecmp(element, "usernm")) {
        tval = label.usernm;
        range = 8;
    }
    else if (!strcasecmp(element, "iscta")) ival = &label.iscta;
    else if (!strcasecmp(element, "isctb")) ival = &label.isctb;
    else if (!strcasecmp(element, "jdatea")) ival = &label.iscta;
    else if (!strcasecmp(element, "jdateb")) ival = &label.iscta;
    else if (!strcasecmp(element, "istb")) ival = &label.istb;
    else if (!strcasecmp(element, "isra")) ival = &label.isra;
    else if (!strcasecmp(element, "isdec")) ival = &label.isdec;
    else if (!strcasecmp(element, "itchan")) ival = &label.itchan;
    else if (!strcasecmp(element, "irmas")) ival = &label.irmas;
    else if (!strcasecmp(element, "revs")) ival = &label.revs;
    else if (!strcasecmp(element, "iband")) {
        ival = label.iband;
        range = 2;
    }
    else if (!strcasecmp(element, "irwav")) ival = &label.irwav;
    else if (!strcasecmp(element, "irespt")) ival = &label.irespt;
    else if (!strcasecmp(element, "irecno")) ival = &label.irecno;
    else if (!strcasecmp(element, "itpntr")) ival = &label.itpntr;
    else if (!strcasecmp(element, "ihist")) {
        tval = label.ihist;
        range = 60;
    }
    else if (!strcasecmp(element, "mhist")) {
        tval = label.mhist;
        range = 296;
    }
    else if (!strcasecmp(element, "nruns")) ival = &label.nruns;
    else if (!strcasecmp(element, "siangl")) ival = &label.siangl;
    else if (!strcasecmp(element, "seangl")) ival = &label.seangl;
    else if (!strcasecmp(element, "sphase")) ival = &label.sphase;
    else if (!strcasecmp(element, "iwtrns")) ival = &label.iwtrns;
    else if (!strcasecmp(element, "itimch")) ival = &label.itimch;
    else if (!strcasecmp(element, "xnrm")) fval = &label.xnrm;
    else if (!strcasecmp(element, "scatin")) fval = &label.scatin;
    else if (!strcasecmp(element, "timint")) fval = &label.timint;
    else if (!strcasecmp(element, "tempd")) fval = &label.tempd;

    else if (!strcasecmp(element, "itxtpt")) ival = &tlabel->itxtpt;
    else if (!strcasecmp(element, "itxtch")) ival = &tlabel->itxtch;
    else if (!strcasecmp(element, "itext")) {
        /**
        ** special case.  Get complete text from data
        **/
    } else {
        sprintf(error_buf, "header(): Unrecognized SpecPR header element: %s\n", element);
        parse_error(NULL);
        return(1);
    }

    if (ival != NULL) {
        v = newVar();
        V_TYPE(v) = ID_VAL;
        V_DSIZE(v) = range;
        V_SIZE(v)[0] = range;
        V_SIZE(v)[1] = V_SIZE(v)[2] = 1;
        V_ORG(v) = BSQ;
        V_FORMAT(v) = INT;
        V_DATA(v) = calloc(range, sizeof(int));
        memcpy(V_DATA(v), ival, sizeof(int)*range);
    }  else if (fval != NULL) {
        v = newVar();
        V_TYPE(v) = ID_VAL;
        V_DSIZE(v) = V_SIZE(v)[0] = range;
        V_SIZE(v)[1] = V_SIZE(v)[2] = 1;
        V_ORG(v) = BSQ;
        V_FORMAT(v) = FLOAT;
        V_DATA(v) = calloc(range, sizeof(float));
        memcpy(V_DATA(v), fval, sizeof(float)*range);
    } else if (tval != NULL) {
        v = newVar();
        V_TYPE(v) = ID_STRING;
        V_STRING(v) = strndup((char *)tval, range);
    }

    *val = v;
    return(1);
}

#endif

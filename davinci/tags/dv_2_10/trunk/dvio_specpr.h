#ifndef _SPECPR_H
#define _SPECPR_H

#define SPECPR_SUFFIX   '#'


#define check_bit(i,n) ((i & (1 << n)) != 0)
#define set_bit(i,n,m) i = ((i & (~(1<<n))) | (m<<n))

#define SPECPR_MAGIC	"SPECPR_FS"
#define SPECPR_STAMP	"SPECPR_FS_2.0\r\nRECORD_BYTES=1536\r\nLABEL_RECORDS=1\r\n"
#define LABELSIZE   1536

struct _label {
    int icflag;
    char ititl[40];
    char usernm[8];
    int iscta;
    int isctb;
    int jdatea;
    int jdateb;
    int istb;
    int isra;
    int isdec;
    int itchan;
    int irmas;
    int revs;
    int iband[2];
    int irwav;			/* pointer to wavelenghts */
    int irespt;		
    int irecno;
    int itpntr;			/* text pointer.  Usually description */
    char ihist[60];
    char mhist[296];
    int nruns;
    int siangl;
    int seangl;
    int sphase;
    int iwtrns;
    int itimch;
    float xnrm;
    float scatin;
    float timint;
    float tempd;
    float data[256];
};

struct _label* make_label();

struct _tlabel {
    int icflag;
    char ititl[40];
    char usernm[8];
    int itxtpt;
    int itxtch;
    char itext[1476];
};

char *decode_time();
char *decode_date();

#ifdef _LITTLE_ENDIAN

static char ctmp;
typedef char *cptr;
#define swp(c1, c2)     (ctmp = (c1) , (c1) = (c2) , (c2) = ctmp)

#define MSB8(s)         (swp(((cptr)(s))[0], ((cptr)(s))[7]), \
                                        swp(((cptr)(s))[1], ((cptr)(s))[6]), \
                                        swp(((cptr)(s))[2], ((cptr)(s))[5]), \
                                        swp(((cptr)(s))[3], ((cptr)(s))[4]),(s))

#define MSB4(s)         (swp(((cptr)(s))[0], ((cptr)(s))[3]), \
                                        swp(((cptr)(s))[1], ((cptr)(s))[2]),(s))

#define MSB2(s)         (swp(((cptr)(s))[0], ((cptr)(s))[1]),(s))

#else

#define MSB8(s)         (s)
#define MSB4(s)         (s)
#define MSB2(s)         (s)

#endif 

#endif /* _SPECPR_H */

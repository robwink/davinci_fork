#ifdef HAVE_CONFIG_H
#include <iom_config.h>
#endif /* HAVE_CONFIG_H */

#include "iomedley.h"

struct goes_area {
	int cookie;        /* 1  */
	int format;        /* 2  */
	int sss;           /* 3  */
	int julian;        /* 4  */
	int time;          /* 5  */
	int upperleftline; /* 6  */
	int upperleftele;  /* 7  */
	int pad;           /* 8  */
	int lines;         /* 9  */
	int samples;       /* 10 */
	int size;          /* 11 */
	int lineres;       /* 12 */
	int eleres;        /* 13 */
	int nchans;        /* 14 */
	int presiz;        /* 15 */
	int proj;          /* 16 */
	int cdate;         /* 17 */
	int ctime;         /* 18 */
	int filter_map;    /* 19 */
	int m1[5];         /* 20 */
	char memo[32];     /* 25 */
	int fileno;        /* 33 */
	int data_offset;   /* 23 */
	int nav_offset;    /* 24 */
	int val_code;      /* 25 */
	int m2[12];        /* 26 */
	int lp_doc_len;    /* 27 */
	int lp_cal_len;    /* 28 */
	int lp_lms_len;    /* 29 */
	char source[4];    /* 30 */
	char cal_type[4];  /* 31 */
	int m3[9];         /* 32 */
	int cal_offset;    /* 33 */
	int n_comment;     /* 34 */
};

#define GOES_MAGIC "\0\0\0\0\0\0\0\4"

int iom_isGOES(FILE* fp)
{
	int len;
	char buf[16];

	/**
	 ** GOES_MAGIC is 8 characters, mostly 0's
	 **/

	rewind(fp);
	len = fread(buf, 1, 8, fp);

	/* GOES_MAGIC is all byte-oriented */
	return (len == 8 && !memcmp(buf, GOES_MAGIC, 8));
}

int iom_GetGOESHeader(FILE* fp, char* fname, struct iom_iheader* h)
{
	struct goes_area g;
	int i;

	if (iom_isGOES(fp) == 0) {
		return (0);
	}

	rewind(fp);
	fread(&g, sizeof(g), 1, fp);

#ifndef WORDS_BIGENDIAN
	/* swap integer fields within the header for little-endian machines */

	iom_MSB4((char*)&g.cookie);
	iom_MSB4((char*)&g.format);
	iom_MSB4((char*)&g.sss);
	iom_MSB4((char*)&g.julian);
	iom_MSB4((char*)&g.time);
	iom_MSB4((char*)&g.upperleftline);
	iom_MSB4((char*)&g.upperleftele);
	iom_MSB4((char*)&g.pad);
	iom_MSB4((char*)&g.lines);
	iom_MSB4((char*)&g.samples);
	iom_MSB4((char*)&g.size);
	iom_MSB4((char*)&g.lineres);
	iom_MSB4((char*)&g.eleres);
	iom_MSB4((char*)&g.nchans);
	iom_MSB4((char*)&g.presiz);
	iom_MSB4((char*)&g.proj);
	iom_MSB4((char*)&g.cdate);
	iom_MSB4((char*)&g.ctime);
	iom_MSB4((char*)&g.filter_map);

	for (i = 0; i < 5; i++) {
		iom_MSB4((char*)&g.m1[i]);
	}

	iom_MSB4((char*)&g.fileno);
	iom_MSB4((char*)&g.data_offset);
	iom_MSB4((char*)&g.nav_offset);
	iom_MSB4((char*)&g.val_code);

	for (i = 0; i < 12; i++) {
		iom_MSB4((char*)&g.m2[i]);
	}

	iom_MSB4((char*)&g.lp_doc_len);
	iom_MSB4((char*)&g.lp_cal_len);
	iom_MSB4((char*)&g.lp_lms_len);

	for (i = 0; i < 9; i++) {
		iom_MSB4((char*)&g.m3[i]);
	}

	iom_MSB4((char*)&g.cal_offset);
	iom_MSB4((char*)&g.n_comment);

#endif /* WORDS_BIGENDIAN */

	/* memset(h, 0, sizeof(*h)); */
	iom_init_iheader(h);

	h->org     = iom_BSQ;
	h->size[0] = g.samples;
	h->size[1] = g.lines;
	h->size[2] = 1;

	h->eformat = iom_EDF_INVALID;
	h->format  = -1;
	if (g.size == 1) {
		h->format  = iom_BYTE;
		h->eformat = iom_MSB_INT_1;
	}
	if (g.size == 2) {
		h->format  = iom_SHORT;
		h->eformat = iom_MSB_INT_2;
	}
	if (g.size == 4) {
		h->format  = iom_INT;
		h->eformat = iom_MSB_INT_4;
	}

	h->prefix[1] = h->prefix[2] = 0;
	h->prefix[0]                = g.presiz;

	h->suffix[0] = h->suffix[1] = h->suffix[2];
	h->dptr                     = g.data_offset;

	return (1);
}

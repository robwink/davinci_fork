struct goes_area {
	int cookie;					/* 1  */
	int format;					/* 2  */
	int sss;					/* 3  */
	int julian;					/* 4  */
	int time;					/* 5  */
	int upperleftline;			/* 6  */
	int upperleftele;			/* 7  */
	int pad;					/* 8  */
	int lines;					/* 9  */
	int samples;				/* 10 */
	int size;					/* 11 */
	int lineres;				/* 12 */
	int eleres;					/* 13 */
	int nchans;					/* 14 */
	int presiz;					/* 15 */
	int proj;					/* 16 */
	int cdate;					/* 17 */
	int ctime;					/* 18 */
	int filter_map;				/* 19 */
	int m1[5];					/* 20 */
	char memo[32];				/* 25 */
	int fileno;					/* 33 */
	int data_offset;			/* 23 */
	int nav_offset;				/* 24 */
	int val_code;				/* 25 */
	int m2[12];					/* 26 */
	int lp_doc_len;				/* 27 */
	int lp_cal_len;				/* 28 */
	int lp_lms_len;				/* 29 */
	char source[4];				/* 30 */
	char cal_type[4];			/* 31 */
	int m3[9];					/* 32 */
	int cal_offset;				/* 33 */
	int n_comment;				/* 34 */
};

#define GOES_MAGIC "\0\0\0\0\0\0\0\4"

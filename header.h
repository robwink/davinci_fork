#ifndef _HEADER_H
#define _HEADER_H

#include <sys/types.h>
#include <sys/stat.h>
#include "tools.h"

#ifdef _WIN32
#include <io.h>

typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned long ulong;

#define F_OK 0
#define R_OK 4

#ifndef PROT_READ
#define PROT_READ 5
#endif

#if 0
/* The following two defines have dummy values */
#define PROT_WRITE 6
#define MAP_PRIVATE 1
#endif /* 0 */


#else
#include <unistd.h>
#include <sys/mman.h>
#endif

enum _external_format {
    INVALID_EFORMAT = -1,
    CHARACTER = 1,
    MSB_INTEGER, MSB_UNSIGNED_INTEGER, 
    IEEE_REAL, ASCII_INTEGER, ASCII_REAL,
    BYTE_OFFSET, MSB_BIT_FIELD, LSB_BIT_FIELD,
	LSB_INTEGER, LSB_UNSIGNED_INTEGER, PC_REAL
};

enum _internal_format {
    INVALID_IFORMAT = -1,
    VINT = 1, UVINT, REAL, STRING
};

typedef enum _internal_format IFORMAT;
typedef enum _external_format EFORMAT;

enum _varformat { 
    VAX_VAR = 1,
    Q15 = 2
};

typedef char *PTR;

typedef struct _dataset DATASET;
typedef struct _label LABEL;
typedef struct _field FIELD;
typedef struct _vardata VARDATA;
typedef struct _fragment FRAGMENT;
typedef union _data DATA;
typedef enum _varformat VARFORMAT;
typedef struct _ostruct OSTRUCT;
typedef struct _table TABLE;
typedef struct _select SELECT;
typedef struct _tblbuff TBLBUFF;
typedef struct _bitfield BITFIELD;
typedef struct _fakefield FAKEFIELD;
typedef void (*FuncPtr) ();

struct _dataset {
    LIST *tablenames;	/* (char) */
    LIST *tables;	/* (TABLE) */
};

struct _table {
    LABEL *label;       /* label for this table */
    LIST *files;        /* (char) sorted list of directory entries */
	LIST *selects;		/* (SELECT) list of selections for this table */
	TBLBUFF *buff;
};


struct _label {
    int reclen;
    char *name;
    int nfields;
	 int nrows;
    LIST *fields;	/* (FIELD) */
    LIST *keys;		/* (FIELD) */
    TABLE *table;       /* pointer to parent table struct */
};

struct _field {
    char *name;         /* field name */
    char *alias;        /* alt field name */
    char *type;
    EFORMAT eformat;    /* external field type */
    IFORMAT iformat;    /* internal field type */
    int dimension;      /* array dimension */
    int start;          /* bytes in from start of record */
    int size;           /* size in bytes */
    float scale;        /* scale factor */
    float offset;       /* scale offset */
    VARDATA *vardata;   /* variable length data info */
    LABEL *label;       /* the label this field lives in */
	BITFIELD *bitfield;
	FAKEFIELD *fakefield;
};

struct _bitfield {
	EFORMAT type;
	int start_bit;
	int bits;
	uint mask;
	int shifts;
};

struct _vardata {
    int type;           /* type of var record (VAX vs Q15) */
    EFORMAT eformat;    /* eformat of 1 element of var record */
    IFORMAT iformat;    /* iformat of 1 element of var record */
    int size;           /* size of 1 element of var record */
};

struct _fakefield {
	char *name;
	int nfields;		/* list of dependent fields */
	FIELD **fields;		
	FuncPtr fptr;		/* function to compute and output results */
	void (*cook_function)(OSTRUCT *, int, int);
	void (*print_header_function)(OSTRUCT *);
};

struct _fragment {
    int offset;         /* pointer to start of data */
    int nrows;          /* number of rows in this fragment */
    LIST *start_keys;   /* (DATA) key values of first record */
    LIST *end_keys;     /* (DATA) key values of last record */
    struct stat sbuf;
};

union _data {
    int i;
    uint ui;
    double r;
    char *str;
};

#define  O_DELIM '\t'


typedef struct {
    PTR start_rec;
    PTR end_rec;
    FIELD *party_key;
    int validate;
} SLICE;

typedef struct {
	int start;
	int end;
} RANGE;


#define OF_SCALED     (1<<0)
#define OF_VARDATA    (1<<1)
#define OF_OPEN_RANGE (1<<2)
#define OF_FAKEFIELD  (1<<3)

struct _ostruct {
	FIELD *field;
	RANGE range;
	SLICE *slice;
	TABLE *table;
	char *text;
	struct {
		RANGE range;
        int   flags;
        int   frame_size;
      
		int is_atomic;                  /* Don't need this */
		FuncPtr print_func;
		FuncPtr print_func2;            /* Don't need this */
		FuncPtr print_func2alt;         /* Don't need this */
		FuncPtr var_print_nelements;    /* Don't need this */
	} cooked;
};

struct _select {
	FIELD *field;
	char *name;
	char *low_str;
	char *high_str;
	DATA low;
	DATA high;
	int start;		/* the real field start offset */
};

struct _tblbuff {
	PTR buf;
	int len;
	int fileidx;
	
	PTR curr;
	PTR end;
	TABLE *tbl;
	int reclen;

	PTR varbuf;
	int varlen;

	FRAGMENT *frag;
};

/* Key sequencing array */
typedef struct {
	int count;
	char **name;
} SEQ;

#include "proto.h"

#ifdef WORDS_BIGENDIAN

#define MSB8(s) 	(s)
#define MSB4(s) 	(s)
#define MSB2(s) 	(s)

#else /* little endian */

char ctmp;

typedef char *cptr;
#define swp(c1, c2)	(ctmp = (c1) , (c1) = (c2) , (c2) = ctmp)

#define MSB8(s) 	(swp(((cptr)(s))[0], ((cptr)(s))[7]), \
					swp(((cptr)(s))[1], ((cptr)(s))[6]), \
					swp(((cptr)(s))[2], ((cptr)(s))[5]), \
					swp(((cptr)(s))[3], ((cptr)(s))[4]),(s))

#define MSB4(s) 	(swp(((cptr)(s))[0], ((cptr)(s))[3]), \
					swp(((cptr)(s))[1], ((cptr)(s))[2]),(s))

#define MSB2(s) 	(swp(((cptr)(s))[0], ((cptr)(s))[1]),(s))

#endif /* WORDS_BIGENDIAN */

#define ListElement(a, b, c)    ((a*)(b)->ptr)[c]

#endif /* _HEADER_H */

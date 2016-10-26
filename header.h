#ifndef _HEADER_H
#define _HEADER_H

#include "io_lablib3.h" //for OBJDESC
#include "tools.h"
#include <sys/stat.h>
#include <sys/types.h>


// NOTE(rswinkle): This file is almost exactly an extension of iomedley/header.h
// All this duplication bothers me.  There must be a way to use a single file.

#include <unistd.h>
#include <sys/mman.h>

enum _external_format {
	INVALID_EFORMAT = -1,
	CHARACTER       = 1,
	MSB_INTEGER,
	MSB_UNSIGNED_INTEGER,
	IEEE_REAL,
	ASCII_INTEGER,
	ASCII_REAL,
	BYTE_OFFSET,
	MSB_BIT_FIELD,
	LSB_BIT_FIELD,
	LSB_INTEGER,
	LSB_UNSIGNED_INTEGER,
	PC_REAL
};

enum _internal_format { INVALID_IFORMAT = -1, VINT = 1, UVINT, REAL, STRING };

typedef enum _internal_format IFORMAT;
typedef enum _external_format EFORMAT;

enum _varformat { VAX_VAR = 1, Q15 = 2 };

typedef char* PTR;


typedef struct _label LABEL;
typedef struct _field FIELD;
typedef struct _bitfield BITFIELD;







typedef struct _dataset DATASET;
typedef struct _vardata VARDATA;
typedef struct _fragment FRAGMENT;
typedef union _data DATA;
typedef enum _varformat VARFORMAT;
typedef struct _ostruct OSTRUCT;
typedef struct _table TABLE;
typedef struct _select SELECT;
typedef struct _tblbuff TBLBUFF;
typedef struct _fakefield FAKEFIELD;
typedef void (*FuncPtr)();


struct _dataset {
	cvector_voidptr tablenames; //(char)
	cvector_voidptr tables;     // (TABLE)
};

struct _table {
	LABEL* label;  /* label for this table */
	cvector_voidptr files;   /* (char) sorted list of directory entries */
	cvector_voidptr selects; /* (SELECT) list of selections for this table */
	TBLBUFF* buff;
};

struct _label {
	int reclen;
	char* name;
	int nfields;
	int nrows;
	cvector_voidptr fields;
	cvector_voidptr keys;
	TABLE* table; /* pointer to parent table struct */
};

struct _field {
	char* name;  /* field name */
	char* alias; /* alt field name */
	char* type;
	EFORMAT eformat;  /* external field type */
	IFORMAT iformat;  /* internal field type */
	int dimension;    /* array dimension */
	int start;        /* bytes in from start of record */
	int size;         /* size in bytes */
	float scale;      /* scale factor */
	float offset;     /* scale offset */
	VARDATA* vardata; /* variable length data info */
	LABEL* label;     /* the label this field lives in */
	BITFIELD* bitfield;
	FAKEFIELD* fakefield;
};

struct _bitfield {
	EFORMAT type;
	int start_bit;
	int bits;
	uint mask;
	int shifts;
};

struct _vardata {
	int type;        /* type of var record (VAX vs Q15) */
	EFORMAT eformat; /* eformat of 1 element of var record */
	IFORMAT iformat; /* iformat of 1 element of var record */
	int size;        /* size of 1 element of var record */
};

struct _fakefield {
	char* name;
	int nfields; /* list of dependent fields */
	FIELD** fields;
	FuncPtr fptr; /* function to compute and output results */
	void (*cook_function)(OSTRUCT*, int, int);
	void (*print_header_function)(OSTRUCT*);
};

struct _fragment {
	int offset;       /* pointer to start of data */
	int nrows;        /* number of rows in this fragment */
	cvector_voidptr start_keys; // (DATA) key values of first record
	cvector_voidptr end_keys;   // (DATA) key values of last record
	struct stat sbuf;
};

union _data {
	int i;
	uint ui;
	double r;
	char* str;
};

#define O_DELIM '\t'

typedef struct {
	PTR start_rec;
	PTR end_rec;
	FIELD* party_key;
	int validate;
} SLICE;

typedef struct {
	int start;
	int end;
} RANGE;

#define OF_SCALED (1 << 0)
#define OF_VARDATA (1 << 1)
#define OF_OPEN_RANGE (1 << 2)
#define OF_FAKEFIELD (1 << 3)

struct _ostruct {
	FIELD* field;
	RANGE range;
	SLICE* slice;
	TABLE* table;
	char* text;
	struct {
		RANGE range;
		int flags;
		int frame_size;

		int is_atomic; /* Don't need this */
		FuncPtr print_func;
		FuncPtr print_func2;         /* Don't need this */
		FuncPtr print_func2alt;      /* Don't need this */
		FuncPtr var_print_nelements; /* Don't need this */
	} cooked;
};

struct _select {
	FIELD* field;
	char* name;
	char* low_str;
	char* high_str;
	DATA low;
	DATA high;
	int start; /* the real field start offset */
};

struct _tblbuff {
	PTR buf;
	int len;
	int fileidx;

	PTR curr;
	PTR end;
	TABLE* tbl;
	int reclen;

	PTR varbuf;
	int varlen;

	FRAGMENT* frag;
};

/* Key sequencing array */
typedef struct {
	int count;
	char** name;
} SEQ;

#ifdef WORDS_BIGENDIAN

#define MSB8(s) (s)
#define MSB4(s) (s)
#define MSB2(s) (s)

#else /* little endian */

static char ctmp;

typedef char* cptr;
#define swp(c1, c2) (ctmp = (c1), (c1) = (c2), (c2) = ctmp)

#define MSB8(s)                                                                \
	(swp(((cptr)(s))[0], ((cptr)(s))[7]), swp(((cptr)(s))[1], ((cptr)(s))[6]), \
	 swp(((cptr)(s))[2], ((cptr)(s))[5]), swp(((cptr)(s))[3], ((cptr)(s))[4]), (s))

#define MSB4(s) (swp(((cptr)(s))[0], ((cptr)(s))[3]), swp(((cptr)(s))[1], ((cptr)(s))[2]), (s))

#define MSB2(s) (swp(((cptr)(s))[0], ((cptr)(s))[1]), (s))

#endif /* WORDS_BIGENDIAN */

#define ListElement(a, b, c) ((a*)(b)->ptr)[c]

// TODO(rswinkle) organize these in same order as header.c for sanity
LABEL* LoadLabel(char*);
LABEL* LoadLabelFromObjDesc(OBJDESC* tbl, const char*);

IFORMAT eformat_to_iformat(EFORMAT e);
EFORMAT ConvertType(char* type);


//FIELD* FindField(char* name, LIST* labels);
//FIELD* FindFieldInLabel(char* name, LABEL* l);


//FRAGMENT* LoadFragment(char* fname, TABLE* table);
//void FreeFragment(FRAGMENT* f);
//DATA ConvertASCIItoData(char* ascii, int i);
//DATA ConvertField(char* ptr, FIELD* f);
//int ConvertSelect(DATASET* d, char* sel_str);
//LIST* LoadFilenames(char* path, char* prefix);
//DATASET* LoadDataset(DATASET* dataset, char* fname);

//DATA ConvertData(PTR ptr, FIELD* f);
//DATA ConvertFieldData(PTR ptr, FIELD* f);
//DATA ConvertVarData(PTR ptr, VARDATA* v);
//int EquivalentData(DATA d1, DATA d2, FIELD* f);
//int CompareData(DATA d1, DATA d2, FIELD* f);
//DATA* maxFieldVal(SLICE* s, int dim, TABLE** tbl, DATA* maxValue);
//
//LIST* Make_Index(char* fields_str, LIST* tables);
//void search(int deep, int maxdepth, SLICE** slice, TABLE** tbl, int tcount);
//void output_rec(OSTRUCT** o, int n);
//void SortFiles(LIST* list);
//PTR RefillTblBuff(TBLBUFF* b);
//
///* TBLBUFF *NewTblBuff(TABLE * t, size_t reccount, size_t overcount); */
//TBLBUFF* NewTblBuff(TABLE* t);
//PTR GetFirstRec(TABLE* t);
//PTR find_jump(TABLE* t, FIELD* f, DATA d, PTR beg, PTR end, int deep);
//PTR find_until(TABLE* t, FIELD* f, PTR beg, PTR end);
//PTR find_select(TABLE* t, PTR beg, PTR end);
//
//int sequence_keys(SEQ* keyseq, TABLE** tables, int num_tables);
//SLICE** init_slices(TABLE** tables, int tcount, SEQ keyseq);
//
//LIST* ConvertOutput(char* output_str, LIST* tables);
//char* find_file(char* fname);
//short ConvertVaxVarByteCount(PTR raw, VARDATA* vdata);
//
//FIELD* FindFakeField(char* name, LIST* tables);
//double ConvertAndScaleData(PTR raw, FIELD* field);
//
//PTR GiveMeVarPtr(PTR raw, TABLE* table, int offset);


#endif /* _HEADER_H */

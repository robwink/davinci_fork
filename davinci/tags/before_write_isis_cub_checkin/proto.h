#ifndef _PROTO_H
#define _PROTO_H

#include "header.h"

LABEL *LoadLabel(char *);
DATASET * LoadDataset(DATASET *dataset, char *fname);
FIELD *FindField(char *name, LIST * labels);
FIELD *FindFieldInLabel(char *name, LABEL * l);
FRAGMENT *LoadFragment(char *fname, TABLE *table);
void FreeFragment(FRAGMENT *f);

DATA ConvertASCIItoData(char *ascii, int i);
DATA ConvertField(char *ptr, FIELD *f);
LIST *LoadFilenames(char *path, char *prefix);

DATA ConvertData(PTR ptr, FIELD *f);
DATA ConvertFieldData(PTR ptr, FIELD *f);
DATA ConvertVarData(PTR ptr, VARDATA *v);
int EquivalentData(DATA d1, DATA d2, FIELD *f);
int CompareData(DATA d1, DATA d2, FIELD *f);
DATA *maxFieldVal(SLICE * s, int dim, TABLE **tbl, DATA *maxValue);

LIST *Make_Index(char *fields_str, LIST *tables);
int ConvertSelect(DATASET * d, char *sel_str);
void search(int deep, int maxdepth, SLICE ** slice, TABLE ** tbl, int tcount);
void output_rec(OSTRUCT **o, int n);
void SortFiles(LIST *list);
PTR RefillTblBuff(TBLBUFF *b);

/* TBLBUFF *NewTblBuff(TABLE * t, size_t reccount, size_t overcount); */
TBLBUFF *NewTblBuff(TABLE *t);
PTR GetFirstRec(TABLE * t);
PTR find_jump(TABLE * t, FIELD * f, DATA d, PTR beg, PTR end, int deep);
PTR find_until(TABLE * t, FIELD * f, PTR beg, PTR end);
PTR find_select(TABLE * t, PTR beg, PTR end);

int sequence_keys(SEQ * keyseq, TABLE ** tables, int num_tables);
SLICE ** init_slices(TABLE ** tables, int tcount, SEQ keyseq);

LIST * ConvertOutput(char *output_str, LIST *tables);
char * find_file(char *fname);
short ConvertVaxVarByteCount(PTR raw, VARDATA *vdata);

FIELD * FindFakeField(char *name, LIST *tables);
double ConvertAndScaleData(PTR raw, FIELD * field);

PTR   GiveMeVarPtr(PTR raw, TABLE *table, int offset);
#endif /* _PROTO_H */

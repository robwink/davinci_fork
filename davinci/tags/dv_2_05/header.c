static char rcsver[] = "$Id$";
 
/**
 ** $Source$
 **
 ** $Log$
 ** Revision 1.5  2002/09/24 21:36:26  gorelick
 ** Endian normalization changes for table reads
 **
 ** Revision 1.4  2002/06/15 03:33:01  gorelick
 ** Miscelaneous checking
 **
 ** Revision 1.3  2001/01/12 00:29:22  gorelick
 ** Version 0.64
 **
 ** Revision 1.2  2000/11/28 21:39:10  gorelick
 ** Some minor compile bugs
 **
 ** Revision 1.1  2000/04/07 19:16:16  asbms
 ** io_pds support files from Vanilla
 **
 ** Revision 1.2  1999/11/19 21:19:46  gorelick
 ** Version 3.1, post PDS delivery of 3.0
 **
 ** Revision 1.1.1.1  1999/10/15 19:30:35  gorelick
 ** Version 3.0
 **
 **
 ** Revision 1.4  1998/11/12 22:58:55  gorelick
 ** first release version
 **
 **/

#include <limits.h>
#include "header.h"
#include "proto.h"
#include "io_lablib3.h"

#define GetKey(ob, name)        OdlFindKwd(ob, name, NULL, 0, ODL_THIS_OBJECT)

FIELD *MakeField(OBJDESC *, LABEL *);
int DetermineFieldType(char *type, int size);
IFORMAT eformat_to_iformat(EFORMAT e);
void MakeBitFields(OBJDESC *col, FIELD *f, LIST *list);
FIELD * MakeBitField(OBJDESC *col, FIELD *f);
EFORMAT ConvertType(char *type);

/*Dummy stub needed for this "borrowed" code*/
DATA
ConvertASCIItoData(char *ascii, int i)
{
	DATA d;
	return(d);
}

/**
** LoadLabel() - Read and decode a PDS label, including individual fields.
 **/
LABEL *
LoadLabel(char *fname)
{
    OBJDESC *ob, *tbl, *col;
    KEYWORD *kw;
    FIELD *f;
    LIST *list;
    LABEL *l;
    int i;
    unsigned short scope;
    int reclen, nfields, nrows;
    char *name;

    ob = OdlParseLabelFile(fname, NULL, ODL_EXPAND_STRUCTURE, 1);
    if (ob == NULL) {
        parse_error("Unable to read file: %s", fname);
        return (NULL);
    }
    /* find the first (and only?) table object */
    if ((tbl = OdlFindObjDesc(ob,"TABLE",NULL,NULL, 0, ODL_TO_END)) == NULL) {
        parse_error("Unable to find TABLE object: %s", fname);
        parse_error("Is this a vanilla file?");
        return (NULL);
    }
    if ((kw = OdlFindKwd(tbl, "ROW_BYTES", NULL,0, ODL_THIS_OBJECT)) != NULL) {
        reclen = atoi(OdlGetKwdValue(kw));
    } else {
        parse_error("Unable to find keyword: ROW_BYTES: %s", fname);
        parse_error("Is this a vanilla file?  Is it's ^STRUCTURE file ok?");
        return (NULL);
    }

	 if ((kw = OdlFindKwd(tbl, "ROWS", NULL,0, ODL_THIS_OBJECT)) != NULL) {
		  nrows = atoi(OdlGetKwdValue(kw));
	 } else {
			parse_error("Unable to find keyword: ROWS: %s", fname);
			parse_error("Is this a vanilla file?  Is it's ^STRUCTURE file ok?");
			return (NULL);
    }


    if ((kw = OdlFindKwd(tbl, "COLUMNS", NULL, 0, ODL_THIS_OBJECT)) != NULL) {
        nfields = atoi(OdlGetKwdValue(kw));
    } else {
        parse_error("Unable to find keyword: COLUMNS");
        return (NULL);
    }


    if ((kw = OdlFindKwd(tbl, "NAME", NULL, 0, ODL_THIS_OBJECT)) != NULL) {
        name = OdlGetKwdValue(kw);
    }

    l = calloc(1, sizeof(LABEL));
    l->reclen = reclen;
    l->nfields = nfields;
    l->name = name;
	 l->nrows = nrows;

    /**
     ** get all the column descriptions
     **/

    list = new_list();
    i = 0;
    scope = ODL_CHILDREN_ONLY;
    col = tbl ; 
    while ((col = OdlNextObjDesc(col, 0, &scope)) != NULL) {
        if ((f = MakeField(col, l)) != NULL) {
			list_add(list, f);
			i++;

			/**
			 ** Fake up some additional fields for bit columns.
			 **/
			if (f->eformat == MSB_BIT_FIELD) {
				MakeBitFields(col, f, list);
			}
		}
    }

    if (i != nfields) {
        fprintf(stderr,
                "Wrong number of column definitions.  Expected %d, got %d.\n",
                i, nfields);
    }

    l->fields = list;

    return (l);
}

/**
 ** Convert a field description into a FIELD struct
 **/

FIELD *
MakeField(OBJDESC *col, LABEL *l)
{
    FIELD *f;
    VARDATA *vardata;
    KEYWORD *kw;
    int i = 0;
    char *ptr;
	
    do {
        f = (FIELD *) calloc(1, sizeof(FIELD));
        f->label = l;

        if ((kw = GetKey(col, "NAME")) == NULL) {
            parse_error("Column %d has no name.", i);
            break;
        }
        f->name = OdlGetKwdValue(kw);

		f->alias = NULL;
        if ((kw = GetKey(col, "ALIAS_NAME")) != NULL) {
			f->alias = OdlGetKwdValue(kw);
        }


        if ((kw = GetKey(col, "START_BYTE")) == NULL) {
            parse_error("Column %s: START_BYTE not specified.", f->name);
            break;
        }
        f->start = atoi(OdlGetKwdValue(kw)) - 1;

        if ((kw = GetKey(col, "BYTES")) == NULL) {
            parse_error("Column %s: BYTES not specified.", f->name);
            break;
        }
        f->size = atoi(OdlGetKwdValue(kw));


        if ((kw = GetKey(col, "ITEMS")) != NULL) {
            f->dimension = atoi(OdlGetKwdValue(kw));
            /**
            ** If BYTES was specified, this will overwrite the value
            ** with the indivdual element size, as we expect.
            **/
            if ((kw = GetKey(col, "ITEM_BYTES")) != NULL) {
                f->size = atoi(OdlGetKwdValue(kw));
            } else {
                parse_error("Column %s: ITEM_BYTES not specified, dividing BYTES by ITEMS.", f->name);
                f->size = f->size / f->dimension;
            }
        }

        if ((kw = GetKey(col, "DATA_TYPE")) == NULL) {
            parse_error("Column %s: DATA_TYPE not specified.", f->name);
            break;
        }
		f->type = OdlGetKwdValue(kw);
		if ((f->eformat = ConvertType(f->type)) == INVALID_EFORMAT) {
            parse_error("Unrecognized type: %s, %s %d bytes",
                    f->name, f->type, f->size);
			break;
		}

		f->iformat = eformat_to_iformat(f->eformat);

        if ((kw = GetKey(col, "SCALING_FACTOR")) != NULL) {
            f->scale = atof(OdlGetKwdValue(kw));
        }

        if ((kw = GetKey(col, "OFFSET")) != NULL) {
            f->offset = atof(OdlGetKwdValue(kw));
        }

        if ((kw = GetKey(col, "VAR_RECORD_TYPE")) != NULL) {
            ptr = OdlGetKwdValue(kw);
            vardata = f->vardata = calloc(1, sizeof(VARDATA));

            if (!strcmp(ptr, "VAX_VARIABLE_LENGTH")) vardata->type = VAX_VAR;
            else if (!strcmp(ptr, "Q15")) vardata->type = Q15;
            else {
                parse_error("Unrecognized VAR_DATA_TYPE: %s", ptr);
            }

            if ((kw = GetKey(col, "VAR_ITEM_BYTES")) != NULL) {
                vardata->size = atoi(OdlGetKwdValue(kw));
            
                if ((kw = GetKey(col, "VAR_DATA_TYPE")) != NULL) {
                    ptr = OdlGetKwdValue(kw);
                } else {
					parse_error("VAR_DATA_TYPE not specified for field: %s", 
								f->name);
					exit(1);
				}
                if ((vardata->eformat = ConvertType(ptr)) == INVALID_EFORMAT) {
                    parse_error("Unrecognized vartype: %s, %s %d bytes",
                            f->name, ptr, vardata->size);
                }
				vardata->iformat = eformat_to_iformat(vardata->eformat);
            } else {
				parse_error("VAR_ITEM_BYTES not specified for field: %s", 
							f->name);
				exit(1);
			}
        }

		if (f->eformat == BYTE_OFFSET) {
			f->eformat = MSB_INTEGER;
			f->iformat = eformat_to_iformat(f->eformat);
            vardata = f->vardata = calloc(1, sizeof(VARDATA));
			vardata->size = 2;
			vardata->eformat = MSB_INTEGER;
			vardata->iformat = VINT;
			vardata->type = Q15;
		}

		return(f);
    } while(0);

	return(NULL);
}

void
MakeBitFields(OBJDESC *col, FIELD *f, LIST *list)
{
    unsigned short scope;
	FIELD *b;

    scope = ODL_CHILDREN_ONLY;
    while ((col = OdlNextObjDesc(col, 0, &scope)) != NULL) {
        if ((b = MakeBitField(col, f)) != NULL) {
			list_add(list, b);
		}
    }
}

FIELD *
MakeBitField(OBJDESC *col, FIELD *f)
{
	FIELD *f2;
    KEYWORD *kw;
	BITFIELD *b;
	char name[256], *ptr;
	int i = 1;

	/**
	 ** Do this once, and allow for breaks
	 **/
	do {
        f2 = (FIELD *) calloc(1, sizeof(FIELD));
		*f2 = *f;

        b = (BITFIELD *) calloc(1, sizeof(BITFIELD));
		f2->bitfield = b;

        if ((kw = GetKey(col, "NAME")) == NULL) {
            parse_error("Bitfield %d has no name.", i);
            break;
        }
		sprintf(name, "%s:%s", f->name, OdlGetKwdValue(kw));
        f2->name = strdup(name);

        if ((kw = GetKey(col, "ALIAS_NAME")) != NULL) {
			sprintf(name, "%s:%s", f->name, OdlGetKwdValue(kw));
			f2->alias = strdup(name);
		}

        if ((kw = GetKey(col, "BIT_DATA_TYPE")) == NULL) {
            parse_error("Bitfield %s has no data type.", f2->name);
            break;
        }
		ptr = OdlGetKwdValue(kw);
		b->type = ConvertType(ptr); /* b->type contains the EFORMAT */
		f2->iformat = eformat_to_iformat(b->type); /* Saadat - I think! */

		if ((kw = GetKey(col, "START_BIT")) == NULL) {
            parse_error("Bitfield %s has no start bit.", f2->name);
            break;
        }
		b->start_bit = atoi(OdlGetKwdValue(kw));

		if ((kw = GetKey(col, "BITS")) == NULL) {
            parse_error("Bitfield %s has no BITS value.", f2->name);
            break;
        }
		b->bits = atoi(OdlGetKwdValue(kw));

		b->shifts = (f->size * 8) - b->start_bit - b->bits + 1;
		/* b->mask = (1 << b->bits) - 1; -- fails for bits=32 */
		b->mask = UINT_MAX;
		b->mask >>= -(b->bits - 32);

		return(f2);
	} while (0);

	return(NULL);
}

IFORMAT
eformat_to_iformat(EFORMAT e)
{
	switch(e) {
		case LSB_INTEGER:
		case MSB_INTEGER:
		case ASCII_INTEGER:
			return(VINT);

		case MSB_UNSIGNED_INTEGER:
		case LSB_UNSIGNED_INTEGER:
		case MSB_BIT_FIELD:
		case LSB_BIT_FIELD:
		case BYTE_OFFSET:
			return(UVINT);

		case IEEE_REAL:
		case PC_REAL:
		case ASCII_REAL:
			return ( REAL );

		case CHARACTER:
			return ( STRING );

		default:
			fprintf(stderr, "Unrecognized etype: %d\n", e);
	}

	return INVALID_IFORMAT;
}

EFORMAT
ConvertType(char *type) 
{
    if (!strcasecmp(type, "MSB_INTEGER") ||
		!strcasecmp(type, "SUN_INTEGER") ||
		!strcasecmp(type, "MAC_INTEGER") ||
		!strcasecmp(type, "INTEGER")) {
			return(MSB_INTEGER);
    } else if (!strcasecmp(type, "MSB_UNSIGNED_INTEGER") ||
               !strcasecmp(type, "SUN_UNSIGNED_INTEGER") || 
               !strcasecmp(type, "MAC_UNSIGNED_INTEGER") || 
               !strcasecmp(type, "UNSIGNED_INTEGER")) {
			return(MSB_UNSIGNED_INTEGER);
    } else if (!strcasecmp(type, "IEEE_REAL") ||
               !strcasecmp(type, "SUN_REAL") ||
               !strcasecmp(type, "MAC_REAL") ||
               !strcasecmp(type, "REAL")) {
			return(IEEE_REAL);
    } else if (!strcasecmp(type, "PC_REAL")) {
		return(PC_REAL);
    } else if (!strcasecmp(type, "CHARACTER")) {
		return(CHARACTER);
    } else if (!strcasecmp(type, "ASCII_INTEGER")) {
		return(ASCII_INTEGER);
    } else if (!strcasecmp(type, "ASCII_REAL")) {
		return(ASCII_REAL);
    } else if (!strcasecmp(type, "BYTE_OFFSET")) {
		return(BYTE_OFFSET);
    } else if (!strcasecmp(type, "MSB_BIT_STRING")) {
		return(MSB_BIT_FIELD);
    } else if (!strcasecmp(type, "LSB_BIT_STRING")) {
		return(LSB_BIT_FIELD);
    } else if (!strcasecmp(type, "LSB_INTEGER") ||
               !strcasecmp(type, "PC_INTEGER") || 
               !strcasecmp(type, "VAX_INTEGER")) {
			return(LSB_INTEGER);
    } else if (!strcasecmp(type, "LSB_UNSIGNED_INTEGER") ||
               !strcasecmp(type, "PC_UNSIGNED_INTEGER") || 
               !strcasecmp(type, "VAX_UNSIGNED_INTEGER")) {
			return(LSB_UNSIGNED_INTEGER);
	}
	return(INVALID_EFORMAT);
}
 
/**
 ** Given a field name, locate it in the list of labels.
 **/
FIELD *
FindField(char *name, LIST *tables)
{
    char buf[256];
    char *p;
    char *field_name;
    char *label_name;
    int i;
	TABLE *t;
    LABEL *l;
    FIELD *f;

    strcpy(buf, name);

    if ((p = strchr(buf, '.')) != NULL) {
        *p = '\0';
        label_name = buf;
        field_name = p + 1;
    } else {
        label_name = NULL;
        field_name = buf;
    }

    /**
    ** If this field name includes a dimension, get rid of it
    **/
    if ((p = strchr(field_name, '[')) != NULL) {
        *p = '\0';
    }
    for (i = 0; i < tables->number; i++) {
        t = (tables->ptr)[i];
		l = t->label;
        /*
        ** If the user told us what struct the field is in, skip all others
        */
        if (label_name && strcasecmp(label_name, l->name))
            continue;

        if ((f = FindFieldInLabel(field_name, l)) != NULL)  {
            return(f);
        }
    }

    return (NULL);
}

FIELD *
FindFieldInLabel(char *name, LABEL * l)
{
    int i;
    FIELD **f = (FIELD **) l->fields->ptr;
    int nfields = l->fields->number;

    for (i = 0; i < nfields; i++) {
        if (!strcasecmp(name, f[i]->name)) {
            return (f[i]);
        }
        if (f[i]->alias && !strcasecmp(name, f[i]->alias)) {
            return (f[i]);
        }
    }
    return (NULL);
}


/**
 ** Load the header values specific to an individual file
 **/

FRAGMENT *
LoadFragment(char *fname, TABLE *table)
{
    OBJDESC *ob, *tbl;
    KEYWORD *kw;
    LIST *startlist=NULL, *endlist=NULL;
    FRAGMENT *f;
    int rows;
    int offset;
    struct stat sbuf;

    if (stat(fname, &sbuf) == -1) {
        parse_error("Unable to find file: %s", fname);
        return(NULL);
    }
    
    ob = OdlParseLabelFile(fname, NULL, ODL_EXPAND_STRUCTURE, 1);
    if (ob == NULL) {
        parse_error("Unable to read file: %s", fname);
        return (NULL);
    }
    
    if ((kw = OdlFindKwd(ob, "^TABLE", NULL, 0, ODL_THIS_OBJECT)) != NULL) {
        offset = atoi(OdlGetKwdValue(kw));
    } else {
        parse_error("Unable to find table pointer (^TABLE) in: %s",
                fname);
        return (NULL);
    }


    /* find the first (and only?) table object */
    if ((tbl = OdlFindObjDesc(ob, "TABLE", NULL, NULL, 0, ODL_TO_END)) == NULL) {
        parse_error("Unable to find TABLE object: %s", fname);
        return (NULL);
    }

    if ((kw = OdlFindKwd(tbl, "ROWS", NULL,0, ODL_THIS_OBJECT)) != NULL) {
        rows = atoi(OdlGetKwdValue(kw));
    } else {
        parse_error("Unable to find keyword ROWS: %s", fname);
        return (NULL);
    }
    
    if ((kw = GetKey(tbl, "START_KEYS")) != NULL || 
		(kw = GetKey(tbl, "START_PRIMARY_KEY")) != NULL) {
        int i,j;
        char **array;
        DATA *dataval;
        FIELD **keys = (FIELD **)table->label->keys->ptr;

        i = OdlGetAllKwdValuesArray(kw, &array);

        startlist = new_list();
        dataval = calloc(i, sizeof(DATA));

        for (j = 0; j < i; j++) {
            dataval[j] = ConvertASCIItoData(array[j], keys[j]->iformat);
            list_add(startlist, &dataval[j]);
        }
    } else if ((kw = GetKey(tbl, "START_KEY")) != NULL) {
        DATA *dataval;
        FIELD **keys = (FIELD **)table->label->keys->ptr;

        startlist = new_list();
        dataval = calloc(1, sizeof(DATA));
		dataval[0] = ConvertASCIItoData(OdlGetKwdValue(kw), keys[0]->iformat);
		list_add(startlist, &dataval[0]);
	}

    if ((kw = GetKey(tbl, "END_KEYS")) != NULL || 
        (kw = GetKey(tbl, "STOP_PRIMARY_KEY")) != NULL) {
        int i,j;
        char **array;
        DATA *dataval;
        FIELD **keys = (FIELD **)table->label->keys->ptr;

        i = OdlGetAllKwdValuesArray(kw, &array);

        endlist = new_list();
        dataval = calloc(i, sizeof(DATA));

        for (j = 0; j < i; j++) {
            dataval[j] = ConvertASCIItoData(array[j], keys[j]->iformat);
            list_add(endlist, &dataval[j]);
        }
    } else if ((kw = GetKey(tbl, "STOP_KEY")) != NULL) {
        DATA *dataval;
        FIELD **keys = (FIELD **)table->label->keys->ptr;

        endlist = new_list();
        dataval = calloc(1, sizeof(DATA));
		dataval[0] = ConvertASCIItoData(OdlGetKwdValue(kw), keys[0]->iformat);
		list_add(endlist, &dataval[0]);
	}

    f = calloc(1, sizeof(FRAGMENT));

    /**
     ** This is the all important ^PTR conversion
     **/
    f->offset = (offset - 1) * table->label->reclen;
    f->nrows = rows;
    f->start_keys = startlist;
    f->end_keys = endlist;
    f->sbuf = sbuf;

    if (sbuf.st_size != f->offset + f->nrows * table->label->reclen) {
        parse_error("File is an odd size: %s", fname);
    }

    return(f);
}

void
FreeFragment(FRAGMENT *f)
{
	list_free(f->start_keys);
	list_free(f->end_keys);
	free(f);
}

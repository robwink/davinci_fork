#include "parser.h"
#include "func.h"
#include <sys/stat.h>


#if 0
#define swap(a,b)  { char t = a; a = b; b = t; }
#ifndef WORDS_BIGENDIAN
#define MSB8(a) msb8((char *)(&(a)))
#define MSB4(a) msb4((char *)(&(a)))
#define MSB2(a) msb2((char *)(&(a)))
#define MSB4n(a,n) { for (int i = 0 ; i < n ; i++) { MSB4(a[i]); } }
#else
#define MSB8(a)
#define MSB4(a)
#define MSB2(a)
#define MSB4n(a,n)
#endif
static void msb8(char *a)
{
  swap(a[0],a[7]);
  swap(a[1],a[6]);
  swap(a[2],a[5]);
  swap(a[3],a[4]);
}
static void msb4(char *a)
{
  swap(a[0],a[3]);
  swap(a[1],a[2]);
}

static void msb2(char *a)
{
  swap(a[0],a[1]);
}
#endif


/*
 ** File format:
 ** 		short hdr_size;
 char hdr[hdr_size];

 rec_hdr

*/
#ifdef WORDS_BIGENDIAN

#define MSB8(s)         (s)
#define MSB4(s)         (s)
#define MSB2(s)         (s)

#else /* little endian */

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

#endif /* WORDS_BIGENDIAN */



typedef union rechdr
{
  struct rechdr_info
  {
    unsigned char  size_lsb;       /* LSByte of size of record */
    unsigned char  size_msb;       /* MSByte of size of record */
    unsigned char  rec_type;       /* type of record (used, free, last) */
    unsigned short n_bytes;        /* number of bytes of information */
  } i;

  double alignment[2];

} RECHDR;

typedef struct cache_hdr
{
  unsigned short     df_sym_loc;             /* Where is disk sym tab located */
  unsigned short     n_df_syms;              /* # of sym's in disk sym tab */
  int        whichscope;
} CACHE_HDR;

#define MAXSYMNAME      35              /* # of chars stored in disk sym tab */
#define MAXDIM           3              /* */

typedef struct df_sym                   /* Disk sym tab structure */
{
  char       name[MAXSYMNAME+1];     /* max of 35 significant chars */
  int        type;
  int        dim;                    /* dimension */
  int        dim_size[MAXDIM];       /* dimension values */
  int        loc;                    /* where is this item located */
  int        size;                   /* how big am i */
} DF_SYM;

#define T_CHAR       0x0001     /* 8 bit signed char */
#define T_INTEGER    0x0002     /* 32 bit signed int */
#define T_FLOAT      0x0003     /* 32 bit IEEE float */
#define T_DOUBLE     0x0004     /* 64 bit IEEE double */
#define T_STRING     0x0005     /* Null terminated list of 8 bit chars */
#define T_BIT        0x0006     /* 1 bit */


#define STREQ(a,b)              (strcmp((a), (b)) == 0)

static char *
find_next_nonwhite(char *line)
{
  while (*line && isspace(*line)) {
    line++;
  }

  return (*line) ? line : 0;
}

static char *
copy_nonwhite(char *from, char *to, int tolen)
{
  tolen--;

  if (*from == '"') {
    from++;

    /* Double quoted string.. copy until close quote */
    while (*from != 0  &&  *from != '"') {
      if (tolen) {
        *to++ = *from;
        tolen--;
      }

      from++;
    }
  } else if (*from == '\'') {
    from++;

    /* Single quoted string.. copy until close quote */
    while (*from != 0  &&  *from != '\'') {
      if (tolen) {
        *to++ = *from;
        tolen--;
      }

      from++;
    }
  } else {
    /* Regular string... copy until whitespace or end of buffer */
    while (*from != 0  &&  !isspace(*from)) {
      if (tolen) {
        *to++ = *from;
        tolen--;
      }

      from++;
    }
  }

  *to = (char) 0;

  return (*from) ? from : 0;
}


  static char *
grab_keyval(char *line, char *key, char *val, int keylen, int vallen)
{
  *key = (char) 0;
  *val = (char) 0;

  /* Find keyword */
  if ((line = find_next_nonwhite(line)) == 0)
    return 0;

  /* Copy keyword */
  line = copy_nonwhite(line, key, keylen);

  /* Find assignment */
  if ((line = find_next_nonwhite(line)) == 0)
    return 0;

  /* Check that assignment character is there */
  if (*line != '=')
    return 0;

  /* Find value */
  if ((line = find_next_nonwhite(line+1)) == 0)
    return 0;

  /* Copy value */
  line = copy_nonwhite(line, val, vallen);

  return line;
}


  static int 
parse_label(char *hdr, char ***scopes, int *nscopes) 
{
  char *label = hdr;
#define KEYLEN  20
#define VALLEN  20
  char       key[KEYLEN];            /* keyword from each line */
  char       val[VALLEN];            /* value from each line */

  int count = 0;
  int size = 0;
  char **s;


  while ((label = grab_keyval(label, key, val, KEYLEN, VALLEN)) != 0) {
    if (STREQ(key, "OBJECT") && STREQ(val, "TABLE")) {
      do {
        label = grab_keyval(label, key, val, KEYLEN, VALLEN);
        if (label == 0) {
          return 0;
        }

        if (STREQ(key, "NAME")) {
          if (count == size) {
            if (size == 0) {
              size = 16;
              s = calloc(16, sizeof(char *));
            } else {
              size *= 2;
              s = realloc(s, size*sizeof(char *));
            }
          }
          s[count++] = strdup(val);
        }

      } while (!STREQ(key, "END_OBJECT") && !STREQ(val, "TABLE"));
    }
  }
  *scopes = s;
  *nscopes = count;
  return(1);
}

/*

   TDB files have the following format:

   msb_ushort header_size 		- Size of the ascii header 
   char header[header_size]	- Text header.  Useful to know scope names

   [[ 
   Record Header {			// one per record
   msb_ushort record_size	- bytes until next record
   char rec_type			- 
   msb_ushort size;		- 
   padding					- Padded out to a multiple of 8 bytes
   (via a union with double[2])
   }
   Cache Header {			// one per record
   short     sym_loc;      - Where is disk sym tab located
   ushort     n_df_syms;   - # of sym's in disk sym tab
   int        whichscope;  - index of scope name (from the ascii header)
   }
   ...  Data ...
   [[ 	Symbol {		// repeated for every symbol in record
   char       name[36];    - Name of symbol
   int        type;		- type of symbol (enum)
   int        dim;         - How many dimensions
   int        dim_size[3]; - dimension values (only first one is used)
   int        loc;         - Bytes after cache_hdr where this data is
   int        size;        - How many bytes is each quantum
   }]]
   ]]

   Fortunately, every record has the name, type and dimensions repeated,
   So we can decode this file without any external references

*/

  Var *
LoadTDB(char *filename)
{
  FILE *fp;
  short hdr_len;
  char *hdr;
  Var *out;
  RECHDR *r;
  CACHE_HDR *c;
  DF_SYM *d;
  char *data;
  short len;
  int count;
  int i, j;
  int hdr_size = sizeof(CACHE_HDR);
  Var *member, *v1;

  char **scopes;
  char name[256];
  int nscopes;

  if ((fp = fopen(filename, "r")) == NULL) {
    parse_error("Unable to open file: %s\n", filename);
    return(NULL);
  }

  if (fread(&hdr_len, sizeof(short), 1, fp) != 1) {
    parse_error("Error reading file size\n");
    return(NULL);
  }
  hdr = calloc(hdr_len+1, 1);

  if (fread(hdr, 1, hdr_len, fp) != hdr_len) {
    parse_error("Error reading file header\n");
    return(NULL);
  }

  parse_label(hdr, &scopes, &nscopes);

  count = 0;
  out = new_struct(1);
  r = calloc(1, sizeof(RECHDR));
  while(1) {
    if (fread(r, 1, sizeof(RECHDR), fp) != sizeof(RECHDR)) {
      /* We're probably out of data now */
      break;
    }
    len = (r->i.size_msb << 8 | r->i.size_lsb) - sizeof(RECHDR);
    data = calloc(len,1);

    if (fread(data, 1, len, fp) != len) {
      /* We're probably out of data now */
      fprintf(stderr, "Short file.\n");
      free(data);
      break;
    }
    c = (CACHE_HDR *)data;
    (void) MSB2(&c->df_sym_loc);
    (void) MSB2(&c->n_df_syms);
    (void) MSB4(&c->whichscope);

    if (c->n_df_syms) {
      member = new_struct(c->n_df_syms);
      add_struct(member, "scope", 
                 newString(strdup(scopes[c->whichscope])));
      for (i = 0 ; i < c->n_df_syms ; i++) {
        d = (data+c->df_sym_loc+(i * sizeof(DF_SYM)));
        (void) MSB4(&d->type);
        (void) MSB4(&d->dim);
        for (j = 0 ; j < d->dim ; j++) {
          (void) MSB4(&d->dim_size[j]);
        }
        (void) MSB4(&d->loc);
        (void) MSB4(&d->size);

        v1 = NULL;
        switch(d->type) {
          case T_STRING: {
                           int *len = data+(d->loc+hdr_size);
                           char *str = len+1;
                           v1 = newString(strdup(str));
                           break;
                         }
          case T_DOUBLE: {
                           double *dbl;
                           int size = 1;
                           for (j = 0 ; j < d->dim ; j++) {
                             size *= (d->dim_size[j] ? d->dim_size[j] : 1);
                           }
                           dbl = calloc(sizeof(double), size);
                           memcpy(dbl, data +(d->loc+hdr_size), size*sizeof(double));
                           for (j = 0 ; j < size ; j++) {
                             (void)MSB8(&dbl[j]);
                           }
                           v1 = newVal(BSQ, 
                                       d->dim_size[0] ? d->dim_size[0] : 1,
                                       d->dim_size[1] ? d->dim_size[1] : 1,
                                       d->dim_size[2] ? d->dim_size[2] : 1,
                                       DOUBLE, dbl);
                           break;
                         }
          case T_FLOAT:
          case T_INTEGER:
          default:
                         fprintf(stderr, "Unrecognized type: %d\n", d->type);
                         break;
        }
        if (v1) add_struct(member, d->name, v1);
      }
      free(data);
      sprintf(name, "v[%d]", count+1);
      add_struct(out, name, member);
      count++;
    }
  }
  free(scopes);
  return(out);
}

static Var * collect_scopes(Var *data);
static Var * collect_values(Var *data, Var *scopes, Var *keys);
static Var * collect_keys(Var *data, Var *scopes) ;
static Var * distribute_xaxis(Var *data);

  Var *
ff_load_tdb(vfuncptr func, Var *arg)
{
  char *fname, *filename;
  int reform = 0;
  int distribute = 0;
  Var *out;

  Alist   alist[4];
  /* make arguments list */
  alist[0] = make_alist("filename", ID_STRING, NULL, &fname);
  alist[1] = make_alist("reform", INT, NULL, &reform);
  alist[2] = make_alist("distribute", INT, NULL, &distribute);
  alist[3].name = NULL;

  /* process arguments */
  if (parse_args(func, arg, alist) == 0){ return NULL; }
  if (fname == NULL){
    parse_error("%s: filename not specified\n", func->name);
    return NULL;
  }

  /* get bin5 file header */
  if ((filename = dv_locate_file(fname)) == NULL) {
    filename = fname;
  }
  out = LoadTDB(filename);

  if (out && distribute) {
    distribute_xaxis(out);
  }

  if (out && reform) {
    Var *scopes, *keys, *values;
    scopes = collect_scopes(out);
    keys = collect_keys(out, scopes);
    values = collect_values(out, scopes, keys);
    return(values);
  } else {
    return(out);
  }
}



  static Var *
collect_scopes(Var *data) 
{
  Var *scopes = new_struct(5);
  Var *v1, *v2;
  char *key;
  int i;

  for (i = 0 ; i < get_struct_count(data) ; i++) {
    get_struct_element(data, i, &key, &v1);
    if (find_struct(v1, "scope", &v2) != -1) {
      if (find_struct(scopes, V_STRING(v2), NULL) == -1) {
        add_struct(scopes, V_STRING(v2), newInt(1));
      }
    }
  }
  return(scopes);
}

  static Var *
collect_keys(Var *data, Var *scopes) 
{
  int i, j, k;
  Var *scope;
  Var *v1, *v2;
  char *key, *scope_name, *key_name;
  Var *keys;

  Var *out = new_struct(1);
  for (j = 0 ; j < get_struct_count(scopes) ; j++) {
    get_struct_element(scopes, j, &scope_name, &scope);

    keys = new_struct(2);
    for (i = 0 ; i < get_struct_count(data) ; i++) {
      /* Make sure this element is of the right scope */
      get_struct_element(data, i, &key_name, &v1);
      if (find_struct(v1, "scope", &v2) != -1 &&
          !strcmp(V_STRING(v2), scope_name)) {

        for (k = 0 ; k < get_struct_count(v1) ; k++) {
          get_struct_element(v1, k, &key, &v2);
          if (find_struct(keys, key, NULL) == -1) {
            add_struct(keys, key, newInt(1));
          }
        }
      }
    }
    add_struct(out, scope_name, keys);
  }
  return(out);
}

  static Var * 
collect_values(Var *data, Var *scopes, Var *keys) 
{
  int i, j, k;
  char *scope_name, *key_name;
  Var *scope, *key, *v1, *v2, *s;
  int count;
  char name[512];
  Var *out, *outscope, *vals;

  out = new_struct(1);
  for (i = 0 ; i < get_struct_count(keys) ; i++) {
    /* get scope from keys */
    get_struct_element(keys, i, &scope_name, &scope);
    outscope = new_struct(1);

    for (j = 0 ; j < get_struct_count(scope) ; j++) {
      /* get an individual key from this scope */
      get_struct_element(scope, j, &key_name, &key);
      if (!strcmp(key_name, "scope")) continue;

      /* Get this key from each record that is of the same scope type */
      vals = new_struct(1);
      count = 1;
      for (k = 0 ; k < get_struct_count(data) ; k++) {
        get_struct_element(data, k, NULL, &s);
        find_struct(s, "scope", &v2);
        if (!strcmp(V_STRING(v2), scope_name)) {
          sprintf(name, "%s[%d]", key_name, count++);
          if (find_struct(s, key_name, &v1) != -1) {
            add_struct(vals, name, V_DUP(v1));
          } else {
            add_struct(vals, name, newString(strdup("")));
          }
        }
      }
      add_struct(outscope, key_name, vals);
    }
    add_struct(out, scope_name, outscope);
  }
  return(out);
}

  static Var *
distribute_xaxis(Var *data) 
{
  /* find struct members of type "Double" */
  /* find their associated "xname" member */
  /* look up struct members of type Xaxis that have name "xname" */
  int mpos, dpos; 
  int i, j;
  Var *v1, *v2, *xname, *xmember, *name, *member, *dval;
  int len;

  for (i = 0 ; i < get_struct_count(data) ; i++) {
    get_struct_element(data, i, NULL, &member);
    find_struct(member, "scope", &v1);
    if (strcmp(V_STRING(v1), "Double") != 0) {
      continue;
    }

    if ((mpos = find_struct(member, "xdata", &xname)) == -1) continue;
    if (strlen(V_STRING(xname)) > 0) {
      for (j = 0 ; j < get_struct_count(data) ; j++) {
        get_struct_element(data, j, NULL, &xmember);
        find_struct(xmember, "scope", &v2);
        if (strcmp(V_STRING(v2), "Xaxis") != 0) {
          continue;
        }
        if (find_struct(xmember, "name", &name) != -1 &&
            !strcmp(V_STRING(xname), V_STRING(name))) {

          free_var(remove_struct(member, mpos));
          dpos = find_struct(xmember, "data", &dval);
          add_struct(member, "xaxis", V_DUP(dval));
          break;
        }
      }
    } else {
      /* WTF?  An existant but empty xname? */
      free_var(remove_struct(member, mpos));
    }
  }
  /* Ok, we've used all the xaxis.  Take them out */
  /* done in reverse order so removal doesn't affect position */
  for (i = get_struct_count(data)-1 ; i >= 0 ; i--) {
    get_struct_element(data, i, NULL, &member);
    find_struct(member, "scope", &v1);
    if (!strcmp(V_STRING(v1), "Xaxis")) {
      free_var(remove_struct(data, i));
    }
  }
  /* And now change all the var names */
  len = get_struct_count(data);
  for (i = 0 ; i < len ; i++) {
    char name[512];
    v1 = remove_struct(data, 0);
    sprintf(name, "v[%d]", i+1);
    add_struct(data, name, v1);
  }

  return(data);
}

//#include "config.h"
#ifdef __cplusplus
extern "C" char *readline(char *);
extern "C" void *add_history(char *);
extern "C" void rl_callback_handler_install(char *, void (*)(char *));
extern "C" struct _hist_state* history_get_history_state(void);
extern "C" void rl_callback_read_char ();
extern "C" int yywrap ( void );
#endif

struct yy_buffer_state;
void yy_delete_buffer ( struct yy_buffer_state *b );
struct yy_buffer_state *yy_scan_string( const char *yy_str );
void yy_switch_to_buffer ( struct yy_buffer_state *new_buffer);


void yyerror(char *s);
int yyparse(int, Var *);

void log_line(char *);
int yylex(void);

void squash_spaces(char *s);
int instring(char *str, char c);
int check_ufunc(Var *v);
void save_function();
void eat_em(void);
int dd_put_argv(Scope *s, Var *v);
void unput_nextc(char c);
int send_to_plot(char *);
char *do_help(char *input);

void parse_stream(FILE *fp);

/* pp.c */
void emit_prompt ();                    /* spit out prompt if interactive */
Var *pp_math (Var *, int , Var *);      /* perform math */
Var *pp_relop (Var *, int, Var *);      /* perform math */
Var *pp_set_var (Var *, Var *, Var *);  /* set variables */
Var *pp_range (Var *, Var *);           /* perform specified subset */
Var *pp_func (Var *, Var *);            /* call function with arglist */
Var *pp_mk_attrib (Var *, Var *);       /* convert ID to ID.attrib */
Var *pp_mk_arglist (Var *, Var *);      /* append arg to arglist */
Var *pp_keyword_to_arg (Var *, Var *);  /* convert keyword pair to arg */
Var *pp_add_range (Var *, Var *);       /* add range to rangelist */
Var *pp_mk_range (Var *, Var *);        /* make range with given 'from' value */
Var *pp_to_range (Var *, Var *);        /* append given 'to' value to range */
Var *pp_get_att (Var *, Var *, Var *);  /* get value of attribute */
Var *pp_parens (Var *);                 /* do parens processing (set hist) */
Var *pp_set_att (Var *, Var *, Var *, Var *);   /* set value of attribute */
Var *pp_set_where(Var *, Var *, Var *);
Var *pp_shellArgs(Var *);               /* find shell command line args */
Var *pp_argv(Var *, Var *);
Var *pp_mk_rstep(Var *r1, Var *r2);		/* add a step value to ranges */
Var *pp_help(Var *);
Var *pp_usage(Var *);
void pp_set_cvar (Var *, Var *);	/* set control variable */
Var *pp_get_cvar (char *);		/* get control variable */
Var * pp_shell(char *cmd);

Var *V_DUP (Var *);
Var *set_array (Var *, Var *, Var *);
Var *extract_array (Var *, Range *);

/* symbol.c */
Var *get_sym (char *name);	/* retrieve named Sym from table */
Var *put_sym (Var *);
Var *eval (Var *);
Var * get_global_sym(char *);
Var * put_global_sym(Var *);


/* rpos.c */
int __BSQ2BSQ (Var * s1, Var * s2, int i);
int __BSQ2BIL (Var * s1, Var * s2, int i);
int __BSQ2BIP (Var * s1, Var * s2, int i);
int __BIL2BSQ (Var * s1, Var * s2, int i);
int __BIL2BIL (Var * s1, Var * s2, int i);
int __BIL2BIP (Var * s1, Var * s2, int i);
int __BIP2BSQ (Var * s1, Var * s2, int i);
int __BIP2BIL (Var * s1, Var * s2, int i);
int __BIP2BIP (Var * s1, Var * s2, int i);
int cpos(int x, int y, int z, Var *v);

/* pp_math.h */
int extract_int (Var * v, int i);
float extract_float (Var * v, int i);
double extract_double (Var * v, int i);
Var *pp_add_strings(Var *a, Var *b);
Var *pp_math_strings(Var *a, int op, Var *b);

/* file.h */
void push_input_file (char *name);
void pop_input_file ();
int is_file (char *name);

/* error.c */
void parse_error(char *, ...);

/* reserved.c */
int is_reserved_var (char *);
Var *set_reserved_var (Var *, Var *, Var *);

/* ff.c */

char *unquote(char *);

int evaluate_keywords (vfuncptr, Var *, struct keywords *);
Var *get_kw (char *, struct keywords *);
Var *verify_single_arg(vfuncptr, Var *);
Var *verify_single_string(vfuncptr , Var *);
int KwToInt(char *, struct keywords *, int *);
int KwToFloat(char *, struct keywords *, float *);

/* vicar.h */
char *get_value (char *, char *);
int GetVicarHeader (FILE *, struct _iheader *);
int GetAVIRISHeader (FILE *, struct _iheader *);
Var *iheader2var (struct _iheader *);

/* read.c */
void *read_qube_data(int, struct _iheader *);

Var *LoadSpecpr(FILE *,char *,int );
Var *LoadVicar (FILE *, char *, struct _iheader *);
Var *LoadISIS (FILE *, char *, struct _iheader *);
Var *LoadGRD(FILE *,char *, struct _iheader *);
Var *LoadPNM(FILE *, char *, struct _iheader *);
Var *LoadGOES (FILE *, char *, struct _iheader *);
Var *LoadAVIRIS (FILE *, char *, struct _iheader *);
Var *Load_imath (FILE *, char *, struct _iheader *);
#ifdef HAVE_LIBMAGICK
Var *LoadGFX_Image(char *filename);
Var *ff_XImage_Display(vfuncptr func, Var * arg);
#endif
int LoadISISHeader(FILE *fp, char *filename, int rec, char *element, Var **var);

int WriteRaw(Var *, FILE *, char *);
int WriteGRD(Var *, FILE *, char *);
int WriteSpecpr(Var *, char *, char *);
int WriteISIS(Var *, FILE *, char *, char *);
void WritePPM(Var *, FILE *, char *);
void WritePGM(Var *, FILE *, char *);
int WriteVicar(Var *, FILE *, char *);
int WriteAscii(Var *, FILE *, char *);
int WriteERS(Var *, FILE *, char *);
int WriteIMath(Var *s, FILE *fp, char *filename);
#ifdef HAVE_LIBMAGICK
void iriieGFX_Image(Var *ob,char *filename,char *GFX_type);
#endif
int is_AVIRIS(FILE *);
int is_GRD(FILE *);
int is_Vicar(FILE *);
int is_compressed(FILE *);
int is_ISIS(FILE *);
int is_specpr(FILE *);
char *is_PNM(FILE *, char *);
int is_imath(FILE *);
FILE *uncompress(FILE *, char *);


char *locate_file(char *);

Var *p_mknod();
Var *p_mkval(int, char *);
Var *evaluate(Var *);
Var * p_rnode(Var *, Var *);
Var * p_lnode(Var *, Var *);
Var * p_rlist(int , Var *, Var *);
Var * p_llist(int , Var *, Var *);


void push_input_stream(FILE *);
int rpos(int, Var *, Var *);
Var * pp_print(Var *);

Var *V_func (char *name, Var *);
void make_sym(Var *, int, char *);


char *get_env_var(char *);
char *expand_filename(char *);
char *enumerated_arg(Var *, char **);
Var *mem_claim(Var *);
Var *mem_malloc(void);
void free_var(Var *);
void commaize(char *);
void free_tree(Var *);
Var *rm_symtab(Var *);

int LoadSpecprHeader(FILE *, char *, int , char *, Var **);
int LoadVicarHeader(FILE *, char *, int , char *, Var **);

Var *RequireKeyword(char *, struct keywords *, int, int, vfuncptr );
Var *HasValue(vfuncptr, Var *);
Var *ufunc_edit(vfuncptr, Var *);


int fixup_ranges(Var *v, Range *in, Range *out);
void split_string(char *buf, int *argc, char ***argv, char *s);
int getline(char **ptr, FILE *fp);

/**
 ** All the internal functions are declared here.
 **/

Var *ff_dfunc (vfuncptr, Var *);
Var *ff_pow (vfuncptr, Var *);
Var *ff_conv (vfuncptr, Var *);
Var *ff_dim (vfuncptr, Var *);
Var *ff_format (vfuncptr, Var *);
Var *ff_org (vfuncptr, Var *);
Var *ff_create (vfuncptr, Var *);
Var *ff_source (vfuncptr, Var *);
Var *ff_load (vfuncptr, Var *);
Var *ff_Frame_Grabber_Read(vfuncptr func, Var * arg);
Var *ff_GSE_VIS_Read(vfuncptr func, Var * arg);
Var *ff_write (vfuncptr, Var *);
Var *ff_filetype(vfuncptr , Var *);
Var *ff_list(vfuncptr, Var *);
Var *ff_atoi(vfuncptr func, Var *arg);
Var *ff_atof(vfuncptr func, Var *arg);
Var *ff_sprintf(vfuncptr func, Var *arg);
Var *ff_version(vfuncptr, Var *);
Var *ff_random(vfuncptr, Var *);
Var *ff_gnoise(vfuncptr func, Var *arg);
Var *ff_cluster(vfuncptr func, Var *arg);
Var *ff_ccount(vfuncptr func, Var *arg);
Var *ff_nop(vfuncptr func, Var *arg);
Var *ff_echo(vfuncptr func, Var *arg);
Var *ff_basename(vfuncptr func, Var *arg);
Var *ff_rgb(vfuncptr func, Var *arg);
Var *ff_replicate(vfuncptr func, Var *arg);
Var *ff_cat(vfuncptr func, Var *arg);
Var *do_cat(Var *, Var *, int);
Var *ff_ascii(vfuncptr, Var *);
Var *ff_text(vfuncptr, Var *);
Var *ff_string(vfuncptr, Var *);
Var *ff_delim(vfuncptr, Var *);
Var *ff_translate(vfuncptr, Var *);
Var *ff_gplot(vfuncptr, Var *);
Var *ff_plot(vfuncptr, Var *);
Var *ff_splot(vfuncptr, Var *);
Var *ff_display(vfuncptr, Var *);
Var *ff_moment(vfuncptr, Var *);
Var *ff_interp(vfuncptr, Var *);
Var *ff_fit(vfuncptr, Var *);
Var *ff_header(vfuncptr func, Var *);
Var *ff_bbr(vfuncptr func, Var *);
Var *ff_bbrf(vfuncptr func, Var *);
Var *ff_vignette(vfuncptr func, Var *);
Var *ff_pause(vfuncptr func, Var *);
Var *ff_btemp(vfuncptr func, Var *);
Var *ff_printf(vfuncptr func, Var *);
Var *ff_sprintf(vfuncptr func, Var *);
Var *ff_system(vfuncptr func, Var *);
Var *ff_ifill(vfuncptr func, Var *arg);
Var *ff_jfill(vfuncptr func, Var *arg);
Var *ff_pfill(vfuncptr func, Var *arg);
Var *ff_bop(vfuncptr func, Var *arg);
Var *ff_avg(vfuncptr func, Var *arg);
Var *ff_basis(vfuncptr func, Var *arg);
Var *ff_mxm(vfuncptr func, Var *arg);
Var *ff_histogram(vfuncptr func, Var *arg);
Var *ff_exit(vfuncptr func, Var *arg);
Var *ff_fsize(vfuncptr func, Var *arg);
Var *ff_history(vfuncptr func, Var *arg);
Var *ff_hedit(vfuncptr func, Var *arg);
Var *ff_sort(vfuncptr func, Var *arg);
Var *ff_min(vfuncptr func, Var *arg);
Var *ff_findmin(vfuncptr func, Var *arg);
Var *ff_rgb2hsv(vfuncptr func, Var *arg);
Var *ff_hsv2rgb(vfuncptr func, Var *arg);
Var *ff_resize(vfuncptr func, Var *arg);

//Var *ff_isis_summary(vfuncptr func, Var *arg);
Var *ff_read_suffix_plane(vfuncptr func, Var * arg);
Var *ff_fork(vfuncptr func, Var *arg);
Var *ff_xrt3d(vfuncptr func, Var *arg);
Var *ff_fft(vfuncptr func, Var *arg);
Var *ff_realfft(vfuncptr func, Var *arg);
Var *ff_realfft2(vfuncptr func, Var *arg);
Var *ff_realfft3(vfuncptr func, Var *arg);
Var *ff_minvert(vfuncptr func, Var *arg);
Var *ff_dct(vfuncptr func, Var *arg);
Var *ff_entropy(vfuncptr func, Var *arg);
#ifdef HAVE_LIBPROJ
Var *ff_projection(vfuncptr func, Var *arg);
#endif
Var *ff_self_convolve(vfuncptr func, Var *arg);
Var *ff_convolve(vfuncptr func, Var *arg);
Var *ff_convolve2(vfuncptr func, Var *arg);

Alist make_alist(char *name, int type, void *limits, void *value);


double bbr(double, double);
double btemp(double, double);
Var *newVal(int org, int x, int y, int z, int format, void *data);

int cmp_byte(const void *, const void *);
int cmp_short(const void *, const void *);
int cmp_int(const void *, const void *);
int cmp_float(const void *, const void *);
int cmp_double(const void *, const void *);

void log_line(char *str);
int make_args(int *ac, Var ***av, vfuncptr func, Var *args);
int parse_args(int ac, Var **av, Alist *alist);
int print_history(int i);

void xfree(void *);
void save_ufunc(char *filename);
void vax_ieee_r(float *from, float *to);

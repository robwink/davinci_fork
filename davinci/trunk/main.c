#include <setjmp.h>
#include "parser.h"

#include "y_tab.h"

#include <Xm/PushB.h>

int interactive = 1;

int debug = 0;
char pp_input_buf[8192];
FILE *lfile = NULL;
FILE *pfp = NULL;

int SCALE = 6;
int VERBOSE = 2;

int allocs = 0;
Var *VZERO;

int windows = 1;

/**
 ** This is stuff from the old input.c
 **/
int pp_count = 0;
int pp_line = 0;
int indent = 0;

/**
 ** Command line args:
 **
 **     -f filename     - read filename for input rather than stdin
 **     -l logfile      - log commands to logfile rather than '.dvlog'
 **     -q              - quick.  Don't load startup file
 **     -e string       - eval.  Evaluate string as commands, then exit
 **     -i              - interactive.  Force an interactive shell
 
 **/
void fake_data();
void env_vars();
void log_time();
void lhandler(char *line);
void quit(void);
void parse_buffer(char *buf);


void init_history(char *fname);
void process_streams(void);
void event_loop(void);

void rl_callback_read_char();
void rl_callback_handler_install(char *, void (char *));

void
sighandler(int data)
{
    extern jmp_buf env;
    Scope *scope;

    while ((scope = scope_tos()) != global_scope()) {
        dd_unput_argv(scope);
        clean_scope(scope_pop());
    }

    signal(SIGINT, sighandler);
    longjmp(env, 1);
}

char *__progname = "davinci";

int 
main(int ac, char **av)
{
    Scope *s;
    Var *v;
    FILE *fp;
    char path[256], cmd[256];
    int quick = 0;
    int i, j, k, flag = 0;
    char *logfile = NULL;
    int iflag = 0;
	
    s = new_scope();

    signal(SIGPIPE, SIG_IGN);
    // signal(SIGINT, sighandler);

    scope_push(s);
    /**
    ** handle $0 specially.
    **/
    v = p_mkval(ID_STRING, *av);
    V_NAME(v) = strdup("$0");
    put_sym(v);

    /**
    ** Everything that starts with a - is assumed to be an option,
    ** until we get something that doesn't.
    **
    ** The user can force this with --, as well.
    **/
    for (i = 1; i < ac; i++) {
        k = 0;
        if (!flag && av[i] && av[i][0] == '-') {
            for (j = 1; j < strlen(av[i]); j++) {
                switch (av[i][j]) {
                case '-':   /* last option */
                    flag = 1;
                    break;
                case 'w':   /* no windows */
                    windows = 0;
                    break;
                case 'f':{  /* redirected input file */
                    k++;
                    push_input_file(av[i + k]);
                    av[i + k] = NULL;
                    interactive = 0;
                    break;
                }
                case 'l':{  /* redirected log file */
                    k++;
                    logfile = av[i + k];
                    av[i + k] = NULL;
                    break;
                }
                case 'e':{  /* execute given command string */
                    FILE *fp;
                    k++;
                    if ((fp = tmpfile()) != NULL) {
                        fputs(av[i + k], fp);
                        fputc('\n', fp);
                        rewind(fp);
                        push_input_stream(fp);
                        interactive = 0;
                    }
                    av[i + k] = NULL;
                    break;
                }
                case 'v':{
                    if (isdigit(av[i][j + 1])) {
                        VERBOSE = av[i][j + 1] - '0';
                    } else {
                        k++;
                        VERBOSE = atoi(av[i + k]);
                        av[i + k] = NULL;
                    }
                    break;
                }
                case 'i':{
                    /** 
                    **/
                    iflag = 1;
                    break;
                }
                case 'q':{
                    quick = 1;
                    break;
                }
                }
            }
            i += k;
        } else {
            flag = 1;
            dd_put_argv(s, p_mkval(ID_STRING, av[i]));
        }
    }

    if (iflag)
        interactive = 1;

    env_vars();
    fake_data();

    if (interactive) {
        if (logfile == NULL)
            logfile = ".dvlog";
        lfile = fopen(logfile, "a");
        log_time();
        if (quick == 0)
            init_history(logfile);
    }
    if (quick == 0) {
        sprintf(path, "%s/.dvrc", getenv("HOME"));
        if ((fp = fopen(path, "r")) != NULL) {
            printf("reading file: %s\n", path);
            push_input_stream(fp);
        }
    }

    /**
    ** set up temporary directory
    **/
    sprintf(path, "TMPDIR=%s/dv_%d", P_tmpdir, getpid());
    mkdir(path + 7, 0777);
    putenv(path);

    /*
    ** Before we get to events, process any pushed files
    */
    process_streams();
    event_loop();
}

/* ARGSUSED */
void get_file_input(XtPointer client_data, int *fid, XtInputId *id)
{
    rl_callback_read_char();
}

extern XtAppContext        app;
extern Widget 		top;

void
event_loop(void)
{
	if (interactive) {
		if (windows && getenv("DISPLAY") != NULL)  {
			char *argv[1];
			char *av0 = "null";
			int argc = 1;
			XEvent event;
			argv[0] = av0;

			top = XtVaAppInitialize(&app, "Simple", NULL, 0,
									&argc,
									argv, NULL, NULL);
		} else {
			/**
			 ** This is a hack to let us use the Xt event model, without
			 ** needing an X server.  It is a bad idea.
			 **/
			XtToolkitInitialize();
			app = XtCreateApplicationContext();
		}

		XtAppAddInput(app,
					  fileno(stdin),
					  (void *) XtInputReadMask,
					  get_file_input,
					  NULL);
		rl_callback_handler_install("dv> ", lhandler);

		XtAppMainLoop(app);
	}
}

void lhandler(char *line)
{
    int i,j;
    char *buf;
    char prompt[256];
    extern int pp_line;
    extern int pp_count;

    /**
    ** Readline has a whole line of input, process it.
    **/
    if (line == NULL) {
        quit();
    }
		

    buf = (char *)malloc(strlen(line)+2);
    strcpy(buf, line);
    strcat(buf, "\n");

    if (*line) { 
        add_history((char *)line);
		log_line(buf);
    }

    pp_line++;
    pp_count = 0;

    parse_buffer(buf);

	if (indent) {
		sprintf(prompt, "%2d> ", indent);
	} else {
		sprintf(prompt, "dv> ", indent);
	}
	rl_callback_handler_install(prompt, lhandler);

    /*
    ** Process anything pushed onto the stream stack by the last command.
    */
    process_streams();
}

void
process_streams(void)
{
    extern FILE *ftos;
    extern int nfstack;
    /*
    ** Process anything that has been pushed onto the input stream stack.
    **/
    while (nfstack != 0) {
        parse_stream(ftos);
        // fclose(ftos);
		pop_input_file();
    }
}

void
parse_stream(FILE *fp)
{
	char buf[1024];
	extern int pp_line;

	pp_line = 0;
    while(fgets(buf, 1024, fp) != NULL) {
		pp_line++;
        parse_buffer(buf);
    }
}

Var *curnode;
void *parent_buffer;

void
parse_buffer(char *buf)
{
    int i,j;
	extern char *yytext;
	extern FILE *save_fp;
	int flag = 0;

    parent_buffer = yy_scan_string(buf);

    while(i = yylex()) {
		/*
		** if this is a function definition, do no further parsing yet.
		*/
		if (save_fp) continue;

        j = yyparse(i, (Var *)yytext);
		if (j == -1) quit();

		if (j == 1 && curnode != NULL) {
			evaluate(curnode);
			pp_print(pop(scope_tos()));
			free_tree(curnode);
			indent = 0;
			cleanup(scope_tos());
		}
    }
    yy_delete_buffer(parent_buffer);
}

void
quit(void)
{
    char cmd[256];
    char *path = getenv("TMPDIR");

    if (interactive) {
        printf("\n");
        rl_callback_handler_remove();
    }

    /**
    ** clean up temporary directory
    **/
    sprintf(cmd, "rm -rf %s &", path + 7);
    system(cmd);
    exit(1);
}

void
log_line(char *str)
{
    if (lfile) {
        fprintf(lfile, "%s", str);
        fflush(lfile);
    }
}

void 
yyerror(char *s)
{
    extern int pp_count;
    extern int pp_line;

    printf("***%*s^ ", pp_count, " ");
    printf("%s, line %d\n", s, pp_line);
}

int 
yywrap()
{
    return (1);
}

void
fake_data()
{
    Var *v;
    int i, j;

    v = new(Var);
    V_NAME(v) = strdup("a");
    V_TYPE(v) = ID_VAL;
    v = put_sym(v);

    V_DSIZE(v) = 4 * 3 * 2;
    V_SIZE(v)[0] = 4;
    V_SIZE(v)[1] = 3;
    V_SIZE(v)[2] = 2;
    V_ORG(v) = BSQ;
    V_FORMAT(v) = FLOAT;
    V_DATA(v) = calloc(4 * 3 * 2, sizeof(float));

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 3; j++) {
            ((float *) V_DATA(v))[i + j * 4] = (float) i + j * 4;
        }
    }

    srand48(getpid());
    for (i = 0; i < 12; i++) {
        ((float *) V_DATA(v))[i + 12] = drand48();
    }

    v = (Var *) calloc(1, sizeof(Var));
    V_NAME(v) = NULL;
    V_TYPE(v) = ID_VAL;
    V_DSIZE(v) = 1;
    V_SIZE(v)[0] = 1;
    V_SIZE(v)[1] = 1;
    V_SIZE(v)[2] = 1;
    V_ORG(v) = BSQ;
    V_FORMAT(v) = BYTE;
    V_DATA(v) = calloc(1, sizeof(u_char));
    VZERO = v;

    v = new(Var);
    V_NAME(v) = strdup("tmp");
    V_TYPE(v) = ID_VAL;
    v = put_sym(v);

    V_DSIZE(v) = 2 * 2 * 2;
    V_SIZE(v)[0] = 2;
    V_SIZE(v)[1] = 2;
    V_SIZE(v)[2] = 2;
    V_ORG(v) = BSQ;
    V_FORMAT(v) = FLOAT;
    V_DATA(v) = calloc(2 * 2 * 2, sizeof(float));

    for (i = 0; i < 8; i++) {
        ((float *) V_DATA(v))[i] = (float) i;
    }

}

void 
make_sym(Var * v, int format, char *str)
{
    V_FORMAT(v) = format;
    V_DSIZE(v) = 1;
    V_SIZE(v)[0] = V_SIZE(v)[1] = V_SIZE(v)[2] = 1;
    V_ORG(v) = BSQ;
    V_DATA(v) = calloc(1, NBYTES(format));

    switch (format) {
    case INT:
        *((int *) (V_DATA(v))) = strtol(str, NULL, 10);
        break;
    case FLOAT:{
        double d;
        d = strtod(str, NULL);
        if (d > MAXFLOAT || d < MINFLOAT && d != 0) {
            free(V_DATA(v));
            V_DATA(v) = calloc(1, NBYTES(DOUBLE));
            V_FORMAT(v) = DOUBLE;
            *((double *) (V_DATA(v))) = d;
        } else {
            *((float *) (V_DATA(v))) = d;
        }
    }
    }
}

void
env_vars()
{
    char *path;
    Var *s;

    if ((path = getenv("DATAPATH")) != NULL) {
        s = new(Var);
        V_NAME(s) = strdup("datapath");
        V_TYPE(s) = ID_STRING;
        V_STRING(s) = strdup(path);
        put_sym(s);
    }
}

char *
unquote(char *name)
{
    char *p = name;

    if (*name == '"') {
        p++;
        name++;
        while (*name) {
            if (*name == '"' && *(name - 1) != '\\')
                break;
            name++;
        }
        *name = '\0';
    } else if (*name == '\'') {
        p++;
        name++;
        while (*name) {
            if (*name == '\'' && *(name - 1) != '\\')
                break;
            name++;
        }
        *name = '\0';
    }
    return (p);
}

char *
unescape(char *str)
{
    char *p = str;
    char *q = str;

    while (*p) {
        if (*p == '\\') {
            if (*(p + 1) == 't')
                *q = '\t';
            else if (*(p + 1) == 'n')
                *q = '\n';
            else
                *q = *(p + 1);
            p++;
        } else {
            *q = *p;
        }
        p++;
        q++;
    }
    *q = *p;
    return (str);
}

void
log_time()
{
    time_t t;
    char *uname;
    if (lfile) {
        t = time(0);
        uname = getenv("USER");
        fprintf(lfile, "###################################################\n");
        fprintf(lfile, "# User: %8.8s    Date: %26.26s", uname, ctime(&t));
        fprintf(lfile, "###################################################\n");
    }
}

void
init_history(char *fname)
{
    char buf[256];
    FILE *fp;

#ifdef HAVE_LIBREADLINE

    if ((fp = fopen(fname, "r")) == NULL)
        return;
    printf("loading history\n");

    while (fgets(buf, 256, fp)) {
        if (buf[0] == '#')
            continue;
        buf[strlen(buf) - 1] = '\0';
        add_history((char *)buf);
    }
#endif
}


#include <time.h>
#include <setjmp.h>
#include "parser.h"

#include "y_tab.h"

#ifdef HAVE_XT
#define USE_X11_EVENTS 1
#endif

#ifdef USE_X11_EVENTS
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xatom.h>

Widget      applicationShell = NULL;
XtAppContext    applicationContext;
#endif

extern int interactive;
extern int continuation;
extern int in_comment;
extern int in_string;

extern int debug;
extern char pp_input_buf[8192];
extern FILE *lfile;
extern FILE *pfp;

extern int SCALE;
extern int VERBOSE;
extern int DEPTH ;

extern int allocs;
extern Var *VZERO;

static int windows = 1;
int usage(char *prog);

/**
 ** This is stuff from the old input.c
 **/
extern pp_count;
extern pp_line;
extern indent;

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
void rmrf(char *path);


void init_history(char *fname);
void process_streams(void);
void event_loop(void);

#ifdef HAVE_LIBREADLINE
#include <readline/readline.h>
#endif

char ** dv_complete_func(char *text, int start, int end);

#if 0
/* JAS FIX: these are all declared in readline.h and as far as I can tell
   they are not used unless readline is present. */

CPPFunction * rl_attempted_completion_function;

#ifdef USE_X11_EVENTS
void rl_callback_read_char();
void rl_callback_handler_install(char *, void (*)(char *));
#endif

#endif

jmp_buf env;

void user_sighandler(int data)
{ 
#ifndef __MINGW32__
     	signal(SIGUSR1, user_sighandler);
#else
	parse_error("Function not spported under Windows.");
#endif

}

void
dv_sighandler(int data)
{
    Scope *scope;
    char cmd[256];
    char *path = getenv("TMPDIR");


    switch (data) {

    case (SIGSEGV):
        rmrf(path);
        signal(SIGSEGV,SIG_DFL);
        break;

#if !(defined(__CYGWIN__) || defined(__MINGW32__))
    case (SIGBUS):
        rmrf(path);
        signal(SIGBUS,SIG_DFL);
        break;
#else
	parse_error("Function not spported under Windows.");	
#endif /* __CYGWIN__ */

    case (SIGINT):
        signal(SIGINT, SIG_IGN); 
        while ((scope = scope_tos()) != global_scope()) {
            dd_unput_argv(scope);
            clean_scope(scope_pop());
        }

        signal(SIGINT, dv_sighandler); 
        longjmp(env, 1);
        break;
    }
}

/* char *__progname = "davinci"; */

int 
main(int ac, char **av)
{
    Scope *s;
    Var *v;
    FILE *fp;
    char path[256];
    int quick = 0;
    int i, j, k, flag = 0;
    char *logfile = NULL;
    int iflag = 0;
    char *p;
    int history = 1;
    
    s = new_scope();

    signal(SIGINT, dv_sighandler); 
    signal(SIGSEGV, dv_sighandler);
#ifndef __MINGW32__
    signal(SIGPIPE, SIG_IGN);
    signal(SIGBUS, dv_sighandler);
    signal(SIGUSR1, user_sighandler);
#endif
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
	**
	** We now pass all "--options" as ARGV parameters.
    **/
    for (i = 1; i < ac; i++) {
        k = 0;
        if (!flag && av[i] && av[i][0] == '-' &&
				!(strlen(av[i]) > 2 && av[i][1] == '-')) {
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
                        push_input_stream(fp, ":command line:");
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
                        if (i+k >= ac) {
                            exit(usage(av[0]));
                        } else {
                            VERBOSE = atoi(av[i + k]);
                            av[i + k] = NULL;
                        }
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
                case 'H':{
                    /* force loading of the history, even in quick mode */
                    history = 1;
                    break;
                }
                case 'V':{
                    dump_version();
                    exit(1);
                }
                case 'h':{
                    usage(av[0]);
                    exit(1);
                }
                }
            }
            i += k;
        } else {
            char buf[256];
            flag = 1;
            dd_put_argv(s, p_mkval(ID_STRING, av[i]));
            v = p_mkval(ID_STRING, av[i]);
            sprintf(buf, "$%d", i);
            V_NAME(v) = strdup(buf);
            put_sym(v);
        }
    }
    dv_set_iom_verbosity();

    env_vars();
    fake_data();

    if (interactive) {
        if (logfile == NULL)
            logfile = ".dvlog";

        lfile = fopen(logfile, "a");
        log_time();
        if (quick == 0 || history == 1)
            init_history(logfile);
#ifdef HAVE_LIBREADLINE
    /* JAS FIX */
        rl_attempted_completion_function = dv_complete_func; 
#endif
    }
    if (quick == 0) {
        sprintf(path, "%s/.dvrc", getenv("HOME"));
        if ((fp = fopen(path, "r")) != NULL) {
            printf("reading file: %s\n", path);
            push_input_stream(fp, path);
        }
    }

    /**
    ** set up temporary directory
    **/
    if ((p = getenv("TMPDIR")) == NULL) {
        sprintf(path, "TMPDIR=%s/dv_%d", P_tmpdir, getpid());
    } else {
        sprintf(path, "TMPDIR=%s/dv_%d", getenv("TMPDIR"), getpid());
    }

#ifndef _WIN32
    mkdir(path + 7, 0777);
#else
	mkdir(path + 7);
#endif    
    putenv(path);

    /*
    ** Before we get to events, process any pushed files
    */
	/* moved the process_streams into the event loop so
	that it happens after Xt is initialized, but before
	the endless loop starts
	*/
    event_loop();
    quit();

    /* event_loop never returns... unless we're not interactive */

    return(0);
}

#if defined(USE_X11_EVENTS) && defined(HAVE_LIBREADLINE)
/* ARGSUSED */
void get_file_input(XtPointer client_data, int *fid, XtInputId *id)
{
    rl_callback_read_char();
}
#endif


#ifdef INCLUDE_API
#ifdef __cplusplus
extern "C" {
    extern SetTopLevel(Widget *);
}
#else
    extern SetTopLevel(Widget *);
#endif
#endif


#ifdef HAVE_XT

/* FIX: move to gui.c */

static String defaultAppResources[] = {
  "*TopLevelShell.allowShellResize: true",
  "*vicar.xZoomIn: 1",
  "*vicar.xZoomOut: 1",
  "*vicar.yZoomIn: 1",
  "*vicar.yZoomOut: 1",
  "*vicar.viewWidth: 640",
  "*vicar.viewHeight: 480",
  "*vicar.tileWidth: 256",
  "*vicar.tileHeight: 256",
  "*vicar.allowShellResize: True",
  "*vicar.shadowThickness: 1",
  /* NOTE: the following is one long string.  Don't add commas. */
  "*vicar.translations: #augment \\n "
  "~Shift<Btn2Down>:            MousePanStart() \\n "
  "~Shift<Btn2Motion>:          MousePan() \\n "
  "~Shift~Ctrl~Meta<Key>osfLeft:    PanOne(left) \\n "
  "~Shift~Ctrl~Meta<Key>osfRight:   PanOne(right) \\n "
  "~Shift~Ctrl~Meta<Key>osfUp:      PanOne(up) \\n "
  "~Shift~Ctrl~Meta<Key>osfDown:    PanOne(down) \\n "
  "Ctrl~Shift~Meta<Key>osfLeft:     PanEdge(left) \\n "
  "Ctrl~Shift~Meta<Key>osfRight:    PanEdge(right) \\n "
  "Ctrl~Shift~Meta<Key>osfUp:       PanEdge(up) \\n "
  "Ctrl~Shift~Meta<Key>osfDown:     PanEdge(down) \\n "
  "Shift~Ctrl~Meta<Key>osfLeft:     PanHalfView(left) \\n "
  "Shift~Ctrl~Meta<Key>osfRight:    PanHalfView(right) \\n "
  "Shift~Ctrl~Meta<Key>osfUp:       PanHalfView(up) \\n "
  "Shift~Ctrl~Meta<Key>osfDown:     PanHalfView(down) \\n "
  "<Key>osfActivate:            Input(\"Return hit\") \\n "
  "<Btn1Down>:              Input(\"Draw\",\"start\") \\n "
  "Button2<Key>space:           Input(\"Draw\",\"mark\") \\n "
  "<Btn1Motion>:            Input(\"Draw\",\"drag\") \\n "
  "<Btn1Up>:                Input(\"Draw\",\"end\") \\n "
  "<Key>osfEscape:          CursorMode(toggle) \\n "
  "~Shift<Key>grave:            CursorMode(toggle) \\n "
  "<Key>asciitilde:         CursorMode(toggle,true) \\n "
  "Shift<Key>grave:         CursorMode(toggle,true) \\n "
  "<Key>plus:               CursorMode(floating) \\n "
  "<Key>minus:              CursorMode(planted) \\n "
  "Shift<Motion>:           MoveCursorMouse() \\n "
  "<Key>c:              MoveCursorMouse() \\n "
  "Shift Ctrl<Key>osfLeft:      MoveCursor(left) \\n "
  "Shift Ctrl<Key>osfRight:     MoveCursor(right) \\n "
  "Shift Ctrl<Key>osfUp:        MoveCursor(up) \\n "
  "Shift Ctrl<Key>osfDown:      MoveCursor(down) \\n "
  "Meta<Key>osfLeft:            MoveCursorScreen(left) \\n "
  "Meta<Key>osfRight:           MoveCursorScreen(right) \\n "
  "Meta<Key>osfUp:          MoveCursorScreen(up) \\n "
  "Meta<Key>osfDown:            MoveCursorScreen(down) \\n "
  "<Visible>:               Input(\"VisibilityNotify\")",
  /* NOTE: end of long string. */
  NULL
};

#endif

void
event_loop(void)
{
    if (interactive) {
#if !defined(USE_X11_EVENTS) || !defined(HAVE_LIBREADLINE)
      /* JAS FIX */
    process_streams();
    lhandler((char *)readline("dv> "));
#else
    // JAS FIX: this should work even with a null DISPLAY, if -display is set..i think there are better
    // ways to get the display as well..
        if (windows && getenv("DISPLAY") != NULL)  {
      // JAS FIX: what is this argv/argv manglation?
            char *argv[1];
            char *av0 = "null";
            int argc = 1;
            argv[0] = av0;
            applicationShell = XtVaAppInitialize(&applicationContext,
                         "Davinci", NULL, 0,
                         &argc, argv,
                         defaultAppResources, NULL);

#ifdef INCLUDE_API
        /* FIX: what's this for? -JAS */
            SetTopLevel(&applicationShell);
#endif
        } else {
            /**
            ** This is a hack to let us use the Xt event model, without
            ** needing an X server.  It is a bad idea.
            **/
            XtToolkitInitialize();
            applicationContext = XtCreateApplicationContext();
        }

        XtAppAddInput(applicationContext,
                      fileno(stdin),
                      (void *) XtInputReadMask,
                      get_file_input,
                      NULL);

		process_streams();
        rl_callback_handler_install("dv> ", lhandler);
        XtAppMainLoop(applicationContext);
#endif
    } else {
		/* not interactive, but still have to process the input streams,
		   or -e (and scripts) will never work. */
		process_streams();
	}
}
void lhandler(char *line)
{
    char *buf;
    char prompt[256];
    extern int pp_line;
    extern int pp_count;

#if !defined(USE_X11_EVENTS) || !defined(HAVE_LIBREADLINE)
    /* JAS FIX */
    while (1) {
#endif

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

        setjmp(env);

        /*
        ** Process anything pushed onto the stream stack by the last command.
        */
        process_streams();

        if (indent) {
            sprintf(prompt, "%2d> ", indent);
        } else if (continuation) {
            continuation = 0;
            sprintf(prompt, "  > ");
        } else if (in_comment) {
            sprintf(prompt, "/*> ");    /* nothing */
        } else if (in_string) {
            sprintf(prompt, "\" > ");
        } else {
            sprintf(prompt, "dv> ");
        }

#if defined(USE_X11_EVENTS) && defined(HAVE_LIBREADLINE)
    /* JAS FIX */
    rl_callback_handler_install(prompt, lhandler);
#else
        line=(char *)readline(prompt);
    }
#endif

}

void
process_streams(void)
{
    extern FILE *ftos;
    extern int nfstack;
    char buf[1024];
    extern int pp_line;
    /*
    ** Process anything that has been pushed onto the input stream stack.
    **/

    while (nfstack) {
        while (fgets(buf, 1024, ftos) != NULL) {
            parse_buffer(buf);
            pp_line++;
        }
        pop_input_file();
    }
}

extern Var *curnode;

void
parse_buffer(char *buf)
{
    int i,j = 0;
    extern char *yytext;
    Var *v = NULL;
    void *parent_buffer;
    void *buffer;
    Var *node;
    extern char *pp_str;

    parent_buffer = (void *) get_current_buffer();
    buffer = (void *) yy_scan_string(buf);
    pp_str = buf;

	curnode = NULL;

    while((i = yylex()) != 0) {
        /*
        ** if this is a function definition, do no further parsing yet.
        */
        j = yyparse(i, (Var *)yytext);
        if (j == -1) quit();

        if (j == 1 && curnode != NULL) {
            node = curnode;
            evaluate(node);
            v = pop(scope_tos());
            pp_print(v);
            free_tree(node);
            indent = 0;
            cleanup(scope_tos());
        }
    }

    yy_delete_buffer((struct yy_buffer_state *)buffer);
    if (parent_buffer) yy_switch_to_buffer(parent_buffer);
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
fake_data()
{
    Var *v;
    int i, j;

    v = newVar();
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

    // srand48(getpid());
    srand( (unsigned int) time( NULL ) ); 
    srand48( (unsigned int) time( NULL ) ); 
    
    for (i = 0; i < 12; i++) {
#ifdef __MINGW32__
       ((float *) V_DATA(v))[i + 12] = ((double) rand())/((double)(RAND_MAX));
					//for some reason calling drand48() 
					//messes up the application
#else
       ((float *) V_DATA(v))[i + 12] = drand48();
#endif

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

    v = newVar();
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
env_vars()
{
    char *path;
    Var *s;

    if ((path = getenv("DATAPATH")) != NULL) {
        s = newVar();
        V_NAME(s) = strdup("datapath");
        V_TYPE(s) = ID_STRING;
        V_STRING(s) = strdup(path);
        put_sym(s);
    }
}

void
log_time()
{
    time_t t;
    char *uname;
    char tbuf[30];
    char *host;
    char cwd[1024];
    if (lfile) {
        t = time(0);
        /* eandres: ctime() seems to return invalid pointers on x86_64 systems.
         * ctime_r with a provided buffer fills the buffer correctly, but still
         * returns an invalid pointer. */

#ifdef __MINGW32__
	 time(&t);
	 strcpy(tbuf,ctime(&t));
#elif __sun
        ctime_r(&t, tbuf, sizeof(tbuf));
#else
        ctime_r(&t, tbuf);
#endif


        /* eandres: This really shouldn't be a problem, but it shouldn't be a crash, either. */
        if ((uname = getenv("USER")) == NULL) {
              uname = "<UNKNOWN>";
        }
        if ((host = getenv("HOST")) == NULL) {
            host = "<UNKNOWN>";
        }
        if (getcwd(cwd, 1024) == NULL) {
            strcpy(cwd, "<UNKNOWN>");
        }

        fprintf(lfile, "###################################################\n");
        fprintf(lfile, "# User: %8.8s    Date: %26.26s", uname, tbuf);
        fprintf(lfile, "# Host: %-11s Cwd: %s\n",  host, cwd);
        fprintf(lfile, "###################################################\n");
    }
}

void
init_history(char *fname)
{
    char buf[256];
    FILE *fp;
    int count = 0;

    /* JAS FIX: what's up with the two empty if/endif below? */
#ifdef HAVE_LIBREADLINE
#endif

    if ((fp = fopen(fname, "r")) == NULL)
        return;
    printf("loading history\n");

    while (fgets(buf, 256, fp)) {
        if (buf[0] == '#')
            continue;
        buf[strlen(buf) - 1] = '\0';
        add_history((char *)buf);
    }
#if 0
#endif
}


char **
dv_complete_func(char *text, int start, int end)
{
    return(NULL);
}


#ifndef HAVE_LIBREADLINE

void add_history () { }
char *readline(char *prompt) 
{
    char buf[256];
    fputs(prompt, stdout); 
    fflush(stdout);
    if (fgets(buf, 256, stdin) != NULL) {
        return(strdup(buf));
    } else {
        return(NULL);
    }
}
#endif


char *usage_str = 
"usage: %s [-Viwq] [-v#] [-l logfile] [-e cmd] [-f script] args\n"
" Options:\n"
"    -V            dump version information\n"
"    -i            force interactive mode\n"
"    -w            don't use X windows\n"
"    -q            quick startup.  Don't load history or .dvrc\n"
"    -H            force loadig of history, even in quick mode\n"
"    -h            print this help\n"
"    -l logfile    use logfile for loading/saving history instead of ./.dvrc\n"
"    -e cmd        execute the specified command and exit\n"
"    -f script     exectue the specified script and exit\n"
"    --            indicates this is the last command line option\n"
""
"  Note: Any --option style options are always passed as $ARGV values\n";


int
usage(char *prog) {
    fprintf(stderr, usage_str, prog);
    return(1);
}

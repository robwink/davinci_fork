#include "parser.h"
#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>

extern int interactive;
extern int debug;

char save_file[256];
static FILE *save_fp = NULL;

static char input_line[1024] = {0};
static int input_ptr = 0;
int indent = 0;
int pp_count = 0;
int pp_line = 0;
int indent = 0;

void log_line(char *);
int yylex(void);

void
input_loop(char *buf, int *result, int max_size)
{
    int len;
    extern FILE *ftos;
    char *il;
    char prompt[16];

    len = strlen(input_line + input_ptr);

    if (len == 0) {
        pp_count = 0;
        while (ftos) {
            if (interactive && ftos == stdin) {
                if (indent)
                    sprintf(prompt, "%2d> ", indent);
                else
                    sprintf(prompt, "dv> ");
                if ((il = readline(prompt)) != NULL) {
#ifdef HAVE_LIBREADLINE
                    /**
                     ** Readline does things the hard way.
                     **/
                    if (*il)
                        add_history(il);
                    strcpy(input_line, il);
                    strcat(input_line, "\n");
                    free(il);
                    il = input_line;
#endif
                }
            } else {
                input_line[0] = '\0';
                il = fgets(input_line, 1024, ftos);
            }
            if (il != NULL) {
                if (debug)
                    printf("%s", input_line);
                break;
            }
            pop_input_file();
        }
        if (ftos == NULL) {
            *result = 0;
            return;
        }
    /**
        ** dont record blank lines
        **/
        if (input_line[0] != '\n')
            log_line(input_line);
        input_ptr = 0;
        len = strlen(input_line);
    }
    max_size = min(max_size, len);
    memcpy(buf, input_line + input_ptr, max_size);
    input_ptr += max_size;
    *result = max_size;

    return;
}

char
input_nextc()
{
    char c[2];
    int r;

    input_loop(c, &r, 1);
    if (*c == '\n')
        pp_line++;
    if (r != 0) {
        pp_count++;
        if (save_fp)
            putc(*c, save_fp);
        return (*c);
    }
    return (0);
}

void
unput_nextc(char c)
{
    if (input_ptr > 0)
        input_ptr--;
    else {
        input_line[0] = c;
        input_line[1] = '\0';
    }
    if (c == '\n')
        pp_line--;
    if (save_fp)
        fseek(save_fp, -1, SEEK_CUR);
    if (pp_count)
        pp_count--;
}

void
eat_em()
{
    int t;
    int count = 0;
    char *tmp;

    tmp = tempnam(NULL, NULL);
    strcpy(save_file, tmp);
    free(tmp);

    save_fp = fopen(save_file, "w");
    fprintf(save_fp, "define");
    fflush(save_fp);

    while (1) {
        t = yylex();
        if (t == '\n' && count == 0) {
            unput_nextc(t);
            fclose(save_fp);
            /*
               save_fp = fopen(save_file, "r");
               while((c = fgetc(save_fp)) != EOF) {
               putc(c, stdout);
               }
               fclose(save_fp);
             */
            save_fp = NULL;
            return;
        }
        if (t == '{')
            count++;
        if (t == '}')
            count--;
    }
}


#ifndef HAVE_LIBREADLINE
char *
readline(char *prompt)
{
    char *ptr;

    printf(prompt);
    fflush(stdout);

    input_line[0] = '\0';
    fgets(input_line, 1024, stdin);
    return (input_line);
}
#endif

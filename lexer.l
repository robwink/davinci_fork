/******************************** lexer.l ******************************/
%{

#include "parser.h"
#include "y_tab.h"

char *yytext;


extern int pp_count;
#define YY_USER_ACTION pp_count+=yyleng;

#define MYECHO	if (save_fp) (void) fwrite( yytext, yyleng, 1, save_fp );

extern int indent;
FILE *save_fp = NULL;
char save_file[256];

void start_save()
{
    char *tmp;

    tmp = tempnam(NULL, NULL);
    strcpy(save_file, tmp);
    xfree(tmp);

    save_fp = fopen(save_file, "w");
}

void end_save()
{
	fclose(save_fp);
	save_fp = NULL;
	save_ufunc(save_file);
}

%}

ws      [ \t]+
ival    [0-9]+
real1   [0-9]+\.[0-9]*
real2   [0-9]*\.[0-9]+
real    {real1}|{real2}
ereal   ({ival}|{real})[Ee][+-]?[0-9]+
rval    {real}|{ereal}
var     [a-zA-Z_][a-zA-Z0-9_]*
comment #.*

%%

"define"	{ start_save(); MYECHO; return(FUNC_DEF); }
"for"		{ MYECHO; return(FOR); }
"while"     { MYECHO; return(WHILE); }
"where"     { MYECHO; return(WHERE); }
"break"     { MYECHO; return(BREAK); }
"continue"  { MYECHO; return(CONTINUE); }
"if"        { MYECHO; return(IF); }
"else"      { MYECHO; return(ELSE); }
"return"    { MYECHO; return(RETURN); }
"quit"      { MYECHO; return(QUIT); }
"help"		{ MYECHO; return(HELP); }

"=="        { MYECHO; return(EQ_OP); }
"!="        { MYECHO; return(NE_OP); }
"<="        { MYECHO; return(LE_OP); }
">="        { MYECHO; return(GE_OP); }
">"         { MYECHO; return(GT_OP); }
"<"         { MYECHO; return(LT_OP); }

"-="		{ MYECHO; return(DEC_OP); }
"+="		{ MYECHO; return(INC_OP); }

"||"        { MYECHO; return(OR_OP); }
"&&"        { MYECHO; return(AND_OP); }
"//"        { MYECHO; return(CAT_OP); }

"{"         { MYECHO; indent++; return('{'); }
"}"         { MYECHO; indent--; 
				if (save_fp && indent == 0) {
					end_save();
					/*
					 * Lie to the parser.
					 */
					return('\n');
				}
				return('}'); 
			}


{ival}              { yytext = strdup(yytext); MYECHO; return(IVAL); }
{rval}              { yytext = strdup(yytext); MYECHO; return(RVAL); }
{var}               { yytext = strdup(yytext); MYECHO; return(ID); }
\"(\\.|[^\\"\n])*\" { yytext = strdup(yytext); MYECHO; return(STRING); }
\'(\\.|[^\\'\n])*\' { yytext = strdup(yytext); MYECHO; return(STRING); }

{comment}   { MYECHO; }

{ws}        { MYECHO; }
"\n"        { MYECHO; return('\n'); }
.           { MYECHO; return(yytext[0]); };

%%

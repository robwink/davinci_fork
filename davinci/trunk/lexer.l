/******************************** lexer.l ******************************/

%{

#include "parser.h"
#include "y_tab.h"

#define MYECHO		if (save_fp) (void)fwrite( yytext, yyleng, 1, save_fp );

int local_line;
extern int pp_line;
extern int pp_count;
extern int continuation;
extern int in_comment;
extern int in_string;

#define YY_USER_ACTION pp_count+=yyleng; MYECHO

YY_BUFFER_STATE
get_current_buffer()
{
	return(YY_CURRENT_BUFFER);
}

extern int indent;
FILE *save_fp = NULL;
char save_file[256];

int caller;			/* used by flex to handle comments in multiple states */


void end_save()
{
	if (save_fp) {
		fprintf(save_fp, "\n");
		fclose(save_fp);
		save_fp = NULL;
	}
}

save_lines()
{
	
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

%x comment ufunc
%%

"define"	{ return(FUNC_DEF); }
"for"		{ return(FOR); }
"while"     { return(WHILE); }
"where"     { return(WHERE); }
"break"     { return(BREAK); }
"continue"  { return(CONTINUE); }
"if"        { return(IF); }
"else"      { return(ELSE); }
"return"    { return(RETURN); }
"quit"      { return(QUIT); }
"help"		{ return(HELP); }
^"!".*"\n"  { return(SHELL); }

"=="        { return(EQ_OP); }
"!="        { return(NE_OP); }
"<="        { return(LE_OP); }
">="        { return(GE_OP); }
">"         { return(GT_OP); }
"<"         { return(LT_OP); }

"-="		{ return(DEC_OP); }
"+="		{ return(INC_OP); }
"*="		{ return(MULSET_OP); }
"/="		{ return(DIVSET_OP); }

"||"        { return(OR_OP); }
"&&"        { return(AND_OP); }
"//"        { return(CAT_OP); }


"{"         { indent++; return('{'); }
"}"         { indent--; return('}'); }

"/*"		{ caller = INITIAL; BEGIN(comment); in_comment = 1; }

{ival}              { yytext = strdup(yytext); return(IVAL); }
{rval}              { yytext = strdup(yytext); return(RVAL); }
{var}               { yytext = strdup(yytext); return(ID); }

\"(\\.|[^\\"\n])*\" { yytext = strdup(yytext); return(STRING); }
\'(\\.|[^\\'\n])*\' { yytext = strdup(yytext); return(STRING); }

{comment}   { }
{ws}        { }

"\\\n"      { continuation=1; pp_line++; }
"\n"        { return('\n'); }
.           { return(yytext[0]); };

<ufunc>\"(\\.|[^\\"\n])*\" {  }
<ufunc>\'(\\.|[^\\'\n])*\' {  }

<ufunc>"/*"		   {  caller = ufunc; BEGIN(comment); in_comment = 1; }

<ufunc>"{"         {  indent++; } 
<ufunc>"}"         { 
						indent--; 
						if (indent == 0) { 
							BEGIN(INITIAL); 
							end_save();
							return(END_DEF);
						} 
						printf(">> %s\n", yytext);
				   }
<ufunc>"\\\n"      {  continuation=1; }
<ufunc>"\n"        { 
						if (indent == 0) {
							/*
							** well, apparently, we aren't in a ufunc.  Punt.
							*/
							return('\n');;
						}
					}
<ufunc>{comment}   {  }
<ufunc>[^{}]*      {  };


<comment>[^*\n]*        {  /* eat anything that's not a '*' */ }
<comment>"*"+[^*/\n]*   {  /* eat up '*'s not followed by '/'s */ }
<comment>\n             {  }
<comment>"*"+"/"        {  BEGIN(caller); in_comment = 0;}
%%

void start_save()
{
    char *tmp;

	local_line = pp_line;
    tmp = tempnam(NULL, NULL);
    strcpy(save_file, tmp);
    free(tmp);

    save_fp = fopen(save_file, "w");
	fprintf(save_fp, "%s", yytext);
	fflush(save_fp);
}


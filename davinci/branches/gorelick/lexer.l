/******************************** lexer.l ******************************/
%{

#include "parser.h"
#include "y_tab.h"

char *yytext;


extern int pp_count;
#define YY_USER_ACTION pp_count+=yyleng;

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

"=="        { return(EQ_OP); }
"!="        { return(NE_OP); }
"<="        { return(LE_OP); }
">="        { return(GE_OP); }
">"         { return(GT_OP); }
"<"         { return(LT_OP); }

"-="		{ return(DEC_OP); }
"+="		{ return(INC_OP); }

"||"        { return(OR_OP); }
"&&"        { return(AND_OP); }
"//"        { return(CAT_OP); }


{ival}              { return(IVAL); }
{rval}              { return(RVAL); }
{var}               { return(ID); }
\"(\\.|[^\\"\n])*\" { return(STRING); }
\'(\\.|[^\\'\n])*\' { return(STRING); }
{comment}   ;

{ws}        { }
"\n"        { return('\n'); }
.           { return(yytext[0]); };

%%

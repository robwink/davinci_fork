%{
#include <stdio.h>
#include <setjmp.h>
#include "parser.h"

Var *p_mknod(int , Var *, Var *);
Var *p_mkval(int , char *);

extern Var *curnode;
extern char *yytext;
extern int indent;
extern FILE *ftos;
jmp_buf env;

int eatNL =1;
%}

%token WHILE CONTINUE BREAK RETURN FOR WHERE
%token IF ELSE
%token IVAL RVAL STRING
%token ID
%token DEC_OP INC_OP
%token LT_OP GT_OP GE_OP LE_OP EQ_OP NE_OP
%token AND_OP OR_OP CAT_OP
%token QUIT HELP LIST
%token FUNC_DEF
%token SHELL

%%

start
    : statement				{ curnode = $$ = $1;  YYACCEPT; }
    | error separator       { curnode = NULL ; YYACCEPT; }
	|
    ;

/**
 ** Functions should evaluate once, but the tree shouldn't be
 ** thrown away.  Store it in the current scope, before returning.
 **/

statements
    : statement                 { 
                                    if (check_ufunc($1)) {
										if (setjmp(env) == 0) {
											evaluate($1);
											pp_print(pop(scope_tos()));
										} else {
											while (ftos && ftos != stdin)
												pop_input_file();
										}
                                        free_tree($1);
										cleanup(scope_tos());
                                    }
                                }
    | error separator           {
                                    indent = 0;
                                }
    | statements statement      {
                                    if (check_ufunc($2)) {
										if (setjmp(env) == 0) {
											evaluate($2);
											pp_print(pop(scope_tos()));
										} else {
											while (ftos && ftos != stdin)
												pop_input_file();
										}
                                        free_tree($2);
										cleanup(scope_tos());
                                    }
                                }
    ;

statement
    : expression_statement                  { $$ = $1; }
    | selection_statement                   { $$ = $1; }
    | jump_statement                        { $$ = $1; }
    | compound_statement                    { $$ = $1; }
    | iteration_statement                   { $$ = $1; }
    | command_statement                     { $$ = $1; }
	| SHELL                                 { $$ = pp_shell(yytext); }
    | FUNC_DEF ID '(' args ')' compound_statement { $$ = NULL; }
    | QUIT                                  { YYABORT; }
	;

command_statement
    : help_statement separator      { $$ = $1; }
    ;

help_statement
    : HELP                          { $$ = pp_help(NULL); }
    | HELP id                       { $$ = pp_help($2); }
    | '?'                           { $$ = pp_help(NULL); }
    | '?' id                        { $$ = pp_help($2); }
    | id '(' '?' ')'                { $$ = pp_help($1); }
    ;

expression_statement
    : separator                     { $$ = NULL; }
    | expression separator          { $$ = $1; }
    ;

compound_statement
    :   '{' {indent++;} statement_list '}'  { indent--;$$ = $3; }
    ;

statement_list
    : statement                             { $$ = $1; } 
    | statement_list statement              { $$ = p_rlist(ID_LIST, $1, $2); }
    ;

selection_statement
    : IF '(' expression ')' statement ELSE statement [YYVALID;]
                            { $$ = p_mknod(ID_IF, $3, p_mknod(ID_ELSE, $5, $7)); }
    | IF '(' expression ')' statement [YYVALID;]
							{ $$ = p_mknod(ID_IF, $3, $5); }
	;


separator
    : ';'
    | '\n'
    ;


iteration_statement
    : WHILE '(' expression ')' statement    { $$ = p_mknod(ID_WHILE, $3, $5); }
    | FOR '(' forval ';' forval ';' forval ')' statement
                                            { $$ = p_mknod(ID_LIST, $3,
                                                   p_mknod(ID_WHILE, $5,
                                                   p_mknod(ID_FOR, $9, $7))); }
    ;

forval
    :                                   { $$ = NULL; }
    | expression                        { $$ = $1; }
    ;

jump_statement
    : CONTINUE separator          { $$ = p_mknod(ID_CONT,NULL,NULL); }
    | BREAK separator             { $$ = p_mknod(ID_BREAK,NULL,NULL); }
    | RETURN expression separator  { $$ = p_mknod(ID_RETURN,$2,NULL); }
    ;

args
    :                                   { $$ = NULL; }
    | arg                               { $$ = p_mknod(ID_ARGS, NULL, $1); }
    | args ',' arg                      { $$ = p_mknod(ID_ARGS, $1, $3); }
    ;

arg
    : concat                            { $$ = p_mknod(ID_ARG, NULL, $1); }
    | id '=' expression                 { $$ = p_mknod(ID_ARG, $1, $3); }
    ;

ranges
    : range2                            { $$ = p_mknod(ID_RANGES, NULL, $1); }
    | ranges ',' range2                 { $$ = p_mknod(ID_RANGES, $1, $3) ; }
    ;

range2
    :                                 { $$ = p_mknod(ID_RANGE, NULL, NULL); }
    | expression                      { $$ = p_mknod(ID_RANGE, $1, $1); }
    | ':' ':' expression              { $$ = p_mknod(ID_RSTEP, NULL, $3); }
	| range   					  	  { $$ = $1; }
	| range ':' 					  { $$ = p_mknod(ID_RSTEP, $1, NULL); }
	| range ':' expression			  { $$ = p_mknod(ID_RSTEP, $1, $3); }
	;

range
    : ':' expression                  { $$ = p_mknod(ID_RANGE, NULL, $2); }
    | expression ':'                  { $$ = p_mknod(ID_RANGE, $1, NULL); }
    | expression ':' expression       { $$ = p_mknod(ID_RANGE, $1, $3); }
    ;

expression
    : concat                            { $$ = $1; }
    | id '=' expression                 { $$ = p_mknod(ID_SET,$1,$3); }
    | id INC_OP expression              { $$ = p_mknod(ID_INC,$1,$3); }
    | id DEC_OP expression              { $$ = p_mknod(ID_DEC,$1,$3); }
    | id '[' ranges ']' '=' expression  { $$ = p_mknod(ID_SET,
                                               p_mknod(ID_ARRAY, $1,$3), $6); }
    | id '[' WHERE expression ']' '=' expression  
										{ $$ = p_mknod(ID_SET,
                                               p_mknod(ID_WHERE, $1, $4), $7); }
    ;

concat
    : logical_OR                        { $$ = $1; }
    | concat CAT_OP logical_OR			{ $$ = p_mknod(ID_CAT, $1, $3); }
    ;

logical_OR
    : logical_AND                       { $$ = $1; }
    | logical_OR OR_OP logical_AND      { $$ = p_mknod(ID_OR ,$1,$3); }
    ;

logical_AND
    : equality_expr                     { $$ = $1; }
    | logical_AND AND_OP equality_expr  { $$ = p_mknod(ID_AND ,$1,$3); }
    ;

equality_expr
    : relation_expr                     { $$ = $1; }
    | equality_expr EQ_OP relation_expr { $$ = p_mknod(ID_EQ ,$1,$3); }
    | equality_expr NE_OP relation_expr { $$ = p_mknod(ID_NE ,$1,$3); }
    ;

relation_expr
    : additive_expr                     { $$ = $1; }
    | relation_expr LT_OP additive_expr { $$ = p_mknod(ID_LT ,$1,$3); }
    | relation_expr GT_OP additive_expr { $$ = p_mknod(ID_GT ,$1,$3); }
    | relation_expr LE_OP additive_expr { $$ = p_mknod(ID_LE ,$1,$3); }
    | relation_expr GE_OP additive_expr { $$ = p_mknod(ID_GE ,$1,$3); }
    ;

additive_expr
    : mult_expr                         { $$ = $1; }
    | additive_expr '+' mult_expr       { $$ = p_mknod(ID_ADD,$1,$3); }
    | additive_expr '-' mult_expr       { $$ = p_mknod(ID_SUB,$1,$3); }
    ;

mult_expr
    : power_expr                        { $$ = $1; }
    | mult_expr '*' power_expr          { $$ = p_mknod(ID_MULT, $1, $3); }
    | mult_expr '/' power_expr          { $$ = p_mknod(ID_DIV, $1, $3); }
    | mult_expr '%' power_expr          { $$ = p_mknod(ID_MOD, $1, $3); }
    ;

power_expr
    : unary_expr                        { $$ = $1; }
    | power_expr '^' unary_expr         { $$ = p_mknod(ID_POW,$1,$3); }
    ;

unary_expr
    : postfix_expr                      { $$ = $1; }
    | '-' unary_expr                    { $$ = p_mknod(ID_UMINUS,NULL,$2); }
    ;


postfix_expr
    : lhs                               { $$ = $1; }
    | id '[' ranges ']'                 { $$ = p_mknod(ID_ARRAY,$1,$3); }
    | id '(' args ')'                   { $$ = p_mknod(ID_FUNCT,$1,$3); }
    | '$' id '(' args ')'               { $$ = p_mknod(ID_FUNCT,
                                                    p_mknod(ID_ARGV, $2, NULL),
                                                    $4); };
    | '$' ival '(' args ')'             { $$ = p_mknod(ID_FUNCT,
                                                    p_mknod(ID_ARGV, $2, NULL),
                                                    $4); };
    | '(' expression ')'                { $$ = $2; }
    ;

lhs
    : ival                              { $$ = $1; }
    | rval                              { $$ = $1; }
    | string                            { $$ = $1; }
    | id                                { $$ = $1; }
    | '$' ival                          { $$ = p_mknod(ID_ARGV, $2, NULL); }
    | '$' id                            { $$ = p_mknod(ID_ARGV, $2, NULL); }
    | '$' id '[' expression ']'         { $$ = p_mknod(ID_ARGV, $2, $4); }
    ;

id: ID                                  { $$ = p_mkval(ID_ID, $1); }
ival : IVAL                             { $$ = p_mkval(ID_IVAL, $1); }
rval: RVAL                              { $$ = p_mkval(ID_RVAL, $1); }
string: STRING                          { $$ = p_mkval(ID_STRING, $1); }
;

%%

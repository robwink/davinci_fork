
/*  A Bison parser, made from parser.y
 by  GNU Bison version 1.27
  */

#define YYBISON 1  /* Identify Bison output.  */

#define	WHILE	257
#define	CONTINUE	258
#define	BREAK	259
#define	RETURN	260
#define	FOR	261
#define	WHERE	262
#define	IF	263
#define	ELSE	264
#define	IVAL	265
#define	RVAL	266
#define	STRING	267
#define	ID	268
#define	DEC_OP	269
#define	INC_OP	270
#define	INCSET_OP	271
#define	DECSET_OP	272
#define	MULSET_OP	273
#define	DIVSET_OP	274
#define	LT_OP	275
#define	GT_OP	276
#define	GE_OP	277
#define	LE_OP	278
#define	EQ_OP	279
#define	NE_OP	280
#define	AND_OP	281
#define	OR_OP	282
#define	CAT_OP	283
#define	LSHIFT_OP	284
#define	RSHIFT_OP	285
#define	QUIT	286
#define	HELP	287
#define	LIST	288
#define	FUNC_DEF	289
#define	SHELL	290

#line 1 "parser.y"

#include <stdio.h>
#include <setjmp.h>
#include "parser.h"

Var *p_mknod(int , Var *, Var *);
Var *p_mkval(int , char *);

extern Var *curnode;
extern char *yytext;
extern FILE *ftos;
jmp_buf env;

extern int indent;
extern int pp_count;

int log_it = 0;
char *pp_str;

void out()
{
    printf("%s", pp_str);
}

#ifndef YYSTYPE
#define YYSTYPE int
#endif
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		199
#define	YYFLAG		-32768
#define	YYNTBASE	58

#define YYTRANSLATE(x) ((unsigned)(x) <= 290 ? yytranslate[x] : 93)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    43,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,    57,    53,     2,     2,    37,
    38,    51,    49,    44,    50,    55,    52,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    46,    42,     2,
    45,     2,    39,    56,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    47,     2,    48,    54,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    40,     2,    41,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    28,    29,    30,    31,    32,    33,    34,    35,    36
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     5,     6,     8,    10,    12,    14,    16,    18,
    20,    22,    25,    32,    35,    37,    40,    43,    45,    48,
    50,    53,    57,    59,    62,    70,    76,    78,    80,    86,
    94,   101,   104,   107,   111,   112,   114,   118,   120,   124,
   126,   130,   131,   133,   137,   139,   142,   146,   149,   152,
   156,   158,   160,   164,   167,   170,   174,   178,   182,   186,
   194,   196,   200,   202,   206,   208,   212,   214,   218,   222,
   224,   228,   232,   236,   240,   242,   246,   250,   252,   256,
   260,   262,   266,   270,   274,   276,   280,   282,   285,   287,
   289,   291,   293,   295,   299,   301,   306,   310,   315,   321,
   326,   331,   335,   337,   339,   341,   343,   346,   352,   355,
   361
};

static const short yyrhs[] = {    59,
     0,     1,    66,     0,     0,    62,     0,    65,     0,    68,
     0,    63,     0,    67,     0,    60,     0,    36,     0,    32,
     0,    35,    66,     0,    35,    86,    37,    69,    38,    63,
     0,    61,    66,     0,    33,     0,    33,    86,     0,    33,
    89,     0,    39,     0,    39,    86,     0,    66,     0,    74,
    66,     0,    40,    64,    41,     0,    59,     0,    64,    59,
     0,     9,    37,    74,    38,    59,    10,    59,     0,     9,
    37,    74,    38,    59,     0,    42,     0,    43,     0,     3,
    37,    74,    38,    59,     0,     7,    37,    62,    62,    74,
    38,    59,     0,     7,    37,    62,    62,    38,    59,     0,
     4,    66,     0,     5,    66,     0,     6,    74,    66,     0,
     0,    70,     0,    69,    44,    70,     0,    76,     0,    86,
    45,    74,     0,    72,     0,    71,    44,    72,     0,     0,
    74,     0,    46,    46,    74,     0,    73,     0,    73,    46,
     0,    73,    46,    74,     0,    46,    74,     0,    74,    46,
     0,    74,    46,    74,     0,    76,     0,    75,     0,    91,
    45,    74,     0,    91,    16,     0,    91,    15,     0,    91,
    17,    74,     0,    91,    18,    74,     0,    91,    19,    74,
     0,    91,    20,    74,     0,    91,    47,     8,    74,    48,
    45,    74,     0,    77,     0,    76,    29,    77,     0,    78,
     0,    77,    28,    78,     0,    79,     0,    78,    27,    79,
     0,    80,     0,    79,    25,    80,     0,    79,    26,    80,
     0,    81,     0,    80,    21,    82,     0,    80,    22,    82,
     0,    80,    24,    82,     0,    80,    23,    82,     0,    82,
     0,    82,    30,    83,     0,    82,    31,    83,     0,    83,
     0,    82,    49,    83,     0,    82,    50,    83,     0,    84,
     0,    83,    51,    84,     0,    83,    52,    84,     0,    83,
    53,    84,     0,    85,     0,    84,    54,    85,     0,    92,
     0,    50,    85,     0,    14,     0,    11,     0,    12,     0,
    13,     0,    86,     0,    37,    74,    38,     0,    90,     0,
    91,    47,    71,    48,     0,    91,    55,    86,     0,    91,
    37,    69,    38,     0,    56,    86,    37,    70,    38,     0,
    56,    37,    70,    38,     0,    91,    37,    39,    38,     0,
    91,    55,    33,     0,    87,     0,    88,     0,    89,     0,
    91,     0,    57,    86,     0,    57,    86,    47,    74,    48,
     0,    57,    87,     0,    57,    87,    47,    74,    48,     0,
    40,    69,    41,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    41,    42,    48,    53,    54,    55,    56,    57,    58,    59,
    60,    61,    62,    68,    72,    73,    74,    75,    76,    80,
    81,    85,    89,    90,    94,    96,   102,   107,   117,   118,
   122,   129,   133,   137,   143,   144,   145,   149,   150,   154,
   155,   159,   160,   161,   162,   163,   164,   168,   169,   170,
   174,   175,   179,   180,   181,   182,   183,   184,   185,   186,
   193,   194,   198,   199,   203,   204,   208,   209,   210,   214,
   215,   216,   217,   218,   222,   223,   224,   228,   229,   230,
   234,   235,   236,   237,   241,   242,   246,   247,   250,   251,
   252,   253,   257,   258,   263,   264,   265,   266,   267,   268,
   269,   270,   274,   275,   276,   277,   278,   279,   280,   281,
   282
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","WHILE",
"CONTINUE","BREAK","RETURN","FOR","WHERE","IF","ELSE","IVAL","RVAL","STRING",
"ID","DEC_OP","INC_OP","INCSET_OP","DECSET_OP","MULSET_OP","DIVSET_OP","LT_OP",
"GT_OP","GE_OP","LE_OP","EQ_OP","NE_OP","AND_OP","OR_OP","CAT_OP","LSHIFT_OP",
"RSHIFT_OP","QUIT","HELP","LIST","FUNC_DEF","SHELL","'('","')'","'?'","'{'",
"'}'","';'","'\\n'","','","'='","':'","'['","']'","'+'","'-'","'*'","'/'","'%'",
"'^'","'.'","'@'","'$'","start","statement","command_statement","help_statement",
"expr_stmt","compound_statement","statement_list","selection_statement","separator",
"iteration_statement","jump_statement","args","arg","ranges","range2","range",
"expr","assignment_expr","concat","logical_OR","logical_AND","equality_expr",
"relation_expr","shift_expr","additive_expr","mult_expr","power_expr","unary_expr",
"id","ival","rval","string","primary_expr","postfix_expr","rhs_postfix_expr", NULL
};
#endif

static const short yyr1[] = {     0,
    58,    58,    58,    59,    59,    59,    59,    59,    59,    59,
    59,    59,    59,    60,    61,    61,    61,    61,    61,    62,
    62,    63,    64,    64,    65,    65,    66,    66,    67,    67,
    67,    68,    68,    68,    69,    69,    69,    70,    70,    71,
    71,    72,    72,    72,    72,    72,    72,    73,    73,    73,
    74,    74,    75,    75,    75,    75,    75,    75,    75,    75,
    76,    76,    77,    77,    78,    78,    79,    79,    79,    80,
    80,    80,    80,    80,    81,    81,    81,    82,    82,    82,
    83,    83,    83,    83,    84,    84,    85,    85,    86,    87,
    88,    89,    90,    90,    91,    91,    91,    91,    91,    91,
    91,    91,    92,    92,    92,    92,    92,    92,    92,    92,
    92
};

static const short yyr2[] = {     0,
     1,     2,     0,     1,     1,     1,     1,     1,     1,     1,
     1,     2,     6,     2,     1,     2,     2,     1,     2,     1,
     2,     3,     1,     2,     7,     5,     1,     1,     5,     7,
     6,     2,     2,     3,     0,     1,     3,     1,     3,     1,
     3,     0,     1,     3,     1,     2,     3,     2,     2,     3,
     1,     1,     3,     2,     2,     3,     3,     3,     3,     7,
     1,     3,     1,     3,     1,     3,     1,     3,     3,     1,
     3,     3,     3,     3,     1,     3,     3,     1,     3,     3,
     1,     3,     3,     3,     1,     3,     1,     2,     1,     1,
     1,     1,     1,     3,     1,     4,     3,     4,     5,     4,
     4,     3,     1,     1,     1,     1,     2,     5,     2,     5,
     3
};

static const short yydefact[] = {     0,
     0,     0,     0,     0,     0,     0,     0,    90,    91,    92,
    89,    11,    15,     0,    10,     0,    18,    35,    27,    28,
     0,     0,     0,     1,     9,     0,     4,     7,     5,    20,
     8,     6,     0,    52,    51,    61,    63,    65,    67,    70,
    75,    78,    81,    85,    93,   103,   104,   105,    95,   106,
    87,     2,     0,    32,    33,    35,     0,     0,     0,    16,
    17,    12,     0,     0,    19,    23,     0,     0,    36,    38,
    93,    88,   106,     0,     0,   107,   109,    14,    21,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,    55,    54,     0,     0,
     0,     0,    35,     0,    42,     0,     0,    38,    34,     0,
     0,    35,    94,    22,    24,   111,     0,     0,    42,     0,
     0,     0,     0,    62,    64,    66,    68,    69,    71,    72,
    74,    73,    76,    77,    79,    80,    82,    83,    84,    86,
    56,    57,    58,    59,     0,     0,    53,     0,     0,     0,
    40,    45,    43,   102,    97,     0,     0,     0,     0,    37,
    39,   100,     0,     0,     0,   101,    98,     0,     0,    48,
    42,    96,    46,    49,    29,     0,     0,    26,     0,    99,
   108,   110,     0,    44,    41,    47,    50,    31,     0,     0,
     0,    13,     0,    30,    25,    60,     0,     0,     0
};

static const short yydefgoto[] = {   197,
    66,    25,    26,    27,    28,    67,    29,    30,    31,    32,
    68,    69,   150,   151,   152,    33,    34,    35,    36,    37,
    38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
    48,    49,    50,    51
};

static const short yypact[] = {    29,
    97,    22,    97,    97,   344,    33,    67,-32768,-32768,-32768,
-32768,-32768,    -3,     4,-32768,   344,    23,   262,-32768,-32768,
   344,   -10,    64,-32768,-32768,    97,-32768,-32768,-32768,-32768,
-32768,-32768,    97,-32768,    10,    21,    69,   117,    59,-32768,
    24,    42,    53,-32768,-32768,-32768,-32768,-32768,-32768,   387,
-32768,-32768,   344,-32768,-32768,   344,    97,   266,   344,-32768,
-32768,-32768,    76,    77,-32768,-32768,   207,    43,-32768,   -21,
    73,-32768,   -24,   344,    91,   103,   104,-32768,-32768,   344,
   344,   344,   344,   344,   344,   344,   344,   344,   344,   344,
   344,   344,   344,   344,   344,   344,-32768,-32768,   344,   344,
   344,   344,   274,   344,   124,    -5,   114,    10,-32768,   266,
   115,   344,-32768,-32768,-32768,-32768,   344,   344,   314,   116,
   344,   344,   344,    21,    69,   117,    59,    59,    95,    95,
    95,    95,    42,    42,    42,    42,    53,    53,    53,-32768,
-32768,-32768,-32768,-32768,   119,   -32,-32768,   344,   322,    12,
-32768,   113,   120,-32768,-32768,   262,   336,   262,     6,-32768,
-32768,-32768,   122,   125,   127,-32768,-32768,   129,   344,-32768,
   314,-32768,   344,   344,-32768,   262,   140,   152,   123,-32768,
-32768,-32768,   134,-32768,-32768,-32768,-32768,-32768,   262,   262,
   262,-32768,   344,-32768,-32768,-32768,   182,   183,-32768
};

static const short yypgoto[] = {-32768,
     0,-32768,-32768,   -57,     5,-32768,-32768,   108,-32768,-32768,
   -86,   -69,-32768,    14,-32768,    -2,-32768,   -11,   107,   128,
   106,    65,-32768,     3,    35,    36,   -19,     2,   194,-32768,
   209,-32768,   112,-32768
};


#define	YYLAST		442


static const short yytable[] = {    24,
   110,    72,    57,    11,   120,   167,    70,    80,    11,    10,
    11,   117,   103,    64,    60,    63,   146,    11,    65,    71,
   -51,   -51,   119,    75,    76,   159,    74,   154,    -3,     1,
   106,     2,     3,     4,     5,     6,    11,     7,    80,     8,
     9,    10,    11,   179,   108,    19,    20,   160,    81,   117,
   107,   163,   157,    89,    90,   171,   111,    71,    53,   172,
    12,    13,   108,    14,    15,    16,   115,    17,    18,    58,
    19,    20,    91,    92,     8,    71,   140,    11,    21,    85,
    86,    87,    88,   116,    22,    23,   117,   129,   130,   131,
   132,   108,    93,    94,    95,    82,   141,   142,   143,   144,
   108,   147,   153,    59,    71,   108,    96,   155,    52,   108,
    54,    55,   112,    71,   113,   161,   153,   118,    71,   164,
   165,    62,    71,   133,   134,   135,   136,   121,   137,   138,
   139,   148,    73,    78,     8,     9,    10,    11,    19,    20,
    79,    83,    84,    91,    92,   168,   170,   127,   128,   122,
   123,   156,   158,   162,   177,   175,   166,   178,   173,   180,
    16,   190,   191,    56,   109,   174,   184,    73,   153,   149,
   186,   187,   181,    21,   182,   188,   183,   189,   193,    22,
    23,   198,   199,   192,   185,    73,   124,   126,   194,   195,
   196,    73,    73,    73,    73,    73,    73,    73,    73,    73,
    73,    73,    73,    73,    73,    73,    73,    73,   125,     2,
     3,     4,     5,     6,    73,     7,    77,     8,     9,    10,
    11,    61,     0,    73,     0,     0,     0,     0,    73,     0,
     0,     0,    73,     0,     0,     0,     0,     0,    12,    13,
     0,    14,    15,    16,     0,    17,    18,   114,    19,    20,
     0,     0,     0,     0,     0,     0,    21,     0,     0,     0,
     0,     0,    22,    23,     2,     3,     4,     5,     6,     0,
     7,     0,     8,     9,    10,    11,     8,     9,    10,    11,
     0,     0,     0,     0,     8,     9,    10,    11,     0,     0,
     0,     0,     0,    12,    13,     0,    14,    15,    16,     0,
    17,    18,    16,    19,    20,    56,     0,    19,    20,     0,
    16,    21,   145,    56,     0,    21,     0,    22,    23,     0,
     0,    22,    23,    21,     8,     9,    10,    11,     0,    22,
    23,     0,     8,     9,    10,    11,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     8,     9,    10,    11,
    16,     0,     0,    56,     8,     9,    10,    11,    16,   149,
     0,    56,     0,    21,     0,     0,     0,   169,     0,    22,
    23,    21,    16,   176,     0,    56,     0,    22,    23,     0,
    16,     0,     0,    56,     0,    21,     0,     0,     0,     0,
     0,    22,    23,    21,     0,     0,     0,     0,     0,    22,
    23,    97,    98,    99,   100,   101,   102,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,   103,     0,     0,     0,     0,     0,     0,
     0,   104,     0,   105,     0,     0,     0,     0,     0,     0,
     0,   106
};

static const short yycheck[] = {     0,
    58,    21,     5,    14,    74,    38,    18,    29,    14,    13,
    14,    44,    37,    16,    13,    14,   103,    14,    17,    18,
    42,    43,    47,    22,    23,   112,    37,    33,     0,     1,
    55,     3,     4,     5,     6,     7,    14,     9,    29,    11,
    12,    13,    14,    38,    56,    42,    43,   117,    28,    44,
    53,   121,   110,    30,    31,    44,    59,    56,    37,    48,
    32,    33,    74,    35,    36,    37,    67,    39,    40,    37,
    42,    43,    49,    50,    11,    74,    96,    14,    50,    21,
    22,    23,    24,    41,    56,    57,    44,    85,    86,    87,
    88,   103,    51,    52,    53,    27,    99,   100,   101,   102,
   112,   104,   105,    37,   103,   117,    54,   106,     1,   121,
     3,     4,    37,   112,    38,   118,   119,    45,   117,   122,
   123,    14,   121,    89,    90,    91,    92,    37,    93,    94,
    95,     8,    21,    26,    11,    12,    13,    14,    42,    43,
    33,    25,    26,    49,    50,   148,   149,    83,    84,    47,
    47,    38,    38,    38,   157,   156,    38,   158,    46,    38,
    37,    10,    40,    40,    57,    46,   169,    56,   171,    46,
   173,   174,    48,    50,    48,   176,    48,    38,    45,    56,
    57,     0,     0,   179,   171,    74,    80,    82,   189,   190,
   193,    80,    81,    82,    83,    84,    85,    86,    87,    88,
    89,    90,    91,    92,    93,    94,    95,    96,    81,     3,
     4,     5,     6,     7,   103,     9,    23,    11,    12,    13,
    14,    13,    -1,   112,    -1,    -1,    -1,    -1,   117,    -1,
    -1,    -1,   121,    -1,    -1,    -1,    -1,    -1,    32,    33,
    -1,    35,    36,    37,    -1,    39,    40,    41,    42,    43,
    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,
    -1,    -1,    56,    57,     3,     4,     5,     6,     7,    -1,
     9,    -1,    11,    12,    13,    14,    11,    12,    13,    14,
    -1,    -1,    -1,    -1,    11,    12,    13,    14,    -1,    -1,
    -1,    -1,    -1,    32,    33,    -1,    35,    36,    37,    -1,
    39,    40,    37,    42,    43,    40,    -1,    42,    43,    -1,
    37,    50,    39,    40,    -1,    50,    -1,    56,    57,    -1,
    -1,    56,    57,    50,    11,    12,    13,    14,    -1,    56,
    57,    -1,    11,    12,    13,    14,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    11,    12,    13,    14,
    37,    -1,    -1,    40,    11,    12,    13,    14,    37,    46,
    -1,    40,    -1,    50,    -1,    -1,    -1,    46,    -1,    56,
    57,    50,    37,    38,    -1,    40,    -1,    56,    57,    -1,
    37,    -1,    -1,    40,    -1,    50,    -1,    -1,    -1,    -1,
    -1,    56,    57,    50,    -1,    -1,    -1,    -1,    -1,    56,
    57,    15,    16,    17,    18,    19,    20,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    37,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    45,    -1,    47,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    55
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/opt/local/alt/share/bison.simple"
/* This file comes from bison-1.27.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 217 "/opt/local/alt/share/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#if 0
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
int yyparse (void *);
#else
int yyparse (void);
#endif
#endif
#endif

/*
** Things that were local to yyparse
*/
int yystate;
int yyn;
short *yyssp;
YYSTYPE *yyvsp;
int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

short	yyssa[YYINITDEPTH];	/*  the state stack			*/
YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
YYLTYPE *yyls = yylsa;
YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

int yystacksize = YYINITDEPTH;
int yyfree_stacks = 0;

#ifdef YYPURE
int yychar;
YYSTYPE yylval;
int yynerrs;
#ifdef YYLSP_NEEDED
YYLTYPE yylloc;
#endif
#endif

YYSTYPE yyval;		/*  the variable used to return		*/
/*  semantic values from the action	*/
/*  routines				*/

int yylen;
int yysetup = 0;

int
yyparse(int yychar, YYSTYPE yylval)
{
	int yytmpchar = YYEMPTY;

    if (!yysetup) {
		yytmpchar = yychar;

#if YYDEBUG != 0
        if (yydebug)
            fprintf(stderr, "Starting parse\n");
#endif

        yystate = 0;
        yyerrstatus = 0;
        yynerrs = 0;
        yychar = YYEMPTY;		/* Cause a token to be read.  */

        /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

        yyssp = yyss - 1;
        yyvsp = yyvs;
#ifdef YYLSP_NEEDED
        yylsp = yyls;
#endif
        yysetup = 1;
    
/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      if (yytmpchar != YYEMPTY) {
          yychar = yytmpchar;
          yytmpchar = YYEMPTY;
      } else {
          return 0; /* ask for more input. */
      }
    }
  }
  /* start here with new input */

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 1:
#line 41 "parser.y"
{ curnode = yyval = yyvsp[0];  YYACCEPT; ;
    break;}
case 2:
#line 43 "parser.y"
{ 
                indent = 0; 
                curnode = NULL; 
                YYACCEPT;
            ;
    break;}
case 4:
#line 53 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 5:
#line 54 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 6:
#line 55 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 7:
#line 56 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 8:
#line 57 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 9:
#line 58 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 10:
#line 59 "parser.y"
{ yyval = pp_shell(yytext); ;
    break;}
case 11:
#line 60 "parser.y"
{ YYABORT; ;
    break;}
case 12:
#line 61 "parser.y"
{ yyval = NULL; ;
    break;}
case 13:
#line 63 "parser.y"
{ yyval = NULL; ;
    break;}
case 14:
#line 68 "parser.y"
{ yyval = yyvsp[-1]; ;
    break;}
case 15:
#line 72 "parser.y"
{ yyval = pp_help(NULL); ;
    break;}
case 16:
#line 73 "parser.y"
{ yyval = pp_help(yyvsp[0]); ;
    break;}
case 17:
#line 74 "parser.y"
{ yyval = pp_help(yyvsp[0]); ;
    break;}
case 18:
#line 75 "parser.y"
{ yyval = pp_help(NULL); ;
    break;}
case 19:
#line 76 "parser.y"
{ yyval = pp_help(yyvsp[0]); ;
    break;}
case 20:
#line 80 "parser.y"
{ yyval = NULL; ;
    break;}
case 21:
#line 81 "parser.y"
{ yyval = p_mknod(ID_LINE, yyvsp[-1], yyvsp[0]); ;
    break;}
case 22:
#line 85 "parser.y"
{ yyval = yyvsp[-1]; ;
    break;}
case 23:
#line 89 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 24:
#line 90 "parser.y"
{ yyval = p_rlist(ID_LIST, yyvsp[-1], yyvsp[0]); ;
    break;}
case 25:
#line 95 "parser.y"
{ yyval = p_mknod(ID_IF, yyvsp[-4], p_mknod(ID_ELSE, yyvsp[-2], yyvsp[0])); ;
    break;}
case 26:
#line 97 "parser.y"
{ yyval = p_mknod(ID_IF, yyvsp[-2], yyvsp[0]); ;
    break;}
case 27:
#line 102 "parser.y"
{  
										if (pp_str && strlen(pp_str) > 1) {
											yyval = p_mkval(ID_STRING, pp_str); 
										}
                                    ;
    break;}
case 28:
#line 107 "parser.y"
{ 
                                        pp_count = 0; 
										if (pp_str && strlen(pp_str) > 1) {
											yyval = p_mkval(ID_STRING, pp_str); 
										}
                                    ;
    break;}
case 29:
#line 117 "parser.y"
{ yyval = p_mknod(ID_WHILE, yyvsp[-2], yyvsp[0]); ;
    break;}
case 30:
#line 119 "parser.y"
{ yyval = p_mknod(ID_LIST, yyvsp[-4],
                                               p_mknod(ID_WHILE, yyvsp[-3],
                                               p_mknod(ID_FOR, yyvsp[0], yyvsp[-2]))); ;
    break;}
case 31:
#line 123 "parser.y"
{ yyval = p_mknod(ID_LIST, yyvsp[-3],
                                               p_mknod(ID_WHILE, yyvsp[-2],
                                               p_mknod(ID_FOR, yyvsp[0], NULL))); ;
    break;}
case 32:
#line 129 "parser.y"
{ yyval = p_mknod(ID_LINE, 
                                        p_mknod(ID_CONT,NULL,NULL),
                                        yyvsp[0]); 
                                  ;
    break;}
case 33:
#line 133 "parser.y"
{ yyval = p_mknod(ID_LINE,
                                         p_mknod(ID_BREAK,NULL,NULL),
                                         yyvsp[0]); 
                                    ;
    break;}
case 34:
#line 137 "parser.y"
{ yyval = p_mknod(ID_LINE, 
                                    p_mknod(ID_RETURN,yyvsp[-1],NULL),
                                    yyvsp[0]); ;
    break;}
case 35:
#line 143 "parser.y"
{ yyval = NULL; ;
    break;}
case 36:
#line 144 "parser.y"
{ yyval = p_mknod(ID_ARGS, NULL, yyvsp[0]); ;
    break;}
case 37:
#line 145 "parser.y"
{ yyval = p_mknod(ID_ARGS, yyvsp[-2], yyvsp[0]); ;
    break;}
case 38:
#line 149 "parser.y"
{ yyval = p_mknod(ID_ARG, NULL, yyvsp[0]); ;
    break;}
case 39:
#line 150 "parser.y"
{ yyval = p_mknod(ID_ARG, yyvsp[-2], yyvsp[0]); ;
    break;}
case 40:
#line 154 "parser.y"
{ yyval = p_mknod(ID_RANGES, NULL, yyvsp[0]); ;
    break;}
case 41:
#line 155 "parser.y"
{ yyval = p_mknod(ID_RANGES, yyvsp[-2], yyvsp[0]) ; ;
    break;}
case 42:
#line 159 "parser.y"
{ yyval = p_mknod(ID_RANGE, NULL, NULL); ;
    break;}
case 43:
#line 160 "parser.y"
{ yyval = p_mknod(ID_RANGE, yyvsp[0], yyvsp[0]); ;
    break;}
case 44:
#line 161 "parser.y"
{ yyval = p_mknod(ID_RSTEP, NULL, yyvsp[0]); ;
    break;}
case 45:
#line 162 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 46:
#line 163 "parser.y"
{ yyval = p_mknod(ID_RSTEP, yyvsp[-1], NULL); ;
    break;}
case 47:
#line 164 "parser.y"
{ yyval = p_mknod(ID_RSTEP, yyvsp[-2], yyvsp[0]); ;
    break;}
case 48:
#line 168 "parser.y"
{ yyval = p_mknod(ID_RANGE, NULL, yyvsp[0]); ;
    break;}
case 49:
#line 169 "parser.y"
{ yyval = p_mknod(ID_RANGE, yyvsp[-1], NULL); ;
    break;}
case 50:
#line 170 "parser.y"
{ yyval = p_mknod(ID_RANGE, yyvsp[-2], yyvsp[0]); ;
    break;}
case 51:
#line 174 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 52:
#line 175 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 53:
#line 179 "parser.y"
{ yyval = p_mknod(ID_SET,yyvsp[-2],yyvsp[0]); ;
    break;}
case 54:
#line 180 "parser.y"
{ yyval = p_mknod(ID_INC,yyvsp[-1],p_mkval(ID_IVAL, (char *)strdup("1"))); ;
    break;}
case 55:
#line 181 "parser.y"
{ yyval = p_mknod(ID_DEC,yyvsp[-1],p_mkval(ID_IVAL, (char *)strdup("1"))); ;
    break;}
case 56:
#line 182 "parser.y"
{ yyval = p_mknod(ID_INC,yyvsp[-2],yyvsp[0]); ;
    break;}
case 57:
#line 183 "parser.y"
{ yyval = p_mknod(ID_DEC,yyvsp[-2],yyvsp[0]); ;
    break;}
case 58:
#line 184 "parser.y"
{ yyval = p_mknod(ID_MULSET,yyvsp[-2],yyvsp[0]); ;
    break;}
case 59:
#line 185 "parser.y"
{ yyval = p_mknod(ID_DIVSET,yyvsp[-2],yyvsp[0]); ;
    break;}
case 60:
#line 187 "parser.y"
{ yyval = p_mknod(ID_SET,
                                               p_mknod(ID_WHERE, yyvsp[-6], yyvsp[-3]), yyvsp[0]); ;
    break;}
case 61:
#line 193 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 62:
#line 194 "parser.y"
{ yyval = p_mknod(ID_CAT, yyvsp[-2], yyvsp[0]); ;
    break;}
case 63:
#line 198 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 64:
#line 199 "parser.y"
{ yyval = p_mknod(ID_OR ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 65:
#line 203 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 66:
#line 204 "parser.y"
{ yyval = p_mknod(ID_AND ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 67:
#line 208 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 68:
#line 209 "parser.y"
{ yyval = p_mknod(ID_EQ ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 69:
#line 210 "parser.y"
{ yyval = p_mknod(ID_NE ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 70:
#line 214 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 71:
#line 215 "parser.y"
{ yyval = p_mknod(ID_LT ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 72:
#line 216 "parser.y"
{ yyval = p_mknod(ID_GT ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 73:
#line 217 "parser.y"
{ yyval = p_mknod(ID_LE ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 74:
#line 218 "parser.y"
{ yyval = p_mknod(ID_GE ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 75:
#line 222 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 76:
#line 223 "parser.y"
{ yyval = p_mknod(ID_LSHIFT,yyvsp[-2],yyvsp[0]); ;
    break;}
case 77:
#line 224 "parser.y"
{ yyval = p_mknod(ID_RSHIFT,yyvsp[-2],yyvsp[0]); ;
    break;}
case 78:
#line 228 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 79:
#line 229 "parser.y"
{ yyval = p_mknod(ID_ADD,yyvsp[-2],yyvsp[0]); ;
    break;}
case 80:
#line 230 "parser.y"
{ yyval = p_mknod(ID_SUB,yyvsp[-2],yyvsp[0]); ;
    break;}
case 81:
#line 234 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 82:
#line 235 "parser.y"
{ yyval = p_mknod(ID_MULT, yyvsp[-2], yyvsp[0]); ;
    break;}
case 83:
#line 236 "parser.y"
{ yyval = p_mknod(ID_DIV, yyvsp[-2], yyvsp[0]); ;
    break;}
case 84:
#line 237 "parser.y"
{ yyval = p_mknod(ID_MOD, yyvsp[-2], yyvsp[0]); ;
    break;}
case 85:
#line 241 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 86:
#line 242 "parser.y"
{ yyval = p_mknod(ID_POW,yyvsp[-2],yyvsp[0]); ;
    break;}
case 87:
#line 246 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 88:
#line 247 "parser.y"
{ yyval = p_mknod(ID_UMINUS,NULL,yyvsp[0]); ;
    break;}
case 89:
#line 250 "parser.y"
{ yyval = p_mkval(ID_ID, (char *)yyvsp[0]); free(yyvsp[0]); ;
    break;}
case 90:
#line 251 "parser.y"
{ yyval = p_mkval(ID_IVAL, (char *)yyvsp[0]); free(yyvsp[0]);  ;
    break;}
case 91:
#line 252 "parser.y"
{ yyval = p_mkval(ID_RVAL, (char *)yyvsp[0]); free(yyvsp[0]); ;
    break;}
case 92:
#line 253 "parser.y"
{ yyval = p_mkval(ID_STRING, (char *)yyvsp[0]); free(yyvsp[0]); ;
    break;}
case 93:
#line 257 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 94:
#line 258 "parser.y"
{ yyval = yyvsp[-1]; ;
    break;}
case 95:
#line 263 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 96:
#line 264 "parser.y"
{ yyval = p_mknod(ID_ARRAY,yyvsp[-3],yyvsp[-1]); ;
    break;}
case 97:
#line 265 "parser.y"
{ yyval = p_mknod(ID_DEREF,yyvsp[-2],yyvsp[0]); ;
    break;}
case 98:
#line 266 "parser.y"
{ yyval = p_mknod(ID_FUNCT,yyvsp[-3],yyvsp[-1]); ;
    break;}
case 99:
#line 267 "parser.y"
{ yyval = p_mknod(ID_PARALLEL,yyvsp[-3],yyvsp[-1]); ;
    break;}
case 100:
#line 268 "parser.y"
{ yyval = p_mknod(ID_PARALLEL,NULL,yyvsp[-1]); ;
    break;}
case 101:
#line 269 "parser.y"
{ yyval = pp_exact_help(yyvsp[-3]); ;
    break;}
case 102:
#line 270 "parser.y"
{ yyval = pp_exact_help(yyvsp[-2]); ;
    break;}
case 103:
#line 274 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 104:
#line 275 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 105:
#line 276 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 106:
#line 277 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 107:
#line 278 "parser.y"
{ yyval = p_mknod(ID_ARGV, yyvsp[0], NULL); ;
    break;}
case 108:
#line 279 "parser.y"
{ yyval = p_mknod(ID_ARGV, yyvsp[-3], yyvsp[-1]); ;
    break;}
case 109:
#line 280 "parser.y"
{ yyval = p_mknod(ID_ARGV, yyvsp[0], NULL); ;
    break;}
case 110:
#line 281 "parser.y"
{ yyval = p_mknod(ID_ARGV, yyvsp[-3], yyvsp[-1]); ;
    break;}
case 111:
#line 282 "parser.y"
{ yyval = p_mknod(ID_CONSTRUCT, yyvsp[-1], NULL); ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 559 "/opt/local/alt/share/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  yysetup = 0;
  return 1;

 yyabortlab:
  /* YYABORT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  yysetup = 0;
  return -1;
}
#line 285 "parser.y"




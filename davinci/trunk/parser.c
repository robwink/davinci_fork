
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
#define	MULSET_OP	271
#define	DIVSET_OP	272
#define	LT_OP	273
#define	GT_OP	274
#define	GE_OP	275
#define	LE_OP	276
#define	EQ_OP	277
#define	NE_OP	278
#define	AND_OP	279
#define	OR_OP	280
#define	CAT_OP	281
#define	QUIT	282
#define	HELP	283
#define	LIST	284
#define	FUNC_DEF	285
#define	SHELL	286

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



#define	YYFINAL		182
#define	YYFLAG		-32768
#define	YYNTBASE	53

#define YYTRANSLATE(x) ((unsigned)(x) <= 286 ? yytranslate[x] : 87)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    39,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,    52,    49,     2,     2,    33,
    34,    47,    45,    40,    46,    51,    48,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    42,    38,     2,
    41,     2,    35,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    43,     2,    44,    50,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    36,     2,    37,     2,     2,     2,     2,     2,
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
    27,    28,    29,    30,    31,    32
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     5,     6,     8,    10,    12,    14,    16,    18,
    20,    22,    25,    32,    35,    37,    40,    42,    45,    47,
    50,    54,    56,    59,    67,    73,    75,    77,    83,    91,
    98,   101,   104,   108,   109,   111,   115,   117,   121,   123,
   127,   128,   130,   134,   136,   139,   143,   146,   149,   153,
   155,   157,   161,   165,   169,   173,   177,   185,   187,   191,
   193,   197,   199,   203,   205,   209,   213,   215,   219,   223,
   227,   231,   233,   237,   241,   243,   247,   251,   255,   257,
   261,   263,   266,   268,   270,   272,   274,   276,   280,   282,
   287,   291,   296,   301,   303,   305,   307,   309,   312,   318,
   321,   327
};

static const short yyrhs[] = {    54,
     0,     1,    61,     0,     0,    57,     0,    60,     0,    63,
     0,    58,     0,    62,     0,    55,     0,    32,     0,    28,
     0,    31,    61,     0,    31,    80,    33,    64,    34,    58,
     0,    56,    61,     0,    29,     0,    29,    80,     0,    35,
     0,    35,    80,     0,    61,     0,    69,    61,     0,    36,
    59,    37,     0,    54,     0,    59,    54,     0,     9,    33,
    69,    34,    54,    10,    54,     0,     9,    33,    69,    34,
    54,     0,    38,     0,    39,     0,     3,    33,    69,    34,
    54,     0,     7,    33,    57,    57,    69,    34,    54,     0,
     7,    33,    57,    57,    34,    54,     0,     4,    61,     0,
     5,    61,     0,     6,    69,    61,     0,     0,    65,     0,
    64,    40,    65,     0,    71,     0,    80,    41,    69,     0,
    67,     0,    66,    40,    67,     0,     0,    69,     0,    42,
    42,    69,     0,    68,     0,    68,    42,     0,    68,    42,
    69,     0,    42,    69,     0,    69,    42,     0,    69,    42,
    69,     0,    71,     0,    70,     0,    85,    41,    69,     0,
    85,    16,    69,     0,    85,    15,    69,     0,    85,    17,
    69,     0,    85,    18,    69,     0,    85,    43,     8,    69,
    44,    41,    69,     0,    72,     0,    71,    27,    72,     0,
    73,     0,    72,    26,    73,     0,    74,     0,    73,    25,
    74,     0,    75,     0,    74,    23,    75,     0,    74,    24,
    75,     0,    76,     0,    75,    19,    76,     0,    75,    20,
    76,     0,    75,    22,    76,     0,    75,    21,    76,     0,
    77,     0,    76,    45,    77,     0,    76,    46,    77,     0,
    78,     0,    77,    47,    78,     0,    77,    48,    78,     0,
    77,    49,    78,     0,    79,     0,    78,    50,    79,     0,
    86,     0,    46,    79,     0,    14,     0,    11,     0,    12,
     0,    13,     0,    80,     0,    33,    69,    34,     0,    84,
     0,    85,    43,    66,    44,     0,    85,    51,    80,     0,
    85,    33,    64,    34,     0,    85,    33,    35,    34,     0,
    81,     0,    82,     0,    83,     0,    85,     0,    52,    80,
     0,    52,    80,    43,    69,    44,     0,    52,    81,     0,
    52,    81,    43,    69,    44,     0,    36,    64,    37,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    41,    42,    48,    53,    54,    55,    56,    57,    58,    59,
    60,    61,    62,    68,    72,    73,    74,    75,    79,    80,
    84,    88,    89,    93,    95,   101,   106,   116,   117,   121,
   128,   132,   136,   142,   143,   144,   148,   149,   153,   154,
   158,   159,   160,   161,   162,   163,   167,   168,   169,   173,
   174,   178,   179,   180,   181,   182,   183,   190,   191,   195,
   196,   200,   201,   205,   206,   207,   211,   212,   213,   214,
   215,   219,   220,   221,   225,   226,   227,   228,   232,   233,
   237,   238,   241,   242,   243,   244,   248,   249,   263,   264,
   265,   266,   267,   271,   272,   273,   274,   275,   276,   277,
   278,   279
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","WHILE",
"CONTINUE","BREAK","RETURN","FOR","WHERE","IF","ELSE","IVAL","RVAL","STRING",
"ID","DEC_OP","INC_OP","MULSET_OP","DIVSET_OP","LT_OP","GT_OP","GE_OP","LE_OP",
"EQ_OP","NE_OP","AND_OP","OR_OP","CAT_OP","QUIT","HELP","LIST","FUNC_DEF","SHELL",
"'('","')'","'?'","'{'","'}'","';'","'\\n'","','","'='","':'","'['","']'","'+'",
"'-'","'*'","'/'","'%'","'^'","'.'","'$'","start","statement","command_statement",
"help_statement","expr_stmt","compound_statement","statement_list","selection_statement",
"separator","iteration_statement","jump_statement","args","arg","ranges","range2",
"range","expr","assignment_expr","concat","logical_OR","logical_AND","equality_expr",
"relation_expr","additive_expr","mult_expr","power_expr","unary_expr","id","ival",
"rval","string","primary_expr","postfix_expr","rhs_postfix_expr", NULL
};
#endif

static const short yyr1[] = {     0,
    53,    53,    53,    54,    54,    54,    54,    54,    54,    54,
    54,    54,    54,    55,    56,    56,    56,    56,    57,    57,
    58,    59,    59,    60,    60,    61,    61,    62,    62,    62,
    63,    63,    63,    64,    64,    64,    65,    65,    66,    66,
    67,    67,    67,    67,    67,    67,    68,    68,    68,    69,
    69,    70,    70,    70,    70,    70,    70,    71,    71,    72,
    72,    73,    73,    74,    74,    74,    75,    75,    75,    75,
    75,    76,    76,    76,    77,    77,    77,    77,    78,    78,
    79,    79,    80,    81,    82,    83,    84,    84,    85,    85,
    85,    85,    85,    86,    86,    86,    86,    86,    86,    86,
    86,    86
};

static const short yyr2[] = {     0,
     1,     2,     0,     1,     1,     1,     1,     1,     1,     1,
     1,     2,     6,     2,     1,     2,     1,     2,     1,     2,
     3,     1,     2,     7,     5,     1,     1,     5,     7,     6,
     2,     2,     3,     0,     1,     3,     1,     3,     1,     3,
     0,     1,     3,     1,     2,     3,     2,     2,     3,     1,
     1,     3,     3,     3,     3,     3,     7,     1,     3,     1,
     3,     1,     3,     1,     3,     3,     1,     3,     3,     3,
     3,     1,     3,     3,     1,     3,     3,     3,     1,     3,
     1,     2,     1,     1,     1,     1,     1,     3,     1,     4,
     3,     4,     4,     1,     1,     1,     1,     2,     5,     2,
     5,     3
};

static const short yydefact[] = {     0,
     0,     0,     0,     0,     0,     0,     0,    84,    85,    86,
    83,    11,    15,     0,    10,     0,    17,    34,    26,    27,
     0,     0,     1,     9,     0,     4,     7,     5,    19,     8,
     6,     0,    51,    50,    58,    60,    62,    64,    67,    72,
    75,    79,    87,    94,    95,    96,    89,    97,    81,     2,
     0,    31,    32,    34,     0,     0,     0,    16,    12,     0,
     0,    18,    22,     0,     0,    35,    37,    87,    82,    97,
    98,   100,    14,    20,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    34,     0,    41,     0,     0,    37,    33,
     0,     0,    34,    88,    21,    23,   102,     0,     0,    41,
     0,     0,    59,    61,    63,    65,    66,    68,    69,    71,
    70,    73,    74,    76,    77,    78,    80,    54,    53,    55,
    56,     0,     0,    52,     0,     0,     0,    39,    44,    42,
    91,     0,     0,     0,     0,    36,    38,     0,     0,    93,
    92,     0,     0,    47,    41,    90,    45,    48,    28,     0,
     0,    25,     0,    99,   101,     0,    43,    40,    46,    49,
    30,     0,     0,     0,    13,     0,    29,    24,    57,     0,
     0,     0
};

static const short yydefgoto[] = {   180,
    63,    24,    25,    26,    27,    64,    28,    29,    30,    31,
    65,    66,   137,   138,   139,    32,    33,    34,    35,    36,
    37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
    47,    48,    49
};

static const short yypact[] = {   175,
   -28,    -3,   -28,   -28,   285,     2,    38,-32768,-32768,-32768,
-32768,-32768,    23,     1,-32768,   285,    23,   231,-32768,-32768,
   285,    54,-32768,-32768,   -28,-32768,-32768,-32768,-32768,-32768,
-32768,   -28,-32768,    27,    47,    50,    75,    62,    15,    -6,
    29,-32768,-32768,-32768,-32768,-32768,-32768,   292,-32768,-32768,
   285,-32768,-32768,   285,   -28,   235,   285,-32768,-32768,    52,
    78,-32768,-32768,   187,    37,-32768,   -15,    64,-32768,   -24,
    89,   100,-32768,-32768,   285,   285,   285,   285,   285,   285,
   285,   285,   285,   285,   285,   285,   285,   285,   285,   285,
   285,   285,   285,   102,   285,    20,    23,   112,    27,-32768,
   235,   113,   285,-32768,-32768,-32768,-32768,   285,   285,    34,
   285,   285,    47,    50,    75,    62,    62,    15,    15,    15,
    15,    -6,    -6,    29,    29,    29,-32768,-32768,-32768,-32768,
-32768,   115,    10,-32768,   285,   242,   -18,-32768,    53,   110,
-32768,   231,   278,   231,    18,-32768,-32768,   119,   120,-32768,
-32768,   121,   285,-32768,    34,-32768,   285,   285,-32768,   231,
   132,   157,   133,-32768,-32768,   127,-32768,-32768,-32768,-32768,
-32768,   231,   231,   231,-32768,   285,-32768,-32768,-32768,   170,
   171,-32768
};

static const short yypgoto[] = {-32768,
     0,-32768,-32768,   -50,    14,-32768,-32768,     4,-32768,-32768,
   -90,    87,-32768,    28,-32768,    -2,-32768,   -16,   122,   109,
   125,    61,    21,    77,    71,   -20,     3,   183,-32768,-32768,
-32768,    42,-32768
};


#define	YYLAST		343


static const short yytable[] = {    23,
    69,    67,    55,   133,    50,   101,    52,    53,    94,    19,
    20,    75,   145,    61,    11,    58,    60,    59,   110,    62,
    68,   155,   -50,   -50,    71,   156,    97,   135,    73,    51,
     8,     9,    10,    11,    56,    74,    11,    99,    19,    20,
    86,    87,    88,   151,     8,     9,    10,    11,    98,   108,
   143,   163,    16,    75,   102,    54,    68,   108,   100,    84,
    85,   136,    70,   106,     8,    21,    16,    11,   127,    54,
    57,    22,    76,   107,    77,   136,   108,    99,    89,    21,
    80,    81,    82,    83,   103,    22,    99,   128,   129,   130,
   131,    99,   134,   140,   157,    70,    68,    78,    79,   141,
   118,   119,   120,   121,   109,    68,   147,   140,   148,   149,
    68,   104,     8,     9,    10,    11,    70,    70,    70,    70,
    70,    70,    70,    70,    70,    70,    70,    70,    70,    70,
    70,   111,   152,   154,    16,    70,   132,    54,   116,   117,
   161,   159,   112,   162,    70,   142,   144,    21,   150,    70,
   167,   158,   140,    22,   169,   170,   124,   125,   126,   171,
   122,   123,   164,   165,   166,   172,   173,   176,   174,   181,
   182,   177,   178,   179,    -3,     1,   175,     2,     3,     4,
     5,     6,   168,     7,   114,     8,     9,    10,    11,     2,
     3,     4,     5,     6,   146,     7,   113,     8,     9,    10,
    11,   115,    12,    13,    72,    14,    15,    16,     0,    17,
    18,     0,    19,    20,    12,    13,     0,    14,    15,    16,
    21,    17,    18,   105,    19,    20,    22,     0,     0,     0,
     0,     0,    21,     2,     3,     4,     5,     6,    22,     7,
     0,     8,     9,    10,    11,     8,     9,    10,    11,     0,
     0,     0,     8,     9,    10,    11,     0,     0,    12,    13,
     0,    14,    15,    16,     0,    17,    18,    16,    19,    20,
    54,     0,    19,    20,    16,     0,    21,    54,     0,     0,
    21,     0,    22,   153,     0,     0,    22,    21,     8,     9,
    10,    11,     0,    22,     0,     8,     9,    10,    11,     0,
     0,     0,     0,     0,     0,     0,    90,    91,    92,    93,
    16,   160,     0,    54,     0,     0,     0,    16,     0,     0,
    54,     0,     0,    21,    94,     0,     0,     0,     0,    22,
    21,     0,    95,     0,    96,     0,    22,     0,     0,     0,
     0,     0,    97
};

static const short yycheck[] = {     0,
    21,    18,     5,    94,     1,    56,     3,     4,    33,    38,
    39,    27,   103,    16,    14,    13,    14,    14,    43,    17,
    18,    40,    38,    39,    22,    44,    51,     8,    25,    33,
    11,    12,    13,    14,    33,    32,    14,    54,    38,    39,
    47,    48,    49,    34,    11,    12,    13,    14,    51,    40,
   101,    34,    33,    27,    57,    36,    54,    40,    55,    45,
    46,    42,    21,    64,    11,    46,    33,    14,    89,    36,
    33,    52,    26,    37,    25,    42,    40,    94,    50,    46,
    19,    20,    21,    22,    33,    52,   103,    90,    91,    92,
    93,   108,    95,    96,    42,    54,    94,    23,    24,    97,
    80,    81,    82,    83,    41,   103,   109,   110,   111,   112,
   108,    34,    11,    12,    13,    14,    75,    76,    77,    78,
    79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
    89,    43,   135,   136,    33,    94,    35,    36,    78,    79,
   143,   142,    43,   144,   103,    34,    34,    46,    34,   108,
   153,    42,   155,    52,   157,   158,    86,    87,    88,   160,
    84,    85,    44,    44,    44,    34,    10,    41,    36,     0,
     0,   172,   173,   176,     0,     1,   163,     3,     4,     5,
     6,     7,   155,     9,    76,    11,    12,    13,    14,     3,
     4,     5,     6,     7,   108,     9,    75,    11,    12,    13,
    14,    77,    28,    29,    22,    31,    32,    33,    -1,    35,
    36,    -1,    38,    39,    28,    29,    -1,    31,    32,    33,
    46,    35,    36,    37,    38,    39,    52,    -1,    -1,    -1,
    -1,    -1,    46,     3,     4,     5,     6,     7,    52,     9,
    -1,    11,    12,    13,    14,    11,    12,    13,    14,    -1,
    -1,    -1,    11,    12,    13,    14,    -1,    -1,    28,    29,
    -1,    31,    32,    33,    -1,    35,    36,    33,    38,    39,
    36,    -1,    38,    39,    33,    -1,    46,    36,    -1,    -1,
    46,    -1,    52,    42,    -1,    -1,    52,    46,    11,    12,
    13,    14,    -1,    52,    -1,    11,    12,    13,    14,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    15,    16,    17,    18,
    33,    34,    -1,    36,    -1,    -1,    -1,    33,    -1,    -1,
    36,    -1,    -1,    46,    33,    -1,    -1,    -1,    -1,    52,
    46,    -1,    41,    -1,    43,    -1,    52,    -1,    -1,    -1,
    -1,    -1,    51
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
{ yyval = pp_help(NULL); ;
    break;}
case 18:
#line 75 "parser.y"
{ yyval = pp_help(yyvsp[0]); ;
    break;}
case 19:
#line 79 "parser.y"
{ yyval = NULL; ;
    break;}
case 20:
#line 80 "parser.y"
{ yyval = p_mknod(ID_LINE, yyvsp[-1], yyvsp[0]); ;
    break;}
case 21:
#line 84 "parser.y"
{ yyval = yyvsp[-1]; ;
    break;}
case 22:
#line 88 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 23:
#line 89 "parser.y"
{ yyval = p_rlist(ID_LIST, yyvsp[-1], yyvsp[0]); ;
    break;}
case 24:
#line 94 "parser.y"
{ yyval = p_mknod(ID_IF, yyvsp[-4], p_mknod(ID_ELSE, yyvsp[-2], yyvsp[0])); ;
    break;}
case 25:
#line 96 "parser.y"
{ yyval = p_mknod(ID_IF, yyvsp[-2], yyvsp[0]); ;
    break;}
case 26:
#line 101 "parser.y"
{  
										if (pp_str && strlen(pp_str) > 1) {
											yyval = p_mkval(ID_STRING, pp_str); 
										}
                                    ;
    break;}
case 27:
#line 106 "parser.y"
{ 
                                        pp_count = 0; 
										if (pp_str && strlen(pp_str) > 1) {
											yyval = p_mkval(ID_STRING, pp_str); 
										}
                                    ;
    break;}
case 28:
#line 116 "parser.y"
{ yyval = p_mknod(ID_WHILE, yyvsp[-2], yyvsp[0]); ;
    break;}
case 29:
#line 118 "parser.y"
{ yyval = p_mknod(ID_LIST, yyvsp[-4],
                                               p_mknod(ID_WHILE, yyvsp[-3],
                                               p_mknod(ID_FOR, yyvsp[0], yyvsp[-2]))); ;
    break;}
case 30:
#line 122 "parser.y"
{ yyval = p_mknod(ID_LIST, yyvsp[-3],
                                               p_mknod(ID_WHILE, yyvsp[-2],
                                               p_mknod(ID_FOR, yyvsp[0], NULL))); ;
    break;}
case 31:
#line 128 "parser.y"
{ yyval = p_mknod(ID_LINE, 
                                        p_mknod(ID_CONT,NULL,NULL),
                                        yyvsp[0]); 
                                  ;
    break;}
case 32:
#line 132 "parser.y"
{ yyval = p_mknod(ID_LINE,
                                         p_mknod(ID_BREAK,NULL,NULL),
                                         yyvsp[0]); 
                                    ;
    break;}
case 33:
#line 136 "parser.y"
{ yyval = p_mknod(ID_LINE, 
                                    p_mknod(ID_RETURN,yyvsp[-1],NULL),
                                    yyvsp[0]); ;
    break;}
case 34:
#line 142 "parser.y"
{ yyval = NULL; ;
    break;}
case 35:
#line 143 "parser.y"
{ yyval = p_mknod(ID_ARGS, NULL, yyvsp[0]); ;
    break;}
case 36:
#line 144 "parser.y"
{ yyval = p_mknod(ID_ARGS, yyvsp[-2], yyvsp[0]); ;
    break;}
case 37:
#line 148 "parser.y"
{ yyval = p_mknod(ID_ARG, NULL, yyvsp[0]); ;
    break;}
case 38:
#line 149 "parser.y"
{ yyval = p_mknod(ID_ARG, yyvsp[-2], yyvsp[0]); ;
    break;}
case 39:
#line 153 "parser.y"
{ yyval = p_mknod(ID_RANGES, NULL, yyvsp[0]); ;
    break;}
case 40:
#line 154 "parser.y"
{ yyval = p_mknod(ID_RANGES, yyvsp[-2], yyvsp[0]) ; ;
    break;}
case 41:
#line 158 "parser.y"
{ yyval = p_mknod(ID_RANGE, NULL, NULL); ;
    break;}
case 42:
#line 159 "parser.y"
{ yyval = p_mknod(ID_RANGE, yyvsp[0], yyvsp[0]); ;
    break;}
case 43:
#line 160 "parser.y"
{ yyval = p_mknod(ID_RSTEP, NULL, yyvsp[0]); ;
    break;}
case 44:
#line 161 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 45:
#line 162 "parser.y"
{ yyval = p_mknod(ID_RSTEP, yyvsp[-1], NULL); ;
    break;}
case 46:
#line 163 "parser.y"
{ yyval = p_mknod(ID_RSTEP, yyvsp[-2], yyvsp[0]); ;
    break;}
case 47:
#line 167 "parser.y"
{ yyval = p_mknod(ID_RANGE, NULL, yyvsp[0]); ;
    break;}
case 48:
#line 168 "parser.y"
{ yyval = p_mknod(ID_RANGE, yyvsp[-1], NULL); ;
    break;}
case 49:
#line 169 "parser.y"
{ yyval = p_mknod(ID_RANGE, yyvsp[-2], yyvsp[0]); ;
    break;}
case 50:
#line 173 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 51:
#line 174 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 52:
#line 178 "parser.y"
{ yyval = p_mknod(ID_SET,yyvsp[-2],yyvsp[0]); ;
    break;}
case 53:
#line 179 "parser.y"
{ yyval = p_mknod(ID_INC,yyvsp[-2],yyvsp[0]); ;
    break;}
case 54:
#line 180 "parser.y"
{ yyval = p_mknod(ID_DEC,yyvsp[-2],yyvsp[0]); ;
    break;}
case 55:
#line 181 "parser.y"
{ yyval = p_mknod(ID_MULSET,yyvsp[-2],yyvsp[0]); ;
    break;}
case 56:
#line 182 "parser.y"
{ yyval = p_mknod(ID_DIVSET,yyvsp[-2],yyvsp[0]); ;
    break;}
case 57:
#line 184 "parser.y"
{ yyval = p_mknod(ID_SET,
                                               p_mknod(ID_WHERE, yyvsp[-6], yyvsp[-3]), yyvsp[0]); ;
    break;}
case 58:
#line 190 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 59:
#line 191 "parser.y"
{ yyval = p_mknod(ID_CAT, yyvsp[-2], yyvsp[0]); ;
    break;}
case 60:
#line 195 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 61:
#line 196 "parser.y"
{ yyval = p_mknod(ID_OR ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 62:
#line 200 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 63:
#line 201 "parser.y"
{ yyval = p_mknod(ID_AND ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 64:
#line 205 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 65:
#line 206 "parser.y"
{ yyval = p_mknod(ID_EQ ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 66:
#line 207 "parser.y"
{ yyval = p_mknod(ID_NE ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 67:
#line 211 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 68:
#line 212 "parser.y"
{ yyval = p_mknod(ID_LT ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 69:
#line 213 "parser.y"
{ yyval = p_mknod(ID_GT ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 70:
#line 214 "parser.y"
{ yyval = p_mknod(ID_LE ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 71:
#line 215 "parser.y"
{ yyval = p_mknod(ID_GE ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 72:
#line 219 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 73:
#line 220 "parser.y"
{ yyval = p_mknod(ID_ADD,yyvsp[-2],yyvsp[0]); ;
    break;}
case 74:
#line 221 "parser.y"
{ yyval = p_mknod(ID_SUB,yyvsp[-2],yyvsp[0]); ;
    break;}
case 75:
#line 225 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 76:
#line 226 "parser.y"
{ yyval = p_mknod(ID_MULT, yyvsp[-2], yyvsp[0]); ;
    break;}
case 77:
#line 227 "parser.y"
{ yyval = p_mknod(ID_DIV, yyvsp[-2], yyvsp[0]); ;
    break;}
case 78:
#line 228 "parser.y"
{ yyval = p_mknod(ID_MOD, yyvsp[-2], yyvsp[0]); ;
    break;}
case 79:
#line 232 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 80:
#line 233 "parser.y"
{ yyval = p_mknod(ID_POW,yyvsp[-2],yyvsp[0]); ;
    break;}
case 81:
#line 237 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 82:
#line 238 "parser.y"
{ yyval = p_mknod(ID_UMINUS,NULL,yyvsp[0]); ;
    break;}
case 83:
#line 241 "parser.y"
{ yyval = p_mkval(ID_ID, (char *)yyvsp[0]); free(yyvsp[0]); ;
    break;}
case 84:
#line 242 "parser.y"
{ yyval = p_mkval(ID_IVAL, (char *)yyvsp[0]); free(yyvsp[0]);  ;
    break;}
case 85:
#line 243 "parser.y"
{ yyval = p_mkval(ID_RVAL, (char *)yyvsp[0]); free(yyvsp[0]); ;
    break;}
case 86:
#line 244 "parser.y"
{ yyval = p_mkval(ID_STRING, (char *)yyvsp[0]); free(yyvsp[0]); ;
    break;}
case 87:
#line 248 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 88:
#line 249 "parser.y"
{ yyval = yyvsp[-1]; ;
    break;}
case 89:
#line 263 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 90:
#line 264 "parser.y"
{ yyval = p_mknod(ID_ARRAY,yyvsp[-3],yyvsp[-1]); ;
    break;}
case 91:
#line 265 "parser.y"
{ yyval = p_mknod(ID_DEREF,yyvsp[-2],yyvsp[0]); ;
    break;}
case 92:
#line 266 "parser.y"
{ yyval = p_mknod(ID_FUNCT,yyvsp[-3],yyvsp[-1]); ;
    break;}
case 93:
#line 267 "parser.y"
{ yyval = pp_help(yyvsp[-3]); ;
    break;}
case 94:
#line 271 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 95:
#line 272 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 96:
#line 273 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 97:
#line 274 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 98:
#line 275 "parser.y"
{ yyval = p_mknod(ID_ARGV, yyvsp[0], NULL); ;
    break;}
case 99:
#line 276 "parser.y"
{ yyval = p_mknod(ID_ARGV, yyvsp[-3], yyvsp[-1]); ;
    break;}
case 100:
#line 277 "parser.y"
{ yyval = p_mknod(ID_ARGV, yyvsp[0], NULL); ;
    break;}
case 101:
#line 278 "parser.y"
{ yyval = p_mknod(ID_ARGV, yyvsp[-3], yyvsp[-1]); ;
    break;}
case 102:
#line 279 "parser.y"
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
#line 282 "parser.y"





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
#define	LT_OP	271
#define	GT_OP	272
#define	GE_OP	273
#define	LE_OP	274
#define	EQ_OP	275
#define	NE_OP	276
#define	AND_OP	277
#define	OR_OP	278
#define	CAT_OP	279
#define	QUIT	280
#define	HELP	281
#define	LIST	282
#define	FUNC_DEF	283
#define	SHELL	284

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
#ifndef YYSTYPE
#define YYSTYPE int
#endif
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		178
#define	YYFLAG		-32768
#define	YYNTBASE	50

#define YYTRANSLATE(x) ((unsigned)(x) <= 284 ? yytranslate[x] : 83)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    37,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,    49,    47,     2,     2,    31,
    32,    45,    43,    38,    44,     2,    46,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    40,    36,     2,
    39,     2,    33,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    41,     2,    42,    48,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    34,     2,    35,     2,     2,     2,     2,     2,
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
    27,    28,    29,    30
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     5,     6,     8,    10,    12,    14,    16,    18,
    20,    22,    29,    32,    34,    37,    39,    42,    47,    49,
    52,    56,    58,    61,    69,    75,    77,    79,    85,    95,
    96,    98,   101,   104,   108,   109,   111,   115,   117,   121,
   123,   127,   128,   130,   134,   136,   139,   143,   146,   149,
   153,   155,   159,   163,   167,   174,   182,   184,   188,   190,
   194,   196,   200,   202,   206,   210,   212,   216,   220,   224,
   228,   230,   234,   238,   240,   244,   248,   252,   254,   258,
   260,   263,   265,   270,   275,   281,   287,   291,   293,   295,
   297,   299,   302,   305,   311,   313,   315,   317,   319
};

static const short yyrhs[] = {    51,
     0,     1,    58,     0,     0,    54,     0,    57,     0,    61,
     0,    55,     0,    59,     0,    52,     0,    30,     0,    26,
     0,    29,    79,    31,    62,    32,    55,     0,    53,    58,
     0,    27,     0,    27,    79,     0,    33,     0,    33,    79,
     0,    79,    31,    33,    32,     0,    58,     0,    67,    58,
     0,    34,    56,    35,     0,    51,     0,    56,    51,     0,
     9,    31,    67,    32,    51,    10,    51,     0,     9,    31,
    67,    32,    51,     0,    36,     0,    37,     0,     3,    31,
    67,    32,    51,     0,     7,    31,    60,    36,    60,    36,
    60,    32,    51,     0,     0,    67,     0,     4,    58,     0,
     5,    58,     0,     6,    67,    58,     0,     0,    63,     0,
    62,    38,    63,     0,    68,     0,    79,    39,    67,     0,
    65,     0,    64,    38,    65,     0,     0,    67,     0,    40,
    40,    67,     0,    66,     0,    66,    40,     0,    66,    40,
    67,     0,    40,    67,     0,    67,    40,     0,    67,    40,
    67,     0,    68,     0,    79,    39,    67,     0,    79,    16,
    67,     0,    79,    15,    67,     0,    79,    41,    64,    42,
    39,    67,     0,    79,    41,     8,    67,    42,    39,    67,
     0,    69,     0,    68,    25,    69,     0,    70,     0,    69,
    24,    70,     0,    71,     0,    70,    23,    71,     0,    72,
     0,    71,    21,    72,     0,    71,    22,    72,     0,    73,
     0,    72,    17,    73,     0,    72,    18,    73,     0,    72,
    20,    73,     0,    72,    19,    73,     0,    74,     0,    73,
    43,    74,     0,    73,    44,    74,     0,    75,     0,    74,
    45,    75,     0,    74,    46,    75,     0,    74,    47,    75,
     0,    76,     0,    75,    48,    76,     0,    77,     0,    44,
    76,     0,    78,     0,    79,    41,    64,    42,     0,    79,
    31,    62,    32,     0,    49,    79,    31,    62,    32,     0,
    49,    80,    31,    62,    32,     0,    31,    67,    32,     0,
    80,     0,    81,     0,    82,     0,    79,     0,    49,    80,
     0,    49,    79,     0,    49,    79,    41,    67,    42,     0,
    14,     0,    11,     0,    12,     0,    13,     0,    51,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    34,    35,    41,    46,    47,    48,    49,    50,    51,    52,
    53,    54,    58,    62,    63,    64,    65,    66,    70,    71,
    75,    79,    80,    84,    86,    92,    93,    98,    99,   106,
   107,   111,   112,   113,   117,   118,   119,   123,   124,   128,
   129,   133,   134,   135,   136,   137,   138,   142,   143,   144,
   148,   149,   150,   151,   152,   154,   160,   161,   165,   166,
   170,   171,   175,   176,   177,   181,   182,   183,   184,   185,
   189,   190,   191,   195,   196,   197,   198,   202,   203,   207,
   208,   213,   214,   215,   216,   219,   222,   226,   227,   228,
   229,   230,   231,   232,   235,   236,   237,   238,   247
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","WHILE",
"CONTINUE","BREAK","RETURN","FOR","WHERE","IF","ELSE","IVAL","RVAL","STRING",
"ID","DEC_OP","INC_OP","LT_OP","GT_OP","GE_OP","LE_OP","EQ_OP","NE_OP","AND_OP",
"OR_OP","CAT_OP","QUIT","HELP","LIST","FUNC_DEF","SHELL","'('","')'","'?'","'{'",
"'}'","';'","'\\n'","','","'='","':'","'['","']'","'+'","'-'","'*'","'/'","'%'",
"'^'","'$'","start","statement","command_statement","help_statement","expression_statement",
"compound_statement","statement_list","selection_statement","separator","iteration_statement",
"forval","jump_statement","args","arg","ranges","range2","range","expression",
"concat","logical_OR","logical_AND","equality_expr","relation_expr","additive_expr",
"mult_expr","power_expr","unary_expr","postfix_expr","lhs","id","ival","rval",
"string", NULL
};
#endif

static const short yyr1[] = {     0,
    50,    50,    50,    51,    51,    51,    51,    51,    51,    51,
    51,    51,    52,    53,    53,    53,    53,    53,    54,    54,
    55,    56,    56,    57,    57,    58,    58,    59,    59,    60,
    60,    61,    61,    61,    62,    62,    62,    63,    63,    64,
    64,    65,    65,    65,    65,    65,    65,    66,    66,    66,
    67,    67,    67,    67,    67,    67,    68,    68,    69,    69,
    70,    70,    71,    71,    71,    72,    72,    72,    72,    72,
    73,    73,    73,    74,    74,    74,    74,    75,    75,    76,
    76,    77,    77,    77,    77,    77,    77,    78,    78,    78,
    78,    78,    78,    78,    79,    80,    81,    82,    -1
};

static const short yyr2[] = {     0,
     1,     2,     0,     1,     1,     1,     1,     1,     1,     1,
     1,     6,     2,     1,     2,     1,     2,     4,     1,     2,
     3,     1,     2,     7,     5,     1,     1,     5,     9,     0,
     1,     2,     2,     3,     0,     1,     3,     1,     3,     1,
     3,     0,     1,     3,     1,     2,     3,     2,     2,     3,
     1,     3,     3,     3,     6,     7,     1,     3,     1,     3,
     1,     3,     1,     3,     3,     1,     3,     3,     3,     3,
     1,     3,     3,     1,     3,     3,     3,     1,     3,     1,
     2,     1,     4,     4,     5,     5,     3,     1,     1,     1,
     1,     2,     2,     5,     1,     1,     1,     1,     1
};

static const short yydefact[] = {     0,
     0,     0,     0,     0,     0,     0,     0,    96,    97,    98,
    95,    11,    14,     0,    10,     0,    16,     0,    26,    27,
     0,     0,     1,     9,     0,     4,     7,     5,    19,     8,
     6,     0,    51,    57,    59,    61,    63,    66,    71,    74,
    78,    80,    82,    91,    88,    89,    90,     2,     0,    32,
    33,     0,    91,    30,     0,    15,     0,     0,    17,    22,
     0,    81,    91,    93,    92,    13,    20,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,    35,     0,    42,     0,    34,    35,
     0,    31,     0,    35,    87,    21,    23,    42,    35,     0,
    35,    58,    60,    62,    64,    65,    67,    68,    70,    69,
    72,    73,    75,    76,    77,    79,    54,    53,     0,     0,
    36,    38,    91,    52,     0,     0,     0,    40,    45,    43,
     0,    30,     0,     0,     0,     0,     0,     0,    18,    84,
     0,     0,     0,     0,    48,    42,    83,    46,    49,    28,
     0,    25,     0,    83,    85,    94,    86,    37,    39,     0,
    44,    41,     0,    47,    50,    30,     0,    12,     0,    55,
     0,    24,    56,     0,    29,     0,     0,     0
};

static const short yydefgoto[] = {   176,
    23,    24,    25,    26,    27,    61,    28,    29,    30,    91,
    31,   120,   121,   127,   128,   129,    32,    33,    34,    35,
    36,    37,    38,    39,    40,    41,    42,    43,    53,    45,
    46,    47
};

static const short yypact[] = {   201,
    68,   -22,    68,    68,    99,   -15,    -3,-32768,-32768,-32768,
-32768,-32768,    31,    31,-32768,    99,    31,   295,-32768,-32768,
    99,     9,-32768,-32768,    68,-32768,-32768,-32768,-32768,-32768,
-32768,    68,    39,    36,    61,    35,    23,    93,    77,    43,
-32768,-32768,-32768,   130,-32768,-32768,-32768,-32768,    99,-32768,
-32768,    68,   148,    99,    99,-32768,    69,    70,-32768,-32768,
   248,-32768,     5,     6,    84,-32768,-32768,    99,    99,    99,
    99,    99,    99,    99,    99,    99,    99,    99,    99,    99,
    99,    99,    99,    99,    18,    99,    95,    87,-32768,    99,
    89,-32768,   106,    99,-32768,-32768,-32768,   141,    99,    99,
    99,    36,    61,    35,    23,    23,    93,    93,    93,    93,
    77,    77,    43,    43,    43,-32768,-32768,-32768,   108,    20,
-32768,    39,    -6,-32768,    99,   180,   -31,-32768,   102,   107,
   295,    99,   295,    27,   -30,    54,   109,    55,-32768,-32768,
    99,    99,   114,    99,-32768,   141,   110,    99,    99,-32768,
   122,   152,   131,-32768,-32768,-32768,-32768,-32768,-32768,   129,
-32768,-32768,    99,-32768,-32768,    99,   295,-32768,    99,-32768,
   138,-32768,-32768,   295,-32768,   173,   175,-32768
};

static const short yypgoto[] = {-32768,
   -17,-32768,-32768,-32768,    24,-32768,-32768,     2,-32768,  -128,
-32768,   -46,    45,    86,    42,-32768,    34,   -75,   149,   140,
   146,   124,    22,   121,    48,   -19,-32768,-32768,     0,   196,
-32768,-32768
};


#define	YYLAST		344


static const short yytable[] = {    44,
    60,    62,    48,   151,    50,    51,   146,   146,    49,   122,
   147,   154,    56,    57,   122,    54,    59,    44,   122,     8,
    63,    64,    11,   122,    90,   122,    66,    55,     8,     9,
    10,    11,   142,    67,    98,    90,    99,   171,    52,    73,
    74,    75,    76,    97,    11,    98,   100,   134,    16,    58,
   119,   140,   136,    89,   138,    71,    72,   141,   153,    69,
    44,    21,   116,    68,   141,   122,    22,    63,    63,    63,
    63,    63,    63,    63,    63,    63,    63,    63,    63,    63,
    63,    63,    88,    70,   123,   155,   157,    92,    93,   123,
    82,   141,   141,   123,   107,   108,   109,   110,   123,    94,
   123,    95,   125,    19,    20,     8,     9,    10,    11,     8,
     9,    10,    11,   150,   101,   152,   117,   118,   131,   124,
   130,    79,    80,    81,   132,    16,   113,   114,   115,    16,
    44,   130,    44,   137,   126,    77,    78,   133,    21,   139,
   123,   148,    21,    22,    83,    84,   149,    22,   163,   172,
   156,     8,     9,    10,    11,   160,   175,   166,   143,   145,
    85,   167,    83,    84,    18,    92,    44,   169,    86,   174,
    87,    16,   177,    44,   178,   159,   168,   161,    90,   130,
   126,   164,   165,   135,    21,   158,    86,   162,    87,    22,
     8,     9,    10,    11,   105,   106,   170,   111,   112,    92,
    -3,     1,   173,     2,     3,     4,     5,     6,   103,     7,
    16,     8,     9,    10,    11,   104,   102,    65,     0,   144,
     0,     0,     0,    21,     0,     0,    12,    13,    22,    14,
    15,    16,     0,    17,    18,     0,    19,    20,     0,     0,
     0,     0,     0,     0,    21,     0,     0,     0,     0,    22,
     2,     3,     4,     5,     6,     0,     7,     0,     8,     9,
    10,    11,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    12,    13,     0,    14,    15,    16,     0,
    17,    18,    96,    19,    20,     0,     0,     0,     0,     0,
     0,    21,     0,     0,     0,     0,    22,     2,     3,     4,
     5,     6,     0,     7,     0,     8,     9,    10,    11,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    12,    13,     0,    14,    15,    16,     0,    17,    18,     0,
    19,    20,     0,     0,     0,     0,     0,     0,    21,     0,
     0,     0,     0,    22
};

static const short yycheck[] = {     0,
    18,    21,     1,   132,     3,     4,    38,    38,    31,    85,
    42,    42,    13,    14,    90,    31,    17,    18,    94,    11,
    21,    22,    14,    99,    31,   101,    25,    31,    11,    12,
    13,    14,    39,    32,    41,    31,    31,   166,     5,    17,
    18,    19,    20,    61,    14,    41,    41,    94,    31,    16,
    33,    32,    99,    52,   101,    21,    22,    38,    32,    24,
    61,    44,    82,    25,    38,   141,    49,    68,    69,    70,
    71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
    81,    82,    49,    23,    85,    32,    32,    54,    55,    90,
    48,    38,    38,    94,    73,    74,    75,    76,    99,    31,
   101,    32,     8,    36,    37,    11,    12,    13,    14,    11,
    12,    13,    14,   131,    31,   133,    83,    84,    32,    86,
    87,    45,    46,    47,    36,    31,    79,    80,    81,    31,
   131,    98,   133,   100,    40,    43,    44,    32,    44,    32,
   141,    40,    44,    49,    15,    16,    40,    49,    39,   167,
    42,    11,    12,    13,    14,    42,   174,    36,   125,   126,
    31,    10,    15,    16,    34,   132,   167,    39,    39,    32,
    41,    31,     0,   174,     0,   142,   153,   144,    31,   146,
    40,   148,   149,    98,    44,   141,    39,   146,    41,    49,
    11,    12,    13,    14,    71,    72,   163,    77,    78,   166,
     0,     1,   169,     3,     4,     5,     6,     7,    69,     9,
    31,    11,    12,    13,    14,    70,    68,    22,    -1,    40,
    -1,    -1,    -1,    44,    -1,    -1,    26,    27,    49,    29,
    30,    31,    -1,    33,    34,    -1,    36,    37,    -1,    -1,
    -1,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,    49,
     3,     4,     5,     6,     7,    -1,     9,    -1,    11,    12,
    13,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    26,    27,    -1,    29,    30,    31,    -1,
    33,    34,    35,    36,    37,    -1,    -1,    -1,    -1,    -1,
    -1,    44,    -1,    -1,    -1,    -1,    49,     3,     4,     5,
     6,     7,    -1,     9,    -1,    11,    12,    13,    14,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    26,    27,    -1,    29,    30,    31,    -1,    33,    34,    -1,
    36,    37,    -1,    -1,    -1,    -1,    -1,    -1,    44,    -1,
    -1,    -1,    -1,    49
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
#line 34 "parser.y"
{ curnode = yyval = yyvsp[0];  YYACCEPT; ;
    break;}
case 2:
#line 36 "parser.y"
{ 
				indent = 0; 
			  	curnode = NULL; 
			  	YYACCEPT;
			;
    break;}
case 4:
#line 46 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 5:
#line 47 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 6:
#line 48 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 7:
#line 49 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 8:
#line 50 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 9:
#line 51 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 10:
#line 52 "parser.y"
{ yyval = pp_shell(yytext); ;
    break;}
case 11:
#line 53 "parser.y"
{ YYABORT; ;
    break;}
case 12:
#line 54 "parser.y"
{ yyval = NULL; ;
    break;}
case 13:
#line 58 "parser.y"
{ yyval = yyvsp[-1]; ;
    break;}
case 14:
#line 62 "parser.y"
{ yyval = pp_help(NULL); ;
    break;}
case 15:
#line 63 "parser.y"
{ yyval = pp_help(yyvsp[0]); ;
    break;}
case 16:
#line 64 "parser.y"
{ yyval = pp_help(NULL); ;
    break;}
case 17:
#line 65 "parser.y"
{ yyval = pp_help(yyvsp[0]); ;
    break;}
case 18:
#line 66 "parser.y"
{ yyval = pp_help(yyvsp[-3]); ;
    break;}
case 19:
#line 70 "parser.y"
{ yyval = NULL; ;
    break;}
case 20:
#line 71 "parser.y"
{ yyval = yyvsp[-1]; ;
    break;}
case 21:
#line 75 "parser.y"
{ yyval = yyvsp[-1]; ;
    break;}
case 22:
#line 79 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 23:
#line 80 "parser.y"
{ yyval = p_rlist(ID_LIST, yyvsp[-1], yyvsp[0]); ;
    break;}
case 24:
#line 85 "parser.y"
{ yyval = p_mknod(ID_IF, yyvsp[-4], p_mknod(ID_ELSE, yyvsp[-2], yyvsp[0])); ;
    break;}
case 25:
#line 87 "parser.y"
{ yyval = p_mknod(ID_IF, yyvsp[-2], yyvsp[0]); ;
    break;}
case 26:
#line 92 "parser.y"
{ yyval = NULL; ;
    break;}
case 27:
#line 93 "parser.y"
{ pp_count = 0; yyval = NULL; ;
    break;}
case 28:
#line 98 "parser.y"
{ yyval = p_mknod(ID_WHILE, yyvsp[-2], yyvsp[0]); ;
    break;}
case 29:
#line 100 "parser.y"
{ yyval = p_mknod(ID_LIST, yyvsp[-6],
                                                   p_mknod(ID_WHILE, yyvsp[-4],
                                                   p_mknod(ID_FOR, yyvsp[0], yyvsp[-2]))); ;
    break;}
case 30:
#line 106 "parser.y"
{ yyval = NULL; ;
    break;}
case 31:
#line 107 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 32:
#line 111 "parser.y"
{ yyval = p_mknod(ID_CONT,NULL,NULL); ;
    break;}
case 33:
#line 112 "parser.y"
{ yyval = p_mknod(ID_BREAK,NULL,NULL); ;
    break;}
case 34:
#line 113 "parser.y"
{ yyval = p_mknod(ID_RETURN,yyvsp[-1],NULL); ;
    break;}
case 35:
#line 117 "parser.y"
{ yyval = NULL; ;
    break;}
case 36:
#line 118 "parser.y"
{ yyval = p_mknod(ID_ARGS, NULL, yyvsp[0]); ;
    break;}
case 37:
#line 119 "parser.y"
{ yyval = p_mknod(ID_ARGS, yyvsp[-2], yyvsp[0]); ;
    break;}
case 38:
#line 123 "parser.y"
{ yyval = p_mknod(ID_ARG, NULL, yyvsp[0]); ;
    break;}
case 39:
#line 124 "parser.y"
{ yyval = p_mknod(ID_ARG, yyvsp[-2], yyvsp[0]); ;
    break;}
case 40:
#line 128 "parser.y"
{ yyval = p_mknod(ID_RANGES, NULL, yyvsp[0]); ;
    break;}
case 41:
#line 129 "parser.y"
{ yyval = p_mknod(ID_RANGES, yyvsp[-2], yyvsp[0]) ; ;
    break;}
case 42:
#line 133 "parser.y"
{ yyval = p_mknod(ID_RANGE, NULL, NULL); ;
    break;}
case 43:
#line 134 "parser.y"
{ yyval = p_mknod(ID_RANGE, yyvsp[0], yyvsp[0]); ;
    break;}
case 44:
#line 135 "parser.y"
{ yyval = p_mknod(ID_RSTEP, NULL, yyvsp[0]); ;
    break;}
case 45:
#line 136 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 46:
#line 137 "parser.y"
{ yyval = p_mknod(ID_RSTEP, yyvsp[-1], NULL); ;
    break;}
case 47:
#line 138 "parser.y"
{ yyval = p_mknod(ID_RSTEP, yyvsp[-2], yyvsp[0]); ;
    break;}
case 48:
#line 142 "parser.y"
{ yyval = p_mknod(ID_RANGE, NULL, yyvsp[0]); ;
    break;}
case 49:
#line 143 "parser.y"
{ yyval = p_mknod(ID_RANGE, yyvsp[-1], NULL); ;
    break;}
case 50:
#line 144 "parser.y"
{ yyval = p_mknod(ID_RANGE, yyvsp[-2], yyvsp[0]); ;
    break;}
case 51:
#line 148 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 52:
#line 149 "parser.y"
{ yyval = p_mknod(ID_SET,yyvsp[-2],yyvsp[0]); ;
    break;}
case 53:
#line 150 "parser.y"
{ yyval = p_mknod(ID_INC,yyvsp[-2],yyvsp[0]); ;
    break;}
case 54:
#line 151 "parser.y"
{ yyval = p_mknod(ID_DEC,yyvsp[-2],yyvsp[0]); ;
    break;}
case 55:
#line 152 "parser.y"
{ yyval = p_mknod(ID_SET,
                                               p_mknod(ID_ARRAY, yyvsp[-5],yyvsp[-3]), yyvsp[0]); ;
    break;}
case 56:
#line 155 "parser.y"
{ yyval = p_mknod(ID_SET,
                                               p_mknod(ID_WHERE, yyvsp[-6], yyvsp[-3]), yyvsp[0]); ;
    break;}
case 57:
#line 160 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 58:
#line 161 "parser.y"
{ yyval = p_mknod(ID_CAT, yyvsp[-2], yyvsp[0]); ;
    break;}
case 59:
#line 165 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 60:
#line 166 "parser.y"
{ yyval = p_mknod(ID_OR ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 61:
#line 170 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 62:
#line 171 "parser.y"
{ yyval = p_mknod(ID_AND ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 63:
#line 175 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 64:
#line 176 "parser.y"
{ yyval = p_mknod(ID_EQ ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 65:
#line 177 "parser.y"
{ yyval = p_mknod(ID_NE ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 66:
#line 181 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 67:
#line 182 "parser.y"
{ yyval = p_mknod(ID_LT ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 68:
#line 183 "parser.y"
{ yyval = p_mknod(ID_GT ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 69:
#line 184 "parser.y"
{ yyval = p_mknod(ID_LE ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 70:
#line 185 "parser.y"
{ yyval = p_mknod(ID_GE ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 71:
#line 189 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 72:
#line 190 "parser.y"
{ yyval = p_mknod(ID_ADD,yyvsp[-2],yyvsp[0]); ;
    break;}
case 73:
#line 191 "parser.y"
{ yyval = p_mknod(ID_SUB,yyvsp[-2],yyvsp[0]); ;
    break;}
case 74:
#line 195 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 75:
#line 196 "parser.y"
{ yyval = p_mknod(ID_MULT, yyvsp[-2], yyvsp[0]); ;
    break;}
case 76:
#line 197 "parser.y"
{ yyval = p_mknod(ID_DIV, yyvsp[-2], yyvsp[0]); ;
    break;}
case 77:
#line 198 "parser.y"
{ yyval = p_mknod(ID_MOD, yyvsp[-2], yyvsp[0]); ;
    break;}
case 78:
#line 202 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 79:
#line 203 "parser.y"
{ yyval = p_mknod(ID_POW,yyvsp[-2],yyvsp[0]); ;
    break;}
case 80:
#line 207 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 81:
#line 208 "parser.y"
{ yyval = p_mknod(ID_UMINUS,NULL,yyvsp[0]); ;
    break;}
case 82:
#line 213 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 83:
#line 214 "parser.y"
{ yyval = p_mknod(ID_ARRAY,yyvsp[-3],yyvsp[-1]); ;
    break;}
case 84:
#line 215 "parser.y"
{ yyval = p_mknod(ID_FUNCT,yyvsp[-3],yyvsp[-1]); ;
    break;}
case 85:
#line 216 "parser.y"
{ yyval = p_mknod(ID_FUNCT,
                                                    p_mknod(ID_ARGV, yyvsp[-3], NULL),
                                                    yyvsp[-1]); ;
    break;}
case 86:
#line 219 "parser.y"
{ yyval = p_mknod(ID_FUNCT,
                                                    p_mknod(ID_ARGV, yyvsp[-3], NULL),
                                                    yyvsp[-1]); ;
    break;}
case 87:
#line 222 "parser.y"
{ yyval = yyvsp[-1]; ;
    break;}
case 88:
#line 226 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 89:
#line 227 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 90:
#line 228 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 91:
#line 229 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 92:
#line 230 "parser.y"
{ yyval = p_mknod(ID_ARGV, yyvsp[0], NULL); ;
    break;}
case 93:
#line 231 "parser.y"
{ yyval = p_mknod(ID_ARGV, yyvsp[0], NULL); ;
    break;}
case 94:
#line 232 "parser.y"
{ yyval = p_mknod(ID_ARGV, yyvsp[-3], yyvsp[-1]); ;
    break;}
case 95:
#line 235 "parser.y"
{ yyval = p_mkval(ID_ID, (char *)yyvsp[0]); free(yyvsp[0]); ;
    break;}
case 96:
#line 236 "parser.y"
{ yyval = p_mkval(ID_IVAL, (char *)yyvsp[0]); free(yyvsp[0]);  ;
    break;}
case 97:
#line 237 "parser.y"
{ yyval = p_mkval(ID_RVAL, (char *)yyvsp[0]); free(yyvsp[0]); ;
    break;}
case 98:
#line 238 "parser.y"
{ yyval = p_mkval(ID_STRING, (char *)yyvsp[0]); free(yyvsp[0]); ;
    break;}
case 99:
#line 247 "parser.y"
{ 
                                    if (check_ufunc(yyvsp[0])) {
										if (setjmp(env) == 0) {
											evaluate(yyvsp[0]);
											pp_print(pop(scope_tos()));
										} else {
											while (ftos && ftos != stdin)
												pop_input_file();
										}
                                        free_tree(yyvsp[0]);
										cleanup(scope_tos());
                                    }
                                ;
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
#line 262 "parser.y"



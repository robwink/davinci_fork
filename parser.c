
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



#define	YYFINAL		173
#define	YYFLAG		-32768
#define	YYNTBASE	51

#define YYTRANSLATE(x) ((unsigned)(x) <= 284 ? yytranslate[x] : 85)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    37,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,    50,    47,     2,     2,    31,
    32,    45,    43,    38,    44,    49,    46,     2,     2,     2,
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
    20,    22,    29,    32,    34,    37,    39,    42,    44,    47,
    51,    53,    56,    64,    70,    72,    74,    80,    88,    95,
    98,   101,   105,   107,   111,   113,   117,   119,   123,   124,
   126,   130,   132,   135,   139,   142,   145,   149,   151,   153,
   157,   161,   165,   173,   175,   179,   181,   185,   187,   191,
   193,   197,   201,   203,   207,   211,   215,   219,   221,   225,
   229,   231,   235,   239,   243,   245,   249,   251,   254,   256,
   258,   260,   262,   264,   268,   270,   275,   279,   283,   288,
   293,   295,   297,   299,   301,   304,   310,   313
};

static const short yyrhs[] = {    52,
     0,     1,    59,     0,     0,    55,     0,    58,     0,    61,
     0,    56,     0,    60,     0,    53,     0,    30,     0,    26,
     0,    29,    78,    31,    62,    32,    56,     0,    54,    59,
     0,    27,     0,    27,    78,     0,    33,     0,    33,    78,
     0,    59,     0,    67,    59,     0,    34,    57,    35,     0,
    52,     0,    57,    52,     0,     9,    31,    67,    32,    52,
    10,    52,     0,     9,    31,    67,    32,    52,     0,    36,
     0,    37,     0,     3,    31,    67,    32,    52,     0,     7,
    31,    55,    55,    67,    32,    52,     0,     7,    31,    55,
    55,    32,    52,     0,     4,    59,     0,     5,    59,     0,
     6,    67,    59,     0,    63,     0,    62,    38,    63,     0,
    69,     0,    78,    39,    67,     0,    65,     0,    64,    38,
    65,     0,     0,    67,     0,    40,    40,    67,     0,    66,
     0,    66,    40,     0,    66,    40,    67,     0,    40,    67,
     0,    67,    40,     0,    67,    40,    67,     0,    69,     0,
    68,     0,    83,    39,    67,     0,    83,    16,    67,     0,
    83,    15,    67,     0,    83,    41,     8,    67,    42,    39,
    67,     0,    70,     0,    69,    25,    70,     0,    71,     0,
    70,    24,    71,     0,    72,     0,    71,    23,    72,     0,
    73,     0,    72,    21,    73,     0,    72,    22,    73,     0,
    74,     0,    73,    17,    74,     0,    73,    18,    74,     0,
    73,    20,    74,     0,    73,    19,    74,     0,    75,     0,
    74,    43,    75,     0,    74,    44,    75,     0,    76,     0,
    75,    45,    76,     0,    75,    46,    76,     0,    75,    47,
    76,     0,    77,     0,    76,    48,    77,     0,    84,     0,
    44,    77,     0,    14,     0,    11,     0,    12,     0,    13,
     0,    78,     0,    31,    67,    32,     0,    82,     0,    83,
    41,    64,    42,     0,    83,    49,    78,     0,    83,    31,
    32,     0,    83,    31,    62,    32,     0,    83,    31,    33,
    32,     0,    79,     0,    80,     0,    81,     0,    83,     0,
    50,    78,     0,    50,    78,    41,    67,    42,     0,    50,
    79,     0,    50,    79,    41,    67,    42,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    34,    35,    41,    46,    47,    48,    49,    50,    51,    52,
    53,    54,    58,    62,    63,    64,    65,    69,    70,    74,
    78,    79,    83,    85,    91,    92,    97,    98,   102,   109,
   110,   111,   115,   116,   120,   121,   125,   126,   130,   131,
   132,   133,   134,   135,   139,   140,   141,   145,   146,   150,
   151,   152,   153,   160,   161,   165,   166,   170,   171,   175,
   176,   177,   181,   182,   183,   184,   185,   189,   190,   191,
   195,   196,   197,   198,   202,   203,   207,   208,   211,   212,
   213,   214,   218,   219,   222,   223,   224,   225,   226,   227,
   231,   232,   233,   234,   235,   236,   237,   238
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","WHILE",
"CONTINUE","BREAK","RETURN","FOR","WHERE","IF","ELSE","IVAL","RVAL","STRING",
"ID","DEC_OP","INC_OP","LT_OP","GT_OP","GE_OP","LE_OP","EQ_OP","NE_OP","AND_OP",
"OR_OP","CAT_OP","QUIT","HELP","LIST","FUNC_DEF","SHELL","'('","')'","'?'","'{'",
"'}'","';'","'\\n'","','","'='","':'","'['","']'","'+'","'-'","'*'","'/'","'%'",
"'^'","'.'","'$'","start","statement","command_statement","help_statement","expr_stmt",
"compound_statement","statement_list","selection_statement","separator","iteration_statement",
"jump_statement","args","arg","ranges","range2","range","expr","assignment_expr",
"concat","logical_OR","logical_AND","equality_expr","relation_expr","additive_expr",
"mult_expr","power_expr","unary_expr","id","ival","rval","string","primary_expr",
"postfix_expr","rhs_postfix_expr", NULL
};
#endif

static const short yyr1[] = {     0,
    51,    51,    51,    52,    52,    52,    52,    52,    52,    52,
    52,    52,    53,    54,    54,    54,    54,    55,    55,    56,
    57,    57,    58,    58,    59,    59,    60,    60,    60,    61,
    61,    61,    62,    62,    63,    63,    64,    64,    65,    65,
    65,    65,    65,    65,    66,    66,    66,    67,    67,    68,
    68,    68,    68,    69,    69,    70,    70,    71,    71,    72,
    72,    72,    73,    73,    73,    73,    73,    74,    74,    74,
    75,    75,    75,    75,    76,    76,    77,    77,    78,    79,
    80,    81,    82,    82,    83,    83,    83,    83,    83,    83,
    84,    84,    84,    84,    84,    84,    84,    84
};

static const short yyr2[] = {     0,
     1,     2,     0,     1,     1,     1,     1,     1,     1,     1,
     1,     6,     2,     1,     2,     1,     2,     1,     2,     3,
     1,     2,     7,     5,     1,     1,     5,     7,     6,     2,
     2,     3,     1,     3,     1,     3,     1,     3,     0,     1,
     3,     1,     2,     3,     2,     2,     3,     1,     1,     3,
     3,     3,     7,     1,     3,     1,     3,     1,     3,     1,
     3,     3,     1,     3,     3,     3,     3,     1,     3,     3,
     1,     3,     3,     3,     1,     3,     1,     2,     1,     1,
     1,     1,     1,     3,     1,     4,     3,     3,     4,     4,
     1,     1,     1,     1,     2,     5,     2,     5
};

static const short yydefact[] = {     0,
     0,     0,     0,     0,     0,     0,     0,    80,    81,    82,
    79,    11,    14,     0,    10,     0,    16,     0,    25,    26,
     0,     0,     1,     9,     0,     4,     7,     5,    18,     8,
     6,     0,    49,    48,    54,    56,    58,    60,    63,    68,
    71,    75,    83,    91,    92,    93,    85,    94,    77,     2,
     0,    30,    31,     0,     0,     0,    15,     0,     0,    17,
    21,     0,    78,    94,    95,    97,    13,    19,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    39,     0,     0,
    32,     0,     0,     0,    84,    20,    22,    39,     0,     0,
    55,    57,    59,    61,    62,    64,    65,    67,    66,    69,
    70,    72,    73,    74,    76,    52,    51,    88,     0,     0,
    33,    35,    83,    50,     0,     0,     0,    37,    42,    40,
    87,     0,     0,     0,     0,     0,     0,    90,    89,     0,
     0,     0,     0,    45,    39,    86,    43,    46,    27,     0,
     0,    24,     0,    96,    98,    34,    36,     0,    41,    38,
    44,    47,    29,     0,     0,    12,     0,    28,    23,    53,
     0,     0,     0
};

static const short yydefgoto[] = {   171,
    23,    24,    25,    26,    27,    62,    28,    29,    30,    31,
   120,   121,   127,   128,   129,    32,    33,    34,    35,    36,
    37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
    47,    48,    49
};

static const short yypact[] = {   152,
     7,   -16,     7,     7,   278,    -2,     4,-32768,-32768,-32768,
-32768,-32768,    13,    13,-32768,   278,    13,   236,-32768,-32768,
   278,    10,-32768,-32768,     7,-32768,-32768,-32768,-32768,-32768,
-32768,     7,-32768,    12,    15,    25,    31,    79,    47,   -14,
     2,-32768,-32768,-32768,-32768,-32768,-32768,    73,-32768,-32768,
   278,-32768,-32768,     7,     5,   278,-32768,    44,    55,-32768,
-32768,   194,-32768,   -11,    51,    61,-32768,-32768,   278,   278,
   278,   278,   278,   278,   278,   278,   278,   278,   278,   278,
   278,   278,   278,   278,   278,   160,   278,    95,    13,    81,
-32768,     5,    84,   278,-32768,-32768,-32768,   243,   278,   278,
    15,    25,    31,    79,    79,    47,    47,    47,    47,   -14,
   -14,     2,     2,     2,-32768,-32768,-32768,-32768,    86,   -25,
-32768,    12,    66,-32768,   278,   264,   -34,-32768,    83,    85,
-32768,   236,   202,   236,    -4,    77,    82,-32768,-32768,   278,
   278,    87,   278,-32768,   243,-32768,   278,   278,-32768,   236,
    98,   117,   100,-32768,-32768,-32768,-32768,   102,-32768,-32768,
-32768,-32768,-32768,   236,   236,-32768,   278,-32768,-32768,-32768,
   137,   144,-32768
};

static const short yypgoto[] = {-32768,
   -17,-32768,-32768,   -52,    -7,-32768,-32768,    22,-32768,-32768,
    56,     9,-32768,     6,-32768,    -5,-32768,   -84,    91,   105,
   106,    28,    93,    32,   -23,     1,    -8,   132,-32768,-32768,
-32768,    -9,-32768
};


#define	YYLAST		328


static const short yytable[] = {    54,
    61,   122,    92,   145,    57,    58,   139,   146,    60,   122,
    59,    64,   140,    65,    51,     8,     9,    10,    11,    86,
     8,    63,    50,    11,    52,    53,    11,   153,    55,    98,
    80,    81,    82,   140,    56,    16,    69,    89,    70,   133,
    19,    20,    19,    20,    97,    90,    67,    71,    21,    83,
    93,    72,    73,    68,    22,   122,   112,   113,   114,    64,
    64,    64,    64,    64,    64,    64,    64,    64,    64,    64,
    64,    64,    64,    64,    94,    91,    64,   123,   116,   117,
   131,   124,   130,   115,    64,   123,    95,    84,    85,    78,
    79,    99,   130,   136,   137,    74,    75,    76,    77,   104,
   105,   100,   125,    86,   141,     8,     9,    10,    11,   110,
   111,    87,   132,    88,   149,   134,   152,   138,   154,   142,
   144,    89,   147,   155,   148,    16,   165,   151,   158,   164,
    64,   123,   163,    18,   126,   157,   172,   159,    21,   130,
   167,   161,   162,   173,    22,   166,   168,   169,   156,   135,
   160,    -3,     1,    66,     2,     3,     4,     5,     6,   101,
     7,   170,     8,     9,    10,    11,   106,   107,   108,   109,
     8,     9,    10,    11,   102,     0,   103,    12,    13,     0,
    14,    15,    16,     0,    17,    18,     0,    19,    20,     0,
    16,   118,   119,     0,     0,    21,     2,     3,     4,     5,
     6,    22,     7,    21,     8,     9,    10,    11,     0,    22,
     0,     0,     8,     9,    10,    11,     0,     0,     0,    12,
    13,     0,    14,    15,    16,     0,    17,    18,    96,    19,
    20,     0,    16,   150,     0,     0,     0,    21,     2,     3,
     4,     5,     6,    22,     7,    21,     8,     9,    10,    11,
     0,    22,     0,     8,     9,    10,    11,     0,     0,     0,
     0,    12,    13,     0,    14,    15,    16,     0,    17,    18,
     0,    19,    20,    16,     8,     9,    10,    11,     0,    21,
     0,     0,   126,     0,     0,    22,    21,     0,     8,     9,
    10,    11,    22,     0,    16,     0,     0,     0,     0,     0,
     0,     0,     0,   143,     0,     0,     0,    21,    16,     0,
     0,     0,     0,    22,     0,     0,     0,     0,     0,     0,
     0,    21,     0,     0,     0,     0,     0,    22
};

static const short yycheck[] = {     5,
    18,    86,    55,    38,    13,    14,    32,    42,    17,    94,
    16,    21,    38,    22,    31,    11,    12,    13,    14,    31,
    11,    21,     1,    14,     3,     4,    14,    32,    31,    41,
    45,    46,    47,    38,    31,    31,    25,    49,    24,    92,
    36,    37,    36,    37,    62,    51,    25,    23,    44,    48,
    56,    21,    22,    32,    50,   140,    80,    81,    82,    69,
    70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
    80,    81,    82,    83,    31,    54,    86,    86,    84,    85,
    89,    87,    88,    83,    94,    94,    32,    15,    16,    43,
    44,    41,    98,    99,   100,    17,    18,    19,    20,    72,
    73,    41,     8,    31,    39,    11,    12,    13,    14,    78,
    79,    39,    32,    41,   132,    32,   134,    32,    42,   125,
   126,    49,    40,    42,    40,    31,    10,   133,    42,    32,
   140,   140,   150,    34,    40,   141,     0,   143,    44,   145,
    39,   147,   148,     0,    50,   153,   164,   165,   140,    94,
   145,     0,     1,    22,     3,     4,     5,     6,     7,    69,
     9,   167,    11,    12,    13,    14,    74,    75,    76,    77,
    11,    12,    13,    14,    70,    -1,    71,    26,    27,    -1,
    29,    30,    31,    -1,    33,    34,    -1,    36,    37,    -1,
    31,    32,    33,    -1,    -1,    44,     3,     4,     5,     6,
     7,    50,     9,    44,    11,    12,    13,    14,    -1,    50,
    -1,    -1,    11,    12,    13,    14,    -1,    -1,    -1,    26,
    27,    -1,    29,    30,    31,    -1,    33,    34,    35,    36,
    37,    -1,    31,    32,    -1,    -1,    -1,    44,     3,     4,
     5,     6,     7,    50,     9,    44,    11,    12,    13,    14,
    -1,    50,    -1,    11,    12,    13,    14,    -1,    -1,    -1,
    -1,    26,    27,    -1,    29,    30,    31,    -1,    33,    34,
    -1,    36,    37,    31,    11,    12,    13,    14,    -1,    44,
    -1,    -1,    40,    -1,    -1,    50,    44,    -1,    11,    12,
    13,    14,    50,    -1,    31,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    40,    -1,    -1,    -1,    44,    31,    -1,
    -1,    -1,    -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    44,    -1,    -1,    -1,    -1,    -1,    50
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
#line 69 "parser.y"
{ yyval = NULL; ;
    break;}
case 19:
#line 70 "parser.y"
{ yyval = yyvsp[-1]; ;
    break;}
case 20:
#line 74 "parser.y"
{ yyval = yyvsp[-1]; ;
    break;}
case 21:
#line 78 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 22:
#line 79 "parser.y"
{ yyval = p_rlist(ID_LIST, yyvsp[-1], yyvsp[0]); ;
    break;}
case 23:
#line 84 "parser.y"
{ yyval = p_mknod(ID_IF, yyvsp[-4], p_mknod(ID_ELSE, yyvsp[-2], yyvsp[0])); ;
    break;}
case 24:
#line 86 "parser.y"
{ yyval = p_mknod(ID_IF, yyvsp[-2], yyvsp[0]); ;
    break;}
case 25:
#line 91 "parser.y"
{ yyval = NULL; ;
    break;}
case 26:
#line 92 "parser.y"
{ pp_count = 0; yyval = NULL; ;
    break;}
case 27:
#line 97 "parser.y"
{ yyval = p_mknod(ID_WHILE, yyvsp[-2], yyvsp[0]); ;
    break;}
case 28:
#line 99 "parser.y"
{ yyval = p_mknod(ID_LIST, yyvsp[-4],
											   p_mknod(ID_WHILE, yyvsp[-3],
											   p_mknod(ID_FOR, yyvsp[0], yyvsp[-2]))); ;
    break;}
case 29:
#line 103 "parser.y"
{ yyval = p_mknod(ID_LIST, yyvsp[-3],
											   p_mknod(ID_WHILE, yyvsp[-2],
											   p_mknod(ID_FOR, yyvsp[0], NULL))); ;
    break;}
case 30:
#line 109 "parser.y"
{ yyval = p_mknod(ID_CONT,NULL,NULL); ;
    break;}
case 31:
#line 110 "parser.y"
{ yyval = p_mknod(ID_BREAK,NULL,NULL); ;
    break;}
case 32:
#line 111 "parser.y"
{ yyval = p_mknod(ID_RETURN,yyvsp[-1],NULL); ;
    break;}
case 33:
#line 115 "parser.y"
{ yyval = p_mknod(ID_ARGS, NULL, yyvsp[0]); ;
    break;}
case 34:
#line 116 "parser.y"
{ yyval = p_mknod(ID_ARGS, yyvsp[-2], yyvsp[0]); ;
    break;}
case 35:
#line 120 "parser.y"
{ yyval = p_mknod(ID_ARG, NULL, yyvsp[0]); ;
    break;}
case 36:
#line 121 "parser.y"
{ yyval = p_mknod(ID_ARG, yyvsp[-2], yyvsp[0]); ;
    break;}
case 37:
#line 125 "parser.y"
{ yyval = p_mknod(ID_RANGES, NULL, yyvsp[0]); ;
    break;}
case 38:
#line 126 "parser.y"
{ yyval = p_mknod(ID_RANGES, yyvsp[-2], yyvsp[0]) ; ;
    break;}
case 39:
#line 130 "parser.y"
{ yyval = p_mknod(ID_RANGE, NULL, NULL); ;
    break;}
case 40:
#line 131 "parser.y"
{ yyval = p_mknod(ID_RANGE, yyvsp[0], yyvsp[0]); ;
    break;}
case 41:
#line 132 "parser.y"
{ yyval = p_mknod(ID_RSTEP, NULL, yyvsp[0]); ;
    break;}
case 42:
#line 133 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 43:
#line 134 "parser.y"
{ yyval = p_mknod(ID_RSTEP, yyvsp[-1], NULL); ;
    break;}
case 44:
#line 135 "parser.y"
{ yyval = p_mknod(ID_RSTEP, yyvsp[-2], yyvsp[0]); ;
    break;}
case 45:
#line 139 "parser.y"
{ yyval = p_mknod(ID_RANGE, NULL, yyvsp[0]); ;
    break;}
case 46:
#line 140 "parser.y"
{ yyval = p_mknod(ID_RANGE, yyvsp[-1], NULL); ;
    break;}
case 47:
#line 141 "parser.y"
{ yyval = p_mknod(ID_RANGE, yyvsp[-2], yyvsp[0]); ;
    break;}
case 48:
#line 145 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 49:
#line 146 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 50:
#line 150 "parser.y"
{ yyval = p_mknod(ID_SET,yyvsp[-2],yyvsp[0]); ;
    break;}
case 51:
#line 151 "parser.y"
{ yyval = p_mknod(ID_INC,yyvsp[-2],yyvsp[0]); ;
    break;}
case 52:
#line 152 "parser.y"
{ yyval = p_mknod(ID_DEC,yyvsp[-2],yyvsp[0]); ;
    break;}
case 53:
#line 154 "parser.y"
{ yyval = p_mknod(ID_SET,
                                               p_mknod(ID_WHERE, yyvsp[-6], yyvsp[-3]), yyvsp[0]); ;
    break;}
case 54:
#line 160 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 55:
#line 161 "parser.y"
{ yyval = p_mknod(ID_CAT, yyvsp[-2], yyvsp[0]); ;
    break;}
case 56:
#line 165 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 57:
#line 166 "parser.y"
{ yyval = p_mknod(ID_OR ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 58:
#line 170 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 59:
#line 171 "parser.y"
{ yyval = p_mknod(ID_AND ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 60:
#line 175 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 61:
#line 176 "parser.y"
{ yyval = p_mknod(ID_EQ ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 62:
#line 177 "parser.y"
{ yyval = p_mknod(ID_NE ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 63:
#line 181 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 64:
#line 182 "parser.y"
{ yyval = p_mknod(ID_LT ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 65:
#line 183 "parser.y"
{ yyval = p_mknod(ID_GT ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 66:
#line 184 "parser.y"
{ yyval = p_mknod(ID_LE ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 67:
#line 185 "parser.y"
{ yyval = p_mknod(ID_GE ,yyvsp[-2],yyvsp[0]); ;
    break;}
case 68:
#line 189 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 69:
#line 190 "parser.y"
{ yyval = p_mknod(ID_ADD,yyvsp[-2],yyvsp[0]); ;
    break;}
case 70:
#line 191 "parser.y"
{ yyval = p_mknod(ID_SUB,yyvsp[-2],yyvsp[0]); ;
    break;}
case 71:
#line 195 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 72:
#line 196 "parser.y"
{ yyval = p_mknod(ID_MULT, yyvsp[-2], yyvsp[0]); ;
    break;}
case 73:
#line 197 "parser.y"
{ yyval = p_mknod(ID_DIV, yyvsp[-2], yyvsp[0]); ;
    break;}
case 74:
#line 198 "parser.y"
{ yyval = p_mknod(ID_MOD, yyvsp[-2], yyvsp[0]); ;
    break;}
case 75:
#line 202 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 76:
#line 203 "parser.y"
{ yyval = p_mknod(ID_POW,yyvsp[-2],yyvsp[0]); ;
    break;}
case 77:
#line 207 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 78:
#line 208 "parser.y"
{ yyval = p_mknod(ID_UMINUS,NULL,yyvsp[0]); ;
    break;}
case 79:
#line 211 "parser.y"
{ yyval = p_mkval(ID_ID, (char *)yyvsp[0]); free(yyvsp[0]); ;
    break;}
case 80:
#line 212 "parser.y"
{ yyval = p_mkval(ID_IVAL, (char *)yyvsp[0]); free(yyvsp[0]);  ;
    break;}
case 81:
#line 213 "parser.y"
{ yyval = p_mkval(ID_RVAL, (char *)yyvsp[0]); free(yyvsp[0]); ;
    break;}
case 82:
#line 214 "parser.y"
{ yyval = p_mkval(ID_STRING, (char *)yyvsp[0]); free(yyvsp[0]); ;
    break;}
case 83:
#line 218 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 84:
#line 219 "parser.y"
{ yyval = yyvsp[-1]; ;
    break;}
case 85:
#line 222 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 86:
#line 223 "parser.y"
{ yyval = p_mknod(ID_ARRAY,yyvsp[-3],yyvsp[-1]); ;
    break;}
case 87:
#line 224 "parser.y"
{ yyval = NULL; ;
    break;}
case 88:
#line 225 "parser.y"
{ yyval = p_mknod(ID_FUNCT,yyvsp[-2],NULL); ;
    break;}
case 89:
#line 226 "parser.y"
{ yyval = p_mknod(ID_FUNCT,yyvsp[-3],yyvsp[-1]); ;
    break;}
case 90:
#line 227 "parser.y"
{ yyval = pp_help(yyvsp[-3]); ;
    break;}
case 91:
#line 231 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 92:
#line 232 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 93:
#line 233 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 94:
#line 234 "parser.y"
{ yyval = yyvsp[0]; ;
    break;}
case 95:
#line 235 "parser.y"
{ yyval = p_mknod(ID_ARGV, yyvsp[0], NULL); ;
    break;}
case 96:
#line 236 "parser.y"
{ yyval = p_mknod(ID_ARGV, yyvsp[-3], yyvsp[-1]); ;
    break;}
case 97:
#line 237 "parser.y"
{ yyval = p_mknod(ID_ARGV, yyvsp[0], NULL); ;
    break;}
case 98:
#line 238 "parser.y"
{ yyval = p_mknod(ID_ARGV, yyvsp[-3], yyvsp[-1]); ;
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
#line 241 "parser.y"




#line 2 "push.skel"
/*
**    "@(#)push.skel, based on byacc 1.8 (Berkeley) 01/20/91";
*/
#define YYBTYACC 1

#line 2 "parser.y"
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
#line 23 "y_tab.c"
#define WHILE 257
#define CONTINUE 258
#define BREAK 259
#define RETURN 260
#define FOR 261
#define WHERE 262
#define IF 263
#define ELSE 264
#define IVAL 265
#define RVAL 266
#define STRING 267
#define ID 268
#define DEC_OP 269
#define INC_OP 270
#define LT_OP 271
#define GT_OP 272
#define GE_OP 273
#define LE_OP 274
#define EQ_OP 275
#define NE_OP 276
#define AND_OP 277
#define OR_OP 278
#define CAT_OP 279
#define QUIT 280
#define HELP 281
#define LIST 282
#define FUNC_DEF 283
#define SHELL 284
#define YYERRCODE 256
static int yylhs[] = {                                        -1,
    0,    0,    0,    3,    3,    3,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    9,   11,   11,   11,   11,
   11,    4,    4,   15,    7,   14,   14,    5,    5,    2,
    2,    8,    8,   16,   16,    6,    6,    6,   10,   10,
   10,   17,   17,   19,   19,   20,   20,   20,   20,   20,
   20,   21,   21,   21,   13,   13,   13,   13,   13,   13,
   18,   18,   22,   22,   23,   23,   24,   24,   24,   25,
   25,   25,   25,   25,   26,   26,   26,   27,   27,   27,
   27,   28,   28,   29,   29,   30,   30,   30,   30,   30,
   30,   31,   31,   31,   31,   31,   31,   31,   12,   32,
   33,   34,
};
static int yylen[] = {                                         2,
    1,    2,    0,    1,    2,    2,    1,    1,    1,    1,
    1,    1,    1,    6,    1,    2,    1,    2,    1,    2,
    4,    1,    2,    0,    4,    1,    2,    7,    5,    1,
    1,    5,    9,    0,    1,    2,    2,    3,    0,    1,
    3,    1,    3,    1,    3,    0,    1,    3,    1,    2,
    3,    2,    2,    3,    1,    3,    3,    3,    6,    7,
    1,    3,    1,    3,    1,    3,    1,    3,    3,    1,
    3,    3,    3,    3,    1,    3,    3,    1,    3,    3,
    3,    1,    3,    1,    2,    1,    4,    4,    5,    5,
    3,    1,    1,    1,    1,    2,    2,    5,    1,    1,
    1,    1,
};
static int yydefred[] = {                                      0,
    0,    0,    0,    0,    0,    0,    0,  100,  101,  102,
   99,   15,    0,    0,   13,    0,    0,   24,   30,   31,
    0,    0,    0,    1,   22,    7,    8,    9,   10,   11,
   12,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   82,   84,   86,   92,   93,   94,    2,    0,
   36,   37,    0,    0,    0,    0,   18,    0,    0,   20,
    0,    0,   85,    0,    0,   16,    0,    0,    0,    0,
    0,   23,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   38,
   35,    0,    0,    0,   91,   26,    0,    0,    0,    0,
    0,   58,   57,    0,    0,    0,   40,    0,   56,    0,
    0,    0,    0,   44,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   83,
    0,    0,    0,    0,   25,   27,    0,    0,    0,    0,
   21,   88,    0,    0,    0,    0,   52,    0,    0,    0,
    0,   32,    0,    0,    0,   87,   89,   98,   90,   41,
   43,    0,   48,   54,   45,    0,   51,    0,    0,   14,
    0,   59,    0,   28,   60,    0,   33,
};
static int yydgoto[] = {                                      23,
   24,   25,    0,   26,   27,   28,   29,   30,   31,  105,
   32,   53,   34,   97,   61,   92,  107,   35,  113,  114,
  115,   36,   37,   38,   39,   40,   41,   42,   43,   44,
   45,   46,   47,   48,
};
static int yysindex[] = {                                     30,
   -4,  -38,   -4,   -4,   68,  -31,  -27,    0,    0,    0,
    0,    0, -250, -236,    0,   68, -250,    0,    0,    0,
   68, -137,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   -4,  -37,   -4, -229, -205, -181, -232, -131,  172,
   65,   12,    0,    0,    0,    0,    0,    0,    0,   68,
    0,    0,  -28,   -4,   68,   68,    0,   71,   74,    0,
   87,  -14,    0,  -10,  121,    0,   68,   68,   38,   68,
  399,    0,   68,   68,   68,   68,   68,   68,   68,   68,
   68,   68,   68,   68,   68,   68,   68,  127,   68,    0,
    0,  116,  151,   68,    0,    0,   58,   93,   68,   68,
   68,    0,    0,  159,  122,  -33,    0, -229,    0,   68,
  611,  153,  -44,    0,  158, -205, -181, -232, -131, -131,
  172,  172,  172,  172,   65,   65,   12,   12,   12,    0,
   87,   68,   87,  126,    0,    0,  -34,  161,  132,  182,
    0,    0,   68,   68,  144,   68,    0,   68,   93,  178,
   68,    0,  181,    0,  140,    0,    0,    0,    0,    0,
    0,  204,    0,    0,    0,   68,    0,   68,   87,    0,
   68,    0,  231,    0,    0,   87,    0,
};
static int yyrindex[] = {                                    278,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   17,    0,    0,    0,   59,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  490,    0,   -6,   41,   21,   51,   -2,  322,
  145,  424,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  115,    0,  220,    0,    0,    0,    0,    0,
    0,  115,    0,  135,  154,    0,    0,    0,  186,    0,
   42,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  186,    0,
    0,    0,    0,  186,    0,    0,    0,   42,  186,    0,
  186,    0,    0,    0,    0,  -22,    0,  190,    0,    0,
    0,   46,    0,    0,   61,  596,   78,  594,  569,  575,
  509,  518,  528,  564,  462,  499,  433,  452,  471,    0,
    0,  220,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   72,   42,  177,
   76,    0,    0,    1,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  239,    0,    0,
    0,    0,    0,    0,    0,    0,    0,
};
static int yycindex[] = {                                      0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   19,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,
};
static int yygindex[] = {                                      0,
  510,   13,    0,    0,    0,    0,  137,    0,    0,  -65,
    0,  867,  907,    0,    0,  -84,  164,  589,  196,  152,
    0,  229,  234,  237,  108,  165,  124,   63,  -16,    0,
    0,  287,    0,    0,
};
#define YYTABLESIZE 1078
static int yytable[] = {                                     149,
   29,   50,   69,   55,   63,   20,   89,   67,   55,  149,
   29,   89,   56,   49,   95,   51,   52,   11,   95,   95,
   95,   95,   95,   70,   95,   89,   17,  144,  134,   99,
   63,   58,   70,  138,   55,  140,   29,   55,   67,   20,
   29,   67,   76,   77,   66,   29,   72,  153,  150,   73,
   61,   55,   55,   71,   19,   67,   67,   98,  156,   29,
   65,   63,   71,   29,   63,   22,   90,   20,   19,   16,
  130,   95,   74,   22,   21,   17,   98,   16,   63,   63,
  100,   61,   21,  173,   61,   46,   55,   64,   19,   47,
   67,   65,   17,   22,   65,   75,   20,   16,   61,   61,
  104,   86,   21,   22,   49,   87,   84,   16,   65,   65,
   94,   85,   21,   63,   95,   53,   19,   19,   64,   50,
   17,   64,   22,   29,   95,   29,   16,    8,   22,   53,
   11,   21,   16,   61,   46,   64,   64,   21,   47,   78,
   79,   80,   81,   65,   97,   19,  127,  128,  129,   17,
  111,   95,   18,   49,   75,   95,   95,   95,   95,   95,
  101,   95,  142,   96,   53,  143,  155,  131,   50,  143,
   64,   97,   95,   95,  132,   97,   97,   97,   97,   97,
   18,   97,  135,  119,  120,   75,   87,   75,   75,   75,
   96,  133,   97,   97,   96,   96,   96,   96,   96,  141,
   96,  157,   75,   75,  143,  125,  126,   95,   95,   18,
  148,   96,   96,   87,   82,  151,   83,   87,   87,   87,
   87,   87,  159,   87,  158,  143,   39,   97,   97,   39,
   42,   67,   68,   42,   87,   87,  162,   75,  166,  168,
   67,   68,  121,  122,  123,  124,   96,   96,   95,   95,
   95,   95,   95,   95,   95,   95,   95,   29,   29,   29,
   29,   29,   18,   29,  171,   29,   29,   29,   29,   87,
   87,  176,   67,   67,   67,   67,   67,    3,   34,   34,
   29,   29,    0,   29,   29,    1,    2,    3,    4,    5,
    6,  170,    7,  137,    8,    9,   10,   11,   63,   63,
  165,  116,    8,    9,   10,   11,  160,  117,   65,   12,
   13,  118,   14,   15,    2,    3,    4,    5,    6,   61,
    7,    0,    8,    9,   10,   11,    0,   65,   65,   65,
    0,   70,    8,    9,   10,   11,    0,   12,   13,    0,
   14,   15,    0,    2,    3,    4,    5,    6,    0,    7,
    0,    8,    9,   10,   11,   64,   64,    8,    9,   10,
   11,    0,   70,    0,    0,   70,   12,   13,    0,   14,
   15,    0,    0,    0,    0,    0,    0,    0,    0,   70,
   70,    0,    0,    0,    0,   95,   95,   95,   95,   95,
   95,   95,   95,   95,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   97,   97,   97,   97,   97,
   97,   97,   97,   97,   70,   75,   75,   75,   75,   75,
   75,   75,   75,   75,   96,   96,   96,   96,   96,   96,
   96,   96,   96,   78,   22,    0,    0,    0,   16,    0,
    0,    0,   79,   21,    0,    0,    0,   87,   87,   87,
   87,   87,   87,   87,   87,   87,  111,    0,    0,    0,
   78,   80,    0,    0,   78,   78,   78,   78,   78,   79,
   78,   76,    0,   79,   79,   79,   79,   79,    0,   79,
   81,   78,   78,    0,    0,    0,    0,    0,   80,    0,
   79,   79,   80,   80,   80,   80,   80,    0,   80,   95,
    0,    0,   76,    0,   76,   76,   76,   81,   77,   80,
   80,   81,   81,   81,   81,   81,   78,   81,   71,   76,
   76,    0,    0,    0,    0,   79,   95,   72,   81,   81,
    0,   95,   95,    0,   95,    0,   95,   74,    0,   77,
    0,   77,   77,   77,   80,    0,    0,    0,   95,   71,
    0,    0,   71,    0,   76,    0,   77,   77,   72,    0,
    0,   72,    0,   81,    0,    0,   71,   71,   74,    0,
   96,   74,    0,   73,    0,   72,   72,    0,   68,    0,
    0,    0,    0,   95,   69,   74,   74,    0,    0,    0,
    0,   77,   70,   70,   70,   70,   70,   70,   70,   70,
   70,   71,    0,   66,   73,   62,  136,   73,    0,   68,
   72,    0,   68,    0,    0,   69,    0,    0,   69,    0,
   74,   73,   73,    0,    0,    0,   68,   68,    0,    0,
    0,    0,   69,   69,   66,    0,   62,   66,    0,   62,
  152,    0,  154,    0,    0,    0,   22,    0,    0,    0,
   16,   66,   66,   62,   62,   21,   73,  108,    0,    0,
  110,   68,    0,    8,    9,   10,   11,   69,  146,    0,
    0,    0,    0,    0,    0,    0,    0,  108,  174,    0,
    0,    0,  108,    0,    0,  177,   66,  108,   62,  108,
    0,    0,    0,    0,   78,   78,   78,   78,   78,   78,
   78,   78,   78,   79,   79,   79,   79,   79,   79,   79,
   79,   79,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   80,   80,   80,   80,   80,   80,   80,   80,
   80,  108,   76,   76,   76,   76,   76,   76,   76,   76,
   76,   81,   81,   81,   81,   81,   81,   81,   81,   81,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
   95,   95,   95,   95,   95,   95,   95,   95,   95,   77,
   77,   77,   77,   77,   77,   77,   77,   77,    0,   71,
   71,   71,   71,   71,   71,   71,   71,   71,   72,   72,
   72,   72,   72,   72,   72,   72,   72,    0,   74,   74,
   74,   74,   74,   74,   74,   74,   74,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   73,   73,   73,   73,   73,   73,
   73,   73,   73,   68,   68,   68,   68,   68,    0,   69,
   69,   69,   69,   69,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   33,    0,    0,    0,
   66,   66,   66,    0,   62,    8,    9,   10,   11,   57,
    0,    0,    0,   60,    0,    0,    0,   62,   64,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   54,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   59,    0,    0,    0,    0,   33,    0,    0,
    0,    0,    0,    0,    0,  106,    0,    0,    0,   62,
   62,   62,   62,   62,   62,   62,   62,   62,   62,   62,
   62,   62,   62,   62,    0,  106,   88,    0,    0,    0,
  106,   91,   93,   33,    0,  106,    0,  106,    0,    0,
    0,    0,    0,  102,  103,    0,  109,  112,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   33,    0,   33,
    0,    0,    0,    0,  112,    0,  139,    0,    0,  106,
    0,    0,    0,    0,    0,    0,  145,  147,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   33,    0,    0,   91,    0,
    0,    0,   33,    0,    0,    0,    0,    0,    0,    0,
  161,    0,  163,    0,  164,  112,    0,  167,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  172,    0,   91,    0,    0,  175,
};
static int yycheck[] = {                                      44,
    0,   40,   40,   10,   21,   10,   40,   10,   40,   44,
   10,   40,   40,    1,   37,    3,    4,  268,   41,   42,
   43,   44,   45,   61,   47,   40,   10,   61,   94,   40,
   10,  268,   61,   99,   41,  101,   36,   44,   41,   10,
   40,   44,  275,  276,   32,   45,   34,  132,   93,  279,
   10,   58,   59,   91,   59,   58,   59,   91,   93,   59,
   10,   41,   91,   63,   44,   36,   54,   10,   10,   40,
   87,   94,  278,   36,   45,   59,   91,   40,   58,   59,
   91,   41,   45,  168,   44,   44,   93,   10,   59,   44,
   93,   41,   63,   36,   44,  277,   10,   40,   58,   59,
   63,   37,   45,   36,   44,   94,   42,   40,   58,   59,
   40,   47,   45,   93,   41,   44,   59,   59,   41,   44,
   63,   44,   36,  123,   10,  125,   40,  265,   36,   58,
  268,   45,   40,   93,   93,   58,   59,   45,   93,  271,
  272,  273,  274,   93,   10,   59,   84,   85,   86,   63,
   58,   37,  123,   93,   10,   41,   42,   43,   44,   45,
   40,   47,   41,   10,   93,   44,   41,   41,   93,   44,
   93,   37,   58,   59,   59,   41,   42,   43,   44,   45,
  123,   47,  125,   76,   77,   41,   10,   43,   44,   45,
   37,   41,   58,   59,   41,   42,   43,   44,   45,   41,
   47,   41,   58,   59,   44,   82,   83,   93,   94,  123,
   58,   58,   59,   37,   43,   58,   45,   41,   42,   43,
   44,   45,   41,   47,   93,   44,   41,   93,   94,   44,
   41,  269,  270,   44,   58,   59,   93,   93,   61,   59,
  269,  270,   78,   79,   80,   81,   93,   94,  271,  272,
  273,  274,  275,  276,  277,  278,  279,  257,  258,  259,
  260,  261,  123,  263,   61,  265,  266,  267,  268,   93,
   94,   41,  275,  276,  277,  278,  279,    0,   59,   41,
  280,  281,  264,  283,  284,  256,  257,  258,  259,  260,
  261,  155,  263,   98,  265,  266,  267,  268,  278,  279,
  149,   73,  265,  266,  267,  268,  143,   74,   22,  280,
  281,   75,  283,  284,  257,  258,  259,  260,  261,  279,
  263,   -1,  265,  266,  267,  268,   -1,  277,  278,  279,
   -1,   10,  265,  266,  267,  268,   -1,  280,  281,   -1,
  283,  284,   -1,  257,  258,  259,  260,  261,   -1,  263,
   -1,  265,  266,  267,  268,  278,  279,  265,  266,  267,
  268,   -1,   41,   -1,   -1,   44,  280,  281,   -1,  283,
  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   58,
   59,   -1,   -1,   -1,   -1,  271,  272,  273,  274,  275,
  276,  277,  278,  279,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  271,  272,  273,  274,  275,
  276,  277,  278,  279,   93,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  271,  272,  273,  274,  275,  276,
  277,  278,  279,   10,   36,   -1,   -1,   -1,   40,   -1,
   -1,   -1,   10,   45,   -1,   -1,   -1,  271,  272,  273,
  274,  275,  276,  277,  278,  279,   58,   -1,   -1,   -1,
   37,   10,   -1,   -1,   41,   42,   43,   44,   45,   37,
   47,   10,   -1,   41,   42,   43,   44,   45,   -1,   47,
   10,   58,   59,   -1,   -1,   -1,   -1,   -1,   37,   -1,
   58,   59,   41,   42,   43,   44,   45,   -1,   47,   10,
   -1,   -1,   41,   -1,   43,   44,   45,   37,   10,   58,
   59,   41,   42,   43,   44,   45,   93,   47,   10,   58,
   59,   -1,   -1,   -1,   -1,   93,   37,   10,   58,   59,
   -1,   42,   43,   -1,   45,   -1,   47,   10,   -1,   41,
   -1,   43,   44,   45,   93,   -1,   -1,   -1,   59,   41,
   -1,   -1,   44,   -1,   93,   -1,   58,   59,   41,   -1,
   -1,   44,   -1,   93,   -1,   -1,   58,   59,   41,   -1,
   61,   44,   -1,   10,   -1,   58,   59,   -1,   10,   -1,
   -1,   -1,   -1,   94,   10,   58,   59,   -1,   -1,   -1,
   -1,   93,  271,  272,  273,  274,  275,  276,  277,  278,
  279,   93,   -1,   10,   41,   10,   97,   44,   -1,   41,
   93,   -1,   44,   -1,   -1,   41,   -1,   -1,   44,   -1,
   93,   58,   59,   -1,   -1,   -1,   58,   59,   -1,   -1,
   -1,   -1,   58,   59,   41,   -1,   41,   44,   -1,   44,
  131,   -1,  133,   -1,   -1,   -1,   36,   -1,   -1,   -1,
   40,   58,   59,   58,   59,   45,   93,   69,   -1,   -1,
  262,   93,   -1,  265,  266,  267,  268,   93,   58,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   89,  169,   -1,
   -1,   -1,   94,   -1,   -1,  176,   93,   99,   93,  101,
   -1,   -1,   -1,   -1,  271,  272,  273,  274,  275,  276,
  277,  278,  279,  271,  272,  273,  274,  275,  276,  277,
  278,  279,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  271,  272,  273,  274,  275,  276,  277,  278,
  279,  143,  271,  272,  273,  274,  275,  276,  277,  278,
  279,  271,  272,  273,  274,  275,  276,  277,  278,  279,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  271,  272,  273,  274,  275,  276,  277,  278,  279,  271,
  272,  273,  274,  275,  276,  277,  278,  279,   -1,  271,
  272,  273,  274,  275,  276,  277,  278,  279,  271,  272,
  273,  274,  275,  276,  277,  278,  279,   -1,  271,  272,
  273,  274,  275,  276,  277,  278,  279,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  271,  272,  273,  274,  275,  276,
  277,  278,  279,  275,  276,  277,  278,  279,   -1,  275,
  276,  277,  278,  279,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,    0,   -1,   -1,   -1,
  277,  278,  279,   -1,  279,  265,  266,  267,  268,   13,
   -1,   -1,   -1,   17,   -1,   -1,   -1,   21,   22,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,    5,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   16,   -1,   -1,   -1,   -1,   61,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   69,   -1,   -1,   -1,   73,
   74,   75,   76,   77,   78,   79,   80,   81,   82,   83,
   84,   85,   86,   87,   -1,   89,   50,   -1,   -1,   -1,
   94,   55,   56,   97,   -1,   99,   -1,  101,   -1,   -1,
   -1,   -1,   -1,   67,   68,   -1,   70,   71,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  131,   -1,  133,
   -1,   -1,   -1,   -1,   98,   -1,  100,   -1,   -1,  143,
   -1,   -1,   -1,   -1,   -1,   -1,  110,  111,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  169,   -1,   -1,  132,   -1,
   -1,   -1,  176,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  144,   -1,  146,   -1,  148,  149,   -1,  151,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  166,   -1,  168,   -1,   -1,  171,
};
static int yyctable[] = {                                    169,
   29,   -1,
};
#define YYFINAL 23
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 284
#if YYDEBUG
static char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,"'\\n'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,"'$'","'%'",0,0,"'('","')'","'*'","'+'","','","'-'",0,"'/'",0,0,0,0,0,
0,0,0,0,0,"':'","';'",0,"'='",0,"'?'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,"'['",0,"']'","'^'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,"'{'",0,"'}'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"WHILE","CONTINUE","BREAK","RETURN",
"FOR","WHERE","IF","ELSE","IVAL","RVAL","STRING","ID","DEC_OP","INC_OP","LT_OP",
"GT_OP","GE_OP","LE_OP","EQ_OP","NE_OP","AND_OP","OR_OP","CAT_OP","QUIT","HELP",
"LIST","FUNC_DEF","SHELL",
};
static char *yyrule[] = {
"$accept : start",
"start : statement",
"start : error separator",
"start :",
"statements : statement",
"statements : error separator",
"statements : statements statement",
"statement : expression_statement",
"statement : selection_statement",
"statement : jump_statement",
"statement : compound_statement",
"statement : iteration_statement",
"statement : command_statement",
"statement : SHELL",
"statement : FUNC_DEF ID '(' args ')' compound_statement",
"statement : QUIT",
"command_statement : help_statement separator",
"help_statement : HELP",
"help_statement : HELP id",
"help_statement : '?'",
"help_statement : '?' id",
"help_statement : id '(' '?' ')'",
"expression_statement : separator",
"expression_statement : expression separator",
"$$1 :",
"compound_statement : '{' $$1 statement_list '}'",
"statement_list : statement",
"statement_list : statement_list statement",
"selection_statement : IF '(' expression ')' statement ELSE statement",
"selection_statement : IF '(' expression ')' statement",
"separator : ';'",
"separator : '\\n'",
"iteration_statement : WHILE '(' expression ')' statement",
"iteration_statement : FOR '(' forval ';' forval ';' forval ')' statement",
"forval :",
"forval : expression",
"jump_statement : CONTINUE separator",
"jump_statement : BREAK separator",
"jump_statement : RETURN expression separator",
"args :",
"args : arg",
"args : args ',' arg",
"arg : concat",
"arg : id '=' expression",
"ranges : range2",
"ranges : ranges ',' range2",
"range2 :",
"range2 : expression",
"range2 : ':' ':' expression",
"range2 : range",
"range2 : range ':'",
"range2 : range ':' expression",
"range : ':' expression",
"range : expression ':'",
"range : expression ':' expression",
"expression : concat",
"expression : id '=' expression",
"expression : id INC_OP expression",
"expression : id DEC_OP expression",
"expression : id '[' ranges ']' '=' expression",
"expression : id '[' WHERE expression ']' '=' expression",
"concat : logical_OR",
"concat : concat CAT_OP logical_OR",
"logical_OR : logical_AND",
"logical_OR : logical_OR OR_OP logical_AND",
"logical_AND : equality_expr",
"logical_AND : logical_AND AND_OP equality_expr",
"equality_expr : relation_expr",
"equality_expr : equality_expr EQ_OP relation_expr",
"equality_expr : equality_expr NE_OP relation_expr",
"relation_expr : additive_expr",
"relation_expr : relation_expr LT_OP additive_expr",
"relation_expr : relation_expr GT_OP additive_expr",
"relation_expr : relation_expr LE_OP additive_expr",
"relation_expr : relation_expr GE_OP additive_expr",
"additive_expr : mult_expr",
"additive_expr : additive_expr '+' mult_expr",
"additive_expr : additive_expr '-' mult_expr",
"mult_expr : power_expr",
"mult_expr : mult_expr '*' power_expr",
"mult_expr : mult_expr '/' power_expr",
"mult_expr : mult_expr '%' power_expr",
"power_expr : unary_expr",
"power_expr : power_expr '^' unary_expr",
"unary_expr : postfix_expr",
"unary_expr : '-' unary_expr",
"postfix_expr : lhs",
"postfix_expr : id '[' ranges ']'",
"postfix_expr : id '(' args ')'",
"postfix_expr : '$' id '(' args ')'",
"postfix_expr : '$' ival '(' args ')'",
"postfix_expr : '(' expression ')'",
"lhs : ival",
"lhs : rval",
"lhs : string",
"lhs : id",
"lhs : '$' ival",
"lhs : '$' id",
"lhs : '$' id '[' expression ']'",
"id : ID",
"ival : IVAL",
"rval : RVAL",
"string : STRING",
};
#endif
#ifndef YYSTYPE
typedef int YYSTYPE;
#endif
#line 8 "push.skel"
#define yyclearin (yychar=(-1))
#define yyerrok (yyerrflag=0)
/*  #ifdef YYSTACKSIZE
    #ifndef YYMAXDEPTH
    #define YYMAXDEPTH YYSTACKSIZE
    #endif
    #else
    #ifdef YYMAXDEPTH
    #define YYSTACKSIZE YYMAXDEPTH
    #else
    #define YYSTACKSIZE 500
    #define YYMAXDEPTH 500
    #endif
    #endif */
int yydebug;
static struct yyparsestate {
	struct yyparsestate *save;
	int state;
	int errflag;
	short *ssp;
	YYSTYPE *vsp;
	YYSTYPE val;
	short *ss;
	YYSTYPE *vs;
	int lexeme;
	unsigned short stacksize;
	short ctry;
	} *yypstate=0, *yypath=0;
#define yyerrflag (yypstate->errflag)
#define yyssp (yypstate->ssp)
#define yyvsp (yypstate->vsp)
#define yyval (yypstate->val)
#define yyss (yypstate->ss)
#define yyvs (yypstate->vs)
#define yystacksize (yypstate->stacksize)
static YYSTYPE *yylvals=0, *yylvp=0, *yylve=0, *yylvlim=0;
static short *yylexemes=0, *yylexp=0;
#define YYLEX (yylvp<yylve ? yylval=*yylvp++, *yylexp++ :		\
    yytrial ? (yylvp==yylvlim ? yyexpand() : 0), *yylexp = yylex(),	\
	      *yylvp++ = yylval, yylve++, *yylexp++			\
	    : yylex())
extern int yylex(), yyparse();
#define yytrial (yypstate->save)
#ifndef __cplusplus
#define YYSCOPY(t, f, s)	memcpy(t, f, (s)*sizeof(YYSTYPE))
#define YYMORESTACK do { int p = yyssp - yyss;				\
    yystacksize += 16;							\
    yyss = (short *)realloc(yyss, yystacksize * sizeof(short));		\
    yyvs = (YYSTYPE *)realloc(yyvs, yystacksize * sizeof(YYSTYPE));	\
    yyssp = yyss + p;							\
    yyvsp = yyvs + p;							\
    } while (0)
#else  /* C++ */
#define	YYSCOPY(to, from, size)	do { int _i;				\
	for (_i = (size)-1; _i >= 0; _i--)				\
	    (to)[_i] = (from)[_i];					\
				    } while(0)
#define YYMORESTACK do { int p = yyssp - yyss;				\
    short *tss = yyss; YYSTYPE *tvs = yyvs;				\
    yyss = new short[yystacksize + 16];					\
    yyvs = new YYSTYPE[yystacksize + 16];				\
    memcpy(yyss, tss, yystacksize * sizeof(short));			\
    YYSCOPY(yyvs, tvs, yystacksize);					\
    yystacksize += 16;							\
    delete[] tss;							\
    delete[] tvs;							\
    yyssp = yyss + p;							\
    yyvsp = yyvs + p;							\
    } while (0)
#endif /* C++ */

#line 80 "push.skel"

#ifndef YYNEWSTATE
#ifdef __oldc
static struct yyparsestate *YYNEWSTATE(size)
int size;
#else
static struct yyparsestate *YYNEWSTATE(int size)
#endif /* __oldc */
{
struct yyparsestate *p;

#ifndef __cplusplus
    p = (struct yyparsestate *)malloc(sizeof(struct yyparsestate));
    p->stacksize = size+4;
    p->ss = (short *)malloc((size+4)*sizeof(short));
    p->vs = (YYSTYPE *)malloc((size+4)*sizeof(YYSTYPE));
#else /* C++ */
    p = new yyparsestate;
    p->stacksize = size+4;
    p->ss = new short[size + 4];
    p->vs = new YYSTYPE[size + 4];
#endif /* C++ */
    return p;
}
#endif /* YYNEWSTATE */
#ifndef YYFREESTATE
#ifndef __cplusplus
#define YYFREESTATE(p) (free((p)->ss), free((p)->vs), free(p))
#else /* C++ */
#define YYFREESTATE(p) (delete[] (p)->ss, delete[] (p)->vs, delete (p))
#endif /* C++ */
#endif /* YYFREESTATE */
static int yyexpand()
{
int p = yylvp-yylvals;
int s = yylvlim-yylvals;
    s += 16;
#ifndef __cplusplus
    yylvals = (YYSTYPE *)realloc(yylvals, s*sizeof(YYSTYPE));
    yylexemes = (short *)realloc(yylexemes, s*sizeof(short));
#else /* C++ */
      { short *tl = yylexemes; YYSTYPE *tv = yylvals;
	yylvals = new YYSTYPE[s];
	yylexemes = new short[s];
	memcpy(yylexemes, tl, (s-16)*sizeof(short));
	YYSCOPY(yylvals, tv, s-16);
	delete[] tl;
	delete[] tv; }
#endif /* C++ */
    yylvp = yylve = yylvals + p;
    yylvlim = yylvals + s;
    yylexp = yylexemes + p;
    return 0;
}

#define YYABORT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
#define YYVALID do { if (yytrial) goto yyvalid; } while(0)
#ifdef __cplusplus
extern "C" char *getenv(const char *);
#else
extern char *getenv();
#endif
int yyparse(int yychar, YYSTYPE yylval)
{
    int yym, yyn, yystate, yynewerrflag;
#if YYDEBUG
    char *yys;
#endif

    if (yychar < 0) yychar = 0;
    if (!yypstate) {
	/* initialize the parser state */
	yypstate = YYNEWSTATE(12);
	yypath = 0;
	yytrial = 0;
	yyerrflag = 0;
	yylvp = yylve = yylvals;
	yylexp = yylexemes;
	yyssp = yyss;
	yyvsp = yyvs;
	*yyssp = yypstate->state = 0; }
    yystate = yypstate->state;
#if YYDEBUG
    if (yydebug) {
	yys = 0;
	if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
	if (!yys) yys = "illegal-symbol";
	printf("yydebug: state %d, input %d (%s)", yystate,
		yychar, yys);
#ifdef YYDBPR
	printf("<");
	YYDBPR(yylval);
	printf(">");
#endif
	printf("\n"); }
#endif
    if (yystate == YYFINAL && yychar == 0)
	goto yyaccept;
    if (yytrial) {
	if (yylvp == yylvlim) yyexpand();
	*yylvp++ = yylval;
	yylve++;
	*yylexp++ = yychar; }
yyloop:
    if ((yyn = yydefred[yystate])) goto yyreduce;
    if (yychar < 0) {
	if (yylvp < yylve) {
	    yylval = *yylvp++;
	    yychar = *yylexp++; }
	else {
	    yypstate->state = yystate;
	    return 0; }
#if YYDEBUG
	if (yydebug) {
	    yys = 0;
	    if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
	    if (!yys) yys = "illegal-symbol";
	    printf("yydebug: state %d, reading %d (%s)", yystate,
		    yychar, yys);
#ifdef YYDBPR
	    printf("<");
	    YYDBPR(yylval);
	    printf(">");
#endif
	    printf("\n"); }
#endif
	}
    if ((yyn = yycindex[yystate]) &&
	(yyn += yychar) >= 0 &&
	yyn <= YYTABLESIZE &&
	yycheck[yyn] == yychar) {
	    int ctry;
	    struct yyparsestate *save;
#if YYDEBUG
	if (yydebug)
	    printf("yydebug: state %d, conflict%s\n", yystate,
		   yypath ? ", following successful trial parse" :
		       yytrial ? "" : ", starting trial parse");
#endif
	if (yypath) {
	    save = yypath;
	    yypath = save->save;
	    ctry = save->ctry;
	    if (save->state != yystate) goto yyabort;
	    YYFREESTATE(save); }
	else {
	    save = YYNEWSTATE(yyssp - yyss);
	    save->save = yypstate->save;
	    save->state = yystate;
	    save->errflag = yypstate->errflag;
	    save->ssp = save->ss + (yyssp - yyss);
	    save->vsp = save->vs + (yyvsp - yyvs);
	    memcpy(save->ss, yyss, (yyssp - yyss + 1)*sizeof(short));
	    YYSCOPY(save->vs, yyvs, yyssp - yyss + 1);
	    ctry = yytable[yyn];
	    if (yyctable[ctry] == -1) {
#if YYDEBUG
		if (yydebug && yychar >= 0)
		    printf("yydebug: backtracking 1 token\n");
#endif
		ctry++; }
	    save->ctry = ctry;
	    if (!yytrial) {
		if (!yylexemes) {
#ifndef __cplusplus
		    yylexemes = (short *)malloc(16*sizeof(short));
		    yylvals = (YYSTYPE *)malloc(16*sizeof(YYSTYPE));
#else  /* C++ */
		    yylexemes = new short[16];
		    yylvals = new YYSTYPE[16];
#endif /* C++ */
		    yylvlim = yylvals + 16; }
		if (yylvp == yylve) {
		    yylvp = yylve = yylvals;
		    yylexp = yylexemes;
		    if (yychar >= 0) {
			*yylve++ = yylval;
			*yylexp = yychar;
			yychar = -1; } } }
	    if (yychar >= 0) {
		yylvp--, yylexp--;
		yychar = -1; }
	    save->lexeme = yylvp - yylvals;
	    yypstate->save = save; }
	if (yytable[yyn] == ctry) {
#if YYDEBUG
	    if (yydebug)
		printf("yydebug: state %d, shifting to state %d\n",
			yystate, yyctable[ctry]);
#endif
	    if (yychar < 0)
		yylvp++, yylexp++;
	    yychar = -1;
	    if (yyerrflag > 0) --yyerrflag;
	    yystate = yyctable[ctry];
	    goto yyshift; }
	else {
	    yyn = yyctable[ctry];
	    goto yyreduce; } }
    if ((yyn = yysindex[yystate]) &&
	(yyn += yychar) >= 0 &&
	yyn <= YYTABLESIZE &&
	yycheck[yyn] == yychar) {
#if YYDEBUG
	if (yydebug)
	    printf("yydebug: state %d, shifting to state %d\n",
		    yystate, yytable[yyn]);
#endif
	yychar = (-1);
	if (yyerrflag > 0)  --yyerrflag;
	yystate = yytable[yyn];
yyshift:
	if (yyssp >= yyss + yystacksize - 1)
	    YYMORESTACK;
	*++yyssp = yystate;
	*++yyvsp = yylval;
	goto yyloop; }
    if ((yyn = yyrindex[yystate]) &&
	(yyn += yychar) >= 0 &&
	yyn <= YYTABLESIZE &&
	yycheck[yyn] == yychar) {
	yyn = yytable[yyn];
	goto yyreduce; }
    if (yyerrflag) goto yyinrecovery;
    yynewerrflag = 1;
    goto yyerrhandler;
yyerrlab:
    yynewerrflag = 0;
yyerrhandler:
    while (yytrial) { int ctry; struct yyparsestate *save;
#if YYDEBUG
	if (yydebug)
	    printf("yydebug: error in state %d, %s state %d, %d tokens\n",
		   yystate, "backtracking to", yypstate->save->state,
		   (int)(yylvp - yylvals - yypstate->save->lexeme));
#endif
	save = yypstate->save;
	yylvp = yylvals + save->lexeme;
	yylexp = yylexemes + save->lexeme;
	yychar = -1;
	yyssp = yyss + (save->ssp - save->ss);
	yyvsp = yyvs + (save->vsp - save->vs);
	memcpy(yyss, save->ss, (yyssp - yyss + 1) * sizeof(short));
	YYSCOPY(yyvs, save->vs, yyvsp - yyvs + 1);
	ctry = ++save->ctry;
	yystate = save->state;
	if ((yyn = yyctable[ctry]) >= 0) goto yyreduce;
	yypstate->save = save->save;
	YYFREESTATE(save);
#if YYDEBUG
	if (yydebug && !yytrial)
	    printf("yydebug: trial parse failed, entering error mode\n");
#endif
	yynewerrflag = 1; }
    if (yynewerrflag)
	yyerror("syntax error");
yyinrecovery:
    if (yyerrflag < 3) {
	yyerrflag = 3;
	for (;;) {
	    if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
		    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE) {
#if YYDEBUG
		if (yydebug)
		    printf("yydebug: state %d, error recovery %s state %d\n",
			   *yyssp, "shifting to", yytable[yyn]);
#endif
		yystate = yytable[yyn];
		goto yyshift; }
	    else {
#if YYDEBUG
		if (yydebug)
		    printf("yydebug: error recovery discarding state %d\n",
			    *yyssp);
#endif
		if (yyssp <= yyss) goto yyabort;
		--yyssp;
		--yyvsp; } } }
    else {
	if (yychar == 0) goto yyabort;
#if YYDEBUG
	if (yydebug) {
	    yys = 0;
	    if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
	    if (!yys) yys = "illegal-symbol";
	    printf("yydebug: state %d, error recovery discards token %d (%s)\n",
		    yystate, yychar, yys); }
#endif
	yychar = (-1);
	goto yyloop; }
yyreduce:
    yym = yylen[yyn];
#if YYDEBUG
    if (yydebug) {
	printf("yydebug: state %d, reducing by rule %d (%s)",
		yystate, yyn, yyrule[yyn]);
#ifdef YYDBPR
	if (yym) {
	    int i;
	    printf("<");
	    for (i=yym; i>0; i--) {
		if (i!=yym) printf(", ");
		YYDBPR(yyvsp[1-i]); }
	    printf(">"); }
#endif
	printf("\n"); }
#endif
    if (yyssp + 1 - yym >= yyss + yystacksize)
	YYMORESTACK;
    yyval = yyvsp[1-yym];
    switch (yyn) {

case 1:
  if (!yytrial)
#line 32 "parser.y"
{ curnode = yyval = yyvsp[0];  YYACCEPT; }
#line 914 "y_tab.c"
break;
case 2:
  if (!yytrial)
#line 33 "parser.y"
{ curnode = NULL ; YYACCEPT; }
#line 920 "y_tab.c"
break;
case 4:
  if (!yytrial)
#line 43 "parser.y"
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
                                }
#line 938 "y_tab.c"
break;
case 5:
  if (!yytrial)
#line 56 "parser.y"
{
                                    indent = 0;
                                }
#line 946 "y_tab.c"
break;
case 6:
  if (!yytrial)
#line 59 "parser.y"
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
                                }
#line 964 "y_tab.c"
break;
case 7:
  if (!yytrial)
#line 75 "parser.y"
{ yyval = yyvsp[0]; }
#line 970 "y_tab.c"
break;
case 8:
  if (!yytrial)
#line 76 "parser.y"
{ yyval = yyvsp[0]; }
#line 976 "y_tab.c"
break;
case 9:
  if (!yytrial)
#line 77 "parser.y"
{ yyval = yyvsp[0]; }
#line 982 "y_tab.c"
break;
case 10:
  if (!yytrial)
#line 78 "parser.y"
{ yyval = yyvsp[0]; }
#line 988 "y_tab.c"
break;
case 11:
  if (!yytrial)
#line 79 "parser.y"
{ yyval = yyvsp[0]; }
#line 994 "y_tab.c"
break;
case 12:
  if (!yytrial)
#line 80 "parser.y"
{ yyval = yyvsp[0]; }
#line 1000 "y_tab.c"
break;
case 13:
  if (!yytrial)
#line 81 "parser.y"
{ yyval = pp_shell(yytext); }
#line 1006 "y_tab.c"
break;
case 14:
  if (!yytrial)
#line 82 "parser.y"
{ yyval = NULL; }
#line 1012 "y_tab.c"
break;
case 15:
  if (!yytrial)
#line 83 "parser.y"
{ YYABORT; }
#line 1018 "y_tab.c"
break;
case 16:
  if (!yytrial)
#line 87 "parser.y"
{ yyval = yyvsp[-1]; }
#line 1024 "y_tab.c"
break;
case 17:
  if (!yytrial)
#line 91 "parser.y"
{ yyval = pp_help(NULL); }
#line 1030 "y_tab.c"
break;
case 18:
  if (!yytrial)
#line 92 "parser.y"
{ yyval = pp_help(yyvsp[0]); }
#line 1036 "y_tab.c"
break;
case 19:
  if (!yytrial)
#line 93 "parser.y"
{ yyval = pp_help(NULL); }
#line 1042 "y_tab.c"
break;
case 20:
  if (!yytrial)
#line 94 "parser.y"
{ yyval = pp_help(yyvsp[0]); }
#line 1048 "y_tab.c"
break;
case 21:
  if (!yytrial)
#line 95 "parser.y"
{ yyval = pp_help(yyvsp[-3]); }
#line 1054 "y_tab.c"
break;
case 22:
  if (!yytrial)
#line 99 "parser.y"
{ yyval = NULL; }
#line 1060 "y_tab.c"
break;
case 23:
  if (!yytrial)
#line 100 "parser.y"
{ yyval = yyvsp[-1]; }
#line 1066 "y_tab.c"
break;
case 24:
  if (!yytrial)
#line 104 "parser.y"
{indent++;}
#line 1072 "y_tab.c"
break;
case 25:
  if (!yytrial)
#line 104 "parser.y"
{ indent--;yyval = yyvsp[-1]; }
#line 1078 "y_tab.c"
break;
case 26:
  if (!yytrial)
#line 108 "parser.y"
{ yyval = yyvsp[0]; }
#line 1084 "y_tab.c"
break;
case 27:
  if (!yytrial)
#line 109 "parser.y"
{ yyval = p_rlist(ID_LIST, yyvsp[-1], yyvsp[0]); }
#line 1090 "y_tab.c"
break;
case 28:
#line 113 "parser.y"
{YYVALID;}
#line 1095 "y_tab.c"
  if (!yytrial)
#line 114 "parser.y"
{ yyval = p_mknod(ID_IF, yyvsp[-4], p_mknod(ID_ELSE, yyvsp[-2], yyvsp[0])); }
#line 1099 "y_tab.c"
break;
case 29:
#line 115 "parser.y"
{YYVALID;}
#line 1104 "y_tab.c"
  if (!yytrial)
#line 116 "parser.y"
{ yyval = p_mknod(ID_IF, yyvsp[-2], yyvsp[0]); }
#line 1108 "y_tab.c"
break;
case 32:
  if (!yytrial)
#line 127 "parser.y"
{ yyval = p_mknod(ID_WHILE, yyvsp[-2], yyvsp[0]); }
#line 1114 "y_tab.c"
break;
case 33:
  if (!yytrial)
#line 129 "parser.y"
{ yyval = p_mknod(ID_LIST, yyvsp[-6],
                                                   p_mknod(ID_WHILE, yyvsp[-4],
                                                   p_mknod(ID_FOR, yyvsp[0], yyvsp[-2]))); }
#line 1122 "y_tab.c"
break;
case 34:
  if (!yytrial)
#line 135 "parser.y"
{ yyval = NULL; }
#line 1128 "y_tab.c"
break;
case 35:
  if (!yytrial)
#line 136 "parser.y"
{ yyval = yyvsp[0]; }
#line 1134 "y_tab.c"
break;
case 36:
  if (!yytrial)
#line 140 "parser.y"
{ yyval = p_mknod(ID_CONT,NULL,NULL); }
#line 1140 "y_tab.c"
break;
case 37:
  if (!yytrial)
#line 141 "parser.y"
{ yyval = p_mknod(ID_BREAK,NULL,NULL); }
#line 1146 "y_tab.c"
break;
case 38:
  if (!yytrial)
#line 142 "parser.y"
{ yyval = p_mknod(ID_RETURN,yyvsp[-1],NULL); }
#line 1152 "y_tab.c"
break;
case 39:
  if (!yytrial)
#line 146 "parser.y"
{ yyval = NULL; }
#line 1158 "y_tab.c"
break;
case 40:
  if (!yytrial)
#line 147 "parser.y"
{ yyval = p_mknod(ID_ARGS, NULL, yyvsp[0]); }
#line 1164 "y_tab.c"
break;
case 41:
  if (!yytrial)
#line 148 "parser.y"
{ yyval = p_mknod(ID_ARGS, yyvsp[-2], yyvsp[0]); }
#line 1170 "y_tab.c"
break;
case 42:
  if (!yytrial)
#line 152 "parser.y"
{ yyval = p_mknod(ID_ARG, NULL, yyvsp[0]); }
#line 1176 "y_tab.c"
break;
case 43:
  if (!yytrial)
#line 153 "parser.y"
{ yyval = p_mknod(ID_ARG, yyvsp[-2], yyvsp[0]); }
#line 1182 "y_tab.c"
break;
case 44:
  if (!yytrial)
#line 157 "parser.y"
{ yyval = p_mknod(ID_RANGES, NULL, yyvsp[0]); }
#line 1188 "y_tab.c"
break;
case 45:
  if (!yytrial)
#line 158 "parser.y"
{ yyval = p_mknod(ID_RANGES, yyvsp[-2], yyvsp[0]) ; }
#line 1194 "y_tab.c"
break;
case 46:
  if (!yytrial)
#line 162 "parser.y"
{ yyval = p_mknod(ID_RANGE, NULL, NULL); }
#line 1200 "y_tab.c"
break;
case 47:
  if (!yytrial)
#line 163 "parser.y"
{ yyval = p_mknod(ID_RANGE, yyvsp[0], yyvsp[0]); }
#line 1206 "y_tab.c"
break;
case 48:
  if (!yytrial)
#line 164 "parser.y"
{ yyval = p_mknod(ID_RSTEP, NULL, yyvsp[0]); }
#line 1212 "y_tab.c"
break;
case 49:
  if (!yytrial)
#line 165 "parser.y"
{ yyval = yyvsp[0]; }
#line 1218 "y_tab.c"
break;
case 50:
  if (!yytrial)
#line 166 "parser.y"
{ yyval = p_mknod(ID_RSTEP, yyvsp[-1], NULL); }
#line 1224 "y_tab.c"
break;
case 51:
  if (!yytrial)
#line 167 "parser.y"
{ yyval = p_mknod(ID_RSTEP, yyvsp[-2], yyvsp[0]); }
#line 1230 "y_tab.c"
break;
case 52:
  if (!yytrial)
#line 171 "parser.y"
{ yyval = p_mknod(ID_RANGE, NULL, yyvsp[0]); }
#line 1236 "y_tab.c"
break;
case 53:
  if (!yytrial)
#line 172 "parser.y"
{ yyval = p_mknod(ID_RANGE, yyvsp[-1], NULL); }
#line 1242 "y_tab.c"
break;
case 54:
  if (!yytrial)
#line 173 "parser.y"
{ yyval = p_mknod(ID_RANGE, yyvsp[-2], yyvsp[0]); }
#line 1248 "y_tab.c"
break;
case 55:
  if (!yytrial)
#line 177 "parser.y"
{ yyval = yyvsp[0]; }
#line 1254 "y_tab.c"
break;
case 56:
  if (!yytrial)
#line 178 "parser.y"
{ yyval = p_mknod(ID_SET,yyvsp[-2],yyvsp[0]); }
#line 1260 "y_tab.c"
break;
case 57:
  if (!yytrial)
#line 179 "parser.y"
{ yyval = p_mknod(ID_INC,yyvsp[-2],yyvsp[0]); }
#line 1266 "y_tab.c"
break;
case 58:
  if (!yytrial)
#line 180 "parser.y"
{ yyval = p_mknod(ID_DEC,yyvsp[-2],yyvsp[0]); }
#line 1272 "y_tab.c"
break;
case 59:
  if (!yytrial)
#line 181 "parser.y"
{ yyval = p_mknod(ID_SET,
                                               p_mknod(ID_ARRAY, yyvsp[-5],yyvsp[-3]), yyvsp[0]); }
#line 1279 "y_tab.c"
break;
case 60:
  if (!yytrial)
#line 184 "parser.y"
{ yyval = p_mknod(ID_SET,
                                               p_mknod(ID_WHERE, yyvsp[-6], yyvsp[-3]), yyvsp[0]); }
#line 1286 "y_tab.c"
break;
case 61:
  if (!yytrial)
#line 189 "parser.y"
{ yyval = yyvsp[0]; }
#line 1292 "y_tab.c"
break;
case 62:
  if (!yytrial)
#line 190 "parser.y"
{ yyval = p_mknod(ID_CAT, yyvsp[-2], yyvsp[0]); }
#line 1298 "y_tab.c"
break;
case 63:
  if (!yytrial)
#line 194 "parser.y"
{ yyval = yyvsp[0]; }
#line 1304 "y_tab.c"
break;
case 64:
  if (!yytrial)
#line 195 "parser.y"
{ yyval = p_mknod(ID_OR ,yyvsp[-2],yyvsp[0]); }
#line 1310 "y_tab.c"
break;
case 65:
  if (!yytrial)
#line 199 "parser.y"
{ yyval = yyvsp[0]; }
#line 1316 "y_tab.c"
break;
case 66:
  if (!yytrial)
#line 200 "parser.y"
{ yyval = p_mknod(ID_AND ,yyvsp[-2],yyvsp[0]); }
#line 1322 "y_tab.c"
break;
case 67:
  if (!yytrial)
#line 204 "parser.y"
{ yyval = yyvsp[0]; }
#line 1328 "y_tab.c"
break;
case 68:
  if (!yytrial)
#line 205 "parser.y"
{ yyval = p_mknod(ID_EQ ,yyvsp[-2],yyvsp[0]); }
#line 1334 "y_tab.c"
break;
case 69:
  if (!yytrial)
#line 206 "parser.y"
{ yyval = p_mknod(ID_NE ,yyvsp[-2],yyvsp[0]); }
#line 1340 "y_tab.c"
break;
case 70:
  if (!yytrial)
#line 210 "parser.y"
{ yyval = yyvsp[0]; }
#line 1346 "y_tab.c"
break;
case 71:
  if (!yytrial)
#line 211 "parser.y"
{ yyval = p_mknod(ID_LT ,yyvsp[-2],yyvsp[0]); }
#line 1352 "y_tab.c"
break;
case 72:
  if (!yytrial)
#line 212 "parser.y"
{ yyval = p_mknod(ID_GT ,yyvsp[-2],yyvsp[0]); }
#line 1358 "y_tab.c"
break;
case 73:
  if (!yytrial)
#line 213 "parser.y"
{ yyval = p_mknod(ID_LE ,yyvsp[-2],yyvsp[0]); }
#line 1364 "y_tab.c"
break;
case 74:
  if (!yytrial)
#line 214 "parser.y"
{ yyval = p_mknod(ID_GE ,yyvsp[-2],yyvsp[0]); }
#line 1370 "y_tab.c"
break;
case 75:
  if (!yytrial)
#line 218 "parser.y"
{ yyval = yyvsp[0]; }
#line 1376 "y_tab.c"
break;
case 76:
  if (!yytrial)
#line 219 "parser.y"
{ yyval = p_mknod(ID_ADD,yyvsp[-2],yyvsp[0]); }
#line 1382 "y_tab.c"
break;
case 77:
  if (!yytrial)
#line 220 "parser.y"
{ yyval = p_mknod(ID_SUB,yyvsp[-2],yyvsp[0]); }
#line 1388 "y_tab.c"
break;
case 78:
  if (!yytrial)
#line 224 "parser.y"
{ yyval = yyvsp[0]; }
#line 1394 "y_tab.c"
break;
case 79:
  if (!yytrial)
#line 225 "parser.y"
{ yyval = p_mknod(ID_MULT, yyvsp[-2], yyvsp[0]); }
#line 1400 "y_tab.c"
break;
case 80:
  if (!yytrial)
#line 226 "parser.y"
{ yyval = p_mknod(ID_DIV, yyvsp[-2], yyvsp[0]); }
#line 1406 "y_tab.c"
break;
case 81:
  if (!yytrial)
#line 227 "parser.y"
{ yyval = p_mknod(ID_MOD, yyvsp[-2], yyvsp[0]); }
#line 1412 "y_tab.c"
break;
case 82:
  if (!yytrial)
#line 231 "parser.y"
{ yyval = yyvsp[0]; }
#line 1418 "y_tab.c"
break;
case 83:
  if (!yytrial)
#line 232 "parser.y"
{ yyval = p_mknod(ID_POW,yyvsp[-2],yyvsp[0]); }
#line 1424 "y_tab.c"
break;
case 84:
  if (!yytrial)
#line 236 "parser.y"
{ yyval = yyvsp[0]; }
#line 1430 "y_tab.c"
break;
case 85:
  if (!yytrial)
#line 237 "parser.y"
{ yyval = p_mknod(ID_UMINUS,NULL,yyvsp[0]); }
#line 1436 "y_tab.c"
break;
case 86:
  if (!yytrial)
#line 242 "parser.y"
{ yyval = yyvsp[0]; }
#line 1442 "y_tab.c"
break;
case 87:
  if (!yytrial)
#line 243 "parser.y"
{ yyval = p_mknod(ID_ARRAY,yyvsp[-3],yyvsp[-1]); }
#line 1448 "y_tab.c"
break;
case 88:
  if (!yytrial)
#line 244 "parser.y"
{ yyval = p_mknod(ID_FUNCT,yyvsp[-3],yyvsp[-1]); }
#line 1454 "y_tab.c"
break;
case 89:
  if (!yytrial)
#line 245 "parser.y"
{ yyval = p_mknod(ID_FUNCT,
                                                    p_mknod(ID_ARGV, yyvsp[-3], NULL),
                                                    yyvsp[-1]); }
#line 1462 "y_tab.c"
break;
case 90:
  if (!yytrial)
#line 248 "parser.y"
{ yyval = p_mknod(ID_FUNCT,
                                                    p_mknod(ID_ARGV, yyvsp[-3], NULL),
                                                    yyvsp[-1]); }
#line 1470 "y_tab.c"
break;
case 91:
  if (!yytrial)
#line 251 "parser.y"
{ yyval = yyvsp[-1]; }
#line 1476 "y_tab.c"
break;
case 92:
  if (!yytrial)
#line 255 "parser.y"
{ yyval = yyvsp[0]; }
#line 1482 "y_tab.c"
break;
case 93:
  if (!yytrial)
#line 256 "parser.y"
{ yyval = yyvsp[0]; }
#line 1488 "y_tab.c"
break;
case 94:
  if (!yytrial)
#line 257 "parser.y"
{ yyval = yyvsp[0]; }
#line 1494 "y_tab.c"
break;
case 95:
  if (!yytrial)
#line 258 "parser.y"
{ yyval = yyvsp[0]; }
#line 1500 "y_tab.c"
break;
case 96:
  if (!yytrial)
#line 259 "parser.y"
{ yyval = p_mknod(ID_ARGV, yyvsp[0], NULL); }
#line 1506 "y_tab.c"
break;
case 97:
  if (!yytrial)
#line 260 "parser.y"
{ yyval = p_mknod(ID_ARGV, yyvsp[0], NULL); }
#line 1512 "y_tab.c"
break;
case 98:
  if (!yytrial)
#line 261 "parser.y"
{ yyval = p_mknod(ID_ARGV, yyvsp[-3], yyvsp[-1]); }
#line 1518 "y_tab.c"
break;
case 99:
  if (!yytrial)
#line 264 "parser.y"
{ yyval = p_mkval(ID_ID, yyvsp[0]); }
#line 1524 "y_tab.c"
break;
case 100:
  if (!yytrial)
#line 265 "parser.y"
{ yyval = p_mkval(ID_IVAL, yyvsp[0]); }
#line 1530 "y_tab.c"
break;
case 101:
  if (!yytrial)
#line 266 "parser.y"
{ yyval = p_mkval(ID_RVAL, yyvsp[0]); }
#line 1536 "y_tab.c"
break;
case 102:
  if (!yytrial)
#line 267 "parser.y"
{ yyval = p_mkval(ID_STRING, yyvsp[0]); }
#line 1542 "y_tab.c"
break;
#line 1544 "y_tab.c"
#line 395 "push.skel"
    }

#if YYDEBUG && defined(YYDBPR)
    if (yydebug) {
	printf("yydebug: after reduction, result is ");
	YYDBPR(yyval);
	printf("\n"); }
#endif
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0) {
#if YYDEBUG
	if (yydebug)
	    printf("yydebug: after reduction, %s from state 0 to state %d\n",
		   "shifting", YYFINAL);
#endif
	yystate = YYFINAL;
	*++yyssp = YYFINAL;
	*++yyvsp = yyval;
	if (yychar == 0) goto yyaccept;
	goto yyloop; }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
	    yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
	yystate = yytable[yyn];
    else
	yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
	printf("yydebug: after reduction, %s from state %d to state %d\n",
	       "shifting", *yyssp, yystate);
#endif
    if (yyssp >= yyss + yystacksize - 1)
	YYMORESTACK;
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyvalid:
    if (yypath)
	goto yyabort;
    while (yypstate->save) {
	struct yyparsestate *save = yypstate->save;
	yypstate->save = save->save;
	save->save = yypath;
	yypath = save; }
#if YYDEBUG
    if (yydebug)
	printf("yydebug: trial successful, %s state %d, %d tokens\n",
	       "backtracking to", yypath->state,
	       (int)(yylvp - yylvals - yypath->lexeme));
#endif
    yychar = -1;
    yyssp = yyss + (yypath->ssp - yypath->ss);
    yyvsp = yyvs + (yypath->vsp - yypath->vs);
    memcpy(yyss, yypath->ss, (yyssp - yyss + 1) * sizeof(short));
    YYSCOPY(yyvs, yypath->vs, yyvsp - yyvs + 1);
    yylvp = yylvals + yypath->lexeme;
    yylexp = yylexemes + yypath->lexeme;
    yystate = yypath->state;
    goto yyloop;
yyabort:
    while (yypstate) {
	struct yyparsestate *save = yypstate;
	yypstate = save->save;
	YYFREESTATE(save); }
    while (yypath) {
	struct yyparsestate *save = yypath;
	yypath = save->save;
	YYFREESTATE(save); }
    return -1;
yyaccept:
    if (yytrial) goto yyvalid;
    while (yypstate) {
	struct yyparsestate *save = yypstate;
	yypstate = save->save;
	YYFREESTATE(save); }
    while (yypath) {
	struct yyparsestate *save = yypath;
	yypath = save->save;
	YYFREESTATE(save); }
    return 1;
}

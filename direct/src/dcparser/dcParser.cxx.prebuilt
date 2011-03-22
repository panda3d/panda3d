/* A Bison parser, made by GNU Bison 2.4.2.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2006, 2009-2010 Free Software
   Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse         dcyyparse
#define yylex           dcyylex
#define yyerror         dcyyerror
#define yylval          dcyylval
#define yychar          dcyychar
#define yydebug         dcyydebug
#define yynerrs         dcyynerrs


/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 6 "dcParser.yxx"

#include "dcLexerDefs.h"
#include "dcParserDefs.h"
#include "dcFile.h"
#include "dcClass.h"
#include "dcSwitch.h"
#include "dcAtomicField.h"
#include "dcMolecularField.h"
#include "dcClassParameter.h"
#include "dcSwitchParameter.h"
#include "dcArrayParameter.h"
#include "dcSimpleParameter.h"
#include "dcTypedef.h"
#include "dcKeyword.h"
#include "dcPacker.h"
#include "dcNumericRange.h"

// Because our token type contains objects of type string, which
// require correct copy construction (and not simply memcpying), we
// cannot use bison's built-in auto-stack-grow feature.  As an easy
// solution, we ensure here that we have enough yacc stack to start
// with, and that it doesn't ever try to grow.
#define YYINITDEPTH 1000
#define YYMAXDEPTH 1000

DCFile *dc_file = (DCFile *)NULL;
static DCClass *current_class = (DCClass *)NULL;
static DCSwitch *current_switch = (DCSwitch *)NULL;
static DCAtomicField *current_atomic = (DCAtomicField *)NULL;
static DCMolecularField *current_molecular = (DCMolecularField *)NULL;
static DCParameter *current_parameter = (DCParameter *)NULL;
static DCKeywordList current_keyword_list;
static DCPacker default_packer;
static DCPacker *current_packer;
static DCDoubleRange double_range;
static DCUnsignedIntRange uint_range;
static DCField *parameter_description = (DCField *)NULL;

////////////////////////////////////////////////////////////////////
// Defining the interface to the parser.
////////////////////////////////////////////////////////////////////

void
dc_init_parser(istream &in, const string &filename, DCFile &file) {
  dc_file = &file;
  dc_init_lexer(in, filename);
}

void
dc_init_parser_parameter_value(istream &in, const string &filename, 
                               DCPacker &packer) {
  dc_file = NULL;
  current_packer = &packer;
  dc_init_lexer(in, filename);
  dc_start_parameter_value();
}

void
dc_init_parser_parameter_description(istream &in, const string &filename,
                                     DCFile *file) {
  dc_file = file;
  dc_init_lexer(in, filename);
  parameter_description = NULL;
  dc_start_parameter_description();
}

DCField *
dc_get_parameter_description() {
  return parameter_description;
}

void
dc_cleanup_parser() {
  dc_file = (DCFile *)NULL;
}



/* Line 189 of yacc.c  */
#line 159 "y.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     UNSIGNED_INTEGER = 258,
     SIGNED_INTEGER = 259,
     REAL = 260,
     STRING = 261,
     HEX_STRING = 262,
     IDENTIFIER = 263,
     KEYWORD = 264,
     KW_DCLASS = 265,
     KW_STRUCT = 266,
     KW_FROM = 267,
     KW_IMPORT = 268,
     KW_TYPEDEF = 269,
     KW_KEYWORD = 270,
     KW_SWITCH = 271,
     KW_CASE = 272,
     KW_DEFAULT = 273,
     KW_BREAK = 274,
     KW_INT8 = 275,
     KW_INT16 = 276,
     KW_INT32 = 277,
     KW_INT64 = 278,
     KW_UINT8 = 279,
     KW_UINT16 = 280,
     KW_UINT32 = 281,
     KW_UINT64 = 282,
     KW_FLOAT64 = 283,
     KW_STRING = 284,
     KW_BLOB = 285,
     KW_BLOB32 = 286,
     KW_INT8ARRAY = 287,
     KW_INT16ARRAY = 288,
     KW_INT32ARRAY = 289,
     KW_UINT8ARRAY = 290,
     KW_UINT16ARRAY = 291,
     KW_UINT32ARRAY = 292,
     KW_UINT32UINT8ARRAY = 293,
     KW_CHAR = 294,
     START_DC = 295,
     START_PARAMETER_VALUE = 296,
     START_PARAMETER_DESCRIPTION = 297
   };
#endif
/* Tokens.  */
#define UNSIGNED_INTEGER 258
#define SIGNED_INTEGER 259
#define REAL 260
#define STRING 261
#define HEX_STRING 262
#define IDENTIFIER 263
#define KEYWORD 264
#define KW_DCLASS 265
#define KW_STRUCT 266
#define KW_FROM 267
#define KW_IMPORT 268
#define KW_TYPEDEF 269
#define KW_KEYWORD 270
#define KW_SWITCH 271
#define KW_CASE 272
#define KW_DEFAULT 273
#define KW_BREAK 274
#define KW_INT8 275
#define KW_INT16 276
#define KW_INT32 277
#define KW_INT64 278
#define KW_UINT8 279
#define KW_UINT16 280
#define KW_UINT32 281
#define KW_UINT64 282
#define KW_FLOAT64 283
#define KW_STRING 284
#define KW_BLOB 285
#define KW_BLOB32 286
#define KW_INT8ARRAY 287
#define KW_INT16ARRAY 288
#define KW_INT32ARRAY 289
#define KW_UINT8ARRAY 290
#define KW_UINT16ARRAY 291
#define KW_UINT32ARRAY 292
#define KW_UINT32UINT8ARRAY 293
#define KW_CHAR 294
#define START_DC 295
#define START_PARAMETER_VALUE 296
#define START_PARAMETER_DESCRIPTION 297




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 284 "y.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  57
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   430

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  58
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  82
/* YYNRULES -- Number of rules.  */
#define YYNRULES  190
/* YYNRULES -- Number of states.  */
#define YYNSTATES  282

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   297

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    54,     2,     2,
      51,    52,    46,     2,    47,    55,    45,    44,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    50,    43,
       2,    53,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    56,     2,    57,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    48,     2,    49,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,     9,    12,    14,    17,    20,    23,
      26,    29,    32,    34,    38,    40,    44,    47,    48,    54,
      56,    58,    60,    64,    67,    70,    72,    75,    78,    80,
      82,    83,    91,    93,    95,    98,   100,   104,   106,   109,
     113,   116,   119,   122,   125,   126,   134,   136,   138,   141,
     143,   147,   149,   152,   156,   159,   162,   165,   168,   169,
     175,   177,   179,   181,   185,   187,   188,   192,   194,   196,
     197,   202,   204,   205,   210,   212,   214,   216,   218,   220,
     222,   225,   228,   231,   233,   238,   242,   246,   248,   250,
     252,   254,   256,   258,   262,   265,   269,   275,   280,   282,
     284,   288,   291,   295,   301,   306,   308,   313,   315,   319,
     323,   328,   330,   332,   334,   336,   338,   340,   342,   344,
     346,   348,   350,   352,   353,   358,   360,   362,   364,   366,
     368,   369,   374,   375,   380,   381,   386,   390,   394,   398,
     402,   404,   407,   409,   411,   413,   417,   419,   421,   423,
     425,   427,   429,   431,   433,   435,   437,   439,   441,   443,
     445,   447,   449,   451,   453,   455,   457,   459,   462,   464,
     465,   470,   472,   474,   478,   480,   482,   483,   493,   495,
     498,   501,   504,   508,   512,   513,   518,   521,   523,   525,
     527
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      59,     0,    -1,    40,    60,    -1,    41,   114,    -1,    42,
     100,    -1,   139,    -1,    60,    43,    -1,    60,    70,    -1,
      60,   131,    -1,    60,    63,    -1,    60,    67,    -1,    60,
      68,    -1,     8,    -1,    61,    44,     8,    -1,    61,    -1,
      62,    45,    61,    -1,    13,    62,    -1,    -1,    12,    62,
      13,    64,    65,    -1,    66,    -1,    46,    -1,    61,    -1,
      66,    47,    61,    -1,    14,    98,    -1,    15,    69,    -1,
     139,    -1,    69,     8,    -1,    69,     9,    -1,    71,    -1,
      78,    -1,    -1,    10,   130,    72,    74,    48,    76,    49,
      -1,     8,    -1,   139,    -1,    50,    75,    -1,    73,    -1,
      75,    47,    73,    -1,   139,    -1,    76,    43,    -1,    76,
      77,    43,    -1,    85,   124,    -1,   126,   125,    -1,    95,
     124,    -1,    93,   124,    -1,    -1,    11,   130,    79,    81,
      48,    83,    49,    -1,     8,    -1,   139,    -1,    50,    82,
      -1,    80,    -1,    82,    47,    80,    -1,   139,    -1,    83,
      43,    -1,    83,    84,    43,    -1,    85,   125,    -1,   126,
     125,    -1,    95,   125,    -1,    93,   125,    -1,    -1,   130,
      51,    86,    87,    52,    -1,   139,    -1,    88,    -1,    89,
      -1,    88,    47,    89,    -1,    98,    -1,    -1,   105,    91,
     106,    -1,   105,    -1,    90,    -1,    -1,    90,    53,    94,
     114,    -1,    92,    -1,    -1,    92,    53,    96,   114,    -1,
      90,    -1,    92,    -1,    93,    -1,    95,    -1,    97,    -1,
      85,    -1,    85,   125,    -1,    95,   125,    -1,    93,   125,
      -1,   123,    -1,   101,    51,   103,    52,    -1,   101,    44,
     108,    -1,   101,    54,   112,    -1,   101,    -1,     8,    -1,
      78,    -1,   131,    -1,   139,    -1,   113,    -1,   113,    55,
     113,    -1,   113,   112,    -1,   103,    47,   113,    -1,   103,
      47,   113,    55,   113,    -1,   103,    47,   113,   112,    -1,
     139,    -1,   107,    -1,   107,    55,   107,    -1,   107,   109,
      -1,   104,    47,   107,    -1,   104,    47,   107,    55,   107,
      -1,   104,    47,   107,   109,    -1,   102,    -1,   105,    56,
     104,    57,    -1,     8,    -1,   106,    44,   108,    -1,   106,
      54,   112,    -1,   106,    56,   104,    57,    -1,     6,    -1,
     108,    -1,     3,    -1,     4,    -1,     4,    -1,     3,    -1,
     111,    -1,   110,    -1,     5,    -1,     6,    -1,   112,    -1,
     116,    -1,    -1,     8,    53,   115,   116,    -1,   110,    -1,
     111,    -1,     5,    -1,     6,    -1,     7,    -1,    -1,    48,
     117,   120,    49,    -1,    -1,    56,   118,   120,    57,    -1,
      -1,    51,   119,   120,    52,    -1,   110,    46,   108,    -1,
     111,    46,   108,    -1,     5,    46,   108,    -1,     7,    46,
     108,    -1,   121,    -1,   122,   121,    -1,   139,    -1,    47,
      -1,   114,    -1,   122,    47,   114,    -1,    20,    -1,    21,
      -1,    22,    -1,    23,    -1,    24,    -1,    25,    -1,    26,
      -1,    27,    -1,    28,    -1,    29,    -1,    30,    -1,    31,
      -1,    32,    -1,    33,    -1,    34,    -1,    35,    -1,    36,
      -1,    37,    -1,    38,    -1,    39,    -1,   139,    -1,   124,
       9,    -1,   124,    -1,    -1,     8,    50,   127,   129,    -1,
       8,    -1,   128,    -1,   129,    47,   128,    -1,   139,    -1,
       8,    -1,    -1,    16,   130,    51,    99,    52,    48,   132,
     133,    49,    -1,   139,    -1,   133,    43,    -1,   133,   134,
      -1,   133,   136,    -1,   133,   137,    43,    -1,   133,   138,
      43,    -1,    -1,    17,   135,   114,    50,    -1,    18,    50,
      -1,    19,    -1,    95,    -1,    93,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   168,   168,   169,   170,   177,   178,   179,   190,   196,
     197,   198,   202,   203,   210,   211,   218,   223,   222,   230,
     231,   238,   242,   249,   267,   271,   272,   276,   287,   288,
     293,   292,   304,   327,   328,   332,   338,   352,   353,   354,
     367,   377,   378,   386,   397,   396,   408,   431,   432,   436,
     442,   451,   452,   453,   464,   471,   472,   476,   484,   483,
     501,   502,   506,   507,   511,   521,   520,   531,   535,   537,
     536,   565,   567,   566,   595,   596,   600,   601,   605,   609,
     616,   620,   624,   631,   635,   644,   660,   675,   676,   708,
     724,   743,   747,   754,   761,   770,   776,   782,   793,   797,
     804,   811,   818,   824,   830,   839,   840,   851,   856,   871,
     886,   893,   902,   906,   917,   931,   935,   939,   943,   947,
     951,   960,   965,   969,   968,   983,   987,   991,   995,   999,
    1004,  1003,  1012,  1011,  1020,  1019,  1027,  1033,  1039,  1045,
    1054,  1055,  1059,  1060,  1064,  1065,  1069,  1073,  1077,  1081,
    1085,  1089,  1093,  1097,  1101,  1105,  1109,  1113,  1117,  1121,
    1125,  1129,  1133,  1137,  1141,  1145,  1152,  1156,  1163,  1173,
    1172,  1183,  1210,  1216,  1230,  1234,  1239,  1238,  1251,  1252,
    1253,  1254,  1255,  1256,  1270,  1269,  1290,  1299,  1306,  1310,
    1316
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "UNSIGNED_INTEGER", "SIGNED_INTEGER",
  "REAL", "STRING", "HEX_STRING", "IDENTIFIER", "KEYWORD", "KW_DCLASS",
  "KW_STRUCT", "KW_FROM", "KW_IMPORT", "KW_TYPEDEF", "KW_KEYWORD",
  "KW_SWITCH", "KW_CASE", "KW_DEFAULT", "KW_BREAK", "KW_INT8", "KW_INT16",
  "KW_INT32", "KW_INT64", "KW_UINT8", "KW_UINT16", "KW_UINT32",
  "KW_UINT64", "KW_FLOAT64", "KW_STRING", "KW_BLOB", "KW_BLOB32",
  "KW_INT8ARRAY", "KW_INT16ARRAY", "KW_INT32ARRAY", "KW_UINT8ARRAY",
  "KW_UINT16ARRAY", "KW_UINT32ARRAY", "KW_UINT32UINT8ARRAY", "KW_CHAR",
  "START_DC", "START_PARAMETER_VALUE", "START_PARAMETER_DESCRIPTION",
  "';'", "'/'", "'.'", "'*'", "','", "'{'", "'}'", "':'", "'('", "')'",
  "'='", "'%'", "'-'", "'['", "']'", "$accept", "grammar", "dc",
  "slash_identifier", "import_identifier", "import", "$@1",
  "import_symbol_list_or_star", "import_symbol_list", "typedef_decl",
  "keyword_decl", "keyword_decl_list", "dclass_or_struct", "dclass", "@2",
  "dclass_name", "dclass_derivation", "dclass_base_list", "dclass_fields",
  "dclass_field", "struct", "@3", "struct_name", "struct_derivation",
  "struct_base_list", "struct_fields", "struct_field", "atomic_field",
  "@4", "parameter_list", "nonempty_parameter_list", "atomic_element",
  "named_parameter", "$@5", "unnamed_parameter",
  "named_parameter_with_default", "$@6", "unnamed_parameter_with_default",
  "$@7", "parameter", "parameter_with_default", "parameter_or_atomic",
  "parameter_description", "simple_type_name", "type_name", "double_range",
  "uint_range", "type_definition", "parameter_definition", "char_or_uint",
  "small_unsigned_integer", "small_negative_integer", "signed_integer",
  "unsigned_integer", "number", "char_or_number", "parameter_value", "$@8",
  "parameter_actual_value", "$@9", "$@10", "$@11", "array", "maybe_comma",
  "array_def", "type_token", "keyword_list", "no_keyword_list",
  "molecular_field", "$@12", "atomic_name", "molecular_atom_list",
  "optional_name", "switch", "@13", "switch_fields", "switch_case", "$@14",
  "switch_default", "switch_break", "switch_field", "empty", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,    59,    47,    46,    42,    44,   123,   125,
      58,    40,    41,    61,    37,    45,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    58,    59,    59,    59,    60,    60,    60,    60,    60,
      60,    60,    61,    61,    62,    62,    63,    64,    63,    65,
      65,    66,    66,    67,    68,    69,    69,    69,    70,    70,
      72,    71,    73,    74,    74,    75,    75,    76,    76,    76,
      77,    77,    77,    77,    79,    78,    80,    81,    81,    82,
      82,    83,    83,    83,    84,    84,    84,    84,    86,    85,
      87,    87,    88,    88,    89,    91,    90,    92,    93,    94,
      93,    95,    96,    95,    97,    97,    98,    98,    99,    99,
     100,   100,   100,   101,   101,   101,   101,   102,   102,   102,
     102,   103,   103,   103,   103,   103,   103,   103,   104,   104,
     104,   104,   104,   104,   104,   105,   105,   106,   106,   106,
     106,   107,   107,   108,   109,   110,   111,   112,   112,   112,
     113,   113,   114,   115,   114,   116,   116,   116,   116,   116,
     117,   116,   118,   116,   119,   116,   116,   116,   116,   116,
     120,   120,   121,   121,   122,   122,   123,   123,   123,   123,
     123,   123,   123,   123,   123,   123,   123,   123,   123,   123,
     123,   123,   123,   123,   123,   123,   124,   124,   125,   127,
     126,   128,   129,   129,   130,   130,   132,   131,   133,   133,
     133,   133,   133,   133,   135,   134,   136,   137,   138,   138,
     139
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     2,     2,     1,     2,     2,     2,     2,
       2,     2,     1,     3,     1,     3,     2,     0,     5,     1,
       1,     1,     3,     2,     2,     1,     2,     2,     1,     1,
       0,     7,     1,     1,     2,     1,     3,     1,     2,     3,
       2,     2,     2,     2,     0,     7,     1,     1,     2,     1,
       3,     1,     2,     3,     2,     2,     2,     2,     0,     5,
       1,     1,     1,     3,     1,     0,     3,     1,     1,     0,
       4,     1,     0,     4,     1,     1,     1,     1,     1,     1,
       2,     2,     2,     1,     4,     3,     3,     1,     1,     1,
       1,     1,     1,     3,     2,     3,     5,     4,     1,     1,
       3,     2,     3,     5,     4,     1,     4,     1,     3,     3,
       4,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     0,     4,     1,     1,     1,     1,     1,
       0,     4,     0,     4,     0,     4,     3,     3,     3,     3,
       1,     2,     1,     1,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     1,     0,
       4,     1,     1,     3,     1,     1,     0,     9,     1,     2,
       2,     2,     3,     3,     0,     4,     2,     1,     1,     1,
       0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,   190,     0,   190,     0,     2,     5,   116,   115,   127,
     128,   129,     0,   130,   134,   132,   125,   126,     3,   122,
      88,   190,   190,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,    89,   190,    68,    71,   190,   190,     4,
      87,   105,    67,    83,     0,    90,   174,     1,   190,     0,
       0,     0,   190,     6,     9,    10,    11,     7,    28,    29,
       8,     0,     0,   123,   190,   190,   190,     0,     0,   175,
      44,     0,   168,    80,   166,    69,    72,    82,    81,     0,
     190,     0,   190,     0,    58,    30,    12,    14,     0,    16,
      88,    76,    77,    23,    24,    25,   113,   138,   139,     0,
     143,   144,     0,   140,   190,   142,     0,     0,   136,   137,
     190,   190,   167,     0,     0,    85,   119,   120,     0,   118,
     117,   121,    92,    91,    86,   111,     0,    99,   112,    98,
     107,    66,   190,   190,     0,    17,     0,    26,    27,   124,
     131,   143,   141,   135,   133,     0,     0,    47,    79,    74,
      75,    78,     0,    70,    73,     0,    84,     0,    94,     0,
     106,   114,     0,   101,     0,     0,   190,     0,    61,    62,
      64,    60,     0,     0,    33,    13,     0,    15,   145,    46,
      49,    48,   190,     0,    95,    93,   102,   100,   108,   109,
       0,    59,     0,    32,    35,    34,   190,    20,    21,    18,
      19,     0,   190,    51,   176,     0,    97,     0,   104,   110,
      63,     0,   190,    37,     0,    50,    88,    52,    45,     0,
     190,   190,   190,   190,   190,    96,   103,    36,    38,    31,
       0,   190,   190,   190,   190,    22,   169,    53,    54,    57,
      56,    55,     0,   178,    39,    40,    43,    42,    41,     0,
     184,     0,   187,   179,   177,   189,   188,   180,   181,     0,
       0,   171,   172,   170,     0,   186,   182,   183,     0,     0,
     173,   185
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     4,     5,    97,    98,    64,   186,   209,   210,    65,
      66,   104,    67,    68,   143,   204,   183,   205,   222,   240,
      43,   120,   190,   156,   191,   212,   229,    44,   142,   177,
     178,   179,    45,    93,    46,   101,   123,   102,   124,   161,
     180,   162,    49,    50,    51,   128,   136,    52,   141,   137,
     138,   173,    16,    17,   131,   132,   111,   109,    19,    74,
      76,    75,   112,   113,   114,    53,    82,    83,   233,   259,
     272,   273,    54,    55,   234,   252,   267,   274,   268,   269,
     270,    84
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -127
static const yytype_int16 yypact[] =
{
       8,  -127,   104,   359,    35,   128,  -127,  -127,  -127,    20,
    -127,    22,    28,  -127,  -127,  -127,    37,    53,  -127,  -127,
      54,   139,   139,  -127,  -127,  -127,  -127,  -127,  -127,  -127,
    -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,
    -127,  -127,  -127,  -127,  -127,    95,    97,  -127,  -127,  -127,
       2,  -127,    26,  -127,   100,  -127,  -127,  -127,   139,   145,
     145,   391,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,
    -127,   151,   151,  -127,    24,    24,    24,   151,   151,  -127,
    -127,   108,   152,  -127,  -127,  -127,  -127,  -127,  -127,   151,
     127,    98,    64,   154,  -127,  -127,  -127,   119,    13,   120,
    -127,  -127,  -127,  -127,   106,  -127,  -127,  -127,  -127,   122,
    -127,  -127,   115,  -127,   121,  -127,   114,   110,  -127,  -127,
     124,   359,  -127,   104,   104,  -127,  -127,  -127,    12,  -127,
    -127,  -127,    14,  -127,  -127,  -127,    16,     7,  -127,  -127,
    -127,     1,   391,   125,   161,  -127,   145,  -127,  -127,  -127,
    -127,   104,  -127,  -127,  -127,   164,   129,  -127,  -127,  -127,
    -127,  -127,   130,  -127,  -127,   127,  -127,   127,  -127,    64,
    -127,  -127,    64,  -127,   151,    98,    64,   131,   133,  -127,
    -127,  -127,   168,   136,  -127,  -127,     6,   119,  -127,  -127,
    -127,   134,  -127,   137,    33,  -127,    19,  -127,  -127,  -127,
      43,  -127,   391,  -127,  -127,   140,  -127,  -127,   119,  -127,
     141,   164,   291,  -127,  -127,   127,  -127,    64,  -127,  -127,
    -127,   168,   325,  -127,   145,  -127,    69,  -127,  -127,   143,
    -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,
     146,  -127,  -127,  -127,  -127,   119,  -127,  -127,  -127,  -127,
    -127,  -127,   257,  -127,  -127,   152,   152,   152,  -127,   186,
    -127,   147,  -127,  -127,  -127,  -127,  -127,  -127,  -127,   153,
     155,  -127,  -127,   156,   104,  -127,  -127,  -127,   186,   149,
    -127,  -127
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -127,  -127,  -127,  -126,   142,  -127,  -127,  -127,  -127,  -127,
    -127,  -127,  -127,  -127,  -127,   -21,  -127,  -127,  -127,  -127,
     196,  -127,    -6,  -127,  -127,  -127,  -127,  -116,  -127,  -127,
    -127,     4,    86,  -127,    87,    -1,  -127,     0,  -127,  -127,
     157,  -127,  -127,  -127,  -127,  -127,    34,  -127,  -127,  -125,
     -56,    17,   -81,   -78,   -90,  -124,    -2,  -127,   105,  -127,
    -127,  -127,    60,   102,  -127,  -127,   -85,   -40,    -5,  -127,
     -59,  -127,    18,   215,  -127,  -127,  -127,  -127,  -127,  -127,
    -127,     3
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -176
static const yytype_int16 yytable[] =
{
      18,   134,    47,    48,     6,   158,    56,    87,    88,   129,
     129,   171,   130,   130,    96,   107,   108,     7,     8,   126,
     187,   118,   119,   171,    56,    56,   145,     7,     8,     9,
      10,    11,    12,   125,   -65,    57,     7,     8,   126,    80,
      81,   194,   168,   195,   196,   174,    89,   197,     1,     2,
       3,   129,   207,    90,   130,   175,    91,   176,   146,   165,
     208,    56,   172,   169,   166,   105,    71,   106,    72,   167,
     135,   110,    13,   170,   217,    14,    95,   115,   115,   115,
      15,    73,    92,    77,   129,   199,   129,   130,   215,   130,
     169,   235,   236,   133,   129,   139,   230,   130,   245,    78,
     219,     7,     8,   126,   216,  -175,   241,     7,     8,     9,
      10,    11,    12,   129,   147,   148,   130,   115,   198,   246,
    -175,   163,   164,   157,    56,     7,     8,     9,    10,    11,
       7,     8,   126,   127,   129,   116,   117,   130,    58,    21,
      59,    60,    61,    62,    22,   181,   184,    79,    85,   188,
      86,    94,    13,    96,   106,    14,   255,   256,   257,   121,
      15,   122,   140,   144,   150,   146,   153,   154,   151,   185,
      13,    63,   189,    14,   155,   182,   203,   192,    15,   139,
     202,   211,   193,   201,   206,   214,   247,   221,   224,   254,
     248,   249,   250,   251,   271,   213,   276,   275,   277,   281,
     237,    69,    99,   278,   258,   225,   220,   159,   160,   223,
     200,   231,   232,   218,   149,    56,   152,   244,   103,   280,
      70,   242,   243,     0,     0,    56,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   253,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   265,   266,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   100,     0,     0,    21,     0,
       0,     0,   279,    22,   260,   261,   262,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,     0,     0,   226,
     263,     0,    21,     0,     0,     0,   264,    22,     0,     0,
       0,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,     0,     0,   226,   227,     0,    21,     0,     0,     0,
     228,    22,     0,     0,     0,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,     0,     0,    20,   238,     0,
      21,     0,     0,     0,   239,    22,     0,     0,     0,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,   100,
       0,     0,    21,     0,     0,     0,     0,    22,     0,     0,
       0,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42
};

static const yytype_int16 yycheck[] =
{
       2,    91,     3,     3,     1,   121,     3,    47,    48,    90,
      91,     4,    90,    91,     8,    71,    72,     3,     4,     5,
     146,    77,    78,     4,    21,    22,    13,     3,     4,     5,
       6,     7,     8,    89,     8,     0,     3,     4,     5,    21,
      22,   165,   132,   167,   169,    44,    44,   172,    40,    41,
      42,   132,    46,    51,   132,    54,    54,    56,    45,    47,
     186,    58,    55,    47,    52,    62,    46,     3,    46,    55,
       6,    47,    48,    57,    55,    51,    58,    74,    75,    76,
      56,    53,    56,    46,   165,   175,   167,   165,    55,   167,
      47,   215,   217,    90,   175,    92,   212,   175,   224,    46,
      57,     3,     4,     5,   194,    51,   222,     3,     4,     5,
       6,     7,     8,   194,     8,     9,   194,   114,   174,    50,
      51,   123,   124,   120,   121,     3,     4,     5,     6,     7,
       3,     4,     5,     6,   215,    75,    76,   215,    10,    11,
      12,    13,    14,    15,    16,   142,   143,     8,    53,   151,
      53,    51,    48,     8,     3,    51,   241,   242,   243,    51,
      56,     9,     8,    44,    49,    45,    52,    57,    47,     8,
      48,    43,     8,    51,    50,    50,     8,    48,    56,   176,
      47,    47,    52,    52,    48,    48,    43,    47,    47,    43,
     230,   231,   232,   233,     8,   192,    43,    50,    43,    50,
     221,     5,    60,    47,   244,   211,   202,   121,   121,   206,
     176,   212,   212,   196,   109,   212,   114,   222,    61,   278,
       5,   222,   222,    -1,    -1,   222,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   234,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   252,   252,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     8,    -1,    -1,    11,    -1,
      -1,    -1,   274,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    -1,    -1,     8,
      43,    -1,    11,    -1,    -1,    -1,    49,    16,    -1,    -1,
      -1,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    -1,    -1,     8,    43,    -1,    11,    -1,    -1,    -1,
      49,    16,    -1,    -1,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    -1,    -1,     8,    43,    -1,
      11,    -1,    -1,    -1,    49,    16,    -1,    -1,    -1,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,     8,
      -1,    -1,    11,    -1,    -1,    -1,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    40,    41,    42,    59,    60,   139,     3,     4,     5,
       6,     7,     8,    48,    51,    56,   110,   111,   114,   116,
       8,    11,    16,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    78,    85,    90,    92,    93,    95,   100,
     101,   102,   105,   123,   130,   131,   139,     0,    10,    12,
      13,    14,    15,    43,    63,    67,    68,    70,    71,    78,
     131,    46,    46,    53,   117,   119,   118,    46,    46,     8,
     130,   130,   124,   125,   139,    53,    53,   125,   125,    44,
      51,    54,    56,    91,    51,   130,     8,    61,    62,    62,
       8,    93,    95,    98,    69,   139,     3,   108,   108,   115,
      47,   114,   120,   121,   122,   139,   120,   120,   108,   108,
      79,    51,     9,    94,    96,   108,     5,     6,   103,   110,
     111,   112,   113,   139,   112,     6,   104,   107,   108,   139,
       8,   106,    86,    72,    44,    13,    45,     8,     9,   116,
      49,    47,   121,    52,    57,    50,    81,   139,    85,    90,
      92,    97,    99,   114,   114,    47,    52,    55,   112,    47,
      57,     4,    55,   109,    44,    54,    56,    87,    88,    89,
      98,   139,    50,    74,   139,     8,    64,    61,   114,     8,
      80,    82,    48,    52,   113,   113,   107,   107,   108,   112,
     104,    52,    47,     8,    73,    75,    48,    46,    61,    65,
      66,    47,    83,   139,    48,    55,   112,    55,   109,    57,
      89,    47,    76,   139,    47,    80,     8,    43,    49,    84,
      85,    93,    95,   126,   132,   113,   107,    73,    43,    49,
      77,    85,    93,    95,   126,    61,    50,    43,   125,   125,
     125,   125,   133,   139,    43,   124,   124,   124,   125,   127,
      17,    18,    19,    43,    49,    93,    95,   134,   136,   137,
     138,     8,   128,   129,   135,    50,    43,    43,    47,   114,
     128,    50
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 4:

/* Line 1464 of yacc.c  */
#line 171 "dcParser.yxx"
    {
  parameter_description = (yyvsp[(2) - (2)].u.field);
}
    break;

  case 7:

/* Line 1464 of yacc.c  */
#line 180 "dcParser.yxx"
    {
  if (!dc_file->add_class((yyvsp[(2) - (2)].u.dclass))) {
    DCClass *old_class = dc_file->get_class_by_name((yyvsp[(2) - (2)].u.dclass)->get_name());
    if (old_class != (DCClass *)NULL && old_class->is_bogus_class()) {
      yyerror("Base class defined after its first reference: " + (yyvsp[(2) - (2)].u.dclass)->get_name());
    } else {
      yyerror("Duplicate class name: " + (yyvsp[(2) - (2)].u.dclass)->get_name());
    }
  }
}
    break;

  case 8:

/* Line 1464 of yacc.c  */
#line 191 "dcParser.yxx"
    {
  if (!dc_file->add_switch((yyvsp[(2) - (2)].u.dswitch))) {
    yyerror("Duplicate class name: " + (yyvsp[(2) - (2)].u.dswitch)->get_name());
  }
}
    break;

  case 13:

/* Line 1464 of yacc.c  */
#line 204 "dcParser.yxx"
    {
  (yyval.str) = (yyvsp[(1) - (3)].str) + string("/") + (yyvsp[(3) - (3)].str);
}
    break;

  case 15:

/* Line 1464 of yacc.c  */
#line 212 "dcParser.yxx"
    {
  (yyval.str) = (yyvsp[(1) - (3)].str) + string(".") + (yyvsp[(3) - (3)].str);
}
    break;

  case 16:

/* Line 1464 of yacc.c  */
#line 219 "dcParser.yxx"
    {
  dc_file->add_import_module((yyvsp[(2) - (2)].str));
}
    break;

  case 17:

/* Line 1464 of yacc.c  */
#line 223 "dcParser.yxx"
    {
  dc_file->add_import_module((yyvsp[(2) - (3)].str));
}
    break;

  case 20:

/* Line 1464 of yacc.c  */
#line 232 "dcParser.yxx"
    {
  dc_file->add_import_symbol("*");
}
    break;

  case 21:

/* Line 1464 of yacc.c  */
#line 239 "dcParser.yxx"
    {
  dc_file->add_import_symbol((yyvsp[(1) - (1)].str));
}
    break;

  case 22:

/* Line 1464 of yacc.c  */
#line 243 "dcParser.yxx"
    {
  dc_file->add_import_symbol((yyvsp[(3) - (3)].str));
}
    break;

  case 23:

/* Line 1464 of yacc.c  */
#line 250 "dcParser.yxx"
    {
  if ((yyvsp[(2) - (2)].u.parameter) != (DCParameter *)NULL) {
    DCTypedef *dtypedef = new DCTypedef((yyvsp[(2) - (2)].u.parameter));
    
    if (!dc_file->add_typedef(dtypedef)) {
      DCTypedef *old_typedef = dc_file->get_typedef_by_name(dtypedef->get_name());
      if (old_typedef->is_bogus_typedef()) {
        yyerror("typedef defined after its first reference: " + dtypedef->get_name());
      } else {
        yyerror("Duplicate typedef name: " + dtypedef->get_name());
      }
    }
  }
}
    break;

  case 26:

/* Line 1464 of yacc.c  */
#line 273 "dcParser.yxx"
    {
  dc_file->add_keyword((yyvsp[(2) - (2)].str));
}
    break;

  case 27:

/* Line 1464 of yacc.c  */
#line 277 "dcParser.yxx"
    {
  // This keyword has already been defined.  But since we are now
  // explicitly defining it, clear its bitmask, so that we will have a
  // new hash code--doing this will allow us to phase out the
  // historical hash code support later.
  ((DCKeyword *)(yyvsp[(2) - (2)].u.keyword))->clear_historical_flag();
}
    break;

  case 30:

/* Line 1464 of yacc.c  */
#line 293 "dcParser.yxx"
    {
  current_class = new DCClass(dc_file, (yyvsp[(2) - (2)].str), false, false);
}
    break;

  case 31:

/* Line 1464 of yacc.c  */
#line 297 "dcParser.yxx"
    {
  (yyval.u.dclass) = current_class;
  current_class = (yyvsp[(3) - (7)].u.dclass);
}
    break;

  case 32:

/* Line 1464 of yacc.c  */
#line 305 "dcParser.yxx"
    {
  if (dc_file == (DCFile *)NULL) {
    yyerror("No DCFile available, so no class names are predefined.");
    (yyval.u.dclass) = NULL;

  } else {
    DCClass *dclass = dc_file->get_class_by_name((yyvsp[(1) - (1)].str));
    if (dclass == (DCClass *)NULL) {
      // Create a bogus class as a forward reference.
      dclass = new DCClass(dc_file, (yyvsp[(1) - (1)].str), false, true);
      dc_file->add_class(dclass);
    }
    if (dclass->is_struct()) {
      yyerror("struct name not allowed");
    }
  
    (yyval.u.dclass) = dclass;
  }
}
    break;

  case 35:

/* Line 1464 of yacc.c  */
#line 333 "dcParser.yxx"
    {
  if ((yyvsp[(1) - (1)].u.dclass) != (DCClass *)NULL) {
    current_class->add_parent((yyvsp[(1) - (1)].u.dclass));
  }
}
    break;

  case 36:

/* Line 1464 of yacc.c  */
#line 339 "dcParser.yxx"
    {
  if (!dc_multiple_inheritance) {
    yyerror("Multiple inheritance is not supported without \"dc-multiple-inheritance 1\" in your Config.prc file.");

  } else {
    if ((yyvsp[(3) - (3)].u.dclass) != (DCClass *)NULL) {
      current_class->add_parent((yyvsp[(3) - (3)].u.dclass));
    }
  }
}
    break;

  case 39:

/* Line 1464 of yacc.c  */
#line 355 "dcParser.yxx"
    {
  if ((yyvsp[(2) - (3)].u.field) == (DCField *)NULL) {
    // Pass this error up.
  } else if (!current_class->add_field((yyvsp[(2) - (3)].u.field))) {
    yyerror("Duplicate field name: " + (yyvsp[(2) - (3)].u.field)->get_name());
  } else if ((yyvsp[(2) - (3)].u.field)->get_number() < 0) {
    yyerror("A non-network field cannot be stored on a dclass");
  }
}
    break;

  case 40:

/* Line 1464 of yacc.c  */
#line 368 "dcParser.yxx"
    {
  if ((yyvsp[(1) - (2)].u.field) != (DCField *)NULL) {
    if ((yyvsp[(1) - (2)].u.field)->get_name().empty()) {
      yyerror("Field name required.");
    }
    (yyvsp[(1) - (2)].u.field)->copy_keywords(current_keyword_list);
  }
  (yyval.u.field) = (yyvsp[(1) - (2)].u.field);
}
    break;

  case 42:

/* Line 1464 of yacc.c  */
#line 379 "dcParser.yxx"
    {
  yyerror("Unnamed parameters are not allowed on a dclass");
  if ((yyvsp[(1) - (2)].u.parameter) != (DCField *)NULL) {
    (yyvsp[(1) - (2)].u.parameter)->copy_keywords(current_keyword_list);
  }
  (yyval.u.field) = (yyvsp[(1) - (2)].u.parameter);
}
    break;

  case 43:

/* Line 1464 of yacc.c  */
#line 387 "dcParser.yxx"
    {
  if ((yyvsp[(1) - (2)].u.parameter) != (DCField *)NULL) {
    (yyvsp[(1) - (2)].u.parameter)->copy_keywords(current_keyword_list);
  }
  (yyval.u.field) = (yyvsp[(1) - (2)].u.parameter);
}
    break;

  case 44:

/* Line 1464 of yacc.c  */
#line 397 "dcParser.yxx"
    {
  current_class = new DCClass(dc_file, (yyvsp[(2) - (2)].str), true, false);
}
    break;

  case 45:

/* Line 1464 of yacc.c  */
#line 401 "dcParser.yxx"
    {
  (yyval.u.dclass) = current_class;
  current_class = (yyvsp[(3) - (7)].u.dclass);
}
    break;

  case 46:

/* Line 1464 of yacc.c  */
#line 409 "dcParser.yxx"
    {
  if (dc_file == (DCFile *)NULL) {
    yyerror("No DCFile available, so no struct names are predefined.");
    (yyval.u.dclass) = NULL;

  } else {
    DCClass *dstruct = dc_file->get_class_by_name((yyvsp[(1) - (1)].str));
    if (dstruct == (DCClass *)NULL) {
      // Create a bogus class as a forward reference.
      dstruct = new DCClass(dc_file, (yyvsp[(1) - (1)].str), false, true);
      dc_file->add_class(dstruct);
    }
    if (!dstruct->is_struct()) {
      yyerror("struct name required");
    }
  
    (yyval.u.dclass) = dstruct;
  }
}
    break;

  case 49:

/* Line 1464 of yacc.c  */
#line 437 "dcParser.yxx"
    {
  if ((yyvsp[(1) - (1)].u.dclass) != (DCClass *)NULL) {
    current_class->add_parent((yyvsp[(1) - (1)].u.dclass));
  }
}
    break;

  case 50:

/* Line 1464 of yacc.c  */
#line 443 "dcParser.yxx"
    {
  if ((yyvsp[(3) - (3)].u.dclass) != (DCClass *)NULL) {
    current_class->add_parent((yyvsp[(3) - (3)].u.dclass));
  }
}
    break;

  case 53:

/* Line 1464 of yacc.c  */
#line 454 "dcParser.yxx"
    {
  if ((yyvsp[(2) - (3)].u.field) == (DCField *)NULL) {
    // Pass this error up.
  } else if (!current_class->add_field((yyvsp[(2) - (3)].u.field))) {
    yyerror("Duplicate field name: " + (yyvsp[(2) - (3)].u.field)->get_name());
  }
}
    break;

  case 54:

/* Line 1464 of yacc.c  */
#line 465 "dcParser.yxx"
    {
  if ((yyvsp[(1) - (2)].u.field)->get_name().empty()) {
    yyerror("Field name required.");
  }
  (yyval.u.field) = (yyvsp[(1) - (2)].u.field);
}
    break;

  case 56:

/* Line 1464 of yacc.c  */
#line 473 "dcParser.yxx"
    {
  (yyval.u.field) = (yyvsp[(1) - (2)].u.parameter);
}
    break;

  case 57:

/* Line 1464 of yacc.c  */
#line 477 "dcParser.yxx"
    {
  (yyval.u.field) = (yyvsp[(1) - (2)].u.parameter);
}
    break;

  case 58:

/* Line 1464 of yacc.c  */
#line 484 "dcParser.yxx"
    {
  if (current_class == (DCClass *)NULL) {
    yyerror("Cannot define a method outside of a struct or class.");
    DCClass *temp_class = new DCClass(dc_file, "temp", false, false);  // memory leak.
    current_atomic = new DCAtomicField((yyvsp[(1) - (2)].str), temp_class, false);
  } else {
    current_atomic = new DCAtomicField((yyvsp[(1) - (2)].str), current_class, false);
  }
}
    break;

  case 59:

/* Line 1464 of yacc.c  */
#line 494 "dcParser.yxx"
    {
  (yyval.u.field) = current_atomic;
  current_atomic = (yyvsp[(3) - (5)].u.atomic);
}
    break;

  case 64:

/* Line 1464 of yacc.c  */
#line 512 "dcParser.yxx"
    {
  if ((yyvsp[(1) - (1)].u.parameter) != (DCParameter *)NULL) {
    current_atomic->add_element((yyvsp[(1) - (1)].u.parameter));
  }
}
    break;

  case 65:

/* Line 1464 of yacc.c  */
#line 521 "dcParser.yxx"
    {
  current_parameter = (yyvsp[(1) - (1)].u.parameter);
}
    break;

  case 66:

/* Line 1464 of yacc.c  */
#line 525 "dcParser.yxx"
    {
  (yyval.u.parameter) = (yyvsp[(3) - (3)].u.parameter);
}
    break;

  case 69:

/* Line 1464 of yacc.c  */
#line 537 "dcParser.yxx"
    {
  current_packer = &default_packer;
  current_packer->clear_data();
  if ((yyvsp[(1) - (2)].u.parameter) != (DCField *)NULL) {
    current_packer->begin_pack((yyvsp[(1) - (2)].u.parameter));
  }
}
    break;

  case 70:

/* Line 1464 of yacc.c  */
#line 545 "dcParser.yxx"
    {
  bool is_valid = false;
  if ((yyvsp[(1) - (4)].u.parameter) != (DCField *)NULL) {
    is_valid = (yyvsp[(1) - (4)].u.parameter)->is_valid();
  }
  if (current_packer->end_pack()) {
    (yyvsp[(1) - (4)].u.parameter)->set_default_value(current_packer->get_string());

  } else {
    if (is_valid) {
      yyerror("Invalid default value for type");
    }
    // If the current parameter isn't valid, we don't mind a pack
    // error (there's no way for us to validate the syntax).  So we'll
    // just ignore the default value in this case.
  }
}
    break;

  case 72:

/* Line 1464 of yacc.c  */
#line 567 "dcParser.yxx"
    {
  current_packer = &default_packer;
  current_packer->clear_data();
  if ((yyvsp[(1) - (2)].u.parameter) != (DCField *)NULL) {
    current_packer->begin_pack((yyvsp[(1) - (2)].u.parameter));
  }
}
    break;

  case 73:

/* Line 1464 of yacc.c  */
#line 575 "dcParser.yxx"
    {
  bool is_valid = false;
  if ((yyvsp[(1) - (4)].u.parameter) != (DCField *)NULL) {
    is_valid = (yyvsp[(1) - (4)].u.parameter)->is_valid();
  }
  if (current_packer->end_pack()) {
    (yyvsp[(1) - (4)].u.parameter)->set_default_value(current_packer->get_string());

  } else {
    if (is_valid) {
      yyerror("Invalid default value for type");
    }
    // If the current parameter isn't valid, we don't mind a pack
    // error (there's no way for us to validate the syntax).  So we'll
    // just ignore the default value in this case.
  }
}
    break;

  case 78:

/* Line 1464 of yacc.c  */
#line 606 "dcParser.yxx"
    {
  (yyval.u.field) = (yyvsp[(1) - (1)].u.parameter);
}
    break;

  case 79:

/* Line 1464 of yacc.c  */
#line 610 "dcParser.yxx"
    {
  (yyval.u.field) = (yyvsp[(1) - (1)].u.field);
}
    break;

  case 80:

/* Line 1464 of yacc.c  */
#line 617 "dcParser.yxx"
    {
  (yyval.u.field) = (yyvsp[(1) - (2)].u.field);
}
    break;

  case 81:

/* Line 1464 of yacc.c  */
#line 621 "dcParser.yxx"
    {
  (yyval.u.field) = (yyvsp[(1) - (2)].u.parameter);
}
    break;

  case 82:

/* Line 1464 of yacc.c  */
#line 625 "dcParser.yxx"
    {
  (yyval.u.field) = (yyvsp[(1) - (2)].u.parameter);
}
    break;

  case 83:

/* Line 1464 of yacc.c  */
#line 632 "dcParser.yxx"
    {
  (yyval.u.parameter) = new DCSimpleParameter((yyvsp[(1) - (1)].u.subatomic));
}
    break;

  case 84:

/* Line 1464 of yacc.c  */
#line 636 "dcParser.yxx"
    {
  DCSimpleParameter *simple_param = (yyvsp[(1) - (4)].u.parameter)->as_simple_parameter();
  nassertr(simple_param != (DCSimpleParameter *)NULL, 0);
  if (!simple_param->set_range(double_range)) {
    yyerror("Inappropriate range for type");
  }
  (yyval.u.parameter) = simple_param;
}
    break;

  case 85:

/* Line 1464 of yacc.c  */
#line 645 "dcParser.yxx"
    {
  DCSimpleParameter *simple_param = (yyvsp[(1) - (3)].u.parameter)->as_simple_parameter();
  nassertr(simple_param != (DCSimpleParameter *)NULL, 0);
  if (!simple_param->is_numeric_type()) {
    yyerror("A divisor is only valid on a numeric type.");

  } else if (!simple_param->set_divisor((yyvsp[(3) - (3)].u.s_uint))) {
    yyerror("Invalid divisor.");

  } else if (simple_param->has_modulus() && !simple_param->set_modulus(simple_param->get_modulus())) {
    // Changing the divisor may change the valid range for the modulus.
    yyerror("Invalid modulus.");
  }
  (yyval.u.parameter) = simple_param;
}
    break;

  case 86:

/* Line 1464 of yacc.c  */
#line 661 "dcParser.yxx"
    { 
  DCSimpleParameter *simple_param = (yyvsp[(1) - (3)].u.parameter)->as_simple_parameter();
  nassertr(simple_param != (DCSimpleParameter *)NULL, 0);
  if (!simple_param->is_numeric_type()) {
    yyerror("A divisor is only valid on a numeric type.");

  } else if (!simple_param->set_modulus((yyvsp[(3) - (3)].u.real))) {
    yyerror("Invalid modulus.");
  }
  (yyval.u.parameter) = simple_param;
}
    break;

  case 88:

/* Line 1464 of yacc.c  */
#line 677 "dcParser.yxx"
    {
  if (dc_file == (DCFile *)NULL) {
    yyerror("Invalid type.");
    (yyval.u.parameter) = NULL;

  } else {
    DCTypedef *dtypedef = dc_file->get_typedef_by_name((yyvsp[(1) - (1)].str));
    if (dtypedef == (DCTypedef *)NULL) {
      // Maybe it's a class name.
      DCClass *dclass = dc_file->get_class_by_name((yyvsp[(1) - (1)].str));
      if (dclass != (DCClass *)NULL) {
        // Create an implicit typedef for this.
        dtypedef = new DCTypedef(new DCClassParameter(dclass), true);
      } else {
        // Maybe it's a switch name.
        DCSwitch *dswitch = dc_file->get_switch_by_name((yyvsp[(1) - (1)].str));
        if (dswitch != (DCSwitch *)NULL) {
          // This also gets an implicit typedef.
          dtypedef = new DCTypedef(new DCSwitchParameter(dswitch), true);
        } else {
          // It's an undefined typedef.  Create a bogus forward reference.
          dtypedef = new DCTypedef((yyvsp[(1) - (1)].str));
        }
      }
      
      dc_file->add_typedef(dtypedef);
    }
    
    (yyval.u.parameter) = dtypedef->make_new_parameter();
  }
}
    break;

  case 89:

/* Line 1464 of yacc.c  */
#line 709 "dcParser.yxx"
    {
  // This is an inline struct definition.
  if ((yyvsp[(1) - (1)].u.dclass) == (DCClass *)NULL) {
    (yyval.u.parameter) = NULL;
  } else {
    if (dc_file != (DCFile *)NULL) {
      dc_file->add_thing_to_delete((yyvsp[(1) - (1)].u.dclass));
    } else {
      // This is a memory leak--this happens when we put an anonymous
      // struct reference within the string passed to
      // DCPackerInterface::check_match().  Maybe it doesn't really matter.
    }
    (yyval.u.parameter) = new DCClassParameter((yyvsp[(1) - (1)].u.dclass));
  }
}
    break;

  case 90:

/* Line 1464 of yacc.c  */
#line 725 "dcParser.yxx"
    {
  // This is an inline switch definition.
  if ((yyvsp[(1) - (1)].u.dswitch) == (DCSwitch *)NULL) {
    (yyval.u.parameter) = NULL;
  } else {
    if (dc_file != (DCFile *)NULL) {
      dc_file->add_thing_to_delete((yyvsp[(1) - (1)].u.dswitch));
    } else {
      // This is a memory leak--this happens when we put an anonymous
      // switch reference within the string passed to
      // DCPackerInterface::check_match().  Maybe it doesn't really matter.
    }
    (yyval.u.parameter) = new DCSwitchParameter((yyvsp[(1) - (1)].u.dswitch));
  }
}
    break;

  case 91:

/* Line 1464 of yacc.c  */
#line 744 "dcParser.yxx"
    {
  double_range.clear();
}
    break;

  case 92:

/* Line 1464 of yacc.c  */
#line 748 "dcParser.yxx"
    {
  double_range.clear();
  if (!double_range.add_range((yyvsp[(1) - (1)].u.real), (yyvsp[(1) - (1)].u.real))) {
    yyerror("Overlapping range");
  }
}
    break;

  case 93:

/* Line 1464 of yacc.c  */
#line 755 "dcParser.yxx"
    {
  double_range.clear();
  if (!double_range.add_range((yyvsp[(1) - (3)].u.real), (yyvsp[(3) - (3)].u.real))) {
    yyerror("Overlapping range");
  }
}
    break;

  case 94:

/* Line 1464 of yacc.c  */
#line 762 "dcParser.yxx"
    {
  double_range.clear();
  if ((yyvsp[(2) - (2)].u.real) >= 0) {
    yyerror("Syntax error");
  } else if (!double_range.add_range((yyvsp[(1) - (2)].u.real), -(yyvsp[(2) - (2)].u.real))) {
    yyerror("Overlapping range");
  }
}
    break;

  case 95:

/* Line 1464 of yacc.c  */
#line 771 "dcParser.yxx"
    {
  if (!double_range.add_range((yyvsp[(3) - (3)].u.real), (yyvsp[(3) - (3)].u.real))) {
    yyerror("Overlapping range");
  }
}
    break;

  case 96:

/* Line 1464 of yacc.c  */
#line 777 "dcParser.yxx"
    {
  if (!double_range.add_range((yyvsp[(3) - (5)].u.real), (yyvsp[(5) - (5)].u.real))) {
    yyerror("Overlapping range");
  }
}
    break;

  case 97:

/* Line 1464 of yacc.c  */
#line 783 "dcParser.yxx"
    {
  if ((yyvsp[(4) - (4)].u.real) >= 0) {
    yyerror("Syntax error");
  } else if (!double_range.add_range((yyvsp[(3) - (4)].u.real), -(yyvsp[(4) - (4)].u.real))) {
    yyerror("Overlapping range");
  }
}
    break;

  case 98:

/* Line 1464 of yacc.c  */
#line 794 "dcParser.yxx"
    {
  uint_range.clear();
}
    break;

  case 99:

/* Line 1464 of yacc.c  */
#line 798 "dcParser.yxx"
    {
  uint_range.clear();
  if (!uint_range.add_range((yyvsp[(1) - (1)].u.s_uint), (yyvsp[(1) - (1)].u.s_uint))) {
    yyerror("Overlapping range");
  }
}
    break;

  case 100:

/* Line 1464 of yacc.c  */
#line 805 "dcParser.yxx"
    {
  uint_range.clear();
  if (!uint_range.add_range((yyvsp[(1) - (3)].u.s_uint), (yyvsp[(3) - (3)].u.s_uint))) {
    yyerror("Overlapping range");
  }
}
    break;

  case 101:

/* Line 1464 of yacc.c  */
#line 812 "dcParser.yxx"
    {
  uint_range.clear();
  if (!uint_range.add_range((yyvsp[(1) - (2)].u.s_uint), (yyvsp[(2) - (2)].u.s_uint))) {
    yyerror("Overlapping range");
  }
}
    break;

  case 102:

/* Line 1464 of yacc.c  */
#line 819 "dcParser.yxx"
    {
  if (!uint_range.add_range((yyvsp[(3) - (3)].u.s_uint), (yyvsp[(3) - (3)].u.s_uint))) {
    yyerror("Overlapping range");
  }
}
    break;

  case 103:

/* Line 1464 of yacc.c  */
#line 825 "dcParser.yxx"
    {
  if (!uint_range.add_range((yyvsp[(3) - (5)].u.s_uint), (yyvsp[(5) - (5)].u.s_uint))) {
    yyerror("Overlapping range");
  }
}
    break;

  case 104:

/* Line 1464 of yacc.c  */
#line 831 "dcParser.yxx"
    {
  if (!uint_range.add_range((yyvsp[(3) - (4)].u.s_uint), (yyvsp[(4) - (4)].u.s_uint))) {
    yyerror("Overlapping range");
  }
}
    break;

  case 106:

/* Line 1464 of yacc.c  */
#line 841 "dcParser.yxx"
    {
  if ((yyvsp[(1) - (4)].u.parameter) == (DCParameter *)NULL) {
    (yyval.u.parameter) = NULL;
  } else {
    (yyval.u.parameter) = (yyvsp[(1) - (4)].u.parameter)->append_array_specification(uint_range);
  }
}
    break;

  case 107:

/* Line 1464 of yacc.c  */
#line 852 "dcParser.yxx"
    {
  current_parameter->set_name((yyvsp[(1) - (1)].str));
  (yyval.u.parameter) = current_parameter;
}
    break;

  case 108:

/* Line 1464 of yacc.c  */
#line 857 "dcParser.yxx"
    {
  DCSimpleParameter *simple_param = (yyvsp[(1) - (3)].u.parameter)->as_simple_parameter();
  if (simple_param == NULL || simple_param->get_typedef() != (DCTypedef *)NULL) {
    yyerror("A divisor is only allowed on a primitive type.");

  } else if (!simple_param->is_numeric_type()) {
      yyerror("A divisor is only valid on a numeric type.");

  } else {
    if (!simple_param->set_divisor((yyvsp[(3) - (3)].u.s_uint))) {
      yyerror("Invalid divisor.");
    }
  }
}
    break;

  case 109:

/* Line 1464 of yacc.c  */
#line 872 "dcParser.yxx"
    {
  DCSimpleParameter *simple_param = (yyvsp[(1) - (3)].u.parameter)->as_simple_parameter();
  if (simple_param == NULL || simple_param->get_typedef() != (DCTypedef *)NULL) {
    yyerror("A modulus is only allowed on a primitive type.");

  } else if (!simple_param->is_numeric_type()) {
      yyerror("A modulus is only valid on a numeric type.");

  } else {
    if (!simple_param->set_modulus((yyvsp[(3) - (3)].u.real))) {
      yyerror("Invalid modulus.");
    }
  }
}
    break;

  case 110:

/* Line 1464 of yacc.c  */
#line 887 "dcParser.yxx"
    {
  (yyval.u.parameter) = (yyvsp[(1) - (4)].u.parameter)->append_array_specification(uint_range);
}
    break;

  case 111:

/* Line 1464 of yacc.c  */
#line 894 "dcParser.yxx"
    {
  if ((yyvsp[(1) - (1)].str).length() != 1) {
    yyerror("Single character required.");
    (yyval.u.s_uint) = 0;
  } else {
    (yyval.u.s_uint) = (unsigned char)(yyvsp[(1) - (1)].str)[0];
  }
}
    break;

  case 113:

/* Line 1464 of yacc.c  */
#line 907 "dcParser.yxx"
    {
  (yyval.u.s_uint) = (unsigned int)(yyvsp[(1) - (1)].u.uint64);
  if ((yyval.u.s_uint) != (yyvsp[(1) - (1)].u.uint64)) {
    yyerror("Number out of range.");
    (yyval.u.s_uint) = 1;
  }
}
    break;

  case 114:

/* Line 1464 of yacc.c  */
#line 918 "dcParser.yxx"
    {
  (yyval.u.s_uint) = (unsigned int)-(yyvsp[(1) - (1)].u.int64);
  if ((yyvsp[(1) - (1)].u.int64) >= 0) {
    yyerror("Syntax error.");

  } else if ((yyval.u.s_uint) != -(yyvsp[(1) - (1)].u.int64)) {
    yyerror("Number out of range.");
    (yyval.u.s_uint) = 1;
  }
}
    break;

  case 117:

/* Line 1464 of yacc.c  */
#line 940 "dcParser.yxx"
    {
  (yyval.u.real) = (double)(yyvsp[(1) - (1)].u.uint64);
}
    break;

  case 118:

/* Line 1464 of yacc.c  */
#line 944 "dcParser.yxx"
    {
  (yyval.u.real) = (double)(yyvsp[(1) - (1)].u.int64);
}
    break;

  case 120:

/* Line 1464 of yacc.c  */
#line 952 "dcParser.yxx"
    {
  if ((yyvsp[(1) - (1)].str).length() != 1) {
    yyerror("Single character required.");
    (yyval.u.real) = 0;
  } else {
    (yyval.u.real) = (double)(unsigned char)(yyvsp[(1) - (1)].str)[0];
  }
}
    break;

  case 122:

/* Line 1464 of yacc.c  */
#line 966 "dcParser.yxx"
    {
}
    break;

  case 123:

/* Line 1464 of yacc.c  */
#line 969 "dcParser.yxx"
    {
  if ((yyvsp[(1) - (2)].str) != current_packer->get_current_field_name()) {
    ostringstream strm;
    strm << "Got '" << (yyvsp[(1) - (2)].str) << "', expected '" 
         << current_packer->get_current_field_name() << "'";
    yyerror(strm.str());
  }
}
    break;

  case 124:

/* Line 1464 of yacc.c  */
#line 978 "dcParser.yxx"
    {
}
    break;

  case 125:

/* Line 1464 of yacc.c  */
#line 984 "dcParser.yxx"
    {
  current_packer->pack_int64((yyvsp[(1) - (1)].u.int64));
}
    break;

  case 126:

/* Line 1464 of yacc.c  */
#line 988 "dcParser.yxx"
    {
  current_packer->pack_uint64((yyvsp[(1) - (1)].u.uint64));
}
    break;

  case 127:

/* Line 1464 of yacc.c  */
#line 992 "dcParser.yxx"
    {
  current_packer->pack_double((yyvsp[(1) - (1)].u.real));
}
    break;

  case 128:

/* Line 1464 of yacc.c  */
#line 996 "dcParser.yxx"
    {
  current_packer->pack_string((yyvsp[(1) - (1)].str));
}
    break;

  case 129:

/* Line 1464 of yacc.c  */
#line 1000 "dcParser.yxx"
    {
  current_packer->pack_literal_value((yyvsp[(1) - (1)].str));
}
    break;

  case 130:

/* Line 1464 of yacc.c  */
#line 1004 "dcParser.yxx"
    {
  current_packer->push();
}
    break;

  case 131:

/* Line 1464 of yacc.c  */
#line 1008 "dcParser.yxx"
    {
  current_packer->pop();
}
    break;

  case 132:

/* Line 1464 of yacc.c  */
#line 1012 "dcParser.yxx"
    {
  current_packer->push();
}
    break;

  case 133:

/* Line 1464 of yacc.c  */
#line 1016 "dcParser.yxx"
    {
  current_packer->pop();
}
    break;

  case 134:

/* Line 1464 of yacc.c  */
#line 1020 "dcParser.yxx"
    {
  current_packer->push();
}
    break;

  case 135:

/* Line 1464 of yacc.c  */
#line 1024 "dcParser.yxx"
    {
  current_packer->pop();
}
    break;

  case 136:

/* Line 1464 of yacc.c  */
#line 1028 "dcParser.yxx"
    {
  for (unsigned int i = 0; i < (yyvsp[(3) - (3)].u.s_uint); i++) {
    current_packer->pack_int64((yyvsp[(1) - (3)].u.int64));
  }
}
    break;

  case 137:

/* Line 1464 of yacc.c  */
#line 1034 "dcParser.yxx"
    {
  for (unsigned int i = 0; i < (yyvsp[(3) - (3)].u.s_uint); i++) {
    current_packer->pack_uint64((yyvsp[(1) - (3)].u.uint64));
  }
}
    break;

  case 138:

/* Line 1464 of yacc.c  */
#line 1040 "dcParser.yxx"
    {
  for (unsigned int i = 0; i < (yyvsp[(3) - (3)].u.s_uint); i++) {
    current_packer->pack_double((yyvsp[(1) - (3)].u.real));
  }
}
    break;

  case 139:

/* Line 1464 of yacc.c  */
#line 1046 "dcParser.yxx"
    {
  for (unsigned int i = 0; i < (yyvsp[(3) - (3)].u.s_uint); i++) {
    current_packer->pack_literal_value((yyvsp[(1) - (3)].str));
  }
}
    break;

  case 146:

/* Line 1464 of yacc.c  */
#line 1070 "dcParser.yxx"
    {
  (yyval.u.subatomic) = ST_int8;
}
    break;

  case 147:

/* Line 1464 of yacc.c  */
#line 1074 "dcParser.yxx"
    {
  (yyval.u.subatomic) = ST_int16;
}
    break;

  case 148:

/* Line 1464 of yacc.c  */
#line 1078 "dcParser.yxx"
    {
  (yyval.u.subatomic) = ST_int32;
}
    break;

  case 149:

/* Line 1464 of yacc.c  */
#line 1082 "dcParser.yxx"
    {
  (yyval.u.subatomic) = ST_int64;
}
    break;

  case 150:

/* Line 1464 of yacc.c  */
#line 1086 "dcParser.yxx"
    {
  (yyval.u.subatomic) = ST_uint8;
}
    break;

  case 151:

/* Line 1464 of yacc.c  */
#line 1090 "dcParser.yxx"
    {
  (yyval.u.subatomic) = ST_uint16;
}
    break;

  case 152:

/* Line 1464 of yacc.c  */
#line 1094 "dcParser.yxx"
    {
  (yyval.u.subatomic) = ST_uint32;
}
    break;

  case 153:

/* Line 1464 of yacc.c  */
#line 1098 "dcParser.yxx"
    {
  (yyval.u.subatomic) = ST_uint64;
}
    break;

  case 154:

/* Line 1464 of yacc.c  */
#line 1102 "dcParser.yxx"
    {
  (yyval.u.subatomic) = ST_float64;
}
    break;

  case 155:

/* Line 1464 of yacc.c  */
#line 1106 "dcParser.yxx"
    {
  (yyval.u.subatomic) = ST_string;
}
    break;

  case 156:

/* Line 1464 of yacc.c  */
#line 1110 "dcParser.yxx"
    {
  (yyval.u.subatomic) = ST_blob;
}
    break;

  case 157:

/* Line 1464 of yacc.c  */
#line 1114 "dcParser.yxx"
    {
  (yyval.u.subatomic) = ST_blob32;
}
    break;

  case 158:

/* Line 1464 of yacc.c  */
#line 1118 "dcParser.yxx"
    {
  (yyval.u.subatomic) = ST_int8array;
}
    break;

  case 159:

/* Line 1464 of yacc.c  */
#line 1122 "dcParser.yxx"
    {
  (yyval.u.subatomic) = ST_int16array;
}
    break;

  case 160:

/* Line 1464 of yacc.c  */
#line 1126 "dcParser.yxx"
    {
  (yyval.u.subatomic) = ST_int32array;
}
    break;

  case 161:

/* Line 1464 of yacc.c  */
#line 1130 "dcParser.yxx"
    {
  (yyval.u.subatomic) = ST_uint8array;
}
    break;

  case 162:

/* Line 1464 of yacc.c  */
#line 1134 "dcParser.yxx"
    {
  (yyval.u.subatomic) = ST_uint16array;
}
    break;

  case 163:

/* Line 1464 of yacc.c  */
#line 1138 "dcParser.yxx"
    {
  (yyval.u.subatomic) = ST_uint32array;
}
    break;

  case 164:

/* Line 1464 of yacc.c  */
#line 1142 "dcParser.yxx"
    {
  (yyval.u.subatomic) = ST_uint32uint8array;
}
    break;

  case 165:

/* Line 1464 of yacc.c  */
#line 1146 "dcParser.yxx"
    {
  (yyval.u.subatomic) = ST_char;
}
    break;

  case 166:

/* Line 1464 of yacc.c  */
#line 1153 "dcParser.yxx"
    {
  current_keyword_list.clear_keywords();
}
    break;

  case 167:

/* Line 1464 of yacc.c  */
#line 1157 "dcParser.yxx"
    {
  current_keyword_list.add_keyword((yyvsp[(2) - (2)].u.keyword));
}
    break;

  case 168:

/* Line 1464 of yacc.c  */
#line 1164 "dcParser.yxx"
    {
  if (current_keyword_list.get_num_keywords() != 0) {
    yyerror("Communication keywords are not allowed here.");
  }
}
    break;

  case 169:

/* Line 1464 of yacc.c  */
#line 1173 "dcParser.yxx"
    {
  current_molecular = new DCMolecularField((yyvsp[(1) - (2)].str), current_class);
}
    break;

  case 170:

/* Line 1464 of yacc.c  */
#line 1177 "dcParser.yxx"
    {
  (yyval.u.field) = current_molecular;
}
    break;

  case 171:

/* Line 1464 of yacc.c  */
#line 1184 "dcParser.yxx"
    {
  DCField *field = current_class->get_field_by_name((yyvsp[(1) - (1)].str));
  (yyval.u.atomic) = (DCAtomicField *)NULL;
  if (field == (DCField *)NULL) {
    // Maybe the field is unknown because the class is partially
    // bogus.  In that case, allow it for now; create a bogus field as
    // a placeholder.
    if (current_class->inherits_from_bogus_class()) {
      (yyval.u.atomic) = new DCAtomicField((yyvsp[(1) - (1)].str), current_class, true);
      current_class->add_field((yyval.u.atomic));

    } else {
      // Nope, it's a fully-defined class, so this is a real error.
      yyerror("Unknown field: " + (yyvsp[(1) - (1)].str));
    }

  } else {
    (yyval.u.atomic) = field->as_atomic_field();
    if ((yyval.u.atomic) == (DCAtomicField *)NULL) {
      yyerror("Not an atomic field: " + (yyvsp[(1) - (1)].str));
    }
  }
}
    break;

  case 172:

/* Line 1464 of yacc.c  */
#line 1211 "dcParser.yxx"
    {
  if ((yyvsp[(1) - (1)].u.atomic) != (DCAtomicField *)NULL) {
    current_molecular->add_atomic((yyvsp[(1) - (1)].u.atomic));
  }
}
    break;

  case 173:

/* Line 1464 of yacc.c  */
#line 1217 "dcParser.yxx"
    {
  if ((yyvsp[(3) - (3)].u.atomic) != (DCAtomicField *)NULL) {
    current_molecular->add_atomic((yyvsp[(3) - (3)].u.atomic));
    if (!(yyvsp[(3) - (3)].u.atomic)->is_bogus_field() && !current_molecular->compare_keywords(*(yyvsp[(3) - (3)].u.atomic))) {
      yyerror("Mismatched keywords in molecule between " + 
              current_molecular->get_atomic(0)->get_name() + " and " +
              (yyvsp[(3) - (3)].u.atomic)->get_name());
    }
  }
}
    break;

  case 174:

/* Line 1464 of yacc.c  */
#line 1231 "dcParser.yxx"
    {
  (yyval.str) = "";
}
    break;

  case 176:

/* Line 1464 of yacc.c  */
#line 1239 "dcParser.yxx"
    {
  current_switch = new DCSwitch((yyvsp[(2) - (6)].str), (yyvsp[(4) - (6)].u.field));
}
    break;

  case 177:

/* Line 1464 of yacc.c  */
#line 1243 "dcParser.yxx"
    {
  (yyval.u.dswitch) = current_switch;
  current_switch = (DCSwitch *)(yyvsp[(7) - (9)].u.parameter);
}
    break;

  case 183:

/* Line 1464 of yacc.c  */
#line 1257 "dcParser.yxx"
    {
  if (!current_switch->is_field_valid()) {
    yyerror("case declaration required before first element");
  } else if ((yyvsp[(2) - (3)].u.field) != (DCField *)NULL) {
    if (!current_switch->add_field((yyvsp[(2) - (3)].u.field))) {
      yyerror("Duplicate field name: " + (yyvsp[(2) - (3)].u.field)->get_name());
    }
  }
}
    break;

  case 184:

/* Line 1464 of yacc.c  */
#line 1270 "dcParser.yxx"
    {
  current_packer = &default_packer;
  current_packer->clear_data();
  current_packer->begin_pack(current_switch->get_key_parameter());
}
    break;

  case 185:

/* Line 1464 of yacc.c  */
#line 1276 "dcParser.yxx"
    {
  if (!current_packer->end_pack()) {
    yyerror("Invalid value for switch parameter");
    current_switch->add_invalid_case();
  } else {
    int case_index = current_switch->add_case(current_packer->get_string());
    if (case_index == -1) {
      yyerror("Duplicate case value");
    }
  }
}
    break;

  case 186:

/* Line 1464 of yacc.c  */
#line 1291 "dcParser.yxx"
    {
  if (!current_switch->add_default()) {
    yyerror("Default case already defined");
  }
}
    break;

  case 187:

/* Line 1464 of yacc.c  */
#line 1300 "dcParser.yxx"
    {
  current_switch->add_break();
}
    break;

  case 188:

/* Line 1464 of yacc.c  */
#line 1307 "dcParser.yxx"
    {
  (yyval.u.field) = (yyvsp[(1) - (1)].u.parameter);
}
    break;

  case 189:

/* Line 1464 of yacc.c  */
#line 1311 "dcParser.yxx"
    {
  (yyval.u.field) = (yyvsp[(1) - (1)].u.parameter);
}
    break;



/* Line 1464 of yacc.c  */
#line 3380 "y.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}




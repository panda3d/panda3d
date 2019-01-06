/* A Bison parser, made by GNU Bison 3.1.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018 Free Software Foundation, Inc.

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
#define YYBISON_VERSION "3.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         dcyyparse
#define yylex           dcyylex
#define yyerror         dcyyerror
#define yydebug         dcyydebug
#define yynerrs         dcyynerrs

#define yylval          dcyylval
#define yychar          dcyychar

/* Copy the first part of user declarations.  */
#line 7 "direct/src/dcparser/dcParser.yxx" /* yacc.c:339  */

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

using std::istream;
using std::ostringstream;
using std::string;

DCFile *dc_file = nullptr;
static DCClass *current_class = nullptr;
static DCSwitch *current_switch = nullptr;
static DCAtomicField *current_atomic = nullptr;
static DCMolecularField *current_molecular = nullptr;
static DCParameter *current_parameter = nullptr;
static DCKeywordList current_keyword_list;
static DCPacker default_packer;
static DCPacker *current_packer;
static DCDoubleRange double_range;
static DCUnsignedIntRange uint_range;
static DCField *parameter_description = nullptr;

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
  dc_file = nullptr;
  current_packer = &packer;
  dc_init_lexer(in, filename);
  dc_start_parameter_value();
}

void
dc_init_parser_parameter_description(istream &in, const string &filename,
                                     DCFile *file) {
  dc_file = file;
  dc_init_lexer(in, filename);
  parameter_description = nullptr;
  dc_start_parameter_description();
}

DCField *
dc_get_parameter_description() {
  return parameter_description;
}

void
dc_cleanup_parser() {
  dc_file = nullptr;
}


#line 156 "built/tmp/dcParser.yxx.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "dcParser.yxx.h".  */
#ifndef YY_DCYY_BUILT_TMP_DCPARSER_YXX_H_INCLUDED
# define YY_DCYY_BUILT_TMP_DCPARSER_YXX_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int dcyydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    UNSIGNED_INTEGER = 258,
    SIGNED_INTEGER = 259,
    REAL = 260,
    STRING = 261,
    IDENTIFIER = 262,
    HEX_STRING = 263,
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
#define IDENTIFIER 262
#define HEX_STRING 263
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

/* Value type.  */


extern YYSTYPE dcyylval;

int dcyyparse (void);

#endif /* !YY_DCYY_BUILT_TMP_DCPARSER_YXX_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 286 "built/tmp/dcParser.yxx.c" /* yacc.c:358  */

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
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
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

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  57
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   431

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  58
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  82
/* YYNRULES -- Number of rules.  */
#define YYNRULES  190
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  282

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   297

#define YYTRANSLATE(YYX)                                                \
  ((unsigned) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
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
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   174,   174,   175,   176,   183,   184,   185,   196,   202,
     203,   204,   208,   209,   216,   217,   224,   229,   228,   236,
     237,   244,   248,   255,   273,   277,   278,   282,   293,   294,
     299,   298,   310,   333,   334,   338,   344,   358,   359,   360,
     373,   383,   384,   392,   403,   402,   414,   437,   438,   442,
     448,   457,   458,   459,   470,   477,   478,   482,   490,   489,
     507,   508,   512,   513,   517,   527,   526,   537,   541,   543,
     542,   571,   573,   572,   601,   602,   606,   607,   611,   615,
     622,   626,   630,   637,   641,   650,   666,   681,   682,   714,
     730,   749,   753,   760,   767,   776,   782,   788,   799,   803,
     810,   817,   824,   830,   836,   845,   846,   857,   862,   877,
     892,   899,   908,   912,   923,   937,   941,   945,   949,   953,
     957,   966,   971,   975,   974,   989,   993,   997,  1001,  1005,
    1010,  1009,  1018,  1017,  1026,  1025,  1033,  1039,  1045,  1051,
    1060,  1061,  1065,  1066,  1070,  1071,  1075,  1079,  1083,  1087,
    1091,  1095,  1099,  1103,  1107,  1111,  1115,  1119,  1123,  1127,
    1131,  1135,  1139,  1143,  1147,  1151,  1158,  1162,  1169,  1179,
    1178,  1189,  1216,  1222,  1236,  1240,  1245,  1244,  1257,  1258,
    1259,  1260,  1261,  1262,  1276,  1275,  1296,  1305,  1312,  1316,
    1322
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "UNSIGNED_INTEGER", "SIGNED_INTEGER",
  "REAL", "STRING", "IDENTIFIER", "HEX_STRING", "KEYWORD", "KW_DCLASS",
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
  "switch_default", "switch_break", "switch_field", "empty", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
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

#define YYPACT_NINF -127

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-127)))

#define YYTABLE_NINF -176

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      61,  -127,   104,   359,    19,   128,  -127,  -127,  -127,   -23,
    -127,   -27,    17,  -127,  -127,  -127,    44,    69,  -127,  -127,
      15,    42,    42,  -127,  -127,  -127,  -127,  -127,  -127,  -127,
    -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,
    -127,  -127,  -127,  -127,  -127,    46,    76,  -127,  -127,  -127,
       4,  -127,    27,  -127,    22,  -127,  -127,  -127,    42,    63,
      63,   392,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,
    -127,   117,  -127,   117,    24,    24,    24,   117,   117,  -127,
    -127,    96,   142,  -127,  -127,  -127,  -127,  -127,  -127,   117,
     153,   158,    53,   146,  -127,  -127,  -127,   110,     1,   119,
    -127,  -127,  -127,  -127,   141,  -127,  -127,  -127,   122,  -127,
    -127,  -127,   116,  -127,   120,  -127,   114,   111,  -127,  -127,
     124,   359,  -127,   104,   104,  -127,  -127,  -127,    67,  -127,
    -127,  -127,    33,  -127,  -127,  -127,   -12,     7,  -127,  -127,
    -127,    20,   392,   125,   162,  -127,    63,  -127,  -127,  -127,
    -127,   104,  -127,  -127,  -127,   165,   129,  -127,  -127,  -127,
    -127,  -127,   130,  -127,  -127,   153,  -127,   153,  -127,    53,
    -127,  -127,    53,  -127,   117,   158,    53,   131,   133,  -127,
    -127,  -127,   169,   136,  -127,  -127,    11,   110,  -127,  -127,
    -127,   134,  -127,   137,    36,  -127,    12,  -127,  -127,  -127,
      -4,  -127,   392,  -127,  -127,   139,  -127,  -127,   110,  -127,
     140,   165,   291,  -127,  -127,   153,  -127,    53,  -127,  -127,
    -127,   169,   325,  -127,    63,  -127,    31,  -127,  -127,   145,
    -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,
     151,  -127,  -127,  -127,  -127,   110,  -127,  -127,  -127,  -127,
    -127,  -127,   257,  -127,  -127,   142,   142,   142,  -127,   182,
    -127,   147,  -127,  -127,  -127,  -127,  -127,  -127,  -127,   155,
     156,  -127,  -127,   149,   104,  -127,  -127,  -127,   182,   150,
    -127,  -127
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,   190,     0,   190,     0,     2,     5,   116,   115,   127,
     128,     0,   129,   130,   134,   132,   125,   126,     3,   122,
      88,   190,   190,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,    89,   190,    68,    71,   190,   190,     4,
      87,   105,    67,    83,     0,    90,   174,     1,   190,     0,
       0,     0,   190,     6,     9,    10,    11,     7,    28,    29,
       8,     0,   123,     0,   190,   190,   190,     0,     0,   175,
      44,     0,   168,    80,   166,    69,    72,    82,    81,     0,
     190,     0,   190,     0,    58,    30,    12,    14,     0,    16,
      88,    76,    77,    23,    24,    25,   113,   138,     0,   139,
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

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -127,  -127,  -127,  -126,   143,  -127,  -127,  -127,  -127,  -127,
    -127,  -127,  -127,  -127,  -127,   -20,  -127,  -127,  -127,  -127,
     197,  -127,    -6,  -127,  -127,  -127,  -127,  -116,  -127,  -127,
    -127,     5,    85,  -127,    87,    -1,  -127,     0,  -127,  -127,
     152,  -127,  -127,  -127,  -127,  -127,    34,  -127,  -127,  -125,
     -56,    18,   -81,   -78,   -90,  -115,    -2,  -127,   108,  -127,
    -127,  -127,    60,   103,  -127,  -127,  -110,   -40,    -3,  -127,
     -60,  -127,    47,   215,  -127,  -127,  -127,  -127,  -127,  -127,
    -127,     3
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     4,     5,    97,    98,    64,   186,   209,   210,    65,
      66,   104,    67,    68,   143,   204,   183,   205,   222,   240,
      43,   120,   190,   156,   191,   212,   229,    44,   142,   177,
     178,   179,    45,    93,    46,   101,   123,   102,   124,   161,
     180,   162,    49,    50,    51,   128,   136,    52,   141,   137,
     138,   173,    16,    17,   131,   132,   111,   108,    19,    74,
      76,    75,   112,   113,   114,    53,    82,    83,   233,   259,
     272,   273,    54,    55,   234,   252,   267,   274,   268,   269,
     270,    84
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      18,   134,    47,    48,     6,   158,    56,    87,    88,   129,
     129,   171,   130,   130,   145,   107,   171,   109,    96,    57,
     187,   118,   119,    71,    56,    56,    72,     7,     8,     9,
      10,    11,    12,   125,   -65,   169,     7,     8,   126,     7,
       8,   126,   168,   169,   196,   170,   146,   197,    89,    79,
     194,   129,   195,   219,   130,    90,   106,   207,    91,   135,
     208,    56,   172,    73,   174,   105,  -175,   217,    80,    81,
      96,   110,    13,    94,   175,    14,   176,   115,   115,   115,
      15,   246,  -175,    92,   129,   199,   129,   130,   167,   130,
      77,   215,   236,   133,   129,   139,   230,   130,   245,    85,
     235,     1,     2,     3,   216,    95,   241,     7,     8,     9,
      10,    11,    12,   129,   165,    78,   130,   115,   198,   166,
     106,   163,   164,   157,    56,     7,     8,     9,    10,    86,
      12,   255,   256,   257,   129,   116,   117,   130,    58,    21,
      59,    60,    61,    62,    22,   181,   184,   121,   147,   188,
     148,   122,    13,   140,   144,    14,     7,     8,   126,   127,
      15,     7,     8,   126,   146,   150,   153,   151,   154,   185,
      13,    63,   189,    14,   155,   182,   203,   192,    15,   139,
     202,   211,   193,   201,   206,   214,   221,   224,   247,   271,
     248,   249,   250,   251,   254,   213,   278,   275,   276,   277,
     281,   237,    69,    99,   258,   225,   159,   220,   160,   223,
     200,   231,   232,   103,   218,    56,   149,   152,   280,   244,
      70,   242,   243,     0,     0,    56,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   253,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   265,   266,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   100,     0,     0,     0,    21,     0,
       0,     0,   279,    22,   260,   261,   262,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,     0,   226,     0,
     263,     0,    21,     0,     0,     0,   264,    22,     0,     0,
       0,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,     0,   226,     0,   227,     0,    21,     0,     0,     0,
     228,    22,     0,     0,     0,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,     0,    20,     0,   238,     0,
      21,     0,     0,     0,   239,    22,     0,     0,     0,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,   100,
       0,     0,     0,    21,     0,     0,     0,     0,    22,     0,
       0,     0,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42
};

static const yytype_int16 yycheck[] =
{
       2,    91,     3,     3,     1,   121,     3,    47,    48,    90,
      91,     4,    90,    91,    13,    71,     4,    73,     7,     0,
     146,    77,    78,    46,    21,    22,    53,     3,     4,     5,
       6,     7,     8,    89,     7,    47,     3,     4,     5,     3,
       4,     5,   132,    47,   169,    57,    45,   172,    44,     7,
     165,   132,   167,    57,   132,    51,     3,    46,    54,     6,
     186,    58,    55,    46,    44,    62,    51,    55,    21,    22,
       7,    47,    48,    51,    54,    51,    56,    74,    75,    76,
      56,    50,    51,    56,   165,   175,   167,   165,    55,   167,
      46,    55,   217,    90,   175,    92,   212,   175,   224,    53,
     215,    40,    41,    42,   194,    58,   222,     3,     4,     5,
       6,     7,     8,   194,    47,    46,   194,   114,   174,    52,
       3,   123,   124,   120,   121,     3,     4,     5,     6,    53,
       8,   241,   242,   243,   215,    75,    76,   215,    10,    11,
      12,    13,    14,    15,    16,   142,   143,    51,     7,   151,
       9,     9,    48,     7,    44,    51,     3,     4,     5,     6,
      56,     3,     4,     5,    45,    49,    52,    47,    57,     7,
      48,    43,     7,    51,    50,    50,     7,    48,    56,   176,
      47,    47,    52,    52,    48,    48,    47,    47,    43,     7,
     230,   231,   232,   233,    43,   192,    47,    50,    43,    43,
      50,   221,     5,    60,   244,   211,   121,   202,   121,   206,
     176,   212,   212,    61,   196,   212,   108,   114,   278,   222,
       5,   222,   222,    -1,    -1,   222,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   234,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   252,   252,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     7,    -1,    -1,    -1,    11,    -1,
      -1,    -1,   274,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    -1,     7,    -1,
      43,    -1,    11,    -1,    -1,    -1,    49,    16,    -1,    -1,
      -1,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    -1,     7,    -1,    43,    -1,    11,    -1,    -1,    -1,
      49,    16,    -1,    -1,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    -1,     7,    -1,    43,    -1,
      11,    -1,    -1,    -1,    49,    16,    -1,    -1,    -1,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,     7,
      -1,    -1,    -1,    11,    -1,    -1,    -1,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    40,    41,    42,    59,    60,   139,     3,     4,     5,
       6,     7,     8,    48,    51,    56,   110,   111,   114,   116,
       7,    11,    16,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    78,    85,    90,    92,    93,    95,   100,
     101,   102,   105,   123,   130,   131,   139,     0,    10,    12,
      13,    14,    15,    43,    63,    67,    68,    70,    71,    78,
     131,    46,    53,    46,   117,   119,   118,    46,    46,     7,
     130,   130,   124,   125,   139,    53,    53,   125,   125,    44,
      51,    54,    56,    91,    51,   130,     7,    61,    62,    62,
       7,    93,    95,    98,    69,   139,     3,   108,   115,   108,
      47,   114,   120,   121,   122,   139,   120,   120,   108,   108,
      79,    51,     9,    94,    96,   108,     5,     6,   103,   110,
     111,   112,   113,   139,   112,     6,   104,   107,   108,   139,
       7,   106,    86,    72,    44,    13,    45,     7,     9,   116,
      49,    47,   121,    52,    57,    50,    81,   139,    85,    90,
      92,    97,    99,   114,   114,    47,    52,    55,   112,    47,
      57,     4,    55,   109,    44,    54,    56,    87,    88,    89,
      98,   139,    50,    74,   139,     7,    64,    61,   114,     7,
      80,    82,    48,    52,   113,   113,   107,   107,   108,   112,
     104,    52,    47,     7,    73,    75,    48,    46,    61,    65,
      66,    47,    83,   139,    48,    55,   112,    55,   109,    57,
      89,    47,    76,   139,    47,    80,     7,    43,    49,    84,
      85,    93,    95,   126,   132,   113,   107,    73,    43,    49,
      77,    85,    93,    95,   126,    61,    50,    43,   125,   125,
     125,   125,   133,   139,    43,   124,   124,   124,   125,   127,
      17,    18,    19,    43,    49,    93,    95,   134,   136,   137,
     138,     7,   128,   129,   135,    50,    43,    43,    47,   114,
     128,    50
};

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

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
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


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

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
#ifndef YYINITDEPTH
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
static YYSIZE_T
yystrlen (const char *yystr)
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
static char *
yystpcpy (char *yydest, const char *yysrc)
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

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
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
  int yytoken = 0;
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

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
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
                  (unsigned long) yystacksize));

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
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
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
      if (yytable_value_is_error (yyn))
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
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

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
     '$$ = $1'.

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
#line 177 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  parameter_description = (yyvsp[0].u.field);
}
#line 1634 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 7:
#line 186 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if (!dc_file->add_class((yyvsp[0].u.dclass))) {
    DCClass *old_class = dc_file->get_class_by_name((yyvsp[0].u.dclass)->get_name());
    if (old_class != nullptr && old_class->is_bogus_class()) {
      yyerror("Base class defined after its first reference: " + (yyvsp[0].u.dclass)->get_name());
    } else {
      yyerror("Duplicate class name: " + (yyvsp[0].u.dclass)->get_name());
    }
  }
}
#line 1649 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 8:
#line 197 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if (!dc_file->add_switch((yyvsp[0].u.dswitch))) {
    yyerror("Duplicate class name: " + (yyvsp[0].u.dswitch)->get_name());
  }
}
#line 1659 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 13:
#line 210 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.str) = (yyvsp[-2].str) + string("/") + (yyvsp[0].str);
}
#line 1667 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 15:
#line 218 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.str) = (yyvsp[-2].str) + string(".") + (yyvsp[0].str);
}
#line 1675 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 16:
#line 225 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  dc_file->add_import_module((yyvsp[0].str));
}
#line 1683 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 17:
#line 229 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  dc_file->add_import_module((yyvsp[-1].str));
}
#line 1691 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 20:
#line 238 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  dc_file->add_import_symbol("*");
}
#line 1699 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 21:
#line 245 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  dc_file->add_import_symbol((yyvsp[0].str));
}
#line 1707 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 22:
#line 249 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  dc_file->add_import_symbol((yyvsp[0].str));
}
#line 1715 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 23:
#line 256 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if ((yyvsp[0].u.parameter) != nullptr) {
    DCTypedef *dtypedef = new DCTypedef((yyvsp[0].u.parameter));

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
#line 1734 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 26:
#line 279 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  dc_file->add_keyword((yyvsp[0].str));
}
#line 1742 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 27:
#line 283 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  // This keyword has already been defined.  But since we are now
  // explicitly defining it, clear its bitmask, so that we will have a
  // new hash code--doing this will allow us to phase out the
  // historical hash code support later.
  ((DCKeyword *)(yyvsp[0].u.keyword))->clear_historical_flag();
}
#line 1754 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 30:
#line 299 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  current_class = new DCClass(dc_file, (yyvsp[0].str), false, false);
}
#line 1762 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 31:
#line 303 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.dclass) = current_class;
  current_class = (yyvsp[-4].u.dclass);
}
#line 1771 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 32:
#line 311 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if (dc_file == nullptr) {
    yyerror("No DCFile available, so no class names are predefined.");
    (yyval.u.dclass) = nullptr;

  } else {
    DCClass *dclass = dc_file->get_class_by_name((yyvsp[0].str));
    if (dclass == nullptr) {
      // Create a bogus class as a forward reference.
      dclass = new DCClass(dc_file, (yyvsp[0].str), false, true);
      dc_file->add_class(dclass);
    }
    if (dclass->is_struct()) {
      yyerror("struct name not allowed");
    }

    (yyval.u.dclass) = dclass;
  }
}
#line 1795 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 35:
#line 339 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if ((yyvsp[0].u.dclass) != nullptr) {
    current_class->add_parent((yyvsp[0].u.dclass));
  }
}
#line 1805 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 36:
#line 345 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if (!dc_multiple_inheritance) {
    yyerror("Multiple inheritance is not supported without \"dc-multiple-inheritance 1\" in your Config.prc file.");

  } else {
    if ((yyvsp[0].u.dclass) != nullptr) {
      current_class->add_parent((yyvsp[0].u.dclass));
    }
  }
}
#line 1820 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 39:
#line 361 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if ((yyvsp[-1].u.field) == nullptr) {
    // Pass this error up.
  } else if (!current_class->add_field((yyvsp[-1].u.field))) {
    yyerror("Duplicate field name: " + (yyvsp[-1].u.field)->get_name());
  } else if ((yyvsp[-1].u.field)->get_number() < 0) {
    yyerror("A non-network field cannot be stored on a dclass");
  }
}
#line 1834 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 40:
#line 374 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if ((yyvsp[-1].u.field) != nullptr) {
    if ((yyvsp[-1].u.field)->get_name().empty()) {
      yyerror("Field name required.");
    }
    (yyvsp[-1].u.field)->copy_keywords(current_keyword_list);
  }
  (yyval.u.field) = (yyvsp[-1].u.field);
}
#line 1848 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 42:
#line 385 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  yyerror("Unnamed parameters are not allowed on a dclass");
  if ((yyvsp[-1].u.parameter) != nullptr) {
    (yyvsp[-1].u.parameter)->copy_keywords(current_keyword_list);
  }
  (yyval.u.field) = (yyvsp[-1].u.parameter);
}
#line 1860 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 43:
#line 393 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if ((yyvsp[-1].u.parameter) != nullptr) {
    (yyvsp[-1].u.parameter)->copy_keywords(current_keyword_list);
  }
  (yyval.u.field) = (yyvsp[-1].u.parameter);
}
#line 1871 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 44:
#line 403 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  current_class = new DCClass(dc_file, (yyvsp[0].str), true, false);
}
#line 1879 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 45:
#line 407 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.dclass) = current_class;
  current_class = (yyvsp[-4].u.dclass);
}
#line 1888 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 46:
#line 415 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if (dc_file == nullptr) {
    yyerror("No DCFile available, so no struct names are predefined.");
    (yyval.u.dclass) = nullptr;

  } else {
    DCClass *dstruct = dc_file->get_class_by_name((yyvsp[0].str));
    if (dstruct == nullptr) {
      // Create a bogus class as a forward reference.
      dstruct = new DCClass(dc_file, (yyvsp[0].str), false, true);
      dc_file->add_class(dstruct);
    }
    if (!dstruct->is_struct()) {
      yyerror("struct name required");
    }

    (yyval.u.dclass) = dstruct;
  }
}
#line 1912 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 49:
#line 443 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if ((yyvsp[0].u.dclass) != nullptr) {
    current_class->add_parent((yyvsp[0].u.dclass));
  }
}
#line 1922 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 50:
#line 449 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if ((yyvsp[0].u.dclass) != nullptr) {
    current_class->add_parent((yyvsp[0].u.dclass));
  }
}
#line 1932 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 53:
#line 460 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if ((yyvsp[-1].u.field) == nullptr) {
    // Pass this error up.
  } else if (!current_class->add_field((yyvsp[-1].u.field))) {
    yyerror("Duplicate field name: " + (yyvsp[-1].u.field)->get_name());
  }
}
#line 1944 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 54:
#line 471 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if ((yyvsp[-1].u.field)->get_name().empty()) {
    yyerror("Field name required.");
  }
  (yyval.u.field) = (yyvsp[-1].u.field);
}
#line 1955 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 56:
#line 479 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.field) = (yyvsp[-1].u.parameter);
}
#line 1963 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 57:
#line 483 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.field) = (yyvsp[-1].u.parameter);
}
#line 1971 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 58:
#line 490 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if (current_class == nullptr) {
    yyerror("Cannot define a method outside of a struct or class.");
    DCClass *temp_class = new DCClass(dc_file, "temp", false, false);  // memory leak.
    current_atomic = new DCAtomicField((yyvsp[-1].str), temp_class, false);
  } else {
    current_atomic = new DCAtomicField((yyvsp[-1].str), current_class, false);
  }
}
#line 1985 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 59:
#line 500 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.field) = current_atomic;
  current_atomic = (yyvsp[-2].u.atomic);
}
#line 1994 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 64:
#line 518 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if ((yyvsp[0].u.parameter) != nullptr) {
    current_atomic->add_element((yyvsp[0].u.parameter));
  }
}
#line 2004 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 65:
#line 527 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  current_parameter = (yyvsp[0].u.parameter);
}
#line 2012 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 66:
#line 531 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.parameter) = (yyvsp[0].u.parameter);
}
#line 2020 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 69:
#line 543 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  current_packer = &default_packer;
  current_packer->clear_data();
  if ((yyvsp[-1].u.parameter) != nullptr) {
    current_packer->begin_pack((yyvsp[-1].u.parameter));
  }
}
#line 2032 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 70:
#line 551 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  bool is_valid = false;
  if ((yyvsp[-3].u.parameter) != nullptr) {
    is_valid = (yyvsp[-3].u.parameter)->is_valid();
  }
  if (current_packer->end_pack()) {
    (yyvsp[-3].u.parameter)->set_default_value(current_packer->get_bytes());

  } else {
    if (is_valid) {
      yyerror("Invalid default value for type");
    }
    // If the current parameter isn't valid, we don't mind a pack
    // error (there's no way for us to validate the syntax).  So we'll
    // just ignore the default value in this case.
  }
}
#line 2054 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 72:
#line 573 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  current_packer = &default_packer;
  current_packer->clear_data();
  if ((yyvsp[-1].u.parameter) != nullptr) {
    current_packer->begin_pack((yyvsp[-1].u.parameter));
  }
}
#line 2066 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 73:
#line 581 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  bool is_valid = false;
  if ((yyvsp[-3].u.parameter) != nullptr) {
    is_valid = (yyvsp[-3].u.parameter)->is_valid();
  }
  if (current_packer->end_pack()) {
    (yyvsp[-3].u.parameter)->set_default_value(current_packer->get_bytes());

  } else {
    if (is_valid) {
      yyerror("Invalid default value for type");
    }
    // If the current parameter isn't valid, we don't mind a pack
    // error (there's no way for us to validate the syntax).  So we'll
    // just ignore the default value in this case.
  }
}
#line 2088 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 78:
#line 612 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.field) = (yyvsp[0].u.parameter);
}
#line 2096 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 79:
#line 616 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.field) = (yyvsp[0].u.field);
}
#line 2104 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 80:
#line 623 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.field) = (yyvsp[-1].u.field);
}
#line 2112 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 81:
#line 627 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.field) = (yyvsp[-1].u.parameter);
}
#line 2120 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 82:
#line 631 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.field) = (yyvsp[-1].u.parameter);
}
#line 2128 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 83:
#line 638 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.parameter) = new DCSimpleParameter((yyvsp[0].u.subatomic));
}
#line 2136 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 84:
#line 642 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  DCSimpleParameter *simple_param = (yyvsp[-3].u.parameter)->as_simple_parameter();
  nassertr(simple_param != nullptr, 0);
  if (!simple_param->set_range(double_range)) {
    yyerror("Inappropriate range for type");
  }
  (yyval.u.parameter) = simple_param;
}
#line 2149 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 85:
#line 651 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  DCSimpleParameter *simple_param = (yyvsp[-2].u.parameter)->as_simple_parameter();
  nassertr(simple_param != nullptr, 0);
  if (!simple_param->is_numeric_type()) {
    yyerror("A divisor is only valid on a numeric type.");

  } else if (!simple_param->set_divisor((yyvsp[0].u.s_uint))) {
    yyerror("Invalid divisor.");

  } else if (simple_param->has_modulus() && !simple_param->set_modulus(simple_param->get_modulus())) {
    // Changing the divisor may change the valid range for the modulus.
    yyerror("Invalid modulus.");
  }
  (yyval.u.parameter) = simple_param;
}
#line 2169 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 86:
#line 667 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  DCSimpleParameter *simple_param = (yyvsp[-2].u.parameter)->as_simple_parameter();
  nassertr(simple_param != nullptr, 0);
  if (!simple_param->is_numeric_type()) {
    yyerror("A divisor is only valid on a numeric type.");

  } else if (!simple_param->set_modulus((yyvsp[0].u.real))) {
    yyerror("Invalid modulus.");
  }
  (yyval.u.parameter) = simple_param;
}
#line 2185 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 88:
#line 683 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if (dc_file == nullptr) {
    yyerror("Invalid type.");
    (yyval.u.parameter) = nullptr;

  } else {
    DCTypedef *dtypedef = dc_file->get_typedef_by_name((yyvsp[0].str));
    if (dtypedef == nullptr) {
      // Maybe it's a class name.
      DCClass *dclass = dc_file->get_class_by_name((yyvsp[0].str));
      if (dclass != nullptr) {
        // Create an implicit typedef for this.
        dtypedef = new DCTypedef(new DCClassParameter(dclass), true);
      } else {
        // Maybe it's a switch name.
        DCSwitch *dswitch = dc_file->get_switch_by_name((yyvsp[0].str));
        if (dswitch != nullptr) {
          // This also gets an implicit typedef.
          dtypedef = new DCTypedef(new DCSwitchParameter(dswitch), true);
        } else {
          // It's an undefined typedef.  Create a bogus forward reference.
          dtypedef = new DCTypedef((yyvsp[0].str));
        }
      }

      dc_file->add_typedef(dtypedef);
    }

    (yyval.u.parameter) = dtypedef->make_new_parameter();
  }
}
#line 2221 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 89:
#line 715 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  // This is an inline struct definition.
  if ((yyvsp[0].u.dclass) == nullptr) {
    (yyval.u.parameter) = nullptr;
  } else {
    if (dc_file != nullptr) {
      dc_file->add_thing_to_delete((yyvsp[0].u.dclass));
    } else {
      // This is a memory leak--this happens when we put an anonymous
      // struct reference within the string passed to
      // DCPackerInterface::check_match().  Maybe it doesn't really matter.
    }
    (yyval.u.parameter) = new DCClassParameter((yyvsp[0].u.dclass));
  }
}
#line 2241 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 90:
#line 731 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  // This is an inline switch definition.
  if ((yyvsp[0].u.dswitch) == nullptr) {
    (yyval.u.parameter) = nullptr;
  } else {
    if (dc_file != nullptr) {
      dc_file->add_thing_to_delete((yyvsp[0].u.dswitch));
    } else {
      // This is a memory leak--this happens when we put an anonymous
      // switch reference within the string passed to
      // DCPackerInterface::check_match().  Maybe it doesn't really matter.
    }
    (yyval.u.parameter) = new DCSwitchParameter((yyvsp[0].u.dswitch));
  }
}
#line 2261 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 91:
#line 750 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  double_range.clear();
}
#line 2269 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 92:
#line 754 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  double_range.clear();
  if (!double_range.add_range((yyvsp[0].u.real), (yyvsp[0].u.real))) {
    yyerror("Overlapping range");
  }
}
#line 2280 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 93:
#line 761 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  double_range.clear();
  if (!double_range.add_range((yyvsp[-2].u.real), (yyvsp[0].u.real))) {
    yyerror("Overlapping range");
  }
}
#line 2291 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 94:
#line 768 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  double_range.clear();
  if ((yyvsp[0].u.real) >= 0) {
    yyerror("Syntax error");
  } else if (!double_range.add_range((yyvsp[-1].u.real), -(yyvsp[0].u.real))) {
    yyerror("Overlapping range");
  }
}
#line 2304 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 95:
#line 777 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if (!double_range.add_range((yyvsp[0].u.real), (yyvsp[0].u.real))) {
    yyerror("Overlapping range");
  }
}
#line 2314 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 96:
#line 783 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if (!double_range.add_range((yyvsp[-2].u.real), (yyvsp[0].u.real))) {
    yyerror("Overlapping range");
  }
}
#line 2324 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 97:
#line 789 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if ((yyvsp[0].u.real) >= 0) {
    yyerror("Syntax error");
  } else if (!double_range.add_range((yyvsp[-1].u.real), -(yyvsp[0].u.real))) {
    yyerror("Overlapping range");
  }
}
#line 2336 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 98:
#line 800 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  uint_range.clear();
}
#line 2344 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 99:
#line 804 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  uint_range.clear();
  if (!uint_range.add_range((yyvsp[0].u.s_uint), (yyvsp[0].u.s_uint))) {
    yyerror("Overlapping range");
  }
}
#line 2355 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 100:
#line 811 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  uint_range.clear();
  if (!uint_range.add_range((yyvsp[-2].u.s_uint), (yyvsp[0].u.s_uint))) {
    yyerror("Overlapping range");
  }
}
#line 2366 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 101:
#line 818 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  uint_range.clear();
  if (!uint_range.add_range((yyvsp[-1].u.s_uint), (yyvsp[0].u.s_uint))) {
    yyerror("Overlapping range");
  }
}
#line 2377 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 102:
#line 825 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if (!uint_range.add_range((yyvsp[0].u.s_uint), (yyvsp[0].u.s_uint))) {
    yyerror("Overlapping range");
  }
}
#line 2387 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 103:
#line 831 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if (!uint_range.add_range((yyvsp[-2].u.s_uint), (yyvsp[0].u.s_uint))) {
    yyerror("Overlapping range");
  }
}
#line 2397 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 104:
#line 837 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if (!uint_range.add_range((yyvsp[-1].u.s_uint), (yyvsp[0].u.s_uint))) {
    yyerror("Overlapping range");
  }
}
#line 2407 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 106:
#line 847 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if ((yyvsp[-3].u.parameter) == nullptr) {
    (yyval.u.parameter) = nullptr;
  } else {
    (yyval.u.parameter) = (yyvsp[-3].u.parameter)->append_array_specification(uint_range);
  }
}
#line 2419 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 107:
#line 858 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  current_parameter->set_name((yyvsp[0].str));
  (yyval.u.parameter) = current_parameter;
}
#line 2428 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 108:
#line 863 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  DCSimpleParameter *simple_param = (yyvsp[-2].u.parameter)->as_simple_parameter();
  if (simple_param == nullptr || simple_param->get_typedef() != nullptr) {
    yyerror("A divisor is only allowed on a primitive type.");

  } else if (!simple_param->is_numeric_type()) {
      yyerror("A divisor is only valid on a numeric type.");

  } else {
    if (!simple_param->set_divisor((yyvsp[0].u.s_uint))) {
      yyerror("Invalid divisor.");
    }
  }
}
#line 2447 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 109:
#line 878 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  DCSimpleParameter *simple_param = (yyvsp[-2].u.parameter)->as_simple_parameter();
  if (simple_param == nullptr || simple_param->get_typedef() != nullptr) {
    yyerror("A modulus is only allowed on a primitive type.");

  } else if (!simple_param->is_numeric_type()) {
      yyerror("A modulus is only valid on a numeric type.");

  } else {
    if (!simple_param->set_modulus((yyvsp[0].u.real))) {
      yyerror("Invalid modulus.");
    }
  }
}
#line 2466 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 110:
#line 893 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.parameter) = (yyvsp[-3].u.parameter)->append_array_specification(uint_range);
}
#line 2474 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 111:
#line 900 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if ((yyvsp[0].str).length() != 1) {
    yyerror("Single character required.");
    (yyval.u.s_uint) = 0;
  } else {
    (yyval.u.s_uint) = (unsigned char)(yyvsp[0].str)[0];
  }
}
#line 2487 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 113:
#line 913 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.s_uint) = (unsigned int)(yyvsp[0].u.uint64);
  if ((yyval.u.s_uint) != (yyvsp[0].u.uint64)) {
    yyerror("Number out of range.");
    (yyval.u.s_uint) = 1;
  }
}
#line 2499 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 114:
#line 924 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.s_uint) = (unsigned int)-(yyvsp[0].u.int64);
  if ((yyvsp[0].u.int64) >= 0) {
    yyerror("Syntax error.");

  } else if ((yyval.u.s_uint) != -(yyvsp[0].u.int64)) {
    yyerror("Number out of range.");
    (yyval.u.s_uint) = 1;
  }
}
#line 2514 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 117:
#line 946 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.real) = (double)(yyvsp[0].u.uint64);
}
#line 2522 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 118:
#line 950 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.real) = (double)(yyvsp[0].u.int64);
}
#line 2530 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 120:
#line 958 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if ((yyvsp[0].str).length() != 1) {
    yyerror("Single character required.");
    (yyval.u.real) = 0;
  } else {
    (yyval.u.real) = (double)(unsigned char)(yyvsp[0].str)[0];
  }
}
#line 2543 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 122:
#line 972 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
}
#line 2550 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 123:
#line 975 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if ((yyvsp[-1].str) != current_packer->get_current_field_name()) {
    ostringstream strm;
    strm << "Got '" << (yyvsp[-1].str) << "', expected '"
         << current_packer->get_current_field_name() << "'";
    yyerror(strm.str());
  }
}
#line 2563 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 124:
#line 984 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
}
#line 2570 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 125:
#line 990 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  current_packer->pack_int64((yyvsp[0].u.int64));
}
#line 2578 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 126:
#line 994 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  current_packer->pack_uint64((yyvsp[0].u.uint64));
}
#line 2586 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 127:
#line 998 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  current_packer->pack_double((yyvsp[0].u.real));
}
#line 2594 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 128:
#line 1002 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  current_packer->pack_string((yyvsp[0].str));
}
#line 2602 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 129:
#line 1006 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  current_packer->pack_literal_value((yyvsp[0].bytes));
}
#line 2610 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 130:
#line 1010 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  current_packer->push();
}
#line 2618 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 131:
#line 1014 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  current_packer->pop();
}
#line 2626 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 132:
#line 1018 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  current_packer->push();
}
#line 2634 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 133:
#line 1022 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  current_packer->pop();
}
#line 2642 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 134:
#line 1026 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  current_packer->push();
}
#line 2650 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 135:
#line 1030 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  current_packer->pop();
}
#line 2658 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 136:
#line 1034 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  for (unsigned int i = 0; i < (yyvsp[0].u.s_uint); i++) {
    current_packer->pack_int64((yyvsp[-2].u.int64));
  }
}
#line 2668 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 137:
#line 1040 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  for (unsigned int i = 0; i < (yyvsp[0].u.s_uint); i++) {
    current_packer->pack_uint64((yyvsp[-2].u.uint64));
  }
}
#line 2678 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 138:
#line 1046 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  for (unsigned int i = 0; i < (yyvsp[0].u.s_uint); i++) {
    current_packer->pack_double((yyvsp[-2].u.real));
  }
}
#line 2688 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 139:
#line 1052 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  for (unsigned int i = 0; i < (yyvsp[0].u.s_uint); i++) {
    current_packer->pack_literal_value((yyvsp[-2].bytes));
  }
}
#line 2698 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 146:
#line 1076 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.subatomic) = ST_int8;
}
#line 2706 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 147:
#line 1080 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.subatomic) = ST_int16;
}
#line 2714 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 148:
#line 1084 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.subatomic) = ST_int32;
}
#line 2722 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 149:
#line 1088 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.subatomic) = ST_int64;
}
#line 2730 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 150:
#line 1092 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.subatomic) = ST_uint8;
}
#line 2738 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 151:
#line 1096 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.subatomic) = ST_uint16;
}
#line 2746 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 152:
#line 1100 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.subatomic) = ST_uint32;
}
#line 2754 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 153:
#line 1104 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.subatomic) = ST_uint64;
}
#line 2762 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 154:
#line 1108 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.subatomic) = ST_float64;
}
#line 2770 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 155:
#line 1112 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.subatomic) = ST_string;
}
#line 2778 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 156:
#line 1116 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.subatomic) = ST_blob;
}
#line 2786 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 157:
#line 1120 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.subatomic) = ST_blob32;
}
#line 2794 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 158:
#line 1124 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.subatomic) = ST_int8array;
}
#line 2802 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 159:
#line 1128 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.subatomic) = ST_int16array;
}
#line 2810 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 160:
#line 1132 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.subatomic) = ST_int32array;
}
#line 2818 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 161:
#line 1136 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.subatomic) = ST_uint8array;
}
#line 2826 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 162:
#line 1140 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.subatomic) = ST_uint16array;
}
#line 2834 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 163:
#line 1144 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.subatomic) = ST_uint32array;
}
#line 2842 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 164:
#line 1148 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.subatomic) = ST_uint32uint8array;
}
#line 2850 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 165:
#line 1152 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.subatomic) = ST_char;
}
#line 2858 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 166:
#line 1159 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  current_keyword_list.clear_keywords();
}
#line 2866 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 167:
#line 1163 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  current_keyword_list.add_keyword((yyvsp[0].u.keyword));
}
#line 2874 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 168:
#line 1170 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if (current_keyword_list.get_num_keywords() != 0) {
    yyerror("Communication keywords are not allowed here.");
  }
}
#line 2884 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 169:
#line 1179 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  current_molecular = new DCMolecularField((yyvsp[-1].str), current_class);
}
#line 2892 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 170:
#line 1183 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.field) = current_molecular;
}
#line 2900 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 171:
#line 1190 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  DCField *field = current_class->get_field_by_name((yyvsp[0].str));
  (yyval.u.atomic) = nullptr;
  if (field == nullptr) {
    // Maybe the field is unknown because the class is partially
    // bogus.  In that case, allow it for now; create a bogus field as
    // a placeholder.
    if (current_class->inherits_from_bogus_class()) {
      (yyval.u.atomic) = new DCAtomicField((yyvsp[0].str), current_class, true);
      current_class->add_field((yyval.u.atomic));

    } else {
      // Nope, it's a fully-defined class, so this is a real error.
      yyerror("Unknown field: " + (yyvsp[0].str));
    }

  } else {
    (yyval.u.atomic) = field->as_atomic_field();
    if ((yyval.u.atomic) == nullptr) {
      yyerror("Not an atomic field: " + (yyvsp[0].str));
    }
  }
}
#line 2928 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 172:
#line 1217 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if ((yyvsp[0].u.atomic) != nullptr) {
    current_molecular->add_atomic((yyvsp[0].u.atomic));
  }
}
#line 2938 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 173:
#line 1223 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if ((yyvsp[0].u.atomic) != nullptr) {
    current_molecular->add_atomic((yyvsp[0].u.atomic));
    if (!(yyvsp[0].u.atomic)->is_bogus_field() && !current_molecular->compare_keywords(*(yyvsp[0].u.atomic))) {
      yyerror("Mismatched keywords in molecule between " +
              current_molecular->get_atomic(0)->get_name() + " and " +
              (yyvsp[0].u.atomic)->get_name());
    }
  }
}
#line 2953 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 174:
#line 1237 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.str) = "";
}
#line 2961 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 176:
#line 1245 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  current_switch = new DCSwitch((yyvsp[-4].str), (yyvsp[-2].u.field));
}
#line 2969 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 177:
#line 1249 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.dswitch) = current_switch;
  current_switch = (DCSwitch *)(yyvsp[-2].u.parameter);
}
#line 2978 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 183:
#line 1263 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if (!current_switch->is_field_valid()) {
    yyerror("case declaration required before first element");
  } else if ((yyvsp[-1].u.field) != nullptr) {
    if (!current_switch->add_field((yyvsp[-1].u.field))) {
      yyerror("Duplicate field name: " + (yyvsp[-1].u.field)->get_name());
    }
  }
}
#line 2992 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 184:
#line 1276 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  current_packer = &default_packer;
  current_packer->clear_data();
  current_packer->begin_pack(current_switch->get_key_parameter());
}
#line 3002 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 185:
#line 1282 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if (!current_packer->end_pack()) {
    yyerror("Invalid value for switch parameter");
    current_switch->add_invalid_case();
  } else {
    int case_index = current_switch->add_case(current_packer->get_bytes());
    if (case_index == -1) {
      yyerror("Duplicate case value");
    }
  }
}
#line 3018 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 186:
#line 1297 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  if (!current_switch->add_default()) {
    yyerror("Default case already defined");
  }
}
#line 3028 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 187:
#line 1306 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  current_switch->add_break();
}
#line 3036 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 188:
#line 1313 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.field) = (yyvsp[0].u.parameter);
}
#line 3044 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;

  case 189:
#line 1317 "direct/src/dcparser/dcParser.yxx" /* yacc.c:1651  */
    {
  (yyval.u.field) = (yyvsp[0].u.parameter);
}
#line 3052 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
    break;


#line 3056 "built/tmp/dcParser.yxx.c" /* yacc.c:1651  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
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

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
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

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


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

#if !defined yyoverflow || YYERROR_VERBOSE
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
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
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
  return yyresult;
}

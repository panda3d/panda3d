/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         xyyparse
#define yylex           xyylex
#define yyerror         xyyerror
#define yydebug         xyydebug
#define yynerrs         xyynerrs
#define yylval          xyylval
#define yychar          xyychar

/* First part of user prologue.  */
#line 13 "pandatool/src/xfile/xParser.yxx"

#include "xLexerDefs.h"
#include "xParserDefs.h"
#include "xFile.h"
#include "xFileTemplate.h"
#include "xFileDataDef.h"
#include "xFileArrayDef.h"
#include "xFileDataNodeTemplate.h"
#include "xFileDataNodeReference.h"
#include "pointerTo.h"
#include "dcast.h"

// Because our token type contains objects of type string, which
// require correct copy construction (and not simply memcpying), we
// cannot use bison's built-in auto-stack-grow feature.  As an easy
// solution, we ensure here that we have enough yacc stack to start
// with, and that it doesn't ever try to grow.
#define YYINITDEPTH 1000
#define YYMAXDEPTH 1000

static XFile *x_file = nullptr;
static XFileNode *current_node = nullptr;
static PT(XFileDataDef) current_data_def;

////////////////////////////////////////////////////////////////////
// Defining the interface to the parser.
////////////////////////////////////////////////////////////////////

void
x_init_parser(std::istream &in, const std::string &filename, XFile &file) {
  x_file = &file;
  current_node = &file;
  x_init_lexer(in, filename);
}

void
x_cleanup_parser() {
  x_file = nullptr;
  current_node = nullptr;
}


#line 121 "built/tmp/xParser.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "xParser.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_TOKEN_NAME = 3,                 /* TOKEN_NAME  */
  YYSYMBOL_TOKEN_STRING = 4,               /* TOKEN_STRING  */
  YYSYMBOL_TOKEN_INTEGER = 5,              /* TOKEN_INTEGER  */
  YYSYMBOL_TOKEN_GUID = 6,                 /* TOKEN_GUID  */
  YYSYMBOL_TOKEN_INTEGER_LIST = 7,         /* TOKEN_INTEGER_LIST  */
  YYSYMBOL_TOKEN_REALNUM_LIST = 8,         /* TOKEN_REALNUM_LIST  */
  YYSYMBOL_TOKEN_OBRACE = 9,               /* TOKEN_OBRACE  */
  YYSYMBOL_TOKEN_CBRACE = 10,              /* TOKEN_CBRACE  */
  YYSYMBOL_TOKEN_OPAREN = 11,              /* TOKEN_OPAREN  */
  YYSYMBOL_TOKEN_CPAREN = 12,              /* TOKEN_CPAREN  */
  YYSYMBOL_TOKEN_OBRACKET = 13,            /* TOKEN_OBRACKET  */
  YYSYMBOL_TOKEN_CBRACKET = 14,            /* TOKEN_CBRACKET  */
  YYSYMBOL_TOKEN_OANGLE = 15,              /* TOKEN_OANGLE  */
  YYSYMBOL_TOKEN_CANGLE = 16,              /* TOKEN_CANGLE  */
  YYSYMBOL_TOKEN_DOT = 17,                 /* TOKEN_DOT  */
  YYSYMBOL_TOKEN_COMMA = 18,               /* TOKEN_COMMA  */
  YYSYMBOL_TOKEN_SEMICOLON = 19,           /* TOKEN_SEMICOLON  */
  YYSYMBOL_TOKEN_TEMPLATE = 20,            /* TOKEN_TEMPLATE  */
  YYSYMBOL_TOKEN_WORD = 21,                /* TOKEN_WORD  */
  YYSYMBOL_TOKEN_DWORD = 22,               /* TOKEN_DWORD  */
  YYSYMBOL_TOKEN_FLOAT = 23,               /* TOKEN_FLOAT  */
  YYSYMBOL_TOKEN_DOUBLE = 24,              /* TOKEN_DOUBLE  */
  YYSYMBOL_TOKEN_CHAR = 25,                /* TOKEN_CHAR  */
  YYSYMBOL_TOKEN_UCHAR = 26,               /* TOKEN_UCHAR  */
  YYSYMBOL_TOKEN_SWORD = 27,               /* TOKEN_SWORD  */
  YYSYMBOL_TOKEN_SDWORD = 28,              /* TOKEN_SDWORD  */
  YYSYMBOL_TOKEN_VOID = 29,                /* TOKEN_VOID  */
  YYSYMBOL_TOKEN_LPSTR = 30,               /* TOKEN_LPSTR  */
  YYSYMBOL_TOKEN_UNICODE = 31,             /* TOKEN_UNICODE  */
  YYSYMBOL_TOKEN_CSTRING = 32,             /* TOKEN_CSTRING  */
  YYSYMBOL_TOKEN_ARRAY = 33,               /* TOKEN_ARRAY  */
  YYSYMBOL_YYACCEPT = 34,                  /* $accept  */
  YYSYMBOL_xfile = 35,                     /* xfile  */
  YYSYMBOL_template = 36,                  /* template  */
  YYSYMBOL_37_1 = 37,                      /* @1  */
  YYSYMBOL_template_parts = 38,            /* template_parts  */
  YYSYMBOL_template_members_part = 39,     /* template_members_part  */
  YYSYMBOL_template_option_info = 40,      /* template_option_info  */
  YYSYMBOL_template_members_list = 41,     /* template_members_list  */
  YYSYMBOL_template_members = 42,          /* template_members  */
  YYSYMBOL_primitive = 43,                 /* primitive  */
  YYSYMBOL_array = 44,                     /* array  */
  YYSYMBOL_template_reference = 45,        /* template_reference  */
  YYSYMBOL_primitive_type = 46,            /* primitive_type  */
  YYSYMBOL_array_data_type = 47,           /* array_data_type  */
  YYSYMBOL_dimension_list = 48,            /* dimension_list  */
  YYSYMBOL_dimension = 49,                 /* dimension  */
  YYSYMBOL_dimension_size = 50,            /* dimension_size  */
  YYSYMBOL_template_option_list = 51,      /* template_option_list  */
  YYSYMBOL_template_option_part = 52,      /* template_option_part  */
  YYSYMBOL_singleword_name = 53,           /* singleword_name  */
  YYSYMBOL_multiword_name = 54,            /* multiword_name  */
  YYSYMBOL_optional_multiword_name = 55,   /* optional_multiword_name  */
  YYSYMBOL_class_id = 56,                  /* class_id  */
  YYSYMBOL_optional_class_id = 57,         /* optional_class_id  */
  YYSYMBOL_ellipsis = 58,                  /* ellipsis  */
  YYSYMBOL_object = 59,                    /* object  */
  YYSYMBOL_60_2 = 60,                      /* @2  */
  YYSYMBOL_data_parts_list = 61,           /* data_parts_list  */
  YYSYMBOL_data_part = 62,                 /* data_part  */
  YYSYMBOL_integer_list = 63,              /* integer_list  */
  YYSYMBOL_realnum_list = 64,              /* realnum_list  */
  YYSYMBOL_string = 65,                    /* string  */
  YYSYMBOL_list_separator = 66,            /* list_separator  */
  YYSYMBOL_data_reference = 67,            /* data_reference  */
  YYSYMBOL_empty = 68                      /* empty  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

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


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
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

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

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
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
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
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   114

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  34
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  35
/* YYNRULES -- Number of rules.  */
#define YYNRULES  71
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  100

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   257


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     3,     4,     5,     2,     6,     7,     8,     2,     2,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    20,     2,     2,     2,     2,     2,     2,     2,     2,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,     2,     2,     2,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   109,   109,   110,   111,   112,   117,   116,   131,   132,
     136,   137,   141,   145,   149,   150,   154,   155,   156,   160,
     168,   172,   185,   189,   193,   197,   201,   205,   209,   213,
     217,   221,   225,   232,   237,   250,   251,   255,   259,   263,
     275,   278,   284,   293,   309,   313,   314,   318,   325,   329,
     333,   337,   341,   345,   350,   349,   377,   378,   382,   387,
     391,   399,   407,   415,   421,   425,   429,   433,   434,   438,
     447,   463
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "TOKEN_NAME",
  "TOKEN_STRING", "TOKEN_INTEGER", "TOKEN_GUID", "TOKEN_INTEGER_LIST",
  "TOKEN_REALNUM_LIST", "TOKEN_OBRACE", "TOKEN_CBRACE", "TOKEN_OPAREN",
  "TOKEN_CPAREN", "TOKEN_OBRACKET", "TOKEN_CBRACKET", "TOKEN_OANGLE",
  "TOKEN_CANGLE", "TOKEN_DOT", "TOKEN_COMMA", "TOKEN_SEMICOLON",
  "TOKEN_TEMPLATE", "TOKEN_WORD", "TOKEN_DWORD", "TOKEN_FLOAT",
  "TOKEN_DOUBLE", "TOKEN_CHAR", "TOKEN_UCHAR", "TOKEN_SWORD",
  "TOKEN_SDWORD", "TOKEN_VOID", "TOKEN_LPSTR", "TOKEN_UNICODE",
  "TOKEN_CSTRING", "TOKEN_ARRAY", "$accept", "xfile", "template", "@1",
  "template_parts", "template_members_part", "template_option_info",
  "template_members_list", "template_members", "primitive", "array",
  "template_reference", "primitive_type", "array_data_type",
  "dimension_list", "dimension", "dimension_size", "template_option_list",
  "template_option_part", "singleword_name", "multiword_name",
  "optional_multiword_name", "class_id", "optional_class_id", "ellipsis",
  "object", "@2", "data_parts_list", "data_part", "integer_list",
  "realnum_list", "string", "list_separator", "data_reference", "empty", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-48)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-12)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
     -48,     8,   -48,   -48,   -48,   -48,     9,   -48,    40,   -48,
      41,   -48,     4,    42,   -48,    43,   -48,   -48,   -48,   -48,
     -48,    43,    52,   -48,   -48,   -48,   -48,   -48,   -48,   -48,
     -48,   -48,   -48,   -48,   -48,   -48,   -48,    65,    44,    39,
      14,   -48,   -48,   -48,   -48,    40,    40,   -48,    95,   -48,
      40,    45,    40,   -48,     3,   -48,    34,    37,   -48,   -48,
     -48,    40,   -48,   -48,   -48,   -48,   -48,   -48,   -48,    11,
     -48,     4,    10,    -9,   -48,     4,    46,    47,     9,   -48,
      43,   -48,   -48,   -48,    16,    49,   -48,   -48,    48,     4,
     -48,   -48,    50,   -48,   -48,   -48,   -48,   -48,   -48,   -48
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
      71,     0,     2,     1,    44,     5,     0,     3,    71,     4,
       0,    45,    49,     0,    48,     0,    46,    47,    54,    50,
       6,    71,    71,    52,    71,    51,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,     0,     0,     0,
       9,    14,    16,    17,    18,    71,    71,    10,     0,    56,
       0,     0,     0,     7,     0,    15,     0,     0,    66,    64,
      65,     0,    55,    68,    67,    59,    57,    60,    61,     0,
      63,    33,     0,     0,    35,    34,     0,     0,    13,    40,
      42,    12,    19,    21,    69,     0,    62,    38,     0,    39,
      20,    36,     0,     8,    41,    43,    70,    58,    37,    53
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -48,   -48,   -48,   -48,   -48,   -48,   -48,   -48,    17,   -48,
     -48,   -48,    28,   -48,   -48,    -7,   -48,   -48,    -8,    -6,
     -47,   -13,   -20,   -48,   -48,    21,   -48,   -48,   -48,   -48,
     -48,   -48,    12,   -48,     2
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     1,     7,    22,    38,    39,    77,    40,    41,    42,
      43,    44,    45,    51,    73,    74,    88,    78,    79,     8,
      12,    13,    20,    24,    81,     9,    21,    48,    66,    67,
      68,    69,    70,    85,    14
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      10,    23,     2,    71,    72,    75,     4,    16,     3,    17,
      90,     4,     4,    11,    84,    87,    46,     4,     5,    16,
      76,    17,    19,    25,    47,    89,    49,   -11,     6,    63,
      64,    52,    56,    57,    46,    26,    27,    28,    29,    30,
      31,    32,    33,    11,    34,    35,    36,    37,    80,    19,
      15,    18,    54,    82,    53,     4,    83,    55,    72,    97,
      95,    93,    98,    92,    96,    50,    91,    99,     4,    65,
      94,     0,    80,    26,    27,    28,    29,    30,    31,    32,
      33,    86,    34,    35,    36,    37,    26,    27,    28,    29,
      30,    31,    32,    33,     0,    34,    35,    36,     4,    58,
       0,     0,    59,    60,    61,    62,     0,     0,     0,     0,
       0,     0,     0,    63,    64
};

static const yytype_int8 yycheck[] =
{
       6,    21,     0,    50,    13,    52,     3,     3,     0,     5,
      19,     3,     3,     3,    61,     5,    22,     3,    10,     3,
      17,     5,     6,    21,    22,    72,    24,    13,    20,    18,
      19,    37,    45,    46,    40,    21,    22,    23,    24,    25,
      26,    27,    28,     3,    30,    31,    32,    33,    54,     6,
       9,     9,    13,    19,    10,     3,    19,    40,    13,    10,
      80,    14,    14,    17,    84,    37,    73,    17,     3,    48,
      78,    -1,    78,    21,    22,    23,    24,    25,    26,    27,
      28,    69,    30,    31,    32,    33,    21,    22,    23,    24,
      25,    26,    27,    28,    -1,    30,    31,    32,     3,     4,
      -1,    -1,     7,     8,     9,    10,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    18,    19
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    35,    68,     0,     3,    10,    20,    36,    53,    59,
      53,     3,    54,    55,    68,     9,     3,     5,     9,     6,
      56,    60,    37,    56,    57,    68,    21,    22,    23,    24,
      25,    26,    27,    28,    30,    31,    32,    33,    38,    39,
      41,    42,    43,    44,    45,    46,    53,    68,    61,    68,
      46,    47,    53,    10,    13,    42,    55,    55,     4,     7,
       8,     9,    10,    18,    19,    59,    62,    63,    64,    65,
      66,    54,    13,    48,    49,    54,    17,    40,    51,    52,
      53,    58,    19,    19,    54,    67,    66,     5,    50,    54,
      19,    49,    17,    14,    52,    56,    56,    10,    14,    17
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    34,    35,    35,    35,    35,    37,    36,    38,    38,
      39,    39,    40,    40,    41,    41,    42,    42,    42,    43,
      44,    45,    46,    46,    46,    46,    46,    46,    46,    46,
      46,    46,    46,    47,    47,    48,    48,    49,    50,    50,
      51,    51,    52,    52,    53,    54,    54,    54,    55,    55,
      56,    57,    57,    58,    60,    59,    61,    61,    62,    62,
      62,    62,    62,    62,    63,    64,    65,    66,    66,    67,
      67,    68
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     2,     2,     2,     0,     7,     4,     1,
       1,     1,     1,     1,     1,     2,     1,     1,     1,     3,
       4,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     2,     1,     2,     3,     1,     1,
       1,     2,     1,     2,     1,     1,     2,     2,     1,     1,
       1,     1,     1,     3,     0,     7,     1,     2,     3,     1,
       1,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       2,     0
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
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

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


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




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
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
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
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
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


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

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
| yyreduce -- do a reduction.  |
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
  case 6: /* @1: %empty  */
#line 117 "pandatool/src/xfile/xParser.yxx"
{
  (yyval.u.node) = current_node;
  XFileTemplate *templ = new XFileTemplate(x_file, (yyvsp[-2].str), (yyvsp[0].guid));
  current_node->add_child(templ);
  current_node = templ;
}
#line 1258 "built/tmp/xParser.c"
    break;

  case 7: /* template: TOKEN_TEMPLATE singleword_name TOKEN_OBRACE class_id @1 template_parts TOKEN_CBRACE  */
#line 124 "pandatool/src/xfile/xParser.yxx"
{
  (yyval.u.node) = current_node;
  current_node = (yyvsp[-2].u.node);
}
#line 1267 "built/tmp/xParser.c"
    break;

  case 12: /* template_option_info: ellipsis  */
#line 142 "pandatool/src/xfile/xParser.yxx"
{
  DCAST(XFileTemplate, current_node)->set_open(true);
}
#line 1275 "built/tmp/xParser.c"
    break;

  case 19: /* primitive: primitive_type optional_multiword_name TOKEN_SEMICOLON  */
#line 161 "pandatool/src/xfile/xParser.yxx"
{
  current_data_def = new XFileDataDef(x_file, (yyvsp[-1].str), (yyvsp[-2].u.primitive_type));
  current_node->add_child(current_data_def);
}
#line 1284 "built/tmp/xParser.c"
    break;

  case 21: /* template_reference: singleword_name optional_multiword_name TOKEN_SEMICOLON  */
#line 173 "pandatool/src/xfile/xParser.yxx"
{
  XFileTemplate *xtemplate = x_file->find_template((yyvsp[-2].str));
  if (xtemplate == nullptr) {
    yyerror("Unknown template: " + (yyvsp[-2].str));
  } else {
    current_data_def = new XFileDataDef(x_file, (yyvsp[-1].str), XFileDataDef::T_template, xtemplate);
    current_node->add_child(current_data_def);
  }
}
#line 1298 "built/tmp/xParser.c"
    break;

  case 22: /* primitive_type: TOKEN_WORD  */
#line 186 "pandatool/src/xfile/xParser.yxx"
{
  (yyval.u.primitive_type) = XFileDataDef::T_word;
}
#line 1306 "built/tmp/xParser.c"
    break;

  case 23: /* primitive_type: TOKEN_DWORD  */
#line 190 "pandatool/src/xfile/xParser.yxx"
{
  (yyval.u.primitive_type) = XFileDataDef::T_dword;
}
#line 1314 "built/tmp/xParser.c"
    break;

  case 24: /* primitive_type: TOKEN_FLOAT  */
#line 194 "pandatool/src/xfile/xParser.yxx"
{
  (yyval.u.primitive_type) = XFileDataDef::T_float;
}
#line 1322 "built/tmp/xParser.c"
    break;

  case 25: /* primitive_type: TOKEN_DOUBLE  */
#line 198 "pandatool/src/xfile/xParser.yxx"
{
  (yyval.u.primitive_type) = XFileDataDef::T_double;
}
#line 1330 "built/tmp/xParser.c"
    break;

  case 26: /* primitive_type: TOKEN_CHAR  */
#line 202 "pandatool/src/xfile/xParser.yxx"
{
  (yyval.u.primitive_type) = XFileDataDef::T_char;
}
#line 1338 "built/tmp/xParser.c"
    break;

  case 27: /* primitive_type: TOKEN_UCHAR  */
#line 206 "pandatool/src/xfile/xParser.yxx"
{
  (yyval.u.primitive_type) = XFileDataDef::T_uchar;
}
#line 1346 "built/tmp/xParser.c"
    break;

  case 28: /* primitive_type: TOKEN_SWORD  */
#line 210 "pandatool/src/xfile/xParser.yxx"
{
  (yyval.u.primitive_type) = XFileDataDef::T_sword;
}
#line 1354 "built/tmp/xParser.c"
    break;

  case 29: /* primitive_type: TOKEN_SDWORD  */
#line 214 "pandatool/src/xfile/xParser.yxx"
{
  (yyval.u.primitive_type) = XFileDataDef::T_sdword;
}
#line 1362 "built/tmp/xParser.c"
    break;

  case 30: /* primitive_type: TOKEN_LPSTR  */
#line 218 "pandatool/src/xfile/xParser.yxx"
{
  (yyval.u.primitive_type) = XFileDataDef::T_string;
}
#line 1370 "built/tmp/xParser.c"
    break;

  case 31: /* primitive_type: TOKEN_UNICODE  */
#line 222 "pandatool/src/xfile/xParser.yxx"
{
  (yyval.u.primitive_type) = XFileDataDef::T_unicode;
}
#line 1378 "built/tmp/xParser.c"
    break;

  case 32: /* primitive_type: TOKEN_CSTRING  */
#line 226 "pandatool/src/xfile/xParser.yxx"
{
  (yyval.u.primitive_type) = XFileDataDef::T_cstring;
}
#line 1386 "built/tmp/xParser.c"
    break;

  case 33: /* array_data_type: primitive_type multiword_name  */
#line 233 "pandatool/src/xfile/xParser.yxx"
{
  current_data_def = new XFileDataDef(x_file, (yyvsp[0].str), (yyvsp[-1].u.primitive_type));
  current_node->add_child(current_data_def);
}
#line 1395 "built/tmp/xParser.c"
    break;

  case 34: /* array_data_type: singleword_name multiword_name  */
#line 238 "pandatool/src/xfile/xParser.yxx"
{
  XFileTemplate *xtemplate = x_file->find_template((yyvsp[-1].str));
  if (xtemplate == nullptr) {
    yyerror("Unknown template: " + (yyvsp[-1].str));
  } else {
    current_data_def = new XFileDataDef(x_file, (yyvsp[0].str), XFileDataDef::T_template, xtemplate);
    current_node->add_child(current_data_def);
  }
}
#line 1409 "built/tmp/xParser.c"
    break;

  case 38: /* dimension_size: TOKEN_INTEGER  */
#line 260 "pandatool/src/xfile/xParser.yxx"
{
  current_data_def->add_array_def(XFileArrayDef((yyvsp[0].u.number)));
}
#line 1417 "built/tmp/xParser.c"
    break;

  case 39: /* dimension_size: multiword_name  */
#line 264 "pandatool/src/xfile/xParser.yxx"
{
  XFileNode *data_def = current_node->find_child((yyvsp[0].str));
  if (data_def == nullptr) {
    yyerror("Unknown identifier: " + (yyvsp[0].str));
  } else {
    current_data_def->add_array_def(XFileArrayDef(DCAST(XFileDataDef, data_def)));
  }
}
#line 1430 "built/tmp/xParser.c"
    break;

  case 40: /* template_option_list: template_option_part  */
#line 276 "pandatool/src/xfile/xParser.yxx"
{
}
#line 1437 "built/tmp/xParser.c"
    break;

  case 41: /* template_option_list: template_option_list template_option_part  */
#line 279 "pandatool/src/xfile/xParser.yxx"
{
}
#line 1444 "built/tmp/xParser.c"
    break;

  case 42: /* template_option_part: singleword_name  */
#line 285 "pandatool/src/xfile/xParser.yxx"
{
  XFileTemplate *xtemplate = x_file->find_template((yyvsp[0].str));
  if (xtemplate == nullptr) {
    yyerror("Unknown template: " + (yyvsp[0].str));
  } else {
    DCAST(XFileTemplate, current_node)->add_option(xtemplate);
  }
}
#line 1457 "built/tmp/xParser.c"
    break;

  case 43: /* template_option_part: singleword_name class_id  */
#line 294 "pandatool/src/xfile/xParser.yxx"
{
  XFileTemplate *xtemplate = x_file->find_template((yyvsp[0].guid));
  if (xtemplate == nullptr) {
    yyerror("Unknown template: " + (yyvsp[-1].str));
  } else {
    if (xtemplate->get_name() != (yyvsp[-1].str)) {
      xyywarning("GUID identifies template " + xtemplate->get_name() +
                 ", not " + (yyvsp[-1].str));
    }
    DCAST(XFileTemplate, current_node)->add_option(xtemplate);
  }
}
#line 1474 "built/tmp/xParser.c"
    break;

  case 46: /* multiword_name: multiword_name TOKEN_NAME  */
#line 315 "pandatool/src/xfile/xParser.yxx"
{
  (yyval.str) = (yyvsp[-1].str) + " " + (yyvsp[0].str);
}
#line 1482 "built/tmp/xParser.c"
    break;

  case 47: /* multiword_name: multiword_name TOKEN_INTEGER  */
#line 319 "pandatool/src/xfile/xParser.yxx"
{
  (yyval.str) = (yyvsp[-1].str) + " " + (yyvsp[0].str);
}
#line 1490 "built/tmp/xParser.c"
    break;

  case 48: /* optional_multiword_name: empty  */
#line 326 "pandatool/src/xfile/xParser.yxx"
{
  (yyval.str) = std::string();
}
#line 1498 "built/tmp/xParser.c"
    break;

  case 51: /* optional_class_id: empty  */
#line 338 "pandatool/src/xfile/xParser.yxx"
{
  (yyval.guid) = WindowsGuid();
}
#line 1506 "built/tmp/xParser.c"
    break;

  case 54: /* @2: %empty  */
#line 350 "pandatool/src/xfile/xParser.yxx"
{
  XFileTemplate *xtemplate = x_file->find_template((yyvsp[-2].str));
  (yyval.u.node) = current_node;
  
  if (xtemplate == nullptr) {
    yyerror("Unknown template: " + (yyvsp[-2].str));
  } else {
    XFileDataNodeTemplate *templ = 
      new XFileDataNodeTemplate(x_file, (yyvsp[-1].str), xtemplate);
    current_node->add_child(templ);
    current_node = templ;
  }
}
#line 1524 "built/tmp/xParser.c"
    break;

  case 55: /* object: singleword_name optional_multiword_name TOKEN_OBRACE @2 optional_class_id data_parts_list TOKEN_CBRACE  */
#line 364 "pandatool/src/xfile/xParser.yxx"
{
  if (current_node->is_exact_type(XFileDataNodeTemplate::get_class_type())) {
    XFileDataNodeTemplate *current_template = 
      DCAST(XFileDataNodeTemplate, current_node);
    current_template->finalize_parse_data();
  }

  (yyval.u.node) = current_node;
  current_node = (yyvsp[-3].u.node);
}
#line 1539 "built/tmp/xParser.c"
    break;

  case 58: /* data_part: TOKEN_OBRACE data_reference TOKEN_CBRACE  */
#line 383 "pandatool/src/xfile/xParser.yxx"
{
  // nested references should be added as children too.
  current_node->add_child((yyvsp[-1].u.node));
}
#line 1548 "built/tmp/xParser.c"
    break;

  case 59: /* data_part: object  */
#line 388 "pandatool/src/xfile/xParser.yxx"
{
  // nested objects are just quietly added as children.
}
#line 1556 "built/tmp/xParser.c"
    break;

  case 60: /* data_part: integer_list  */
#line 392 "pandatool/src/xfile/xParser.yxx"
{
  if (current_node->is_exact_type(XFileDataNodeTemplate::get_class_type())) {
    XFileDataNodeTemplate *current_template = 
      DCAST(XFileDataNodeTemplate, current_node);
    current_template->add_parse_int((yyvsp[0].int_list));
  }
}
#line 1568 "built/tmp/xParser.c"
    break;

  case 61: /* data_part: realnum_list  */
#line 400 "pandatool/src/xfile/xParser.yxx"
{
  if (current_node->is_exact_type(XFileDataNodeTemplate::get_class_type())) {
    XFileDataNodeTemplate *current_template = 
      DCAST(XFileDataNodeTemplate, current_node);
    current_template->add_parse_double((yyvsp[0].double_list));
  }
}
#line 1580 "built/tmp/xParser.c"
    break;

  case 62: /* data_part: string list_separator  */
#line 408 "pandatool/src/xfile/xParser.yxx"
{
  if (current_node->is_exact_type(XFileDataNodeTemplate::get_class_type())) {
    XFileDataNodeTemplate *current_template = 
      DCAST(XFileDataNodeTemplate, current_node);
    current_template->add_parse_string((yyvsp[-1].str));
  }
}
#line 1592 "built/tmp/xParser.c"
    break;

  case 63: /* data_part: list_separator  */
#line 416 "pandatool/src/xfile/xParser.yxx"
{
}
#line 1599 "built/tmp/xParser.c"
    break;

  case 69: /* data_reference: multiword_name  */
#line 439 "pandatool/src/xfile/xParser.yxx"
{
  XFileDataNodeTemplate *data_object = x_file->find_data_object((yyvsp[0].str));
  if (data_object == nullptr) {
    yyerror("Unknown data_object: " + (yyvsp[0].str));
  }

  (yyval.u.node) = new XFileDataNodeReference(data_object);
}
#line 1612 "built/tmp/xParser.c"
    break;

  case 70: /* data_reference: multiword_name class_id  */
#line 448 "pandatool/src/xfile/xParser.yxx"
{
  XFileDataNodeTemplate *data_object = x_file->find_data_object((yyvsp[0].guid));
  if (data_object == nullptr) {
    yyerror("Unknown data_object: " + (yyvsp[-1].str));
  } else {
    if (data_object->get_name() != (yyvsp[-1].str)) {
      xyywarning("GUID identifies data_object " + data_object->get_name() +
                 ", not " + (yyvsp[-1].str));
    }
  }

  (yyval.u.node) = new XFileDataNodeReference(data_object);
}
#line 1630 "built/tmp/xParser.c"
    break;


#line 1634 "built/tmp/xParser.c"

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
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
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
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

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

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
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
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}


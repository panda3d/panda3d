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
#define yyparse         dcyyparse
#define yylex           dcyylex
#define yyerror         dcyyerror
#define yydebug         dcyydebug
#define yynerrs         dcyynerrs
#define yylval          dcyylval
#define yychar          dcyychar

/* First part of user prologue.  */
#line 7 "direct/src/dcparser/dcParser.yxx"

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


#line 160 "built/tmp/dcParser.yxx.c"

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

#include "dcParser.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_UNSIGNED_INTEGER = 3,           /* UNSIGNED_INTEGER  */
  YYSYMBOL_SIGNED_INTEGER = 4,             /* SIGNED_INTEGER  */
  YYSYMBOL_REAL = 5,                       /* REAL  */
  YYSYMBOL_STRING = 6,                     /* STRING  */
  YYSYMBOL_IDENTIFIER = 7,                 /* IDENTIFIER  */
  YYSYMBOL_HEX_STRING = 8,                 /* HEX_STRING  */
  YYSYMBOL_KEYWORD = 9,                    /* KEYWORD  */
  YYSYMBOL_KW_DCLASS = 10,                 /* KW_DCLASS  */
  YYSYMBOL_KW_STRUCT = 11,                 /* KW_STRUCT  */
  YYSYMBOL_KW_FROM = 12,                   /* KW_FROM  */
  YYSYMBOL_KW_IMPORT = 13,                 /* KW_IMPORT  */
  YYSYMBOL_KW_TYPEDEF = 14,                /* KW_TYPEDEF  */
  YYSYMBOL_KW_KEYWORD = 15,                /* KW_KEYWORD  */
  YYSYMBOL_KW_SWITCH = 16,                 /* KW_SWITCH  */
  YYSYMBOL_KW_CASE = 17,                   /* KW_CASE  */
  YYSYMBOL_KW_DEFAULT = 18,                /* KW_DEFAULT  */
  YYSYMBOL_KW_BREAK = 19,                  /* KW_BREAK  */
  YYSYMBOL_KW_INT8 = 20,                   /* KW_INT8  */
  YYSYMBOL_KW_INT16 = 21,                  /* KW_INT16  */
  YYSYMBOL_KW_INT32 = 22,                  /* KW_INT32  */
  YYSYMBOL_KW_INT64 = 23,                  /* KW_INT64  */
  YYSYMBOL_KW_UINT8 = 24,                  /* KW_UINT8  */
  YYSYMBOL_KW_UINT16 = 25,                 /* KW_UINT16  */
  YYSYMBOL_KW_UINT32 = 26,                 /* KW_UINT32  */
  YYSYMBOL_KW_UINT64 = 27,                 /* KW_UINT64  */
  YYSYMBOL_KW_FLOAT64 = 28,                /* KW_FLOAT64  */
  YYSYMBOL_KW_STRING = 29,                 /* KW_STRING  */
  YYSYMBOL_KW_BLOB = 30,                   /* KW_BLOB  */
  YYSYMBOL_KW_BLOB32 = 31,                 /* KW_BLOB32  */
  YYSYMBOL_KW_INT8ARRAY = 32,              /* KW_INT8ARRAY  */
  YYSYMBOL_KW_INT16ARRAY = 33,             /* KW_INT16ARRAY  */
  YYSYMBOL_KW_INT32ARRAY = 34,             /* KW_INT32ARRAY  */
  YYSYMBOL_KW_UINT8ARRAY = 35,             /* KW_UINT8ARRAY  */
  YYSYMBOL_KW_UINT16ARRAY = 36,            /* KW_UINT16ARRAY  */
  YYSYMBOL_KW_UINT32ARRAY = 37,            /* KW_UINT32ARRAY  */
  YYSYMBOL_KW_UINT32UINT8ARRAY = 38,       /* KW_UINT32UINT8ARRAY  */
  YYSYMBOL_KW_CHAR = 39,                   /* KW_CHAR  */
  YYSYMBOL_START_DC = 40,                  /* START_DC  */
  YYSYMBOL_START_PARAMETER_VALUE = 41,     /* START_PARAMETER_VALUE  */
  YYSYMBOL_START_PARAMETER_DESCRIPTION = 42, /* START_PARAMETER_DESCRIPTION  */
  YYSYMBOL_43_ = 43,                       /* ';'  */
  YYSYMBOL_44_ = 44,                       /* '/'  */
  YYSYMBOL_45_ = 45,                       /* '.'  */
  YYSYMBOL_46_ = 46,                       /* '*'  */
  YYSYMBOL_47_ = 47,                       /* ','  */
  YYSYMBOL_48_ = 48,                       /* '{'  */
  YYSYMBOL_49_ = 49,                       /* '}'  */
  YYSYMBOL_50_ = 50,                       /* ':'  */
  YYSYMBOL_51_ = 51,                       /* '('  */
  YYSYMBOL_52_ = 52,                       /* ')'  */
  YYSYMBOL_53_ = 53,                       /* '='  */
  YYSYMBOL_54_ = 54,                       /* '%'  */
  YYSYMBOL_55_ = 55,                       /* '-'  */
  YYSYMBOL_56_ = 56,                       /* '['  */
  YYSYMBOL_57_ = 57,                       /* ']'  */
  YYSYMBOL_YYACCEPT = 58,                  /* $accept  */
  YYSYMBOL_grammar = 59,                   /* grammar  */
  YYSYMBOL_dc = 60,                        /* dc  */
  YYSYMBOL_slash_identifier = 61,          /* slash_identifier  */
  YYSYMBOL_import_identifier = 62,         /* import_identifier  */
  YYSYMBOL_import = 63,                    /* import  */
  YYSYMBOL_64_1 = 64,                      /* $@1  */
  YYSYMBOL_import_symbol_list_or_star = 65, /* import_symbol_list_or_star  */
  YYSYMBOL_import_symbol_list = 66,        /* import_symbol_list  */
  YYSYMBOL_typedef_decl = 67,              /* typedef_decl  */
  YYSYMBOL_keyword_decl = 68,              /* keyword_decl  */
  YYSYMBOL_keyword_decl_list = 69,         /* keyword_decl_list  */
  YYSYMBOL_dclass_or_struct = 70,          /* dclass_or_struct  */
  YYSYMBOL_dclass = 71,                    /* dclass  */
  YYSYMBOL_72_2 = 72,                      /* @2  */
  YYSYMBOL_dclass_name = 73,               /* dclass_name  */
  YYSYMBOL_dclass_derivation = 74,         /* dclass_derivation  */
  YYSYMBOL_dclass_base_list = 75,          /* dclass_base_list  */
  YYSYMBOL_dclass_fields = 76,             /* dclass_fields  */
  YYSYMBOL_dclass_field = 77,              /* dclass_field  */
  YYSYMBOL_struct = 78,                    /* struct  */
  YYSYMBOL_79_3 = 79,                      /* @3  */
  YYSYMBOL_struct_name = 80,               /* struct_name  */
  YYSYMBOL_struct_derivation = 81,         /* struct_derivation  */
  YYSYMBOL_struct_base_list = 82,          /* struct_base_list  */
  YYSYMBOL_struct_fields = 83,             /* struct_fields  */
  YYSYMBOL_struct_field = 84,              /* struct_field  */
  YYSYMBOL_atomic_field = 85,              /* atomic_field  */
  YYSYMBOL_86_4 = 86,                      /* @4  */
  YYSYMBOL_parameter_list = 87,            /* parameter_list  */
  YYSYMBOL_nonempty_parameter_list = 88,   /* nonempty_parameter_list  */
  YYSYMBOL_atomic_element = 89,            /* atomic_element  */
  YYSYMBOL_named_parameter = 90,           /* named_parameter  */
  YYSYMBOL_91_5 = 91,                      /* $@5  */
  YYSYMBOL_unnamed_parameter = 92,         /* unnamed_parameter  */
  YYSYMBOL_named_parameter_with_default = 93, /* named_parameter_with_default  */
  YYSYMBOL_94_6 = 94,                      /* $@6  */
  YYSYMBOL_unnamed_parameter_with_default = 95, /* unnamed_parameter_with_default  */
  YYSYMBOL_96_7 = 96,                      /* $@7  */
  YYSYMBOL_parameter = 97,                 /* parameter  */
  YYSYMBOL_parameter_with_default = 98,    /* parameter_with_default  */
  YYSYMBOL_parameter_or_atomic = 99,       /* parameter_or_atomic  */
  YYSYMBOL_parameter_description = 100,    /* parameter_description  */
  YYSYMBOL_simple_type_name = 101,         /* simple_type_name  */
  YYSYMBOL_type_name = 102,                /* type_name  */
  YYSYMBOL_double_range = 103,             /* double_range  */
  YYSYMBOL_uint_range = 104,               /* uint_range  */
  YYSYMBOL_type_definition = 105,          /* type_definition  */
  YYSYMBOL_parameter_definition = 106,     /* parameter_definition  */
  YYSYMBOL_char_or_uint = 107,             /* char_or_uint  */
  YYSYMBOL_small_unsigned_integer = 108,   /* small_unsigned_integer  */
  YYSYMBOL_small_negative_integer = 109,   /* small_negative_integer  */
  YYSYMBOL_signed_integer = 110,           /* signed_integer  */
  YYSYMBOL_unsigned_integer = 111,         /* unsigned_integer  */
  YYSYMBOL_number = 112,                   /* number  */
  YYSYMBOL_char_or_number = 113,           /* char_or_number  */
  YYSYMBOL_parameter_value = 114,          /* parameter_value  */
  YYSYMBOL_115_8 = 115,                    /* $@8  */
  YYSYMBOL_parameter_actual_value = 116,   /* parameter_actual_value  */
  YYSYMBOL_117_9 = 117,                    /* $@9  */
  YYSYMBOL_118_10 = 118,                   /* $@10  */
  YYSYMBOL_119_11 = 119,                   /* $@11  */
  YYSYMBOL_array = 120,                    /* array  */
  YYSYMBOL_maybe_comma = 121,              /* maybe_comma  */
  YYSYMBOL_array_def = 122,                /* array_def  */
  YYSYMBOL_type_token = 123,               /* type_token  */
  YYSYMBOL_keyword_list = 124,             /* keyword_list  */
  YYSYMBOL_no_keyword_list = 125,          /* no_keyword_list  */
  YYSYMBOL_molecular_field = 126,          /* molecular_field  */
  YYSYMBOL_127_12 = 127,                   /* $@12  */
  YYSYMBOL_atomic_name = 128,              /* atomic_name  */
  YYSYMBOL_molecular_atom_list = 129,      /* molecular_atom_list  */
  YYSYMBOL_optional_name = 130,            /* optional_name  */
  YYSYMBOL_switch = 131,                   /* switch  */
  YYSYMBOL_132_13 = 132,                   /* @13  */
  YYSYMBOL_switch_fields = 133,            /* switch_fields  */
  YYSYMBOL_switch_case = 134,              /* switch_case  */
  YYSYMBOL_135_14 = 135,                   /* $@14  */
  YYSYMBOL_switch_default = 136,           /* switch_default  */
  YYSYMBOL_switch_break = 137,             /* switch_break  */
  YYSYMBOL_switch_field = 138,             /* switch_field  */
  YYSYMBOL_empty = 139                     /* empty  */
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
typedef yytype_int16 yy_state_t;

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

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   297


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
static const yytype_int16 yyrline[] =
{
       0,   174,   174,   175,   176,   183,   184,   185,   196,   202,
     203,   204,   208,   209,   216,   217,   224,   229,   228,   236,
     237,   244,   248,   255,   273,   277,   278,   282,   293,   294,
     299,   298,   311,   334,   335,   339,   345,   359,   360,   361,
     374,   384,   385,   393,   404,   403,   416,   439,   440,   444,
     450,   459,   460,   461,   472,   479,   480,   484,   492,   491,
     510,   511,   515,   516,   520,   530,   529,   540,   544,   546,
     545,   574,   576,   575,   604,   605,   609,   610,   614,   618,
     625,   629,   633,   640,   644,   653,   669,   684,   685,   717,
     733,   752,   756,   763,   770,   779,   785,   791,   802,   806,
     813,   820,   827,   833,   839,   848,   849,   860,   865,   880,
     895,   902,   911,   915,   926,   940,   944,   948,   952,   956,
     960,   969,   974,   978,   977,   992,   996,  1000,  1004,  1008,
    1013,  1012,  1021,  1020,  1029,  1028,  1036,  1042,  1048,  1054,
    1063,  1064,  1068,  1069,  1073,  1074,  1078,  1082,  1086,  1090,
    1094,  1098,  1102,  1106,  1110,  1114,  1118,  1122,  1126,  1130,
    1134,  1138,  1142,  1146,  1150,  1154,  1161,  1165,  1172,  1182,
    1181,  1192,  1219,  1225,  1239,  1243,  1248,  1247,  1261,  1262,
    1263,  1264,  1265,  1266,  1280,  1279,  1300,  1309,  1316,  1320,
    1326
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
  "\"end of file\"", "error", "\"invalid token\"", "UNSIGNED_INTEGER",
  "SIGNED_INTEGER", "REAL", "STRING", "IDENTIFIER", "HEX_STRING",
  "KEYWORD", "KW_DCLASS", "KW_STRUCT", "KW_FROM", "KW_IMPORT",
  "KW_TYPEDEF", "KW_KEYWORD", "KW_SWITCH", "KW_CASE", "KW_DEFAULT",
  "KW_BREAK", "KW_INT8", "KW_INT16", "KW_INT32", "KW_INT64", "KW_UINT8",
  "KW_UINT16", "KW_UINT32", "KW_UINT64", "KW_FLOAT64", "KW_STRING",
  "KW_BLOB", "KW_BLOB32", "KW_INT8ARRAY", "KW_INT16ARRAY", "KW_INT32ARRAY",
  "KW_UINT8ARRAY", "KW_UINT16ARRAY", "KW_UINT32ARRAY",
  "KW_UINT32UINT8ARRAY", "KW_CHAR", "START_DC", "START_PARAMETER_VALUE",
  "START_PARAMETER_DESCRIPTION", "';'", "'/'", "'.'", "'*'", "','", "'{'",
  "'}'", "':'", "'('", "')'", "'='", "'%'", "'-'", "'['", "']'", "$accept",
  "grammar", "dc", "slash_identifier", "import_identifier", "import",
  "$@1", "import_symbol_list_or_star", "import_symbol_list",
  "typedef_decl", "keyword_decl", "keyword_decl_list", "dclass_or_struct",
  "dclass", "@2", "dclass_name", "dclass_derivation", "dclass_base_list",
  "dclass_fields", "dclass_field", "struct", "@3", "struct_name",
  "struct_derivation", "struct_base_list", "struct_fields", "struct_field",
  "atomic_field", "@4", "parameter_list", "nonempty_parameter_list",
  "atomic_element", "named_parameter", "$@5", "unnamed_parameter",
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

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-127)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-176)

#define yytable_value_is_error(Yyn) \
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
       0,     4,     5,    97,    98,    64,   186,   209,   210,    65,
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

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
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

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
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

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
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
  case 4: /* grammar: START_PARAMETER_DESCRIPTION parameter_description  */
#line 177 "direct/src/dcparser/dcParser.yxx"
{
  parameter_description = (yyvsp[0].u.field);
}
#line 1549 "built/tmp/dcParser.yxx.c"
    break;

  case 7: /* dc: dc dclass_or_struct  */
#line 186 "direct/src/dcparser/dcParser.yxx"
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
#line 1564 "built/tmp/dcParser.yxx.c"
    break;

  case 8: /* dc: dc switch  */
#line 197 "direct/src/dcparser/dcParser.yxx"
{
  if (!dc_file->add_switch((yyvsp[0].u.dswitch))) {
    yyerror("Duplicate class name: " + (yyvsp[0].u.dswitch)->get_name());
  }
}
#line 1574 "built/tmp/dcParser.yxx.c"
    break;

  case 13: /* slash_identifier: slash_identifier '/' IDENTIFIER  */
#line 210 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.str) = (yyvsp[-2].str) + string("/") + (yyvsp[0].str);
}
#line 1582 "built/tmp/dcParser.yxx.c"
    break;

  case 15: /* import_identifier: import_identifier '.' slash_identifier  */
#line 218 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.str) = (yyvsp[-2].str) + string(".") + (yyvsp[0].str);
}
#line 1590 "built/tmp/dcParser.yxx.c"
    break;

  case 16: /* import: KW_IMPORT import_identifier  */
#line 225 "direct/src/dcparser/dcParser.yxx"
{
  dc_file->add_import_module((yyvsp[0].str));
}
#line 1598 "built/tmp/dcParser.yxx.c"
    break;

  case 17: /* $@1: %empty  */
#line 229 "direct/src/dcparser/dcParser.yxx"
{
  dc_file->add_import_module((yyvsp[-1].str));
}
#line 1606 "built/tmp/dcParser.yxx.c"
    break;

  case 20: /* import_symbol_list_or_star: '*'  */
#line 238 "direct/src/dcparser/dcParser.yxx"
{
  dc_file->add_import_symbol("*");
}
#line 1614 "built/tmp/dcParser.yxx.c"
    break;

  case 21: /* import_symbol_list: slash_identifier  */
#line 245 "direct/src/dcparser/dcParser.yxx"
{
  dc_file->add_import_symbol((yyvsp[0].str));
}
#line 1622 "built/tmp/dcParser.yxx.c"
    break;

  case 22: /* import_symbol_list: import_symbol_list ',' slash_identifier  */
#line 249 "direct/src/dcparser/dcParser.yxx"
{
  dc_file->add_import_symbol((yyvsp[0].str));
}
#line 1630 "built/tmp/dcParser.yxx.c"
    break;

  case 23: /* typedef_decl: KW_TYPEDEF parameter_with_default  */
#line 256 "direct/src/dcparser/dcParser.yxx"
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
#line 1649 "built/tmp/dcParser.yxx.c"
    break;

  case 26: /* keyword_decl_list: keyword_decl_list IDENTIFIER  */
#line 279 "direct/src/dcparser/dcParser.yxx"
{
  dc_file->add_keyword((yyvsp[0].str));
}
#line 1657 "built/tmp/dcParser.yxx.c"
    break;

  case 27: /* keyword_decl_list: keyword_decl_list KEYWORD  */
#line 283 "direct/src/dcparser/dcParser.yxx"
{
  // This keyword has already been defined.  But since we are now
  // explicitly defining it, clear its bitmask, so that we will have a
  // new hash code--doing this will allow us to phase out the
  // historical hash code support later.
  ((DCKeyword *)(yyvsp[0].u.keyword))->clear_historical_flag();
}
#line 1669 "built/tmp/dcParser.yxx.c"
    break;

  case 30: /* @2: %empty  */
#line 299 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.dclass) = current_class;
  current_class = new DCClass(dc_file, (yyvsp[0].str), false, false);
}
#line 1678 "built/tmp/dcParser.yxx.c"
    break;

  case 31: /* dclass: KW_DCLASS optional_name @2 dclass_derivation '{' dclass_fields '}'  */
#line 304 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.dclass) = current_class;
  current_class = (yyvsp[-4].u.dclass);
}
#line 1687 "built/tmp/dcParser.yxx.c"
    break;

  case 32: /* dclass_name: IDENTIFIER  */
#line 312 "direct/src/dcparser/dcParser.yxx"
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
#line 1711 "built/tmp/dcParser.yxx.c"
    break;

  case 35: /* dclass_base_list: dclass_name  */
#line 340 "direct/src/dcparser/dcParser.yxx"
{
  if ((yyvsp[0].u.dclass) != nullptr) {
    current_class->add_parent((yyvsp[0].u.dclass));
  }
}
#line 1721 "built/tmp/dcParser.yxx.c"
    break;

  case 36: /* dclass_base_list: dclass_base_list ',' dclass_name  */
#line 346 "direct/src/dcparser/dcParser.yxx"
{
  if (!dc_multiple_inheritance) {
    yyerror("Multiple inheritance is not supported without \"dc-multiple-inheritance 1\" in your Config.prc file.");

  } else {
    if ((yyvsp[0].u.dclass) != nullptr) {
      current_class->add_parent((yyvsp[0].u.dclass));
    }
  }
}
#line 1736 "built/tmp/dcParser.yxx.c"
    break;

  case 39: /* dclass_fields: dclass_fields dclass_field ';'  */
#line 362 "direct/src/dcparser/dcParser.yxx"
{
  if ((yyvsp[-1].u.field) == nullptr) {
    // Pass this error up.
  } else if (!current_class->add_field((yyvsp[-1].u.field))) {
    yyerror("Duplicate field name: " + (yyvsp[-1].u.field)->get_name());
  } else if ((yyvsp[-1].u.field)->get_number() < 0) {
    yyerror("A non-network field cannot be stored on a dclass");
  }
}
#line 1750 "built/tmp/dcParser.yxx.c"
    break;

  case 40: /* dclass_field: atomic_field keyword_list  */
#line 375 "direct/src/dcparser/dcParser.yxx"
{
  if ((yyvsp[-1].u.field) != nullptr) {
    if ((yyvsp[-1].u.field)->get_name().empty()) {
      yyerror("Field name required.");
    }
    (yyvsp[-1].u.field)->copy_keywords(current_keyword_list);
  }
  (yyval.u.field) = (yyvsp[-1].u.field);
}
#line 1764 "built/tmp/dcParser.yxx.c"
    break;

  case 42: /* dclass_field: unnamed_parameter_with_default keyword_list  */
#line 386 "direct/src/dcparser/dcParser.yxx"
{
  yyerror("Unnamed parameters are not allowed on a dclass");
  if ((yyvsp[-1].u.parameter) != nullptr) {
    (yyvsp[-1].u.parameter)->copy_keywords(current_keyword_list);
  }
  (yyval.u.field) = (yyvsp[-1].u.parameter);
}
#line 1776 "built/tmp/dcParser.yxx.c"
    break;

  case 43: /* dclass_field: named_parameter_with_default keyword_list  */
#line 394 "direct/src/dcparser/dcParser.yxx"
{
  if ((yyvsp[-1].u.parameter) != nullptr) {
    (yyvsp[-1].u.parameter)->copy_keywords(current_keyword_list);
  }
  (yyval.u.field) = (yyvsp[-1].u.parameter);
}
#line 1787 "built/tmp/dcParser.yxx.c"
    break;

  case 44: /* @3: %empty  */
#line 404 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.dclass) = current_class;
  current_class = new DCClass(dc_file, (yyvsp[0].str), true, false);
}
#line 1796 "built/tmp/dcParser.yxx.c"
    break;

  case 45: /* struct: KW_STRUCT optional_name @3 struct_derivation '{' struct_fields '}'  */
#line 409 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.dclass) = current_class;
  current_class = (yyvsp[-4].u.dclass);
}
#line 1805 "built/tmp/dcParser.yxx.c"
    break;

  case 46: /* struct_name: IDENTIFIER  */
#line 417 "direct/src/dcparser/dcParser.yxx"
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
#line 1829 "built/tmp/dcParser.yxx.c"
    break;

  case 49: /* struct_base_list: struct_name  */
#line 445 "direct/src/dcparser/dcParser.yxx"
{
  if ((yyvsp[0].u.dclass) != nullptr) {
    current_class->add_parent((yyvsp[0].u.dclass));
  }
}
#line 1839 "built/tmp/dcParser.yxx.c"
    break;

  case 50: /* struct_base_list: struct_base_list ',' struct_name  */
#line 451 "direct/src/dcparser/dcParser.yxx"
{
  if ((yyvsp[0].u.dclass) != nullptr) {
    current_class->add_parent((yyvsp[0].u.dclass));
  }
}
#line 1849 "built/tmp/dcParser.yxx.c"
    break;

  case 53: /* struct_fields: struct_fields struct_field ';'  */
#line 462 "direct/src/dcparser/dcParser.yxx"
{
  if ((yyvsp[-1].u.field) == nullptr) {
    // Pass this error up.
  } else if (!current_class->add_field((yyvsp[-1].u.field))) {
    yyerror("Duplicate field name: " + (yyvsp[-1].u.field)->get_name());
  }
}
#line 1861 "built/tmp/dcParser.yxx.c"
    break;

  case 54: /* struct_field: atomic_field no_keyword_list  */
#line 473 "direct/src/dcparser/dcParser.yxx"
{
  if ((yyvsp[-1].u.field)->get_name().empty()) {
    yyerror("Field name required.");
  }
  (yyval.u.field) = (yyvsp[-1].u.field);
}
#line 1872 "built/tmp/dcParser.yxx.c"
    break;

  case 56: /* struct_field: unnamed_parameter_with_default no_keyword_list  */
#line 481 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.field) = (yyvsp[-1].u.parameter);
}
#line 1880 "built/tmp/dcParser.yxx.c"
    break;

  case 57: /* struct_field: named_parameter_with_default no_keyword_list  */
#line 485 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.field) = (yyvsp[-1].u.parameter);
}
#line 1888 "built/tmp/dcParser.yxx.c"
    break;

  case 58: /* @4: %empty  */
#line 492 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.atomic) = current_atomic;
  if (current_class == nullptr) {
    yyerror("Cannot define a method outside of a struct or class.");
    DCClass *temp_class = new DCClass(dc_file, "temp", false, false);  // memory leak.
    current_atomic = new DCAtomicField((yyvsp[-1].str), temp_class, false);
  } else {
    current_atomic = new DCAtomicField((yyvsp[-1].str), current_class, false);
  }
}
#line 1903 "built/tmp/dcParser.yxx.c"
    break;

  case 59: /* atomic_field: optional_name '(' @4 parameter_list ')'  */
#line 503 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.field) = current_atomic;
  current_atomic = (yyvsp[-2].u.atomic);
}
#line 1912 "built/tmp/dcParser.yxx.c"
    break;

  case 64: /* atomic_element: parameter_with_default  */
#line 521 "direct/src/dcparser/dcParser.yxx"
{
  if ((yyvsp[0].u.parameter) != nullptr) {
    current_atomic->add_element((yyvsp[0].u.parameter));
  }
}
#line 1922 "built/tmp/dcParser.yxx.c"
    break;

  case 65: /* $@5: %empty  */
#line 530 "direct/src/dcparser/dcParser.yxx"
{
  current_parameter = (yyvsp[0].u.parameter);
}
#line 1930 "built/tmp/dcParser.yxx.c"
    break;

  case 66: /* named_parameter: type_definition $@5 parameter_definition  */
#line 534 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.parameter) = (yyvsp[0].u.parameter);
}
#line 1938 "built/tmp/dcParser.yxx.c"
    break;

  case 69: /* $@6: %empty  */
#line 546 "direct/src/dcparser/dcParser.yxx"
{
  current_packer = &default_packer;
  current_packer->clear_data();
  if ((yyvsp[-1].u.parameter) != nullptr) {
    current_packer->begin_pack((yyvsp[-1].u.parameter));
  }
}
#line 1950 "built/tmp/dcParser.yxx.c"
    break;

  case 70: /* named_parameter_with_default: named_parameter '=' $@6 parameter_value  */
#line 554 "direct/src/dcparser/dcParser.yxx"
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
#line 1972 "built/tmp/dcParser.yxx.c"
    break;

  case 72: /* $@7: %empty  */
#line 576 "direct/src/dcparser/dcParser.yxx"
{
  current_packer = &default_packer;
  current_packer->clear_data();
  if ((yyvsp[-1].u.parameter) != nullptr) {
    current_packer->begin_pack((yyvsp[-1].u.parameter));
  }
}
#line 1984 "built/tmp/dcParser.yxx.c"
    break;

  case 73: /* unnamed_parameter_with_default: unnamed_parameter '=' $@7 parameter_value  */
#line 584 "direct/src/dcparser/dcParser.yxx"
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
#line 2006 "built/tmp/dcParser.yxx.c"
    break;

  case 78: /* parameter_or_atomic: parameter  */
#line 615 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.field) = (yyvsp[0].u.parameter);
}
#line 2014 "built/tmp/dcParser.yxx.c"
    break;

  case 79: /* parameter_or_atomic: atomic_field  */
#line 619 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.field) = (yyvsp[0].u.field);
}
#line 2022 "built/tmp/dcParser.yxx.c"
    break;

  case 80: /* parameter_description: atomic_field no_keyword_list  */
#line 626 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.field) = (yyvsp[-1].u.field);
}
#line 2030 "built/tmp/dcParser.yxx.c"
    break;

  case 81: /* parameter_description: unnamed_parameter_with_default no_keyword_list  */
#line 630 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.field) = (yyvsp[-1].u.parameter);
}
#line 2038 "built/tmp/dcParser.yxx.c"
    break;

  case 82: /* parameter_description: named_parameter_with_default no_keyword_list  */
#line 634 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.field) = (yyvsp[-1].u.parameter);
}
#line 2046 "built/tmp/dcParser.yxx.c"
    break;

  case 83: /* simple_type_name: type_token  */
#line 641 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.parameter) = new DCSimpleParameter((yyvsp[0].u.subatomic));
}
#line 2054 "built/tmp/dcParser.yxx.c"
    break;

  case 84: /* simple_type_name: simple_type_name '(' double_range ')'  */
#line 645 "direct/src/dcparser/dcParser.yxx"
{
  DCSimpleParameter *simple_param = (yyvsp[-3].u.parameter)->as_simple_parameter();
  nassertr(simple_param != nullptr, 0);
  if (!simple_param->set_range(double_range)) {
    yyerror("Inappropriate range for type");
  }
  (yyval.u.parameter) = simple_param;
}
#line 2067 "built/tmp/dcParser.yxx.c"
    break;

  case 85: /* simple_type_name: simple_type_name '/' small_unsigned_integer  */
#line 654 "direct/src/dcparser/dcParser.yxx"
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
#line 2087 "built/tmp/dcParser.yxx.c"
    break;

  case 86: /* simple_type_name: simple_type_name '%' number  */
#line 670 "direct/src/dcparser/dcParser.yxx"
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
#line 2103 "built/tmp/dcParser.yxx.c"
    break;

  case 88: /* type_name: IDENTIFIER  */
#line 686 "direct/src/dcparser/dcParser.yxx"
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
#line 2139 "built/tmp/dcParser.yxx.c"
    break;

  case 89: /* type_name: struct  */
#line 718 "direct/src/dcparser/dcParser.yxx"
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
#line 2159 "built/tmp/dcParser.yxx.c"
    break;

  case 90: /* type_name: switch  */
#line 734 "direct/src/dcparser/dcParser.yxx"
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
#line 2179 "built/tmp/dcParser.yxx.c"
    break;

  case 91: /* double_range: empty  */
#line 753 "direct/src/dcparser/dcParser.yxx"
{
  double_range.clear();
}
#line 2187 "built/tmp/dcParser.yxx.c"
    break;

  case 92: /* double_range: char_or_number  */
#line 757 "direct/src/dcparser/dcParser.yxx"
{
  double_range.clear();
  if (!double_range.add_range((yyvsp[0].u.real), (yyvsp[0].u.real))) {
    yyerror("Overlapping range");
  }
}
#line 2198 "built/tmp/dcParser.yxx.c"
    break;

  case 93: /* double_range: char_or_number '-' char_or_number  */
#line 764 "direct/src/dcparser/dcParser.yxx"
{
  double_range.clear();
  if (!double_range.add_range((yyvsp[-2].u.real), (yyvsp[0].u.real))) {
    yyerror("Overlapping range");
  }
}
#line 2209 "built/tmp/dcParser.yxx.c"
    break;

  case 94: /* double_range: char_or_number number  */
#line 771 "direct/src/dcparser/dcParser.yxx"
{
  double_range.clear();
  if ((yyvsp[0].u.real) >= 0) {
    yyerror("Syntax error");
  } else if (!double_range.add_range((yyvsp[-1].u.real), -(yyvsp[0].u.real))) {
    yyerror("Overlapping range");
  }
}
#line 2222 "built/tmp/dcParser.yxx.c"
    break;

  case 95: /* double_range: double_range ',' char_or_number  */
#line 780 "direct/src/dcparser/dcParser.yxx"
{
  if (!double_range.add_range((yyvsp[0].u.real), (yyvsp[0].u.real))) {
    yyerror("Overlapping range");
  }
}
#line 2232 "built/tmp/dcParser.yxx.c"
    break;

  case 96: /* double_range: double_range ',' char_or_number '-' char_or_number  */
#line 786 "direct/src/dcparser/dcParser.yxx"
{
  if (!double_range.add_range((yyvsp[-2].u.real), (yyvsp[0].u.real))) {
    yyerror("Overlapping range");
  }
}
#line 2242 "built/tmp/dcParser.yxx.c"
    break;

  case 97: /* double_range: double_range ',' char_or_number number  */
#line 792 "direct/src/dcparser/dcParser.yxx"
{
  if ((yyvsp[0].u.real) >= 0) {
    yyerror("Syntax error");
  } else if (!double_range.add_range((yyvsp[-1].u.real), -(yyvsp[0].u.real))) {
    yyerror("Overlapping range");
  }
}
#line 2254 "built/tmp/dcParser.yxx.c"
    break;

  case 98: /* uint_range: empty  */
#line 803 "direct/src/dcparser/dcParser.yxx"
{
  uint_range.clear();
}
#line 2262 "built/tmp/dcParser.yxx.c"
    break;

  case 99: /* uint_range: char_or_uint  */
#line 807 "direct/src/dcparser/dcParser.yxx"
{
  uint_range.clear();
  if (!uint_range.add_range((yyvsp[0].u.s_uint), (yyvsp[0].u.s_uint))) {
    yyerror("Overlapping range");
  }
}
#line 2273 "built/tmp/dcParser.yxx.c"
    break;

  case 100: /* uint_range: char_or_uint '-' char_or_uint  */
#line 814 "direct/src/dcparser/dcParser.yxx"
{
  uint_range.clear();
  if (!uint_range.add_range((yyvsp[-2].u.s_uint), (yyvsp[0].u.s_uint))) {
    yyerror("Overlapping range");
  }
}
#line 2284 "built/tmp/dcParser.yxx.c"
    break;

  case 101: /* uint_range: char_or_uint small_negative_integer  */
#line 821 "direct/src/dcparser/dcParser.yxx"
{
  uint_range.clear();
  if (!uint_range.add_range((yyvsp[-1].u.s_uint), (yyvsp[0].u.s_uint))) {
    yyerror("Overlapping range");
  }
}
#line 2295 "built/tmp/dcParser.yxx.c"
    break;

  case 102: /* uint_range: uint_range ',' char_or_uint  */
#line 828 "direct/src/dcparser/dcParser.yxx"
{
  if (!uint_range.add_range((yyvsp[0].u.s_uint), (yyvsp[0].u.s_uint))) {
    yyerror("Overlapping range");
  }
}
#line 2305 "built/tmp/dcParser.yxx.c"
    break;

  case 103: /* uint_range: uint_range ',' char_or_uint '-' char_or_uint  */
#line 834 "direct/src/dcparser/dcParser.yxx"
{
  if (!uint_range.add_range((yyvsp[-2].u.s_uint), (yyvsp[0].u.s_uint))) {
    yyerror("Overlapping range");
  }
}
#line 2315 "built/tmp/dcParser.yxx.c"
    break;

  case 104: /* uint_range: uint_range ',' char_or_uint small_negative_integer  */
#line 840 "direct/src/dcparser/dcParser.yxx"
{
  if (!uint_range.add_range((yyvsp[-1].u.s_uint), (yyvsp[0].u.s_uint))) {
    yyerror("Overlapping range");
  }
}
#line 2325 "built/tmp/dcParser.yxx.c"
    break;

  case 106: /* type_definition: type_definition '[' uint_range ']'  */
#line 850 "direct/src/dcparser/dcParser.yxx"
{
  if ((yyvsp[-3].u.parameter) == nullptr) {
    (yyval.u.parameter) = nullptr;
  } else {
    (yyval.u.parameter) = (yyvsp[-3].u.parameter)->append_array_specification(uint_range);
  }
}
#line 2337 "built/tmp/dcParser.yxx.c"
    break;

  case 107: /* parameter_definition: IDENTIFIER  */
#line 861 "direct/src/dcparser/dcParser.yxx"
{
  current_parameter->set_name((yyvsp[0].str));
  (yyval.u.parameter) = current_parameter;
}
#line 2346 "built/tmp/dcParser.yxx.c"
    break;

  case 108: /* parameter_definition: parameter_definition '/' small_unsigned_integer  */
#line 866 "direct/src/dcparser/dcParser.yxx"
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
#line 2365 "built/tmp/dcParser.yxx.c"
    break;

  case 109: /* parameter_definition: parameter_definition '%' number  */
#line 881 "direct/src/dcparser/dcParser.yxx"
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
#line 2384 "built/tmp/dcParser.yxx.c"
    break;

  case 110: /* parameter_definition: parameter_definition '[' uint_range ']'  */
#line 896 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.parameter) = (yyvsp[-3].u.parameter)->append_array_specification(uint_range);
}
#line 2392 "built/tmp/dcParser.yxx.c"
    break;

  case 111: /* char_or_uint: STRING  */
#line 903 "direct/src/dcparser/dcParser.yxx"
{
  if ((yyvsp[0].str).length() != 1) {
    yyerror("Single character required.");
    (yyval.u.s_uint) = 0;
  } else {
    (yyval.u.s_uint) = (unsigned char)(yyvsp[0].str)[0];
  }
}
#line 2405 "built/tmp/dcParser.yxx.c"
    break;

  case 113: /* small_unsigned_integer: UNSIGNED_INTEGER  */
#line 916 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.s_uint) = (unsigned int)(yyvsp[0].u.uint64);
  if ((yyval.u.s_uint) != (yyvsp[0].u.uint64)) {
    yyerror("Number out of range.");
    (yyval.u.s_uint) = 1;
  }
}
#line 2417 "built/tmp/dcParser.yxx.c"
    break;

  case 114: /* small_negative_integer: SIGNED_INTEGER  */
#line 927 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.s_uint) = (unsigned int)-(yyvsp[0].u.int64);
  if ((yyvsp[0].u.int64) >= 0) {
    yyerror("Syntax error.");

  } else if ((yyval.u.s_uint) != -(yyvsp[0].u.int64)) {
    yyerror("Number out of range.");
    (yyval.u.s_uint) = 1;
  }
}
#line 2432 "built/tmp/dcParser.yxx.c"
    break;

  case 117: /* number: unsigned_integer  */
#line 949 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.real) = (double)(yyvsp[0].u.uint64);
}
#line 2440 "built/tmp/dcParser.yxx.c"
    break;

  case 118: /* number: signed_integer  */
#line 953 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.real) = (double)(yyvsp[0].u.int64);
}
#line 2448 "built/tmp/dcParser.yxx.c"
    break;

  case 120: /* char_or_number: STRING  */
#line 961 "direct/src/dcparser/dcParser.yxx"
{
  if ((yyvsp[0].str).length() != 1) {
    yyerror("Single character required.");
    (yyval.u.real) = 0;
  } else {
    (yyval.u.real) = (double)(unsigned char)(yyvsp[0].str)[0];
  }
}
#line 2461 "built/tmp/dcParser.yxx.c"
    break;

  case 122: /* parameter_value: parameter_actual_value  */
#line 975 "direct/src/dcparser/dcParser.yxx"
{
}
#line 2468 "built/tmp/dcParser.yxx.c"
    break;

  case 123: /* $@8: %empty  */
#line 978 "direct/src/dcparser/dcParser.yxx"
{
  if ((yyvsp[-1].str) != current_packer->get_current_field_name()) {
    ostringstream strm;
    strm << "Got '" << (yyvsp[-1].str) << "', expected '"
         << current_packer->get_current_field_name() << "'";
    yyerror(strm.str());
  }
}
#line 2481 "built/tmp/dcParser.yxx.c"
    break;

  case 124: /* parameter_value: IDENTIFIER '=' $@8 parameter_actual_value  */
#line 987 "direct/src/dcparser/dcParser.yxx"
{
}
#line 2488 "built/tmp/dcParser.yxx.c"
    break;

  case 125: /* parameter_actual_value: signed_integer  */
#line 993 "direct/src/dcparser/dcParser.yxx"
{
  current_packer->pack_int64((yyvsp[0].u.int64));
}
#line 2496 "built/tmp/dcParser.yxx.c"
    break;

  case 126: /* parameter_actual_value: unsigned_integer  */
#line 997 "direct/src/dcparser/dcParser.yxx"
{
  current_packer->pack_uint64((yyvsp[0].u.uint64));
}
#line 2504 "built/tmp/dcParser.yxx.c"
    break;

  case 127: /* parameter_actual_value: REAL  */
#line 1001 "direct/src/dcparser/dcParser.yxx"
{
  current_packer->pack_double((yyvsp[0].u.real));
}
#line 2512 "built/tmp/dcParser.yxx.c"
    break;

  case 128: /* parameter_actual_value: STRING  */
#line 1005 "direct/src/dcparser/dcParser.yxx"
{
  current_packer->pack_string((yyvsp[0].str));
}
#line 2520 "built/tmp/dcParser.yxx.c"
    break;

  case 129: /* parameter_actual_value: HEX_STRING  */
#line 1009 "direct/src/dcparser/dcParser.yxx"
{
  current_packer->pack_literal_value((yyvsp[0].bytes));
}
#line 2528 "built/tmp/dcParser.yxx.c"
    break;

  case 130: /* $@9: %empty  */
#line 1013 "direct/src/dcparser/dcParser.yxx"
{
  current_packer->push();
}
#line 2536 "built/tmp/dcParser.yxx.c"
    break;

  case 131: /* parameter_actual_value: '{' $@9 array '}'  */
#line 1017 "direct/src/dcparser/dcParser.yxx"
{
  current_packer->pop();
}
#line 2544 "built/tmp/dcParser.yxx.c"
    break;

  case 132: /* $@10: %empty  */
#line 1021 "direct/src/dcparser/dcParser.yxx"
{
  current_packer->push();
}
#line 2552 "built/tmp/dcParser.yxx.c"
    break;

  case 133: /* parameter_actual_value: '[' $@10 array ']'  */
#line 1025 "direct/src/dcparser/dcParser.yxx"
{
  current_packer->pop();
}
#line 2560 "built/tmp/dcParser.yxx.c"
    break;

  case 134: /* $@11: %empty  */
#line 1029 "direct/src/dcparser/dcParser.yxx"
{
  current_packer->push();
}
#line 2568 "built/tmp/dcParser.yxx.c"
    break;

  case 135: /* parameter_actual_value: '(' $@11 array ')'  */
#line 1033 "direct/src/dcparser/dcParser.yxx"
{
  current_packer->pop();
}
#line 2576 "built/tmp/dcParser.yxx.c"
    break;

  case 136: /* parameter_actual_value: signed_integer '*' small_unsigned_integer  */
#line 1037 "direct/src/dcparser/dcParser.yxx"
{
  for (unsigned int i = 0; i < (yyvsp[0].u.s_uint); i++) {
    current_packer->pack_int64((yyvsp[-2].u.int64));
  }
}
#line 2586 "built/tmp/dcParser.yxx.c"
    break;

  case 137: /* parameter_actual_value: unsigned_integer '*' small_unsigned_integer  */
#line 1043 "direct/src/dcparser/dcParser.yxx"
{
  for (unsigned int i = 0; i < (yyvsp[0].u.s_uint); i++) {
    current_packer->pack_uint64((yyvsp[-2].u.uint64));
  }
}
#line 2596 "built/tmp/dcParser.yxx.c"
    break;

  case 138: /* parameter_actual_value: REAL '*' small_unsigned_integer  */
#line 1049 "direct/src/dcparser/dcParser.yxx"
{
  for (unsigned int i = 0; i < (yyvsp[0].u.s_uint); i++) {
    current_packer->pack_double((yyvsp[-2].u.real));
  }
}
#line 2606 "built/tmp/dcParser.yxx.c"
    break;

  case 139: /* parameter_actual_value: HEX_STRING '*' small_unsigned_integer  */
#line 1055 "direct/src/dcparser/dcParser.yxx"
{
  for (unsigned int i = 0; i < (yyvsp[0].u.s_uint); i++) {
    current_packer->pack_literal_value((yyvsp[-2].bytes));
  }
}
#line 2616 "built/tmp/dcParser.yxx.c"
    break;

  case 146: /* type_token: KW_INT8  */
#line 1079 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.subatomic) = ST_int8;
}
#line 2624 "built/tmp/dcParser.yxx.c"
    break;

  case 147: /* type_token: KW_INT16  */
#line 1083 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.subatomic) = ST_int16;
}
#line 2632 "built/tmp/dcParser.yxx.c"
    break;

  case 148: /* type_token: KW_INT32  */
#line 1087 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.subatomic) = ST_int32;
}
#line 2640 "built/tmp/dcParser.yxx.c"
    break;

  case 149: /* type_token: KW_INT64  */
#line 1091 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.subatomic) = ST_int64;
}
#line 2648 "built/tmp/dcParser.yxx.c"
    break;

  case 150: /* type_token: KW_UINT8  */
#line 1095 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.subatomic) = ST_uint8;
}
#line 2656 "built/tmp/dcParser.yxx.c"
    break;

  case 151: /* type_token: KW_UINT16  */
#line 1099 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.subatomic) = ST_uint16;
}
#line 2664 "built/tmp/dcParser.yxx.c"
    break;

  case 152: /* type_token: KW_UINT32  */
#line 1103 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.subatomic) = ST_uint32;
}
#line 2672 "built/tmp/dcParser.yxx.c"
    break;

  case 153: /* type_token: KW_UINT64  */
#line 1107 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.subatomic) = ST_uint64;
}
#line 2680 "built/tmp/dcParser.yxx.c"
    break;

  case 154: /* type_token: KW_FLOAT64  */
#line 1111 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.subatomic) = ST_float64;
}
#line 2688 "built/tmp/dcParser.yxx.c"
    break;

  case 155: /* type_token: KW_STRING  */
#line 1115 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.subatomic) = ST_string;
}
#line 2696 "built/tmp/dcParser.yxx.c"
    break;

  case 156: /* type_token: KW_BLOB  */
#line 1119 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.subatomic) = ST_blob;
}
#line 2704 "built/tmp/dcParser.yxx.c"
    break;

  case 157: /* type_token: KW_BLOB32  */
#line 1123 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.subatomic) = ST_blob32;
}
#line 2712 "built/tmp/dcParser.yxx.c"
    break;

  case 158: /* type_token: KW_INT8ARRAY  */
#line 1127 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.subatomic) = ST_int8array;
}
#line 2720 "built/tmp/dcParser.yxx.c"
    break;

  case 159: /* type_token: KW_INT16ARRAY  */
#line 1131 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.subatomic) = ST_int16array;
}
#line 2728 "built/tmp/dcParser.yxx.c"
    break;

  case 160: /* type_token: KW_INT32ARRAY  */
#line 1135 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.subatomic) = ST_int32array;
}
#line 2736 "built/tmp/dcParser.yxx.c"
    break;

  case 161: /* type_token: KW_UINT8ARRAY  */
#line 1139 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.subatomic) = ST_uint8array;
}
#line 2744 "built/tmp/dcParser.yxx.c"
    break;

  case 162: /* type_token: KW_UINT16ARRAY  */
#line 1143 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.subatomic) = ST_uint16array;
}
#line 2752 "built/tmp/dcParser.yxx.c"
    break;

  case 163: /* type_token: KW_UINT32ARRAY  */
#line 1147 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.subatomic) = ST_uint32array;
}
#line 2760 "built/tmp/dcParser.yxx.c"
    break;

  case 164: /* type_token: KW_UINT32UINT8ARRAY  */
#line 1151 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.subatomic) = ST_uint32uint8array;
}
#line 2768 "built/tmp/dcParser.yxx.c"
    break;

  case 165: /* type_token: KW_CHAR  */
#line 1155 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.subatomic) = ST_char;
}
#line 2776 "built/tmp/dcParser.yxx.c"
    break;

  case 166: /* keyword_list: empty  */
#line 1162 "direct/src/dcparser/dcParser.yxx"
{
  current_keyword_list.clear_keywords();
}
#line 2784 "built/tmp/dcParser.yxx.c"
    break;

  case 167: /* keyword_list: keyword_list KEYWORD  */
#line 1166 "direct/src/dcparser/dcParser.yxx"
{
  current_keyword_list.add_keyword((yyvsp[0].u.keyword));
}
#line 2792 "built/tmp/dcParser.yxx.c"
    break;

  case 168: /* no_keyword_list: keyword_list  */
#line 1173 "direct/src/dcparser/dcParser.yxx"
{
  if (current_keyword_list.get_num_keywords() != 0) {
    yyerror("Communication keywords are not allowed here.");
  }
}
#line 2802 "built/tmp/dcParser.yxx.c"
    break;

  case 169: /* $@12: %empty  */
#line 1182 "direct/src/dcparser/dcParser.yxx"
{
  current_molecular = new DCMolecularField((yyvsp[-1].str), current_class);
}
#line 2810 "built/tmp/dcParser.yxx.c"
    break;

  case 170: /* molecular_field: IDENTIFIER ':' $@12 molecular_atom_list  */
#line 1186 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.field) = current_molecular;
}
#line 2818 "built/tmp/dcParser.yxx.c"
    break;

  case 171: /* atomic_name: IDENTIFIER  */
#line 1193 "direct/src/dcparser/dcParser.yxx"
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
#line 2846 "built/tmp/dcParser.yxx.c"
    break;

  case 172: /* molecular_atom_list: atomic_name  */
#line 1220 "direct/src/dcparser/dcParser.yxx"
{
  if ((yyvsp[0].u.atomic) != nullptr) {
    current_molecular->add_atomic((yyvsp[0].u.atomic));
  }
}
#line 2856 "built/tmp/dcParser.yxx.c"
    break;

  case 173: /* molecular_atom_list: molecular_atom_list ',' atomic_name  */
#line 1226 "direct/src/dcparser/dcParser.yxx"
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
#line 2871 "built/tmp/dcParser.yxx.c"
    break;

  case 174: /* optional_name: empty  */
#line 1240 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.str) = "";
}
#line 2879 "built/tmp/dcParser.yxx.c"
    break;

  case 176: /* @13: %empty  */
#line 1248 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.dswitch) = current_switch;
  current_switch = new DCSwitch((yyvsp[-4].str), (yyvsp[-2].u.field));
}
#line 2888 "built/tmp/dcParser.yxx.c"
    break;

  case 177: /* switch: KW_SWITCH optional_name '(' parameter_or_atomic ')' '{' @13 switch_fields '}'  */
#line 1253 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.dswitch) = current_switch;
  current_switch = (yyvsp[-2].u.dswitch);
}
#line 2897 "built/tmp/dcParser.yxx.c"
    break;

  case 183: /* switch_fields: switch_fields switch_field ';'  */
#line 1267 "direct/src/dcparser/dcParser.yxx"
{
  if (!current_switch->is_field_valid()) {
    yyerror("case declaration required before first element");
  } else if ((yyvsp[-1].u.field) != nullptr) {
    if (!current_switch->add_field((yyvsp[-1].u.field))) {
      yyerror("Duplicate field name: " + (yyvsp[-1].u.field)->get_name());
    }
  }
}
#line 2911 "built/tmp/dcParser.yxx.c"
    break;

  case 184: /* $@14: %empty  */
#line 1280 "direct/src/dcparser/dcParser.yxx"
{
  current_packer = &default_packer;
  current_packer->clear_data();
  current_packer->begin_pack(current_switch->get_key_parameter());
}
#line 2921 "built/tmp/dcParser.yxx.c"
    break;

  case 185: /* switch_case: KW_CASE $@14 parameter_value ':'  */
#line 1286 "direct/src/dcparser/dcParser.yxx"
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
#line 2937 "built/tmp/dcParser.yxx.c"
    break;

  case 186: /* switch_default: KW_DEFAULT ':'  */
#line 1301 "direct/src/dcparser/dcParser.yxx"
{
  if (!current_switch->add_default()) {
    yyerror("Default case already defined");
  }
}
#line 2947 "built/tmp/dcParser.yxx.c"
    break;

  case 187: /* switch_break: KW_BREAK  */
#line 1310 "direct/src/dcparser/dcParser.yxx"
{
  current_switch->add_break();
}
#line 2955 "built/tmp/dcParser.yxx.c"
    break;

  case 188: /* switch_field: unnamed_parameter_with_default  */
#line 1317 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.field) = (yyvsp[0].u.parameter);
}
#line 2963 "built/tmp/dcParser.yxx.c"
    break;

  case 189: /* switch_field: named_parameter_with_default  */
#line 1321 "direct/src/dcparser/dcParser.yxx"
{
  (yyval.u.field) = (yyvsp[0].u.parameter);
}
#line 2971 "built/tmp/dcParser.yxx.c"
    break;


#line 2975 "built/tmp/dcParser.yxx.c"

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


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
#define yyparse         vrmlyyparse
#define yylex           vrmlyylex
#define yyerror         vrmlyyerror
#define yydebug         vrmlyydebug
#define yynerrs         vrmlyynerrs
#define yylval          vrmlyylval
#define yychar          vrmlyychar

/* First part of user prologue.  */
#line 22 "pandatool/src/vrml/vrmlParser.yxx"


//
// Parser for VRML 2.0 files.
// This is a minimal parser that does NOT generate an in-memory scene graph.
// 

// The original parser was developed on a Windows 95 PC with
// Borland's C++ 5.0 development tools.  This was then ported
// to a Windows 95 PC with Microsoft's MSDEV C++ 4.0 development
// tools.  The port introduced the ifdef's for
//    USING_BORLAND_CPP_5          : since this provides a "std namespace",
//    TWO_ARGUMENTS_FOR_STL_STACK  : STL is a moving target.  The stack template
//                                     class takes either one or two arguments.

#include "pandatoolbase.h"
#include "vrmlLexerDefs.h"
#include "vrmlNodeType.h"
#include "vrmlNode.h"
#include "pnotify.h"
#include "plist.h"

#include <stack>
#include <stdio.h>  // for sprintf()

//#define YYDEBUG 1

// Currently-being-define proto.  Prototypes may be nested, so a stack
// is needed:

std::stack<VrmlNodeType*, plist<VrmlNodeType*> > currentProtoStack;

// This is used to keep track of which field in which type of node is being
// parsed.  Field are nested (nodes are contained inside MFNode/SFNode fields)
// so a stack of these is needed:
typedef struct {
    const VrmlNodeType *nodeType;
    const char *fieldName;
    const VrmlNodeType::NameTypeRec *typeRec;
} FieldRec;

std::stack<FieldRec*, plist<FieldRec*> > currentField;

// Similarly for the node entries (which contain the actual values for
// the fields as they are encountered):

std::stack<VrmlNode*, plist<VrmlNode*> > currentNode;

// This is used when the parser knows what kind of token it expects
// to get next-- used when parsing field values (whose types are declared
// and read by the parser) and at certain other places:
extern int expectToken;

// This is where we store the parsed scene.
VrmlScene *parsed_scene = nullptr;

// Some helper routines defined below:
static void beginProto(const char *);
static void endProto();
int addField(const char *type, const char *name, const VrmlFieldValue *dflt = nullptr);
int addEventIn(const char *type, const char *name, const VrmlFieldValue *dflt = nullptr);
int addEventOut(const char *type, const char *name, const VrmlFieldValue *dflt = nullptr);
int addExposedField(const char *type, const char *name, const VrmlFieldValue *dflt = nullptr);
int add(void (VrmlNodeType::*func)(const char *, int, const VrmlFieldValue *), 
        const char *typeString, const char *name,
        const VrmlFieldValue *dflt);
int fieldType(const char *type);
void enterNode(const char *);
VrmlNode *exitNode();
void inScript();
void enterField(const char *);
void storeField(const VrmlFieldValue &value);
void exitField();
void expect(int type);

extern void vrmlyyerror(const std::string &);

////////////////////////////////////////////////////////////////////
// Defining the interface to the parser.
////////////////////////////////////////////////////////////////////

void
vrml_init_parser(std::istream &in, const std::string &filename) {
  //yydebug = 0;
  vrml_init_lexer(in, filename);
}

void
vrml_cleanup_parser() {
}


#line 171 "built/tmp/vrmlParser.c"

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

#include "vrmlParser.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_IDENTIFIER = 3,                 /* IDENTIFIER  */
  YYSYMBOL_DEF = 4,                        /* DEF  */
  YYSYMBOL_USE = 5,                        /* USE  */
  YYSYMBOL_PROTO = 6,                      /* PROTO  */
  YYSYMBOL_EXTERNPROTO = 7,                /* EXTERNPROTO  */
  YYSYMBOL_TO = 8,                         /* TO  */
  YYSYMBOL_IS = 9,                         /* IS  */
  YYSYMBOL_ROUTE = 10,                     /* ROUTE  */
  YYSYMBOL_SFN_NULL = 11,                  /* SFN_NULL  */
  YYSYMBOL_EVENTIN = 12,                   /* EVENTIN  */
  YYSYMBOL_EVENTOUT = 13,                  /* EVENTOUT  */
  YYSYMBOL_FIELD = 14,                     /* FIELD  */
  YYSYMBOL_EXPOSEDFIELD = 15,              /* EXPOSEDFIELD  */
  YYSYMBOL_SFBOOL = 16,                    /* SFBOOL  */
  YYSYMBOL_SFCOLOR = 17,                   /* SFCOLOR  */
  YYSYMBOL_SFFLOAT = 18,                   /* SFFLOAT  */
  YYSYMBOL_SFIMAGE = 19,                   /* SFIMAGE  */
  YYSYMBOL_SFINT32 = 20,                   /* SFINT32  */
  YYSYMBOL_SFNODE = 21,                    /* SFNODE  */
  YYSYMBOL_SFROTATION = 22,                /* SFROTATION  */
  YYSYMBOL_SFSTRING = 23,                  /* SFSTRING  */
  YYSYMBOL_SFTIME = 24,                    /* SFTIME  */
  YYSYMBOL_SFVEC2F = 25,                   /* SFVEC2F  */
  YYSYMBOL_SFVEC3F = 26,                   /* SFVEC3F  */
  YYSYMBOL_MFCOLOR = 27,                   /* MFCOLOR  */
  YYSYMBOL_MFFLOAT = 28,                   /* MFFLOAT  */
  YYSYMBOL_MFINT32 = 29,                   /* MFINT32  */
  YYSYMBOL_MFROTATION = 30,                /* MFROTATION  */
  YYSYMBOL_MFSTRING = 31,                  /* MFSTRING  */
  YYSYMBOL_MFVEC2F = 32,                   /* MFVEC2F  */
  YYSYMBOL_MFVEC3F = 33,                   /* MFVEC3F  */
  YYSYMBOL_MFNODE = 34,                    /* MFNODE  */
  YYSYMBOL_35_ = 35,                       /* '['  */
  YYSYMBOL_36_ = 36,                       /* ']'  */
  YYSYMBOL_37_ = 37,                       /* '{'  */
  YYSYMBOL_38_ = 38,                       /* '}'  */
  YYSYMBOL_39_ = 39,                       /* '.'  */
  YYSYMBOL_YYACCEPT = 40,                  /* $accept  */
  YYSYMBOL_vrmlscene = 41,                 /* vrmlscene  */
  YYSYMBOL_declarations = 42,              /* declarations  */
  YYSYMBOL_nodeDeclaration = 43,           /* nodeDeclaration  */
  YYSYMBOL_protoDeclaration = 44,          /* protoDeclaration  */
  YYSYMBOL_proto = 45,                     /* proto  */
  YYSYMBOL_46_1 = 46,                      /* $@1  */
  YYSYMBOL_externproto = 47,               /* externproto  */
  YYSYMBOL_48_2 = 48,                      /* $@2  */
  YYSYMBOL_49_3 = 49,                      /* $@3  */
  YYSYMBOL_interfaceDeclarations = 50,     /* interfaceDeclarations  */
  YYSYMBOL_interfaceDeclaration = 51,      /* interfaceDeclaration  */
  YYSYMBOL_52_4 = 52,                      /* $@4  */
  YYSYMBOL_53_5 = 53,                      /* $@5  */
  YYSYMBOL_externInterfaceDeclarations = 54, /* externInterfaceDeclarations  */
  YYSYMBOL_externInterfaceDeclaration = 55, /* externInterfaceDeclaration  */
  YYSYMBOL_routeDeclaration = 56,          /* routeDeclaration  */
  YYSYMBOL_node = 57,                      /* node  */
  YYSYMBOL_58_6 = 58,                      /* $@6  */
  YYSYMBOL_nodeGuts = 59,                  /* nodeGuts  */
  YYSYMBOL_nodeGut = 60,                   /* nodeGut  */
  YYSYMBOL_61_7 = 61,                      /* $@7  */
  YYSYMBOL_62_8 = 62,                      /* $@8  */
  YYSYMBOL_fieldValue = 63,                /* fieldValue  */
  YYSYMBOL_mfnodeValue = 64,               /* mfnodeValue  */
  YYSYMBOL_nodes = 65                      /* nodes  */
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
#define YYLAST   126

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  40
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  26
/* YYNRULES -- Number of rules.  */
#define YYNRULES  70
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  125

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   289


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
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,    39,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    35,     2,    36,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    37,     2,    38,     2,     2,     2,     2,
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
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   146,   146,   154,   157,   164,   165,   169,   175,   181,
     190,   191,   195,   195,   201,   203,   201,   206,   208,   212,
     214,   217,   216,   228,   227,   240,   242,   246,   248,   250,
     252,   257,   262,   262,   266,   268,   272,   272,   279,   280,
     283,   284,   285,   285,   289,   291,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   314,   315,   321,   322,   326,   330,   341,
     344
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
  "\"end of file\"", "error", "\"invalid token\"", "IDENTIFIER", "DEF",
  "USE", "PROTO", "EXTERNPROTO", "TO", "IS", "ROUTE", "SFN_NULL",
  "EVENTIN", "EVENTOUT", "FIELD", "EXPOSEDFIELD", "SFBOOL", "SFCOLOR",
  "SFFLOAT", "SFIMAGE", "SFINT32", "SFNODE", "SFROTATION", "SFSTRING",
  "SFTIME", "SFVEC2F", "SFVEC3F", "MFCOLOR", "MFFLOAT", "MFINT32",
  "MFROTATION", "MFSTRING", "MFVEC2F", "MFVEC3F", "MFNODE", "'['", "']'",
  "'{'", "'}'", "'.'", "$accept", "vrmlscene", "declarations",
  "nodeDeclaration", "protoDeclaration", "proto", "$@1", "externproto",
  "$@2", "$@3", "interfaceDeclarations", "interfaceDeclaration", "$@4",
  "$@5", "externInterfaceDeclarations", "externInterfaceDeclaration",
  "routeDeclaration", "node", "$@6", "nodeGuts", "nodeGut", "$@7", "$@8",
  "fieldValue", "mfnodeValue", "nodes", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-73)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
     -73,     2,    79,   -73,   -73,     0,     3,     4,     6,    11,
     -73,   -73,   -73,   -73,   -73,   -73,   -16,    26,   -73,   -73,
     -73,    -4,   -73,   -73,     5,     7,    38,    -2,   -73,   -73,
      35,   -73,    41,    45,    48,   -73,   -73,   -73,   -73,    19,
      66,    50,    43,    51,    54,    84,    85,    87,    88,    89,
      56,   -73,    91,    92,    93,    94,   -73,   -73,    59,    96,
     -73,   -73,   -73,   -73,   -73,    34,   -73,   -73,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,    23,   -73,
      95,    97,   -73,    98,   100,   102,   104,   -73,   105,   106,
     107,   108,    43,   109,   -73,   -73,   -73,   -73,   -73,   -73,
     110,   111,    43,   -73,   -73,   -73,   -73,    12,   -73,   -73,
     -73,   -73,   -73,   -73,    20,   -73,   -73,   -73,    43,    43,
     -73,   -73,   -73,   -73,   -73
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       3,     0,     2,     1,    32,     0,     0,     0,     0,     0,
       4,     5,    10,    11,     6,     7,     0,     0,     9,    12,
      14,     0,    34,     8,     0,     0,     0,     0,    17,    25,
       0,    36,     0,     0,     0,    33,    39,    38,    35,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    18,     0,     0,     0,     0,    15,    26,     0,     0,
      46,    47,    49,    51,    52,     0,    54,    56,    58,    59,
      61,    48,    50,    53,    55,    57,    60,    62,     0,    37,
      40,    41,    42,     0,     0,     0,     0,     3,     0,     0,
       0,     0,     0,     0,    66,    64,    63,    69,    68,    65,
       0,     0,     0,    19,    20,    21,    23,     0,    27,    28,
      29,    30,    16,    31,     0,    44,    45,    43,     0,     0,
      13,    67,    70,    22,    24
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -73,   -73,    13,   -65,    90,   -73,   -73,   -73,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,    99,   101,   -73,   -73,
     -73,   -73,   -73,   -72,   -73,   -73
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     1,     2,    10,    11,    12,    24,    13,    25,    92,
      39,    51,   118,   119,    40,    57,    14,    15,    16,    27,
      38,    42,   102,    79,    99,   114
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      96,    31,     3,    17,     7,     8,    18,    19,     9,    20,
      32,    33,    34,    98,    21,     4,     5,     6,     7,     8,
     112,    22,     9,     4,     5,     6,     4,     5,     6,     4,
     117,    46,    47,    48,    49,    26,    35,     4,     5,     6,
      28,    30,    29,    41,    43,    95,   123,   124,    44,   122,
     120,    45,    59,    58,    80,    50,   121,    81,    97,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    52,    53,
      54,    55,     4,     5,     6,     7,     8,    82,    83,     9,
      84,    85,    86,    87,    88,    89,    90,    91,    93,    94,
     107,   103,    56,   104,   100,   105,   101,   106,   108,   109,
     110,   111,   113,   115,   116,     0,     0,    36,    23,     0,
       0,     0,     0,     0,     0,     0,    37
};

static const yytype_int8 yycheck[] =
{
      65,     3,     0,     3,     6,     7,     3,     3,    10,     3,
      12,    13,    14,    78,     3,     3,     4,     5,     6,     7,
      92,    37,    10,     3,     4,     5,     3,     4,     5,     3,
     102,    12,    13,    14,    15,    39,    38,     3,     4,     5,
      35,     3,    35,     8,     3,    11,   118,   119,     3,   114,
      38,     3,     9,     3,     3,    36,    36,     3,    35,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    12,    13,
      14,    15,     3,     4,     5,     6,     7,     3,     3,    10,
       3,     3,     3,    37,     3,     3,     3,     3,    39,     3,
      87,     3,    36,     3,     9,     3,     9,     3,     3,     3,
       3,     3,     3,     3,     3,    -1,    -1,    27,    17,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    41,    42,     0,     3,     4,     5,     6,     7,    10,
      43,    44,    45,    47,    56,    57,    58,     3,     3,     3,
       3,     3,    37,    57,    46,    48,    39,    59,    35,    35,
       3,     3,    12,    13,    14,    38,    44,    56,    60,    50,
      54,     8,    61,     3,     3,     3,    12,    13,    14,    15,
      36,    51,    12,    13,    14,    15,    36,    55,     3,     9,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    63,
       3,     3,     3,     3,     3,     3,     3,    37,     3,     3,
       3,     3,    49,    39,     3,    11,    43,    35,    43,    64,
       9,     9,    62,     3,     3,     3,     3,    42,     3,     3,
       3,     3,    63,     3,    65,     3,     3,    63,    52,    53,
      38,    36,    43,    63,    63
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    40,    41,    42,    42,    42,    42,    43,    43,    43,
      44,    44,    46,    45,    48,    49,    47,    50,    50,    51,
      51,    52,    51,    53,    51,    54,    54,    55,    55,    55,
      55,    56,    58,    57,    59,    59,    61,    60,    60,    60,
      60,    60,    62,    60,    60,    60,    63,    63,    63,    63,
      63,    63,    63,    63,    63,    63,    63,    63,    63,    63,
      63,    63,    63,    63,    63,    63,    63,    64,    64,    65,
      65
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     0,     2,     2,     2,     1,     3,     2,
       1,     1,     0,     9,     0,     0,     8,     0,     2,     3,
       3,     0,     5,     0,     5,     0,     2,     3,     3,     3,
       3,     8,     0,     5,     0,     2,     0,     3,     1,     1,
       3,     3,     0,     5,     5,     5,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     2,     2,     2,     3,     1,     0,
       2
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
  case 2: /* vrmlscene: declarations  */
#line 147 "pandatool/src/vrml/vrmlParser.yxx"
{
  parsed_scene = (yyvsp[0].scene);
}
#line 1309 "built/tmp/vrmlParser.c"
    break;

  case 3: /* declarations: %empty  */
#line 154 "pandatool/src/vrml/vrmlParser.yxx"
{
  (yyval.scene) = new VrmlScene;
}
#line 1317 "built/tmp/vrmlParser.c"
    break;

  case 4: /* declarations: declarations nodeDeclaration  */
#line 158 "pandatool/src/vrml/vrmlParser.yxx"
{
  Declaration d;
  d._node = (yyvsp[0].nodeRef);
  (yyvsp[-1].scene)->push_back(d);
  (yyval.scene) = (yyvsp[-1].scene);
}
#line 1328 "built/tmp/vrmlParser.c"
    break;

  case 7: /* nodeDeclaration: node  */
#line 170 "pandatool/src/vrml/vrmlParser.yxx"
{
  (yyval.nodeRef)._p = (yyvsp[0].node);
  (yyval.nodeRef)._type = SFNodeRef::T_unnamed;
  (yyval.nodeRef)._name = nullptr;
}
#line 1338 "built/tmp/vrmlParser.c"
    break;

  case 8: /* nodeDeclaration: DEF IDENTIFIER node  */
#line 176 "pandatool/src/vrml/vrmlParser.yxx"
{
  (yyval.nodeRef)._p = (yyvsp[0].node);
  (yyval.nodeRef)._type = SFNodeRef::T_def;
  (yyval.nodeRef)._name = (yyvsp[-1].string);
}
#line 1348 "built/tmp/vrmlParser.c"
    break;

  case 9: /* nodeDeclaration: USE IDENTIFIER  */
#line 182 "pandatool/src/vrml/vrmlParser.yxx"
{
  (yyval.nodeRef)._p = nullptr;
  (yyval.nodeRef)._type = SFNodeRef::T_use;
  (yyval.nodeRef)._name = (yyvsp[0].string);
}
#line 1358 "built/tmp/vrmlParser.c"
    break;

  case 12: /* $@1: %empty  */
#line 195 "pandatool/src/vrml/vrmlParser.yxx"
                                    { beginProto((yyvsp[0].string)); }
#line 1364 "built/tmp/vrmlParser.c"
    break;

  case 13: /* proto: PROTO IDENTIFIER $@1 '[' interfaceDeclarations ']' '{' declarations '}'  */
#line 197 "pandatool/src/vrml/vrmlParser.yxx"
                                    { endProto();  free((yyvsp[-7].string));}
#line 1370 "built/tmp/vrmlParser.c"
    break;

  case 14: /* $@2: %empty  */
#line 201 "pandatool/src/vrml/vrmlParser.yxx"
                                        { beginProto((yyvsp[0].string)); }
#line 1376 "built/tmp/vrmlParser.c"
    break;

  case 15: /* $@3: %empty  */
#line 203 "pandatool/src/vrml/vrmlParser.yxx"
                                        { expect(MFSTRING); }
#line 1382 "built/tmp/vrmlParser.c"
    break;

  case 16: /* externproto: EXTERNPROTO IDENTIFIER $@2 '[' externInterfaceDeclarations ']' $@3 fieldValue  */
#line 204 "pandatool/src/vrml/vrmlParser.yxx"
                                        { endProto();  free((yyvsp[-6].string)); }
#line 1388 "built/tmp/vrmlParser.c"
    break;

  case 19: /* interfaceDeclaration: EVENTIN IDENTIFIER IDENTIFIER  */
#line 212 "pandatool/src/vrml/vrmlParser.yxx"
                                            { addEventIn((yyvsp[-1].string), (yyvsp[0].string));
                                              free((yyvsp[-1].string)); free((yyvsp[0].string)); }
#line 1395 "built/tmp/vrmlParser.c"
    break;

  case 20: /* interfaceDeclaration: EVENTOUT IDENTIFIER IDENTIFIER  */
#line 214 "pandatool/src/vrml/vrmlParser.yxx"
                                            { addEventOut((yyvsp[-1].string), (yyvsp[0].string));
                                              free((yyvsp[-1].string)); free((yyvsp[0].string)); }
#line 1402 "built/tmp/vrmlParser.c"
    break;

  case 21: /* $@4: %empty  */
#line 217 "pandatool/src/vrml/vrmlParser.yxx"
{
  int type = fieldType((yyvsp[-1].string));
  expect(type); 
}
#line 1411 "built/tmp/vrmlParser.c"
    break;

  case 22: /* interfaceDeclaration: FIELD IDENTIFIER IDENTIFIER $@4 fieldValue  */
#line 222 "pandatool/src/vrml/vrmlParser.yxx"
{
  addField((yyvsp[-3].string), (yyvsp[-2].string), &((yyvsp[0].fv)));
  free((yyvsp[-3].string)); 
  free((yyvsp[-2].string)); 
}
#line 1421 "built/tmp/vrmlParser.c"
    break;

  case 23: /* $@5: %empty  */
#line 228 "pandatool/src/vrml/vrmlParser.yxx"
{ 
  int type = fieldType((yyvsp[-1].string));
  expect(type); 
}
#line 1430 "built/tmp/vrmlParser.c"
    break;

  case 24: /* interfaceDeclaration: EXPOSEDFIELD IDENTIFIER IDENTIFIER $@5 fieldValue  */
#line 233 "pandatool/src/vrml/vrmlParser.yxx"
{ 
  addExposedField((yyvsp[-3].string), (yyvsp[-2].string), &((yyvsp[0].fv)));
  free((yyvsp[-3].string)); 
  free((yyvsp[-2].string)); 
}
#line 1440 "built/tmp/vrmlParser.c"
    break;

  case 27: /* externInterfaceDeclaration: EVENTIN IDENTIFIER IDENTIFIER  */
#line 246 "pandatool/src/vrml/vrmlParser.yxx"
                                            { addEventIn((yyvsp[-1].string), (yyvsp[0].string));
                                              free((yyvsp[-1].string)); free((yyvsp[0].string)); }
#line 1447 "built/tmp/vrmlParser.c"
    break;

  case 28: /* externInterfaceDeclaration: EVENTOUT IDENTIFIER IDENTIFIER  */
#line 248 "pandatool/src/vrml/vrmlParser.yxx"
                                            { addEventOut((yyvsp[-1].string), (yyvsp[0].string));
                                              free((yyvsp[-1].string)); free((yyvsp[0].string)); }
#line 1454 "built/tmp/vrmlParser.c"
    break;

  case 29: /* externInterfaceDeclaration: FIELD IDENTIFIER IDENTIFIER  */
#line 250 "pandatool/src/vrml/vrmlParser.yxx"
                                            { addField((yyvsp[-1].string), (yyvsp[0].string));
                                              free((yyvsp[-1].string)); free((yyvsp[0].string)); }
#line 1461 "built/tmp/vrmlParser.c"
    break;

  case 30: /* externInterfaceDeclaration: EXPOSEDFIELD IDENTIFIER IDENTIFIER  */
#line 252 "pandatool/src/vrml/vrmlParser.yxx"
                                            { addExposedField((yyvsp[-1].string), (yyvsp[0].string));
                                              free((yyvsp[-1].string)); free((yyvsp[0].string)); }
#line 1468 "built/tmp/vrmlParser.c"
    break;

  case 31: /* routeDeclaration: ROUTE IDENTIFIER '.' IDENTIFIER TO IDENTIFIER '.' IDENTIFIER  */
#line 258 "pandatool/src/vrml/vrmlParser.yxx"
                { free((yyvsp[-6].string)); free((yyvsp[-4].string)); free((yyvsp[-2].string)); free((yyvsp[0].string)); }
#line 1474 "built/tmp/vrmlParser.c"
    break;

  case 32: /* $@6: %empty  */
#line 262 "pandatool/src/vrml/vrmlParser.yxx"
                                    { enterNode((yyvsp[0].string)); }
#line 1480 "built/tmp/vrmlParser.c"
    break;

  case 33: /* node: IDENTIFIER $@6 '{' nodeGuts '}'  */
#line 263 "pandatool/src/vrml/vrmlParser.yxx"
                                    { (yyval.node) = exitNode(); free((yyvsp[-4].string));}
#line 1486 "built/tmp/vrmlParser.c"
    break;

  case 36: /* $@7: %empty  */
#line 272 "pandatool/src/vrml/vrmlParser.yxx"
                                    { enterField((yyvsp[0].string)); }
#line 1492 "built/tmp/vrmlParser.c"
    break;

  case 37: /* nodeGut: IDENTIFIER $@7 fieldValue  */
#line 274 "pandatool/src/vrml/vrmlParser.yxx"
{
  storeField((yyvsp[0].fv));
  exitField(); 
  free((yyvsp[-2].string)); 
}
#line 1502 "built/tmp/vrmlParser.c"
    break;

  case 40: /* nodeGut: EVENTIN IDENTIFIER IDENTIFIER  */
#line 283 "pandatool/src/vrml/vrmlParser.yxx"
                                            { inScript(); free((yyvsp[-1].string)); free((yyvsp[0].string)); }
#line 1508 "built/tmp/vrmlParser.c"
    break;

  case 41: /* nodeGut: EVENTOUT IDENTIFIER IDENTIFIER  */
#line 284 "pandatool/src/vrml/vrmlParser.yxx"
                                            { inScript(); free((yyvsp[-1].string)); free((yyvsp[0].string)); }
#line 1514 "built/tmp/vrmlParser.c"
    break;

  case 42: /* $@8: %empty  */
#line 285 "pandatool/src/vrml/vrmlParser.yxx"
                                            { inScript(); 
                                              int type = fieldType((yyvsp[-1].string));
                                              expect(type); }
#line 1522 "built/tmp/vrmlParser.c"
    break;

  case 43: /* nodeGut: FIELD IDENTIFIER IDENTIFIER $@8 fieldValue  */
#line 288 "pandatool/src/vrml/vrmlParser.yxx"
                                            { free((yyvsp[-3].string)); free((yyvsp[-2].string)); }
#line 1528 "built/tmp/vrmlParser.c"
    break;

  case 44: /* nodeGut: EVENTIN IDENTIFIER IDENTIFIER IS IDENTIFIER  */
#line 290 "pandatool/src/vrml/vrmlParser.yxx"
                { inScript(); free((yyvsp[-3].string)); free((yyvsp[-2].string)); free((yyvsp[0].string)); }
#line 1534 "built/tmp/vrmlParser.c"
    break;

  case 45: /* nodeGut: EVENTOUT IDENTIFIER IDENTIFIER IS IDENTIFIER  */
#line 292 "pandatool/src/vrml/vrmlParser.yxx"
                { inScript(); free((yyvsp[-3].string)); free((yyvsp[-2].string)); free((yyvsp[0].string)); }
#line 1540 "built/tmp/vrmlParser.c"
    break;

  case 63: /* fieldValue: SFNODE nodeDeclaration  */
#line 314 "pandatool/src/vrml/vrmlParser.yxx"
                               { (yyval.fv)._sfnode = (yyvsp[0].nodeRef); }
#line 1546 "built/tmp/vrmlParser.c"
    break;

  case 64: /* fieldValue: SFNODE SFN_NULL  */
#line 316 "pandatool/src/vrml/vrmlParser.yxx"
{ 
  (yyval.fv)._sfnode._p = nullptr;
  (yyval.fv)._sfnode._type = SFNodeRef::T_null;
  (yyval.fv)._sfnode._name = nullptr;
}
#line 1556 "built/tmp/vrmlParser.c"
    break;

  case 65: /* fieldValue: MFNODE mfnodeValue  */
#line 321 "pandatool/src/vrml/vrmlParser.yxx"
                           { (yyval.fv)._mf = (yyvsp[0].mfarray); }
#line 1562 "built/tmp/vrmlParser.c"
    break;

  case 66: /* fieldValue: IS IDENTIFIER  */
#line 322 "pandatool/src/vrml/vrmlParser.yxx"
                                    { free((yyvsp[0].string)); }
#line 1568 "built/tmp/vrmlParser.c"
    break;

  case 67: /* mfnodeValue: '[' nodes ']'  */
#line 327 "pandatool/src/vrml/vrmlParser.yxx"
{
  (yyval.mfarray) = (yyvsp[-1].mfarray); 
}
#line 1576 "built/tmp/vrmlParser.c"
    break;

  case 68: /* mfnodeValue: nodeDeclaration  */
#line 331 "pandatool/src/vrml/vrmlParser.yxx"
{
  (yyval.mfarray) = new MFArray;
  VrmlFieldValue v;
  v._sfnode = (yyvsp[0].nodeRef);
  (yyval.mfarray)->push_back(v);
}
#line 1587 "built/tmp/vrmlParser.c"
    break;

  case 69: /* nodes: %empty  */
#line 341 "pandatool/src/vrml/vrmlParser.yxx"
{
  (yyval.mfarray) = new MFArray;
}
#line 1595 "built/tmp/vrmlParser.c"
    break;

  case 70: /* nodes: nodes nodeDeclaration  */
#line 345 "pandatool/src/vrml/vrmlParser.yxx"
{
  VrmlFieldValue v;
  v._sfnode = (yyvsp[0].nodeRef);
  (yyvsp[-1].mfarray)->push_back(v);
  (yyval.mfarray) = (yyvsp[-1].mfarray);
}
#line 1606 "built/tmp/vrmlParser.c"
    break;


#line 1610 "built/tmp/vrmlParser.c"

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

#line 353 "pandatool/src/vrml/vrmlParser.yxx"


static void
beginProto(const char *protoName)
{
    // Any protos in the implementation are in a local namespace:
    VrmlNodeType::pushNameSpace();

    VrmlNodeType *t = new VrmlNodeType(protoName);
    currentProtoStack.push(t);
}

static void
endProto()
{
    // Make any protos defined in implementation unavailable:
    VrmlNodeType::popNameSpace();

    // Add this proto definition:
    if (currentProtoStack.empty()) {
        std::cerr << "Error: Empty PROTO stack!\n";
    }
    else {
        VrmlNodeType *t = currentProtoStack.top();
        currentProtoStack.pop();
        VrmlNodeType::addToNameSpace(t);
    }
}

int
addField(const char *type, const char *name,
         const VrmlFieldValue *dflt)
{
    return add(&VrmlNodeType::addField, type, name, dflt);
}

int
addEventIn(const char *type, const char *name,
           const VrmlFieldValue *dflt)
{
    return add(&VrmlNodeType::addEventIn, type, name, dflt);
}
int
addEventOut(const char *type, const char *name,
            const VrmlFieldValue *dflt)
{
  return add(&VrmlNodeType::addEventOut, type, name, dflt);
}
int
addExposedField(const char *type, const char *name,
                const VrmlFieldValue *dflt)
{
    return add(&VrmlNodeType::addExposedField, type, name, dflt);
}

int
add(void (VrmlNodeType::*func)(const char *, int, const VrmlFieldValue *), 
    const char *typeString, const char *name,
    const VrmlFieldValue *dflt)
{
    int type = fieldType(typeString);

    if (type == 0) {
        std::cerr << "Error: invalid field type: " << type << "\n";
    }

    // Need to add support for Script nodes:
    // if (inScript) ... ???

    if (currentProtoStack.empty()) {
        std::cerr << "Error: declaration outside of prototype\n";
        return 0;
    }
    VrmlNodeType *t = currentProtoStack.top();
    (t->*func)(name, type, dflt);

    return type;
}

int
fieldType(const char *type)
{
    if (strcmp(type, "SFBool") == 0) return SFBOOL;
    if (strcmp(type, "SFColor") == 0) return SFCOLOR;
    if (strcmp(type, "SFFloat") == 0) return SFFLOAT;
    if (strcmp(type, "SFImage") == 0) return SFIMAGE;
    if (strcmp(type, "SFInt32") == 0) return SFINT32;
    if (strcmp(type, "SFNode") == 0) return SFNODE;
    if (strcmp(type, "SFRotation") == 0) return SFROTATION;
    if (strcmp(type, "SFString") == 0) return SFSTRING;
    if (strcmp(type, "SFTime") == 0) return SFTIME;
    if (strcmp(type, "SFVec2f") == 0) return SFVEC2F;
    if (strcmp(type, "SFVec3f") == 0) return SFVEC3F;
    if (strcmp(type, "MFColor") == 0) return MFCOLOR;
    if (strcmp(type, "MFFloat") == 0) return MFFLOAT;
    if (strcmp(type, "MFInt32") == 0) return MFINT32;
    if (strcmp(type, "MFNode") == 0) return MFNODE;
    if (strcmp(type, "MFRotation") == 0) return MFROTATION;
    if (strcmp(type, "MFString") == 0) return MFSTRING;
    if (strcmp(type, "MFVec2f") == 0) return MFVEC2F;
    if (strcmp(type, "MFVec3f") == 0) return MFVEC3F;

    std::cerr << "Illegal field type: " << type << "\n";

    return 0;
}

void
enterNode(const char *nodeType)
{
    const VrmlNodeType *t = VrmlNodeType::find(nodeType);
    if (t == nullptr) {
        char tmp[1000];
        sprintf(tmp, "Unknown node type '%s'", nodeType);
        vrmlyyerror(tmp);
    }
    FieldRec *fr = new FieldRec;
    fr->nodeType = t;
    fr->fieldName = nullptr;
    fr->typeRec = nullptr;
    currentField.push(fr);

    VrmlNode *node = new VrmlNode(t);
    currentNode.push(node);
}

VrmlNode *
exitNode()
{
    FieldRec *fr = currentField.top();
    nassertr(fr != nullptr, nullptr);
    currentField.pop();

    VrmlNode *node = currentNode.top();
    nassertr(node != nullptr, nullptr);
    currentNode.pop();

    //    std::cerr << "Just defined node:\n" << *node << "\n\n";

    delete fr;
    return node;
}

void
inScript()
{
    FieldRec *fr = currentField.top();
    if (fr->nodeType == nullptr ||
        strcmp(fr->nodeType->getName(), "Script") != 0) {
        vrmlyyerror("interface declaration outside of Script or prototype");
    }
}

void
enterField(const char *fieldName)
{
    FieldRec *fr = currentField.top();
    nassertv(fr != nullptr);

    fr->fieldName = fieldName;
    fr->typeRec = nullptr;
    if (fr->nodeType != nullptr) {
        // enterField is called when parsing eventIn and eventOut IS
        // declarations, in which case we don't need to do anything special--
        // the IS IDENTIFIER will be returned from the lexer normally.
        if (fr->nodeType->hasEventIn(fieldName) ||
            fr->nodeType->hasEventOut(fieldName))
            return;
    
        const VrmlNodeType::NameTypeRec *typeRec =
          fr->nodeType->hasField(fieldName);
        if (typeRec != nullptr) {
            fr->typeRec = typeRec;
            // Let the lexer know what field type to expect:
            expect(typeRec->type);
        }
        else {
            std::cerr << "Error: Nodes of type " << fr->nodeType->getName() <<
                " do not have fields/eventIn/eventOut named " <<
                fieldName << "\n";
            // expect(ANY_FIELD);
        }
    }
    // else expect(ANY_FIELD);
}

void
storeField(const VrmlFieldValue &value) {
  FieldRec *fr = currentField.top();
  nassertv(fr != nullptr);

  VrmlNode *node = currentNode.top();
  nassertv(node != nullptr);

  if (fr->typeRec != nullptr) {
    node->_fields.push_back(VrmlNode::Field(fr->typeRec, value));
  }
}

void
exitField()
{
    FieldRec *fr = currentField.top();
    nassertv(fr != nullptr);

    fr->fieldName = nullptr;
    fr->typeRec = nullptr;
}

void
expect(int type)
{
    expectToken = type;
}


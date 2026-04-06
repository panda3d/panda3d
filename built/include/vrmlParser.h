/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_VRMLYY_BUILT_TMP_VRMLPARSER_H_INCLUDED
# define YY_VRMLYY_BUILT_TMP_VRMLPARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int vrmlyydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    IDENTIFIER = 258,              /* IDENTIFIER  */
    DEF = 259,                     /* DEF  */
    USE = 260,                     /* USE  */
    PROTO = 261,                   /* PROTO  */
    EXTERNPROTO = 262,             /* EXTERNPROTO  */
    TO = 263,                      /* TO  */
    IS = 264,                      /* IS  */
    ROUTE = 265,                   /* ROUTE  */
    SFN_NULL = 266,                /* SFN_NULL  */
    EVENTIN = 267,                 /* EVENTIN  */
    EVENTOUT = 268,                /* EVENTOUT  */
    FIELD = 269,                   /* FIELD  */
    EXPOSEDFIELD = 270,            /* EXPOSEDFIELD  */
    SFBOOL = 271,                  /* SFBOOL  */
    SFCOLOR = 272,                 /* SFCOLOR  */
    SFFLOAT = 273,                 /* SFFLOAT  */
    SFIMAGE = 274,                 /* SFIMAGE  */
    SFINT32 = 275,                 /* SFINT32  */
    SFNODE = 276,                  /* SFNODE  */
    SFROTATION = 277,              /* SFROTATION  */
    SFSTRING = 278,                /* SFSTRING  */
    SFTIME = 279,                  /* SFTIME  */
    SFVEC2F = 280,                 /* SFVEC2F  */
    SFVEC3F = 281,                 /* SFVEC3F  */
    MFCOLOR = 282,                 /* MFCOLOR  */
    MFFLOAT = 283,                 /* MFFLOAT  */
    MFINT32 = 284,                 /* MFINT32  */
    MFROTATION = 285,              /* MFROTATION  */
    MFSTRING = 286,                /* MFSTRING  */
    MFVEC2F = 287,                 /* MFVEC2F  */
    MFVEC3F = 288,                 /* MFVEC3F  */
    MFNODE = 289                   /* MFNODE  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define IDENTIFIER 258
#define DEF 259
#define USE 260
#define PROTO 261
#define EXTERNPROTO 262
#define TO 263
#define IS 264
#define ROUTE 265
#define SFN_NULL 266
#define EVENTIN 267
#define EVENTOUT 268
#define FIELD 269
#define EXPOSEDFIELD 270
#define SFBOOL 271
#define SFCOLOR 272
#define SFFLOAT 273
#define SFIMAGE 274
#define SFINT32 275
#define SFNODE 276
#define SFROTATION 277
#define SFSTRING 278
#define SFTIME 279
#define SFVEC2F 280
#define SFVEC3F 281
#define MFCOLOR 282
#define MFFLOAT 283
#define MFINT32 284
#define MFROTATION 285
#define MFSTRING 286
#define MFVEC2F 287
#define MFVEC3F 288
#define MFNODE 289

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 115 "pandatool/src/vrml/vrmlParser.yxx"

  char *string;
  VrmlFieldValue fv;
  VrmlNode *node;
  MFArray *mfarray;
  SFNodeRef nodeRef;
  VrmlScene *scene;

#line 144 "built/tmp/vrmlParser.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE vrmlyylval;


int vrmlyyparse (void);


#endif /* !YY_VRMLYY_BUILT_TMP_VRMLPARSER_H_INCLUDED  */

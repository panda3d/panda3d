/* A Bison parser, made by GNU Bison 1.875b.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

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

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     INTEGER = 258,
     REAL = 259,
     STRING = 260,
     IDENTIFIER = 261,
     WINDOWS_GUID = 262,
     KW_ARRAY = 263,
     KW_BINARY = 264,
     KW_BYTE = 265,
     KW_CHAR = 266,
     KW_CSTRING = 267,
     KW_DOUBLE = 268,
     KW_DWORD = 269,
     KW_FLOAT = 270,
     KW_STRING = 271,
     KW_TEMPLATE = 272,
     KW_UCHAR = 273,
     KW_UNICODE = 274,
     KW_WORD = 275
   };
#endif
#define INTEGER 258
#define REAL 259
#define STRING 260
#define IDENTIFIER 261
#define WINDOWS_GUID 262
#define KW_ARRAY 263
#define KW_BINARY 264
#define KW_BYTE 265
#define KW_CHAR 266
#define KW_CSTRING 267
#define KW_DOUBLE 268
#define KW_DWORD 269
#define KW_FLOAT 270
#define KW_STRING 271
#define KW_TEMPLATE 272
#define KW_UCHAR 273
#define KW_UNICODE 274
#define KW_WORD 275




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE xyylval;




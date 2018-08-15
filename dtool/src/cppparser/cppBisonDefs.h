/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppBisonDefs.h
 * @author drose
 * @date 1999-01-17
 */

#ifndef CPPBISON_H
#define CPPBISON_H

// This header file defines the interface to the yacc (actually, bison) parser
// and grammar.  None of these interfaces are intended to be used directly;
// they're defined here strictly to be used by the CPPParser and
// CPPExpressionParser classes.

#include "dtoolbase.h"

#include <string>

#include "cppClosureType.h"
#include "cppExtensionType.h"
#include "cppFile.h"

class CPPParser;
class CPPExpression;
class CPPPreprocessor;
class CPPDeclaration;
class CPPInstance;
class CPPType;
class CPPStructType;
class CPPEnumType;
class CPPSimpleType;
class CPPInstanceIdentifier;
class CPPParameterList;
class CPPTemplateParameterList;
class CPPScope;
class CPPIdentifier;
class CPPCaptureType;

void parse_cpp(CPPParser *cp);
CPPExpression *parse_const_expr(CPPPreprocessor *pp,
                                CPPScope *new_current_scope,
                                CPPScope *new_global_scope);
CPPType *parse_type(CPPPreprocessor *pp,
                    CPPScope *new_current_scope,
                    CPPScope *new_global_scope);

extern CPPScope *current_scope;
extern CPPScope *global_scope;
extern CPPPreprocessor *current_lexer;


// This structure holds the return value for each token.  Traditionally, this
// is a union, and is declared with the %union declaration in the parser.y
// file, but unions are pretty worthless in C++ (you can't include an object
// that has member functions in a union), so we'll use a class instead.  That
// means we need to declare it externally, here.

class cppyystype {
public:
  std::string str;
  union {
    unsigned long long integer;
    long double real;
    CPPScope *scope;
    CPPDeclaration *decl;
    CPPInstance *instance;
    CPPType *type;
    CPPStructType *struct_type;
    CPPEnumType *enum_type;
    CPPSimpleType *simple_type;
    CPPInstanceIdentifier *inst_ident;
    CPPParameterList *param_list;
    CPPTemplateParameterList *template_param_list;
    CPPExtensionType::Type extension_enum;
    CPPExpression *expr;
    CPPIdentifier *identifier;
    CPPClosureType *closure_type;
    CPPClosureType::Capture *capture;
  } u;
};
#define YYSTYPE cppyystype

// This structure takes advantage of a bison feature to track the exact
// location in the file of each token, for more useful error reporting.  We
// define it up here so we can reference it in the lexer.

struct cppyyltype {
  // Bison expects these members to be part of this struct.
  int first_line;
  int first_column;
  int last_line;
  int last_column;

  // Early versions of bison (1.25 and earlier) expected these members to be
  // in this struct as well.
  int timestamp;
  char *text;

  // The remaining members are added for this application and have no meaning
  // to bison.
  CPPFile file;
};
#define YYLTYPE cppyyltype

// Beginning around bison 1.35 or so, we need to define this macro as well, to
// tell bison how to collect multiple locations together.  (The default
// implementation copies only first_line through last_column, whereas here we
// use the struct assignment operator to copy all the members of the
// structure).
#define YYLLOC_DEFAULT(Current, Rhs, N)          \
  (Current) = (Rhs)[1];                              \
  (Current).last_line    = (Rhs)[N].last_line;       \
  (Current).last_column  = (Rhs)[N].last_column;

#endif

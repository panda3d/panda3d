/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xParserDefs.h
 * @author drose
 * @date 2004-10-03
 */

#ifndef XPARSERDEFS_H
#define XPARSERDEFS_H

#include "pandatoolbase.h"
#include "windowsGuid.h"
#include "xFileDataDef.h"
#include "pta_int.h"
#include "pta_double.h"

class XFile;
class XFileNode;

void x_init_parser(std::istream &in, const std::string &filename, XFile &file);
void x_cleanup_parser();
int xyyparse();

// This structure holds the return value for each token.  Traditionally, this
// is a union, and is declared with the %union declaration in the parser.y
// file, but unions are pretty worthless in C++ (you can't include an object
// that has member functions in a union), so we'll use a class instead.  That
// means we need to declare it externally, here.

class XTokenType {
public:
  union U {
    int number;
    XFileNode *node;
    XFileDataDef::Type primitive_type;
  } u;
  std::string str;
  WindowsGuid guid;
  PTA_double double_list;
  PTA_int int_list;
};

// The yacc-generated code expects to use the symbol 'YYSTYPE' to refer to the
// above class.
#define YYSTYPE XTokenType

#endif

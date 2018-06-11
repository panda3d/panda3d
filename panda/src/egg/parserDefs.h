/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file parserDefs.h
 * @author drose
 * @date 1999-01-17
 */

#ifndef PARSER_H
#define PARSER_H

#include "pandabase.h"

#include "eggObject.h"

#include "pointerTo.h"
#include "pointerToArray.h"
#include "pta_double.h"

#include <string>

class EggGroupNode;
class LightMutex;

extern LightMutex egg_lock;

void egg_init_parser(std::istream &in, const std::string &filename,
                     EggObject *tos, EggGroupNode *egg_top_node);

void egg_cleanup_parser();

// This structure holds the return value for each token.  Traditionally, this
// is a union, and is declared with the %union declaration in the parser.y
// file, but unions are pretty worthless in C++ (you can't include an object
// that has member functions in a union), so we'll use a class instead.  That
// means we need to declare it externally, here.

class EXPCL_PANDA_EGG EggTokenType {
public:
  double _number;
  unsigned long _ulong;
  std::string _string;
  PT(EggObject) _egg;
  PTA_double _number_list;
};

// The yacc-generated code expects to use the symbol 'YYSTYPE' to refer to the
// above class.
#define YYSTYPE EggTokenType

#endif

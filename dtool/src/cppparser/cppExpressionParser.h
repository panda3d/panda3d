// Filename: cppExpressionParser.h
// Created by:  drose (25Oct99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef CPPEXPRESSIONPARSER_H
#define CPPEXPRESSIONPARSER_H

#include "dtoolbase.h"

#include "cppPreprocessor.h"

class CPPExpression;
class CPPScope;

///////////////////////////////////////////////////////////////////
//       Class : CPPExpressionParser
// Description :
////////////////////////////////////////////////////////////////////
class CPPExpressionParser : public CPPPreprocessor {
public:
  CPPExpressionParser(CPPScope *current_scope, CPPScope *global_scope);
  ~CPPExpressionParser();

  bool parse_expr(const string &expr);
  bool parse_expr(const string &expr, const CPPPreprocessor &filepos);

  void output(ostream &out) const;

  CPPScope *_current_scope;
  CPPScope *_global_scope;
  CPPExpression *_expr;
};

inline ostream &
operator << (ostream &out, const CPPExpressionParser &ep) {
  ep.output(out);
  return out;
}

#endif



// Filename: cppExpressionParser.h
// Created by:  drose (25Oct99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
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



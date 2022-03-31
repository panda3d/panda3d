/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppExpressionParser.h
 * @author drose
 * @date 1999-10-25
 */

#ifndef CPPEXPRESSIONPARSER_H
#define CPPEXPRESSIONPARSER_H

#include "dtoolbase.h"

#include "cppPreprocessor.h"

class CPPExpression;
class CPPScope;

/**
 *
 */
class CPPExpressionParser : public CPPPreprocessor {
public:
  CPPExpressionParser(CPPScope *current_scope, CPPScope *global_scope);
  ~CPPExpressionParser();

  bool parse_expr(const std::string &expr);
  bool parse_expr(const std::string &expr, const CPPPreprocessor &filepos);

  void output(std::ostream &out) const;

  CPPScope *_current_scope;
  CPPScope *_global_scope;
  CPPExpression *_expr;
};

inline std::ostream &
operator << (std::ostream &out, const CPPExpressionParser &ep) {
  ep.output(out);
  return out;
}

#endif

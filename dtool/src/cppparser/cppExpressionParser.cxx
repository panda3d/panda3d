/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppExpressionParser.cxx
 * @author drose
 * @date 1999-10-25
 */

#include "cppExpressionParser.h"
#include "cppExpression.h"

/**
 *
 */
CPPExpressionParser::
CPPExpressionParser(CPPScope *current_scope, CPPScope *global_scope) :
  _current_scope(current_scope),
  _global_scope(global_scope)
{
  _expr = nullptr;
}

/**
 *
 */
CPPExpressionParser::
~CPPExpressionParser() {
}

/**
 *
 */
bool CPPExpressionParser::
parse_expr(const std::string &expr) {
  if (!init_const_expr(expr)) {
    std::cerr << "Unable to parse expression\n";
    return false;
  }

  _expr = parse_const_expr(this, _current_scope, _global_scope);

  return get_error_count() == 0;
}

/**
 *
 */
bool CPPExpressionParser::
parse_expr(const std::string &expr, const CPPPreprocessor &filepos) {
  if (!init_const_expr(expr)) {
    std::cerr << "Unable to parse expression\n";
    return false;
  }

  copy_filepos(filepos);

  _expr = parse_const_expr(this, _current_scope, _global_scope);

  return get_error_count() == 0;
}

/**
 *
 */
void CPPExpressionParser::
output(std::ostream &out) const {
  if (_expr == nullptr) {
    out << "(null expr)";
  } else {
    out << *_expr;
  }
}

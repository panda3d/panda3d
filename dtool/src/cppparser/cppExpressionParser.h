// Filename: cppExpressionParser.h
// Created by:  drose (25Oct99)
//
////////////////////////////////////////////////////////////////////

#ifndef CPPEXPRESSIONPARSER_H
#define CPPEXPRESSIONPARSER_H

#include <dtoolbase.h>

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

 

// Filename: cppExpression.h
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

#ifndef CPPEXPRESSION_H
#define CPPEXPRESSION_H

#include "dtoolbase.h"

#include "cppDeclaration.h"

class CPPIdentifier;
class CPPType;
class CPPPreprocessor;
class CPPFunctionGroup;

///////////////////////////////////////////////////////////////////
//       Class : CPPExpression
// Description :
////////////////////////////////////////////////////////////////////
class CPPExpression : public CPPDeclaration {
public:
  CPPExpression(int value);
  CPPExpression(const string &value);
  CPPExpression(double value);
  CPPExpression(CPPIdentifier *ident, CPPScope *current_scope,
                CPPScope *global_scope, CPPPreprocessor *error_sink = NULL);
  CPPExpression(int unary_operator, CPPExpression *op1);
  CPPExpression(int binary_operator, CPPExpression *op1, CPPExpression *op2);
  CPPExpression(int trinary_operator, CPPExpression *op1, CPPExpression *op2, CPPExpression *op3);

  static CPPExpression typecast_op(CPPType *type, CPPExpression *op1);
  static CPPExpression construct_op(CPPType *type, CPPExpression *op1);
  static CPPExpression new_op(CPPType *type, CPPExpression *op1 = NULL);
  static CPPExpression sizeof_func(CPPType *type);

  ~CPPExpression();

  enum ResultType {
    RT_integer,
    RT_real,
    RT_pointer,
    RT_error
  };

  class Result {
  public:
    Result();
    Result(int value);
    Result(double value);
    Result(void *value);

    int as_integer() const;
    double as_real() const;
    void *as_pointer() const;
    bool as_boolean() const;
    void output(ostream &out) const;

    ResultType _type;
    union {
      int _integer;
      double _real;
      void *_pointer;
    } _u;
  };


  Result evaluate() const;
  CPPType *determine_type() const;
  bool is_tbd() const;

  virtual bool is_fully_specified() const;
  virtual CPPDeclaration *substitute_decl(SubstDecl &subst,
                                          CPPScope *current_scope,
                                          CPPScope *global_scope);

  virtual void output(ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;
  virtual SubType get_subtype() const;

  virtual CPPExpression *as_expression();


  enum Type {
    T_integer,
    T_real,
    T_string,
    T_variable,
    T_function,
    T_unknown_ident,
    T_typecast,
    T_construct,
    T_default_construct,
    T_new,
    T_default_new,
    T_sizeof,
    T_unary_operation,
    T_binary_operation,
    T_trinary_operation,
  };

  Type _type;
  string _str;
  union {
    int _integer;
    double _real;
    CPPInstance *_variable;
    CPPFunctionGroup *_fgroup;
    CPPIdentifier *_ident;
    class {
    public:
      CPPType *_to;
      CPPExpression *_op1;
    } _typecast;
    class {
    public:
      // One of the yytoken values: a character, or something
      // like EQCOMPARE.
      int _operator;
      CPPExpression *_op1;
      CPPExpression *_op2;
      CPPExpression *_op3;
    } _op;
  } _u;

protected:
  static CPPType *elevate_type(CPPType *t1, CPPType *t2);
  virtual bool is_equal(const CPPDeclaration *other) const;
  virtual bool is_less(const CPPDeclaration *other) const;
};

inline ostream &
operator << (ostream &out, const CPPExpression::Result &result) {
  result.output(out);
  return out;
}

#endif



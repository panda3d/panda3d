/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppExpression.h
 * @author drose
 * @date 1999-10-25
 */

#ifndef CPPEXPRESSION_H
#define CPPEXPRESSION_H

#include "dtoolbase.h"

#include "cppDeclaration.h"

class CPPIdentifier;
class CPPType;
class CPPPreprocessor;
class CPPFunctionGroup;

/**
 *
 */
class CPPExpression : public CPPDeclaration {
public:
  enum Type {
    T_nullptr,
    T_boolean,
    T_integer,
    T_real,
    T_string,
    T_wstring,
    T_u8string,
    T_u16string,
    T_u32string,
    T_variable,
    T_function,
    T_unknown_ident,
    T_typecast,
    T_static_cast,
    T_dynamic_cast,
    T_const_cast,
    T_reinterpret_cast,
    T_construct,
    T_default_construct,
    T_aggregate_init,
    T_empty_aggregate_init,
    T_new,
    T_default_new,
    T_sizeof,
    T_sizeof_ellipsis,
    T_alignof,
    T_unary_operation,
    T_binary_operation,
    T_trinary_operation,
    T_literal,
    T_raw_literal,
    T_typeid_type,
    T_typeid_expr,
    T_type_trait,
    T_lambda,

    // These are used when parsing =default and =delete methods.
    T_default,
    T_delete,
  };

  CPPExpression(bool value);
  CPPExpression(unsigned long long value);
  CPPExpression(int value);
  CPPExpression(const std::string &value);
  CPPExpression(long double value);
  CPPExpression(CPPIdentifier *ident, CPPScope *current_scope,
                CPPScope *global_scope, CPPPreprocessor *error_sink = nullptr);
  CPPExpression(int unary_operator, CPPExpression *op1);
  CPPExpression(int binary_operator, CPPExpression *op1, CPPExpression *op2);
  CPPExpression(int trinary_operator, CPPExpression *op1, CPPExpression *op2, CPPExpression *op3);

  static CPPExpression typecast_op(CPPType *type, CPPExpression *op1, Type cast_type = T_typecast);
  static CPPExpression construct_op(CPPType *type, CPPExpression *op1);
  static CPPExpression aggregate_init_op(CPPType *type, CPPExpression *op1);
  static CPPExpression new_op(CPPType *type, CPPExpression *op1 = nullptr);
  static CPPExpression typeid_op(CPPType *type, CPPType *std_type_info);
  static CPPExpression typeid_op(CPPExpression *op1, CPPType *std_type_info);
  static CPPExpression type_trait(int trait, CPPType *type, CPPType *arg = nullptr);
  static CPPExpression sizeof_func(CPPType *type);
  static CPPExpression sizeof_ellipsis_func(CPPIdentifier *ident);
  static CPPExpression alignof_func(CPPType *type);
  static CPPExpression lambda(CPPClosureType *type);

  static CPPExpression literal(unsigned long long value, CPPInstance *lit_op);
  static CPPExpression literal(long double value, CPPInstance *lit_op);
  static CPPExpression literal(CPPExpression *value, CPPInstance *lit_op);
  static CPPExpression raw_literal(const std::string &raw, CPPInstance *lit_op);

  static const CPPExpression &get_nullptr();
  static const CPPExpression &get_default();
  static const CPPExpression &get_delete();

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
    void output(std::ostream &out) const;

    ResultType _type;
    union {
      int _integer;
      double _real;
      void *_pointer;
    } _u;
  };


  Result evaluate() const;
  CPPType *determine_type() const;
  bool is_lvalue() const;
  bool is_tbd() const;

  virtual bool is_fully_specified() const;
  virtual CPPDeclaration *substitute_decl(SubstDecl &subst,
                                          CPPScope *current_scope,
                                          CPPScope *global_scope);

  virtual void output(std::ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;
  virtual SubType get_subtype() const;

  virtual CPPExpression *as_expression();

  Type _type;
  std::string _str;
  union {
    bool _boolean;
    unsigned long long _integer;
    long double _real;
    CPPInstance *_variable;
    CPPFunctionGroup *_fgroup;
    CPPIdentifier *_ident;
    CPPClosureType *_closure_type;
    struct {
      union {
        CPPType *_type;
        CPPExpression *_expr;
      };
      CPPType *_std_type_info;
    } _typeid;
    struct {
      CPPType *_to;
      CPPExpression *_op1;
    } _typecast;
    struct {
      // One of the yytoken values: a character, or something like EQCOMPARE.
      int _operator;
      CPPExpression *_op1;
      CPPExpression *_op2;
      CPPExpression *_op3;
    } _op;
    struct {
      CPPInstance *_operator;
      CPPExpression *_value;
    } _literal;
    struct {
      int _trait;
      CPPType *_type;
      CPPType *_arg;
    } _type_trait;
  } _u;

protected:
  static CPPType *elevate_type(CPPType *t1, CPPType *t2);
  virtual bool is_equal(const CPPDeclaration *other) const;
  virtual bool is_less(const CPPDeclaration *other) const;
};

inline std::ostream &
operator << (std::ostream &out, const CPPExpression::Result &result) {
  result.output(out);
  return out;
}

#endif

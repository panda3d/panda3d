/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppFunctionType.h
 * @author drose
 * @date 1999-10-21
 */

#ifndef CPPFUNCTIONTYPE_H
#define CPPFUNCTIONTYPE_H

#include "dtoolbase.h"

#include "cppType.h"

class CPPParameterList;
class CPPIdentifier;

/**
 *
 */
class CPPFunctionType : public CPPType {
public:
  enum Flags {
    F_const_method      = 0x001,
    F_operator_typecast = 0x002,
    F_constructor       = 0x004,
    F_destructor        = 0x008,
    F_method_pointer    = 0x010,
    F_unary_op          = 0x020,
    F_operator          = 0x040,
    F_noexcept          = 0x080,
    F_copy_constructor  = 0x200,
    F_move_constructor  = 0x400,
    F_trailing_return_type = 0x800,
    F_final             = 0x1000,
    F_override          = 0x2000,
    F_volatile_method   = 0x4000,
    F_lvalue_method     = 0x8000,
    F_rvalue_method     = 0x10000,
    F_copy_assignment_operator = 0x20000,
    F_move_assignment_operator = 0x40000,
  };

  CPPFunctionType(CPPType *return_type, CPPParameterList *parameters,
                  int flags);
  CPPFunctionType(const CPPFunctionType &copy);
  void operator = (const CPPFunctionType &copy);

  bool accepts_num_parameters(int num_parameters);

  CPPType *_return_type;
  CPPParameterList *_parameters;
  int _flags;

  virtual bool is_fully_specified() const;
  virtual CPPDeclaration *substitute_decl(SubstDecl &subst,
                                          CPPScope *current_scope,
                                          CPPScope *global_scope);

  virtual CPPType *resolve_type(CPPScope *current_scope,
                                CPPScope *global_scope);

  virtual bool is_tbd() const;
  virtual bool is_trivial() const;

  virtual void output(std::ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;
  void output(std::ostream &out, int indent_level, CPPScope *scope,
              bool complete, int num_default_parameters) const;
  virtual void output_instance(std::ostream &out, int indent_level,
                               CPPScope *scope,
                               bool complete, const std::string &prename,
                               const std::string &name) const;
  void output_instance(std::ostream &out, int indent_level,
                       CPPScope *scope,
                       bool complete, const std::string &prename,
                       const std::string &name,
                       int num_default_parameters) const;
  int get_num_default_parameters() const;

  virtual SubType get_subtype() const;

  virtual CPPFunctionType *as_function_type();

  bool match_virtual_override(const CPPFunctionType &other) const;

  CPPIdentifier *_class_owner;

protected:
  virtual bool is_equal(const CPPDeclaration *other) const;
  virtual bool is_less(const CPPDeclaration *other) const;
};

#endif

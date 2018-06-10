/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppSimpleType.h
 * @author drose
 * @date 1999-10-19
 */

#ifndef CPPSIMPLETYPE_H
#define CPPSIMPLETYPE_H

#include "dtoolbase.h"

#include "cppType.h"

/**
 * Represents a C++ fundamental type.
 */
class CPPSimpleType : public CPPType {
public:
  enum Type {
    T_unknown,
    T_bool,
    T_char,
    T_wchar_t,
    T_char16_t,
    T_char32_t,
    T_int,
    T_float,
    T_double,
    T_void,

    // We need something to represent the type of nullptr so that we can
    // return it from decltype(nullptr).  Note that this is not the same as
    // nullptr_t, which is a typedef of decltype(nullptr).
    T_nullptr,

    // T_parameter is a special type which is assigned to expressions that are
    // discovered where a formal parameter was expected.  This is a special
    // case for handling cases like this: int foo(0); which really means the
    // same thing as: int foo = 0; but it initially looks like a function
    // prototype.
    T_parameter,

    // T_auto is also a special type that corresponds to the "auto" keyword
    // used in a variable assignment.  The type of it is automatically
    // determined at a later stage based on the type of the expression that is
    // assigned to it.
    T_auto,
  };

  enum Flags {
    F_long      = 0x001,
    F_longlong  = 0x002,
    F_short     = 0x004,
    F_unsigned  = 0x008,
    F_signed    = 0x010,
  };

  CPPSimpleType(Type type, int flags = 0);

  Type _type;
  int _flags;

  virtual bool is_tbd() const;
  bool is_arithmetic() const;
  virtual bool is_fundamental() const;
  virtual bool is_standard_layout() const;
  virtual bool is_trivial() const;
  virtual bool is_constructible(const CPPType *type) const;
  virtual bool is_default_constructible() const;
  virtual bool is_copy_constructible() const;
  virtual bool is_copy_assignable() const;
  virtual bool is_destructible() const;
  virtual bool is_parameter_expr() const;

  virtual std::string get_preferred_name() const;

  virtual void output(std::ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;
  virtual SubType get_subtype() const;

  virtual CPPSimpleType *as_simple_type();

protected:
  virtual bool is_equal(const CPPDeclaration *other) const;
  virtual bool is_less(const CPPDeclaration *other) const;
};

#endif

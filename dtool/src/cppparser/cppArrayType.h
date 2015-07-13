// Filename: cppArrayType.h
// Created by:  drose (19Oct99)
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

#ifndef CPPARRAYTYPE_H
#define CPPARRAYTYPE_H

#include "dtoolbase.h"

#include "cppType.h"

class CPPExpression;

///////////////////////////////////////////////////////////////////
//       Class : CPPArrayType
// Description :
////////////////////////////////////////////////////////////////////
class CPPArrayType : public CPPType {
public:
  CPPArrayType(CPPType *element_type, CPPExpression *bounds);

  CPPType *_element_type;
  CPPExpression *_bounds;

  virtual bool is_fully_specified() const;
  virtual CPPDeclaration *substitute_decl(SubstDecl &subst,
                                          CPPScope *current_scope,
                                          CPPScope *global_scope);

  virtual CPPType *resolve_type(CPPScope *current_scope,
                                CPPScope *global_scope);
  virtual bool is_tbd() const;
  virtual bool is_trivial() const;
  virtual bool is_equivalent(const CPPType &other) const;

  virtual void output(ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;
  virtual void output_instance(ostream &out, int indent_level,
                               CPPScope *scope,
                               bool complete, const string &prename,
                               const string &name) const;

  virtual SubType get_subtype() const;

  virtual CPPArrayType *as_array_type();

protected:
  virtual bool is_equal(const CPPDeclaration *other) const;
  virtual bool is_less(const CPPDeclaration *other) const;
};

#endif

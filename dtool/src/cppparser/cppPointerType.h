/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppPointerType.h
 * @author drose
 * @date 1999-10-19
 */

#ifndef CPPPOINTERTYPE_H
#define CPPPOINTERTYPE_H

#include "dtoolbase.h"

#include "cppType.h"

/**
 *
 */
class CPPPointerType : public CPPType {
public:
  CPPPointerType(CPPType *pointing_at);

  CPPType *_pointing_at;

  virtual bool is_fully_specified() const;
  virtual CPPDeclaration *substitute_decl(SubstDecl &subst,
                                          CPPScope *current_scope,
                                          CPPScope *global_scope);

  virtual CPPType *resolve_type(CPPScope *current_scope,
                                CPPScope *global_scope);

  virtual bool is_tbd() const;
  virtual bool is_standard_layout() const;
  virtual bool is_trivial() const;
  virtual bool is_constructible(const CPPType *other) const;
  virtual bool is_default_constructible() const;
  virtual bool is_copy_constructible() const;
  virtual bool is_copy_assignable() const;
  virtual bool is_equivalent(const CPPType &other) const;

  virtual void output(std::ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;
  virtual void output_instance(std::ostream &out, int indent_level,
                               CPPScope *scope,
                               bool complete, const std::string &prename,
                               const std::string &name) const;

  virtual SubType get_subtype() const;

  virtual CPPPointerType *as_pointer_type();

protected:
  virtual bool is_equal(const CPPDeclaration *other) const;
  virtual bool is_less(const CPPDeclaration *other) const;
};

#endif

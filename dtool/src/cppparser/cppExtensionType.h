/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppExtensionType.h
 * @author drose
 * @date 1999-10-21
 */

#ifndef CPPEXTENSIONTYPE_H
#define CPPEXTENSIONTYPE_H

#include "dtoolbase.h"

#include "cppType.h"
#include "cppInstance.h"

class CPPScope;
class CPPIdentifier;

/**
 * Base class of enum, class, struct, and union types.  An instance of the
 * base class (instead of one of the specializations) is used for forward
 * references.
 */
class CPPExtensionType : public CPPType {
public:
  enum Type {
    T_enum,
    T_class,
    T_struct,
    T_union,
    T_enum_class,
    T_enum_struct,
  };

  CPPExtensionType(Type type, CPPIdentifier *ident, CPPScope *current_scope,
                   const CPPFile &file);

  virtual std::string get_simple_name() const;
  virtual std::string get_local_name(CPPScope *scope = nullptr) const;
  virtual std::string get_fully_scoped_name() const;

  virtual bool is_incomplete() const;
  virtual bool is_tbd() const;
  virtual bool is_standard_layout() const;
  virtual bool is_trivial() const;
  virtual bool is_constructible(const CPPType *type) const;
  virtual bool is_default_constructible() const;
  virtual bool is_copy_constructible() const;
  virtual bool is_copy_assignable() const;

  virtual CPPDeclaration *substitute_decl(SubstDecl &subst,
                                          CPPScope *current_scope,
                                          CPPScope *global_scope);

  virtual CPPType *resolve_type(CPPScope *current_scope,
                                CPPScope *global_scope);

  virtual bool is_equivalent(const CPPType &other) const;


  virtual void output(std::ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;
  virtual SubType get_subtype() const;

  virtual CPPExtensionType *as_extension_type();

  Type _type;
  CPPIdentifier *_ident;
  CPPExpression *_alignment;
};

std::ostream &operator << (std::ostream &out, CPPExtensionType::Type type);

#endif

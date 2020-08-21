/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppTypedefType.h
 * @author rdb
 * @date 2014-08-01
 */

#ifndef CPPTYPEDEFTYPE_H
#define CPPTYPEDEFTYPE_H

#include "dtoolbase.h"
#include "cppType.h"

class CPPIdentifier;
class CPPInstanceIdentifier;

/**
 * A type alias created by a C++ typedef or using declaration.  These aren't
 * officially supposed to be types in themselves, but we represent them as
 * such so that we can preserve typedef names in the generated code.
 */
class CPPTypedefType : public CPPType {
public:
  CPPTypedefType(CPPType *type, const std::string &name, CPPScope *current_scope);
  CPPTypedefType(CPPType *type, CPPIdentifier *ident, CPPScope *current_scope);
  CPPTypedefType(CPPType *type, CPPInstanceIdentifier *ii,
                 CPPScope *current_scope, const CPPFile &file);

  bool is_scoped() const;
  CPPScope *get_scope(CPPScope *current_scope, CPPScope *global_scope,
                      CPPPreprocessor *error_sink = nullptr) const;

  virtual std::string get_simple_name() const;
  virtual std::string get_local_name(CPPScope *scope = nullptr) const;
  virtual std::string get_fully_scoped_name() const;

  virtual bool is_incomplete() const;
  virtual bool is_tbd() const;
  virtual bool is_fundamental() const;
  virtual bool is_standard_layout() const;
  virtual bool is_trivial() const;
  virtual bool is_constructible(const CPPType *type) const;
  virtual bool is_default_constructible() const;
  virtual bool is_copy_constructible() const;
  virtual bool is_copy_assignable() const;
  virtual bool is_destructible() const;

  virtual bool is_fully_specified() const;

  virtual CPPDeclaration *
  instantiate(const CPPTemplateParameterList *actual_params,
              CPPScope *current_scope, CPPScope *global_scope,
              CPPPreprocessor *error_sink = nullptr) const;

  virtual CPPDeclaration *substitute_decl(SubstDecl &subst,
                                          CPPScope *current_scope,
                                          CPPScope *global_scope);

  virtual CPPType *resolve_type(CPPScope *current_scope,
                                CPPScope *global_scope);

  virtual bool is_convertible_to(const CPPType *other) const;
  virtual bool is_equivalent(const CPPType &other) const;

  virtual void output(std::ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;
  virtual SubType get_subtype() const;

  virtual CPPTypedefType *as_typedef_type();

  CPPType *_type;
  CPPIdentifier *_ident;
  bool _using;

protected:
  virtual bool is_equal(const CPPDeclaration *other) const;
  virtual bool is_less(const CPPDeclaration *other) const;

  bool _subst_decl_recursive_protect;
  typedef std::vector<CPPTypeProxy *> Proxies;
  Proxies _proxies;
};

#endif

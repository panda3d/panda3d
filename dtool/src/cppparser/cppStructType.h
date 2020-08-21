/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppStructType.h
 * @author drose
 * @date 1999-10-19
 */

#ifndef CPPSTRUCTTYPE_H
#define CPPSTRUCTTYPE_H

#include "dtoolbase.h"

#include "cppIdentifier.h"
#include "cppExtensionType.h"
#include "cppFunctionGroup.h"
#include "cppVisibility.h"

#include <vector>
#include <list>

class CPPScope;
class CPPTypeProxy;

/**
 *
 */
class CPPStructType : public CPPExtensionType {
public:
  CPPStructType(Type type, CPPIdentifier *ident,
                CPPScope *current_scope,
                CPPScope *scope,
                const CPPFile &file);
  CPPStructType(const CPPStructType &copy);
  void operator = (const CPPStructType &copy);

  void append_derivation(CPPType *base, CPPVisibility vis, bool is_virtual);

  CPPScope *get_scope() const;

  bool is_abstract() const;
  bool is_base_of(const CPPStructType *other) const;
  bool is_empty() const;
  bool is_polymorphic() const;
  bool check_virtual() const;
  bool has_virtual_destructor() const;
  virtual bool is_fully_specified() const;
  virtual bool is_incomplete() const;
  virtual bool is_standard_layout() const;
  virtual bool is_trivial() const;
  virtual bool is_constructible(const CPPType *arg_type) const;
  virtual bool is_default_constructible() const;
  virtual bool is_copy_constructible() const;
  virtual bool is_copy_assignable() const;
  virtual bool is_destructible() const;
  bool is_default_constructible(CPPVisibility min_vis) const;
  bool is_copy_constructible(CPPVisibility min_vis) const;
  bool is_move_constructible(CPPVisibility min_vis  = V_public) const;
  bool is_copy_assignable(CPPVisibility min_vis) const;
  bool is_move_assignable(CPPVisibility min_vis = V_public) const;
  bool is_destructible(CPPVisibility min_vis) const;
  virtual bool is_convertible_to(const CPPType *other) const;

  inline bool is_final() const { return _final; }

  CPPFunctionGroup *get_constructor() const;
  CPPInstance *get_default_constructor() const;
  CPPInstance *get_copy_constructor() const;
  CPPInstance *get_move_constructor() const;
  CPPFunctionGroup *get_assignment_operator() const;
  CPPInstance *get_copy_assignment_operator() const;
  CPPInstance *get_move_assignment_operator() const;
  CPPInstance *get_destructor() const;

  virtual CPPDeclaration *
  instantiate(const CPPTemplateParameterList *actual_params,
              CPPScope *current_scope, CPPScope *global_scope,
              CPPPreprocessor *error_sink = nullptr) const;

  virtual CPPDeclaration *substitute_decl(SubstDecl &subst,
                                          CPPScope *current_scope,
                                          CPPScope *global_scope);

  virtual void output(std::ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;
  virtual SubType get_subtype() const;

  virtual CPPStructType *as_struct_type();

  CPPScope *_scope;
  bool _incomplete;
  bool _final;

  class Base {
  public:
    void output(std::ostream &out) const;

    CPPType *_base;
    CPPVisibility _vis;
    bool _is_virtual;
  };

  typedef std::vector<Base> Derivation;
  Derivation _derivation;

  typedef std::list<CPPInstance *> VFunctions;
  void get_virtual_funcs(VFunctions &funcs) const;
  void get_pure_virtual_funcs(VFunctions &funcs) const;

protected:
  virtual bool is_equal(const CPPDeclaration *other) const;
  virtual bool is_less(const CPPDeclaration *other) const;

  bool _subst_decl_recursive_protect;
  typedef std::vector<CPPTypeProxy *> Proxies;
  Proxies _proxies;
};

inline std::ostream &operator << (std::ostream &out, const CPPStructType::Base &base) {
  base.output(out);
  return out;
}


#endif

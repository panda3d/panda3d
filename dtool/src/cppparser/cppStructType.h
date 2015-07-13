// Filename: cppStructType.h
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

#ifndef CPPSTRUCTTYPE_H
#define CPPSTRUCTTYPE_H

#include "dtoolbase.h"

#include "cppExtensionType.h"
#include "cppVisibility.h"

#include <vector>
#include <list>

class CPPScope;
class CPPTypeProxy;

///////////////////////////////////////////////////////////////////
//       Class : CPPStructType
// Description :
////////////////////////////////////////////////////////////////////
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
  bool check_virtual();
  virtual bool is_fully_specified() const;
  virtual bool is_incomplete() const;
  virtual bool is_trivial() const;

  CPPInstance *get_destructor() const;

  virtual CPPDeclaration *
  instantiate(const CPPTemplateParameterList *actual_params,
              CPPScope *current_scope, CPPScope *global_scope,
              CPPPreprocessor *error_sink = NULL) const;

  virtual CPPDeclaration *substitute_decl(SubstDecl &subst,
                                          CPPScope *current_scope,
                                          CPPScope *global_scope);

  virtual void output(ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;
  virtual SubType get_subtype() const;

  virtual CPPStructType *as_struct_type();

  CPPScope *_scope;
  bool _incomplete;

  class Base {
  public:
    void output(ostream &out) const;

    CPPType *_base;
    CPPVisibility _vis;
    bool _is_virtual;
  };

  typedef vector<Base> Derivation;
  Derivation _derivation;

  typedef list<CPPInstance *> VFunctions;
  void get_virtual_funcs(VFunctions &funcs) const;
  void get_pure_virtual_funcs(VFunctions &funcs) const;

protected:
  virtual bool is_equal(const CPPDeclaration *other) const;
  virtual bool is_less(const CPPDeclaration *other) const;

  bool _subst_decl_recursive_protect;
  typedef vector<CPPTypeProxy *> Proxies;
  Proxies _proxies;
};

inline ostream &operator << (ostream &out, const CPPStructType::Base &base) {
  base.output(out);
  return out;
}


#endif

// Filename: cppScope.h
// Created by:  drose (21Oct99)
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

#ifndef CPPSCOPE_H
#define CPPSCOPE_H

#include "dtoolbase.h"

#include "cppVisibility.h"
#include "cppTemplateParameterList.h"
#include "cppNameComponent.h"

#include <vector>
#include <map>
#include <set>
#include <string>

using namespace std;

class CPPType;
class CPPDeclaration;
class CPPExtensionType;
class CPPStructType;
class CPPNamespace;
class CPPUsing;
class CPPTypedefType;
class CPPInstance;
class CPPFunctionGroup;
class CPPTemplateScope;
class CPPTemplateParameterList;
class CPPPreprocessor;
class CPPNameComponent;
struct cppyyltype;

///////////////////////////////////////////////////////////////////
//       Class : CPPScope
// Description :
////////////////////////////////////////////////////////////////////
class CPPScope {
public:
  CPPScope(CPPScope *parent_scope,
           const CPPNameComponent &name, CPPVisibility starting_vis);
  virtual ~CPPScope();

  void set_current_vis(CPPVisibility current_vis);
  CPPVisibility get_current_vis() const;

  void set_struct_type(CPPStructType *struct_type);
  CPPStructType *get_struct_type() const;
  CPPScope *get_parent_scope() const;

  virtual void add_declaration(CPPDeclaration *decl, CPPScope *global_scope,
                               CPPPreprocessor *preprocessor,
                               const cppyyltype &pos);
  virtual void add_enum_value(CPPInstance *inst,
                              CPPPreprocessor *preprocessor,
                              const cppyyltype &pos);
  virtual void define_extension_type(CPPExtensionType *type,
                                     CPPPreprocessor *error_sink = NULL);
  virtual void define_namespace(CPPNamespace *scope);
  virtual void add_using(CPPUsing *using_decl, CPPScope *global_scope,
                         CPPPreprocessor *error_sink = NULL);

  virtual bool is_fully_specified() const;

  CPPScope *
  instantiate(const CPPTemplateParameterList *actual_params,
              CPPScope *current_scope, CPPScope *global_scope,
              CPPPreprocessor *error_sink = NULL) const;

  CPPScope *
  substitute_decl(CPPDeclaration::SubstDecl &subst,
                  CPPScope *current_scope,
                  CPPScope *global_scope) const;

  CPPType *find_type(const string &name, bool recurse = true) const;
  CPPType *find_type(const string &name,
                     CPPDeclaration::SubstDecl &subst,
                     CPPScope *global_scope,
                     bool recurse = true) const;
  CPPScope *find_scope(const string &name, bool recurse = true) const;
  CPPScope *find_scope(const string &name,
                       CPPDeclaration::SubstDecl &subst,
                       CPPScope *global_scope,
                       bool recurse = true) const;
  CPPDeclaration *find_symbol(const string &name,
                              bool recurse = true) const;
  CPPDeclaration *find_template(const string &name,
                                bool recurse = true) const;

  virtual string get_simple_name() const;
  virtual string get_local_name(CPPScope *scope = NULL) const;
  virtual string get_fully_scoped_name() const;

  virtual void output(ostream &out, CPPScope *scope) const;
  void write(ostream &out, int indent, CPPScope *scope) const;

  CPPTemplateScope *get_template_scope();
  virtual CPPTemplateScope *as_template_scope();

private:
  bool
  copy_substitute_decl(CPPScope *to_scope, CPPDeclaration::SubstDecl &subst,
                       CPPScope *global_scope) const;

  void handle_declaration(CPPDeclaration *decl, CPPScope *global_scope,
                          CPPPreprocessor *error_sink = NULL);

public:
  typedef vector<CPPDeclaration *> Declarations;
  Declarations _declarations;

  typedef map<string, CPPType *> ExtensionTypes;
  ExtensionTypes _structs;
  ExtensionTypes _classes;
  ExtensionTypes _unions;
  ExtensionTypes _enums;

  typedef map<string, CPPNamespace *> Namespaces;
  Namespaces _namespaces;

  typedef map<string, CPPType *> Types;
  Types _types;
  typedef map<string, CPPInstance *> Variables;
  Variables _variables;
  Variables _enum_values;
  typedef map<string, CPPFunctionGroup *> Functions;
  Functions _functions;
  typedef map<string, CPPDeclaration *> Templates;
  Templates _templates;
  CPPNameComponent _name;

protected:
  CPPScope *_parent_scope;
  CPPStructType *_struct_type;
  typedef set<CPPScope *> Using;
  Using _using;
  CPPVisibility _current_vis;

private:
  typedef map<const CPPTemplateParameterList *, CPPScope *, CPPTPLCompare> Instantiations;
  Instantiations _instantiations;

  bool _is_fully_specified;
  bool _fully_specified_known;
  bool _is_fully_specified_recursive_protect;
  bool _subst_decl_recursive_protect;
};

inline ostream &
operator << (ostream &out, const CPPScope &scope) {
  scope.output(out, (CPPScope *)NULL);
  return out;
}

#endif

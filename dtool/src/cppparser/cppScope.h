/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppScope.h
 * @author drose
 * @date 1999-10-21
 */

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

/**
 *
 */
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
  virtual void add_enum_value(CPPInstance *inst);
  virtual void define_typedef_type(CPPTypedefType *type,
                                   CPPPreprocessor *error_sink = nullptr);
  virtual void define_extension_type(CPPExtensionType *type,
                                     CPPPreprocessor *error_sink = nullptr);
  virtual void define_namespace(CPPNamespace *scope);
  virtual void add_using(CPPUsing *using_decl, CPPScope *global_scope,
                         CPPPreprocessor *error_sink = nullptr);

  virtual bool is_fully_specified() const;

  CPPScope *
  instantiate(const CPPTemplateParameterList *actual_params,
              CPPScope *current_scope, CPPScope *global_scope,
              CPPPreprocessor *error_sink = nullptr) const;

  CPPScope *
  substitute_decl(CPPDeclaration::SubstDecl &subst,
                  CPPScope *current_scope,
                  CPPScope *global_scope) const;

  CPPType *find_type(const std::string &name, bool recurse = true) const;
  CPPType *find_type(const std::string &name,
                     CPPDeclaration::SubstDecl &subst,
                     CPPScope *global_scope,
                     bool recurse = true) const;
  CPPScope *find_scope(const std::string &name, CPPScope *global_scope,
                       bool recurse = true) const;
  CPPScope *find_scope(const std::string &name,
                       CPPDeclaration::SubstDecl &subst,
                       CPPScope *global_scope,
                       bool recurse = true) const;
  CPPDeclaration *find_symbol(const std::string &name,
                              bool recurse = true) const;
  CPPDeclaration *find_template(const std::string &name,
                                bool recurse = true) const;

  virtual std::string get_simple_name() const;
  virtual std::string get_local_name(CPPScope *scope = nullptr) const;
  virtual std::string get_fully_scoped_name() const;

  virtual void output(std::ostream &out, CPPScope *scope) const;
  void write(std::ostream &out, int indent, CPPScope *scope) const;

  CPPTemplateScope *get_template_scope();
  virtual CPPTemplateScope *as_template_scope();

private:
  bool
  copy_substitute_decl(CPPScope *to_scope, CPPDeclaration::SubstDecl &subst,
                       CPPScope *global_scope) const;

  void handle_declaration(CPPDeclaration *decl, CPPScope *global_scope,
                          CPPPreprocessor *error_sink = nullptr);

public:
  typedef std::vector<CPPDeclaration *> Declarations;
  Declarations _declarations;

  typedef std::map<std::string, CPPType *> ExtensionTypes;
  ExtensionTypes _structs;
  ExtensionTypes _classes;
  ExtensionTypes _unions;
  ExtensionTypes _enums;

  typedef std::map<std::string, CPPNamespace *> Namespaces;
  Namespaces _namespaces;

  typedef std::map<std::string, CPPType *> Types;
  Types _types;
  typedef std::map<std::string, CPPInstance *> Variables;
  Variables _variables;
  Variables _enum_values;
  typedef std::map<std::string, CPPFunctionGroup *> Functions;
  Functions _functions;
  typedef std::map<std::string, CPPDeclaration *> Templates;
  Templates _templates;
  CPPNameComponent _name;

  typedef std::set<CPPScope *> Using;
  Using _using;

protected:
  CPPScope *_parent_scope;
  CPPStructType *_struct_type;
  CPPVisibility _current_vis;

private:
  typedef std::map<const CPPTemplateParameterList *, CPPScope *, CPPTPLCompare> Instantiations;
  Instantiations _instantiations;

  bool _is_fully_specified;
  bool _fully_specified_known;
  bool _is_fully_specified_recursive_protect;
  bool _subst_decl_recursive_protect;
};

inline std::ostream &
operator << (std::ostream &out, const CPPScope &scope) {
  scope.output(out, nullptr);
  return out;
}

#endif

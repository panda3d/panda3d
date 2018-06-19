/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppIdentifier.h
 * @author drose
 * @date 1999-10-26
 */

#ifndef CPPIDENTIFIER_H
#define CPPIDENTIFIER_H

#include "dtoolbase.h"

#include "cppDeclaration.h"
#include "cppNameComponent.h"
#include "cppFile.h"
#include "cppBisonDefs.h"

#include <string>
#include <vector>

class CPPScope;
class CPPType;
class CPPPreprocessor;
class CPPTemplateParameterList;

/**
 *
 */
class CPPIdentifier {
public:
  CPPIdentifier(const std::string &name, const CPPFile &file = CPPFile());
  CPPIdentifier(const CPPNameComponent &name, const CPPFile &file = CPPFile());
  CPPIdentifier(const std::string &name, const cppyyltype &loc);
  CPPIdentifier(const CPPNameComponent &name, const cppyyltype &loc);
  void add_name(const std::string &name);
  void add_name(const CPPNameComponent &name);

  bool operator == (const CPPIdentifier &other) const;
  bool operator != (const CPPIdentifier &other) const;
  bool operator < (const CPPIdentifier &other) const;

  bool is_scoped() const;

  std::string get_simple_name() const;
  std::string get_local_name(CPPScope *scope = nullptr) const;
  std::string get_fully_scoped_name() const;

  bool is_fully_specified() const;
  bool is_tbd() const;


  CPPScope *get_scope(CPPScope *current_scope, CPPScope *global_scope,
                      CPPPreprocessor *error_sink = nullptr) const;
  CPPScope *get_scope(CPPScope *current_scope, CPPScope *global_scope,
                      CPPDeclaration::SubstDecl &subst,
                      CPPPreprocessor *error_sink = nullptr) const;

  CPPType *find_type(CPPScope *current_scope, CPPScope *global_scope,
                     bool force_instantiate = false,
                     CPPPreprocessor *error_sink = nullptr) const;
  CPPType *find_type(CPPScope *current_scope, CPPScope *global_scope,
                     CPPDeclaration::SubstDecl &subst,
                     CPPPreprocessor *error_sink = nullptr) const;
  CPPDeclaration *find_symbol(CPPScope *current_scope,
                              CPPScope *global_scope,
                              CPPPreprocessor *error_sink = nullptr) const;
  CPPDeclaration *find_symbol(CPPScope *current_scope,
                              CPPScope *global_scope,
                              CPPDeclaration::SubstDecl &subst,
                              CPPPreprocessor *error_sink = nullptr) const;
  CPPDeclaration *find_template(CPPScope *current_scope,
                                CPPScope *global_scope,
                                CPPPreprocessor *error_sink = nullptr) const;
  CPPScope *find_scope(CPPScope *current_scope,
                       CPPScope *global_scope,
                       CPPPreprocessor *error_sink = nullptr) const;

  CPPIdentifier *substitute_decl(CPPDeclaration::SubstDecl &subst,
                                 CPPScope *current_scope,
                                 CPPScope *global_scope);

  void output(std::ostream &out, CPPScope *scope) const;
  void output_local_name(std::ostream &out, CPPScope *scope) const;
  void output_fully_scoped_name(std::ostream &out) const;

  typedef std::vector<CPPNameComponent> Names;
  Names _names;
  CPPScope *_native_scope;
  cppyyltype _loc;
};

inline std::ostream &operator << (std::ostream &out, const CPPIdentifier &identifier) {
  identifier.output(out, nullptr);
  return out;
}


#endif

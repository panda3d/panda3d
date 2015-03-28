// Filename: cppIdentifier.h
// Created by:  drose (26Oct99)
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

#ifndef CPPIDENTIFIER_H
#define CPPIDENTIFIER_H

#include "dtoolbase.h"

#include "cppDeclaration.h"
#include "cppNameComponent.h"
#include "cppFile.h"

#include <string>
#include <vector>

class CPPScope;
class CPPType;
class CPPPreprocessor;
class CPPTemplateParameterList;

///////////////////////////////////////////////////////////////////
//       Class : CPPIdentifier
// Description :
////////////////////////////////////////////////////////////////////
class CPPIdentifier {
public:
  CPPIdentifier(const string &name, const CPPFile &file = CPPFile());
  CPPIdentifier(const CPPNameComponent &name, const CPPFile &file = CPPFile());
  void add_name(const string &name);
  void add_name(const CPPNameComponent &name);

  bool operator == (const CPPIdentifier &other) const;
  bool operator != (const CPPIdentifier &other) const;
  bool operator < (const CPPIdentifier &other) const;

  bool is_scoped() const;

  string get_simple_name() const;
  string get_local_name(CPPScope *scope = NULL) const;
  string get_fully_scoped_name() const;

  bool is_fully_specified() const;
  bool is_tbd() const;


  CPPScope *get_scope(CPPScope *current_scope, CPPScope *global_scope,
                      CPPPreprocessor *error_sink = NULL) const;
  CPPScope *get_scope(CPPScope *current_scope, CPPScope *global_scope,
                      CPPDeclaration::SubstDecl &subst,
                      CPPPreprocessor *error_sink = NULL) const;

  CPPType *find_type(CPPScope *current_scope, CPPScope *global_scope,
                     bool force_instantiate = false,
                     CPPPreprocessor *error_sink = NULL) const;
  CPPType *find_type(CPPScope *current_scope, CPPScope *global_scope,
                     CPPDeclaration::SubstDecl &subst,
                     CPPPreprocessor *error_sink = NULL) const;
  CPPDeclaration *find_symbol(CPPScope *current_scope,
                              CPPScope *global_scope,
                              CPPPreprocessor *error_sink = NULL) const;
  CPPDeclaration *find_symbol(CPPScope *current_scope,
                              CPPScope *global_scope,
                              CPPDeclaration::SubstDecl &subst,
                              CPPPreprocessor *error_sink = NULL) const;
  CPPDeclaration *find_template(CPPScope *current_scope,
                                CPPScope *global_scope,
                                CPPPreprocessor *error_sink = NULL) const;
  CPPScope *find_scope(CPPScope *current_scope,
                       CPPScope *global_scope,
                       CPPPreprocessor *error_sink = NULL) const;

  CPPIdentifier *substitute_decl(CPPDeclaration::SubstDecl &subst,
                                 CPPScope *current_scope,
                                 CPPScope *global_scope);

  void output(ostream &out, CPPScope *scope) const;
  void output_local_name(ostream &out, CPPScope *scope) const;
  void output_fully_scoped_name(ostream &out) const;

  typedef vector<CPPNameComponent> Names;
  Names _names;
  CPPScope *_native_scope;
  CPPFile _file;
};

inline ostream &operator << (ostream &out, const CPPIdentifier &identifier) {
  identifier.output(out, (CPPScope *)NULL);
  return out;
}


#endif

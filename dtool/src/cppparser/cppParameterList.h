// Filename: cppParameterList.h
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

#ifndef CPPPARAMETERLIST_H
#define CPPPARAMETERLIST_H

#include "dtoolbase.h"

#include "cppDeclaration.h"

#include <vector>

class CPPInstance;
class CPPScope;

///////////////////////////////////////////////////////////////////
//       Class : CPPParameterList
// Description : A list of formal parameters for a function
//               declaration.
////////////////////////////////////////////////////////////////////
class CPPParameterList {
public:
  CPPParameterList();

  bool is_equivalent(const CPPParameterList &other) const;

  bool operator == (const CPPParameterList &other) const;
  bool operator != (const CPPParameterList &other) const;
  bool operator < (const CPPParameterList &other) const;

  bool is_tbd() const;
  bool is_parameter_expr() const;

  bool is_fully_specified() const;
  CPPParameterList *substitute_decl(CPPDeclaration::SubstDecl &subst,
                                    CPPScope *current_scope,
                                    CPPScope *global_scope);

  CPPParameterList *resolve_type(CPPScope *current_scope,
                                 CPPScope *global_scope);

  // This vector contains a list of formal parameters, in order.  A
  // parameter may have an empty identifer name.
  typedef vector<CPPInstance *> Parameters;
  Parameters _parameters;
  bool _includes_ellipsis;

  void output(ostream &out, CPPScope *scope, bool parameter_names,
              int num_default_parameters = -1) const;
};

inline ostream &
operator << (ostream &out, const CPPParameterList &plist) {
  plist.output(out, (CPPScope *)NULL, true);
  return out;
}

#endif



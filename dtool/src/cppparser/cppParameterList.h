// Filename: cppParameterList.h
// Created by:  drose (21Oct99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
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
// Description :
////////////////////////////////////////////////////////////////////
class CPPParameterList {
public:
  CPPParameterList();

  bool is_equivalent(const CPPParameterList &other) const;

  bool operator == (const CPPParameterList &other) const;
  bool operator != (const CPPParameterList &other) const;
  bool operator < (const CPPParameterList &other) const;

  bool is_tbd() const;

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



// Filename: cppTemplateParameterList.h
// Created by:  drose (28Oct99)
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

#ifndef CPPTEMPLATEPARAMETERLIST_H
#define CPPTEMPLATEPARAMETERLIST_H

#include "dtoolbase.h"

#include "cppDeclaration.h"

#include <vector>
#include <string>

class CPPScope;

///////////////////////////////////////////////////////////////////
//       Class : CPPTemplateParameterList
// Description : This class serves to store the parameter list for a
//               template function or class, both for the formal
//               parameter list (given when the template is defined)
//               and for the actual parameter list (given when the
//               template is instantiated).
////////////////////////////////////////////////////////////////////
class CPPTemplateParameterList {
public:
  CPPTemplateParameterList();

  string get_string() const;
  void build_subst_decl(const CPPTemplateParameterList &formal_params,
                        CPPDeclaration::SubstDecl &subst,
                        CPPScope *current_scope, CPPScope *global_scope) const;

  bool is_fully_specified() const;
  bool is_tbd() const;

  bool operator == (const CPPTemplateParameterList &other) const;
  bool operator != (const CPPTemplateParameterList &other) const;
  bool operator < (const CPPTemplateParameterList &other) const;

  CPPTemplateParameterList *substitute_decl(CPPDeclaration::SubstDecl &subst,
                                            CPPScope *current_scope,
                                            CPPScope *global_scope);

  void output(ostream &out, CPPScope *scope) const;
  void write_formal(ostream &out, CPPScope *scope) const;

  typedef vector<CPPDeclaration *> Parameters;
  Parameters _parameters;
};

inline ostream &
operator << (ostream &out, const CPPTemplateParameterList &plist) {
  plist.output(out, (CPPScope *)NULL);
  return out;
}


// This is an STL function object used to uniquely order
// CPPTemplateParameterList pointers.
class CPPTPLCompare {
public:
  bool operator () (const CPPTemplateParameterList *a,
                    const CPPTemplateParameterList *b) const {
    return (*a) < (*b);
  }
};

#endif



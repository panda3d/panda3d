// Filename: cppEnumType.h
// Created by:  drose (25Oct99)
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

#ifndef CPPENUMTYPE_H
#define CPPENUMTYPE_H

#include "dtoolbase.h"

#include "cppExtensionType.h"

#include <vector>

class CPPExpression;
class CPPInstance;
class CPPScope;


///////////////////////////////////////////////////////////////////
//       Class : CPPEnumType
// Description :
////////////////////////////////////////////////////////////////////
class CPPEnumType : public CPPExtensionType {
public:
  CPPEnumType(CPPIdentifier *ident, CPPScope *current_scope,
              const CPPFile &file);

  void add_element(const string &name, CPPScope *scope,
                   CPPExpression *value = (CPPExpression *)NULL);

  virtual bool is_incomplete() const;

  virtual CPPDeclaration *substitute_decl(SubstDecl &subst,
                                          CPPScope *current_scope,
                                          CPPScope *global_scope);

  virtual void output(ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;
  virtual SubType get_subtype() const;

  virtual CPPEnumType *as_enum_type();

  typedef vector<CPPInstance *> Elements;
  Elements _elements;
};


#endif

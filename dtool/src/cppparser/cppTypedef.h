// Filename: cppTypedef.h
// Created by:  drose (19Oct99)
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

#ifndef CPPTYPEDEF_H
#define CPPTYPEDEF_H

#include "dtoolbase.h"

#include "cppInstance.h"

///////////////////////////////////////////////////////////////////
//       Class : CPPTypedef
// Description :
////////////////////////////////////////////////////////////////////
class CPPTypedef : public CPPInstance {
public:
  CPPTypedef(CPPInstance *instance, bool global);

  virtual CPPDeclaration *substitute_decl(SubstDecl &subst,
                                          CPPScope *current_scope,
                                          CPPScope *global_scope);

  virtual void output(ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;
  virtual SubType get_subtype() const;

  virtual CPPTypedef *as_typedef();
};

#endif


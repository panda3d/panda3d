// Filename: cppTypedef.h
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


// Filename: cppTypeDeclaration.h
// Created by:  drose (14Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef CPPTYPEDECLARATION_H
#define CPPTYPEDECLARATION_H

#include "dtoolbase.h"

#include "cppInstance.h"

///////////////////////////////////////////////////////////////////
//       Class : CPPTypeDeclaration
// Description : A CPPTypeDeclaration is a special declaration that
//               represents the top-level declaration of a type in a
//               source file.  Typically this is the first appearance
//               of the type.
////////////////////////////////////////////////////////////////////
class CPPTypeDeclaration : public CPPInstance {
public:
  CPPTypeDeclaration(CPPType *type);

  virtual CPPDeclaration *substitute_decl(SubstDecl &subst,
                                          CPPScope *current_scope,
                                          CPPScope *global_scope);

  virtual void output(ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;
  virtual SubType get_subtype() const;

  virtual CPPTypeDeclaration *as_type_declaration();
};

#endif


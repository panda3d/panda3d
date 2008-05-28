// Filename: cppNamespace.h
// Created by:  drose (16Nov99)
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

#ifndef CPPNAMESPACE_H
#define CPPNAMESPACE_H

#include "dtoolbase.h"

#include "cppDeclaration.h"

class CPPIdentifier;
class CPPScope;

///////////////////////////////////////////////////////////////////
//       Class : CPPNamespace
// Description :
////////////////////////////////////////////////////////////////////
class CPPNamespace : public CPPDeclaration {
public:
  CPPNamespace(CPPIdentifier *ident, CPPScope *scope,
               const CPPFile &file);

  string get_simple_name() const;
  string get_local_name(CPPScope *scope = NULL) const;
  string get_fully_scoped_name() const;
  CPPScope *get_scope() const;

  virtual void output(ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;
  virtual SubType get_subtype() const;

  virtual CPPNamespace *as_namespace();

private:
  CPPIdentifier *_ident;
  CPPScope *_scope;
};

#endif

// Filename: cppMakeProperty.h
// Created by:  rdb (18Sep14)
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

#ifndef CPPMAKEPROPERTY_H
#define CPPMAKEPROPERTY_H

#include "dtoolbase.h"

#include "cppDeclaration.h"
#include "cppIdentifier.h"

///////////////////////////////////////////////////////////////////
//       Class : CPPMakeProperty
// Description : This is a MAKE_PROPERTY() declaration appearing
//               within a class body.  It means to generate a property
//               within Python, replacing (for instance)
//               get_something()/set_something() with a synthetic
//               'something' attribute.
////////////////////////////////////////////////////////////////////
class CPPMakeProperty : public CPPDeclaration {
public:
  CPPMakeProperty(CPPIdentifier *ident,
                  CPPFunctionGroup *getter, CPPFunctionGroup *setter,
                  CPPScope *current_scope, const CPPFile &file);

  virtual string get_simple_name() const;
  virtual string get_local_name(CPPScope *scope = NULL) const;
  virtual string get_fully_scoped_name() const;

  virtual void output(ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;

  virtual SubType get_subtype() const;
  virtual CPPMakeProperty *as_make_property();

  CPPIdentifier *_ident;
  CPPFunctionGroup *_getter;
  CPPFunctionGroup *_setter;
};

#endif

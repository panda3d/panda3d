// Filename: cppFunctionGroup.h
// Created by:  drose (11Nov99)
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

#ifndef CPPFUNCTIONGROUP_H
#define CPPFUNCTIONGROUP_H

#include "dtoolbase.h"

#include "cppDeclaration.h"

class CPPInstance;

///////////////////////////////////////////////////////////////////
//       Class : CPPFunctionGroup
// Description : This class is simply a container for one or more
//               CPPInstances for functions of the same name.  It's
//               handy for storing in the CPPScope, so that
//               CPPScope::find_symbol() can return a single pointer
//               to indicate all of the functions that may share a
//               given name.
////////////////////////////////////////////////////////////////////
class CPPFunctionGroup : public CPPDeclaration {
public:
  CPPFunctionGroup(const string &name);
  ~CPPFunctionGroup();

  CPPType *get_return_type() const;

  virtual void output(ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;
  virtual SubType get_subtype() const;

  virtual CPPFunctionGroup *as_function_group();

  typedef vector<CPPInstance *> Instances;
  Instances _instances;
  string _name;
};

#endif



// Filename: parameterRemapConcreteToPointer.h
// Created by:  drose (01Aug00)
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

#ifndef PARAMETERREMAPCONCRETETOPOINTER_H
#define PARAMETERREMAPCONCRETETOPOINTER_H

#include "dtoolbase.h"

#include "parameterRemap.h"

////////////////////////////////////////////////////////////////////
//       Class : ParameterRemapConcreteToPointer
// Description : A ParameterRemap class that handles remapping a
//               concrete structure or class parameter to a pointer
//               parameter.
////////////////////////////////////////////////////////////////////
class ParameterRemapConcreteToPointer : public ParameterRemap {
public:
  ParameterRemapConcreteToPointer(CPPType *orig_type);

  virtual void pass_parameter(ostream &out, const string &variable_name);
  virtual string get_return_expr(const string &expression);
  virtual bool return_value_needs_management();
  virtual FunctionIndex get_return_value_destructor();
  virtual bool return_value_should_be_simple();
};

#endif

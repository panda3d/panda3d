// Filename: parameterRemapHandleToInt.h
// Created by:  rdb (08Sep15)
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

#ifndef PARAMETERREMAPHANDLETOINT_H
#define PARAMETERREMAPHANDLETOINT_H

#include "dtoolbase.h"

#include "parameterRemap.h"

////////////////////////////////////////////////////////////////////
//       Class : ParameterRemapHandleToInt
// Description : A ParameterRemap class that handles remapping a
//               Handle parameter to an integer.  This makes it
//               easier to set up a dynamic typing system on the
//               scripting language side.
//
//               It also applies to ButtonHandle or any other class
//               with the same semantics, because why not.
////////////////////////////////////////////////////////////////////
class ParameterRemapHandleToInt : public ParameterRemap {
public:
  ParameterRemapHandleToInt(CPPType *orig_type);

  virtual void pass_parameter(ostream &out, const string &variable_name);
  virtual string get_return_expr(const string &expression);
};

#endif

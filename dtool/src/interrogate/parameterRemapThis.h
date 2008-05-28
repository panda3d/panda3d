// Filename: parameterRemapThis.h
// Created by:  drose (02Aug00)
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

#ifndef PARAMETERREMAPTHIS_H
#define PARAMETERREMAPTHIS_H

#include "dtoolbase.h"

#include "parameterRemap.h"

class CPPType;

////////////////////////////////////////////////////////////////////
//       Class : ParameterRemapThis
// Description : A ParameterRemap class that represents a generated
//               "this" parameter.
////////////////////////////////////////////////////////////////////
class ParameterRemapThis : public ParameterRemap {
public:
  ParameterRemapThis(CPPType *type, bool is_const);

  virtual void pass_parameter(ostream &out, const string &variable_name);
  virtual string get_return_expr(const string &expression);
  virtual bool is_this();
};

#endif

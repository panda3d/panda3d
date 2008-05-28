// Filename: parameterRemapEnumToInt.h
// Created by:  drose (04Aug00)
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

#ifndef PARAMETERREMAPENUMTOINT_H
#define PARAMETERREMAPENUMTOINT_H

#include "dtoolbase.h"

#include "parameterRemap.h"

////////////////////////////////////////////////////////////////////
//       Class : ParameterRemapEnumToInt
// Description : A ParameterRemap class that handles remapping an
//               enumerated type to an integer parameter.
////////////////////////////////////////////////////////////////////
class ParameterRemapEnumToInt : public ParameterRemap {
public:
  ParameterRemapEnumToInt(CPPType *orig_type);

  virtual void pass_parameter(ostream &out, const string &variable_name);
  virtual string get_return_expr(const string &expression);

private:
  CPPType *_enum_type;

  CPPType *unwrap_type(CPPType *source_type) const;
};

#endif

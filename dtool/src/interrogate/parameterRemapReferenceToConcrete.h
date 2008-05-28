// Filename: parameterRemapReferenceToConcrete.h
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

#ifndef PARAMETERREMAPREFERENCETOCONCRETE_H
#define PARAMETERREMAPREFERENCETOCONCRETE_H

#include "dtoolbase.h"

#include "parameterRemap.h"

////////////////////////////////////////////////////////////////////
//       Class : ParameterRemapReferenceToConcrete
// Description : A ParameterRemap class that handles remapping a
//               const reference parameter to a concrete.  This only
//               makes sense when we're talking about a const
//               reference to a simple type.
////////////////////////////////////////////////////////////////////
class ParameterRemapReferenceToConcrete : public ParameterRemap {
public:
  ParameterRemapReferenceToConcrete(CPPType *orig_type);

  virtual void pass_parameter(ostream &out, const string &variable_name);
  virtual string get_return_expr(const string &expression);
};

#endif

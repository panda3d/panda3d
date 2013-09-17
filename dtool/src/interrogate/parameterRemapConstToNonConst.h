// Filename: parameterRemapConstToNonConst.h
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

#ifndef PARAMETERREMAPCONSTTONONCONST_H
#define PARAMETERREMAPCONSTTONONCONST_H

#include "dtoolbase.h"

#include "parameterRemap.h"

////////////////////////////////////////////////////////////////////
//       Class : ParameterRemapConstToNonConst
// Description : A ParameterRemap class that handles remapping a
//               simple const parameter (like const int) to an
//               ordinary parameter (like int).  It doesn't apply to
//               const references or const pointers, however.
////////////////////////////////////////////////////////////////////
class ParameterRemapConstToNonConst : public ParameterRemap {
public:
  ParameterRemapConstToNonConst(CPPType *orig_type);

  virtual void pass_parameter(ostream &out, const string &variable_name);
  virtual string get_return_expr(const string &expression);
};

#endif

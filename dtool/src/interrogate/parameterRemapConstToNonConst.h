// Filename: parameterRemapConstToNonConst.h
// Created by:  drose (04Aug00)
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

#ifndef PARAMETERREMAPCONSTTONONCONST_H
#define PARAMETERREMAPCONSTTONONCONST_H

#include "dtoolbase.h"

#include "parameterRemap.h"

////////////////////////////////////////////////////////////////////
//       Class : ParameterRemapConstToNonConst
// Description : A ParameterRemap class that handles remapping a
//               simple const parameter (like const int) to an
//               ordinary parameter (line int).  It doesn't apply to
//               const references or const pointers, however.
////////////////////////////////////////////////////////////////////
class ParameterRemapConstToNonConst : public ParameterRemap {
public:
  ParameterRemapConstToNonConst(CPPType *orig_type);

  virtual void pass_parameter(ostream &out, const string &variable_name);
  virtual string get_return_expr(const string &expression);
};

#endif

// Filename: parameterRemapReferenceToConcrete.h
// Created by:  drose (04Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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

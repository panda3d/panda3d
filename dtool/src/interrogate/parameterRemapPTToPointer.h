// Filename: parameterRemapPTToPointer.h
// Created by:  drose (10Aug00)
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

#ifndef PARAMETERREMAPPTTOPOINTER_H
#define PARAMETERREMAPPTTOPOINTER_H

#include "dtoolbase.h"

#include "parameterRemap.h"

class CPPType;
class CPPStructType;

////////////////////////////////////////////////////////////////////
//       Class : ParameterRemapPTToPointer
// Description : A ParameterRemap class that handles remapping a
//               PT(Type) or PointerTo<Type> to a Type *.
////////////////////////////////////////////////////////////////////
class ParameterRemapPTToPointer : public ParameterRemap {
public:
  ParameterRemapPTToPointer(CPPType *orig_type);

  virtual void pass_parameter(ostream &out, const string &variable_name);
  virtual string get_return_expr(const string &expression);
  virtual string temporary_to_return(const string &temporary);

private:
  CPPType *_pointer_type;
};

#endif

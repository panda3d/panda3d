// Filename: parameterRemapReferenceToPointer.h
// Created by:  drose (01Aug00)
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

#ifndef PARAMETERREMAPREFERENCETOPOINTER_H
#define PARAMETERREMAPREFERENCETOPOINTER_H

#include "dtoolbase.h"

#include "parameterRemap.h"

////////////////////////////////////////////////////////////////////
//       Class : ParameterRemapReferenceToPointer
// Description : A ParameterRemap class that handles remapping a
//               reference (or a const reference) parameter to a
//               pointer (or const pointer) parameter.
////////////////////////////////////////////////////////////////////
class ParameterRemapReferenceToPointer : public ParameterRemap {
public:
  ParameterRemapReferenceToPointer(CPPType *orig_type);

  virtual void pass_parameter(ostream &out, const string &variable_name);
  virtual string get_return_expr(const string &expression);
};

#endif

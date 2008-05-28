// Filename: parameterRemapUnchanged.h
// Created by:  drose (01Aug00)
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

#ifndef PARAMETERREMAPUNCHANGED_H
#define PARAMETERREMAPUNCHANGED_H

#include "dtoolbase.h"

#include "parameterRemap.h"

////////////////////////////////////////////////////////////////////
//       Class : ParameterRemapUnchanged
// Description : A ParameterRemap class that represents no change to
//               the parameter: the parameter type is legal as is.
////////////////////////////////////////////////////////////////////
class ParameterRemapUnchanged : public ParameterRemap {
public:
  ParameterRemapUnchanged(CPPType *orig_type);
};

#endif

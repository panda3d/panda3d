// Filename: parameterRemapUnchanged.h
// Created by:  drose (01Aug00)
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

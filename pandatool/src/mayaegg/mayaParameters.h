// Filename: mayaParameters.h
// Created by:  drose (16Feb00)
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

#ifndef MAYAPARAMETERS_H
#define MAYAPARAMETERS_H

#include "pandatoolbase.h"

////////////////////////////////////////////////////////////////////
//       Class : MayaParameters
// Description : This class is just used as a scope to hold the global
//               parameters for the Maya converter.
////////////////////////////////////////////////////////////////////
class MayaParameters {
public:
  static bool polygon_output;
  static double polygon_tolerance;
  static bool ignore_transforms;
};


#endif


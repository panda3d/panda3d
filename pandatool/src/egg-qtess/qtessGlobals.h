// Filename: qtessGlobals.h
// Created by:  drose (13Oct03)
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

#ifndef QTESS_GLOBALS_H
#define QTESS_GLOBALS_H

#include "pandatoolbase.h"

////////////////////////////////////////////////////////////////////
//       Class : QtessGlobals
// Description : Simply used as a namespace to scope some global
//               variables for this program, set from the command
//               line.
////////////////////////////////////////////////////////////////////
class QtessGlobals {
public:
  static bool _auto_place;
  static bool _auto_distribute;
  static double _curvature_ratio;
  static bool _respect_egg;
};

#endif


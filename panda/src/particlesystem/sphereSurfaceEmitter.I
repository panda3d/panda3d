// Filename: sphereSurfaceEmitter.I
// Created by:  charles (26Jun00)
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

#include "config_particlesystem.h"

////////////////////////////////////////////////////////////////////
//    Function : set_radius
//      Access : public
// Description : radius set
////////////////////////////////////////////////////////////////////

INLINE void SphereSurfaceEmitter::
set_radius(float r) {
  _radius = r;
}

////////////////////////////////////////////////////////////////////
//    Function : get_radius
//      Access : public
// Description : radius get
////////////////////////////////////////////////////////////////////

INLINE float SphereSurfaceEmitter::
get_radius() const {
  return _radius;
}

// Filename: linearFrictionForce.I
// Created by:  charles (31Jul00)
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

////////////////////////////////////////////////////////////////////
// Function : set_coef
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE void LinearFrictionForce::
set_coef(float coef) {
  _coef = coef;
}

////////////////////////////////////////////////////////////////////
// Function : get_coef
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE float LinearFrictionForce::
get_coef(void) const {
  return _coef;
}

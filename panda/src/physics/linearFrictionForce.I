// Filename: linearFrictionForce.I
// Created by:  charles (31Jul00)
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

////////////////////////////////////////////////////////////////////
// Function : set_coef
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE void LinearFrictionForce::
set_coef(float coef) {
  // friction shouldn't be outside of [0, 1]
  if (coef < 0.0f) {
    coef = 0.0f;
  } else if (coef > 1.0f) {
    coef = 1.0f;
  }
  _coef = coef;
}

////////////////////////////////////////////////////////////////////
// Function : get_coef
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE float LinearFrictionForce::
get_coef() const {
  return _coef;
}

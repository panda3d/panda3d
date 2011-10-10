// Filename: linearFrictionForce.I
// Created by:  charles (31Jul00)
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

////////////////////////////////////////////////////////////////////
// Function : set_coef
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE void LinearFrictionForce::
set_coef(PN_stdfloat coef) {
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
INLINE PN_stdfloat LinearFrictionForce::
get_coef() const {
  return _coef;
}

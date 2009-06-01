// Filename: physxCapsule.I
// Created by:  pratt (Apr 7, 2006)
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
//     Function : get_radius
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE float PhysxCapsule::
get_radius() const {
  nassertr(nCapsule != NULL, -1.0f);

  return nCapsule->radius;
}

////////////////////////////////////////////////////////////////////
//     Function : set_radius
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxCapsule::
set_radius(float value) {
  nassertv(nCapsule != NULL);

  nCapsule->radius = value;
}


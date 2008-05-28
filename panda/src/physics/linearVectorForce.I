// Filename: linearVectorForce.I
// Created by:  charles (21Jun00)
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
//    Function : set_vector
//      Access : Public
// Description : encapsulating wrapper
////////////////////////////////////////////////////////////////////
INLINE void LinearVectorForce::
set_vector(const LVector3f& v) {
  _fvec = v;
}

////////////////////////////////////////////////////////////////////
//    Function : set_vector
//      Access : Public
// Description : piecewise encapsulating wrapper
////////////////////////////////////////////////////////////////////
INLINE void LinearVectorForce::
set_vector(float x, float y, float z) {
  _fvec.set(x, y, z);
}

////////////////////////////////////////////////////////////////////
//    Function : get_local_vector
//      Access : Public
// Description :
////////////////////////////////////////////////////////////////////
INLINE LVector3f LinearVectorForce::
get_local_vector() const {
  return _fvec;
}

////////////////////////////////////////////////////////////////////
//     Function: LinearVectorForce::LinearVectorForce += LinearVectorForce
//       Access: Public
//  Description: Performs a memberwise addition between two LinearVectorForces.
////////////////////////////////////////////////////////////////////
LinearVectorForce& LinearVectorForce::
operator += (const LinearVectorForce &other) {
  _fvec+=other._fvec;
  // Should something happen with _amplitude?
  return *this;
}

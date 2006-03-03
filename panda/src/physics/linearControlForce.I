// Filename: linearControlForce.I
// Created by: Dave Schuyler (2006)
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
//    Function : set_physics_object
//      Access : Public
// Description : encapsulating wrapper
////////////////////////////////////////////////////////////////////
INLINE void LinearControlForce::
set_physics_object(const PhysicsObject *po) {
  _physics_object = po;
}

////////////////////////////////////////////////////////////////////
//    Function : get_physics_object
//      Access : Public
// Description : piecewise encapsulating wrapper
////////////////////////////////////////////////////////////////////
INLINE CPT(PhysicsObject) LinearControlForce::
get_physics_object() const {
  return _physics_object;
}

////////////////////////////////////////////////////////////////////
//    Function : set_vector
//      Access : Public
// Description : encapsulating wrapper
////////////////////////////////////////////////////////////////////
INLINE void LinearControlForce::
set_vector(const LVector3f& v) {
  _fvec = v;
}

////////////////////////////////////////////////////////////////////
//    Function : set_vector
//      Access : Public
// Description : piecewise encapsulating wrapper
////////////////////////////////////////////////////////////////////
INLINE void LinearControlForce::
set_vector(float x, float y, float z) {
  _fvec.set(x, y, z);
}

////////////////////////////////////////////////////////////////////
//    Function : get_local_vector
//      Access : Public
// Description :
////////////////////////////////////////////////////////////////////
INLINE LVector3f LinearControlForce::
get_local_vector() const {
  return _fvec;
}

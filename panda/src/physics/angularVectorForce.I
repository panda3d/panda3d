// Filename: angularVectorForce.I
// Created by:  charles (09Aug00)
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
// Function : set_quat
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE void AngularVectorForce::
set_quat(const LRotationf &v) {
  _fvec = v;
}

////////////////////////////////////////////////////////////////////
// Function : set_hpr
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE void AngularVectorForce::
set_hpr(float h, float p, float r) {
  _fvec.set_hpr(LVecBase3f(h, p, r));
}

////////////////////////////////////////////////////////////////////
// Function : get_local_quat
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE LRotationf AngularVectorForce::
get_local_quat() const {
  return _fvec;
}

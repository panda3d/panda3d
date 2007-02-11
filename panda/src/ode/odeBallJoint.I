// Filename: odeBallJoint.I
// Created by:  joswilso (27Dec06)
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

INLINE void OdeBallJoint::
set_anchor(dReal x, dReal y, dReal z) {
  dJointSetBallAnchor(_id, x, y, z);
}

INLINE void OdeBallJoint::
set_anchor(const LVecBase3f &anchor) {
  dJointSetBallAnchor(_id, anchor[0], anchor[1], anchor[2]);
}

INLINE void OdeBallJoint::
set_anchor2(dReal x, dReal y, dReal z) {
  dJointSetBallAnchor2(_id, x, y, z);
}

INLINE void OdeBallJoint::
set_anchor2(const LVecBase3f &anchor) {
  dJointSetBallAnchor2(_id, anchor[0], anchor[1], anchor[2]);
}

INLINE LVecBase3f OdeBallJoint::
get_anchor() const {
  dVector3 result;
  dJointGetBallAnchor(_id, result);
  return LVecBase3f(result[0], result[1], result[2]);
}

INLINE LVecBase3f OdeBallJoint::
get_anchor2() const {
  dVector3 result;
  dJointGetBallAnchor2(_id, result);
  return LVecBase3f(result[0], result[1], result[2]);
}

/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeBallJoint.I
 * @author joswilso
 * @date 2006-12-27
 */

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

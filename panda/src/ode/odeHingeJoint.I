// Filename: odeHingeJoint.I
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

INLINE void OdeHingeJoint::
set_hinge_anchor(dReal x, dReal y, dReal z) {
  dJointSetHingeAnchor(_id, x, y, z);
}

INLINE void OdeHingeJoint::
set_hinge_anchor_delta(dReal x, dReal y, dReal z, dReal ax, dReal ay, dReal az) {
  dJointSetHingeAnchorDelta(_id, x, y, z, ax, ay, az);
}

INLINE void OdeHingeJoint::
set_hinge_axis(dReal x, dReal y, dReal z) {
  dJointSetHingeAxis(_id, x, y, z);
}

INLINE void OdeHingeJoint::
set_hinge_param(int parameter, dReal value) {
  dJointSetHingeParam(_id, parameter, value);
}

INLINE void OdeHingeJoint::
add_hinge_torque(dReal torque) {
  dJointAddHingeTorque(_id, torque);
}

INLINE void OdeHingeJoint::
get_hinge_anchor(dVector3 result) const {
  return dJointGetHingeAnchor(_id, result);
}

INLINE void OdeHingeJoint::
get_hinge_anchor2(dVector3 result) const {
  return dJointGetHingeAnchor2(_id, result);
}

INLINE void OdeHingeJoint::
get_hinge_axis(dVector3 result) const {
  return dJointGetHingeAxis(_id, result);
}

INLINE dReal OdeHingeJoint::
get_hinge_param(int parameter) const {
  return dJointGetHingeParam(_id, parameter);
}

INLINE dReal OdeHingeJoint::
get_hinge_angle() const {
  return dJointGetHingeAngle(_id);
}

INLINE dReal OdeHingeJoint::
get_hinge_angle_rate() const {
  return dJointGetHingeAngleRate(_id);
}


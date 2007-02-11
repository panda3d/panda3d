// Filename: odeUniversalJoint.I
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

INLINE void OdeUniversalJoint::
set_anchor(dReal x, dReal y, dReal z) {
  dJointSetUniversalAnchor(_id, x, y, z);
}

INLINE void OdeUniversalJoint::
set_axis1(dReal x, dReal y, dReal z) {
  dJointSetUniversalAxis1(_id, x, y, z);
}

INLINE void OdeUniversalJoint::
set_axis2(dReal x, dReal y, dReal z) {
  dJointSetUniversalAxis2(_id, x, y, z);
}

INLINE void OdeUniversalJoint::
set_param(int parameter, dReal value) {
  dJointSetUniversalParam(_id, parameter, value);
}

INLINE void OdeUniversalJoint::
add_torques(dReal torque1, dReal torque2) {
  dJointAddUniversalTorques(_id, torque1, torque2);
}

INLINE void OdeUniversalJoint::
get_anchor(dVector3 result) const {
  return dJointGetUniversalAnchor(_id, result);
}

INLINE void OdeUniversalJoint::
get_anchor2(dVector3 result) const {
  return dJointGetUniversalAnchor2(_id, result);
}

INLINE void OdeUniversalJoint::
get_axis1(dVector3 result) const {
  return dJointGetUniversalAxis1(_id, result);
}

INLINE void OdeUniversalJoint::
get_axis2(dVector3 result) const {
  return dJointGetUniversalAxis2(_id, result);
}

INLINE dReal OdeUniversalJoint::
get_param(int parameter) const {
  return dJointGetUniversalParam(_id, parameter);
}

INLINE dReal OdeUniversalJoint::
get_angle1() const {
  return dJointGetUniversalAngle1(_id);
}

INLINE dReal OdeUniversalJoint::
get_angle2() const {
  return dJointGetUniversalAngle2(_id);
}

INLINE dReal OdeUniversalJoint::
get_angle1_rate() const {
  return dJointGetUniversalAngle1Rate(_id);
}

INLINE dReal OdeUniversalJoint::
get_angle2_rate() const {
  return dJointGetUniversalAngle2Rate(_id);
}

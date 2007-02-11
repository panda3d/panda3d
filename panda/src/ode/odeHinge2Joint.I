// Filename: odeHinge2Joint.I
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


INLINE void OdeHinge2Joint::
set_anchor(dReal x, dReal y, dReal z) {
  dJointSetHinge2Anchor(_id, x, y, z);
}

INLINE void OdeHinge2Joint::
set_axis1(dReal x, dReal y, dReal z) {
  dJointSetHinge2Axis1(_id, x, y, z);
}

INLINE void OdeHinge2Joint::
set_axis2(dReal x, dReal y, dReal z) {
  dJointSetHinge2Axis2(_id, x, y, z);
}

INLINE void OdeHinge2Joint::
set_param(int parameter, dReal value) {
  dJointSetHinge2Param(_id, parameter, value);
}

INLINE void OdeHinge2Joint::
add_torques(dReal torque1, dReal torque2) {
  dJointAddHinge2Torques(_id, torque1, torque2);
}

INLINE void OdeHinge2Joint::
get_anchor(dVector3 result) const {
  return dJointGetHinge2Anchor(_id, result);
}

INLINE void OdeHinge2Joint::
get_anchor2(dVector3 result) const {
  return dJointGetHinge2Anchor2(_id, result);
}

INLINE void OdeHinge2Joint::
get_axis1(dVector3 result) const {
  return dJointGetHinge2Axis1(_id, result);
}

INLINE void OdeHinge2Joint::
get_axis2(dVector3 result) const {
  return dJointGetHinge2Axis2(_id, result);
}

INLINE dReal OdeHinge2Joint::
get_param(int parameter) const {
  return dJointGetHinge2Param(_id, parameter);
}

INLINE dReal OdeHinge2Joint::
get_angle1() const {
  return dJointGetHinge2Angle1(_id);
}

INLINE dReal OdeHinge2Joint::
get_angle1_rate() const {
  return dJointGetHinge2Angle1Rate(_id);
}

INLINE dReal OdeHinge2Joint::
get_angle2_rate() const {
  return dJointGetHinge2Angle2Rate(_id);
}

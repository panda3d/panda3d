// Filename: odeAMotorJoint.I
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

INLINE void OdeAMotorJoint::
set_num_axes(int num) {
  dJointSetAMotorNumAxes(_id, num);
}

INLINE void OdeAMotorJoint::
set_axis(int anum, int rel, dReal x, dReal y, dReal z) {
  dJointSetAMotorAxis(_id, anum, rel, x, y, z);
}

INLINE void OdeAMotorJoint::
set_angle(int anum, dReal angle) {
  dJointSetAMotorAngle(_id, anum, angle);
}

INLINE void OdeAMotorJoint::
set_param(int parameter, dReal value) {
  dJointSetAMotorParam(_id, parameter, value);
}

INLINE void OdeAMotorJoint::
set_mode(int mode) {
  dJointSetAMotorMode(_id, mode);
}

INLINE void OdeAMotorJoint::
add_torques(dReal torque1, dReal torque2, dReal torque3) {
  dJointAddAMotorTorques(_id, torque1, torque2, torque3);
}

INLINE int OdeAMotorJoint::
get_num_axes() const {
  return dJointGetAMotorNumAxes(_id);
}

INLINE void OdeAMotorJoint::
get_axis(int anum, dVector3 result) const {
  return dJointGetAMotorAxis(_id, anum, result);
}

INLINE int OdeAMotorJoint::
get_axis_rel(int anum) const {
  return dJointGetAMotorAxisRel(_id, anum);
}

INLINE dReal OdeAMotorJoint::
get_angle(int anum) const {
  return dJointGetAMotorAngle(_id, anum);
}

INLINE dReal OdeAMotorJoint::
get_angle_rate(int anum) const {
  return dJointGetAMotorAngleRate(_id, anum);
}

INLINE dReal OdeAMotorJoint::
get_param(int parameter) const {
  return dJointGetAMotorParam(_id, parameter);
}

INLINE int OdeAMotorJoint::
get_mode() const {
  return dJointGetAMotorMode(_id);
}

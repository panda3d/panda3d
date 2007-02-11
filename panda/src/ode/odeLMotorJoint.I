// Filename: odeLMotorJoint.I
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


INLINE void OdeLMotorJoint::
set_num_axes(int num) {
  dJointSetLMotorNumAxes(_id, num);
}

INLINE void OdeLMotorJoint::
set_axis(int anum, int rel, dReal x, dReal y, dReal z) {
  dJointSetLMotorAxis(_id, anum, rel, x, y, z);
}

INLINE void OdeLMotorJoint::
set_axis(int anum, int rel, const LVecBase3f &axis) {
  dJointSetLMotorAxis(_id, anum, rel, axis[0], axis[1], axis[2]);
}

INLINE void OdeLMotorJoint::
set_param(int parameter, dReal value) {
  dJointSetLMotorParam(_id, parameter, value);
}

INLINE int OdeLMotorJoint::
get_num_axes() const {
  return dJointGetLMotorNumAxes(_id);
}

INLINE LVecBase3f OdeLMotorJoint::
get_axis(int anum) const {
  dVector3 result;
  dJointGetLMotorAxis(_id, anum, result);
  return LVecBase3f(result[0], result[1], result[2]);
}

INLINE dReal OdeLMotorJoint::
get_param(int parameter) const {
  return dJointGetLMotorParam(_id, parameter);
}


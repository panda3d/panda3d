// Filename: odeSliderJoint.I
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

INLINE void OdeSliderJoint::
set_axis(dReal x, dReal y, dReal z) {
  dJointSetSliderAxis(_id, x, y, z);
}

INLINE void OdeSliderJoint::
set_axis_delta(dReal x, dReal y, dReal z, dReal ax, dReal ay, dReal az) {
  dJointSetSliderAxisDelta(_id, x, y, z, ax, ay, az);
}

INLINE void OdeSliderJoint::
set_param(int parameter, dReal value) {
  dJointSetSliderParam(_id, parameter, value);
}

INLINE void OdeSliderJoint::
add_force(dReal force) {
  dJointAddSliderForce(_id, force);
}

INLINE dReal OdeSliderJoint::
get_position() const {
  return dJointGetSliderPosition(_id);
}

INLINE dReal OdeSliderJoint::
get_position_rate() const {
  return dJointGetSliderPositionRate(_id);
}

INLINE void OdeSliderJoint::
get_axis(dVector3 result) const {
  return dJointGetSliderAxis(_id, result);
}

INLINE dReal OdeSliderJoint::
get_param(int parameter) const {
  return dJointGetSliderParam(_id, parameter);
}

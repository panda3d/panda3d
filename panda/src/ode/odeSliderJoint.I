// Filename: odeSliderJoint.I
// Created by:  joswilso (27Dec06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

INLINE void OdeSliderJoint::
set_axis(dReal x, dReal y, dReal z) {
  dJointSetSliderAxis(_id, x, y, z);
}

INLINE void OdeSliderJoint::
set_axis(const LVecBase3f &axis) {
  dJointSetSliderAxis(_id, axis[0], axis[1], axis[2]);
}

INLINE void OdeSliderJoint::
set_axis_delta(dReal x, dReal y, dReal z, dReal ax, dReal ay, dReal az) {
  dJointSetSliderAxisDelta(_id, x, y, z, ax, ay, az);
}

INLINE void OdeSliderJoint::
set_axis_delta(const LVecBase3f &axis, const LVecBase3f &vec) {
  dJointSetSliderAxisDelta(_id, axis[0], axis[1], axis[2], vec[0], vec[1], vec[2]);
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

INLINE LVecBase3f OdeSliderJoint::
get_axis() const {
  dVector3 result;
  dJointGetSliderAxis(_id, result);
  return LVecBase3f(result[0], result[1], result[2]);
}

INLINE void OdeSliderJoint::
set_param_lo_stop(dReal val) {
  nassertv( _id != 0 );
  dJointSetSliderParam(_id, dParamLoStop, val);
}

INLINE void OdeSliderJoint::
set_param_hi_stop(dReal val) {
  nassertv( _id != 0 );
  dJointSetSliderParam(_id, dParamHiStop, val);
}

INLINE void OdeSliderJoint::
set_param_vel(dReal val) {
  nassertv( _id != 0 );
  dJointSetSliderParam(_id, dParamVel, val);
}

INLINE void OdeSliderJoint::
set_param_f_max(dReal val) {
  nassertv( _id != 0 );
  dJointSetSliderParam(_id, dParamFMax, val);
}

INLINE void OdeSliderJoint::
set_param_fudge_factor(dReal val) {
  nassertv( _id != 0 );
  dJointSetSliderParam(_id, dParamFudgeFactor, val);
}

INLINE void OdeSliderJoint::
set_param_bounce(dReal val) {
  nassertv( _id != 0 );
  dJointSetSliderParam(_id, dParamBounce, val);
}

INLINE void OdeSliderJoint::
set_param_CFM(dReal val) {
  nassertv( _id != 0 );
  dJointSetSliderParam(_id, dParamCFM, val);
}

INLINE void OdeSliderJoint::
set_param_stop_ERP(dReal val) {
  nassertv( _id != 0 );
  dJointSetSliderParam(_id, dParamStopERP, val);
}

INLINE void OdeSliderJoint::
set_param_stop_CFM(dReal val) {
  nassertv( _id != 0 );
  dJointSetSliderParam(_id, dParamStopCFM, val);
}

INLINE dReal OdeSliderJoint::
get_param_lo_stop() const {
  nassertr( _id != 0, 0 );
  return dJointGetSliderParam(_id, dParamLoStop);
}

INLINE dReal OdeSliderJoint::
get_param_hi_stop() const {
  nassertr( _id != 0, 0 );
  return dJointGetSliderParam(_id, dParamHiStop);
}

INLINE dReal OdeSliderJoint::
get_param_vel() const {
  nassertr( _id != 0, 0 );
  return dJointGetSliderParam(_id, dParamVel);
}

INLINE dReal OdeSliderJoint::
get_param_f_max() const {
  nassertr( _id != 0, 0 );
  return dJointGetSliderParam(_id, dParamFMax);
}

INLINE dReal OdeSliderJoint::
get_param_fudge_factor() const {
  nassertr( _id != 0, 0 );
  return dJointGetSliderParam(_id, dParamFudgeFactor);
}

INLINE dReal OdeSliderJoint::
get_param_bounce() const {
  nassertr( _id != 0, 0 );
  return dJointGetSliderParam(_id, dParamBounce);
}

INLINE dReal OdeSliderJoint::
get_param_CFM() const {
  nassertr( _id != 0, 0 );
  return dJointGetSliderParam(_id, dParamCFM);
}

INLINE dReal OdeSliderJoint::
get_param_stop_ERP() const {
  nassertr( _id != 0, 0 );
  return dJointGetSliderParam(_id, dParamStopERP);
}

INLINE dReal OdeSliderJoint::
get_param_stop_CFM() const {
  nassertr( _id != 0, 0 );
  return dJointGetSliderParam(_id, dParamStopCFM);
}


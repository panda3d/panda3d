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


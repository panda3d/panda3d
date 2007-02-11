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
set_anchor(dReal x, dReal y, dReal z) {
  dJointSetHingeAnchor(_id, x, y, z);
}

INLINE void OdeHingeJoint::
set_anchor_delta(dReal x, dReal y, dReal z, dReal ax, dReal ay, dReal az) {
  dJointSetHingeAnchorDelta(_id, x, y, z, ax, ay, az);
}

INLINE void OdeHingeJoint::
set_axis(dReal x, dReal y, dReal z) {
  dJointSetHingeAxis(_id, x, y, z);
}

INLINE void OdeHingeJoint::
add_torque(dReal torque) {
  dJointAddHingeTorque(_id, torque);
}

INLINE LVecBase3f OdeHingeJoint::
get_anchor() const {
  dVector3 result;
  dJointGetHingeAnchor(_id, result);
  return LVecBase3f(result[0], result[1], result[2]);
}

INLINE LVecBase3f OdeHingeJoint::
get_anchor2() const {
  dVector3 result;
  dJointGetHingeAnchor2(_id, result);
  return LVecBase3f(result[0], result[1], result[2]);
}

INLINE LVecBase3f OdeHingeJoint::
get_axis() const {
  dVector3 result;
  dJointGetHingeAxis(_id, result);
  return LVecBase3f(result[0], result[1], result[2]);
}

INLINE dReal OdeHingeJoint::
get_angle() const {
  return dJointGetHingeAngle(_id);
}

INLINE dReal OdeHingeJoint::
get_angle_rate() const {
  return dJointGetHingeAngleRate(_id);
}

INLINE void OdeHingeJoint::
set_param_lo_stop(dReal val) {
  nassertv( _id != 0 );
  dJointSetHingeParam(_id, dParamLoStop, val);
}

INLINE void OdeHingeJoint::
set_param_hi_stop(dReal val) {
  nassertv( _id != 0 );
  dJointSetHingeParam(_id, dParamHiStop, val);
}

INLINE void OdeHingeJoint::
set_param_vel(dReal val) {
  nassertv( _id != 0 );
  dJointSetHingeParam(_id, dParamVel, val);
}

INLINE void OdeHingeJoint::
set_param_f_max(dReal val) {
  nassertv( _id != 0 );
  dJointSetHingeParam(_id, dParamFMax, val);
}

INLINE void OdeHingeJoint::
set_param_fudge_factor(dReal val) {
  nassertv( _id != 0 );
  dJointSetHingeParam(_id, dParamFudgeFactor, val);
}

INLINE void OdeHingeJoint::
set_param_bounce(dReal val) {
  nassertv( _id != 0 );
  dJointSetHingeParam(_id, dParamBounce, val);
}

INLINE void OdeHingeJoint::
set_param_CFM(dReal val) {
  nassertv( _id != 0 );
  dJointSetHingeParam(_id, dParamCFM, val);
}

INLINE void OdeHingeJoint::
set_param_stop_ERP(dReal val) {
  nassertv( _id != 0 );
  dJointSetHingeParam(_id, dParamStopERP, val);
}

INLINE void OdeHingeJoint::
set_param_stop_CFM(dReal val) {
  nassertv( _id != 0 );
  dJointSetHingeParam(_id, dParamStopCFM, val);
}

INLINE dReal OdeHingeJoint::
get_param_lo_stop() const {
  nassertr( _id != 0, 0 );
  return dJointGetHingeParam(_id, dParamLoStop);
}

INLINE dReal OdeHingeJoint::
get_param_hi_stop() const {
  nassertr( _id != 0, 0 );
  return dJointGetHingeParam(_id, dParamHiStop);
}

INLINE dReal OdeHingeJoint::
get_param_vel() const {
  nassertr( _id != 0, 0 );
  return dJointGetHingeParam(_id, dParamVel);
}

INLINE dReal OdeHingeJoint::
get_param_f_max() const {
  nassertr( _id != 0, 0 );
  return dJointGetHingeParam(_id, dParamFMax);
}

INLINE dReal OdeHingeJoint::
get_param_fudge_factor() const {
  nassertr( _id != 0, 0 );
  return dJointGetHingeParam(_id, dParamFudgeFactor);
}

INLINE dReal OdeHingeJoint::
get_param_bounce() const {
  nassertr( _id != 0, 0 );
  return dJointGetHingeParam(_id, dParamBounce);
}

INLINE dReal OdeHingeJoint::
get_param_CFM() const {
  nassertr( _id != 0, 0 );
  return dJointGetHingeParam(_id, dParamCFM);
}

INLINE dReal OdeHingeJoint::
get_param_stop_ERP() const {
  nassertr( _id != 0, 0 );
  return dJointGetHingeParam(_id, dParamStopERP);
}

INLINE dReal OdeHingeJoint::
get_param_stop_CFM() const {
  nassertr( _id != 0, 0 );
  return dJointGetHingeParam(_id, dParamStopCFM);
}


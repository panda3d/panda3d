// Filename: odeUniversalJoint.I
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

INLINE void OdeUniversalJoint::
set_anchor(dReal x, dReal y, dReal z) {
  dJointSetUniversalAnchor(_id, x, y, z);
}

INLINE void OdeUniversalJoint::
set_anchor(const LVecBase3f &anchor) {
  dJointSetUniversalAnchor(_id, anchor[0], anchor[1], anchor[2]);
}

INLINE void OdeUniversalJoint::
set_axis1(dReal x, dReal y, dReal z) {
  dJointSetUniversalAxis1(_id, x, y, z);
}

INLINE void OdeUniversalJoint::
set_axis1(const LVecBase3f &axis) {
  dJointSetUniversalAxis1(_id, axis[0], axis[1], axis[2]);
}

INLINE void OdeUniversalJoint::
set_axis2(dReal x, dReal y, dReal z) {
  dJointSetUniversalAxis2(_id, x, y, z);
}

INLINE void OdeUniversalJoint::
set_axis2(const LVecBase3f &axis) {
  dJointSetUniversalAxis2(_id, axis[0], axis[1], axis[2]);
}

INLINE void OdeUniversalJoint::
add_torques(dReal torque1, dReal torque2) {
  dJointAddUniversalTorques(_id, torque1, torque2);
}

INLINE LVecBase3f OdeUniversalJoint::
get_anchor() const {
  dVector3 result;
  dJointGetUniversalAnchor(_id, result);
  return LVecBase3f(result[0], result[1], result[2]);
}

INLINE LVecBase3f OdeUniversalJoint::
get_anchor2() const {
  dVector3 result;
  dJointGetUniversalAnchor2(_id, result);
  return LVecBase3f(result[0], result[1], result[2]);
}

INLINE LVecBase3f OdeUniversalJoint::
get_axis1() const {
  dVector3 result;
  dJointGetUniversalAxis1(_id, result);
  return LVecBase3f(result[0], result[1], result[2]);
}

INLINE LVecBase3f OdeUniversalJoint::
get_axis2() const {
  dVector3 result;
  dJointGetUniversalAxis2(_id, result);
  return LVecBase3f(result[0], result[1], result[2]);
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


INLINE void OdeUniversalJoint::
set_param_lo_stop(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 1 );
  if ( axis == 0 ) {
    dJointSetUniversalParam(_id, dParamLoStop, val);
  } else if ( axis == 1 ) {
    dJointSetUniversalParam(_id, dParamLoStop2, val);
  }
}

INLINE void OdeUniversalJoint::
set_param_hi_stop(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 1 );
  if ( axis == 0 ) {
    dJointSetUniversalParam(_id, dParamHiStop, val);
  } else if ( axis == 1 ) {
    dJointSetUniversalParam(_id, dParamHiStop2, val);
  }
}

INLINE void OdeUniversalJoint::
set_param_vel(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 1 );
  if ( axis == 0 ) {
    dJointSetUniversalParam(_id, dParamVel, val);
  } else if ( axis == 1 ) {
    dJointSetUniversalParam(_id, dParamVel2, val);
  }
}

INLINE void OdeUniversalJoint::
set_param_f_max(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 1 );
  if ( axis == 0 ) {
    dJointSetUniversalParam(_id, dParamFMax, val);
  } else if ( axis == 1 ) {
    dJointSetUniversalParam(_id, dParamFMax2, val);
  }
}

INLINE void OdeUniversalJoint::
set_param_fudge_factor(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 1 );
  if ( axis == 0 ) {
    dJointSetUniversalParam(_id, dParamFudgeFactor, val);
  } else if ( axis == 1 ) {
    dJointSetUniversalParam(_id, dParamFudgeFactor2, val);
  }
}

INLINE void OdeUniversalJoint::
set_param_bounce(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 1 );
  if ( axis == 0 ) {
    dJointSetUniversalParam(_id, dParamBounce, val);
  } else if ( axis == 1 ) {
    dJointSetUniversalParam(_id, dParamBounce2, val);
  }
}

INLINE void OdeUniversalJoint::
set_param_CFM(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 1 );
  if ( axis == 0 ) {
    dJointSetUniversalParam(_id, dParamCFM, val);
  } else if ( axis == 1 ) {
    dJointSetUniversalParam(_id, dParamCFM2, val);
  }
}

INLINE void OdeUniversalJoint::
set_param_stop_ERP(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 1 );
  if ( axis == 0 ) {
    dJointSetUniversalParam(_id, dParamStopERP, val);
  } else if ( axis == 1 ) {
    dJointSetUniversalParam(_id, dParamStopERP2, val);
  }
}

INLINE void OdeUniversalJoint::
set_param_stop_CFM(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 1 );
  if ( axis == 0 ) {
    dJointSetUniversalParam(_id, dParamStopCFM, val);
  } else if ( axis == 1 ) {
    dJointSetUniversalParam(_id, dParamStopCFM2, val);
  }
}

INLINE dReal OdeUniversalJoint::
get_param_lo_stop(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 1, 0 );
  if ( axis == 0 ) {
    return dJointGetUniversalParam(_id, dParamLoStop);
  } else if ( axis == 1 ) {
    return dJointGetUniversalParam(_id, dParamLoStop2);
  }
  return 0;
}

INLINE dReal OdeUniversalJoint::
get_param_hi_stop(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 1, 0 );
  if ( axis == 0 ) {
    return dJointGetUniversalParam(_id, dParamHiStop);
  } else if ( axis == 1 ) {
    return dJointGetUniversalParam(_id, dParamHiStop2);
  }
  return 0;
}

INLINE dReal OdeUniversalJoint::
get_param_vel(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 1, 0 );
  if ( axis == 0 ) {
    return dJointGetUniversalParam(_id, dParamVel);
  } else if ( axis == 1 ) {
    return dJointGetUniversalParam(_id, dParamVel2);
  }
  return 0;
}

INLINE dReal OdeUniversalJoint::
get_param_f_max(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 1, 0 );
  if ( axis == 0 ) {
    return dJointGetUniversalParam(_id, dParamFMax);
  } else if ( axis == 1 ) {
    return dJointGetUniversalParam(_id, dParamFMax2);
  }
  return 0;
}

INLINE dReal OdeUniversalJoint::
get_param_fudge_factor(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 1, 0 );
  if ( axis == 0 ) {
    return dJointGetUniversalParam(_id, dParamFudgeFactor);
  } else if ( axis == 1 ) {
    return dJointGetUniversalParam(_id, dParamFudgeFactor2);
  }
  return 0;
}

INLINE dReal OdeUniversalJoint::
get_param_bounce(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 1, 0 );
  if ( axis == 0 ) {
    return dJointGetUniversalParam(_id, dParamBounce);
  } else if ( axis == 1 ) {
    return dJointGetUniversalParam(_id, dParamBounce2);
  }
  return 0;
}

INLINE dReal OdeUniversalJoint::
get_param_CFM(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 1, 0 );
  if ( axis == 0 ) {
    return dJointGetUniversalParam(_id, dParamCFM);
  } else if ( axis == 1 ) {
    return dJointGetUniversalParam(_id, dParamCFM2);
  }
  return 0;
}

INLINE dReal OdeUniversalJoint::
get_param_stop_ERP(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 1, 0 );
  if ( axis == 0 ) {
    return dJointGetUniversalParam(_id, dParamStopERP);
  } else if ( axis == 1 ) {
    return dJointGetUniversalParam(_id, dParamStopERP2);
  }
  return 0;
}

INLINE dReal OdeUniversalJoint::
get_param_stop_CFM(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 1, 0 );
  if ( axis == 0 ) {
    return dJointGetUniversalParam(_id, dParamStopCFM);
  } else if ( axis == 1 ) {
    return dJointGetUniversalParam(_id, dParamStopCFM2);
  }
  return 0;
}


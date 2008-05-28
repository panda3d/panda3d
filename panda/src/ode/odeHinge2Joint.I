// Filename: odeHinge2Joint.I
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


INLINE void OdeHinge2Joint::
set_anchor(dReal x, dReal y, dReal z) {
  dJointSetHinge2Anchor(_id, x, y, z);
}

INLINE void OdeHinge2Joint::
set_anchor(const LVecBase3f &anchor) {
  dJointSetHinge2Anchor(_id, anchor[0], anchor[1], anchor[2]);
}

INLINE void OdeHinge2Joint::
set_axis1(dReal x, dReal y, dReal z) {
  dJointSetHinge2Axis1(_id, x, y, z);
}

INLINE void OdeHinge2Joint::
set_axis1(const LVecBase3f &axis) {
  dJointSetHinge2Axis1(_id, axis[0], axis[1], axis[2]);
}

INLINE void OdeHinge2Joint::
set_axis2(dReal x, dReal y, dReal z) {
  dJointSetHinge2Axis2(_id, x, y, z);
}

INLINE void OdeHinge2Joint::
set_axis2(const LVecBase3f &axis) {
  dJointSetHinge2Axis2(_id, axis[0], axis[1], axis[2]);
}

INLINE void OdeHinge2Joint::
add_torques(dReal torque1, dReal torque2) {
  dJointAddHinge2Torques(_id, torque1, torque2);
}

INLINE LVecBase3f OdeHinge2Joint::
get_anchor() const {
  dVector3 result;
  dJointGetHinge2Anchor(_id, result);
  return LVecBase3f(result[0], result[1], result[2]);
}

INLINE LVecBase3f OdeHinge2Joint::
get_anchor2() const {
  dVector3 result;
  dJointGetHinge2Anchor2(_id, result);
  return LVecBase3f(result[0], result[1], result[2]);
}

INLINE LVecBase3f OdeHinge2Joint::
get_axis1() const {
  dVector3 result;
  dJointGetHinge2Axis1(_id, result);
  return LVecBase3f(result[0], result[1], result[2]);
}

INLINE LVecBase3f OdeHinge2Joint::
get_axis2() const {
  dVector3 result;
  dJointGetHinge2Axis2(_id, result);
  return LVecBase3f(result[0], result[1], result[2]);
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

INLINE void OdeHinge2Joint::
set_param_lo_stop(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 1 );
  if ( axis == 0 ) {
    dJointSetHinge2Param(_id, dParamLoStop, val);
  } else if ( axis == 1 ) {
    dJointSetHinge2Param(_id, dParamLoStop2, val);
  }
}

INLINE void OdeHinge2Joint::
set_param_hi_stop(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 1 );
  if ( axis == 0 ) {
    dJointSetHinge2Param(_id, dParamHiStop, val);
  } else if ( axis == 1 ) {
    dJointSetHinge2Param(_id, dParamHiStop2, val);
  }
}

INLINE void OdeHinge2Joint::
set_param_vel(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 1 );
  if ( axis == 0 ) {
    dJointSetHinge2Param(_id, dParamVel, val);
  } else if ( axis == 1 ) {
    dJointSetHinge2Param(_id, dParamVel2, val);
  }
}

INLINE void OdeHinge2Joint::
set_param_f_max(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 1 );
  if ( axis == 0 ) {
    dJointSetHinge2Param(_id, dParamFMax, val);
  } else if ( axis == 1 ) {
    dJointSetHinge2Param(_id, dParamFMax2, val);
  }
}

INLINE void OdeHinge2Joint::
set_param_fudge_factor(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 1 );
  if ( axis == 0 ) {
    dJointSetHinge2Param(_id, dParamFudgeFactor, val);
  } else if ( axis == 1 ) {
    dJointSetHinge2Param(_id, dParamFudgeFactor2, val);
  }
}

INLINE void OdeHinge2Joint::
set_param_bounce(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 1 );
  if ( axis == 0 ) {
    dJointSetHinge2Param(_id, dParamBounce, val);
  } else if ( axis == 1 ) {
    dJointSetHinge2Param(_id, dParamBounce2, val);
  }
}

INLINE void OdeHinge2Joint::
set_param_CFM(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 1 );
  if ( axis == 0 ) {
    dJointSetHinge2Param(_id, dParamCFM, val);
  } else if ( axis == 1 ) {
    dJointSetHinge2Param(_id, dParamCFM2, val);
  }
}

INLINE void OdeHinge2Joint::
set_param_stop_ERP(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 1 );
  if ( axis == 0 ) {
    dJointSetHinge2Param(_id, dParamStopERP, val);
  } else if ( axis == 1 ) {
    dJointSetHinge2Param(_id, dParamStopERP2, val);
  }
}

INLINE void OdeHinge2Joint::
set_param_stop_CFM(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 1 );
  if ( axis == 0 ) {
    dJointSetHinge2Param(_id, dParamStopCFM, val);
  } else if ( axis == 1 ) {
    dJointSetHinge2Param(_id, dParamStopCFM2, val);
  }
}

INLINE void OdeHinge2Joint::
set_param_suspension_ERP(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 1 );
  if ( axis == 0 ) {
    dJointSetHinge2Param(_id, dParamSuspensionERP, val);
  } else if ( axis == 1 ) {
    dJointSetHinge2Param(_id, dParamSuspensionERP2, val);
  }
}

INLINE void OdeHinge2Joint::
set_param_suspension_CFM(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 1 );
  if ( axis == 0 ) {
    dJointSetHinge2Param(_id, dParamSuspensionCFM, val);
  } else if ( axis == 1 ) {
    dJointSetHinge2Param(_id, dParamSuspensionCFM2, val);
  }
}

INLINE dReal OdeHinge2Joint::
get_param_lo_stop(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 1, 0 );
  if ( axis == 0 ) {
    return dJointGetHinge2Param(_id, dParamLoStop);
  } else if ( axis == 1 ) {
    return dJointGetHinge2Param(_id, dParamLoStop2);
  }
  return 0;
}

INLINE dReal OdeHinge2Joint::
get_param_hi_stop(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 1, 0 );
  if ( axis == 0 ) {
    return dJointGetHinge2Param(_id, dParamHiStop);
  } else if ( axis == 1 ) {
    return dJointGetHinge2Param(_id, dParamHiStop2);
  }
  return 0;
}

INLINE dReal OdeHinge2Joint::
get_param_vel(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 1, 0 );
  if ( axis == 0 ) {
    return dJointGetHinge2Param(_id, dParamVel);
  } else if ( axis == 1 ) {
    return dJointGetHinge2Param(_id, dParamVel2);
  }
  return 0;
}

INLINE dReal OdeHinge2Joint::
get_param_f_max(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 1, 0 );
  if ( axis == 0 ) {
    return dJointGetHinge2Param(_id, dParamFMax);
  } else if ( axis == 1 ) {
    return dJointGetHinge2Param(_id, dParamFMax2);
  }
  return 0;
}

INLINE dReal OdeHinge2Joint::
get_param_fudge_factor(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 1, 0 );
  if ( axis == 0 ) {
    return dJointGetHinge2Param(_id, dParamFudgeFactor);
  } else if ( axis == 1 ) {
    return dJointGetHinge2Param(_id, dParamFudgeFactor2);
  }
  return 0;
}

INLINE dReal OdeHinge2Joint::
get_param_bounce(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 1, 0 );
  if ( axis == 0 ) {
    return dJointGetHinge2Param(_id, dParamBounce);
  } else if ( axis == 1 ) {
    return dJointGetHinge2Param(_id, dParamBounce2);
  }
  return 0;
}

INLINE dReal OdeHinge2Joint::
get_param_CFM(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 1, 0 );
  if ( axis == 0 ) {
    return dJointGetHinge2Param(_id, dParamCFM);
  } else if ( axis == 1 ) {
    return dJointGetHinge2Param(_id, dParamCFM2);
  }
  return 0;
}

INLINE dReal OdeHinge2Joint::
get_param_stop_ERP(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 1, 0 );
  if ( axis == 0 ) {
    return dJointGetHinge2Param(_id, dParamStopERP);
  } else if ( axis == 1 ) {
    return dJointGetHinge2Param(_id, dParamStopERP2);
  }
  return 0;
}

INLINE dReal OdeHinge2Joint::
get_param_stop_CFM(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 1, 0 );
  if ( axis == 0 ) {
    return dJointGetHinge2Param(_id, dParamStopCFM);
  } else if ( axis == 1 ) {
    return dJointGetHinge2Param(_id, dParamStopCFM2);
  }
  return 0;
}

INLINE dReal OdeHinge2Joint::
get_param_suspension_ERP(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 1, 0 );
  if ( axis == 0 ) {
    return dJointGetHinge2Param(_id, dParamSuspensionERP);
  } else if ( axis == 1 ) {
    return dJointGetHinge2Param(_id, dParamSuspensionERP2);
  }
  return 0;
}

INLINE dReal OdeHinge2Joint::
get_param_suspension_CFM(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 1, 0 );
  if ( axis == 0 ) {
    return dJointGetHinge2Param(_id, dParamSuspensionCFM);
  } else if ( axis == 1 ) {
    return dJointGetHinge2Param(_id, dParamSuspensionCFM2);
  }
  return 0;
}


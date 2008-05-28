// Filename: odeAMotorJoint.I
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

INLINE void OdeAMotorJoint::
set_num_axes(int num) {
  dJointSetAMotorNumAxes(_id, num);
}

INLINE void OdeAMotorJoint::
set_axis(int anum, int rel, dReal x, dReal y, dReal z) {
  dJointSetAMotorAxis(_id, anum, rel, x, y, z);
}

INLINE void OdeAMotorJoint::
set_axis(int anum, int rel, const LVecBase3f &axis) {
  dJointSetAMotorAxis(_id, anum, rel, axis[0], axis[1], axis[2]);
}

INLINE void OdeAMotorJoint::
set_angle(int anum, dReal angle) {
  dJointSetAMotorAngle(_id, anum, angle);
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

INLINE LVecBase3f OdeAMotorJoint::
get_axis(int anum) const {
  dVector3 result;
  dJointGetAMotorAxis(_id, anum, result);
  return LVecBase3f(result[0], result[1], result[2]);
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

INLINE int OdeAMotorJoint::
get_mode() const {
  return dJointGetAMotorMode(_id);
}

INLINE void OdeAMotorJoint::
set_param_lo_stop(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 2 );
  if ( axis == 0 ) {
    dJointSetAMotorParam(_id, dParamLoStop, val);
  } else if ( axis == 1 ) {
    dJointSetAMotorParam(_id, dParamLoStop, val);
  } else if ( axis == 2 ) {
    dJointSetAMotorParam(_id, dParamLoStop, val);
  }
}

INLINE void OdeAMotorJoint::
set_param_hi_stop(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 2 );
  if ( axis == 0 ) {
    dJointSetAMotorParam(_id, dParamHiStop, val);
  } else if ( axis == 1 ) {
    dJointSetAMotorParam(_id, dParamHiStop, val);
  } else if ( axis == 2 ) {
    dJointSetAMotorParam(_id, dParamHiStop, val);
  }
}

INLINE void OdeAMotorJoint::
set_param_vel(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 2 );
  if ( axis == 0 ) {
    dJointSetAMotorParam(_id, dParamVel, val);
  } else if ( axis == 1 ) {
    dJointSetAMotorParam(_id, dParamVel, val);
  } else if ( axis == 2 ) {
    dJointSetAMotorParam(_id, dParamVel, val);
  }
}

INLINE void OdeAMotorJoint::
set_param_f_max(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 2 );
  if ( axis == 0 ) {
    dJointSetAMotorParam(_id, dParamFMax, val);
  } else if ( axis == 1 ) {
    dJointSetAMotorParam(_id, dParamFMax, val);
  } else if ( axis == 2 ) {
    dJointSetAMotorParam(_id, dParamFMax, val);
  }
}

INLINE void OdeAMotorJoint::
set_param_fudge_factor(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 2 );
  if ( axis == 0 ) {
    dJointSetAMotorParam(_id, dParamFudgeFactor, val);
  } else if ( axis == 1 ) {
    dJointSetAMotorParam(_id, dParamFudgeFactor, val);
  } else if ( axis == 2 ) {
    dJointSetAMotorParam(_id, dParamFudgeFactor, val);
  }
}

INLINE void OdeAMotorJoint::
set_param_bounce(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 2 );
  if ( axis == 0 ) {
    dJointSetAMotorParam(_id, dParamBounce, val);
  } else if ( axis == 1 ) {
    dJointSetAMotorParam(_id, dParamBounce, val);
  } else if ( axis == 2 ) {
    dJointSetAMotorParam(_id, dParamBounce, val);
  }
}

INLINE void OdeAMotorJoint::
set_param_CFM(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 2 );
  if ( axis == 0 ) {
    dJointSetAMotorParam(_id, dParamCFM, val);
  } else if ( axis == 1 ) {
    dJointSetAMotorParam(_id, dParamCFM, val);
  } else if ( axis == 2 ) {
    dJointSetAMotorParam(_id, dParamCFM, val);
  }
}

INLINE void OdeAMotorJoint::
set_param_stop_ERP(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 2 );
  if ( axis == 0 ) {
    dJointSetAMotorParam(_id, dParamStopERP, val);
  } else if ( axis == 1 ) {
    dJointSetAMotorParam(_id, dParamStopERP, val);
  } else if ( axis == 2 ) {
    dJointSetAMotorParam(_id, dParamStopERP, val);
  }
}

INLINE void OdeAMotorJoint::
set_param_stop_CFM(int axis, dReal val) {
  nassertv( _id != 0 );
  nassertv( 0 <= axis && axis <= 2 );
  if ( axis == 0 ) {
    dJointSetAMotorParam(_id, dParamStopCFM, val);
  } else if ( axis == 1 ) {
    dJointSetAMotorParam(_id, dParamStopCFM, val);
  } else if ( axis == 2 ) {
    dJointSetAMotorParam(_id, dParamStopCFM, val);
  }
}

INLINE dReal OdeAMotorJoint::
get_param_lo_stop(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 2, 0 );
  if ( axis == 0 ) {
    return dJointGetAMotorParam(_id, dParamLoStop);
  } else if ( axis == 1 ) {
    return dJointGetAMotorParam(_id, dParamLoStop);
  } else if ( axis == 2 ) {
    return dJointGetAMotorParam(_id, dParamLoStop);
  }
  return 0;
}

INLINE dReal OdeAMotorJoint::
get_param_hi_stop(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 2, 0 );
  if ( axis == 0 ) {
    return dJointGetAMotorParam(_id, dParamHiStop);
  } else if ( axis == 1 ) {
    return dJointGetAMotorParam(_id, dParamHiStop);
  } else if ( axis == 2 ) {
    return dJointGetAMotorParam(_id, dParamHiStop);
  }
  return 0;
}

INLINE dReal OdeAMotorJoint::
get_param_vel(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 2, 0 );
  if ( axis == 0 ) {
    return dJointGetAMotorParam(_id, dParamVel);
  } else if ( axis == 1 ) {
    return dJointGetAMotorParam(_id, dParamVel);
  } else if ( axis == 2 ) {
    return dJointGetAMotorParam(_id, dParamVel);
  }
  return 0;
}

INLINE dReal OdeAMotorJoint::
get_param_f_max(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 2, 0 );
  if ( axis == 0 ) {
    return dJointGetAMotorParam(_id, dParamFMax);
  } else if ( axis == 1 ) {
    return dJointGetAMotorParam(_id, dParamFMax);
  } else if ( axis == 2 ) {
    return dJointGetAMotorParam(_id, dParamFMax);
  }
  return 0;
}

INLINE dReal OdeAMotorJoint::
get_param_fudge_factor(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 2, 0 );
  if ( axis == 0 ) {
    return dJointGetAMotorParam(_id, dParamFudgeFactor);
  } else if ( axis == 1 ) {
    return dJointGetAMotorParam(_id, dParamFudgeFactor);
  } else if ( axis == 2 ) {
    return dJointGetAMotorParam(_id, dParamFudgeFactor);
  }
  return 0;
}

INLINE dReal OdeAMotorJoint::
get_param_bounce(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 2, 0 );
  if ( axis == 0 ) {
    return dJointGetAMotorParam(_id, dParamBounce);
  } else if ( axis == 1 ) {
    return dJointGetAMotorParam(_id, dParamBounce);
  } else if ( axis == 2 ) {
    return dJointGetAMotorParam(_id, dParamBounce);
  }
  return 0;
}

INLINE dReal OdeAMotorJoint::
get_param_CFM(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 2, 0 );
  if ( axis == 0 ) {
    return dJointGetAMotorParam(_id, dParamCFM);
  } else if ( axis == 1 ) {
    return dJointGetAMotorParam(_id, dParamCFM);
  } else if ( axis == 2 ) {
    return dJointGetAMotorParam(_id, dParamCFM);
  }
  return 0;
}

INLINE dReal OdeAMotorJoint::
get_param_stop_ERP(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 2, 0 );
  if ( axis == 0 ) {
    return dJointGetAMotorParam(_id, dParamStopERP);
  } else if ( axis == 1 ) {
    return dJointGetAMotorParam(_id, dParamStopERP);
  } else if ( axis == 2 ) {
    return dJointGetAMotorParam(_id, dParamStopERP);
  }
  return 0;
}

INLINE dReal OdeAMotorJoint::
get_param_stop_CFM(int axis) const {
  nassertr( _id != 0, 0 );
  nassertr( 0 <= axis && axis <= 2, 0 );
  if ( axis == 0 ) {
    return dJointGetAMotorParam(_id, dParamStopCFM);
  } else if ( axis == 1 ) {
    return dJointGetAMotorParam(_id, dParamStopCFM);
  } else if ( axis == 2 ) {
    return dJointGetAMotorParam(_id, dParamStopCFM);
  }
  return 0;
}


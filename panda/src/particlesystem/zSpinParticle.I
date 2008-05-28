// Filename: zSpinParticle.I
// Created by:  charles (16Aug00)
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

////////////////////////////////////////////////////////////////////
//    Function : set_initial_angle
// Description : accessor
////////////////////////////////////////////////////////////////////
INLINE void ZSpinParticle::
set_initial_angle(float t) {
  _initial_angle = t;
}

////////////////////////////////////////////////////////////////////
//    Function : get_initial_angle
// Description : accessor
////////////////////////////////////////////////////////////////////
INLINE float ZSpinParticle::
get_initial_angle() const {
  return _initial_angle;
}

////////////////////////////////////////////////////////////////////
//    Function : set_final_angle
// Description : accessor
////////////////////////////////////////////////////////////////////
INLINE void ZSpinParticle::
set_final_angle(float t) {
  _final_angle = t;
}

////////////////////////////////////////////////////////////////////
//    Function : get_final_angle
// Description : accessor
////////////////////////////////////////////////////////////////////
INLINE float ZSpinParticle::
get_final_angle() const {
  return _final_angle;
}

INLINE float ZSpinParticle::
get_angular_velocity() const {
  return _angular_velocity;
}

INLINE void ZSpinParticle::
set_angular_velocity(float v) {
  _angular_velocity = v;
}

INLINE void ZSpinParticle::
enable_angular_velocity(bool bEnabled) {
  _bUseAngularVelocity = bEnabled;
}

INLINE bool ZSpinParticle::
get_angular_velocity_enabled() const {
  return _bUseAngularVelocity;
}



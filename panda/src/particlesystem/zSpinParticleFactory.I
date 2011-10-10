// Filename: zSpinParticleFactory.I
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
// Function : set_initial_angle
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE void ZSpinParticleFactory::
set_initial_angle(PN_stdfloat angle) {
  _initial_angle = angle;
}

////////////////////////////////////////////////////////////////////
// Function : set_final_angle
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE void ZSpinParticleFactory::
set_final_angle(PN_stdfloat angle) {
  _final_angle = angle;
}

////////////////////////////////////////////////////////////////////
// Function : set_initial_angle_spread
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE void ZSpinParticleFactory::
set_initial_angle_spread(PN_stdfloat spread) {
  _initial_angle_spread = spread;
}

////////////////////////////////////////////////////////////////////
// Function : set_final_angle_spread
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE void ZSpinParticleFactory::
set_final_angle_spread(PN_stdfloat spread) {
  _final_angle_spread = spread;
}

////////////////////////////////////////////////////////////////////
// Function : get_initial_angle
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat ZSpinParticleFactory::
get_initial_angle() const {
  return _initial_angle;
}

////////////////////////////////////////////////////////////////////
// Function : get_final_angle
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat ZSpinParticleFactory::
get_final_angle() const {
  return _final_angle;
}

////////////////////////////////////////////////////////////////////
// Function : get_initial_angle_spread
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat ZSpinParticleFactory::
get_initial_angle_spread() const {
  return _initial_angle_spread;
}

////////////////////////////////////////////////////////////////////
// Function : get_final_angle_spread
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat ZSpinParticleFactory::
get_final_angle_spread() const {
  return _final_angle_spread;
}

////////////////////////////////////////////////////////////////////
// Function : get_angular_velocity
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat ZSpinParticleFactory::
get_angular_velocity() const {
  return _angular_velocity;
}

INLINE void ZSpinParticleFactory::
set_angular_velocity(PN_stdfloat v) {
  _angular_velocity = v;
}

INLINE PN_stdfloat ZSpinParticleFactory::
get_angular_velocity_spread() const {
  return _angular_velocity_spread;
}

INLINE void ZSpinParticleFactory::
set_angular_velocity_spread(PN_stdfloat spread) {
  _angular_velocity_spread = spread;
}


INLINE void ZSpinParticleFactory::
enable_angular_velocity(bool bEnabled) {
  _bUseAngularVelocity = bEnabled;
}

INLINE bool ZSpinParticleFactory::
get_angular_velocity_enabled() const {
  return _bUseAngularVelocity;
}


// Filename: zSpinParticleFactory.I
// Created by:  charles (16Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
// Function : set_initial_angle
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE void ZSpinParticleFactory::
set_initial_angle(float angle) {
  _initial_angle = angle;
}

////////////////////////////////////////////////////////////////////
// Function : set_final_angle
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE void ZSpinParticleFactory::
set_final_angle(float angle) {
  _final_angle = angle;
}

////////////////////////////////////////////////////////////////////
// Function : set_initial_angle_spread
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE void ZSpinParticleFactory::
set_initial_angle_spread(float spread) {
  _initial_angle_spread = spread;
}

////////////////////////////////////////////////////////////////////
// Function : set_final_angle_spread
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE void ZSpinParticleFactory::
set_final_angle_spread(float spread) {
  _final_angle_spread = spread;
}

////////////////////////////////////////////////////////////////////
// Function : get_initial_angle
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE float ZSpinParticleFactory::
get_initial_angle() const {
  return _initial_angle;
}

////////////////////////////////////////////////////////////////////
// Function : get_final_angle
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE float ZSpinParticleFactory::
get_final_angle() const {
  return _final_angle;
}

////////////////////////////////////////////////////////////////////
// Function : get_initial_angle_spread
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE float ZSpinParticleFactory::
get_initial_angle_spread() const {
  return _initial_angle_spread;
}

////////////////////////////////////////////////////////////////////
// Function : get_final_angle_spread
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE float ZSpinParticleFactory::
get_final_angle_spread() const {
  return _final_angle_spread;
}

////////////////////////////////////////////////////////////////////
// Function : get_angular_velocity
//   Access : public
////////////////////////////////////////////////////////////////////
INLINE float ZSpinParticleFactory::
get_angular_velocity() const {
  return _angular_velocity;
}

INLINE void ZSpinParticleFactory::
set_angular_velocity(float v) {
  _angular_velocity = v;
}

INLINE float ZSpinParticleFactory::
get_angular_velocity_spread() const {
  return _angular_velocity_spread;
}

INLINE void ZSpinParticleFactory::
set_angular_velocity_spread(float spread) {
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


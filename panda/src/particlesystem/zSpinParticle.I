// Filename: zSpinParticle.I
// Created by:  charles (16Aug00)
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



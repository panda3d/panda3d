// Filename: physicsCollisionHandler.I
// Created by:  drose (16Mar02)
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


INLINE void PhysicsCollisionHandler::
set_almost_stationary_speed(float speed) {
  _almost_stationary_speed = speed;
}

INLINE float PhysicsCollisionHandler::
get_almost_stationary_speed() {
  return _almost_stationary_speed;
}

INLINE void PhysicsCollisionHandler::
set_static_friction_coef(float coef) {
  _static_friction_coef = coef;
}

INLINE float PhysicsCollisionHandler::
get_static_friction_coef() {
  return _static_friction_coef;
}

INLINE void PhysicsCollisionHandler::
set_dynamic_friction_coef(float coef) {
  _dynamic_friction_coef = coef;
}

INLINE float PhysicsCollisionHandler::
get_dynamic_friction_coef() {
  return _dynamic_friction_coef;
}



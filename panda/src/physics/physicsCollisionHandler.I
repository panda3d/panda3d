// Filename: physicsCollisionHandler.I
// Created by:  drose (16Mar02)
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


INLINE void PhysicsCollisionHandler::
set_almost_stationary_speed(PN_stdfloat speed) {
  _almost_stationary_speed = speed;
}

INLINE PN_stdfloat PhysicsCollisionHandler::
get_almost_stationary_speed() {
  return _almost_stationary_speed;
}

INLINE void PhysicsCollisionHandler::
set_static_friction_coef(PN_stdfloat coef) {
  _static_friction_coef = coef;
}

INLINE PN_stdfloat PhysicsCollisionHandler::
get_static_friction_coef() {
  return _static_friction_coef;
}

INLINE void PhysicsCollisionHandler::
set_dynamic_friction_coef(PN_stdfloat coef) {
  _dynamic_friction_coef = coef;
}

INLINE PN_stdfloat PhysicsCollisionHandler::
get_dynamic_friction_coef() {
  return _dynamic_friction_coef;
}



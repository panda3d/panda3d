// Filename: physicsObject.I
// Created by:  charles (13Jun00)
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
//    Function : set_mass
//      Access : Public
// Description : Set the mass in slugs (or kilograms).
////////////////////////////////////////////////////////////////////
INLINE void PhysicsObject::
set_mass(float m) {
  _mass = m;
}

////////////////////////////////////////////////////////////////////
//    Function : set_position
//      Access : Public
// Description : Vector position assignment
////////////////////////////////////////////////////////////////////
INLINE void PhysicsObject::
set_position(const LPoint3f &pos) {
  nassertv(!pos.is_nan());
  _position = pos;
}

////////////////////////////////////////////////////////////////////
//    Function : set_position
//      Access : Public
// Description : Piecewise position assignment
////////////////////////////////////////////////////////////////////
INLINE void PhysicsObject::
set_position(float x, float y, float z) {
  nassertv(!LPoint3f(x, y, z).is_nan());
  _position.set(x, y, z);
}

////////////////////////////////////////////////////////////////////
//    Function : reset_position
//      Access : Public
// Description : use this to place an object in a completely new
//               position, that has nothing to do with its last
//               position.
////////////////////////////////////////////////////////////////////
INLINE void PhysicsObject::
reset_position(const LPoint3f &pos) {
  nassertv(!pos.is_nan());
  _position = pos;
  _last_position = pos;
  _velocity.set(0.0f, 0.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////
//    Function : reset_orientation
//      Access : Public
// Description : set the orientation while clearing the rotation
//               velocity.
////////////////////////////////////////////////////////////////////
INLINE void PhysicsObject::
reset_orientation(const LOrientationf &orientation) {
  nassertv(!orientation.is_nan());
  _orientation = orientation;
  _rotation.set(0.0f, 0.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////
//    Function : set_last_position
//      Access : Public
// Description : Last position assignment
////////////////////////////////////////////////////////////////////
INLINE void PhysicsObject::
set_last_position(const LPoint3f &pos) {
  _last_position = pos;
}

////////////////////////////////////////////////////////////////////
//    Function : set_velocity
//      Access : Public
// Description : Vector velocity assignment
////////////////////////////////////////////////////////////////////
INLINE void PhysicsObject::
set_velocity(const LVector3f &vel) {
  nassertv(!vel.is_nan());
  _velocity = vel;
}

////////////////////////////////////////////////////////////////////
//    Function : set_velocity
//      Access : Public
// Description : Piecewise velocity assignment
////////////////////////////////////////////////////////////////////
INLINE void PhysicsObject::
set_velocity(float x, float y, float z) {
  nassertv(!LVector3f(x, y, z).is_nan());
  _velocity.set(x, y, z);
}

////////////////////////////////////////////////////////////////////
//    Function : add_impulse
//      Access : Public
// Description : Adds an impulse force (i.e. an instantanious change
//               in velocity).  This is a quicker way to get the 
//               velocity, add a vector to it and set that value to
//               be the new velocity.
////////////////////////////////////////////////////////////////////
INLINE void PhysicsObject::
add_impulse(const LVector3f &impulse) {
  _velocity+=impulse;
}

////////////////////////////////////////////////////////////////////
//    Function : set_active
//      Access : Public
// Description : Process Flag assignment
////////////////////////////////////////////////////////////////////
INLINE void PhysicsObject::
set_active(bool flag) {
  _process_me = flag;
}

////////////////////////////////////////////////////////////////////
//    Function : set_terminal_velocity
//      Access : Public
// Description : tv assignment
////////////////////////////////////////////////////////////////////
INLINE void PhysicsObject::
set_terminal_velocity(float tv) {
  _terminal_velocity = tv;
}

////////////////////////////////////////////////////////////////////
//    Function : get_mass
//      Access : Public
// Description : Get the mass in slugs (or kilograms).
////////////////////////////////////////////////////////////////////
INLINE float PhysicsObject::
get_mass() const {
  return _mass;
}

////////////////////////////////////////////////////////////////////
//    Function : get_position
//      Access : Public
// Description : Position Query
////////////////////////////////////////////////////////////////////
INLINE LPoint3f PhysicsObject::
get_position() const {
  return _position;
}

////////////////////////////////////////////////////////////////////
//    Function : get_last_position
//      Access : Public
// Description : Get the position of the physics object at the start
//               of the most recent do_physics.
////////////////////////////////////////////////////////////////////
INLINE LPoint3f PhysicsObject::
get_last_position() const {
  return _last_position;
}

////////////////////////////////////////////////////////////////////
//    Function : get_velocity
//      Access : Public
// Description : Velocity Query per second
////////////////////////////////////////////////////////////////////
INLINE LVector3f PhysicsObject::
get_velocity() const {
  return _velocity;
}

////////////////////////////////////////////////////////////////////
//    Function : get_implicit_velocity
//      Access : Public
// Description : Velocity Query over the last dt
////////////////////////////////////////////////////////////////////
INLINE LVector3f PhysicsObject::
get_implicit_velocity() const {
  return _position-_last_position;
}

////////////////////////////////////////////////////////////////////
//    Function : get_active
//      Access : Public
// Description : Process Flag Query
////////////////////////////////////////////////////////////////////
INLINE bool PhysicsObject::
get_active() const {
  return _process_me;
}

////////////////////////////////////////////////////////////////////
//    Function : get_terminal_velocity
//      Access : Public
// Description : tv query
////////////////////////////////////////////////////////////////////
INLINE float PhysicsObject::
get_terminal_velocity() const {
  return _terminal_velocity;
}

////////////////////////////////////////////////////////////////////
//    Function : set_orientation
//      Access : Public
// Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysicsObject::
set_orientation(const LOrientationf &orientation) {
  nassertv(!orientation.is_nan());
  _orientation = orientation;
}

////////////////////////////////////////////////////////////////////
//    Function : set_rotation
//      Access : Public
// Description : set rotation on each axis per second.
////////////////////////////////////////////////////////////////////
INLINE void PhysicsObject::
set_rotation(const LVector3f &rotation) {
  nassertv(!rotation.is_nan());
  _rotation = rotation;
}

////////////////////////////////////////////////////////////////////
//    Function : get_orientation
//      Access : Public
// Description : get current orientation.
////////////////////////////////////////////////////////////////////
INLINE LOrientationf PhysicsObject::
get_orientation() const {
  return _orientation;
}

////////////////////////////////////////////////////////////////////
//    Function : get_rotation
//      Access : Public
// Description : get rotation per second.
////////////////////////////////////////////////////////////////////
INLINE LVector3f PhysicsObject::
get_rotation() const {
  return _rotation;
}

////////////////////////////////////////////////////////////////////
//    Function : set_oriented
//      Access : Public
// Description : Set flag to determine whether this object should do
//               any rotation or orientation calculations.  Optimization.
////////////////////////////////////////////////////////////////////
INLINE void PhysicsObject::
set_oriented(bool flag) {
  _oriented = flag;
}

////////////////////////////////////////////////////////////////////
//    Function : get_oriented
//      Access : Public
// Description : See set_oriented().
////////////////////////////////////////////////////////////////////
INLINE bool PhysicsObject::
get_oriented() const {
  return _oriented;
}

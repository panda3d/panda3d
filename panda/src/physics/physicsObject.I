// Filename: physicsObject.I
// Created by:  charles (13Jun00)
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
//    Function : set_mass
//      Access : Public
// Description : Set the mass in slugs (or kilograms).
////////////////////////////////////////////////////////////////////
INLINE void PhysicsObject::
set_mass(PN_stdfloat m) {
  nassertv(m > 0);
  _mass = m;
}

////////////////////////////////////////////////////////////////////
//    Function : set_position
//      Access : Public
// Description : Vector position assignment.  This is also used as
//               the center of mass.
////////////////////////////////////////////////////////////////////
INLINE void PhysicsObject::
set_position(const LPoint3 &pos) {
  nassertv(!pos.is_nan());
  _position = pos;
}

////////////////////////////////////////////////////////////////////
//    Function : set_position
//      Access : Public
// Description : Piecewise position assignment
////////////////////////////////////////////////////////////////////
INLINE void PhysicsObject::
set_position(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  nassertv(!LPoint3(x, y, z).is_nan());
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
reset_position(const LPoint3 &pos) {
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
reset_orientation(const LOrientation &orientation) {
  nassertv(!orientation.is_nan());
  _orientation = orientation;
  _rotation = LRotation::ident_quat();
}

////////////////////////////////////////////////////////////////////
//    Function : set_last_position
//      Access : Public
// Description : Last position assignment
////////////////////////////////////////////////////////////////////
INLINE void PhysicsObject::
set_last_position(const LPoint3 &pos) {
  _last_position = pos;
}

////////////////////////////////////////////////////////////////////
//    Function : set_velocity
//      Access : Public
// Description : Vector velocity assignment
////////////////////////////////////////////////////////////////////
INLINE void PhysicsObject::
set_velocity(const LVector3 &vel) {
  nassertv(!vel.is_nan());
  _velocity = vel;
}

////////////////////////////////////////////////////////////////////
//    Function : set_velocity
//      Access : Public
// Description : Piecewise velocity assignment
////////////////////////////////////////////////////////////////////
INLINE void PhysicsObject::
set_velocity(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  nassertv(!LVector3(x, y, z).is_nan());
  _velocity.set(x, y, z);
}

////////////////////////////////////////////////////////////////////
//    Function : add_local_torque
//      Access : Public
// Description : Adds an torque force (i.e. an instantanious change
//               in velocity).  This is a quicker way to get the 
//               angular velocity, add a vector to it and set that
//               value to be the new angular velocity.
////////////////////////////////////////////////////////////////////
INLINE void PhysicsObject::
add_local_torque(const LRotation &torque) {
  nassertv(!torque.is_nan());
  _rotation+=_orientation.xform(torque);
}

////////////////////////////////////////////////////////////////////
//    Function : add_local_impulse
//      Access : Public
// Description : Adds an impulse force (i.e. an instantanious change
//               in velocity).  This is a quicker way to get the 
//               velocity, add a vector to it and set that value to
//               be the new velocity.
////////////////////////////////////////////////////////////////////
INLINE void PhysicsObject::
add_local_impulse(const LVector3 &impulse) {
  nassertv(!impulse.is_nan());
  _velocity += _orientation.xform(impulse);
}

////////////////////////////////////////////////////////////////////
//    Function : add_torque
//      Access : Public
// Description : Adds an torque force (i.e. an instantanious change
//               in velocity).  This is a quicker way to get the 
//               angular velocity, add a vector to it and set that
//               value to be the new angular velocity.
////////////////////////////////////////////////////////////////////
INLINE void PhysicsObject::
add_torque(const LRotation &torque) {
  nassertv(!torque.is_nan());
  _rotation+=torque;
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
add_impulse(const LVector3 &impulse) {
  nassertv(!impulse.is_nan());
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
set_terminal_velocity(PN_stdfloat tv) {
  _terminal_velocity = tv;
}

////////////////////////////////////////////////////////////////////
//    Function : get_mass
//      Access : Public
// Description : Get the mass in slugs (or kilograms).
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat PhysicsObject::
get_mass() const {
  return _mass;
}

////////////////////////////////////////////////////////////////////
//    Function : get_position
//      Access : Public
// Description : Position Query
////////////////////////////////////////////////////////////////////
INLINE LPoint3 PhysicsObject::
get_position() const {
  return _position;
}

////////////////////////////////////////////////////////////////////
//    Function : get_last_position
//      Access : Public
// Description : Get the position of the physics object at the start
//               of the most recent do_physics.
////////////////////////////////////////////////////////////////////
INLINE LPoint3 PhysicsObject::
get_last_position() const {
  return _last_position;
}

////////////////////////////////////////////////////////////////////
//    Function : get_velocity
//      Access : Public
// Description : Velocity Query per second
////////////////////////////////////////////////////////////////////
INLINE LVector3 PhysicsObject::
get_velocity() const {
  return _velocity;
}

////////////////////////////////////////////////////////////////////
//    Function : get_implicit_velocity
//      Access : Public
// Description : Velocity Query over the last dt
////////////////////////////////////////////////////////////////////
INLINE LVector3 PhysicsObject::
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
INLINE PN_stdfloat PhysicsObject::
get_terminal_velocity() const {
  return _terminal_velocity;
}

////////////////////////////////////////////////////////////////////
//    Function : set_orientation
//      Access : Public
// Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysicsObject::
set_orientation(const LOrientation &orientation) {
  nassertv(!orientation.is_nan());
  _orientation = orientation;
}

////////////////////////////////////////////////////////////////////
//    Function : set_rotation
//      Access : Public
// Description : set rotation as a quaternion delta per second.
////////////////////////////////////////////////////////////////////
INLINE void PhysicsObject::
set_rotation(const LRotation &rotation) {
  nassertv(!rotation.is_nan());
  _rotation = rotation;
}

////////////////////////////////////////////////////////////////////
//    Function : get_orientation
//      Access : Public
// Description : get current orientation.
////////////////////////////////////////////////////////////////////
INLINE LOrientation PhysicsObject::
get_orientation() const {
  return _orientation;
}

////////////////////////////////////////////////////////////////////
//    Function : get_rotation
//      Access : Public
// Description : get rotation per second.
////////////////////////////////////////////////////////////////////
INLINE LRotation PhysicsObject::
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

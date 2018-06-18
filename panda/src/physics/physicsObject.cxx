/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physicsObject.cxx
 * @author charles
 * @date 2000-06-13
 */

#include "physicsObject.h"

ConfigVariableDouble PhysicsObject::_default_terminal_velocity
("default_terminal_velocity", 400.0f);

TypeHandle PhysicsObject::_type_handle;

/**
 * Default Constructor
 */
PhysicsObject::
PhysicsObject() :
  _terminal_velocity(_default_terminal_velocity),
  _mass(1.0f),
  _process_me(false),
  _oriented(true)
{
  _position.set(0.0f, 0.0f, 0.0f);
  _last_position = _position;
  _velocity.set(0.0f, 0.0f, 0.0f);
  _orientation.set(1.0 ,0.0f, 0.0f, 0.0f);
  _rotation = LRotation::ident_quat();
}

/**
 * copy constructor
 */
PhysicsObject::
PhysicsObject(const PhysicsObject& copy) {
  operator=(copy);
}

/**
 * Destructor
 */
PhysicsObject::
~PhysicsObject() {
}

/**
 *
 */
const PhysicsObject &PhysicsObject::
operator =(const PhysicsObject &other) {
  _process_me = other._process_me;
  _mass = other._mass;
  _position = other._position;
  _last_position = other._last_position;
  _velocity = other._velocity;
  _orientation = other._orientation;
  _rotation = other._rotation;
  _terminal_velocity = other._terminal_velocity;
  _oriented = other._oriented;

  return *this;
}

/**
 * dynamic copy.
 */
PhysicsObject *PhysicsObject::
make_copy() const {
  return new PhysicsObject(*this);
}

/**
 * Adds an impulse and/or torque (i.e.  an instantanious change in velocity)
 * based on how well the offset and impulse align with the center of mass (aka
 * position). If you wanted to immitate this function you could work out the
 * impulse and torque and call add_impulse and add_torque respectively.
 * offset and force are in local coordinates.
 */
void PhysicsObject::
add_local_impact(const LPoint3 &offset_from_center_of_mass,
    const LVector3 &force) {
  nassertv(!offset_from_center_of_mass.is_nan());
  nassertv(!force.is_nan());
  add_impact(
      _orientation.xform(offset_from_center_of_mass),
      _orientation.xform(force));
}

/**
 * Adds an impulse and/or torque (i.e.  an instantanious change in velocity)
 * based on how well the offset and impulse align with the center of mass (aka
 * position). If you wanted to immitate this function you could work out the
 * impulse and torque and call add_impulse and add_torque respectively.
 * offset and force are in global (or parent) coordinates.
 */
void PhysicsObject::
add_impact(const LPoint3 &offset,
    const LVector3 &force) {
  nassertv(!offset.is_nan());
  nassertv(!force.is_nan());
  LVector3 a = offset;
  LVector3 b = force;
  a.normalize();
  b.normalize();
  a = a.cross(b);
  PN_stdfloat angle = a.length();
  if (angle) {
    LRotation torque(0, 0, 0, 0);
    PN_stdfloat spin = force.length()*0.1; // todo: this should account for
                                        // impact distance and mass.
    a.normalize();
    assert(IS_THRESHOLD_EQUAL(a.length(), 1.0f, 0.001f));
    torque.set_from_axis_angle(spin, a);
    add_torque(torque);
  }
  LVector3 impulse = (1.0f - angle) * force;
  add_impulse(impulse);
}

/**
 * returns a transform matrix to this object's local coordinate system.
 */
LMatrix4 PhysicsObject::
get_lcs() const {
  LMatrix4 m = LMatrix4::translate_mat(_position);
  if (_oriented) {
    m=m*_orientation;
  }
  nassertr(!m.is_nan(), m);
  return m;
}

/**
 * returns a transform matrix that represents the object's willingness to be
 * forced.
 */
LMatrix4 PhysicsObject::
get_inertial_tensor() const {
  return LMatrix4::ident_mat();
}

/**
 * Write a string representation of this instance to <out>.
 */
void PhysicsObject::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"PhysicsObject";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void PhysicsObject::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent);
  out<<""<<"PhysicsObject "<<_name<<"\n";
  out.width(indent+2); out<<""; out<<"_position "<<_position<<"\n";
  out.width(indent+2); out<<""; out<<"_last_position "<<_last_position<<"\n";
  out.width(indent+2); out<<""; out<<"_velocity "<<_velocity<<"\n";
  out.width(indent+2); out<<""; out<<"(implicit velocity "<<get_implicit_velocity()<<")\n";
  out.width(indent+2); out<<""; out<<"_orientation "<<_orientation<<"\n";
  out.width(indent+2); out<<""; out<<"(hpr "<<_orientation.get_hpr()<<")\n";
  out.width(indent+2); out<<""; out<<"_rotation "<<_rotation<<"\n";
  out.width(indent+2); out<<""; out<<"_terminal_velocity "<<_terminal_velocity<<"\n";
  out.width(indent+2); out<<""; out<<"_mass "<<_mass<<"\n";
  out.width(indent+2); out<<""; out<<"_process_me "<<_process_me<<"\n";
  out.width(indent+2); out<<""; out<<"_oriented "<<_oriented<<"\n";
  #endif //] NDEBUG
}

// Filename: physicsObject.cxx
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

#include "physicsObject.h"

ConfigVariableDouble PhysicsObject::_default_terminal_velocity
("default_terminal_velocity", 400.0f);

TypeHandle PhysicsObject::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function : PhysicsObject
//       Access : Public
//  Description : Default Constructor
////////////////////////////////////////////////////////////////////
PhysicsObject::
PhysicsObject(void) :
  _terminal_velocity(_default_terminal_velocity),
  _mass(1.0f),
  _process_me(false),
  _oriented(true)
{
  _position.set(0.0f, 0.0f, 0.0f);
  _last_position = _position;
  _velocity.set(0.0f, 0.0f, 0.0f);
  _orientation.set(1.0 ,0.0f, 0.0f, 0.0f);
  _rotation.set(0.0f, 0.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function : PhysicsObject
//       Access : Public
//  Description : copy constructor
////////////////////////////////////////////////////////////////////
PhysicsObject::
PhysicsObject(const PhysicsObject& copy) {
  operator=(copy);
}

////////////////////////////////////////////////////////////////////
//     Function : ~PhysicsObject
//       Access : Public
//  Description : Destructor
////////////////////////////////////////////////////////////////////
PhysicsObject::
~PhysicsObject() {
}

////////////////////////////////////////////////////////////////////
//     Function : Assignment operator
//       Access : Public
//  Description :
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function : make_copy
//       Access : Public, virtual
//  Description : dynamic copy.
////////////////////////////////////////////////////////////////////
PhysicsObject *PhysicsObject::
make_copy() const {
  return new PhysicsObject(*this);
}

////////////////////////////////////////////////////////////////////
//     Function : get_lcs
//       Access : Public
//  Description : returns a transform matrix to this object's
//                local coordinate system.
////////////////////////////////////////////////////////////////////
LMatrix4f PhysicsObject::
get_lcs() const {
  LMatrix4f m = LMatrix4f::translate_mat(_position);
  if (_oriented) {
    m=m*_orientation;
  }
  return m;
}

////////////////////////////////////////////////////////////////////
//     Function : get_inertial_tensor
//       Access : Public
//  Description : returns a transform matrix that represents the
//                object's willingness to be forced.
////////////////////////////////////////////////////////////////////
LMatrix4f PhysicsObject::
get_inertial_tensor() const {
  return LMatrix4f::ident_mat();
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void PhysicsObject::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"PhysicsObject";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void PhysicsObject::
write(ostream &out, unsigned int indent) const {
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

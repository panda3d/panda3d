// Filename: physics_object.C
// Created by:  charles (13Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "physicsObject.h"

TypeHandle PhysicsObject::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function : PhysicsObject
//       Access : Public
//  Description : Default Constructor
////////////////////////////////////////////////////////////////////
PhysicsObject::
PhysicsObject(void) :
  _process_me(false), _mass(1.0f), _oriented(true),
  _terminal_velocity(_default_terminal_velocity) {
  _position.set(0, 0, 0);
  _last_position = _position;
  _velocity.set(0, 0, 0);
  _orientation.set(1, 0, 0, 0);
  _rotation.set(0, 0, 0);
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
~PhysicsObject(void) {
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
make_copy(void) const {
  return new PhysicsObject(*this);
}

////////////////////////////////////////////////////////////////////
//     Function : get_lcs
//       Access : Public
//  Description : returns a transform matrix to this object's
//                local coordinate system.
////////////////////////////////////////////////////////////////////
LMatrix4f PhysicsObject::
get_lcs(void) const {
  LMatrix4f m = LMatrix4f::translate_mat(_position);

  if (_oriented == true)
    _orientation.extract_to_matrix(m);

  return m;
}

////////////////////////////////////////////////////////////////////
//     Function : get_inertial_tensor
//       Access : Public
//  Description : returns a transform matrix that represents the
//                object's willingness to be forced.
////////////////////////////////////////////////////////////////////
LMatrix4f PhysicsObject::
get_inertial_tensor(void) const {
  return LMatrix4f::ident_mat();
}

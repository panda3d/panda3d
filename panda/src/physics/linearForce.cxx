// Filename: LinearForce.cxx
// Created by:  charles (14Jun00)
// 
////////////////////////////////////////////////////////////////////

#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

#include "linearForce.h"

TypeHandle LinearForce::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function : LinearForce
//       Access : Protected
//  Description : Default/component-based constructor
////////////////////////////////////////////////////////////////////
LinearForce::
LinearForce(float a, bool mass) :
  BaseForce(true),
  _amplitude(a), _mass_dependent(mass),
  _x_mask(true), _y_mask(true), _z_mask(true) {
}

////////////////////////////////////////////////////////////////////
//     Function : LinearForce
//       Access : Protected
//  Description : copy constructor
////////////////////////////////////////////////////////////////////
LinearForce::
LinearForce(const LinearForce& copy) :
  BaseForce(copy) {
  _amplitude = copy._amplitude;
  _mass_dependent = copy._mass_dependent;
  _x_mask = copy._x_mask;
  _y_mask = copy._y_mask;
  _z_mask = copy._z_mask;
}

////////////////////////////////////////////////////////////////////
//     Function : ~LinearForce
//       Access : Public
//  Description : Destructor
////////////////////////////////////////////////////////////////////
LinearForce::
~LinearForce(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : get_vector
//      Access : Public
////////////////////////////////////////////////////////////////////
LVector3f LinearForce::
get_vector(const PhysicsObject *po) {
  LVector3f child_vector = get_child_vector(po) * _amplitude;

  if (_x_mask == false)
    child_vector[0] = 0.0f;

  if (_y_mask == false)
    child_vector[1] = 0.0f;

  if (_z_mask == false)
    child_vector[2] = 0.0f;

  return child_vector;
}

////////////////////////////////////////////////////////////////////
//    Function : is_linear 
//      Access : Public
////////////////////////////////////////////////////////////////////
bool LinearForce::
is_linear(void) const {
  return true;
}

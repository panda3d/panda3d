/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearForce.cxx
 * @author charles
 * @date 2000-06-14
 */

#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

#include "linearForce.h"

TypeHandle LinearForce::_type_handle;

/**
 * Default/component-based constructor
 */
LinearForce::
LinearForce(PN_stdfloat a, bool mass) :
  BaseForce(true),
  _amplitude(a), _mass_dependent(mass),
  _x_mask(true), _y_mask(true), _z_mask(true) {
}

/**
 * copy constructor
 */
LinearForce::
LinearForce(const LinearForce& copy) :
  BaseForce(copy) {
  _amplitude = copy._amplitude;
  _mass_dependent = copy._mass_dependent;
  _x_mask = copy._x_mask;
  _y_mask = copy._y_mask;
  _z_mask = copy._z_mask;
}

/**
 * Destructor
 */
LinearForce::
~LinearForce() {
}

/**

 */
LVector3 LinearForce::
get_vector(const PhysicsObject *po) {
  LVector3 child_vector = get_child_vector(po) * _amplitude;
  nassertr(!child_vector.is_nan(), LVector3::zero());

  if (_x_mask == false)
    child_vector[0] = 0.0f;

  if (_y_mask == false)
    child_vector[1] = 0.0f;

  if (_z_mask == false)
    child_vector[2] = 0.0f;

  return child_vector;
}

/**

 */
bool LinearForce::
is_linear() const {
  return true;
}

/**
 * Write a string representation of this instance to <out>.
 */
void LinearForce::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"LinearForce (id "<<this<<")";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void LinearForce::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"LinearForce (id "<<this<<")\n";
  out.width(indent+2); out<<""; out<<"_amplitude "<<_amplitude<<"\n";
  out.width(indent+2); out<<""; out<<"_mass_dependent "<<_mass_dependent<<"\n";
  out.width(indent+2); out<<""; out<<"_x_mask "<<_x_mask<<"\n";
  out.width(indent+2); out<<""; out<<"_y_mask "<<_y_mask<<"\n";
  out.width(indent+2); out<<""; out<<"_z_mask "<<_z_mask<<"\n";
  BaseForce::write(out, indent+2);
  #endif //] NDEBUG
}

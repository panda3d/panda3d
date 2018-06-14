/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearJitterForce.cxx
 * @author charles
 * @date 2000-06-16
 */

#include "linearJitterForce.h"

TypeHandle LinearJitterForce::_type_handle;

/**
 * constructor
 */
LinearJitterForce::
LinearJitterForce(PN_stdfloat a, bool mass) :
  LinearRandomForce(a, mass) {
}

/**
 * copy constructor
 */
LinearJitterForce::
LinearJitterForce(const LinearJitterForce &copy) :
  LinearRandomForce(copy) {
}

/**
 * constructor
 */
LinearJitterForce::
~LinearJitterForce() {
}

/**
 * copier
 */
LinearForce *LinearJitterForce::
make_copy() {
  return new LinearJitterForce(*this);
}

/**
 * random value
 */
LVector3 LinearJitterForce::
get_child_vector(const PhysicsObject *) {
  return random_unit_vector();
}

/**
 * Write a string representation of this instance to <out>.
 */
void LinearJitterForce::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"LinearJitterForce";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void LinearJitterForce::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"LinearJitterForce:\n";
  LinearRandomForce::write(out, indent+2);
  #endif //] NDEBUG
}

/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearRandomForce.cxx
 * @author charles
 * @date 2000-06-19
 */

#include "linearRandomForce.h"

TypeHandle LinearRandomForce::_type_handle;

/**
 * vector constructor
 */
LinearRandomForce::
LinearRandomForce(PN_stdfloat a, bool mass) :
  LinearForce(a, mass) {
}

/**
 * copy constructor
 */
LinearRandomForce::
LinearRandomForce(const LinearRandomForce &copy) :
  LinearForce(copy) {
}

/**
 * destructor
 */
LinearRandomForce::
~LinearRandomForce() {
}

/**
 * Returns a float in [0, 1]
 */
PN_stdfloat LinearRandomForce::
bounded_rand() {
  return ((PN_stdfloat)rand() / (PN_stdfloat)RAND_MAX);
}

/**
 * Write a string representation of this instance to <out>.
 */
void LinearRandomForce::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"LinearRandomForce";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void LinearRandomForce::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"LinearRandomForce:\n";
  LinearForce::write(out, indent+2);
  #endif //] NDEBUG
}

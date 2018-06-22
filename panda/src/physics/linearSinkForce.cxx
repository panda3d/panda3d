/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearSinkForce.cxx
 * @author charles
 * @date 2000-06-21
 */

#include "linearSinkForce.h"

TypeHandle LinearSinkForce::_type_handle;

/**
 * Simple constructor
 */
LinearSinkForce::
LinearSinkForce(const LPoint3& p, FalloffType f, PN_stdfloat r, PN_stdfloat a,
                bool mass) :
  LinearDistanceForce(p, f, r, a, mass) {
}

/**
 * Simple constructor
 */
LinearSinkForce::
LinearSinkForce() :
  LinearDistanceForce(LPoint3(0.0f, 0.0f, 0.0f), FT_ONE_OVER_R_SQUARED,
                      1.0f, 1.0f, true) {
}

/**
 * copy constructor
 */
LinearSinkForce::
LinearSinkForce(const LinearSinkForce &copy) :
  LinearDistanceForce(copy) {
}

/**
 * Simple destructor
 */
LinearSinkForce::
~LinearSinkForce() {
}

/**
 * copier
 */
LinearForce *LinearSinkForce::
make_copy() {
  return new LinearSinkForce(*this);
}

/**
 * virtual force query
 */
LVector3 LinearSinkForce::
get_child_vector(const PhysicsObject *po) {
  return (get_force_center() - po->get_position()) * get_scalar_term();
}

/**
 * Write a string representation of this instance to <out>.
 */
void LinearSinkForce::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"LinearSinkForce";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void LinearSinkForce::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"LinearSinkForce:\n";
  LinearDistanceForce::write(out, indent+2);
  #endif //] NDEBUG
}

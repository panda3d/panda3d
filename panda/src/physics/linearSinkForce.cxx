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
  LVector3 distance_vector = get_force_center() - po->get_position();
  PN_stdfloat distance_squared = distance_vector.length_squared();

  if (distance_squared == 0) {
    return distance_vector;
  }

  PN_stdfloat scalar = get_scalar_term();

  switch (get_falloff_type()) {
  case FT_ONE_OVER_R_OVER_DISTANCE:
    return (distance_vector / sqrt(distance_squared)) * scalar;
  case FT_ONE_OVER_R_OVER_DISTANCE_SQUARED:
    return (distance_vector / distance_squared) * scalar;
  case FT_ONE_OVER_R_OVER_DISTANCE_CUBED:
    return (distance_vector / (distance_squared * sqrt(distance_squared))) * scalar;
  default:
    return distance_vector * scalar;
  }
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

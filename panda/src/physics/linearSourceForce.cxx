/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearSourceForce.cxx
 * @author charles
 * @date 2000-06-21
 */

#include "linearSourceForce.h"

TypeHandle LinearSourceForce::_type_handle;

/**
 * Simple constructor
 */
LinearSourceForce::
LinearSourceForce(const LPoint3& p, FalloffType f, PN_stdfloat r, PN_stdfloat a,
          bool mass) :
  LinearDistanceForce(p, f, r, a, mass) {
}

/**
 * Simple constructor
 */
LinearSourceForce::
LinearSourceForce() :
  LinearDistanceForce(LPoint3(0.0f, 0.0f, 0.0f), FT_ONE_OVER_R_SQUARED,
                      1.0f, 1.0f, true) {
}

/**
 * copy constructor
 */
LinearSourceForce::
LinearSourceForce(const LinearSourceForce &copy) :
  LinearDistanceForce(copy) {
}

/**
 * Simple destructor
 */
LinearSourceForce::
~LinearSourceForce() {
}

/**
 * copier
 */
LinearForce *LinearSourceForce::
make_copy() {
  return new LinearSourceForce(*this);
}

/**
 * virtual force query
 */
LVector3 LinearSourceForce::
get_child_vector(const PhysicsObject *po) {
  LVector3 distance_vector = po->get_position() - get_force_center();
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
void LinearSourceForce::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"LinearSourceForce";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void LinearSourceForce::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"LinearSourceForce:\n";
  LinearDistanceForce::write(out, indent+2);
  #endif //] NDEBUG
}

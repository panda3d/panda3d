/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearDistanceForce.cxx
 * @author charles
 * @date 2000-06-21
 */

#include "linearDistanceForce.h"

TypeHandle LinearDistanceForce::_type_handle;

/**
 * Simple constructor
 */
LinearDistanceForce::
LinearDistanceForce(const LPoint3& p, FalloffType ft, PN_stdfloat r, PN_stdfloat a, bool m) :
  LinearForce(a, m),
  _force_center(p), _falloff(ft), _radius(r)
{
}

/**
 * copy constructor
 */
LinearDistanceForce::
LinearDistanceForce(const LinearDistanceForce &copy) :
  LinearForce(copy) {
  _falloff = copy._falloff;
  _radius = copy._radius;
  _force_center = copy._force_center;
}

/**
 * destructor
 */
LinearDistanceForce::
~LinearDistanceForce() {
}

/**
 * Write a string representation of this instance to <out>.
 */
void LinearDistanceForce::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"LinearDistanceForce";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void LinearDistanceForce::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"LinearDistanceForce:\n";
  out.width(indent+2); out<<""; out<<"_force_center "<<_force_center<<"\n";
  out.width(indent+2); out<<""; out<<"_falloff "<<_falloff<<"\n";
  out.width(indent+2); out<<""; out<<"_radius "<<_radius<<"\n";
  LinearForce::write(out, indent+2);
  #endif //] NDEBUG
}

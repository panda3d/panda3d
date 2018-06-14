/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointEmitter.cxx
 * @author charles
 * @date 2000-06-22
 */

#include "pointEmitter.h"

/**
 * constructor
 */
PointEmitter::
PointEmitter() :
  BaseParticleEmitter() {
  _location.set(0.0f, 0.0f, 0.0f);
}

/**
 * copy constructor
 */
PointEmitter::
PointEmitter(const PointEmitter &copy) :
  BaseParticleEmitter(copy) {
  _location = copy._location;
}

/**
 * destructor
 */
PointEmitter::
~PointEmitter() {
}

/**
 * copier
 */
BaseParticleEmitter *PointEmitter::
make_copy() {
  return new PointEmitter(*this);
}

/**
 * Generates a location for a new particle
 */
void PointEmitter::
assign_initial_position(LPoint3& pos) {
  pos = _location;
}

/**
 * Generates a velocity for a new particle
 */
void PointEmitter::
assign_initial_velocity(LVector3& vel) {
  vel.set(0,0,0);
}

/**
 * Write a string representation of this instance to <out>.
 */
void PointEmitter::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"PointEmitter";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void PointEmitter::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"PointEmitter:\n";
  out.width(indent+2); out<<""; out<<"_location "<<_location<<"\n";
  BaseParticleEmitter::write(out, indent+2);
  #endif //] NDEBUG
}

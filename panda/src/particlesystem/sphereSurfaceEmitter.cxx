/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sphereSurfaceEmitter.cxx
 * @author charles
 * @date 2000-06-22
 */

#include "sphereSurfaceEmitter.h"

/**
 * constructor
 */
SphereSurfaceEmitter::
SphereSurfaceEmitter() {
  _radius = 1.0f;
}

/**
 * copy constructor
 */
SphereSurfaceEmitter::
SphereSurfaceEmitter(const SphereSurfaceEmitter &copy) :
  BaseParticleEmitter(copy) {
  _radius = copy._radius;
}

/**
 * destructor
 */
SphereSurfaceEmitter::
~SphereSurfaceEmitter() {
}

/**
 * copier
 */
BaseParticleEmitter *SphereSurfaceEmitter::
make_copy() {
  return new SphereSurfaceEmitter(*this);
}

/**
 * Generates a location for a new particle
 */
void SphereSurfaceEmitter::
assign_initial_position(LPoint3& pos) {
  PN_stdfloat z, theta, r;

  z = SPREAD(_radius);
  r = sqrtf((_radius * _radius) - (z * z));
  theta = NORMALIZED_RAND() * 2.0f * MathNumbers::pi_f;

  pos.set(r * cosf(theta), r * sinf(theta), z);
}

/**
 * Generates a velocity for a new particle
 */
void SphereSurfaceEmitter::
assign_initial_velocity(LVector3& vel) {
  vel.set(0,0,0);
}

/**
 * Write a string representation of this instance to <out>.
 */
void SphereSurfaceEmitter::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"SphereSurfaceEmitter";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void SphereSurfaceEmitter::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"SphereSurfaceEmitter:\n";
  out.width(indent+2); out<<""; out<<"_radius "<<_radius<<"\n";
  BaseParticleEmitter::write(out, indent+2);
  #endif //] NDEBUG
}

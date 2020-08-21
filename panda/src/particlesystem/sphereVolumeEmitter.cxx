/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sphereVolumeEmitter.cxx
 * @author charles
 * @date 2000-06-22
 */

#include "sphereVolumeEmitter.h"

/**
 * constructor
 */
SphereVolumeEmitter::
SphereVolumeEmitter() {
  _radius = 1.0f;
}

/**
 * copy constructor
 */
SphereVolumeEmitter::
SphereVolumeEmitter(const SphereVolumeEmitter &copy) :
  BaseParticleEmitter(copy) {
  _radius = copy._radius;
  _particle_pos = copy._particle_pos;
}

/**
 * destructor
 */
SphereVolumeEmitter::
~SphereVolumeEmitter() {
}

/**
 * copier
 */
BaseParticleEmitter *SphereVolumeEmitter::
make_copy() {
  return new SphereVolumeEmitter(*this);
}

/**
 * Generates a location for a new particle
 */
void SphereVolumeEmitter::
assign_initial_position(LPoint3& pos) {
  PN_stdfloat z, theta, r;
  PN_stdfloat t;

  z = SPREAD(_radius);
  r = sqrtf((_radius * _radius) - (z * z));
  theta = NORMALIZED_RAND() * 2.0f * MathNumbers::pi_f;

  t = NORMALIZED_RAND();

  while (t == 0.0f)
    t = NORMALIZED_RAND();

  PN_stdfloat pos_x = r * cosf(theta) * t;
  PN_stdfloat pos_y = r * sinf(theta) * t;
  PN_stdfloat pos_z = z * t;

  _particle_pos.set(pos_x, pos_y, pos_z);
  pos = _particle_pos;
}

/**
 * Generates a velocity for a new particle
 */
void SphereVolumeEmitter::
assign_initial_velocity(LVector3& vel) {
  // set velocity to [0..1] according to distance from center, along vector
  // from center to position
  vel = _particle_pos / _radius;
}

/**
 * Write a string representation of this instance to <out>.
 */
void SphereVolumeEmitter::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"SphereVolumeEmitter";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void SphereVolumeEmitter::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"SphereVolumeEmitter:\n";
  out.width(indent+2); out<<""; out<<"_radius "<<_radius<<"\n";
  BaseParticleEmitter::write(out, indent+2);
  #endif //] NDEBUG
}

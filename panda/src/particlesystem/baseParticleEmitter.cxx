/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file baseParticleEmitter.cxx
 * @author charles
 * @date 2000-06-14
 */

#include "baseParticleEmitter.h"

#include <stdlib.h>

/**
 * constructor
 */
BaseParticleEmitter::
BaseParticleEmitter() {
  _emission_type = ET_RADIATE;
  _explicit_launch_vector.set(1,0,0);
  _radiate_origin.set(0,0,0);
  _amplitude = 1.0f;
  _amplitude_spread = 0.0f;
  _offset_force.set(0,0,0);
}

/**
 * copy constructor
 */
BaseParticleEmitter::
BaseParticleEmitter(const BaseParticleEmitter &copy) {
  _emission_type = copy._emission_type;
  _explicit_launch_vector = copy._explicit_launch_vector;
  _radiate_origin = copy._radiate_origin;
  _amplitude = copy._amplitude;
  _amplitude_spread = copy._amplitude_spread;
  _offset_force = copy._offset_force;
}

/**
 * destructor
 */
BaseParticleEmitter::
~BaseParticleEmitter() {
}

/**
 * parent generation function
 */
void BaseParticleEmitter::
generate(LPoint3& pos, LVector3& vel) {
  assign_initial_position(pos);

  switch(_emission_type)
  {
    case ET_EXPLICIT:
      vel = _explicit_launch_vector;
      break;

    case ET_RADIATE:
      vel = pos - _radiate_origin;
      vel.normalize();
      break;

    case ET_CUSTOM:
      assign_initial_velocity(vel);
      break;
  }

  vel *= _amplitude + SPREAD(_amplitude_spread);
  vel += _offset_force;
}

/**
 * Write a string representation of this instance to <out>.
 */
void BaseParticleEmitter::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"BaseParticleEmitter";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void BaseParticleEmitter::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"BaseParticleEmitter:\n";
  out.width(indent+2); out<<""; out<<"_emission_type "<<_emission_type<<"\n";
  out.width(indent+2); out<<""; out<<"_amplitude "<<_amplitude<<"\n";
  out.width(indent+2); out<<""; out<<"_amplitude_spread "<<_amplitude_spread<<"\n";
  out.width(indent+2); out<<""; out<<"_offset_force "<<_offset_force<<"\n";
  // ReferenceCount::write(out, indent+2);
  #endif //] NDEBUG
}

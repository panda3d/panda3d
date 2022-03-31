/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ringEmitter.cxx
 * @author charles
 * @date 2000-06-22
 */

#include "ringEmitter.h"

/**
 * constructor
 */
RingEmitter::
RingEmitter() :
  _radius(1.0f), _radius_spread(0.0f), _aoe(0.0f), _uniform_emission(0), _theta(0.0f)
{
}

/**
 * copy constructor
 */
RingEmitter::
RingEmitter(const RingEmitter &copy) :
  BaseParticleEmitter(copy) {
  _radius = copy._radius;
  _aoe = copy._aoe;
  _radius_spread = copy._radius_spread;
  _uniform_emission = copy._uniform_emission;

  _theta = copy._theta;
  _sin_theta = copy._sin_theta;
  _cos_theta = copy._cos_theta;
}

/**
 * destructor
 */
RingEmitter::
~RingEmitter() {
}

/**
 * copier
 */
BaseParticleEmitter *RingEmitter::
make_copy() {
  return new RingEmitter(*this);
}

/**
 * Generates a location for a new particle
 */
void RingEmitter::
assign_initial_position(LPoint3& pos) {
  if (_uniform_emission > 0)
  {
    _theta = _theta + 1.0/_uniform_emission;
    if (_theta > 1.0)
      _theta = _theta - 1.0;
  }
  else
  {
    _theta = NORMALIZED_RAND();
  }

  _cos_theta = cosf(_theta * 2.0f * MathNumbers::pi_f);
  _sin_theta = sinf(_theta * 2.0f * MathNumbers::pi_f);

  PN_stdfloat new_radius_spread = SPREAD(_radius_spread);
  PN_stdfloat new_x = _cos_theta * (_radius + new_radius_spread);
  PN_stdfloat new_y = _sin_theta * (_radius + new_radius_spread);

  pos.set(new_x, new_y, 0.0f);
}

/**
 * Generates a velocity for a new particle
 */
void RingEmitter::
assign_initial_velocity(LVector3& vel) {
  PN_stdfloat vel_z = sinf(deg_2_rad(_aoe));
  PN_stdfloat abs_diff = fabs(1.0f - (vel_z * vel_z));
  PN_stdfloat root_mag_minus_z_squared = sqrtf(abs_diff);

  PN_stdfloat vel_x = _cos_theta * root_mag_minus_z_squared;
  PN_stdfloat vel_y = _sin_theta * root_mag_minus_z_squared;

  // quick and dirty
  if((_aoe > 90.0f) && (_aoe < 270.0f))
  {
    vel_x = -vel_x;
    vel_y = -vel_y;
  }

  vel.set(vel_x, vel_y, vel_z);
}

/**
 * Write a string representation of this instance to <out>.
 */
void RingEmitter::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"RingEmitter";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void RingEmitter::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"RingEmitter:\n";
  out.width(indent+2); out<<""; out<<"_radius "<<_radius<<"\n";
  out.width(indent+2); out<<""; out<<"_radius_spread "<<_radius_spread<<"\n";
  out.width(indent+2); out<<""; out<<"_aoe "<<_aoe<<"\n";
  out.width(indent+2); out<<""; out<<"_sin_theta "<<_sin_theta<<"\n";
  out.width(indent+2); out<<""; out<<"_cos_theta "<<_cos_theta<<"\n";
  BaseParticleEmitter::write(out, indent+2);
  #endif //] NDEBUG
}

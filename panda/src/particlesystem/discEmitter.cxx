/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file discEmitter.cxx
 * @author charles
 * @date 2000-06-22
 */

#include "discEmitter.h"

/**
 * constructor
 */
DiscEmitter::
DiscEmitter() {
  _radius = 1.0f;
  _inner_aoe = _outer_aoe = 0.0f;
  _inner_magnitude = _outer_magnitude = 1.0f;
  _cubic_lerping = false;
}

/**
 * copy constructor
 */
DiscEmitter::
DiscEmitter(const DiscEmitter &copy) :
  BaseParticleEmitter(copy) {
  _radius = copy._radius;
  _inner_aoe = copy._inner_aoe;
  _outer_aoe = copy._outer_aoe;
  _inner_magnitude = copy._inner_magnitude;
  _outer_magnitude = copy._outer_magnitude;
  _cubic_lerping = copy._cubic_lerping;

  _distance_from_center = copy._distance_from_center;
  _sinf_theta = copy._sinf_theta;
  _cosf_theta = copy._cosf_theta;
}

/**
 * destructor
 */
DiscEmitter::
~DiscEmitter() {
}

/**
 * copier
 */
BaseParticleEmitter *DiscEmitter::
make_copy() {
  return new DiscEmitter(*this);
}

/**
 * Generates a location for a new particle
 */
void DiscEmitter::
assign_initial_position(LPoint3& pos) {
  // position
  PN_stdfloat theta = NORMALIZED_RAND() * 2.0f * MathNumbers::pi_f;

  _distance_from_center = NORMALIZED_RAND();
  PN_stdfloat r_scalar = _distance_from_center * _radius;

  _sinf_theta = sinf(theta);
  _cosf_theta = cosf(theta);

  PN_stdfloat new_x = _cosf_theta * r_scalar;
  PN_stdfloat new_y = _sinf_theta * r_scalar;

  pos.set(new_x, new_y, 0.0f);
}

/**
 * Generates a velocity for a new particle
 */
void DiscEmitter::
assign_initial_velocity(LVector3& vel) {
  PN_stdfloat aoe, mag;

  // lerp type
  if (_cubic_lerping == true) {
    aoe = CLERP(_distance_from_center, _inner_aoe, _outer_aoe);
    mag = CLERP(_distance_from_center, _inner_magnitude, _outer_magnitude);
  }
  else {
    aoe = LERP(_distance_from_center, _inner_aoe, _outer_aoe);
    mag = LERP(_distance_from_center, _inner_magnitude, _outer_magnitude);
  }

  // velocity
  PN_stdfloat vel_z = mag * sinf(deg_2_rad(aoe));
  PN_stdfloat abs_diff = fabs((mag * mag) - (vel_z * vel_z));
  PN_stdfloat root_mag_minus_z_squared = sqrtf(abs_diff);
  PN_stdfloat vel_x = _cosf_theta * root_mag_minus_z_squared;
  PN_stdfloat vel_y = _sinf_theta * root_mag_minus_z_squared;

  // quick and dirty
  if((aoe > 90.0f) && (aoe < 270.0f))
  {
    vel_x = -vel_x;
    vel_y = -vel_y;
  }

  vel.set(vel_x, vel_y, vel_z);
}

/**
 * Write a string representation of this instance to <out>.
 */
void DiscEmitter::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"DiscEmitter";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void DiscEmitter::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"DiscEmitter:\n";
  out.width(indent+2); out<<""; out<<"_radius "<<_radius<<"\n";
  out.width(indent+2); out<<""; out<<"_outer_aoe "<<_outer_aoe<<"\n";
  out.width(indent+2); out<<""; out<<"_inner_aoe "<<_inner_aoe<<"\n";
  out.width(indent+2); out<<""; out<<"_outer_magnitude "<<_outer_magnitude<<"\n";
  out.width(indent+2); out<<""; out<<"_inner_magnitude "<<_inner_magnitude<<"\n";
  out.width(indent+2); out<<""; out<<"_cubic_lerping "<<_cubic_lerping<<"\n";
  out.width(indent+2); out<<""; out<<"_distance_from_center "<<_distance_from_center<<"\n";
  out.width(indent+2); out<<""; out<<"_sinf_theta "<<_sinf_theta<<"\n";
  out.width(indent+2); out<<""; out<<"_cosf_theta "<<_cosf_theta<<"\n";
  BaseParticleEmitter::write(out, indent+2);
  #endif //] NDEBUG
}

/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file boxEmitter.cxx
 * @author charles
 * @date 2000-06-22
 */

#include "boxEmitter.h"

/**
 * constructor
 */
BoxEmitter::
BoxEmitter() :
  BaseParticleEmitter() {
  _vmin.set(-0.5f, -0.5f, -0.5f);
  _vmax.set( 0.5f,  0.5f,  0.5f);
}

/**
 * copy constructor
 */
BoxEmitter::
BoxEmitter(const BoxEmitter &copy) :
  BaseParticleEmitter(copy) {
  _vmin = copy._vmin;
  _vmax = copy._vmax;
}

/**
 * destructor
 */
BoxEmitter::
~BoxEmitter() {
}

/**
 * copier
 */
BaseParticleEmitter *BoxEmitter::
make_copy() {
  return new BoxEmitter(*this);
}

/**
 * Generates a location for a new particle
 */
void BoxEmitter::
assign_initial_position(LPoint3& pos) {
  PN_stdfloat t_x = NORMALIZED_RAND();
  PN_stdfloat t_y = NORMALIZED_RAND();
  PN_stdfloat t_z = NORMALIZED_RAND();

  LVector3 v_diff = _vmax - _vmin;

  PN_stdfloat lerp_x = _vmin[0] + t_x * v_diff[0];
  PN_stdfloat lerp_y = _vmin[1] + t_y * v_diff[1];
  PN_stdfloat lerp_z = _vmin[2] + t_z * v_diff[2];

  pos.set(lerp_x, lerp_y, lerp_z);
}

/**
 * Generates a velocity for a new particle
 */
void BoxEmitter::
assign_initial_velocity(LVector3& vel) {
  vel.set(0,0,0);
}

/**
 * Write a string representation of this instance to <out>.
 */
void BoxEmitter::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"BoxEmitter";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void BoxEmitter::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"BoxEmitter:\n";
  out.width(indent+2); out<<""; out<<"_vmin "<<_vmin<<"\n";
  out.width(indent+2); out<<""; out<<"_vmax "<<_vmax<<"\n";
  BaseParticleEmitter::write(out, indent+2);
  #endif //] NDEBUG
}

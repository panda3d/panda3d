/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rectangleEmitter.cxx
 * @author charles
 * @date 2000-06-22
 */

#include "rectangleEmitter.h"

/**
 * constructor
 */
RectangleEmitter::
RectangleEmitter() :
  BaseParticleEmitter() {
  _vmin.set(-0.5f, -0.5f);
  _vmax.set( 0.5f,  0.5f);
}

/**
 * copy constructor
 */
RectangleEmitter::
RectangleEmitter(const RectangleEmitter &copy) :
  BaseParticleEmitter(copy) {
  _vmin = copy._vmin;
  _vmax = copy._vmax;
}

/**
 * destructor
 */
RectangleEmitter::
~RectangleEmitter() {
}

/**
 * copier
 */
BaseParticleEmitter *RectangleEmitter::
make_copy() {
  return new RectangleEmitter(*this);
}

/**
 * Generates a location for a new particle
 */
void RectangleEmitter::
assign_initial_position(LPoint3& pos) {
  PN_stdfloat t_x = NORMALIZED_RAND();
  PN_stdfloat t_y = NORMALIZED_RAND();

  LVector2 v_diff = _vmax - _vmin;

  PN_stdfloat lerp_x = _vmin[0] + t_x * v_diff[0];
  PN_stdfloat lerp_y = _vmin[1] + t_y * v_diff[1];

  pos.set(lerp_x, lerp_y, 0.0f);
}

/**
 * Generates a velocity for a new particle
 */
void RectangleEmitter::
assign_initial_velocity(LVector3& vel) {
  vel.set(0.0f,0.0f,0.0f);
}

/**
 * Write a string representation of this instance to <out>.
 */
void RectangleEmitter::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"RectangleEmitter";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void RectangleEmitter::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"RectangleEmitter:\n";
  out.width(indent+2); out<<""; out<<"_vmin "<<_vmin<<"\n";
  out.width(indent+2); out<<""; out<<"_vmax "<<_vmax<<"\n";
  BaseParticleEmitter::write(out, indent+2);
  #endif //] NDEBUG
}

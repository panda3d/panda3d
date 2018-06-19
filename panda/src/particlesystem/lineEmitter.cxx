/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lineEmitter.cxx
 * @author charles
 * @date 2000-06-22
 */

#include "lineEmitter.h"

/**
 * constructor
 */
LineEmitter::
LineEmitter() :
  BaseParticleEmitter() {
  _endpoint1.set(1.0f, 0.0f, 0.0f);
  _endpoint2.set(0.0f, 0.0f, 0.0f);
}

/**
 * constructor
 */
LineEmitter::
LineEmitter(const LineEmitter &copy) :
  BaseParticleEmitter(copy) {
  _endpoint1 = copy._endpoint1;
  _endpoint2 = copy._endpoint2;
}

/**
 * constructor
 */
LineEmitter::
~LineEmitter() {
}

/**
 * copier
 */
BaseParticleEmitter *LineEmitter::
make_copy() {
  return new LineEmitter(*this);
}

/**
 * Generates a location for a new particle
 */
void LineEmitter::
assign_initial_position(LPoint3& pos) {
  PN_stdfloat t = NORMALIZED_RAND();

  LVector3 v_diff = _endpoint2 - _endpoint1;

  PN_stdfloat lerp_x = _endpoint1[0] + t * v_diff[0];
  PN_stdfloat lerp_y = _endpoint1[1] + t * v_diff[1];
  PN_stdfloat lerp_z = _endpoint1[2] + t * v_diff[2];

  pos.set(lerp_x, lerp_y, lerp_z);
}

/**
 * Generates a velocity for a new particle
 */
void LineEmitter::
assign_initial_velocity(LVector3& vel) {
  vel.set(0,0,0);
}

/**
 * Write a string representation of this instance to <out>.
 */
void LineEmitter::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"LineEmitter";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void LineEmitter::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"LineEmitter:\n";
  out.width(indent+2); out<<""; out<<"_endpoint1 "<<_endpoint1<<"\n";
  out.width(indent+2); out<<""; out<<"_endpoint2 "<<_endpoint2<<"\n";
  BaseParticleEmitter::write(out, indent+2);
  #endif //] NDEBUG
}

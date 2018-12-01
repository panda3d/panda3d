/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file zSpinParticleFactory.cxx
 * @author charles
 * @date 2000-08-16
 */

#include "zSpinParticleFactory.h"
#include "zSpinParticle.h"

/**
 * constructor
 */
ZSpinParticleFactory::
ZSpinParticleFactory() :
  BaseParticleFactory() {
  _initial_angle = 0.0f;
  _final_angle = 0.0f;
  _initial_angle_spread = 0.0f;
  _final_angle_spread = 0.0f;
  _angular_velocity = 0.0f;
  _angular_velocity_spread = 0.0f;
  _bUseAngularVelocity = false;
}

/**
 * copy constructor
 */
ZSpinParticleFactory::
ZSpinParticleFactory(const ZSpinParticleFactory &copy) :
  BaseParticleFactory(copy) {
  _initial_angle = copy._initial_angle;
  _final_angle = copy._final_angle;
  _initial_angle_spread = copy._initial_angle_spread;
  _final_angle_spread = copy._final_angle_spread;
  _angular_velocity = copy._angular_velocity;
  _angular_velocity_spread = copy._angular_velocity_spread;
  _bUseAngularVelocity = copy._bUseAngularVelocity;
}

/**
 * destructor
 */
ZSpinParticleFactory::
~ZSpinParticleFactory() {
}

/**
 * factory method
 */
BaseParticle *ZSpinParticleFactory::
alloc_particle() const {
  return new ZSpinParticle;
}

/**
 * factory populator
 */
void ZSpinParticleFactory::
populate_child_particle(BaseParticle *bp) const {
  ZSpinParticle *zsp = (ZSpinParticle *) bp;

  zsp->set_initial_angle(_initial_angle + SPREAD(_initial_angle_spread));
  zsp->set_final_angle(_final_angle + SPREAD(_final_angle_spread));
  zsp->set_angular_velocity(_angular_velocity + SPREAD(_angular_velocity_spread));
  zsp->enable_angular_velocity(_bUseAngularVelocity);
}

/**
 * Write a string representation of this instance to <out>.
 */
void ZSpinParticleFactory::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"ZSpinParticleFactory";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void ZSpinParticleFactory::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"ZSpinParticleFactory:\n";
  out.width(indent+2); out<<""; out<<"_initial_angle "<<_initial_angle<<"\n";
  out.width(indent+2); out<<""; out<<"_initial_angle_spread "<<_initial_angle_spread<<"\n";
  out.width(indent+2); out<<""; out<<"_final_angle "<<_final_angle<<"\n";
  out.width(indent+2); out<<""; out<<"_final_angle_spread "<<_final_angle_spread<<"\n";
  out.width(indent+2); out<<""; out<<"_angular_velocity "<<_angular_velocity<<"\n";
  out.width(indent+2); out<<""; out<<"_angular_velocity_spread "<<_angular_velocity_spread<<"\n";
  out.width(indent+2); out<<""; out<<"_bUseAngularVelocity "<<_bUseAngularVelocity<<"\n";
  BaseParticleFactory::write(out, indent+2);
  #endif //] NDEBUG
}

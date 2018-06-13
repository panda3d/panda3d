/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointParticle.cxx
 * @author charles
 * @date 2000-06-19
 */

#include "pointParticle.h"

/**
 * simple constructor
 */
PointParticle::
PointParticle(PN_stdfloat lifespan, bool alive) :
  BaseParticle(lifespan, alive) {
  set_oriented(false);
}

/**
 * copy constructor
 */
PointParticle::
PointParticle(const PointParticle &copy) :
  BaseParticle(copy) {
  set_oriented(false);
}

/**
 * simple destructor
 */
PointParticle::
~PointParticle() {
}

/**
 * dynamic copier
 */
PhysicsObject *PointParticle::
make_copy() const {
  return new PointParticle(*this);
}

/**
 * particle death routine
 */
void PointParticle::
die() {
}

/**
 * particle init routine
 */
void PointParticle::
init() {
}

/**
 * particle update
 */
void PointParticle::
update() {
}

/**
 * Write a string representation of this instance to <out>.
 */
void PointParticle::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"PointParticle";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void PointParticle::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"PointParticle:\n";
  BaseParticle::write(out, indent+2);
  #endif //] NDEBUG
}

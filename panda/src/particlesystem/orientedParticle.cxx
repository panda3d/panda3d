/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file orientedParticle.cxx
 * @author charles
 * @date 2000-06-19
 */

#include "orientedParticle.h"

/**
 * simple constructor
 */
OrientedParticle::
OrientedParticle(int lifespan, bool alive) :
  BaseParticle(lifespan, alive) {
  set_oriented(true);
}

/**
 * copy constructor
 */
OrientedParticle::
OrientedParticle(const OrientedParticle &copy) :
  BaseParticle(copy) {
}

/**
 * simple destructor
 */
OrientedParticle::
~OrientedParticle() {
}

/**
 * simple destructor
 */
PhysicsObject *OrientedParticle::
make_copy() const {
  return new OrientedParticle(*this);
}

/**
 * particle init routine
 */
void OrientedParticle::
init() {
}

/**
 * particle death routine
 */
void OrientedParticle::
die() {
}

/**
 * particle update routine.  This NEEDS to be filled in with quaternion slerp
 * stuff, or oriented particles will not rotate.
 */
void OrientedParticle::
update() {
}

/**
 * Write a string representation of this instance to <out>.
 */
void OrientedParticle::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"OrientedParticle";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void OrientedParticle::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"OrientedParticle:\n";
  BaseParticle::write(out, indent+2);
  #endif //] NDEBUG
}

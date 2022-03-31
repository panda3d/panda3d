/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file orientedParticleFactory.cxx
 * @author charles
 * @date 2000-07-05
 */

#include "orientedParticleFactory.h"
#include "orientedParticle.h"

/**
 * Constructor
 */
OrientedParticleFactory::
OrientedParticleFactory() :
  BaseParticleFactory() {
}

/**
 * copy Constructor
 */
OrientedParticleFactory::
OrientedParticleFactory(const OrientedParticleFactory &copy) :
  BaseParticleFactory(copy) {
  _initial_orientation = copy._initial_orientation;
  _final_orientation = copy._final_orientation;
}

/**
 * destructor
 */
OrientedParticleFactory::
~OrientedParticleFactory() {
}

/**
 * child spawn
 */
void OrientedParticleFactory::
populate_child_particle(BaseParticle *bp) const {
  bp->set_orientation(_initial_orientation);
}

/**
 * child particle generation function
 */
BaseParticle *OrientedParticleFactory::
alloc_particle() const {
  return new OrientedParticle;
}

/**
 * Write a string representation of this instance to <out>.
 */
void OrientedParticleFactory::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"OrientedParticleFactory";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void OrientedParticleFactory::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"OrientedParticleFactory:\n";
  BaseParticleFactory::write(out, indent+2);
  #endif //] NDEBUG
}

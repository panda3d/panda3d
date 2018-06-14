/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointParticleFactory.cxx
 * @author charles
 * @date 2000-07-05
 */

#include "pointParticleFactory.h"
#include "pointParticle.h"

#include <stdlib.h>

/**
 * default constructor
 */
PointParticleFactory::
PointParticleFactory() :
  BaseParticleFactory() {
}

/**
 * copy constructor
 */
PointParticleFactory::
PointParticleFactory(const PointParticleFactory &copy) :
  BaseParticleFactory(copy) {
}

/**
 * destructor
 */
PointParticleFactory::
~PointParticleFactory() {
}

/**
 * child particle generation function
 */
void PointParticleFactory::
populate_child_particle(BaseParticle *bp) const {
  bp->set_oriented(false);
}

/**
 * child particle generation function
 */
BaseParticle *PointParticleFactory::
alloc_particle() const {
  return new PointParticle;
}

/**
 * Write a string representation of this instance to <out>.
 */
void PointParticleFactory::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"PointParticleFactory";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void PointParticleFactory::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"PointParticleFactory:\n";
  BaseParticleFactory::write(out, indent+2);
  #endif //] NDEBUG
}

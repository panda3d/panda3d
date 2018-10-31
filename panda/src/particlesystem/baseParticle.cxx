/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file baseParticle.cxx
 * @author charles
 * @date 2000-06-14
 */

#include "baseParticle.h"

/**
 * Default Constructor
 */
BaseParticle::
BaseParticle(PN_stdfloat lifespan, bool alive) :
  _age(0.0f), _lifespan(lifespan), _alive(alive), _index(0){
}

/**
 * Copy Constructor
 */
BaseParticle::
BaseParticle(const BaseParticle &copy) :
  _age(copy._age),
  _lifespan(copy._lifespan),
  _alive(copy._alive),
  _index(copy._index) {
}

/**
 * Default Destructor
 */
BaseParticle::
~BaseParticle() {
}

/**
 * for spriteParticleRenderer
 */
PN_stdfloat BaseParticle::
get_theta() const {
  return 0.0f;
}

/**
 * Write a string representation of this instance to <out>.
 */
void BaseParticle::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"BaseParticle";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void BaseParticle::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"BaseParticle:\n";
  out.width(indent+2); out<<""; out<<"_age "<<_age<<"\n";
  out.width(indent+2); out<<""; out<<"_lifespan "<<_lifespan<<"\n";
  out.width(indent+2); out<<""; out<<"_alive "<<_alive<<"\n";
  out.width(indent+2); out<<""; out<<"_index "<<_index<<"\n";
  PhysicsObject::write(out, indent+2);
  #endif //] NDEBUG
}

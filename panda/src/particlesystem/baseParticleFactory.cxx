/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file baseParticleFactory.cxx
 * @author charles
 * @date 2000-07-05
 */

#include "baseParticleFactory.h"

/**
 * constructor
 */
BaseParticleFactory::
BaseParticleFactory() :
  _lifespan_base(1.0),
  _lifespan_spread(0.0),
  _mass_base(1.0f),
  _mass_spread(0.0f),
  _terminal_velocity_base(PhysicsObject::_default_terminal_velocity),
  _terminal_velocity_spread(0.0f)
{
}

/**
 * copy constructor
 */
BaseParticleFactory::
BaseParticleFactory(const BaseParticleFactory &copy) :
  _lifespan_base(copy._lifespan_base),
  _lifespan_spread(copy._lifespan_spread),
  _mass_base(copy._mass_base),
  _mass_spread(copy._mass_spread),
  _terminal_velocity_base(copy._terminal_velocity_base),
  _terminal_velocity_spread(copy._terminal_velocity_spread)
{
}

/**
 * destructor
 */
BaseParticleFactory::
~BaseParticleFactory() {
}

/**
 * public
 */
void BaseParticleFactory::
populate_particle(BaseParticle *bp) {
  bp->set_lifespan(_lifespan_base + SPREAD(_lifespan_spread));
  bp->set_mass(_mass_base + SPREAD(_mass_spread));
  bp->set_terminal_velocity(_terminal_velocity_base + SPREAD(_terminal_velocity_spread));

  bp->set_active(false);
  bp->set_alive(false);
  bp->set_age(0.0f);
  bp->set_index(0);

  populate_child_particle(bp);
}

/**
 * Write a string representation of this instance to <out>.
 */
void BaseParticleFactory::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"BaseParticleFactory";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void BaseParticleFactory::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"BaseParticleFactory:\n";
  out.width(indent+2); out<<""; out<<"_lifespan_base "<<_lifespan_base<<"\n";
  out.width(indent+2); out<<""; out<<"_lifespan_spread "<<_lifespan_spread<<"\n";
  out.width(indent+2); out<<""; out<<"_mass_base "<<_mass_base<<"\n";
  out.width(indent+2); out<<""; out<<"_mass_spread "<<_mass_spread<<"\n";
  out.width(indent+2); out<<""; out<<"_terminal_velocity_base "<<_terminal_velocity_base<<"\n";
  out.width(indent+2); out<<""; out<<"_terminal_velocity_spread "<<_terminal_velocity_spread<<"\n";
  // ReferenceCount::write(out, indent+2);
  #endif //] NDEBUG
}

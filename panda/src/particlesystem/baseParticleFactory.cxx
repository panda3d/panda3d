// Filename: baseParticleFactory.cxx
// Created by:  charles (05Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "baseParticleFactory.h"

////////////////////////////////////////////////////////////////////
//    Function : BaseParticleFactory
//      Access : protected
// Description : constructor
////////////////////////////////////////////////////////////////////
BaseParticleFactory::
BaseParticleFactory() {
  _mass_base = 1.0f;
  _mass_spread = 0.0f;

  _terminal_velocity_base = PhysicsObject::_default_terminal_velocity;
  _terminal_velocity_spread = 0.0f;

  _lifespan_base = 1.0f;
  _lifespan_spread = 0.0f;
}

////////////////////////////////////////////////////////////////////
//    Function : BaseParticleFactory
//      Access : protected
// Description : copy constructor
////////////////////////////////////////////////////////////////////
BaseParticleFactory::
BaseParticleFactory(const BaseParticleFactory &copy) {
  _terminal_velocity_base = copy._terminal_velocity_base;
  _terminal_velocity_spread = copy._terminal_velocity_spread;
  _lifespan_base = copy._lifespan_base;
  _lifespan_spread = copy._lifespan_spread;
}

////////////////////////////////////////////////////////////////////
//    Function : ~BaseParticleFactory
//      Access : public virtual
// Description : destructor
////////////////////////////////////////////////////////////////////
BaseParticleFactory::
~BaseParticleFactory() {
}

////////////////////////////////////////////////////////////////////
//    Function : make_particle
// Description : public
////////////////////////////////////////////////////////////////////
void BaseParticleFactory::
populate_particle(BaseParticle *bp) {
  bp->set_lifespan(_lifespan_base + SPREAD(_lifespan_spread));
  bp->set_mass(_mass_base + SPREAD(_mass_spread));
  bp->set_terminal_velocity(_terminal_velocity_base + SPREAD(_terminal_velocity_spread));

  bp->set_active(false);
  bp->set_alive(false);
  bp->set_age(0.0f);

  populate_child_particle(bp);
}

// Filename: baseParticleFactory.C
// Created by:  charles (05Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "baseParticleFactory.h"

////////////////////////////////////////////////////////////////////
//    Function : BaseParticleFactory
//      Access : protected
// Description : constructor
////////////////////////////////////////////////////////////////////
BaseParticleFactory::
BaseParticleFactory(void) {
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
~BaseParticleFactory(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : make_particle
// Description : public
////////////////////////////////////////////////////////////////////
void BaseParticleFactory::
populate_particle(BaseParticle *bp) {
  float lifespan = _lifespan_base;
  float mass = _mass_base;
  float tv = _terminal_velocity_base;

  // lifespan
  lifespan += SPREAD(_lifespan_spread);

  // mass
  mass += SPREAD(_mass_spread);

  // tv
  tv += SPREAD(_terminal_velocity_spread);

  bp->set_lifespan(lifespan);
  bp->set_mass(mass);
  bp->set_terminal_velocity(tv);
  bp->set_active(false);
  bp->set_alive(false);
  bp->set_age(0.0f);

  populate_child_particle(bp);
}

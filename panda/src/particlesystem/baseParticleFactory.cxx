// Filename: baseParticleFactory.cxx
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
  _terminal_velocity_base = PhysicsObject::_default_terminal_velocity;
  _terminal_velocity_delta = 0.0f;

  _lifespan_base = 1.0f;
  _lifespan_delta = 0.0f;
}

////////////////////////////////////////////////////////////////////
//    Function : BaseParticleFactory
//      Access : protected
// Description : copy constructor
////////////////////////////////////////////////////////////////////
BaseParticleFactory::
BaseParticleFactory(const BaseParticleFactory &copy) {
  _terminal_velocity_base = copy._terminal_velocity_base;
  _terminal_velocity_delta = copy._terminal_velocity_delta;
  _lifespan_base = copy._lifespan_base;
  _lifespan_delta = copy._lifespan_delta;
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
  float t;

  // lifespan
  t = bounded_rand();
  lifespan += _lifespan_delta - (2.0f * t * _lifespan_delta);

  // mass
  t = bounded_rand();
  mass += _mass_delta - (2.0f * t * _mass_delta);

  // tv
  t = bounded_rand();
  tv += _terminal_velocity_delta - (2.0f * t * _terminal_velocity_delta);

  bp->set_lifespan(lifespan);
  bp->set_mass(mass);
  bp->set_terminal_velocity(tv);
  bp->set_active(false);
  bp->set_alive(false);
  bp->set_age(0.0f);

  populate_child_particle(bp);
}

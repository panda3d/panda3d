// Filename: zSpinParticleFactory.cxx
// Created by:  charles (16Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "zSpinParticleFactory.h"
#include "zSpinParticle.h"

////////////////////////////////////////////////////////////////////
//    Function : ZSpinParticleFactory
//      Access : public
// Description : constructor
////////////////////////////////////////////////////////////////////
ZSpinParticleFactory::
ZSpinParticleFactory(void) :
  BaseParticleFactory() {
  _initial_theta = 0.0f;
  _final_theta = 0.0f;
  _theta_delta = 0.0f;
}

////////////////////////////////////////////////////////////////////
//    Function : ZSpinParticleFactory
//      Access : public
// Description : copy constructor
////////////////////////////////////////////////////////////////////
ZSpinParticleFactory::
ZSpinParticleFactory(const ZSpinParticleFactory &copy) :
  BaseParticleFactory(copy) {
  _initial_theta = copy._initial_theta;
  _final_theta = copy._final_theta;
  _theta_delta = copy._theta_delta;
}

////////////////////////////////////////////////////////////////////
//    Function : ~ZSpinParticleFactory
//      Access : virtual, public
// Description : destructor
////////////////////////////////////////////////////////////////////
ZSpinParticleFactory::
~ZSpinParticleFactory(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : alloc_particle
//      Access : private, virtual
// Description : factory method
////////////////////////////////////////////////////////////////////
BaseParticle *ZSpinParticleFactory::
alloc_particle(void) const {
  return new ZSpinParticle;
}

////////////////////////////////////////////////////////////////////
//    Function : populate_child_particle
//      Access : private, virtual
// Description : factory populator
////////////////////////////////////////////////////////////////////
void ZSpinParticleFactory::
populate_child_particle(BaseParticle *bp) const {
  ZSpinParticle *zsp = (ZSpinParticle *) bp;

  float final_theta = _final_theta;
  if (_theta_delta != 0.0f)
    final_theta += _theta_delta - (bounded_rand() * 2.0f * _theta_delta);

  zsp->set_thetas(_initial_theta, final_theta);
}

// Filename: zSpinParticleFactory.C
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
  _initial_angle = 0.0f;
  _final_angle = 0.0f;
  _initial_angle_spread = 0.0f;
  _final_angle_spread = 0.0f;
}

////////////////////////////////////////////////////////////////////
//    Function : ZSpinParticleFactory
//      Access : public
// Description : copy constructor
////////////////////////////////////////////////////////////////////
ZSpinParticleFactory::
ZSpinParticleFactory(const ZSpinParticleFactory &copy) :
  BaseParticleFactory(copy) {
  _initial_angle = copy._initial_angle;
  _final_angle = copy._final_angle;
  _initial_angle_spread = copy._initial_angle_spread;
  _final_angle_spread = copy._final_angle_spread;
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

  zsp->set_initial_angle(_initial_angle + SPREAD(_initial_angle_spread));
  zsp->set_final_angle(_final_angle + SPREAD(_final_angle_spread));
}

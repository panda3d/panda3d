// Filename: orientedParticleFactory.cxx
// Created by:  charles (05Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "orientedParticleFactory.h"
#include "orientedParticle.h"

////////////////////////////////////////////////////////////////////
//    Function : OrientedParticleFactory
//      Access : Public
// Description : Constructor
////////////////////////////////////////////////////////////////////
OrientedParticleFactory::
OrientedParticleFactory(void) :
  BaseParticleFactory() {
}

////////////////////////////////////////////////////////////////////
//    Function : OrientedParticleFactory
//      Access : Public
// Description : copy Constructor
////////////////////////////////////////////////////////////////////
OrientedParticleFactory::
OrientedParticleFactory(const OrientedParticleFactory &copy) :
  BaseParticleFactory(copy) {
  _initial_orientation = copy._initial_orientation;
  _final_orientation = copy._final_orientation;
}

////////////////////////////////////////////////////////////////////
//    Function : ~OrientedParticleFactory
//      Access : public
// Description : destructor
////////////////////////////////////////////////////////////////////
OrientedParticleFactory::
~OrientedParticleFactory(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : populate_child_particle
//      Access : private
// Description : child spawn
////////////////////////////////////////////////////////////////////
void OrientedParticleFactory::
populate_child_particle(BaseParticle *bp) const {
  bp->set_orientation(_initial_orientation);
}

////////////////////////////////////////////////////////////////////
//    Function : alloc_particle
//      Access : public
// Description : child particle generation function
////////////////////////////////////////////////////////////////////
BaseParticle *OrientedParticleFactory::
alloc_particle(void) const {
  return new OrientedParticle;
}

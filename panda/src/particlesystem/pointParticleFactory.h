// Filename: pointParticleFactory.h
// Created by:  charles (05Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef POINTPARTICLEFACTORY_H
#define POINTPARTICLEFACTORY_H

#include "baseParticleFactory.h"

////////////////////////////////////////////////////////////////////
//       Class : PointParticleFactory
// Description : Creates point particles to user specs
////////////////////////////////////////////////////////////////////

class EXPCL_PANDAPHYSICS PointParticleFactory : public BaseParticleFactory {
private:
  virtual BaseParticle *alloc_particle(void) const;
  virtual void populate_child_particle(BaseParticle *bp) const;

public:
  PointParticleFactory(void);
  PointParticleFactory(const PointParticleFactory &copy);
  virtual ~PointParticleFactory(void);
};

#endif // POINTPARTICLEFACTORY_H

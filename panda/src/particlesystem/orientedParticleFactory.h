// Filename: orientedParticleFactory.h
// Created by:  charles (05Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef ORIENTEDPARTICLEFACTORY_H
#define ORIENTEDPARTICLEFACTORY_H

#include "baseParticleFactory.h"

#include <luse.h>

////////////////////////////////////////////////////////////////////
//       Class : OrientedParticleFactory
// Description : Creates particles that are affected by angular
//               forces.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS OrientedParticleFactory : public BaseParticleFactory {
private:
  virtual void populate_child_particle(BaseParticle *bp) const;
  virtual BaseParticle *alloc_particle(void) const;

  LOrientationf _initial_orientation;
  LOrientationf _final_orientation;

public:
  OrientedParticleFactory(void);
  OrientedParticleFactory(const OrientedParticleFactory &copy);
  virtual ~OrientedParticleFactory(void);  

  INLINE void set_initial_orientation(const LOrientationf &o);
  INLINE void set_final_orientation(const LOrientationf &o);
  INLINE LOrientationf get_initial_orientation(void) const;
  INLINE LOrientationf get_final_orientation(void) const;
};

#include "orientedParticleFactory.I"

#endif // ORIENTEDPARTICLEFACTORY_H

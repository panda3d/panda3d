// Filename: zSpinParticleFactory.h
// Created by:  charles (16Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef ZSPINPARTICLEFACTORY_H
#define ZSPINPARTICLEFACTORY_H

#include "baseParticleFactory.h"

////////////////////////////////////////////////////////////////////
//       Class : ZSpinParticleFactory
// Description : see filename
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS ZSpinParticleFactory : 
  public BaseParticleFactory {
private:
  virtual void populate_child_particle(BaseParticle *bp) const;
  virtual BaseParticle *alloc_particle(void) const;

  float _initial_theta;
  float _final_theta;
  float _theta_delta;

public:
  ZSpinParticleFactory(void);
  ZSpinParticleFactory(const ZSpinParticleFactory &copy);
  virtual ~ZSpinParticleFactory(void);

  INLINE void set_initial_theta(float theta);
  INLINE void set_final_theta(float theta);
  INLINE void set_theta_delta(float delta);

  INLINE float get_initial_theta(void) const;
  INLINE float get_final_theta(void) const;
  INLINE float get_theta_delta(void) const;
};

#include "zSpinParticleFactory.I"

#endif // ZSPINPARTICLEFACTORY_H

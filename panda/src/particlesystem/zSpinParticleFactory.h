// Filename: zSpinParticleFactory.h
// Created by:  charles (16Aug00)
//
////////////////////////////////////////////////////////////////////

#ifndef ZSPINPARTICLEFACTORY_H
#define ZSPINPARTICLEFACTORY_H

#include "baseParticleFactory.h"

////////////////////////////////////////////////////////////////////
//       Class : ZSpinParticleFactory
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS ZSpinParticleFactory :
  public BaseParticleFactory {
private:
  virtual void populate_child_particle(BaseParticle *bp) const;
  virtual BaseParticle *alloc_particle(void) const;

  float _initial_angle;
  float _final_angle;
  float _initial_angle_spread;
  float _final_angle_spread;

PUBLISHED:
  ZSpinParticleFactory(void);
  ZSpinParticleFactory(const ZSpinParticleFactory &copy);
  virtual ~ZSpinParticleFactory(void);

  INLINE void set_initial_angle(float angle);
  INLINE void set_final_angle(float angle);
  INLINE void set_initial_angle_spread(float spread);
  INLINE void set_final_angle_spread(float spread);

  INLINE float get_initial_angle(void) const;
  INLINE float get_final_angle(void) const;
  INLINE float get_initial_angle_spread(void) const;
  INLINE float get_final_angle_spread(void) const;
};

#include "zSpinParticleFactory.I"

#endif // ZSPINPARTICLEFACTORY_H

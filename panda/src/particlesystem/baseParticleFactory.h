// Filename: baseParticleFactory.h
// Created by:  charles (05Jul00)
//
////////////////////////////////////////////////////////////////////

#ifndef BASEPARTICLEFACTORY_H
#define BASEPARTICLEFACTORY_H

#include <pandabase.h>
#include <referenceCount.h>

#include "baseParticle.h"
#include "particleCommonFuncs.h"

#include <stdlib.h>

////////////////////////////////////////////////////////////////////
//       Class : BaseParticleFactory
// Description : Pure Virtual base class for creating particles
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS BaseParticleFactory : public ReferenceCount {
private:
  float _lifespan_base;
  float _lifespan_spread;

  float _mass_base;
  float _mass_spread;

  float _terminal_velocity_base;
  float _terminal_velocity_spread;

  virtual void populate_child_particle(BaseParticle *bp) const = 0;

protected:
  BaseParticleFactory(void);
  BaseParticleFactory(const BaseParticleFactory &copy);

public:
  virtual ~BaseParticleFactory(void);

  INLINE void set_lifespan_base(float lb);
  INLINE void set_lifespan_spread(float ls);
  INLINE void set_mass_base(float mb);
  INLINE void set_mass_spread(float ms);
  INLINE void set_terminal_velocity_base(float tvb);
  INLINE void set_terminal_velocity_spread(float tvs);

  INLINE float get_lifespan_base(void) const;
  INLINE float get_lifespan_spread(void) const;
  INLINE float get_mass_base(void) const;
  INLINE float get_mass_spread(void) const;
  INLINE float get_terminal_velocity_base(void) const;
  INLINE float get_terminal_velocity_spread(void) const;

  virtual BaseParticle *alloc_particle(void) const = 0;

  void populate_particle(BaseParticle* bp);
};

#include "baseParticleFactory.I"

#endif // BASEPARTICLEFACTORY_H

// Filename: baseParticleFactory.h
// Created by:  charles (05Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef BASEPARTICLEFACTORY_H
#define BASEPARTICLEFACTORY_H

#include <pandabase.h>
#include <referenceCount.h>

#include "baseParticle.h"

////////////////////////////////////////////////////////////////////
//       Class : BaseParticleFactory 
// Description : Pure Virtual base class for creating particles
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS BaseParticleFactory : public ReferenceCount {
private:
  float _lifespan_base;
  float _lifespan_delta;

  float _mass_base;
  float _mass_delta;

  float _terminal_velocity_base;
  float _terminal_velocity_delta;

  virtual void populate_child_particle(BaseParticle *bp) const = 0;

protected:
  BaseParticleFactory(void);
  BaseParticleFactory(const BaseParticleFactory &copy);

  INLINE float bounded_rand(void) const;

public:
  virtual ~BaseParticleFactory(void);

  INLINE void set_lifespan_base(float lb);
  INLINE void set_lifespan_delta(float ld);
  INLINE void set_mass_base(float mb);
  INLINE void set_mass_delta(float md);
  INLINE void set_terminal_velocity_base(float tvb);
  INLINE void set_terminal_velocity_delta(float tvd);

  INLINE float get_lifespan_base(void) const;
  INLINE float get_lifespan_delta(void) const;
  INLINE float get_mass_base(void) const;
  INLINE float get_mass_delta(void) const;
  INLINE float get_terminal_velocity_base(void) const;
  INLINE float get_terminal_velocity_delta(void) const;

  virtual BaseParticle *alloc_particle(void) const = 0;

  void populate_particle(BaseParticle* bp);
};

#include "baseParticleFactory.I"

#endif // BASEPARTICLEFACTORY_H

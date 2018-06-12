/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file baseParticleFactory.h
 * @author charles
 * @date 2000-07-05
 */

#ifndef BASEPARTICLEFACTORY_H
#define BASEPARTICLEFACTORY_H

#include "pandabase.h"
#include "referenceCount.h"

#include "baseParticle.h"
#include "particleCommonFuncs.h"

#include <stdlib.h>

/**
 * Pure Virtual base class for creating particles
 */
class EXPCL_PANDA_PARTICLESYSTEM BaseParticleFactory : public ReferenceCount {
PUBLISHED:
  virtual ~BaseParticleFactory();

  INLINE void set_lifespan_base(PN_stdfloat lb);
  INLINE void set_lifespan_spread(PN_stdfloat ls);
  INLINE void set_mass_base(PN_stdfloat mb);
  INLINE void set_mass_spread(PN_stdfloat ms);
  INLINE void set_terminal_velocity_base(PN_stdfloat tvb);
  INLINE void set_terminal_velocity_spread(PN_stdfloat tvs);

  INLINE PN_stdfloat get_lifespan_base() const;
  INLINE PN_stdfloat get_lifespan_spread() const;
  INLINE PN_stdfloat get_mass_base() const;
  INLINE PN_stdfloat get_mass_spread() const;
  INLINE PN_stdfloat get_terminal_velocity_base() const;
  INLINE PN_stdfloat get_terminal_velocity_spread() const;

  virtual BaseParticle *alloc_particle() const = 0;

  void populate_particle(BaseParticle* bp);

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent=0) const;

protected:
  BaseParticleFactory();
  BaseParticleFactory(const BaseParticleFactory &copy);

private:
  PN_stdfloat _lifespan_base;
  PN_stdfloat _lifespan_spread;

  PN_stdfloat _mass_base;
  PN_stdfloat _mass_spread;

  PN_stdfloat _terminal_velocity_base;
  PN_stdfloat _terminal_velocity_spread;

  virtual void populate_child_particle(BaseParticle *bp) const = 0;
};

#include "baseParticleFactory.I"

#endif // BASEPARTICLEFACTORY_H

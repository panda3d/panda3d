// Filename: baseParticleFactory.h
// Created by:  charles (05Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef BASEPARTICLEFACTORY_H
#define BASEPARTICLEFACTORY_H

#include "pandabase.h"
#include "referenceCount.h"

#include "baseParticle.h"
#include "particleCommonFuncs.h"

#include <stdlib.h>

////////////////////////////////////////////////////////////////////
//       Class : BaseParticleFactory
// Description : Pure Virtual base class for creating particles
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS BaseParticleFactory : public ReferenceCount {
PUBLISHED:
  virtual ~BaseParticleFactory();

  INLINE void set_lifespan_base(float lb);
  INLINE void set_lifespan_spread(float ls);
  INLINE void set_mass_base(float mb);
  INLINE void set_mass_spread(float ms);
  INLINE void set_terminal_velocity_base(float tvb);
  INLINE void set_terminal_velocity_spread(float tvs);

  INLINE float get_lifespan_base() const;
  INLINE float get_lifespan_spread() const;
  INLINE float get_mass_base() const;
  INLINE float get_mass_spread() const;
  INLINE float get_terminal_velocity_base() const;
  INLINE float get_terminal_velocity_spread() const;

  virtual BaseParticle *alloc_particle() const = 0;

  void populate_particle(BaseParticle* bp);

protected:
  BaseParticleFactory();
  BaseParticleFactory(const BaseParticleFactory &copy);

private:
  float _lifespan_base;
  float _lifespan_spread;

  float _mass_base;
  float _mass_spread;

  float _terminal_velocity_base;
  float _terminal_velocity_spread;

  virtual void populate_child_particle(BaseParticle *bp) const = 0;
};

#include "baseParticleFactory.I"

#endif // BASEPARTICLEFACTORY_H

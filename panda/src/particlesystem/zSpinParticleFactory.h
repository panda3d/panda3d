// Filename: zSpinParticleFactory.h
// Created by:  charles (16Aug00)
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
  virtual BaseParticle *alloc_particle() const;

  float _initial_angle,_initial_angle_spread;
  float _final_angle,_final_angle_spread;
  float _angular_velocity,_angular_velocity_spread;
  bool  _bUseAngularVelocity;


PUBLISHED:
  ZSpinParticleFactory();
  ZSpinParticleFactory(const ZSpinParticleFactory &copy);
  virtual ~ZSpinParticleFactory();

  INLINE void set_initial_angle(float angle);
  INLINE void set_final_angle(float angle);
  INLINE void set_initial_angle_spread(float spread);
  INLINE void set_final_angle_spread(float spread);

  INLINE float get_initial_angle() const;
  INLINE float get_final_angle() const;
  INLINE float get_initial_angle_spread() const;
  INLINE float get_final_angle_spread() const;

  INLINE void  set_angular_velocity(float v);
  INLINE float get_angular_velocity() const;

  INLINE void  set_angular_velocity_spread(float spread);
  INLINE float get_angular_velocity_spread() const;

  INLINE void enable_angular_velocity(bool bEnabled);
  INLINE bool get_angular_velocity_enabled() const;
};

#include "zSpinParticleFactory.I"

#endif // ZSPINPARTICLEFACTORY_H

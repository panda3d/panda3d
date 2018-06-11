/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file zSpinParticleFactory.h
 * @author charles
 * @date 2000-08-16
 */

#ifndef ZSPINPARTICLEFACTORY_H
#define ZSPINPARTICLEFACTORY_H

#include "baseParticleFactory.h"

/**
 *
 */
class EXPCL_PANDA_PARTICLESYSTEM ZSpinParticleFactory : public BaseParticleFactory {
PUBLISHED:
  ZSpinParticleFactory();
  ZSpinParticleFactory(const ZSpinParticleFactory &copy);
  virtual ~ZSpinParticleFactory();

  INLINE void set_initial_angle(PN_stdfloat angle);
  INLINE void set_final_angle(PN_stdfloat angle);
  INLINE void set_initial_angle_spread(PN_stdfloat spread);
  INLINE void set_final_angle_spread(PN_stdfloat spread);

  INLINE PN_stdfloat get_initial_angle() const;
  INLINE PN_stdfloat get_final_angle() const;
  INLINE PN_stdfloat get_initial_angle_spread() const;
  INLINE PN_stdfloat get_final_angle_spread() const;

  INLINE void  set_angular_velocity(PN_stdfloat v);
  INLINE PN_stdfloat get_angular_velocity() const;

  INLINE void  set_angular_velocity_spread(PN_stdfloat spread);
  INLINE PN_stdfloat get_angular_velocity_spread() const;

  INLINE void enable_angular_velocity(bool bEnabled);
  INLINE bool get_angular_velocity_enabled() const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent=0) const;

private:
  PN_stdfloat _initial_angle;
  PN_stdfloat _initial_angle_spread;
  PN_stdfloat _final_angle;
  PN_stdfloat _final_angle_spread;
  PN_stdfloat _angular_velocity;
  PN_stdfloat _angular_velocity_spread;
  bool  _bUseAngularVelocity;

  virtual void populate_child_particle(BaseParticle *bp) const;
  virtual BaseParticle *alloc_particle() const;
};

#include "zSpinParticleFactory.I"

#endif // ZSPINPARTICLEFACTORY_H

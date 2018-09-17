/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file zSpinParticle.h
 * @author charles
 * @date 2000-08-16
 */

#ifndef ZSPINPARTICLE_H
#define ZSPINPARTICLE_H

#include "baseParticle.h"

/**
 * describes a particle that spins along its z axis.  this is kind of an
 * intermediary class- if you're using a SpriteParticleRenderer and you want
 * your sprites to spin without having them be full-blown oriented (i.e.
 * angry quat math), use this.
 */
class EXPCL_PANDA_PARTICLESYSTEM ZSpinParticle : public BaseParticle {
public:
  ZSpinParticle();
  ZSpinParticle(const ZSpinParticle &copy);
  virtual ~ZSpinParticle();

  virtual PhysicsObject *make_copy() const;

  virtual void init();
  virtual void update();
  virtual void die();

  virtual PN_stdfloat get_theta() const;

  INLINE void set_initial_angle(PN_stdfloat t);
  INLINE PN_stdfloat get_initial_angle() const;

  INLINE void set_final_angle(PN_stdfloat t);
  INLINE PN_stdfloat get_final_angle() const;

  // 'set_final_angle' and 'angular_velocity' are mutually exclusive apis if
  // angular-velocity is specified, final_angle is ignored
  INLINE void  set_angular_velocity(PN_stdfloat v);
  INLINE PN_stdfloat get_angular_velocity() const;

  INLINE void enable_angular_velocity(bool bEnabled);
  INLINE bool get_angular_velocity_enabled() const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent=0) const;

private:
  PN_stdfloat _initial_angle;
  PN_stdfloat _final_angle;
  PN_stdfloat _cur_angle;
  PN_stdfloat _angular_velocity;
  bool  _bUseAngularVelocity;
};

#include "zSpinParticle.I"

#endif // ZSPINPARTICLE_H

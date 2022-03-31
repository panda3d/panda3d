/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file baseParticle.h
 * @author charles
 * @date 2000-06-14
 */

#ifndef BASEPARTICLE_H
#define BASEPARTICLE_H

#include "pandabase.h"
#include "physicsObject.h"

/**
 * An individual, physically-modelable particle abstract base class.
 */
class EXPCL_PANDA_PARTICLESYSTEM BaseParticle : public PhysicsObject {
public:
  // local methods
  INLINE void set_age(PN_stdfloat age);
  INLINE void set_lifespan(PN_stdfloat lifespan);
  INLINE void set_alive(bool alive);
  INLINE void set_index(int index);


  INLINE PN_stdfloat get_age() const;
  INLINE PN_stdfloat get_lifespan() const;
  INLINE bool get_alive() const;
  INLINE int get_index() const;

  INLINE PN_stdfloat get_parameterized_age() const;
  INLINE PN_stdfloat get_parameterized_vel() const;

  // child methods
  virtual void init() = 0;
  virtual void die() = 0;
  virtual void update() = 0;

  // for spriteParticleRenderer
  virtual PN_stdfloat get_theta() const;

  // from PhysicsObject
  virtual PhysicsObject *make_copy() const = 0;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent=0) const;

protected:
  BaseParticle(PN_stdfloat lifespan = 1.0f, bool alive = false);
  BaseParticle(const BaseParticle &copy);
  virtual ~BaseParticle();

private:
  // NOTE: age and lifespan are in seconds.
  PN_stdfloat _age;
  PN_stdfloat _lifespan;
  bool _alive;
  int _index;
};

#include "baseParticle.I"

#endif // BASEPARTICLE_H

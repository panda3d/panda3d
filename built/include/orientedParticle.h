/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file orientedParticle.h
 * @author charles
 * @date 2000-06-19
 */

#ifndef ORIENTEDPARTICLE_H
#define ORIENTEDPARTICLE_H

#include "baseParticle.h"

/**
 * Describes a particle that has angular characteristics (velocity,
 * orientation).
 */
class EXPCL_PANDA_PARTICLESYSTEM OrientedParticle : public BaseParticle {
public:
  OrientedParticle(int lifespan = 0, bool alive = false);
  OrientedParticle(const OrientedParticle &copy);
  virtual ~OrientedParticle();

  virtual PhysicsObject *make_copy() const;

  INLINE void set_velocity();
  INLINE void set_orientation();

  virtual void init();
  virtual void update();
  virtual void die();

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent=0) const;
};

#include "orientedParticle.I"

#endif // ORIENTEDPARTICLE_H

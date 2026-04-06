/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointParticle.h
 * @author charles
 * @date 2000-06-19
 */

#ifndef POINTPARTICLE_H
#define POINTPARTICLE_H

#include "baseParticle.h"

/**
 * Describes a particle that requires representation by a point (pixel,
 * sparkle, billboard)
 */
class EXPCL_PANDA_PARTICLESYSTEM PointParticle : public BaseParticle {
public:
  PointParticle(PN_stdfloat lifespan = 0.0f, bool alive = false);
  PointParticle(const PointParticle &copy);
  virtual ~PointParticle();

  virtual void init();
  virtual void die();
  virtual void update();

  virtual PhysicsObject *make_copy() const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent=0) const;
};

#endif // POINTPARTICLE_H

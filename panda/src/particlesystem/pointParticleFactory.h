/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointParticleFactory.h
 * @author charles
 * @date 2000-07-05
 */

#ifndef POINTPARTICLEFACTORY_H
#define POINTPARTICLEFACTORY_H

#include "baseParticleFactory.h"

/**
 * Creates point particles to user specs
 */

class EXPCL_PANDA_PARTICLESYSTEM PointParticleFactory : public BaseParticleFactory {
PUBLISHED:
  PointParticleFactory();
  PointParticleFactory(const PointParticleFactory &copy);
  virtual ~PointParticleFactory();

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent=0) const;

private:
  virtual BaseParticle *alloc_particle() const;
  virtual void populate_child_particle(BaseParticle *bp) const;
};

#endif // POINTPARTICLEFACTORY_H

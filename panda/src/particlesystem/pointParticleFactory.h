// Filename: pointParticleFactory.h
// Created by:  charles (05Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef POINTPARTICLEFACTORY_H
#define POINTPARTICLEFACTORY_H

#include "baseParticleFactory.h"

////////////////////////////////////////////////////////////////////
//       Class : PointParticleFactory
// Description : Creates point particles to user specs
////////////////////////////////////////////////////////////////////

class EXPCL_PANDAPHYSICS PointParticleFactory : public BaseParticleFactory {
PUBLISHED:
  PointParticleFactory();
  PointParticleFactory(const PointParticleFactory &copy);
  virtual ~PointParticleFactory();

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent=0) const;

private:
  virtual BaseParticle *alloc_particle() const;
  virtual void populate_child_particle(BaseParticle *bp) const;
};

#endif // POINTPARTICLEFACTORY_H

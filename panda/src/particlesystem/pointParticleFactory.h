// Filename: pointParticleFactory.h
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

private:
  virtual BaseParticle *alloc_particle() const;
  virtual void populate_child_particle(BaseParticle *bp) const;
};

#endif // POINTPARTICLEFACTORY_H

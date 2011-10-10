// Filename: orientedParticleFactory.h
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

#ifndef ORIENTEDPARTICLEFACTORY_H
#define ORIENTEDPARTICLEFACTORY_H

#include "baseParticleFactory.h"

#include "luse.h"

////////////////////////////////////////////////////////////////////
//       Class : OrientedParticleFactory
// Description : Creates particles that are affected by angular
//               forces.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS OrientedParticleFactory : public BaseParticleFactory {
PUBLISHED:
  OrientedParticleFactory();
  OrientedParticleFactory(const OrientedParticleFactory &copy);
  virtual ~OrientedParticleFactory();

  INLINE void set_initial_orientation(const LOrientation &o);
  INLINE void set_final_orientation(const LOrientation &o);
  INLINE LOrientation get_initial_orientation() const;
  INLINE LOrientation get_final_orientation() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, unsigned int indent=0) const;

private:
  virtual void populate_child_particle(BaseParticle *bp) const;
  virtual BaseParticle *alloc_particle() const;

  LOrientation _initial_orientation;
  LOrientation _final_orientation;
};

#include "orientedParticleFactory.I"

#endif // ORIENTEDPARTICLEFACTORY_H

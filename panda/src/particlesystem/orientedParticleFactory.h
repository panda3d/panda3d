// Filename: orientedParticleFactory.h
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

  INLINE void set_initial_orientation(const LOrientationf &o);
  INLINE void set_final_orientation(const LOrientationf &o);
  INLINE LOrientationf get_initial_orientation() const;
  INLINE LOrientationf get_final_orientation() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, unsigned int indent=0) const;

private:
  virtual void populate_child_particle(BaseParticle *bp) const;
  virtual BaseParticle *alloc_particle() const;

  LOrientationf _initial_orientation;
  LOrientationf _final_orientation;
};

#include "orientedParticleFactory.I"

#endif // ORIENTEDPARTICLEFACTORY_H

// Filename: orientedParticle.h
// Created by:  charles (19Jun00)
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

#ifndef ORIENTEDPARTICLE_H
#define ORIENTEDPARTICLE_H

#include "baseParticle.h"

///////////////////////////////////////////////////////////////////
//        Class : OrientedParticle
//  Description : Describes a particle that has angular
//                characteristics (velocity, orientation).
///////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS OrientedParticle : public BaseParticle {
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

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent=0) const;
};

#include "orientedParticle.I"

#endif // ORIENTEDPARTICLE_H

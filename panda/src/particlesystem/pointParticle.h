// Filename: pointParticle.h
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

#ifndef POINTPARTICLE_H
#define POINTPARTICLE_H

#include "baseParticle.h"

////////////////////////////////////////////////////////////////////
//       Class : PointParticle
// Description : Describes a particle that requires representation
//               by a point (pixel, sparkle, billboard)
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS PointParticle : public BaseParticle {
public:
  PointParticle(int lifespan = 0, bool alive = false);
  PointParticle(const PointParticle &copy);
  virtual ~PointParticle(void);

  virtual void init(void);
  virtual void die(void);
  virtual void update(void);

  virtual PhysicsObject *make_copy(void) const;
};

#endif // POINTPARTICLE_H

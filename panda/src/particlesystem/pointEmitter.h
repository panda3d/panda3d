// Filename: pointEmitter.h
// Created by:  charles (22Jun00)
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

#ifndef POINTEMITTER_H
#define POINTEMITTER_H

#include "baseParticleEmitter.h"

////////////////////////////////////////////////////////////////////
//       Class : PointEmitter
// Description : Describes a planar ring region in which
//               particles are generated.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS PointEmitter : public BaseParticleEmitter {
PUBLISHED:
  PointEmitter(void);
  PointEmitter(const PointEmitter &copy);
  virtual ~PointEmitter(void);

  virtual BaseParticleEmitter *make_copy(void);

  INLINE void set_location(const LPoint3f& p);
  INLINE LPoint3f get_location(void) const;

private:
  LPoint3f _location;

  // CUSTOM EMISSION PARAMETERS
  // none

  virtual void assign_initial_position(LPoint3f& pos);
  virtual void assign_initial_velocity(LVector3f& vel);
};

#include "pointEmitter.I"

#endif // POINTEMITTER_H

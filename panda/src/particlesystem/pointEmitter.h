// Filename: pointEmitter.h
// Created by:  charles (22Jun00)
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
  PointEmitter();
  PointEmitter(const PointEmitter &copy);
  virtual ~PointEmitter();

  virtual BaseParticleEmitter *make_copy();

  INLINE void set_location(const LPoint3& p);
  INLINE LPoint3 get_location() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent=0) const;

private:
  LPoint3 _location;

  // CUSTOM EMISSION PARAMETERS
  // none

  virtual void assign_initial_position(LPoint3& pos);
  virtual void assign_initial_velocity(LVector3& vel);
};

#include "pointEmitter.I"

#endif // POINTEMITTER_H

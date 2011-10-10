// Filename: sphereSurfaceEmitter.h
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

#ifndef SPHERESURFACEEMITTER_H
#define SPHERESURFACEEMITTER_H

#include "baseParticleEmitter.h"

////////////////////////////////////////////////////////////////////
//       Class : SphereSurfaceEmitter
// Description : Describes a curved space in which
//               particles are generated.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS SphereSurfaceEmitter : public BaseParticleEmitter {
PUBLISHED:
  SphereSurfaceEmitter();
  SphereSurfaceEmitter(const SphereSurfaceEmitter &copy);
  virtual ~SphereSurfaceEmitter();

  virtual BaseParticleEmitter *make_copy();

  INLINE void set_radius(PN_stdfloat r);
  INLINE PN_stdfloat get_radius() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent=0) const;

private:
  PN_stdfloat _radius;

  // CUSTOM EMISSION PARAMETERS
  // none

  virtual void assign_initial_position(LPoint3& pos);
  virtual void assign_initial_velocity(LVector3& vel);
};

#include "sphereSurfaceEmitter.I"

#endif // SPHERESURFACEEMITTER_H

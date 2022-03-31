/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file tangentRingEmitter.h
 * @author charles
 * @date 2000-07-25
 */

#ifndef TANGENTRINGEMITTER_H
#define TANGENTRINGEMITTER_H

#include "baseParticleEmitter.h"

/**
 * Describes a planar ring region in which tangent particles are generated,
 * and particles fly off tangential to the ring.
 */
class EXPCL_PANDA_PARTICLESYSTEM TangentRingEmitter : public BaseParticleEmitter {
PUBLISHED:
  TangentRingEmitter();
  TangentRingEmitter(const TangentRingEmitter &copy);
  virtual ~TangentRingEmitter();

  virtual BaseParticleEmitter *make_copy();

  INLINE void set_radius(PN_stdfloat r);
  INLINE void set_radius_spread(PN_stdfloat spread);

  INLINE PN_stdfloat get_radius() const;
  INLINE PN_stdfloat get_radius_spread() const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent=0) const;

private:
  PN_stdfloat _radius;
  PN_stdfloat _radius_spread;

  // CUSTOM EMISSION PARAMETERS none

  // scratch variables that carry over from position calc to velocity calc
  PN_stdfloat _x;
  PN_stdfloat _y;

  virtual void assign_initial_position(LPoint3& pos);
  virtual void assign_initial_velocity(LVector3& vel);
};

#include "tangentRingEmitter.I"

#endif // TANGENTRINGEMITTER_H

/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointEmitter.h
 * @author charles
 * @date 2000-06-22
 */

#ifndef POINTEMITTER_H
#define POINTEMITTER_H

#include "baseParticleEmitter.h"

/**
 * Describes a planar ring region in which particles are generated.
 */
class EXPCL_PANDA_PARTICLESYSTEM PointEmitter : public BaseParticleEmitter {
PUBLISHED:
  PointEmitter();
  PointEmitter(const PointEmitter &copy);
  virtual ~PointEmitter();

  virtual BaseParticleEmitter *make_copy();

  INLINE void set_location(const LPoint3& p);
  INLINE LPoint3 get_location() const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent=0) const;

private:
  LPoint3 _location;

  // CUSTOM EMISSION PARAMETERS none

  virtual void assign_initial_position(LPoint3& pos);
  virtual void assign_initial_velocity(LVector3& vel);
};

#include "pointEmitter.I"

#endif // POINTEMITTER_H

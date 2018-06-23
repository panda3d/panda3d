/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lineEmitter.h
 * @author charles
 * @date 2000-06-22
 */

#ifndef LINEEMITTER_H
#define LINEEMITTER_H

#include "baseParticleEmitter.h"

/**
 * Describes a linear region in which particles are generated.
 */
class EXPCL_PANDA_PARTICLESYSTEM LineEmitter : public BaseParticleEmitter {
PUBLISHED:
  LineEmitter();
  LineEmitter(const LineEmitter &copy);
  virtual ~LineEmitter();

  virtual BaseParticleEmitter *make_copy();

  INLINE void set_endpoint1(const LPoint3& point);
  INLINE void set_endpoint2(const LPoint3& point);

  INLINE LPoint3 get_endpoint1() const;
  INLINE LPoint3 get_endpoint2() const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent=0) const;

private:
  LPoint3 _endpoint1;
  LPoint3 _endpoint2;

  // CUSTOM EMISSION PARAMETERS none

  virtual void assign_initial_position(LPoint3& pos);
  virtual void assign_initial_velocity(LVector3& vel);
};

#include "lineEmitter.I"

#endif // LINEEMITTER_H

/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sphereVolumeEmitter.h
 * @author charles
 * @date 2000-06-22
 */

#ifndef SPHEREVOLUMEEMITTER_H
#define SPHEREVOLUMEEMITTER_H

#include "baseParticleEmitter.h"

/**
 * Describes a voluminous spherical region in which particles are generated.
 */
class EXPCL_PANDA_PARTICLESYSTEM SphereVolumeEmitter : public BaseParticleEmitter {
PUBLISHED:
  SphereVolumeEmitter();
  SphereVolumeEmitter(const SphereVolumeEmitter &copy);
  virtual ~SphereVolumeEmitter();

  virtual BaseParticleEmitter *make_copy();

  INLINE void set_radius(PN_stdfloat r);
  INLINE PN_stdfloat get_radius() const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent=0) const;

private:
  PN_stdfloat _radius;

  // CUSTOM EMISSION PARAMETERS none

  // scratch variables that carry over from position calc to velocity calc
  LPoint3 _particle_pos;

  virtual void assign_initial_position(LPoint3& pos);
  virtual void assign_initial_velocity(LVector3& vel);
};

#include "sphereVolumeEmitter.I"

#endif // SPHEREVOLUMEEMITTER_H

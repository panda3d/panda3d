// Filename: sphereVolumeEmitter.h
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

#ifndef SPHEREVOLUMEEMITTER_H
#define SPHEREVOLUMEEMITTER_H

#include "baseParticleEmitter.h"

////////////////////////////////////////////////////////////////////
//       Class : SphereVolumeEmitter
// Description : Describes a voluminous spherical region in which
//               particles are generated.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS SphereVolumeEmitter : public BaseParticleEmitter {
PUBLISHED:
  SphereVolumeEmitter();
  SphereVolumeEmitter(const SphereVolumeEmitter &copy);
  virtual ~SphereVolumeEmitter();

  virtual BaseParticleEmitter *make_copy();

  INLINE void set_radius(float r);
  INLINE float get_radius() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent=0) const;

private:
  float _radius;

  // CUSTOM EMISSION PARAMETERS
  // none

  ///////////////////////////////
  // scratch variables that carry over from position calc to velocity calc
  LPoint3f _particle_pos;
  ///////////////////////////////

  virtual void assign_initial_position(LPoint3f& pos);
  virtual void assign_initial_velocity(LVector3f& vel);
};

#include "sphereVolumeEmitter.I"

#endif // SPHEREVOLUMEEMITTER_H

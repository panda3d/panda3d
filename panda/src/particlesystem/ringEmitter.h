// Filename: ringEmitter.h
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

#ifndef RINGEMITTER_H
#define RINGEMITTER_H

#include "baseParticleEmitter.h"

////////////////////////////////////////////////////////////////////
//       Class : RingEmitter
// Description : Describes a planar ring region in which
//               particles are generated.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS RingEmitter : public BaseParticleEmitter {
PUBLISHED:
  RingEmitter();
  RingEmitter(const RingEmitter &copy);
  virtual ~RingEmitter();

  virtual BaseParticleEmitter *make_copy();

  INLINE void set_radius(float r);
  INLINE void set_angle(float angle);
  INLINE void set_radius_spread(float spread);
  INLINE void set_uniform_emission(int uniform_emission);

  INLINE float get_radius() const;
  INLINE float get_angle() const;
  INLINE float get_radius_spread() const;
  INLINE int get_uniform_emission() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent=0) const;

protected:
  float _radius;
  float _radius_spread;

  // CUSTOM EMISSION PARAMETERS
  float _aoe;  // angle of elevation

  // viariables used for uniform particle emission
  int _uniform_emission;
  float _theta;

  ///////////////////////////////
  // scratch variables that carry over from position calc to velocity calc
  float _sin_theta;
  float _cos_theta;
  ///////////////////////////////

private:
  virtual void assign_initial_position(LPoint3f& pos);
  virtual void assign_initial_velocity(LVector3f& vel);
};

#include "ringEmitter.I"

#endif // RINGEMITTER_H

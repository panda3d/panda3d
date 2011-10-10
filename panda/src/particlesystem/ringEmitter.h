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

  INLINE void set_radius(PN_stdfloat r);
  INLINE void set_angle(PN_stdfloat angle);
  INLINE void set_radius_spread(PN_stdfloat spread);
  INLINE void set_uniform_emission(int uniform_emission);

  INLINE PN_stdfloat get_radius() const;
  INLINE PN_stdfloat get_angle() const;
  INLINE PN_stdfloat get_radius_spread() const;
  INLINE int get_uniform_emission() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent=0) const;

protected:
  PN_stdfloat _radius;
  PN_stdfloat _radius_spread;

  // CUSTOM EMISSION PARAMETERS
  PN_stdfloat _aoe;  // angle of elevation

  // viariables used for uniform particle emission
  int _uniform_emission;
  PN_stdfloat _theta;

  ///////////////////////////////
  // scratch variables that carry over from position calc to velocity calc
  PN_stdfloat _sin_theta;
  PN_stdfloat _cos_theta;
  ///////////////////////////////

private:
  virtual void assign_initial_position(LPoint3& pos);
  virtual void assign_initial_velocity(LVector3& vel);
};

#include "ringEmitter.I"

#endif // RINGEMITTER_H

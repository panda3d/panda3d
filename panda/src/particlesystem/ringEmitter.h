// Filename: ringEmitter.h
// Created by:  charles (22Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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

  INLINE float get_radius() const;
  INLINE float get_angle() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent=0) const;

private:
  float _radius;

  // CUSTOM EMISSION PARAMETERS
  float _aoe;  // angle of elevation

  ///////////////////////////////
  // scratch variables that carry over from position calc to velocity calc
  float _sin_theta;
  float _cos_theta;
  ///////////////////////////////

  virtual void assign_initial_position(LPoint3f& pos);
  virtual void assign_initial_velocity(LVector3f& vel);
};

#include "ringEmitter.I"

#endif // RINGEMITTER_H

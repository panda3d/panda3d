// Filename: discEmitter.h
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

#ifndef DISCEMITTER_H
#define DISCEMITTER_H

#include "baseParticleEmitter.h"

////////////////////////////////////////////////////////////////////
//       Class : DiscEmitter
// Description : Describes a planar disc region from which particles
//               are generated
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS DiscEmitter : public BaseParticleEmitter {
PUBLISHED:
  DiscEmitter();
  DiscEmitter(const DiscEmitter &copy);
  virtual ~DiscEmitter();

  virtual BaseParticleEmitter *make_copy();

  INLINE void set_radius(float r);
  INLINE void set_outer_angle(float o_angle);
  INLINE void set_inner_angle(float i_angle);
  INLINE void set_outer_magnitude(float o_mag);
  INLINE void set_inner_magnitude(float i_mag);
  INLINE void set_cubic_lerping(bool clerp);

  INLINE float get_radius() const;
  INLINE float get_outer_angle() const;
  INLINE float get_inner_angle() const;
  INLINE float get_outer_magnitude() const;
  INLINE float get_inner_magnitude() const;
  INLINE bool get_cubic_lerping() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent=0) const;

private:
  float _radius;

  // CUSTOM EMISSION PARAMETERS
  float _inner_aoe;
  float _outer_aoe;
  float _inner_magnitude;
  float _outer_magnitude;
  bool _cubic_lerping;

  ///////////////////////////////
  // scratch variables that carry over from position calc to velocity calc
  float _distance_from_center;
  float _sinf_theta;
  float _cosf_theta;
  ///////////////////////////////

  virtual void assign_initial_position(LPoint3f& pos);
  virtual void assign_initial_velocity(LVector3f& vel);
};

#include "discEmitter.I"

#endif // DISCEMITTER_H

/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file discEmitter.h
 * @author charles
 * @date 2000-06-22
 */

#ifndef DISCEMITTER_H
#define DISCEMITTER_H

#include "baseParticleEmitter.h"

/**
 * Describes a planar disc region from which particles are generated
 */
class EXPCL_PANDA_PARTICLESYSTEM DiscEmitter : public BaseParticleEmitter {
PUBLISHED:
  DiscEmitter();
  DiscEmitter(const DiscEmitter &copy);
  virtual ~DiscEmitter();

  virtual BaseParticleEmitter *make_copy();

  INLINE void set_radius(PN_stdfloat r);
  INLINE void set_outer_angle(PN_stdfloat o_angle);
  INLINE void set_inner_angle(PN_stdfloat i_angle);
  INLINE void set_outer_magnitude(PN_stdfloat o_mag);
  INLINE void set_inner_magnitude(PN_stdfloat i_mag);
  INLINE void set_cubic_lerping(bool clerp);

  INLINE PN_stdfloat get_radius() const;
  INLINE PN_stdfloat get_outer_angle() const;
  INLINE PN_stdfloat get_inner_angle() const;
  INLINE PN_stdfloat get_outer_magnitude() const;
  INLINE PN_stdfloat get_inner_magnitude() const;
  INLINE bool get_cubic_lerping() const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent=0) const;

private:
  PN_stdfloat _radius;

  // CUSTOM EMISSION PARAMETERS
  PN_stdfloat _inner_aoe;
  PN_stdfloat _outer_aoe;
  PN_stdfloat _inner_magnitude;
  PN_stdfloat _outer_magnitude;
  bool _cubic_lerping;

  // scratch variables that carry over from position calc to velocity calc
  PN_stdfloat _distance_from_center;
  PN_stdfloat _sinf_theta;
  PN_stdfloat _cosf_theta;

  virtual void assign_initial_position(LPoint3& pos);
  virtual void assign_initial_velocity(LVector3& vel);
};

#include "discEmitter.I"

#endif // DISCEMITTER_H

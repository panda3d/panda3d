// Filename: discEmitter.h
// Created by:  charles (22Jun00)
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
private:
  float _radius;

  // CUSTOM EMISSION PARAMETERS
  float _outer_aoe, _inner_aoe;
  float _outer_magnitude, _inner_magnitude;
  bool _cubic_lerping;

  ///////////////////////////////
  // scratch variables that carry over from position calc to velocity calc
  float _distance_from_center;
  float _sinf_theta;
  float _cosf_theta;
  ///////////////////////////////

  virtual void assign_initial_position(LPoint3f& pos);
  virtual void assign_initial_velocity(LVector3f& vel);

PUBLISHED:
  DiscEmitter(void);
  DiscEmitter(const DiscEmitter &copy);
  virtual ~DiscEmitter(void);

  virtual BaseParticleEmitter *make_copy(void);

  INLINE void set_radius(float r);
  INLINE void set_outer_angle(float o_angle);
  INLINE void set_inner_angle(float i_angle);
  INLINE void set_outer_magnitude(float o_mag);
  INLINE void set_inner_magnitude(float i_mag);
  INLINE void set_cubic_lerping(bool clerp);

  INLINE float get_radius(void) const;
  INLINE float get_outer_angle(void) const;
  INLINE float get_inner_angle(void) const;
  INLINE float get_outer_magnitude(void) const;
  INLINE float get_inner_magnitude(void) const;
  INLINE bool get_cubic_lerping(void) const;
};

#include "discEmitter.I"

#endif // DISCEMITTER_H

// Filename: ringEmitter.h
// Created by:  charles (22Jun00)
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

PUBLISHED:
  RingEmitter(void);
  RingEmitter(const RingEmitter &copy);
  virtual ~RingEmitter(void);

  virtual BaseParticleEmitter *make_copy(void);

  INLINE void set_radius(float r);
  INLINE void set_angle(float angle);

  INLINE float get_radius(void) const;
  INLINE float get_angle(void) const;
};

#include "ringEmitter.I"

#endif // RINGEMITTER_H

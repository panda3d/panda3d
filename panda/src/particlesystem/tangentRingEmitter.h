// Filename: tangentRingEmitter.h
// Created by:  charles (25Jul00)
//
////////////////////////////////////////////////////////////////////

#ifndef TANGENTRINGEMITTER_H
#define TANGENTRINGEMITTER_H

#include "baseParticleEmitter.h"

////////////////////////////////////////////////////////////////////
//       Class : TangentRingEmitter
// Description : Describes a planar ring region in which
//               tangent particles are generated, and particles
//               fly off tangential to the ring.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS TangentRingEmitter : public BaseParticleEmitter {
private:
  float _radius;

  // CUSTOM EMISSION PARAMETERS
  // none

  ///////////////////////////////
  // scratch variables that carry over from position calc to velocity calc
  float _x, _y;
  ///////////////////////////////

  virtual void assign_initial_position(LPoint3f& pos);
  virtual void assign_initial_velocity(LVector3f& vel);

PUBLISHED:
  TangentRingEmitter(void);
  TangentRingEmitter(const TangentRingEmitter &copy);
  virtual ~TangentRingEmitter(void);

  virtual BaseParticleEmitter *make_copy(void);

  INLINE void set_radius(float r);
  INLINE float get_radius(void) const;
};

#include "tangentRingEmitter.I"

#endif // TANGENTRINGEMITTER_H

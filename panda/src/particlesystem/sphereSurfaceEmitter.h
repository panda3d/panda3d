// Filename: sphereSurfaceEmitter.h
// Created by:  charles (22Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef SPHERESURFACEEMITTER_H
#define SPHERESURFACEEMITTER_H

#include "baseParticleEmitter.h"

////////////////////////////////////////////////////////////////////
//       Class : SphereSurfaceEmitter
// Description : Describes a curved space in which
//               particles are generated.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS SphereSurfaceEmitter : public BaseParticleEmitter {
private:
  float _radius;

  virtual void assign_initial_position(LPoint3f& pos);
  virtual void assign_initial_velocity(LVector3f& vel);

PUBLISHED:
  SphereSurfaceEmitter(void);
  SphereSurfaceEmitter(const SphereSurfaceEmitter &copy);
  virtual ~SphereSurfaceEmitter(void);

  virtual BaseParticleEmitter *make_copy(void);

  INLINE void set_radius(float r);
  INLINE float get_radius(void) const;
};

#include "sphereSurfaceEmitter.I"

#endif // SPHERESURFACEEMITTER_H

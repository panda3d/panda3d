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

  virtual void assign_initial_values(LPoint3f& pos, LVector3f& vel);

public:
  SphereSurfaceEmitter(void);
  SphereSurfaceEmitter(const SphereSurfaceEmitter &copy);
  virtual ~SphereSurfaceEmitter(void);

  virtual BaseParticleEmitter *make_copy(void);

  INLINE void set_radius(float r);
};

#include "sphereSurfaceEmitter.I"

#endif // SPHERESURFACEEMITTER_H

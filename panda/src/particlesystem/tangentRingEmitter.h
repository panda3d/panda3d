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
//               tangent particles are generated.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS TangentRingEmitter : public BaseParticleEmitter {
private:
  float _radius;

  virtual void assign_initial_values(LPoint3f& pos, LVector3f& vel);

public:
  TangentRingEmitter(void);
  TangentRingEmitter(const TangentRingEmitter &copy);
  virtual ~TangentRingEmitter(void);

  virtual BaseParticleEmitter *make_copy(void);

  INLINE void set_radius(float r);
  INLINE float get_radius(void) const;
};

#include "tangentRingEmitter.I"

#endif // TANGENTRINGEMITTER_H

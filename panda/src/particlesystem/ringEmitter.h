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
  float _aoe;  // angle of elevation
  float _mag;

  virtual void assign_initial_values(LPoint3f& pos, LVector3f& vel);

public:
  RingEmitter(void);
  RingEmitter(const RingEmitter &copy);
  virtual ~RingEmitter(void);

  virtual BaseParticleEmitter *make_copy(void);

  INLINE void set_radius(float r);
  INLINE void set_aoe(float aoe);
  INLINE void set_magnitude(float m);
};

#include "ringEmitter.I"

#endif // RINGEMITTER_H

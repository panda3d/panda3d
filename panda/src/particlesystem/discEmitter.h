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
  float _outer_aoe, _inner_aoe;
  float _outer_magnitude, _inner_magnitude;
  bool _cubic_lerping;

  virtual void assign_initial_values(LPoint3f& pos, LVector3f& vel);

  INLINE float lerp(float t, float x0, float x1);
  INLINE float cubic_lerp(float t, float x0, float x1);
  
public:
  DiscEmitter(void);
  DiscEmitter(const DiscEmitter &copy);
  virtual ~DiscEmitter(void);

  virtual BaseParticleEmitter *make_copy(void);

  INLINE void set_radius(float r);
  INLINE void set_outer_aoe(float o_aoe);
  INLINE void set_inner_aoe(float i_aoe);
  INLINE void set_outer_magnitude(float o_mag);
  INLINE void set_inner_magnitude(float i_mag);
  INLINE void set_cubic_lerping(bool clerp);
};

#include "discEmitter.I"

#endif // DISCEMITTER_H

// Filename: pointEmitter.h
// Created by:  charles (22Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef POINTEMITTER_H
#define POINTEMITTER_H

#include "baseParticleEmitter.h"

////////////////////////////////////////////////////////////////////
//       Class : PointEmitter
// Description : Describes a planar ring region in which
//               particles are generated.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS PointEmitter : public BaseParticleEmitter {
private:
  LPoint3f _point;
  LVector3f _launch_vec;
 
  virtual void assign_initial_values(LPoint3f& pos, LVector3f& vel);
 
public:
  PointEmitter(void);
  PointEmitter(const PointEmitter &copy);
  virtual ~PointEmitter(void);

  virtual BaseParticleEmitter *make_copy(void);
   
  INLINE void set_point(const LPoint3f& p);
  INLINE void set_launch_vec(const LVector3f &v);
};

#include "pointEmitter.I"

#endif // POINTEMITTER_H

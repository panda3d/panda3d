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
  LPoint3f _location;

  virtual void assign_initial_position(LPoint3f& pos);
  virtual void assign_initial_velocity(LVector3f& vel);
 
public:
  PointEmitter(void);
  PointEmitter(const PointEmitter &copy);
  virtual ~PointEmitter(void);

  virtual BaseParticleEmitter *make_copy(void);

  INLINE void set_location(const LPoint3f& p);
  INLINE LPoint3f get_location(void) const;
};

#include "pointEmitter.I"

#endif // POINTEMITTER_H

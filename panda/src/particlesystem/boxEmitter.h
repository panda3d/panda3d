// Filename: boxEmitter.h
// Created by:  charles (22Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef BOXEMITTER_H
#define BOXEMITTER_H

#include "baseParticleEmitter.h"

////////////////////////////////////////////////////////////////////
//       Class : BoxEmitter
// Description : Describes a voluminous box region in which 
//               particles are generated.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS BoxEmitter : public BaseParticleEmitter {
private:
  LPoint3f _vmin, _vmax;
  LVector3f _launch_vec;

  virtual void assign_initial_values(LPoint3f& pos, LVector3f& vel);

public:
  BoxEmitter(void);
  BoxEmitter(const BoxEmitter &copy);
  virtual ~BoxEmitter(void);

  virtual BaseParticleEmitter *make_copy(void);
  INLINE void set_boundaries(const LPoint3f& vmin, const LPoint3f& vmax);
  INLINE void set_launch_vec(const LVector3f& lv);
};

#include "boxEmitter.I"

#endif // BOXEMITTER_H

// Filename: rectangleEmitter.h
// Created by:  charles (22Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef RECTANGLEEMITTER_H
#define RECTANGLEEMITTER_H

#include "baseParticleEmitter.h"

////////////////////////////////////////////////////////////////////
//       Class : RectangleEmitter
// Description : Describes a planar square region in which
//               particles are generated.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS RectangleEmitter : public BaseParticleEmitter {
private:
  LPoint2f _vmin, _vmax;
  LVector3f _launch_vec;
  
  virtual void assign_initial_values(LPoint3f& pos, LVector3f& vel);

public:
  RectangleEmitter(void);
  RectangleEmitter(const RectangleEmitter &copy);
  virtual ~RectangleEmitter(void);

  virtual BaseParticleEmitter *make_copy(void);
  
  INLINE void set_boundaries(const LPoint2f& vmin, const LPoint2f& vmax);
  INLINE void set_launch_vec(const LVector3f& lv);
};

#include "rectangleEmitter.I"

#endif // RECTANGLEEMITTER_H

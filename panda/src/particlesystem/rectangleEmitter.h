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

  virtual void assign_initial_position(LPoint3f& pos);
  virtual void assign_initial_velocity(LVector3f& vel);

public:
  RectangleEmitter(void);
  RectangleEmitter(const RectangleEmitter &copy);
  virtual ~RectangleEmitter(void);

  virtual BaseParticleEmitter *make_copy(void);

  INLINE void set_min_bound(const LPoint2f& vmin);
  INLINE void set_max_bound(const LPoint2f& vmax);

  INLINE LPoint2f get_min_bound(void) const;
  INLINE LPoint2f get_max_bound(void) const;
};

#include "rectangleEmitter.I"

#endif // RECTANGLEEMITTER_H

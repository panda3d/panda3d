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

  // CUSTOM EMISSION PARAMETERS
  // none

  virtual void assign_initial_position(LPoint3f& pos);
  virtual void assign_initial_velocity(LVector3f& vel);

PUBLISHED:
  BoxEmitter(void);
  BoxEmitter(const BoxEmitter &copy);
  virtual ~BoxEmitter(void);

  virtual BaseParticleEmitter *make_copy(void);

  INLINE void set_min_bound(const LPoint3f& vmin);
  INLINE void set_max_bound(const LPoint3f& vmax);

  INLINE LPoint3f get_min_bound(void) const;
  INLINE LPoint3f get_max_bound(void) const;
};

#include "boxEmitter.I"

#endif // BOXEMITTER_H

// Filename: lineEmitter.h
// Created by:  charles (22Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LINEEMITTER_H
#define LINEEMITTER_H

#include "baseParticleEmitter.h"

////////////////////////////////////////////////////////////////////
//       Class : LineEmitter
// Description : Describes a linear region in which
//               particles are generated.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS LineEmitter : public BaseParticleEmitter {
private:
  LPoint3f _vmin, _vmax;
  LVector3f _launch_vec;

  virtual void assign_initial_values(LPoint3f& pos, LVector3f& vel);
  
public:
  LineEmitter(void);
  LineEmitter(const LineEmitter &copy);
  virtual ~LineEmitter(void);

  virtual BaseParticleEmitter *make_copy(void);

  INLINE void set_endpoints(const LPoint3f& vmin, const LPoint3f& vmax);
  INLINE void set_launch_vec(const LVector3f& lv);
};

#include "lineEmitter.I"

#endif // LINEEMITTER_H

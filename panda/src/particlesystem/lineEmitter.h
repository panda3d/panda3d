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
  LPoint3f _endpoint1, _endpoint2;

  virtual void assign_initial_position(LPoint3f& pos);
  virtual void assign_initial_velocity(LVector3f& vel);

public:
  LineEmitter(void);
  LineEmitter(const LineEmitter &copy);
  virtual ~LineEmitter(void);

  virtual BaseParticleEmitter *make_copy(void);

  INLINE void set_endpoint1(const LPoint3f& point);
  INLINE void set_endpoint2(const LPoint3f& point);

  INLINE LPoint3f get_endpoint1(void) const;
  INLINE LPoint3f get_endpoint2(void) const;
};

#include "lineEmitter.I"

#endif // LINEEMITTER_H

// Filename: baseParticleEmitter.h
// Created by:  charles (14Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef BASEPARTICLEEMITTER_H
#define BASEPARTICLEEMITTER_H

#include <pandabase.h>
#include <referenceCount.h>
#include <luse.h>

#include <mathNumbers.h>

////////////////////////////////////////////////////////////////////
//       Class : BaseParticleEmitter
// Description : Describes a physical region in space in which
//               particles are randomly generated.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS BaseParticleEmitter : public ReferenceCount {
private:
  virtual void assign_initial_values(LPoint3f& pos, LVector3f& vel) = 0;
  LVector3f _offset_force;

protected:
  BaseParticleEmitter(void);
  BaseParticleEmitter(const BaseParticleEmitter &copy);

  float _amplitude;
  float bounded_rand(void);

public:
  virtual ~BaseParticleEmitter(void);
  virtual BaseParticleEmitter *make_copy(void) = 0;

  INLINE void set_offset_force(const LVector3f& of);
  INLINE LVector3f get_offset_force(void) const;

  INLINE void set_amplitude(float a);
  INLINE float get_amplitude(void) const;

  INLINE void generate(LPoint3f& pos, LVector3f& vel);
};

#include "baseParticleEmitter.I"

#endif // BASEPARTICLEEMITTER_H

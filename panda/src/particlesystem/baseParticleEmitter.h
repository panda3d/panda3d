// Filename: baseParticleEmitter.h
// Created by:  charles (14Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef BASEPARTICLEEMITTER_H
#define BASEPARTICLEEMITTER_H

#include "pandabase.h"
#include "referenceCount.h"
#include "luse.h"

#include "particleCommonFuncs.h"

#include "mathNumbers.h"

////////////////////////////////////////////////////////////// //////
//       Class : BaseParticleEmitter
// Description : Describes a physical region in space in which
//               particles are randomly generated.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS BaseParticleEmitter : public ReferenceCount {
PUBLISHED:
  enum emissionType {
    ET_EXPLICIT, // all particles are emitted in parallel along the same vector
    ET_RADIATE,  // all particles radiate away from a single point
    ET_CUSTOM    // particle launch vectors are dependent on particular derived emitter
  };

  virtual ~BaseParticleEmitter();
  virtual BaseParticleEmitter *make_copy() = 0;

  void generate(LPoint3& pos, LVector3& vel);

  INLINE void set_emission_type(emissionType et);
  INLINE void set_amplitude(PN_stdfloat a);
  INLINE void set_amplitude_spread(PN_stdfloat as);
  INLINE void set_offset_force(const LVector3& of);  // this is a constant force applied to all particles
  INLINE void set_explicit_launch_vector(const LVector3& elv);
  INLINE void set_radiate_origin(const LPoint3& ro);

  INLINE emissionType get_emission_type() const;
  INLINE PN_stdfloat get_amplitude() const;
  INLINE PN_stdfloat get_amplitude_spread() const;
  INLINE LVector3 get_offset_force() const;
  INLINE LVector3 get_explicit_launch_vector() const;
  INLINE LPoint3 get_radiate_origin() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent=0) const;

protected:
  BaseParticleEmitter();
  BaseParticleEmitter(const BaseParticleEmitter &copy);

  emissionType _emission_type;
  LVector3 _explicit_launch_vector;
  LPoint3  _radiate_origin;

  PN_stdfloat _amplitude;
  PN_stdfloat _amplitude_spread;

private:
  // these should be called in sequence (pos, then vel)
  virtual void assign_initial_position(LPoint3& pos) = 0;
  virtual void assign_initial_velocity(LVector3& vel) = 0;

  LVector3 _offset_force;
};

#include "baseParticleEmitter.I"

#endif // BASEPARTICLEEMITTER_H

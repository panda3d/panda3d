// Filename: baseParticleEmitter.h
// Created by:  charles (14Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
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

  void generate(LPoint3f& pos, LVector3f& vel);

  INLINE void set_emission_type(emissionType et);
  INLINE void set_amplitude(float a);
  INLINE void set_amplitude_spread(float as);
  INLINE void set_offset_force(const LVector3f& of);  // this is a constant force applied to all particles
  INLINE void set_explicit_launch_vector(const LVector3f& elv);
  INLINE void set_radiate_origin(const LPoint3f& ro);

  INLINE emissionType get_emission_type() const;
  INLINE float get_amplitude() const;
  INLINE float get_amplitude_spread() const;
  INLINE LVector3f get_offset_force() const;
  INLINE LVector3f get_explicit_launch_vector() const;
  INLINE LPoint3f get_radiate_origin() const;
  
  virtual void output(ostream &out, unsigned int indent=0) const;

protected:
  BaseParticleEmitter();
  BaseParticleEmitter(const BaseParticleEmitter &copy);

  emissionType _emission_type;
  LVector3f _explicit_launch_vector;
  LPoint3f  _radiate_origin;

  float _amplitude;
  float _amplitude_spread;

private:
  // these should be called in sequence (pos, then vel)
  virtual void assign_initial_position(LPoint3f& pos) = 0;
  virtual void assign_initial_velocity(LVector3f& vel) = 0;

  LVector3f _offset_force;
};

#include "baseParticleEmitter.I"

#endif // BASEPARTICLEEMITTER_H

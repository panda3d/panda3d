// Filename: boxEmitter.h
// Created by:  charles (22Jun00)
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

#ifndef BOXEMITTER_H
#define BOXEMITTER_H

#include "baseParticleEmitter.h"

////////////////////////////////////////////////////////////////////
//       Class : BoxEmitter
// Description : Describes a voluminous box region in which
//               particles are generated.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS BoxEmitter : public BaseParticleEmitter {
PUBLISHED:
  BoxEmitter();
  BoxEmitter(const BoxEmitter &copy);
  virtual ~BoxEmitter();

  virtual BaseParticleEmitter *make_copy();

  INLINE void set_min_bound(const LPoint3& vmin);
  INLINE void set_max_bound(const LPoint3& vmax);

  INLINE LPoint3 get_min_bound() const;
  INLINE LPoint3 get_max_bound() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent=0) const;

private:
  LPoint3 _vmin;
  LPoint3 _vmax;

  // CUSTOM EMISSION PARAMETERS
  // none

  virtual void assign_initial_position(LPoint3& pos);
  virtual void assign_initial_velocity(LVector3& vel);
};

#include "boxEmitter.I"

#endif // BOXEMITTER_H

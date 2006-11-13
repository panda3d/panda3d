// Filename: ringEmitter.h
// Created by:  charles (22Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef ARCEMITTER_H
#define ARCEMITTER_H

#include "ringEmitter.h"

////////////////////////////////////////////////////////////////////
//       Class : ArcEmitter
// Description : Describes a planar ring region in which
//               particles are generated.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS ArcEmitter : public RingEmitter {
PUBLISHED:
  ArcEmitter();
  ArcEmitter(const ArcEmitter &copy);
  virtual ~ArcEmitter();

  virtual BaseParticleEmitter *make_copy();

  INLINE void set_start_angle(float angle);
  INLINE void set_end_angle(float angle);
  INLINE void set_arc(float startAngle, float endAngle);

  INLINE float get_start_angle();
  INLINE float get_end_angle();

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent=0) const;

private:
  // our emitter limits
  float _start_theta;
  float _end_theta;
  ///////////////////////////////

  virtual void assign_initial_position(LPoint3f& pos);
};

#include "arcEmitter.I"

#endif // ARCEMITTER_H

// Filename: rectangleEmitter.h
// Created by:  charles (22Jun00)
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

#ifndef RECTANGLEEMITTER_H
#define RECTANGLEEMITTER_H

#include "baseParticleEmitter.h"

////////////////////////////////////////////////////////////////////
//       Class : RectangleEmitter
// Description : Describes a planar square region in which
//               particles are generated.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS RectangleEmitter : public BaseParticleEmitter {
PUBLISHED:
  RectangleEmitter(void);
  RectangleEmitter(const RectangleEmitter &copy);
  virtual ~RectangleEmitter(void);

  virtual BaseParticleEmitter *make_copy(void);

  INLINE void set_min_bound(const LPoint2f& vmin);
  INLINE void set_max_bound(const LPoint2f& vmax);

  INLINE LPoint2f get_min_bound(void) const;
  INLINE LPoint2f get_max_bound(void) const;
  
  virtual void output(ostream &out, unsigned int indent=0) const;

private:
  LPoint2f _vmin, _vmax;

  // CUSTOM EMISSION PARAMETERS
  // none

  virtual void assign_initial_position(LPoint3f& pos);
  virtual void assign_initial_velocity(LVector3f& vel);
};

#include "rectangleEmitter.I"

#endif // RECTANGLEEMITTER_H

// Filename: lineEmitter.h
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

#ifndef LINEEMITTER_H
#define LINEEMITTER_H

#include "baseParticleEmitter.h"

////////////////////////////////////////////////////////////////////
//       Class : LineEmitter
// Description : Describes a linear region in which
//               particles are generated.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS LineEmitter : public BaseParticleEmitter {
PUBLISHED:
  LineEmitter(void);
  LineEmitter(const LineEmitter &copy);
  virtual ~LineEmitter(void);

  virtual BaseParticleEmitter *make_copy(void);

  INLINE void set_endpoint1(const LPoint3f& point);
  INLINE void set_endpoint2(const LPoint3f& point);

  INLINE LPoint3f get_endpoint1(void) const;
  INLINE LPoint3f get_endpoint2(void) const;
  
  virtual void output(ostream &out, unsigned int indent=0) const;

private:
  LPoint3f _endpoint1, _endpoint2;

  // CUSTOM EMISSION PARAMETERS
  // none

  virtual void assign_initial_position(LPoint3f& pos);
  virtual void assign_initial_velocity(LVector3f& vel);
};

#include "lineEmitter.I"

#endif // LINEEMITTER_H

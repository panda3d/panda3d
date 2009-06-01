// Filename: physxSegment.h
// Created by:  pratt (Apr 7, 2006)
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

#ifndef PHYSXSEGMENT_H
#define PHYSXSEGMENT_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxSegment
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxSegment {
PUBLISHED:
  PhysxSegment();
  PhysxSegment(const LVecBase3f &p0, const LVecBase3f &p1);
  PhysxSegment(const PhysxSegment & seg);
  ~PhysxSegment();

  INLINE void compute_direction(LVecBase3f & dir) const;
  INLINE float compute_length() const;
  INLINE void compute_point(LVecBase3f & pt, float t) const;
  INLINE float compute_square_length() const;
  LVecBase3f get_origin() const;
  INLINE void set_origin_direction(const LVecBase3f & origin, const LVecBase3f & direction);

  LVecBase3f get_p0() const;
  LVecBase3f get_p1() const;

  void set_p0( LVecBase3f value );
  void set_p1( LVecBase3f value );

public:
  NxSegment *nSegment;
};

#include "physxSegment.I"

#endif // HAVE_PHYSX

#endif // PHYSXSEGMENT_H

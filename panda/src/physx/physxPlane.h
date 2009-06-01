// Filename: physxPlane.h
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

#ifndef PHYSXPLANE_H
#define PHYSXPLANE_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxPlane
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxPlane {
PUBLISHED:
  PhysxPlane(const PhysxPlane & plane);
  ~PhysxPlane();

  INLINE bool belongs(const LVecBase3f & p) const;
  INLINE float distance(const LVecBase3f & p) const;
  INLINE void inverse_transform(const LMatrix4f & transform, PhysxPlane & transformed) const;
  INLINE void normalize();
  LVecBase3f point_in_plane() const;
  LVecBase3f project(const LVecBase3f & p) const;
  PhysxPlane & set(const LVecBase3f & p0, const LVecBase3f & p1, const LVecBase3f & p2);
  INLINE void transform(const LMatrix4f & transform, PhysxPlane & transformed) const;
  PhysxPlane & zero();

  INLINE float get_d() const;
  LVecBase3f get_normal() const;

  INLINE void set_d( float value );
  void set_normal( LVecBase3f value );

public:
  NxPlane *nPlane;
};

#include "physxPlane.I"

#endif // HAVE_PHYSX

#endif // PHYSXPLANE_H

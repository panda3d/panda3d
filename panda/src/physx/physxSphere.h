// Filename: physxSphere.h
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

#ifndef PHYSXSPHERE_H
#define PHYSXSPHERE_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxSphere
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxSphere {
PUBLISHED:
  PhysxSphere(const PhysxSphere & sphere);
  ~PhysxSphere();

  INLINE bool contains(const LVecBase3f & min, const LVecBase3f & max) const;
  INLINE bool intersect(const PhysxSphere & sphere) const;
  INLINE bool is_valid() const;

  LVecBase3f get_center() const;
  INLINE float get_radius() const;

  void set_center( LVecBase3f value );
  INLINE void set_radius( float value );

public:
  NxSphere *nSphere;
};

#include "physxSphere.I"

#endif // HAVE_PHYSX

#endif // PHYSXSPHERE_H

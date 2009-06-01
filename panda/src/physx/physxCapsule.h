// Filename: physxCapsule.h
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

#ifndef PHYSXCAPSULE_H
#define PHYSXCAPSULE_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

#include "physxSegment.h"

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxCapsule
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxCapsule : public PhysxSegment {
PUBLISHED:
  PhysxCapsule(const PhysxSegment & seg, float _radius);
  ~PhysxCapsule();


  INLINE float get_radius() const;

  INLINE void set_radius( float value );

public:
  NxCapsule *nCapsule;
};

#include "physxCapsule.I"

#endif // HAVE_PHYSX

#endif // PHYSXCAPSULE_H

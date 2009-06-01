// Filename: physxSphereShapeDesc.h
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

#ifndef PHYSXSPHERESHAPEDESC_H
#define PHYSXSPHERESHAPEDESC_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

#include "physxShapeDesc.h"

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxSphereShapeDesc
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxSphereShapeDesc : public PhysxShapeDesc {
PUBLISHED:
  PhysxSphereShapeDesc();

  INLINE bool is_valid() const;
  INLINE void set_to_default();

  INLINE float get_radius() const;

  INLINE void set_radius( float value );

public:
  NxSphereShapeDesc nSphereShapeDesc;
};

#include "physxSphereShapeDesc.I"

#endif // HAVE_PHYSX

#endif // PHYSXSPHERESHAPEDESC_H

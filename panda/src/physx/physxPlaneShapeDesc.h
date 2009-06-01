// Filename: physxPlaneShapeDesc.h
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

#ifndef PHYSXPLANESHAPEDESC_H
#define PHYSXPLANESHAPEDESC_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

#include "physxShapeDesc.h"

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxPlaneShapeDesc
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxPlaneShapeDesc : public PhysxShapeDesc {
PUBLISHED:
  PhysxPlaneShapeDesc();

  INLINE bool is_valid() const;
  INLINE void set_to_default();

  INLINE float get_d() const;
  LVecBase3f get_normal() const;

  INLINE void set_d( float value );
  void set_normal( LVecBase3f value );

public:
  NxPlaneShapeDesc nPlaneShapeDesc;
};

#include "physxPlaneShapeDesc.I"

#endif // HAVE_PHYSX

#endif // PHYSXPLANESHAPEDESC_H

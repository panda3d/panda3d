// Filename: physxCapsuleShapeDesc.h
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

#ifndef PHYSXCAPSULESHAPEDESC_H
#define PHYSXCAPSULESHAPEDESC_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

#include "physxShapeDesc.h"

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxCapsuleShapeDesc
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxCapsuleShapeDesc : public PhysxShapeDesc {
PUBLISHED:
  PhysxCapsuleShapeDesc();

  INLINE bool is_valid() const;
  INLINE void set_to_default();

  INLINE unsigned int get_flags() const;
  INLINE float get_height() const;
  INLINE float get_radius() const;

  INLINE void set_flags( unsigned int value );
  INLINE void set_height( float value );
  INLINE void set_radius( float value );

public:
  NxCapsuleShapeDesc nCapsuleShapeDesc;
};

#include "physxCapsuleShapeDesc.I"

#endif // HAVE_PHYSX

#endif // PHYSXCAPSULESHAPEDESC_H

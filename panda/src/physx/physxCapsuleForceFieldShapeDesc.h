// Filename: physxCapsuleForceFieldShapeDesc.h
// Created by:  enn0x (06Nov09)
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

#ifndef PHYSXCAPSULEFORCEFIELDSHAPEDESC_H
#define PHYSXCAPSULEFORCEFIELDSHAPEDESC_H

#include "pandabase.h"

#include "physxForceFieldShapeDesc.h"
#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxCapsuleForceFieldShapeDesc
// Description : Descriptor for a capsule force field shape.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxCapsuleForceFieldShapeDesc : public PhysxForceFieldShapeDesc {

PUBLISHED:
  INLINE PhysxCapsuleForceFieldShapeDesc();
  INLINE ~PhysxCapsuleForceFieldShapeDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

  void set_radius(float radius);
  void set_height(float height);

  float get_radius() const;
  float get_height() const;

public:
  NxForceFieldShapeDesc *ptr() const { return (NxForceFieldShapeDesc *)&_desc; };
  NxCapsuleForceFieldShapeDesc _desc;
};

#include "physxCapsuleForceFieldShapeDesc.I"

#endif // PHYSXCAPSULEFORCEFIELDSHAPEDESC_H

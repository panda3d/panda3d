// Filename: physxSphereForceFieldShapeDesc.h
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

#ifndef PHYSXSPHEREFORCEFIELDSHAPEDESC_H
#define PHYSXSPHEREFORCEFIELDSHAPEDESC_H

#include "pandabase.h"

#include "physxForceFieldShapeDesc.h"
#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxSphereForceFieldShapeDesc
// Description : A descriptor for a sphere force field shape.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxSphereForceFieldShapeDesc : public PhysxForceFieldShapeDesc {

PUBLISHED:
  INLINE PhysxSphereForceFieldShapeDesc();
  INLINE ~PhysxSphereForceFieldShapeDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

  void set_radius(float radius);

  float get_radius() const;

public:
  NxForceFieldShapeDesc *ptr() const { return (NxForceFieldShapeDesc *)&_desc; };
  NxSphereForceFieldShapeDesc _desc;
};

#include "physxSphereForceFieldShapeDesc.I"

#endif // PHYSXSPHEREFORCEFIELDSHAPEDESC_H
